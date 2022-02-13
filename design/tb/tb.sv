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
// ========================================================================== //

`include "common_defs.vh"

`include "v_pkg.vh"

module tb #(

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
//  UUT                                                                       //
//                                                                            //
// ========================================================================== //

v #(.CONTEXT_N, .ENTRIES_N) u_v (
  //
    .i_upd_vld                          (i_upd_vld)
  , .i_upd_prod_id                      (i_upd_prod_id)
  , .i_upd_cmd                          (i_upd_cmd)
  , .i_upd_key                          (i_upd_key)
  , .i_upd_size                         (i_upd_size)
  //
  , .i_lut_vld                          (i_lut_vld)
  , .i_lut_prod_id                      (i_lut_prod_id)
  , .i_lut_level                        (i_lut_level)
  , .o_lut_key                          (o_lut_key)
  , .o_lut_size                         (o_lut_size)
  , .o_lut_error                        (o_lut_error)
  , .o_lut_listsize                     (o_lut_listsize)
  //
  , .o_lv0_vld                          (o_lv0_vld)
  , .o_lv0_prod_id                      (o_lv0_prod_id)
  , .o_lv0_key                          (o_lv0_key)
  , .o_lv0_size                         (o_lv0_size)
  //
  , .o_busy_r                           (o_busy_r)
  //
  , .clk                                (clk)
  , .rst                                (rst)
);
  
endmodule // v
