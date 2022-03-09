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

module v_pipe_update_exe (
// -------------------------------------------------------------------------- //
// Command Interface
  input v_pkg::cmd_t                              i_pipe_cmd_r
, input v_pkg::key_t                              i_pipe_key_r
, input v_pkg::volume_t                           i_pipe_volume_r

// -------------------------------------------------------------------------- //
// State Current
, input [v_pkg::ENTRIES_N - 1:0]                  i_stcur_vld_r
, input v_pkg::key_t [v_pkg::ENTRIES_N - 1:0]     i_stcur_keys_r
, input v_pkg::volume_t [v_pkg::ENTRIES_N - 1:0]  i_stcur_volumes_r
//
, input v_pkg::listsize_t                         i_stcur_listsize_r

// -------------------------------------------------------------------------- //
// State Next
, output logic [v_pkg::ENTRIES_N - 1:0]           o_stnxt_vld
, output v_pkg::key_t [v_pkg::ENTRIES_N - 1:0]    o_stnxt_keys
, output v_pkg::volume_t [v_pkg::ENTRIES_N - 1:0] o_stnxt_volumes
//
, output v_pkg::listsize_t                        o_stnxt_listsize

// -------------------------------------------------------------------------- //
// Notify Interrace
, output logic                                    o_notify_vld
, output v_pkg::key_t                             o_notify_key
, output v_pkg::volume_t                          o_notify_volume
);

// ========================================================================== //
//                                                                            //
//  Wires                                                                     //
//                                                                            //
// ========================================================================== //

// Decoder:
logic                                   op_clr;
logic                                   op_add;
logic                                   op_del;
logic                                   op_rep;

// Match:
logic [v_pkg::ENTRIES_N - 1:0]          match_sel;
logic                                   match_hit;
logic                                   match_full;
v_pkg::volume_t                         match_volume;

// Add:
logic [v_pkg::ENTRIES_N - 1:0]          add_mask_cmp;
logic [v_pkg::ENTRIES_N - 1:0]          add_vld_shift;
logic [v_pkg::ENTRIES_N - 1:0]          add_vld_sel;
logic [v_pkg::ENTRIES_N - 1:0]          add_vld;
logic                                   add_listsize_inc;

logic [v_pkg::ENTRIES_N - 1:0]          add_mask_left;
logic [v_pkg::ENTRIES_N - 1:0]          add_mask_insert;

// Delete:
logic [v_pkg::ENTRIES_N - 1:0]          del_vld_shift;
logic [v_pkg::ENTRIES_N - 1:0]          del_vld;
logic [v_pkg::ENTRIES_N - 1:0]          del_sel;
logic [v_pkg::ENTRIES_N - 1:0]          del_mask_left;
logic                                   del_listsize_dec;

// Validity:
logic [v_pkg::ENTRIES_N - 1:0]          vld_nxt;

//
logic [v_pkg::ENTRIES_N - 1:0]          mask_right;
logic [v_pkg::ENTRIES_N - 1:0]          mask_left;
logic [v_pkg::ENTRIES_N - 1:0]          mask_insert_key;
logic [v_pkg::ENTRIES_N - 1:0]          mask_insert_vol;

//
v_pkg::listsize_t                       stnxt_listsize_nxt;
v_pkg::listsize_t                       stnxt_listsize;
logic                                   stnxt_listsize_inc;
logic                                   stnxt_listsize_dec;
logic                                   stnxt_listsize_def;

logic [v_pkg::ENTRIES_N - 1:0]          cmp_eq;
logic [v_pkg::ENTRIES_N - 1:0]          cmp_gt;
logic [v_pkg::ENTRIES_N - 1:0]          cmp_lt;


logic [v_pkg::ENTRIES_N - 1:0]          stnxt_keys_do_upt;
v_pkg::key_t [v_pkg::ENTRIES_N - 1:0]   stnxt_keys_upt;
v_pkg::key_t [v_pkg::ENTRIES_N - 1:0]   stnxt_keys;

