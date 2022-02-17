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
, input v_pkg::level_t                            i_stcur_count_r

// -------------------------------------------------------------------------- //
// State Next
, output logic [v_pkg::ENTRIES_N - 1:0]           o_stnxt_vld
, output v_pkg::key_t [v_pkg::ENTRIES_N - 1:0]    o_stnxt_keys
, output v_pkg::volume_t [v_pkg::ENTRIES_N - 1:0] o_stnxt_volumes
//
, output v_pkg::level_t                           o_stnxt_count

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

// Add:
logic [v_pkg::ENTRIES_N - 1:0]          add_mask_cmp;
logic [v_pkg::ENTRIES_N - 1:0]          add_vld_shift;
logic [v_pkg::ENTRIES_N - 1:0]          add_vld;

logic [v_pkg::ENTRIES_N - 1:0]          add_mask_right;
logic [v_pkg::ENTRIES_N - 1:0]          add_mask_insert;

// Delete:
logic [v_pkg::ENTRIES_N - 1:0]          del_mask;
logic [v_pkg::ENTRIES_N - 1:0]          del_vld_shift;
logic [v_pkg::ENTRIES_N - 1:0]          del_vld;
logic [v_pkg::ENTRIES_N - 1:0]          del_mask_left;

// Replace:
logic [v_pkg::ENTRIES_N - 1:0]          rep_mask_insert;

// Validity:
logic [v_pkg::ENTRIES_N - 1:0]          vld_nxt;

//
logic [v_pkg::ENTRIES_N - 1:0]          mask_right;
logic [v_pkg::ENTRIES_N - 1:0]          mask_left;
logic [v_pkg::ENTRIES_N - 1:0]          mask_insert_key;
logic [v_pkg::ENTRIES_N - 1:0]          mask_insert_vol;
//
logic                                   cnt_do_add;
logic                                   cnt_do_del;

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
logic                                   notify_did_rep_or_del;
logic                                   notify_vld;

// ========================================================================== //
//                                                                            //
//  Combinatorial Logic                                                       //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
// Command Decoder
assign op_clr = (i_pipe_cmd_r == v_pkg::CMD_CLEAR);
assign op_add = (i_pipe_cmd_r == v_pkg::CMD_ADD);
assign op_del = (i_pipe_cmd_r == v_pkg::CMD_DELETE);
assign op_rep = (i_pipe_cmd_r == v_pkg::CMD_REPLACE);

// -------------------------------------------------------------------------- //
// Construct one-hot vector denoting the position of matching keys in the
// current state (if any).
//
for (genvar i = 0; i < v_pkg::ENTRIES_N; i++) begin

assign match_sel [i] = i_stcur_vld_r [i] & (i_pipe_key_r == i_stcur_keys_r [i]);

end // for (genvar i = 0; i < v_pkg::ENTRIES_N; i++)

// Flag denoting that a matching key was found in the current state.
assign match_hit = (match_sel != '0);

// -------------------------------------------------------------------------- //
// Add:
//
//

// -------------------------------------------------------------------------- //
//
for (genvar i = 0; i < v_pkg::ENTRIES_N; i++) begin

  cmp #(.W($bits(v_pkg::key_t))) u_cmp (
  //
    .i_a                                  (i_pipe_key_r)
  , .i_b                                  (i_stcur_keys_r [i])
  //
  , .o_eq                                 (cmp_eq [i])
  , .o_gt                                 (cmp_gt [i])
  , .o_lt                                 (cmp_lt [i])
  );

end

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
assign add_mask_cmp = i_stcur_vld_r & 
		      (cmp_eq | (v_pkg::IS_BID_TABLE ? cmp_gt : cmp_lt));

// Use Leading-Zero Detect (LZD) to compute insertion position.
//
//                     +-- Insertion position
//                     |
//   0  0  0  0  0  0  1  0  0  0  0  0  0
//
lzd #(.W(v_pkg::ENTRIES_N)) u_lzd (
  //
    .i_x                                (add_mask_cmp)
  //
  , .o_y                                (add_mask_insert)
);

// Compute mask inclusive of the current selected insertion
// point and oriented towards the LSB to denote the set of positions/indices to
// be shifted-right for the insertion sort position.
//
//                     +-- Insertion position
//                     |
//   0  0  0  0  0  0  1  1  1  1  1  1  1
//
mask #(.W(v_pkg::ENTRIES_N)) u_mask_add (
  //
    .i_x                                (add_mask_insert)
  //
  , .o_y                                (add_mask_right)
);

