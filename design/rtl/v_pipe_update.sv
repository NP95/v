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

// S0:
logic                                             s1_upd_en;
logic                                             s1_upd_vld_w;
//
v_pkg::id_t                                       s1_upd_prod_id_w;
v_pkg::cmd_t                                      s1_upd_cmd_w;
v_pkg::key_t                                      s1_upd_key_w;
v_pkg::size_t                                     s1_upd_size_w;

// S1:
//
logic                                             s1_upd_vld_r;
//
v_pkg::id_t                                       s1_upd_prod_id_r;
v_pkg::cmd_t                                      s1_upd_cmd_r;
v_pkg::key_t                                      s1_upd_key_r;
v_pkg::size_t                                     s1_upd_size_r;
//
logic                                             s1_state_ren;
v_pkg::addr_t                                     s1_state_raddr;
//
logic                                             s2_upd_en;
logic                                             s2_upd_vld_w;
//
v_pkg::id_t                                       s2_upd_prod_id_w;
v_pkg::cmd_t                                      s2_upd_cmd_w;
v_pkg::key_t                                      s2_upd_key_w;
v_pkg::size_t                                     s2_upd_size_w;
//
logic                                             s2_upd_wrbk_vld_w;
v_pkg::state_t                                    s2_upd_wrbk_w;

// S2:
//
logic                                             s2_upd_vld_r;
//
logic                                             s2_upd_wrbk_vld_r;
v_pkg::state_t                                    s2_upd_wrbk_r;
//
v_pkg::id_t                                       s2_upd_prod_id_r;
v_pkg::cmd_t                                      s2_upd_cmd_r;
v_pkg::key_t                                      s2_upd_key_r;
v_pkg::size_t                                     s2_upd_size_r;
//
logic [1:0]                                       s2_upd_state_fwd;
logic [1:0]                                       s2_upd_state_sel;
logic                                             s2_upd_state_sel_early;
v_pkg::state_t                                    s2_upd_state_early;
//
logic                                             s3_upd_en;
logic                                             s3_upd_vld_w;
//
v_pkg::state_t                                    s3_upd_state_w;
v_pkg::id_t                                       s3_upd_prod_id_w;
v_pkg::cmd_t                                      s3_upd_cmd_w;
v_pkg::key_t                                      s3_upd_key_w;
v_pkg::size_t                                     s3_upd_size_w;

// S3:
//
logic                                             s3_upd_vld_r;
//
v_pkg::state_t                                    s3_upd_state_r;
v_pkg::id_t                                       s3_upd_prod_id_r;
v_pkg::cmd_t                                      s3_upd_cmd_r;
v_pkg::key_t                                      s3_upd_key_r;
v_pkg::size_t                                     s3_upd_size_r;
//
logic [v_pkg::ENTRIES_N - 1:0]                    s3_exe_stcur_vld_r;
v_pkg::key_t [v_pkg::ENTRIES_N - 1:0]             s3_exe_stcur_keys_r;
v_pkg::volume_t [v_pkg::ENTRIES_N - 1:0]          s3_exe_stcur_volumes_r;
v_pkg::level_t                                    s3_exe_stcur_count_r;
//
logic [v_pkg::ENTRIES_N - 1:0]                    s3_exe_stnxt_vld;
v_pkg::key_t [v_pkg::ENTRIES_N - 1:0]             s3_exe_stnxt_keys;
v_pkg::volume_t [v_pkg::ENTRIES_N - 1:0]          s3_exe_stnxt_volumes;
v_pkg::level_t                                    s3_exe_stnxt_count;
//
logic                                             s4_upd_en;
logic                                             s4_upd_vld_w;
v_pkg::id_t                                       s4_upd_prod_id_w;
v_pkg::state_t                                    s4_upd_state_w;

// S4:
//
//
logic                                             s4_upd_vld_r;
v_pkg::id_t                                       s4_upd_prod_id_r;
v_pkg::state_t                                    s4_upd_state_r;
logic                                             s4_upd_notify_r;


// EXE
logic                                             exe_notify_vld;
v_pkg::key_t                                      exe_notify_key;
v_pkg::volume_t                                   exe_notify_volume;