logic [v_pkg::ENTRIES_N - 1:0]          stnxt_volumes_do_upt;
v_pkg::volume_t [v_pkg::ENTRIES_N - 1:0]stnxt_volumes_upt;
v_pkg::volume_t [v_pkg::ENTRIES_N - 1:0]stnxt_volumes;

// Notify:
logic                                   notify_cleared_list;
logic                                   notify_did_add;
logic                                   notify_did_del;
logic                                   notify_did_rep_or_del;
logic                                   notify_vld;
v_pkg::key_t                            notify_key;
v_pkg::volume_t                         notify_volume;

// ========================================================================== //
//                                                                            //
//  Command decode                                                            //
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
//  Table match logic                                                         //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
// Construct one-hot vector denoting the position of matching keys in the
// current state (if any).
//
for (genvar i = 0; i < v_pkg::ENTRIES_N; i++) begin

assign match_sel [i] = i_stcur_vld_r [i] & (i_pipe_key_r == i_stcur_keys_r [i]);

end // for (genvar i = 0; i < v_pkg::ENTRIES_N; i++)

// Flag denoting that a matching key was found in the current state.
assign match_hit = (match_sel != '0);

assign match_full = (i_stcur_vld_r == '1);

mux #(.N(v_pkg::ENTRIES_N), .W($bits(v_pkg::volume_t))) u_max_match_volume (
  //
    .i_x                      (i_stcur_volumes_r)
  , .i_sel                    (match_sel)
  //
  , .o_y                      (match_volume)
);

// -------------------------------------------------------------------------- //
// Add:
//
//

// -------------------------------------------------------------------------- //
//
for (genvar i = 0; i < v_pkg::ENTRIES_N; i++) begin

  cmp #(.W($bits(v_pkg::key_t))) u_cmp (
  //
    .i_a                                  (i_stcur_keys_r [i])
  , .i_b                                  (i_pipe_key_r)
  //
  , .o_eq                                 (cmp_eq [i])
  , .o_gt                                 (cmp_gt [i])
  , .o_lt                                 (cmp_lt [i])
  );

end // for (genvar i = 0; i < v_pkg::ENTRIES_N; i++)

// ========================================================================== //
//                                                                            //
//  Add command                                                               //
//                                                                            //
// ========================================================================== //

// The list is ordered from MSB (left) to LSB (right) on decreasing key in the
// BID_TABLE case, or increasing key (!BID_TABLE, i.e. ask-table) case. Compare
// each valid {key, volume} 2-tuple by key and form a unary mask denoting those
// elements which are greater-than or equal, or lesser-than or equal to the
// current commands key. From this comparison, compute the appropriate location
// into which the new entry is to be inserted, perform the insertion and update
// the validity vector.
//
//  Below (where X indicates don't-care),
//
// Table:        A[N - 1]    A[N - 2]    A[N - 3]   X          X ...
//
// Valid:        1           1           1          0          0
//
// Comparison:   fn          fn          fn         fn         fn
//
// Relation:     1           1           0          x          x
//
// Insertion:    0           0           1          0          0
//
// Shift:        0           0           1          1          1
//
// Final:        A[N - 1]    A[N - 2]    CMD        A[N - 3]   0
//
// Valid (next)  1           1           1          1          0
//
assign add_mask_cmp =
   i_stcur_vld_r & (cmp_eq | (v_pkg::IS_BID_TABLE ? cmp_gt : cmp_lt));

// Use Leading-Zero Detect (LZD) to compute insertion position.
//
//                     +-- Insertion position
//                     |
//   0  0  0  0  0  0  1  0  0  0  0  0  0
//
lzd #(.W(v_pkg::ENTRIES_N), .DETECT_ZERO(1), .FROM_LSB(1)) u_lzd (
  //
    .i_x                                (add_mask_cmp)
  //
  , .o_y                                (add_mask_insert)
);

