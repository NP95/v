//========================================================================== //
// Copyright (c) 2022, Stephen Henry
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//========================================================================== //

`include "common_defs.vh"

`include "v_pkg.vh"
`include "cfg_pkg.vh"

module v_pipe_update_exe (
// -------------------------------------------------------------------------- //
// Command Interface
  input wire v_pkg::cmd_t                                i_pipe_cmd_r
, input wire v_pkg::key_t                                i_pipe_key_r
, input wire v_pkg::volume_t                             i_pipe_volume_r
//
, input wire logic                                       i_pipe_match_hit_r
, input wire logic                                       i_pipe_match_full_r
, input wire logic [cfg_pkg::ENTRIES_N - 1:0]            i_pipe_match_sel_r
, input wire logic [cfg_pkg::ENTRIES_N - 1:0]            i_pipe_mask_cmp_r

// -------------------------------------------------------------------------- //
// State Current
, input wire logic [cfg_pkg::ENTRIES_N - 1:0]            i_stcur_vld_r
, input wire v_pkg::key_t [cfg_pkg::ENTRIES_N - 1:0]     i_stcur_keys_r
, input wire v_pkg::volume_t [cfg_pkg::ENTRIES_N - 1:0]  i_stcur_volumes_r
//
, input wire v_pkg::listsize_t                           i_stcur_listsize_r

// -------------------------------------------------------------------------- //
// State Next
, output wire logic [cfg_pkg::ENTRIES_N - 1:0]           o_stnxt_vld
, output wire v_pkg::key_t [cfg_pkg::ENTRIES_N - 1:0]    o_stnxt_keys
, output wire v_pkg::volume_t [cfg_pkg::ENTRIES_N - 1:0] o_stnxt_volumes
//
, output wire v_pkg::listsize_t                          o_stnxt_listsize

// -------------------------------------------------------------------------- //
// Notify Interrace
, output wire logic                                      o_notify_vld
, output wire v_pkg::key_t                               o_notify_key
, output wire v_pkg::volume_t                            o_notify_volume
);

// ========================================================================== //
//                                                                            //
//  Wires                                                                     //
//                                                                            //
// ========================================================================== //

// Decoder:
logic                                      op_clr;
logic                                      op_add;
logic                                      op_del;
logic                                      op_rep;

// Match:
v_pkg::volume_t                            match_volume;

// Add:
logic [cfg_pkg::ENTRIES_N - 1:0]           add_vld_shift;
logic [cfg_pkg::ENTRIES_N - 1:0]           add_vld_sel;
logic [cfg_pkg::ENTRIES_N - 1:0]           add_vld;
logic                                      add_listsize_inc;

logic [cfg_pkg::ENTRIES_N - 1:0]           add_mask_left;
logic [cfg_pkg::ENTRIES_N - 1:0]           add_mask_insert;

// Delete:
logic [cfg_pkg::ENTRIES_N - 1:0]           del_vld;
logic [cfg_pkg::ENTRIES_N - 1:0]           del_sel;
logic [cfg_pkg::ENTRIES_N - 1:0]           del_mask_left;
logic                                      del_listsize_dec;

// Validity:
logic [cfg_pkg::ENTRIES_N - 1:0]           vld_nxt;

//
logic [cfg_pkg::ENTRIES_N - 1:0]           mask_right;
logic [cfg_pkg::ENTRIES_N - 1:0]           mask_left;
logic [cfg_pkg::ENTRIES_N - 1:0]           mask_insert_key;
logic [cfg_pkg::ENTRIES_N - 1:0]           mask_insert_vol;

//
v_pkg::listsize_t                          stnxt_listsize_nxt;
v_pkg::listsize_t                          stnxt_listsize;
logic                                      stnxt_listsize_inc;
logic                                      stnxt_listsize_dec;
logic                                      stnxt_listsize_def;


logic [cfg_pkg::ENTRIES_N - 1:0]           stnxt_keys_do_upt;
v_pkg::key_t [cfg_pkg::ENTRIES_N - 1:0]    stnxt_keys_upt;
v_pkg::key_t [cfg_pkg::ENTRIES_N - 1:0]    stnxt_keys;

logic [cfg_pkg::ENTRIES_N - 1:0]           stnxt_volumes_do_upt;
v_pkg::volume_t [cfg_pkg::ENTRIES_N - 1:0] stnxt_volumes_upt;
v_pkg::volume_t [cfg_pkg::ENTRIES_N - 1:0] stnxt_volumes;

// Notify:
logic                                      notify_cleared_list;
logic                                      notify_did_add;
logic                                      notify_did_del;
logic                                      notify_did_rep_or_del;
logic                                      notify_vld;
v_pkg::key_t                               notify_key;
v_pkg::volume_t                            notify_volume;

// ========================================================================== //
//                                                                            //
//  Command Decode                                                            //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
// Command Decoder
assign op_clr = (i_pipe_cmd_r == v_pkg::CMD_CLEAR);
assign op_add = (i_pipe_cmd_r == v_pkg::CMD_ADD);
assign op_del = (i_pipe_cmd_r == v_pkg::CMD_DELETE);
assign op_rep = (i_pipe_cmd_r == v_pkg::CMD_REPLACE);

// ========================================================================== //
//                                                                            //
//  Add Command                                                               //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
// The location into which the current command is to be inserted is the leading
// zero of the comparison vector).
//
//  Compare 0  0  0  0  0  0  0  0  0  0  1  1  1  1
//
//                                     +-- Insertion position
//                                     |
//  Insert  0  0  0  0  0  0  0  0  0  1  0  0  0  0
//
lzd #(.W(cfg_pkg::ENTRIES_N), .DETECT_ZERO(1), .FROM_LSB(1)) u_lzd (
  //
    .i_x                                (i_pipe_mask_cmp_r)
  //
  , .o_y                                (add_mask_insert)
);

// -------------------------------------------------------------------------- //
// Compute a mask denoting the valid entries to be shifted leftwards to allow
// the new element to be inserted.
//
//  Valid   0  0  0  0  0  0  0  1  1  1  1  1  1  1
//
//  Insert  0  0  0  0  0  0  0  0  0  1  0  0  0  0
//
//  Mask    1  1  1  1  1  1  1  1  1  1  0  0  0  0
//
mask #(.W(cfg_pkg::ENTRIES_N), .TOWARDS_LSB(0), .INCLUSIVE(1)) u_mask_add (
  //
    .i_x                                (add_mask_insert)
  //
  , .o_y                                (add_mask_left)
);

// -------------------------------------------------------------------------- //
// Qualify the mask on element validity.
//
//  Valid   0  0  0  0  0  0  0  1  1  1  1  1  1  1
//
//  Mask    1  1  1  1  1  1  1  1  1  1  0  0  0  0
//
//  Sel     0  0  0  0  0  0  0  1  1  1  0  0  0  0

assign add_vld_sel = (i_stcur_vld_r & add_mask_left);

// -------------------------------------------------------------------------- //
// Derive validity mask of newly shifted elements
//
//  Sel     0  0  0  0  0  0  0  1  1  1  0  0  0  0
//
//  VldSh   0  0  0  0  0  0  1  1  1  0  0  0  0  0
//
assign add_vld_shift = (add_vld_sel << 1);

// -------------------------------------------------------------------------- //
// Next valid mask for the add command case "right sign extended" value (i.e. we
// simply push a 1'b1 into the LSB regardless of the current "full-ness" of the
// context. In the full case, although a 'add' may or may-not occur, the next
// state of the mask, equals the initial case (i.e. '1 becomes '1).
//
//  VldCur  0  0  0  0  0  0  0  1  1  1  1  1  1  1
//
//  VldNxt  0  0  0  0  0  0  1  1  1  1  1  1  1  1
//
assign add_vld = {i_stcur_vld_r [cfg_pkg::ENTRIES_N - 2:0], 1'b1};

// -------------------------------------------------------------------------- //
// Increment listsize (integer) value whenever we're executing an add command
// and the current context is not full (i.e. we are guarenteed to insert the
// element somewhere).
//
assign add_listsize_inc = (~i_pipe_match_full_r);

// ========================================================================== //
//                                                                            //
//  Delete Command                                                            //
//                                                                            //
// ========================================================================== //

if (cfg_pkg::ALLOW_DUPLICATES) begin

// -------------------------------------------------------------------------- //
// In the case where we can have multiple keys within the same context, we have
// have multiple matches. The table update logic supports only one
// insertion/deletion operation per cycle (which is not unreasonable). Therefore
// by convention, delete the right-most (towards LSB) element using a
// prioritization network.
//
//  Match   0  0  0  0  0  1  1  1  0  0  0  0  0  0
//
//  DelSel  0  0  0  0  0  0  0  1  0  0  0  0  0  0

pri #(.W(cfg_pkg::ENTRIES_N), .FROM_LSB(1)) u_pri_del (
  //
    .i_x                                (i_pipe_match_sel_r)
  //
  , .o_y                                (del_sel)
);

end else begin

// -------------------------------------------------------------------------- //
// Otherwise, stimulus is constrained such that duplicate keys cannot exist
// within the same context (we do not care about inter-context key
// aliasing). Therefore, no prioritization is required, we simply take the 1-hot
// match vector.
//
//  Match   0  0  0  0  0  0  0  1  0  0  0  0  0  0
//
assign del_sel = i_pipe_match_sel_r;

end // else: !if(cfg_pkg::ALLOW_DUPLICATES)

// -------------------------------------------------------------------------- //
// Compute a left-leaning mask from the 1-hot match vector to denote the
// elements to be shifted right after the matched element has been removed.
//
//  Match   0  0  0  0  0  0  0  1  0  0  0  0  0  0
//
//  Mask    1  1  1  1  1  1  1  1  0  0  0  0  0  0
//
mask #(.W(cfg_pkg::ENTRIES_N), .TOWARDS_LSB(0), .INCLUSIVE(1)) u_mask_del (
  //
    .i_x                                (del_sel)
  //
  , .o_y                                (del_mask_left)
);

// -------------------------------------------------------------------------- //
// On a successful delete, the next valid bit-vector is simply the existing
// right-shifted one bit.
//
//  VldOld  0  0  0  0  1  1  1  1  1  1  1  1  1  1
//
//  VldNxt  0  0  0  0  0  1  1  1  1  1  1  1  1  1
//
assign del_vld = i_pipe_match_hit_r ? (i_stcur_vld_r >> 1) : i_stcur_vld_r;

// -------------------------------------------------------------------------- //
// On delete, decrement the current listsize count whenever an element has been
// removed (i.e. on a match hit operation).
//
assign del_listsize_dec = i_pipe_match_hit_r;

// ========================================================================== //
//                                                                            //
//  Validity Update                                                           //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
// Compute update to the validatity vector. On a replacement, the overall state
// of the vector remains unchanged regardless of whether a replacement has taken
// place.
//
assign vld_nxt = ({cfg_pkg::ENTRIES_N{op_add}} & add_vld) |
                 ({cfg_pkg::ENTRIES_N{op_del}} & del_vld) |
                 ({cfg_pkg::ENTRIES_N{op_rep}} & i_stcur_vld_r);

// -------------------------------------------------------------------------- //
// Compute final validity vector. On a CLEAR op., all bits are cleared
// regardless of prior state.
assign o_stnxt_vld = ({cfg_pkg::ENTRIES_N{~op_clr}} & vld_nxt);

// ========================================================================== //
//                                                                            //
//  Table Update                                                              //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
// Shift elements left (accept right element) on an add command
//
assign mask_right = ({cfg_pkg::ENTRIES_N{op_add}} & add_vld_shift);

// -------------------------------------------------------------------------- //
// Shift elements right (accept left element) on an del command
//
assign mask_left = ({cfg_pkg::ENTRIES_N{op_del}} & del_mask_left);

// -------------------------------------------------------------------------- //
// Insert element at position 'X' on ADD.
//
assign mask_insert_key = ({cfg_pkg::ENTRIES_N{op_add}} & add_mask_insert);

// -------------------------------------------------------------------------- //
// Update volume on ADD or REPlacement commands
//
assign mask_insert_vol = ({cfg_pkg::ENTRIES_N{op_add}} & add_mask_insert) |
                         ({cfg_pkg::ENTRIES_N{op_rep}} & i_pipe_match_sel_r);

// -------------------------------------------------------------------------- //
// State update logic
//
for (genvar i = 0; i < cfg_pkg::ENTRIES_N; i++) begin : upt_GEN

  if (i == 0) begin : upt_0_GEN
    // Right-most entry:

    assign stnxt_keys_do_upt [i] = (mask_insert_key [i] | mask_left [i]);

    assign stnxt_keys_upt [i] =
      // Insertion,
      ({v_pkg::KEY_BITS{mask_insert_key [i]}} & i_pipe_key_r) |
      // Take left,
      ({v_pkg::KEY_BITS{mask_left [i]}} & i_stcur_keys_r [i + 1]);

    assign stnxt_volumes_do_upt [i] = (mask_insert_vol [i] | mask_left [i]);

    assign stnxt_volumes_upt [i] =
      // Insertion
      ({v_pkg::VOLUME_BITS{mask_insert_vol [i]}} & i_pipe_volume_r) |
      // Take left,
      ({v_pkg::VOLUME_BITS{mask_left [i]}} & i_stcur_volumes_r [i + 1]);

  end else if (i == cfg_pkg::ENTRIES_N - 1) begin : upt_N_GEN
    // Left-most entry:

    assign stnxt_keys_do_upt [i] = (mask_insert_key [i] | mask_right [i]);

    assign stnxt_keys_upt [i] =
      // Insertion,
      ({v_pkg::KEY_BITS{mask_insert_key [i]}} & i_pipe_key_r) |
      // Take right,
      ({v_pkg::KEY_BITS{mask_right [i]}} & i_stcur_keys_r [i - 1]);

    assign stnxt_volumes_do_upt [i] = (mask_insert_vol [i] | mask_right [i]);

    assign stnxt_volumes_upt [i] =
      // Insertion
      ({v_pkg::VOLUME_BITS{mask_insert_vol [i]}} & i_pipe_volume_r) |
      // Take right,
      ({v_pkg::VOLUME_BITS{mask_right [i]}} & i_stcur_volumes_r [i - 1]);

  end else begin : upt_i_GEN
    // Internal entry:

    assign stnxt_keys_do_upt [i] =
      (mask_insert_key [i] | mask_left [i] | mask_right [i]);

    assign stnxt_keys_upt [i] =
      // Insertion,
      ({v_pkg::KEY_BITS{mask_insert_key [i]}} & i_pipe_key_r) |
      // Take left,
      ({v_pkg::KEY_BITS{mask_left [i]}} & i_stcur_keys_r [i + 1]) |
      // Take right,
      ({v_pkg::KEY_BITS{mask_right [i]}} & i_stcur_keys_r [i - 1]);

    assign stnxt_volumes_do_upt [i] =
      (mask_insert_vol [i] | mask_left [i] | mask_right [i]);

    assign stnxt_volumes_upt [i] =
      // Insertion
      ({v_pkg::VOLUME_BITS{mask_insert_vol [i]}} & i_pipe_volume_r) |
      // Take left,
      ({v_pkg::VOLUME_BITS{mask_left [i]}} & i_stcur_volumes_r [i + 1]) |
      // Take right,
      ({v_pkg::VOLUME_BITS{mask_right [i]}} & i_stcur_volumes_r [i - 1]);

  end : upt_i_GEN


  // Select update or retain prior.
  assign stnxt_keys [i] =
      ({v_pkg::KEY_BITS{ stnxt_keys_do_upt [i]}} & stnxt_keys_upt [i]) |
      ({v_pkg::KEY_BITS{~stnxt_keys_do_upt [i]}} & i_stcur_keys_r [i]);

  // Select update or retain prior.
  assign stnxt_volumes [i] =
      ({v_pkg::VOLUME_BITS{ stnxt_volumes_do_upt [i]}} & stnxt_volumes_upt [i]) |
      ({v_pkg::VOLUME_BITS{~stnxt_volumes_do_upt [i]}} & i_stcur_volumes_r [i]);

end : upt_GEN

// ========================================================================== //
//                                                                            //
//  List Size                                                                 //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
// Compute update to list size as function of current command
//
assign stnxt_listsize_inc = (op_add & add_listsize_inc);
assign stnxt_listsize_dec = (op_del & del_listsize_dec);
assign stnxt_listsize_def = ~(stnxt_listsize_inc | stnxt_listsize_dec);

// -------------------------------------------------------------------------- //
// Listsize update increment (on add)/decrement (on delete)
//
assign stnxt_listsize_nxt =
      ({v_pkg::LISTSIZE_W{stnxt_listsize_inc}} & (i_stcur_listsize_r + 'b1))
    | ({v_pkg::LISTSIZE_W{stnxt_listsize_dec}} & (i_stcur_listsize_r - 'b1))
    | ({v_pkg::LISTSIZE_W{stnxt_listsize_def}} &  i_stcur_listsize_r);

// -------------------------------------------------------------------------- //
// Next listsize is conditionally reset to '0 on a clear operation regardless of
// prior computed values.
//
assign stnxt_listsize = ({v_pkg::LISTSIZE_W{~op_clr}} & stnxt_listsize_nxt);

// ========================================================================== //
//                                                                            //
//  Notify                                                                    //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
// Notify on clear command whenever head element was valid.
//
assign notify_cleared_list = op_clr & i_stcur_vld_r [0];

// -------------------------------------------------------------------------- //
// Notify on add command to head element.
//
assign notify_did_add = op_add & add_mask_insert [0];

// -------------------------------------------------------------------------- //
// Notify on delete command on the head element.
//
assign notify_did_del = op_del & i_pipe_match_sel_r [0];

// -------------------------------------------------------------------------- //
// Notify on delete to or replacement on head element
//
assign notify_did_rep_or_del = (op_rep | op_del) & i_pipe_match_sel_r [0];

// -------------------------------------------------------------------------- //
// Issue notify message on either of these prior conditions.
//
assign notify_vld =
  (notify_cleared_list | notify_did_add | notify_did_rep_or_del);

// -------------------------------------------------------------------------- //
// Notify key is current command key, indicated by the hit. On a Clear we ignore
// the key returned as we may have just cleared an empty context (where the head
// entry is invalid).
//
assign notify_key = i_pipe_key_r;

// -------------------------------------------------------------------------- //
// Select volume for matching Entry
//
mux #(.N(cfg_pkg::ENTRIES_N), .W(v_pkg::VOLUME_BITS)) u_max_match_volume (
  //
    .i_x                      (i_stcur_volumes_r)
  , .i_sel                    (i_pipe_match_sel_r)
  //
  , .o_y                      (match_volume)
);

// -------------------------------------------------------------------------- //
// Notify volume is the volume placed into the head position, or the value just
// removed. On clear, we don't case since the volume is to become invalid and we
// don't consider if the context was initially empty.
assign notify_volume =
  ({v_pkg::VOLUME_BITS{notify_did_add}} & i_pipe_volume_r) |
  ({v_pkg::VOLUME_BITS{notify_did_del}} & match_volume);

// ========================================================================== //
//                                                                            //
//  Outputs                                                                   //
//                                                                            //
// ========================================================================== //

assign o_stnxt_keys = stnxt_keys;
assign o_stnxt_volumes = stnxt_volumes;
assign o_stnxt_listsize = stnxt_listsize;

assign o_notify_vld =  notify_vld;
assign o_notify_key = notify_key;
assign o_notify_volume = notify_volume;

endmodule // v_pipe_update_exe