// Notify bus:
logic                                             lv0_vld_w;
logic                                             lv0_vld_r;
logic                                             lv0_en;
v_pkg::id_t                                       lv0_prod_id_w;
v_pkg::id_t                                       lv0_prod_id_r;
v_pkg::key_t                                      lv0_key_w;
v_pkg::key_t                                      lv0_key_r;
v_pkg::volume_t                                   lv0_size_w;
v_pkg::volume_t                                   lv0_size_r;

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

  // Writeback collision: Forward write-back state to S2 read port on collision;
  // also kill lookup into state table to prevent possible data corruption where
  // memory does not support forwarding.
  s2_upd_wrbk_vld_w = s4_upd_vld_r & (s4_upd_prod_id_w == s1_upd_prod_id_r);
  s2_upd_wrbk_w     = s4_upd_state_r;

  //
  s1_state_ren 	    = s1_upd_vld_r & (~s2_upd_wrbk_vld_w);
  s1_state_raddr    = s1_upd_prod_id_r;

  // Pipeline controls:
  //
  s2_upd_en 	    = s1_upd_vld_r;

  s2_upd_prod_id_w  = s1_upd_prod_id_r;
  s2_upd_cmd_w 	    = s1_upd_cmd_r;
  s2_upd_key_w 	    = s1_upd_key_r;
  s2_upd_size_w     = s1_upd_size_r;

end // block: s1_PROC

// -------------------------------------------------------------------------- //
// S2 Stage: State arrival and EXE-forwarding stage.
//

// Attempt hit on current writeback.
assign s2_upd_state_fwd [1] = s4_upd_vld_r & (s4_upd_prod_id_r == s2_upd_prod_id_r);
// Otherwise, attempt hit on prior writeback
assign s2_upd_state_fwd [0] = s2_upd_wrbk_vld_r;

assign s2_upd_state_sel_early = (|s2_upd_state_fwd);

// Early state forwarding; the state that is expected to arrival
// relatively early into the current cycle.
assign s2_upd_state_early =
   {v_pkg::STATE_BITS{s2_upd_state_fwd[1]}} & s4_upd_state_r |
   {v_pkg::STATE_BITS{s2_upd_state_fwd[0]}} & s2_upd_wrbk_r;

// Final State forwarding injecting those signals (from RAM) that is expected to
// arrive relatively late into the cycle. To this so that such state is injected
// late into the overall combinatorial logic cone.
assign s3_upd_state_w =
   {v_pkg::STATE_BITS{ s2_upd_state_sel_early}} & s2_upd_state_early |
   {v_pkg::STATE_BITS{~s2_upd_state_sel_early}} & i_state_rdata;

assign s3_upd_prod_id_w = s2_upd_prod_id_r;
assign s3_upd_cmd_w = s2_upd_cmd_r;
assign s3_upd_key_w = s2_upd_key_r;
assign s3_upd_size_w = s2_upd_size_r;

// -------------------------------------------------------------------------- //
// S3 Stage: Execute Stage
//
assign s3_exe_stcur_vld_r = s3_upd_state_r.vld;
assign s3_exe_stcur_keys_r = s3_upd_state_r.key;
assign s3_exe_stcur_volumes_r = s3_upd_state_r.volume;

//assign s3_exe_stcur_count_r = s3_upd_state_r.listsize;

assign s4_upd_en = s3_upd_vld_r;

// Ucode:
//assign s4_upd_prod_id_w = s3_upd_prod_id_r;
//assign s4_upd_key_w = s3_upd_key_r;
//assign s4_upd_size_w = s3_upd_size_r;
//assign s4_upd_notify_w = 'b0;

// Notify bus:
assign lv0_vld_w = s3_upd_vld_r & exe_notify_vld;
assign lv0_en = lv0_vld_w;
assign lv0_prod_id_w = s3_upd_prod_id_r;
assign lv0_key_w = exe_notify_key;
assign lv0_size_w = exe_notify_volume;

