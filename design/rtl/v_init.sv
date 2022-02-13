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

module v_init #(

// -------------------------------------------------------------------------- //
// Word count
  parameter int N

// -------------------------------------------------------------------------- //
// Word width
, parameter int W
) (

// -------------------------------------------------------------------------- //
// Memory Interfacex
  output logic                                    o_init_wen_r
, output logic [$clog2(N) - 1:0]                  o_init_waddr_r
, output logic [W - 1:0]                          o_init_wdata_r

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

localparam int BUSY_B = 1;
localparam int CNT_EN_B = 0;

typedef enum logic [1:0] {  FSM_STATE_IN_RESET = 2'b10,
                            FSM_STATE_INIT     = 2'b11,
                            FSM_STATE_DONE     = 2'b00
                          } fsm_state_t;

fsm_state_t                             fsm_state_r;
fsm_state_t                             fsm_state_w;

logic                                   fsm_last_word;
logic [$clog2(N) - 1:0]                 waddr_r;
logic [$clog2(N) - 1:0]                 waddr_w;
logic                                   waddr_en;

// ========================================================================== //
//                                                                            //
//  Combinatorial Logic                                                       //
//                                                                            //
// ========================================================================== //
  
// -------------------------------------------------------------------------- //
//
always_comb begin : fsm_PROC

  fsm_last_word = (waddr_r == N[$clog2(N) - 1:0]);

  // FSM state transition logic:
  case (fsm_state_r)
    FSM_STATE_IN_RESET: fsm_state_w = FSM_STATE_INIT;
    FSM_STATE_INIT:     fsm_state_w =
      fsm_last_word ? FSM_STATE_DONE : FSM_STATE_INIT;
    FSM_STATE_DONE:     fsm_state_w = fsm_state_r;
    default:            fsm_state_w = 'x;
  endcase // case (fsm_state_r)

  waddr_en = fsm_state_r [CNT_EN_B];
  waddr_w  = (fsm_state_r == FSM_STATE_IN_RESET) ? '0 : (waddr_r + 'b1);

end // block: fsm_PROC

// ========================================================================== //
//                                                                            //
//  Flops                                                                     //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
//
always_ff @(posedge clk)
  if (rst)
    fsm_state_r <= FSM_STATE_IN_RESET;
  else
    fsm_state_r <= fsm_state_w;

// -------------------------------------------------------------------------- //
//
always_ff @(posedge clk)
  if (waddr_en)
    waddr_r <= waddr_w;

// ========================================================================== //
//                                                                            //
//  Outputs                                                                   //
//                                                                            //
// ========================================================================== //

assign o_busy_r = fsm_state_r [BUSY_B];

assign o_init_wen_r = o_busy_r;
assign o_init_waddr_r = waddr_r;
assign o_init_wdata_r = '0;

endmodule // v_init
