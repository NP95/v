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

#include "../log.h"
#include "../mdl.h"
#include "../tb.h"
#include "../test.h"
#include "Vobj/Vtb.h"
#include "reset.h"

namespace {

class Stimulus {
 public:
  Stimulus() {}

  bool get(tb::UpdateCommand& uc, tb::QueryCommand& qc) {
    if (cnt_ == 0) return false;

    int issue_count = 0;
    issue_count += handle(uc) ? 1 : 0;
    if (cnt_ > 0) {
      issue_count += handle(qc) ? 1 : 0;
    }
    cnt_ -= issue_count;
    return (issue_count > 0);
  }

 private:
  bool handle(tb::UpdateCommand& uc) {
    b = !b;
    if (b) return false;
    uc = tb::UpdateCommand{0, tb::Cmd::Add, 0, 0};
    return true;
  }

  bool handle(tb::QueryCommand& qc) {
    qc = tb::QueryCommand{0, 0};
    return true;
  }

  int cnt_ = 5;
  bool b = true;
};

struct RegressCB : public tb::VKernelCB {
  RegressCB(tb::Test* parent, Stimulus* s)
      : parent_(parent), s_(s), rstt_(parent->lg()) {}

  bool on_negedge_clk(Vtb* tb) {
    // Issue reset process.
    if (!rstt_.is_done()) {
      rstt_.check_reset(tb);
      return !rstt_.is_failed();
    }

    tb::UpdateCommand uc{};
    tb::QueryCommand qc{};
    if (!s_->get(uc, qc)) {
      // No further stimulus.
      return false;
    }

    // Issue commands to UUT
    tb::VDriver::issue(tb, uc);
    tb::VDriver::issue(tb, qc);

    return true;
  }

 private:
  Stimulus* s_;
  tb::Test* parent_;
  tb::ResetTracker rstt_;
};

struct Regress : public tb::Test {
  CREATE_TEST_BUILDER(Regress);

  bool run() override {
    Stimulus s{};
    RegressCB cb{this, std::addressof(s)};
    return k()->run(std::addressof(cb));
  }
};

}  // namespace

namespace tb::tests::regress {

void init(tb::TestRegistry* r) { Regress::Builder::init(r); }

}  // namespace tb::tests::regress
