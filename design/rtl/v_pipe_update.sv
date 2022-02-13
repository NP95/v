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

module v_pipe_update (

// -------------------------------------------------------------------------- //
// List Update Bus
  input                                           i_upd_vld
, input v_pkg::id_t                               i_upd_prod_id
, input v_pkg::cmd_t                              i_upd_cmd
, input v_pkg::key_t                              i_upd_key
, input v_pkg::size_t                             i_upd_size

// -------------------------------------------------------------------------- //
// State Interface
//
, input v_pkg::state_t                            i_state_rdata
//
, output logic                                    o_state_ren
, output v_pkg::addr_t                            o_state_raddr
//
, output logic                                    o_state_wen_r
, output v_pkg::addr_t                            o_state_waddr_r
, output v_pkg::state_t                           o_state_wdata_r

// -------------------------------------------------------------------------- //
// Notify Bus
, output logic                                    o_lv0_vld_r
, output v_pkg::id_t                              o_lv0_prod_id_r
, output v_pkg::key_t                             o_lv0_key_r
, output v_pkg::size_t                            o_lv0_size_r

// -------------------------------------------------------------------------- //
// Update Pipeline Interface
//
, output                                          o_s1_upd_vld_r
, output v_pkg::id_t                              o_s1_upd_prod_id_r
//
, output                                          o_s2_upd_vld_r
, output v_pkg::id_t                              o_s2_upd_prod_id_r
//
, output                                          o_s3_upd_vld_r
, output v_pkg::id_t                              o_s3_upd_prod_id_r
//
, output                                          o_s4_upd_vld_r
, output v_pkg::id_t                              o_s4_upd_prod_id_r

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

// S1:
//
logic                                             s1_upd_vld_r;
logic                                             s1_upd_vld_w;
//
logic                                             s1_upd_en;
//
v_pkg::id_t                                       s1_upd_prod_id_w;
v_pkg::cmd_t                                      s1_upd_cmd_w;
v_pkg::key_t                                      s1_upd_key_w;
v_pkg::size_t                                     s1_upd_size_w;
//
v_pkg::id_t                                       s1_upd_prod_id_r;
v_pkg::cmd_t                                      s1_upd_cmd_r;
v_pkg::key_t                                      s1_upd_key_r;
v_pkg::size_t                                     s1_upd_size_r;
//
logic                                             s1_state_ren;
v_pkg::addr_t                                     s1_state_raddr;

// S2:
//
logic                                             s2_upd_vld_r;
logic                                             s2_upd_vld_w;
//
logic                                             s2_upd_en;
//
v_pkg::id_t                                       s2_upd_prod_id_w;
v_pkg::cmd_t                                      s2_upd_cmd_w;
v_pkg::key_t                                      s2_upd_key_w;
v_pkg::size_t                                     s2_upd_size_w;
//
v_pkg::id_t                                       s2_upd_prod_id_r;
v_pkg::cmd_t                                      s2_upd_cmd_r;
v_pkg::key_t                                      s2_upd_key_r;
v_pkg::size_t                                     s2_upd_size_r;

// S3:
//
logic                                             s3_upd_vld_r;
logic                                             s3_upd_vld_w;
//
logic                                             s3_upd_en;

// ========================================================================== //
//                                                                            //
//  Combinatorial Logic                                                       //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
//
always_comb begin : s0_PROC

  // Pipeline controls:
  //
  s1_upd_en 	   = i_upd_vld;

  s1_upd_prod_id_w = i_upd_prod_id;
  s1_upd_cmd_w 	   = i_upd_cmd;
  s1_upd_key_w 	   = i_upd_key;
  s1_upd_size_w    = i_upd_size;

end // block: s0_PROC

// -------------------------------------------------------------------------- //
//
always_comb begin : s1_PROC

  //
  s1_state_ren 	   = s1_upd_vld_r;
  s1_state_raddr   = '0;

  // Pipeline controls:
  //
  s2_upd_en 	   = s1_upd_vld_r;

  s2_upd_prod_id_w = s1_upd_prod_id_r;
  s2_upd_cmd_w 	   = s1_upd_cmd_r;
  s2_upd_key_w 	   = s1_upd_key_r;
  s2_upd_size_w    = s1_upd_size_r;

end // block: s1_PROC

// -------------------------------------------------------------------------- //
//
always_comb begin : s2_PROC

end // block: s2_PROC

// -------------------------------------------------------------------------- //
//
always_comb begin : s3_PROC

end // block: s3_PROC
  
// ========================================================================== //
//                                                                            //
//  Flops                                                                     //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
//
always_ff @(posedge clk)
  if (rst)
    s1_upd_vld_r <= 'b0;
  else
    s1_upd_vld_r <= s1_upd_vld_w;

// -------------------------------------------------------------------------- //
//
always_ff @(posedge clk)
  if (s1_upd_en) begin
    s1_upd_prod_id_r <= s1_upd_prod_id_w;
    s1_upd_cmd_r     <= s1_upd_cmd_w;
    s1_upd_key_r     <= s1_upd_key_w;
    s1_upd_size_r    <= s1_upd_size_w;
  end

// -------------------------------------------------------------------------- //
//
always_ff @(posedge clk)
  if (rst)
    s2_upd_vld_r <= 'b0;
  else
    s2_upd_vld_r <= s2_upd_vld_w;

// -------------------------------------------------------------------------- //
//
always_ff @(posedge clk)
  if (s2_upd_en) begin
    s2_upd_prod_id_r <= s2_upd_prod_id_w;
    s2_upd_cmd_r     <= s2_upd_cmd_w;
    s2_upd_key_r     <= s2_upd_key_w;
    s2_upd_size_r    <= s2_upd_size_w;
  end

// -------------------------------------------------------------------------- //
//
always_ff @(posedge clk)
  if (rst)
    s3_upd_vld_r <= 'b0;
  else
    s3_upd_vld_r <= s3_upd_vld_w;

// -------------------------------------------------------------------------- //
//
always_ff @(posedge clk)
  if (s3_upd_en)
    ;

// ========================================================================== //
//                                                                            //
//  Instances                                                                 //
//                                                                            //
// ========================================================================== //
  
// -------------------------------------------------------------------------- //
//
v_pipe_update_exe u_v_pipe_update_exe (
  //
    .i_pipe_cmd_r                       ()
  , .i_pipe_key_r                       ()
  , .i_pipe_volume_r                    ()
  //
  , .i_stcur_vld_r                      ()
  , .i_stcur_keys_r                     ()
  , .i_stcur_volumes_r                  ()
  //
  , .o_stnxt_vld                        ()
  , .o_stnxt_keys                       ()
  , .o_stnxt_volumes                    ()
);

// ========================================================================== //
//                                                                            //
//  Outputs                                                                   //
//                                                                            //
// ========================================================================== //

assign o_state_ren = s1_state_ren;
assign o_state_raddr = s1_state_raddr;

// State update
assign o_state_wen_r = '0;
assign o_state_waddr_r = '0;
assign o_state_wdata_r = '0;

// Notify interface
assign o_lv0_vld_r = '0;
assign o_lv0_prod_id_r = '0;
assign o_lv0_key_r = '0;
assign o_lv0_size_r = '0;

endmodule // v_pipe_update