// Compute mask inclusive of the current selected insertion point and oriented
// towards the LSB to denote the set of positions/indices to be shifted-right
// for the insertion sort position.
//
//                     +-- Insertion position
//                     |
//   1  1  1  1  1  1  1  0  0  0  0  0  0
//
mask #(.W(v_pkg::ENTRIES_N), .TOWARDS_LSB(0), .INCLUSIVE(1)) u_mask_add (
  //
    .i_x                                (add_mask_insert)
  //
  , .o_y                                (add_mask_left)
);

assign add_vld_sel = i_stcur_vld_r & add_mask_left;

// Compute update to valid vector,
//
// Prior validity vector (vld):
//
//   0  0  0  1  1  1  1  1  1  1  1  1  1
//
// Entry to be inserted (pivot):
//
//   0  0  0  0  0  0  1  0  0  0  0  0  0
//
// Next validity vector (vld_nxt):
//
//   0  0  1  1  1  1  1  1  1  1  1  1  1
//
assign add_vld_shift = add_vld_sel << 1;

// Compose final valid vector as unmodified positions and new left-shifted
// positions.
//
assign add_vld = (i_stcur_vld_r | add_vld_shift | add_mask_insert);

// On an ADD, the list size is always incremented unless the context/prod_id is
// full (i.e. there are no free entries available). In this case, we simply
// saturate the count.
//
assign add_listsize_inc = (~match_full);

// ========================================================================== //
//                                                                            //
//  Delete command                                                            //
//                                                                            //
// ========================================================================== //

// NOTE: to avoid a costly prioritization operation, we implicitly assume that
// all keys within the current state vector are unique.

// From the comparison vector (which is constrained to be one-hot),
//
//               +-- Matching key
//               |
//   0  0  0  0  1  0  0  0  0  0  0  0  0
//
// compute the mask denoting the set of positions to be shifted left.
//
//   1  1  1  1  1  0  0  0  0  0  0  0  0
//

if (v_pkg::ALLOW_DUPLICATES) begin

// The table update logic cannot remove more than one entry per-cycle. In the
// case where multiple matching keys are allowed to co-exist in the same
// context, by convention we select the entry nearest the head.

pri #(.W(v_pkg::ENTRIES_N), .FROM_LSB(1)) u_pri_del (
  //
    .i_x                                (match_sel)
  //
  , .o_y                                (del_sel)
);

end else begin

// The table has not be configured support multiple keys. For correct operation
// therefore we require that the stimulus be appropriately constrained.

assign del_sel = match_sel;

end // else: !if(v_pkg::ALLOW_DUPLICATES)

//
//
mask #(.W(v_pkg::ENTRIES_N), .TOWARDS_LSB(0), .INCLUSIVE(1)) u_mask_del (
  //
    .i_x                                (del_sel)
  //
  , .o_y                                (del_mask_left)
);

// Compute update to valid vector,
//
// Entry to be deleted:
//
//   0  0  0  0  1  0  0  0  0  0  0  0  0
//
// Prior validity vector:
//
//   1  1  1  1  1  1  1  1  0  0  0  0  0
//
// Next validity vector:
//
//   1  1  1  1  1  1  1  0  0  0  0  0  0
//

// Positions rightward of currently nominated bit (inclusive) update to new
// post-delete positions.
assign del_vld_shift = (i_stcur_vld_r & del_mask_left) >> 1;

// Compose final valid vector as unmodified positions and new left-shifted
// positions.
assign del_vld =
      match_hit
    ? ((i_stcur_vld_r & ~del_mask_left) | del_vld_shift)
    : i_stcur_vld_r;

// On delete, the listsize is decremented whenever a hit takes place (i.e. a
// matching entry has been found in the context, which will now be removed).
//
assign del_listsize_dec = match_hit;

// TODO: flag indicating that a replacement took place.


// ========================================================================== //
//                                                                            //
//  Validity update                                                           //
//                                                                            //
// ========================================================================== //

