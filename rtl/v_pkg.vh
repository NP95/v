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

`ifndef DESIGN_RTL_V_PKG_VH
`define DESIGN_RTL_V_PKG_VH

package v_pkg;

typedef logic [$clog2(CONTEXT_N) - 1:0]  id_t;

typedef enum logic [1:0] {
  // Clear contents of list
  CMD_CLEAR   = 2'b00,
  // Add new entry to list, may implicitly delete
  // entry if list is currently full occupied.
  CMD_ADD     = 2'b01,
  // Delete entry by key value.
  CMD_DELETE  = 2'b10,
  // Replace entry by key.
  CMD_REPLACE = 2'b11

} cmd_t;

typedef logic [63:0] key_t;

localparam int KEY_BITS = $bits(key_t);

typedef logic [31:0] volume_t;

localparam int VOLUME_BITS = $bits(volume_t);

typedef logic [31:0] size_t;


typedef logic [$clog2(ENTRIES_N) - 1:0]  level_t;

typedef logic [$clog2(ENTRIES_N + 1) - 1:0]  listsize_t;
localparam int LISTSIZE_W = $bits(listsize_t);

typedef struct packed {
  listsize_t listsize;
  logic [ENTRIES_N - 1:0] vld;
  key_t [ENTRIES_N - 1:0] key;
  volume_t [ENTRIES_N - 1:0] volume;
} state_t;

localparam int      STATE_BITS = $bits(state_t);

typedef logic [$clog2(CONTEXT_N) - 1:0] addr_t;

endpackage // v_pkg

`endif