// Compute update to valid vector,
//
// Prior validity vector (vld):
//
//   1  1  1  1  1  1  1  1  1  0  0  0  0
//
// Entry to be inserted (pivot):
//
//   0  0  0  0  0  0  1  0  0  0  0  0  0
//
// Next validity vector (vld_nxt):
//
//   1  1  1  1  1  1  1  1  1  1  0  0  0
//
assign add_vld_shift = i_stcur_vld_r & (add_mask_right >> 1);

// Compose final valid vector as unmodified positions and new left-shifted
// positions.
//
assign add_vld = i_stcur_vld_r |
                 ({v_pkg::ENTRIES_N{match_hit}} & add_vld_shift);

// -------------------------------------------------------------------------- //
// Delete
//
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
//   0  0  0  0  1  1  1  1  1  1  1  1  1
//
mask #(.W(v_pkg::ENTRIES_N)) u_mask_del (
  //
    .i_x                                (match_sel)
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
assign del_vld_shift = i_stcur_vld_r & (del_mask_left << 1);

// Compose final valid vector as unmodified positions and new left-shifted
// positions.
assign del_vld = i_stcur_vld_r |
                 ({v_pkg::ENTRIES_N{match_hit}} & del_vld_shift);

// -------------------------------------------------------------------------- //
// Replace
//
//
assign rep_mask_insert = match_sel;

// TODO: flag indicating that a replacement took place.

// -------------------------------------------------------------------------- //
// Validity vector
//
// Compute update to the validatity vector. On a replacement, the overall state
// of the vector remains unchanged regardless of whether a replacement has taken
// place.
//
assign vld_nxt = ({v_pkg::ENTRIES_N{op_add}} & add_vld) |
                 ({v_pkg::ENTRIES_N{op_del}} & del_vld) |
                 ({v_pkg::ENTRIES_N{op_rep}} & i_stcur_vld_r);

// Compute final validity vector. On a CLEAR op., all bits are cleared
// regardless of prior state.
assign o_stnxt_vld = {v_pkg::ENTRIES_N{~op_clr}} & vld_nxt;

// -------------------------------------------------------------------------- //
// Next Keys/Volumes:
//

// Shift Right only on ADD operation
assign mask_right = ({v_pkg::ENTRIES_N{op_add}} & add_mask_right);

// Shift Left only on LEFT operation.
assign mask_left = ({v_pkg::ENTRIES_N{op_del}} & del_mask_left);

// TODO: replace
assign mask_insert_key = ({v_pkg::ENTRIES_N{op_add}} & add_mask_insert);

assign mask_insert_vol = ({v_pkg::ENTRIES_N{op_add}} & add_mask_insert) |
			 ({v_pkg::ENTRIES_N{op_rep}} & match_sel);

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

    assign stnxt_keys_do_upt [i] = (mask_insert_key [i] | mask_insert_vol [i]);
    
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


// -------------------------------------------------------------------------- //
// Next Count:
//

assign cnt_do_add = op_add;

assign cnt_do_del = op_del;

assign o_stnxt_count = // List has been cleared, count becomes zero
                       op_clr ? '0 :
                       // Entry added to list, count is incremented
                       cnt_do_add ? (i_stcur_count_r + 'b1) :
                       // Entry removed from list, count is decremented
                       cnt_do_del ? (i_stcur_count_r - 'b1) :
                       // Otherwise, count remains unchanged.
                       i_stcur_count_r;

// -------------------------------------------------------------------------- //
// Notify
//

// Clear operation and N'th entry was valid.
assign notify_cleared_list = op_clr & i_stcur_vld_r [v_pkg::ENTRIES_N - 1];

// Add operation. inserted into the N'th entry.
assign notify_did_add = op_add & add_mask_insert [v_pkg::ENTRIES_N - 1];

// Replace or Delete operations took place into the N'th entry.
assign notify_did_rep_or_del = (op_rep | op_del) &
			       match_sel [v_pkg::ENTRIES_N - 1];

assign notify_vld =
   (notify_cleared_list | notify_did_add | notify_did_rep_or_del);

// ========================================================================== //
//                                                                            //
//  Outputs                                                                   //
//                                                                            //
// ========================================================================== //

assign o_stnxt_keys = stnxt_keys;
assign o_stnxt_volumes = stnxt_volumes;

assign o_notify_vld =  notify_vld;
assign o_notify_key = i_stcur_keys_r [v_pkg::ENTRIES_N - 1];
assign o_notify_volume = i_stcur_volumes_r [v_pkg::ENTRIES_N - 1];

endmodule // v_pipe_update_exe
