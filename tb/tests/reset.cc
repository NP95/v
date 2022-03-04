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

#include "reset.h"

#include "../log.h"
#include "../tb.h"
#include "../test.h"
#include "Vobj/Vtb.h"
#include "directed.h"

namespace {

enum class State { PreReset, AssertReset, InReset, PostReset, PostInit, Done };

const char* to_string(State s) {
  switch (s) {
    case State::PreReset:
      return "PreReset";
    case State::AssertReset:
      return "AssertReset";
    case State::InReset:
      return "InReset";
    case State::PostReset:
      return "PostReset";
    case State::PostInit:
      return "PostInit";
    case State::Done:
      return "Done";
  }
  return "Invalid";
}

std::uint32_t expected_init_cycles() {
  // Total initialization time (in cycles) is computed as:
  //
  //    Total number of contexts in the machine
  //
  //  + FSM transition IDLE -> BUSY (1 cycle)
  //
  //  + FSM transition BUSY -> DONE (1 cycle)
  //
  return cfg::CONTEXT_N + 2;
}

struct CheckResetCB : tb::VKernelCB {
  CheckResetCB(tb::log::Scope* lg) : lg_(lg) {}
  bool on_negedge_clk(Vtb* tb) override {
    bool do_continue = true;
    switch (st_) {
      case State::PreReset: {
        V_LOG(lg_, Info, "In pre-reset...");
        st_ = State::AssertReset;
      } break;
      case State::AssertReset: {
        V_LOG(lg_, Info, "In setting reset...");
        tb::VDriver::reset(tb, true);
        st_ = State::InReset;
      } break;
      case State::InReset: {
        tb::VDriver::reset(tb, false);
        st_ = State::PostReset;
      } break;
      case State::PostReset: {
        const bool is_busy = tb::VDriver::is_busy(tb);
        V_EXPECT_TRUE(lg_, is_busy);
        if (is_busy) {
          V_LOG(lg_, Info, "Machine becomes busy...");
          cnt_ = expected_init_cycles();
          st_ = State::PostInit;
        }
      } break;
      case State::PostInit: {
        const bool timeout = (--cnt_ == 0);
        const bool is_busy = tb::VDriver::is_busy(tb);
        if (timeout) {
          V_EXPECT_TRUE(lg_, !is_busy);
        } else if (!timeout) {
          V_EXPECT_TRUE(lg_, is_busy);
        }
        if (timeout || !is_busy) {
          V_LOG(lg_, Info, "Machine becomes idle...");
          // Winddown interval for tracing.
          cnt_ = 10;
          st_ = State::Done;
        }
      } break;
      case State::Done: {
        do_continue = (--cnt_ > 0);
        if (cnt_ == 0) {
          V_LOG(lg_, Info, "Reset test complete...");
        }
      } break;
    }
    return do_continue;
  }

  std::uint32_t cnt_;
  State st_{State::PreReset};
  tb::log::Scope* lg_;
};

struct CheckReset : public tb::Test {
  CREATE_TEST_BUILDER(CheckReset);

  bool run() override {
    CheckResetCB cb{lg()};
    return k()->run(&cb);
  };
};

}  // namespace

namespace tb::tests::reset {

void init(TestRegistry* r) { CheckReset::Builder::init(r); }

}  // namespace tb::tests::reset
