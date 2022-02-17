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
);

// ========================================================================== //
//                                                                            //
//  Wires                                                                     //
//                                                                            //
// ========================================================================== //

// Decoder:
logic                                   op_clear;
logic                                   op_add;
logic                                   op_del;
logic                                   op_replace;

// Match:
logic [v_pkg::ENTRIES_N - 1:0]          match_sel;
logic                                   match_hit;

// Add:
logic [v_pkg::ENTRIES_N - 1:0]          add_cmp_mask;
logic [v_pkg::ENTRIES_N - 1:0]          add_cmp_sel;
logic [v_pkg::ENTRIES_N - 1:0]          add_insert_pos;
logic [v_pkg::ENTRIES_N - 1:0]          add_sel;
logic [v_pkg::ENTRIES_N - 1:0]          add_mask;
logic [v_pkg::ENTRIES_N - 1:0]          add_vld;

// Delete:
logic [v_pkg::ENTRIES_N - 1:0]          del_mask;
logic [v_pkg::ENTRIES_N - 1:0]          del_vld;

// Validity:
logic [v_pkg::ENTRIES_N - 1:0]          vld_nxt;

//
logic [v_pkg::ENTRIES_N - 1:0]          right_mask;
logic [v_pkg::ENTRIES_N - 1:0]          left_mask;
logic [v_pkg::ENTRIES_N - 1:0]          insert_mask;
//
logic                                   cnt_do_add;
logic                                   cnt_do_del;

logic [v_pkg::ENTRIES_N - 1:0]          cmp_eq;
logic [v_pkg::ENTRIES_N - 1:0]          cmp_gt;
logic [v_pkg::ENTRIES_N - 1:0]          cmp_lt;

// ========================================================================== //
//                                                                            //
//  Combinatorial Logic                                                       //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
// Command Decoder
assign op_clear = (i_pipe_cmd_r == v_pkg::CMD_CLEAR);
assign op_add = (i_pipe_cmd_r == v_pkg::CMD_ADD);
assign op_del = (i_pipe_cmd_r == v_pkg::CMD_DELETE);
assign op_replace = (i_pipe_cmd_r == v_pkg::CMD_REPLACE);

// -------------------------------------------------------------------------- //
// Construct one-hot vector denoting the position of matching keys in the
// current state (if any).
//
for (genvar i = 0; i < v_pkg::ENTRIES_N; i++) begin

  assign match_sel [i] = (i_pipe_key_r == i_stcur_keys_r [i]);

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

// Form unary mask:
//
//                     +-- Insertion position
//                     |
//   1  1  1  1  1  1  0  0  0  0  0  0  0
//
assign add_cmp_mask = '0;


// Use Leading-Zero Detect (LZD) to compute insertion position.
//
//                     +-- Insertion position
//                     |
//   0  0  0  0  0  0  1  0  0  0  0  0  0
//
lzd #(.W(v_pkg::ENTRIES_N)) u_lzd (
  //
    .i_x                                ()
  //
  , .o_y                                (add_sel)
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
    .i_x                                (add_sel)
  //
  , .o_y                                (add_mask) // Result
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
vld #(.W(v_pkg::ENTRIES_N), .IS_INSERT('b1)) u_vld_add (
  //
    .i_vld                              (i_stcur_vld_r)
  , .i_pivot                            ()
  //
  , .o_vld_nxt                          (add_vld)
);

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
  , .o_y                                (del_mask)
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
vld #(.W(v_pkg::ENTRIES_N)) u_vld_del (
  //
    .i_vld                              (i_stcur_vld_r)
  , .i_pivot                            ()
  //
  , .o_vld_nxt                          (del_vld)
);


// -------------------------------------------------------------------------- //
// Replace
//
//


// -------------------------------------------------------------------------- //
// Validity vector
//
assign vld_nxt = ({v_pkg::ENTRIES_N{op_add}} & add_vld) |
		 ({v_pkg::ENTRIES_N{op_del}} & del_vld);

// Compute final validity vector. On OP_CLEAR, all bits are cleared regardless
// of prior state.
assign o_stnxt_vld = {v_pkg::ENTRIES_N{~op_clear}} & vld_nxt;

// -------------------------------------------------------------------------- //
// Next Keys/Volumes:
//

assign right_mask = '0;

assign left_mask = '0;

assign insert_mask = '0;

for (genvar i = 0; i < v_pkg::ENTRIES_N; i++) begin

  if (i == 0) begin

    // Right-most entry:
    
    assign o_stnxt_keys [i] =
      // Insertion,
      ({v_pkg::KEY_BITS{insert_mask [i]}} & i_pipe_key_r) |
      // Take left,
      ({v_pkg::KEY_BITS{left_mask [i]}} & i_stcur_keys_r [i + 1]);

    assign o_stnxt_volumes [i] =
      // Insertion
      ({v_pkg::VOLUME_BITS{insert_mask [i]}} & i_pipe_volume_r) |
      // Take left,
      ({v_pkg::VOLUME_BITS{left_mask [i]}} & i_stcur_volumes_r [i + 1]);

  end else if (i == v_pkg::ENTRIES_N - 1) begin // if (i == 0)

    // Left-most entry:
    
    assign o_stnxt_keys [i] =
      // Insertion,
      ({v_pkg::KEY_BITS{insert_mask [i]}} & i_pipe_key_r) |
      // Take right,
      ({v_pkg::KEY_BITS{right_mask [i]}} & i_stcur_keys_r [i - 1]);

    assign o_stnxt_volumes [i] =
      // Insertion
      ({v_pkg::VOLUME_BITS{insert_mask [i]}} & i_pipe_volume_r) |
      // Take right,
      ({v_pkg::VOLUME_BITS{right_mask [i]}} & i_stcur_volumes_r [i - 1]);

  end else begin // if (i == v_pkg::ENTRIES_N - 1)

    // Internal entry:
    
    assign o_stnxt_keys [i] =
      // Insertion,
      ({v_pkg::KEY_BITS{insert_mask [i]}} & i_pipe_key_r) |
      // Take left,
      ({v_pkg::KEY_BITS{left_mask [i]}} & i_stcur_keys_r [i + 1]) |
      // Take right,
      ({v_pkg::KEY_BITS{right_mask [i]}} & i_stcur_keys_r [i - 1]);

    assign o_stnxt_volumes [i] =
      // Insertion
      ({v_pkg::VOLUME_BITS{insert_mask [i]}} & i_pipe_volume_r) |
      // Take left,
      ({v_pkg::VOLUME_BITS{left_mask [i]}} & i_stcur_volumes_r [i + 1]) |
      // Take right,
      ({v_pkg::VOLUME_BITS{right_mask [i]}} & i_stcur_volumes_r [i - 1]);

  end // else: !if(i == v_pkg::ENTRIES_N - 1)

end // for (genvar i = 0; i < v_pkg::ENTRIES_N; i++)

// -------------------------------------------------------------------------- //
// Next Count:
//

assign cnt_do_add = op_add;

assign cnt_do_del = op_del;

assign o_stnxt_count = // List has been cleared, count becomes zero
                       op_clear ? '0 :
		       // Entry added to list, count is incremented
                       cnt_do_add ? (i_stcur_count_r + 'b1) :
		       // Entry removed from list, count is decremented
		       cnt_do_del ? (i_stcur_count_r - 'b1) :
		       // Otherwise, count remains unchanged.
		       i_stcur_count_r;

endmodule // v_pipe_update_exe
