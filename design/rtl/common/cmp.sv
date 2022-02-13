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

module cmp #(
  // Width of comparitor
  parameter int W
) (
// -------------------------------------------------------------------------- //
//
  input [W - 1:0]                                 i_a
, input [W - 1:0]                                 i_b
//
, output logic                                    o_eq
, output logic                                    o_gt
, output logic                                    o_lt
);

// ========================================================================== //
//                                                                            //
//  Wires                                                                     //
//                                                                            //
// ========================================================================== //

`ifdef GATEY
logic [W - 1:0]                         a_xor_b;
logic [W - 1:0]                         a_andnot_b;
`endif
logic                                   eq;
logic                                   gt;
logic                                   lt;

// TOOD: magnitude comparison is supposed to be signed!!!!

// ========================================================================== //
//                                                                            //
//  Combinatorial Logic                                                       //
//                                                                            //
// ========================================================================== //

`ifdef GATEY

// -------------------------------------------------------------------------- //
//
always_comb begin : cmp_PROC

  // EQ:
  for (int i = 0; i < W; i++) begin
    a_xor_b    = (a[i] ^ b[i]);
    a_andnot_b = (a[i] & ~b[i]);
  end

  // Equal if no bits differ.
  eq = (~|a_xor_b);

  // Greater if the first highest bit in A
  gt = (i_a > i_b); // TODO

  // If not equal, or greater-than, it must be less-than.
  lt = ~(eq | gt);

end // block: cmp_PROC

`else // !`ifdef GATEY

// -------------------------------------------------------------------------- //
//
always_comb begin : cmp_PROC

  eq = (i_a == i_b);
  gt = (i_a > i_b);

  // If not equal, or greater-than, it must be less-than.
  lt = ~(eq | gt);

end // block: cmp_PROC

`endif

// ========================================================================== //
//                                                                            //
//  Outputs                                                                   //
//                                                                            //
// ========================================================================== //

assign o_eq = eq;
assign o_gt = gt;
assign o_lt = lt;

endmodule // cmp
