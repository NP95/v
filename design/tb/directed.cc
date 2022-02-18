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

#include "tb.h"
#include <tuple>
#include <deque>

TEST(Directed, CheckMemoryInitializationProcess) {

  class WaitForEndOfInitTest : public verif::Test {
    enum class State {
      PreReset,
      InReset,
      PostReset,
      InInitialization,
      Timeout,
      Okay
    };
  public:
    WaitForEndOfInitTest(verif::UUTHarness uut, std::uint64_t timeout = 1000)
      : uut_(uut), timeout_(timeout), state_(State::PreReset) {
    }

    Status on_negedge_clk(verif::UpdateCommand& up,
                          verif::QueryCommand& qp) override {
      switch (state_) {
      case State::PreReset:
        cnt_ = 0;
        state_ = State::InReset;
        return Status::ApplyReset;
        break;
      case State::InReset:
        if (++cnt_ > 10) {
          state_ = State::PostReset;
          return Status::RescindReset;
        }
        break;
      case State::PostReset:
        state_ = State::InInitialization;
        cnt_ = 0;
        break;
      case State::InInitialization:
        if (++cnt_ > timeout_) {
          state_ = State::Timeout;
          return Status::Terminate;
        } else if (!uut_.busy()) {
          state_ = State::Okay;
        }
        break;
      case State::Timeout:
      case State::Okay:
      default:
        return Status::Terminate;
        break;
      }

      return Status::Continue;
    }

    bool is_passed() const override { return state_ == State::Okay; }
  private:
    State state_;
    std::uint64_t cnt_ = 0;
    std::uint64_t timeout_ = 0;
    bool has_seen_busy_ = false;
    verif::UUTHarness uut_;
  };

  verif::Options opts;
  opts.enable_vcd = true;
  verif::TB tb{opts};
  WaitForEndOfInitTest test{tb.get_harness(), 1000};

  // Run test
  //  tb.run(&test);
  //  EXPECT_TRUE(test.is_passed());
}

class DirectedTestDriver : public verif::Test {
  enum class State {
    ApplyReset,
    RescindReset,
    AwaitInitializationStart,
    Initializing,
    DriveStimulus,
    WindDown
  };
public:

  using input_t = std::tuple<verif::UpdateCommand, verif::QueryCommand>;

  DirectedTestDriver(verif::UUTHarness uut)
    : uut_(uut), state_(State::ApplyReset)
  {}

  void push_back(const verif::UpdateCommand& up,
                 const verif::QueryCommand& qp = verif::QueryCommand{}) {
    inputs_.push_back(std::make_tuple(up, qp));
  }

  void push_back(const verif::QueryCommand& qp,
                 const verif::UpdateCommand& up = verif::UpdateCommand{}) {
    push_back(up, qp);
  }

  void wait_cycles(std::size_t n = 1) {
    // Insert N * NOPs into machine.
    while (n-- > 0) {
      push_back(verif::UpdateCommand{}, verif::QueryCommand{});
    }
  }

private:

  Status on_negedge_clk(verif::UpdateCommand& up,
                        verif::QueryCommand& qp) override {
    switch (state_) {
    default: {
      // Invalid state;
      return Status::Terminate;
    } break;
    case State::ApplyReset: {
      state_ = State::RescindReset;
      return Status::ApplyReset;
    } break;
    case State::RescindReset: {
      state_ = State::AwaitInitializationStart;
      return Status::RescindReset;
    } break;
    case State::AwaitInitializationStart: {
      if (uut_.busy()) {
        state_ = State::Initializing;
      }
    } break;
    case State::Initializing: {
      if (!uut_.busy()) {
        if  (!inputs_.empty()) {
          state_ = State::DriveStimulus;
        } else {
          // No stimulus provided.
          state_ = State::WindDown;
        }
      }
    } break;
    case State::DriveStimulus: {
      std::tie(up, qp) = inputs_.front();
      inputs_.pop_front();
      if (inputs_.empty()) {
        // Stimulus Exhausted
        cnt_ = 0;
        state_ = State::WindDown;
      }
    } break;
    case State::WindDown: {
      if (++cnt_ > 10) {
        return Status::Terminate;
      }
    } break;

    }
    return Status::Continue;
  }

  std::uint64_t cnt_ = 0;

  // Transactor state.
  State state_;

  // Stimulus
  std::deque<input_t> inputs_;

  // Unit under test
  verif::UUTHarness uut_;
};

TEST(Directed, CheckAddItem) {
  verif::Options opts;
  opts.enable_vcd = true;
  verif::TB tb{opts};
  DirectedTestDriver test{tb.get_harness()};

  verif::UpdateCommand up;
  up.vld = true;
  test.push_back(up);


  test.wait_cycles(10);

  verif::QueryCommand qp;
  qp.vld = true;
  test.push_back(qp);

  // Run test
  tb.run(&test);
  EXPECT_TRUE(test.is_passed());
}
