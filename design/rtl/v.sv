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

module v #(

// -------------------------------------------------------------------------- //
// Total number of contexts
  parameter int CONTEXT_N = 128

// -------------------------------------------------------------------------- //
// Number of entries per context
, parameter int ENTRIES_N = 4
) (

// -------------------------------------------------------------------------- //
// List Update Bus
  input                                           i_upd_vld
, input v_pkg::id_t                               i_upd_prod_id
, input v_pkg::cmd_t                              i_upd_cmd
, input v_pkg::key_t                              i_upd_key
, input v_pkg::size_t                             i_upd_size

// -------------------------------------------------------------------------- //
// List Query Bus
, input                                           i_lut_vld
, input v_pkg::id_t                               i_lut_prod_id
, input v_pkg::level_t                            i_lut_level
//
, output v_pkg::key_t                             o_lut_key
, output v_pkg::size_t                            o_lut_size
, output logic                                    o_lut_error
, output v_pkg::listsize_t                        o_lut_listsize

// -------------------------------------------------------------------------- //
// Notify Bus

, output logic                                    o_lv0_vld
, output v_pkg::id_t                              o_lv0_prod_id
, output v_pkg::key_t                             o_lv0_key
, output v_pkg::size_t                            o_lv0_size

// -------------------------------------------------------------------------- //
// Status
, output logic                                    o_busy_r

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

logic                                   init_wen_r;
v_pkg::addr_t                           init_waddr_r;
v_pkg::state_t                          init_wdata_r;

logic                                   state_wen_r;
v_pkg::addr_t                           state_waddr_r;
v_pkg::state_t                          state_wdata_r;

logic                                   wen;
v_pkg::addr_t                           waddr;
v_pkg::state_t                          wdata;

// ========================================================================== //
//                                                                            //
//  Combinatorial Logic                                                       //
//                                                                            //
// ========================================================================== //
  
// -------------------------------------------------------------------------- //
//
always_comb begin : table_update_PROC

  wen 	= o_busy_r ? init_wen_r : state_wen_r;
  waddr = o_busy_r ? init_waddr_r : state_waddr_r;
  wdata = o_busy_r ? init_wdata_r : state_wdata_r;

end // block: table_update_PROC

// ========================================================================== //
//                                                                            //
//  Instances                                                                 //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
//
v_update_pipe u_v_update_pipe (
  //
    .i_upd_vld                          (i_upd_vld)
  , .i_upd_prod_id                      (i_upd_prod_id)
  , .i_upd_cmd                          (i_upd_cmd)
  , .i_upd_key                          (i_upd_key)
  , .i_upd_size                         (i_upd_size)
  //
  , .i_state_rdata                      ()
  , .o_state_ren                        ()
  , .o_state_raddr                      ()
  //
  , .o_state_wen_r                      (state_wen_r)
  , .o_state_waddr_r                    (state_waddr_r)
  , .o_state_wdata_r                    (state_wdata_r)
  //
  , .clk                                (clk)
  , .rst                                (rst)
);

// -------------------------------------------------------------------------- //
//
sram1r1w #(.W($bits(v_pkg::state_t)), .N(v_pkg::CONTEXT_N)) u_sram1r1w_update (
  //
    .i_ren                              ()
  , .i_raddr                            ()
  , .o_rdata                            ()
  //
  , .i_wen                              (wen)
  , .i_waddr                            (waddr)
  , .i_wdata                            (wdata)
  //
  , .clk                                (clk)
);

// -------------------------------------------------------------------------- //
v_query_pipe u_v_query_pipe (
  //
    .clk                                (clk)
  , .rst                                (rst)
);

// -------------------------------------------------------------------------- //
//
sram1r1w #(.W($bits(v_pkg::state_t)), .N(v_pkg::CONTEXT_N)) u_sram1r1w_query (
  //
    .i_ren                              ()
  , .i_raddr                            ()
  , .o_rdata                            ()
  //
  , .i_wen                              (wen)
  , .i_waddr                            (waddr)
  , .i_wdata                            (wdata)
  //
  , .clk                                (clk)
);

// -------------------------------------------------------------------------- //
//
v_init #(.W($bits(v_pkg::state_t)), .N(v_pkg::CONTEXT_N)) u_init (
  //
    .o_init_wen_r                       (init_wen_r)
  , .o_init_waddr_r                     (init_waddr_r)
  , .o_init_wdata_r                     (init_wdata_r)
  //
  , .o_busy_r                           (o_busy_r)
  //
  , .clk                                (clk)
  , .rst                                (rst)
);

endmodule // v