// -------------------------------------------------------------------------- //
// S4 Stage: Writeback Stage
//

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
    // Pipeline ucode:
    s1_upd_prod_id_r  <= s1_upd_prod_id_w;
    s1_upd_cmd_r      <= s1_upd_cmd_w;
    s1_upd_key_r      <= s1_upd_key_w;
    s1_upd_size_r     <= s1_upd_size_w;
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
    // Writeback collision ucode:
    s2_upd_wrbk_vld_r <= s2_upd_wrbk_vld_w;
    s2_upd_wrbk_r     <= s2_upd_wrbk_w;

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
  if (s3_upd_en) begin
    s3_upd_prod_id_r <= s3_upd_prod_id_w;
    s3_upd_cmd_r     <= s3_upd_cmd_w;
    s3_upd_key_r     <= s3_upd_key_w;
    s3_upd_size_r    <= s3_upd_size_w;
  end

// -------------------------------------------------------------------------- //
//
always_ff @(posedge clk)
  if (rst)
    s4_upd_vld_r <= 'b0;
  else
    s4_upd_vld_r <= s4_upd_vld_w;

// -------------------------------------------------------------------------- //
//
/*
always_ff @(posedge clk)
  if (s4_upd_en) begin
    s4_upd_prod_id_r <= s4_upd_prod_id_w;
    s4_upd_key_r     <= s4_upd_key_w;
    s4_upd_size_r    <= s4_upd_size_w;

    s4_upd_notify_r  <= s4_upd_notify_w;
  end
*/
// -------------------------------------------------------------------------- //
//
always_ff @(posedge clk)
  if (rst)
    lv0_vld_r <= 'b0;
  else
    lv0_vld_r <= lv0_vld_w;

// -------------------------------------------------------------------------- //
//
always_ff @(posedge clk)
  if (lv0_en) begin
    lv0_prod_id_r <= lv0_prod_id_w;
    lv0_key_r 	  <= lv0_key_w;
    lv0_size_r 	  <= lv0_size_w;
  end

// ========================================================================== //
//                                                                            //
//  Instances                                                                 //
//                                                                            //
// ========================================================================== //
  
// -------------------------------------------------------------------------- //
//
pri #(.W(2)) u_s3_forwarding_pri (
  //
    .i_x                                (s2_upd_state_fwd)
  //
  , .o_y                                (s2_upd_state_sel)
);

// -------------------------------------------------------------------------- //
//
v_pipe_update_exe u_v_pipe_update_exe (
  //
    .i_pipe_cmd_r                       (s3_upd_cmd_r)
  , .i_pipe_key_r                       (s3_upd_key_r)
  , .i_pipe_volume_r                    (s3_upd_size_r)
  //
  , .i_stcur_vld_r                      (s3_exe_stcur_vld_r)
  , .i_stcur_keys_r                     (s3_exe_stcur_keys_r)
  , .i_stcur_volumes_r                  (s3_exe_stcur_volumes_r)
  , .i_stcur_count_r                    (s3_exe_stcur_count_r)
  //
  , .o_stnxt_vld                        (s3_exe_stnxt_vld)
  , .o_stnxt_keys                       (s3_exe_stnxt_keys)
  , .o_stnxt_volumes                    (s3_exe_stnxt_volumes)
  , .o_stnxt_count                      (s3_exe_stnxt_count)
  //
  , .o_notify_vld                       (exe_notify_vld)
  , .o_notify_key                       (exe_notify_key)
  , .o_notify_volume                    (exe_notify_volume)
);

// ========================================================================== //
//                                                                            //
//  Outputs                                                                   //
//                                                                            //
// ========================================================================== //

assign o_state_ren = s1_state_ren;
assign o_state_raddr = s1_state_raddr;

// State update
assign o_state_wen_r = s4_upd_vld_r;
assign o_state_waddr_r = s4_upd_prod_id_r;
assign o_state_wdata_r = s4_upd_state_r;

// Notify interface
assign o_lv0_vld_r = lv0_vld_r;
assign o_lv0_prod_id_r = lv0_prod_id_r;
assign o_lv0_key_r = lv0_key_r;
assign o_lv0_size_r = lv0_size_r;

endmodule // v_pipe_update
