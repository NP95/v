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

// Insert operation (IS_INSERT):
//
// Current validity:
//
//   1  1  1  1  1  1  1  1  1  0  0  0  0
//
// Pivot item (position for insertion):
//
//   0  0  0  0  0  0  1  0  0  0  0  0  0
//
// Final result:
//
//   1  1  1  1  1  1  1  1  1  1  0  0  0
//
//
// Delete operation (!IS_INSERT):
//
// Current validity:
//
//   1  1  1  1  1  1  1  1  1  0  0  0  0
//
// Pivot item (position for insertion):
//
//   0  0  0  0  0  0  1  0  0  0  0  0  0
//
// Final result:
//
//   1  1  1  1  1  1  1  1  0  0  0  0  0
//

module vld #(
  // Width of vector
  parameter int W

, parameter bit IS_INSERT = 'b0
) (
// -------------------------------------------------------------------------- //
//
  input [W - 1:0]                                 i_vld
, input [W - 1:0]                                 i_pivot

// -------------------------------------------------------------------------- //
//
, output logic [W - 1:0]                          o_vld_nxt
);

// ========================================================================== //
//                                                                            //
//  Wires                                                                     //
//                                                                            //
// ========================================================================== //

logic [W - 1:0]                         vld_nxt;
logic [W - 1:0]                         unary_mask; // BETTER NAME
logic [W - 1:0]                         bits_to_shift; // BETTER NAME
logic [W - 1:0]                         shifted; // BETTER NAME

// ========================================================================== //
//                                                                            //
//  Combinatorial Logic                                                       //
//                                                                            //
// ========================================================================== //

mask #(.W(W), .TOWARDS_LSB('b1)) u_mask (
  //
    .i_x                              (i_pivot)
  //
  , .o_y                              (unary_mask)
);

assign bits_to_shift = i_vld & unary_mask;

assign shifted = IS_INSERT ?
                 { 1'b0, bits_to_shift [W - 1:1] } :
		   { bits_to_shift [W - 2:0], 1'b0 } ;

assign vld_nxt = (i_vld | i_pivot | shifted);

// ========================================================================== //
//                                                                            //
//  Outputs                                                                   //
//                                                                            //
// ========================================================================== //

assign o_vld_nxt = vld_nxt;

endmodule // vld

