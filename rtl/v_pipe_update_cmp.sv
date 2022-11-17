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

module v_pipe_update_cmp (
// -------------------------------------------------------------------------- //
// Command Interface
  input v_pkg::key_t                                i_pipe_key_r

// -------------------------------------------------------------------------- //
// State Current
, input [cfg_pkg::ENTRIES_N - 1:0]                  i_stcur_vld_r
, input v_pkg::key_t [cfg_pkg::ENTRIES_N - 1:0]     i_stcur_keys_r

// -------------------------------------------------------------------------- //
// Command Interface
, output logic                                      o_match_hit
, output logic                                      o_match_full
, output logic [cfg_pkg::ENTRIES_N - 1:0]           o_match_sel
, output logic [cfg_pkg::ENTRIES_N - 1:0]           o_mask_cmp
);

// ========================================================================== //
//                                                                            //
//  Wires                                                                     //
//                                                                            //
// ========================================================================== //

logic [cfg_pkg::ENTRIES_N - 1:0]           mask_cmp;
logic [cfg_pkg::ENTRIES_N - 1:0]           match_sel;
logic                                      match_hit;
logic                                      match_full;

logic [cfg_pkg::ENTRIES_N - 1:0]           cmp_eq;
logic [cfg_pkg::ENTRIES_N - 1:0]           cmp_gt;
logic [cfg_pkg::ENTRIES_N - 1:0]           cmp_lt;

// ========================================================================== //
//                                                                            //
//  Table match logic                                                         //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
// Construct one-hot vector denoting the position of matching keys in the
// current state (if any).
//
for (genvar i = 0; i < cfg_pkg::ENTRIES_N; i++) begin

assign match_sel [i] = i_stcur_vld_r [i] & (i_pipe_key_r == i_stcur_keys_r [i]);

end // for (genvar i = 0; i < cfg_pkg::ENTRIES_N; i++)

// -------------------------------------------------------------------------- //
// Flag denoting that a matching key was found in the current state.
//
assign match_hit = (match_sel != '0);

// -------------------------------------------------------------------------- //
// Flag indicating that all Entries in the  context are full
//
assign match_full = (i_stcur_vld_r == '1);

// ========================================================================== //
//                                                                            //
//  Comparison                                                                //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
// Compare table keys against current command key.a
//
for (genvar i = 0; i < cfg_pkg::ENTRIES_N; i++) begin : cmp_GEN

  cmp #(.W(v_pkg::KEY_BITS)) u_cmp (
  //
    .i_a                                  (i_stcur_keys_r [i])
  , .i_b                                  (i_pipe_key_r)
  //
  , .o_eq                                 (cmp_eq [i])
  , .o_gt                                 (cmp_gt [i])
  , .o_lt                                 (cmp_lt [i])
  );

end : cmp_GEN

// -------------------------------------------------------------------------- //
// Form a unary mask denoting the valid elements on the context that are
// greater-than (BID-TABLE)/less-than (ASK-Table) or equal-to the current
// command (where X is ENTRIES_N - 1).
//
//  Index   X                                      0
//
//  Valid   0  0  0  0  0  0  0  1  1  1  1  1  1  1
//
//  Compare 0  0  0  0  0  0  0  0  0  0  1  1  1  1
//
assign mask_cmp =
   i_stcur_vld_r & (cmp_eq | (cfg_pkg::IS_BID_TABLE ? cmp_gt : cmp_lt));

// ========================================================================== //
//                                                                            //
//  Outputs                                                                   //
//                                                                            //
// ========================================================================== //

assign o_match_hit = match_hit;
assign o_match_full = match_full;
assign o_mask_cmp = mask_cmp;
assign o_match_sel = match_sel;

endmodule // v_pipe_update_cmp