// Compute update to the validatity vector. On a replacement, the overall state
// of the vector remains unchanged regardless of whether a replacement has taken
// place.
//
assign vld_nxt =
      ({v_pkg::ENTRIES_N{op_add}} & add_vld)
    | ({v_pkg::ENTRIES_N{op_del}} & del_vld)
    | ({v_pkg::ENTRIES_N{op_rep}} & i_stcur_vld_r);

// Compute final validity vector. On a CLEAR op., all bits are cleared
// regardless of prior state.
assign o_stnxt_vld =
    ({v_pkg::ENTRIES_N{~op_clr}} & vld_nxt);

// -------------------------------------------------------------------------- //
// Next Keys/Volumes:
//

// ========================================================================== //
//                                                                            //
//  Table update                                                              //
//                                                                            //
// ========================================================================== //

// Shift Right only on ADD operation
assign mask_right = ({v_pkg::ENTRIES_N{op_add}} & add_vld_shift);

// Shift Left only on LEFT operation.
assign mask_left = ({v_pkg::ENTRIES_N{op_del}} & del_mask_left);

// TODO: replace
assign mask_insert_key = ({v_pkg::ENTRIES_N{op_add}} & add_mask_insert);

assign mask_insert_vol =
      ({v_pkg::ENTRIES_N{op_add}} & add_mask_insert)
    | ({v_pkg::ENTRIES_N{op_rep}} & match_sel)
    ;

for (genvar i = 0; i < v_pkg::ENTRIES_N; i++) begin

  if (i == 0) begin
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

  end else if (i == v_pkg::ENTRIES_N - 1) begin // if (i == 0)
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

  end else begin // if (i == v_pkg::ENTRIES_N - 1)
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

  end // else: !if(i == v_pkg::ENTRIES_N - 1)


  // Select update or retain prior.
  assign stnxt_keys [i] =
      ({v_pkg::KEY_BITS{ stnxt_keys_do_upt [i]}} & stnxt_keys_upt [i]) |
      ({v_pkg::KEY_BITS{~stnxt_keys_do_upt [i]}} & i_stcur_keys_r [i]);

  // Select update or retain prior.
  assign stnxt_volumes [i] =
      ({v_pkg::VOLUME_BITS{ stnxt_volumes_do_upt [i]}} & stnxt_volumes_upt [i]) |
      ({v_pkg::VOLUME_BITS{~stnxt_volumes_do_upt [i]}} & i_stcur_volumes_r [i]);

end // for (genvar i = 0; i < v_pkg::ENTRIES_N; i++)


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

//
assign stnxt_listsize_nxt =
      ({v_pkg::LISTSIZE_W{stnxt_listsize_inc}} & (i_stcur_listsize_r + 'b1))
    | ({v_pkg::LISTSIZE_W{stnxt_listsize_dec}} & (i_stcur_listsize_r - 'b1))
    | ({v_pkg::LISTSIZE_W{stnxt_listsize_def}} &  i_stcur_listsize_r);

// Next listsize is conditionally reset to '0 on a clear operation regardless of
// prior computed values.
//
assign stnxt_listsize = ({v_pkg::LISTSIZE_W{~op_clr}} & stnxt_listsize_nxt);

// ========================================================================== //
//                                                                            //
//  Notify                                                                    //
//                                                                            //
// ========================================================================== //

// Clear operation and head entry was valid.
//
assign notify_cleared_list = op_clr & i_stcur_vld_r [0];

// Add operation. inserted into the head entry.
//
assign notify_did_add = op_add & add_mask_insert [0];

assign notify_did_del = op_del & match_sel [0];

// Notify on match replacement or deletion command to the head entry.
//
assign notify_did_rep_or_del = (op_rep | op_del) & match_sel [0];

assign notify_vld =
  (notify_cleared_list | notify_did_add | notify_did_rep_or_del);

// Notify key is current command key, indicated by the hit. On a Clear we ignore
// the key returned as we may have just cleared an empty context (where the head
// entry is invalid).
//
assign notify_key = i_pipe_key_r;

// Notify volume is the volume placed into the head position, or the value just
// removed.
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
