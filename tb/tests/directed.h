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

#ifndef V_TB_TESTS_DIRECTED_H
#define V_TB_TESTS_DIRECTED_H

#include <deque>
#include <memory>
#include <tuple>

#include "../mdl.h"
#include "../test.h"

class Instruction;

namespace tb::tests {

class Directed : public Test {
  friend class Impl;

  class Impl;
  std::unique_ptr<Impl> impl_;

  using command_pair_type = std::tuple<const UpdateCommand, const QueryCommand>;

 public:
  explicit Directed();
  ~Directed();

  void run() override;

  void wait_until_not_busy();

  void push_back(const UpdateCommand& uc,
                 const QueryCommand& qc = QueryCommand{});

  void push_back(const QueryCommand& qc,
                 const UpdateCommand& uc = UpdateCommand{}) {
    push_back(uc, qc);
  }

  void wait_cycles(std::size_t n = 1);

 private:
  std::deque<std::unique_ptr<Instruction> > d_;
};

}  // namespace tb::tests

#endif
