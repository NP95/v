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

module v_pipe_query (

// -------------------------------------------------------------------------- //
// List Query Bus
  input                                           i_lut_vld
, input v_pkg::id_t                               i_lut_prod_id
, input v_pkg::level_t                            i_lut_level
//
, output v_pkg::key_t                             o_lut_key
, output v_pkg::size_t                            o_lut_size
, output logic                                    o_lut_error
, output v_pkg::listsize_t                        o_lut_listsize

// -------------------------------------------------------------------------- //
// State Interface
//
, input v_pkg::state_t                            i_state_rdata
//
, output logic                                    o_state_ren
, output v_pkg::addr_t                            o_state_raddr

// -------------------------------------------------------------------------- //
// Clk/Reset
, input                                           clk
, input                                           rst
);

// ========================================================================== //
//                                                                            //
//  Wires                                                                     //
//                                                                            //
// ========================================================================== //

// S1

// ========================================================================== //
//                                                                            //
//  Combinatorial Logic                                                       //
//                                                                            //
// ========================================================================== //
  
// -------------------------------------------------------------------------- //
//
always_comb begin : s0_PROC

end // block: s0_PROC
  
// -------------------------------------------------------------------------- //
//
always_comb begin : s1_PROC

end // block: s1_PROC

// ========================================================================== //
//                                                                            //
//  Flops                                                                     //
//                                                                            //
// ========================================================================== //
  
// ========================================================================== //
//                                                                            //
//  Outputs                                                                   //
//                                                                            //
// ========================================================================== //

endmodule // v_pipe_query
