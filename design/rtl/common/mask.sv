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

// Compute a unary mask from a one-hot bit-vector:
//
//       TOWARDS_LSB INCLUSIVE
// i_x                           000000000000010000000000000000
// ------------------------------------------------------------
// i_y             0         0   111111111111100000000000000000
// i_y             0         1   111111111111110000000000000000
// i_y             1         0   000000000000001111111111111111
// i_y             1         1   000000000000011111111111111111

module mask #(
  // Width of vector
  parameter int W
  
  // Operation is rightward (TOWARDS_LSB) or leftward (!TOWARDS_LSB)
, parameter bit TOWARDS_LSB = 'b1

  // Operation is inclusive of the seletion bit ('x').
, parameter bit INCLUSIVE = 'b0
) (
// -------------------------------------------------------------------------- //
//
  input [W - 1:0]                                 i_x

// -------------------------------------------------------------------------- //
//
, output logic [W - 1:0]                          o_y
);

// ========================================================================== //
//                                                                            //
//  Wires                                                                     //
//                                                                            //
// ========================================================================== //

logic [W - 1:0][W - 1:0]                matrix;
logic [W - 1:0]                         y_right_non_inclusive;
logic [W - 1:0]                         y;

// ========================================================================== //
//                                                                            //
//  Combinatorial Logic                                                       //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
//
always_comb begin : mask_PROC

  for (int j = 0; j < W; j++) begin

    for (int i = 0; i < W; i++) begin

      matrix [j][i] = (i < j) ? i_x [i] : 'b0;

    end // for (int i = 0; i < N; i++)

    // OR-reduction across entire vector for each bit, expect the majority of
    // these bits to be 'b0 at elaboration. Additionally, there's a lot of
    // duplication here that synthesis should be able to optimize away.
    y_right_non_inclusive [j] = (matrix [j] != '0); 

  end // for (int j = 0; j < N; j++)

  // Conditionally invert non-inclusive mask, OR-ing in original 1-hot bit
  // vector pivot if we're inclusive.
  y = ({W{~TOWARDS_LSB}} ^ y_right_non_inclusive) | ({W{INCLUSIVE}} & i_x);

end // block: mask_PROC

// ========================================================================== //
//                                                                            //
//  Outputs                                                                   //
//                                                                            //
// ========================================================================== //

// -------------------------------------------------------------------------- //
//
assign o_y = y;

endmodule // mask
