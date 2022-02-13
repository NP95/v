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
, output v_pkg::volume_t                          o_lut_size
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
// Update Pipeline Interface
//
, input                                           i_s1_upd_vld_r
, input v_pkg::id_t                               i_s1_upd_prod_id_r
//
, input                                           i_s2_upd_vld_r
, input v_pkg::id_t                               i_s2_upd_prod_id_r
//
, input                                           i_s3_upd_vld_r
, input v_pkg::id_t                               i_s3_upd_prod_id_r
//
, input                                           i_s4_upd_vld_r
, input v_pkg::id_t                               i_s4_upd_prod_id_r

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

// S0
logic                                   s0_state_ren;
v_pkg::id_t                             s0_state_raddr;

// S1
logic                                   s1_lut_en;
v_pkg::id_t                             s1_lut_prod_id_w;
v_pkg::id_t                             s1_lut_prod_id_r;
v_pkg::level_t                          s1_lut_level_w;
v_pkg::level_t                          s1_lut_level_r;
logic                                   s1_lut_error_w;
logic                                   s1_lut_error_r;

v_pkg::listsize_t                       s1_lut_listsize;
logic [v_pkg::ENTRIES_N - 1:0]          s1_lut_level_dec;
v_pkg::key_t                            s1_lut_key;
v_pkg::volume_t                         s1_lut_volume;
logic                                   s1_lut_error;

// ========================================================================== //
//                                                                            //
//  Combinatorial Logic                                                       //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
//
always_comb begin : s0_ucode_PROC

  // State table lookup
  s0_state_ren     = i_lut_vld;
  s0_state_raddr   = i_lut_prod_id;

  s1_lut_en        = i_lut_vld;
  s1_lut_prod_id_w = i_lut_prod_id;
  s1_lut_level_w   = i_lut_level;

  // Flag indicating "list busy or not valid entry".
  //
  // We consider the "list busy" whenever there is a in-flight operation to the
  // currently addressed ID in the update pipeline. Whenever an update is
  // in-flight, we simply error-out. Otherwise, it would be possible to use some
  // more sophisticated forwarding, but this is probably overkill in this
  // context and is not required by the specification.
  //
  // We consider a "not valid entry" whenever the queried entry is empty. In
  // this case, although the size returned is valid (zero), the key/volume tuple
  // is not. This calculation is computed in S1 once data has returned from the
  // state table.
  //
  s1_lut_error_w   = (i_s1_upd_vld_r & (i_s1_upd_prod_id_r == i_lut_prod_id)) |
                     (i_s2_upd_vld_r & (i_s2_upd_prod_id_r == i_lut_prod_id)) |
                     (i_s3_upd_vld_r & (i_s3_upd_prod_id_r == i_lut_prod_id)) |
                     (i_s4_upd_vld_r & (i_s4_upd_prod_id_r == i_lut_prod_id));

end // block: s0_ucode_PROC

// -------------------------------------------------------------------------- //
//
always_comb begin : s1_ucode_PROC

  s1_lut_listsize = i_state_rdata.listsize;

  // Error on prior hazard in update pipeline, or whenever table size is zero.
  s1_lut_error 	  = s1_lut_error_r | (i_state_rdata.listsize == '0);

end // block: s1_ucode_PROC

// ========================================================================== //
//                                                                            //
//  Flops                                                                     //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
//
always_ff @(posedge clk)
  if (s1_lut_en) begin : s1_ucode_reg_PROC
    s1_lut_prod_id_r <= s1_lut_prod_id_w;
    s1_lut_level_r   <= s1_lut_level_w;
    s1_lut_error_r   <= s1_lut_error_w;
  end // block: s1_ucode_reg_PROC

// ========================================================================== //
//                                                                            //
//  Instances                                                                 //
//                                                                            //
// ========================================================================== //

dec #(.N(v_pkg::ENTRIES_N)) u_s1_id_dec (
//
  .i_x                                  (s1_lut_level_r)
//
, .o_y                                  (s1_lut_level_dec)
);

mux #(.N(v_pkg::ENTRIES_N), .W($bits(v_pkg::key_t))) u_s1_key_mux (
//
  .i_x                                  (i_state_rdata.key)
, .i_sel                                (s1_lut_level_dec)
//
, .o_y                                  (s1_lut_key)
);

mux #(.N(v_pkg::ENTRIES_N), .W($bits(v_pkg::volume_t))) u_s1_volume_mux (
//
  .i_x                                  (i_state_rdata.volume)
, .i_sel                                (s1_lut_level_dec)
//
, .o_y                                  (s1_lut_volume)
);

// ========================================================================== //
//                                                                            //
//  Outputs                                                                   //
//                                                                            //
// ========================================================================== //

assign o_lut_key = s1_lut_key;
assign o_lut_size = s1_lut_volume;
assign o_lut_error = s1_lut_error;
assign o_lut_listsize = s1_lut_listsize;

assign o_state_ren = s0_state_ren;
assign o_state_raddr = s0_state_raddr;

endmodule // v_pipe_query
