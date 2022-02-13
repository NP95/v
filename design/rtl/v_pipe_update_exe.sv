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

// -------------------------------------------------------------------------- //
// State Next
, output logic [v_pkg::ENTRIES_N - 1:0]           o_stnxt_vld
, output v_pkg::key_t [v_pkg::ENTRIES_N - 1:0]    o_stnxt_keys
, output v_pkg::volume_t [v_pkg::ENTRIES_N - 1:0] o_stnxt_volumes
);

// ========================================================================== //
//                                                                            //
//  Wires                                                                     //
//                                                                            //
// ========================================================================== //

logic [v_pkg::ENTRIES_N - 1:0]          cmp_eq;
logic [v_pkg::ENTRIES_N - 1:0]          cmp_gt;
logic [v_pkg::ENTRIES_N - 1:0]          cmp_lt;

// ========================================================================== //
//                                                                            //
//  Instances                                                                 //
//                                                                            //
// ========================================================================== //

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

endmodule // v_pipe_update_exe
