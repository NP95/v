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
#include "../rnd.h"
#include "../tb.h"
#include "../test.h"
#include "Vobj/Vtb.h"
#include "cfg.h"
#include "reset.h"

namespace {

struct Options {
  float clr_weight = 0.01f;
  float add_weight = 1.0f;
  float del_weight = 1.0f;
  float rep_weight = 1.0f;
  float inv_weight = 1.0f;

  int contexts_n = 1;

  int n = 10000;

  tb::Rnd* rnd = nullptr;

  const tb::Mdl* mdl = nullptr;
};

class Stimulus {
 public:
  Stimulus(const Options& opts) : opts_(opts), val_(opts.mdl) {
    bag_.push_back(tb::Cmd::Clr, opts_.clr_weight);
    bag_.push_back(tb::Cmd::Add, opts_.add_weight);
    bag_.push_back(tb::Cmd::Del, opts_.del_weight);
    bag_.push_back(tb::Cmd::Rep, opts_.rep_weight);
    bag_.push_back(tb::Cmd::Invalid, opts_.inv_weight);
  }

  bool get(tb::UpdateCommand& uc, tb::QueryCommand& qc) {
    if (opts_.n == 0) return false;

    int issue_count = 0;
    issue_count += handle(uc);
    if (opts_.n > 0) {
      issue_count += handle(qc);
    }
    opts_.n -= issue_count;
    return (issue_count > 0);
  }

 private:
  int handle(tb::UpdateCommand& uc) {
    b = !b;
    if (b) return 0;

    // Constrain such that keys are unique

    generate(uc);
    return 1;
  }

  int handle(tb::QueryCommand& qc) {
    const tb::prod_id_t prod_id = opts_.rnd->uniform(opts_.contexts_n - 1, 0);
    const tb::level_t level = opts_.rnd->uniform(cfg::ENTRIES_N - 1);
    qc = tb::QueryCommand{prod_id, 0};
    return 1;
  }

  void generate(tb::UpdateCommand& uc) {
    const tb::Cmd cmd = bag_.pick(opts_.rnd);
    switch (cmd) {
      case tb::Cmd::Clr: {
        // No further updates required.
        const tb::prod_id_t prod_id =
            opts_.rnd->uniform(opts_.contexts_n - 1, 0);
        uc = tb::UpdateCommand{prod_id, cmd, 0, 0};
      } break;
      case tb::Cmd::Add: {
        const tb::prod_id_t prod_id =
            opts_.rnd->uniform(opts_.contexts_n - 1, 0);
        const tb::key_t key = opts_.rnd->uniform<tb::key_t>();
        const tb::volume_t volume = opts_.rnd->uniform<tb::volume_t>();
        uc = tb::UpdateCommand{prod_id, cmd, key, volume};
      } break;
      case tb::Cmd::Rep:
      case tb::Cmd::Del: {
        const tb::prod_id_t prod_id = 0;
        auto [success, key] = val_.pick_active_key(opts_.rnd, prod_id);
        uc = tb::UpdateCommand{prod_id, tb::Cmd::Del, key, 0};
      } break;
      case tb::Cmd::Invalid: {
        // Insert bubble.
        uc = tb::UpdateCommand{};
      } break;
      default:;
    }
  }

  void generate(tb::QueryCommand& qc) {}

  bool b = true;
  tb::Bag<tb::Cmd> bag_;
  Options opts_;
  tb::MdlValidation val_;
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
    Options opts;
    tb::Rnd rnd;
    opts.contexts_n = cfg::CONTEXT_N;
    opts.mdl = k()->mdl();
    opts.rnd = &rnd;
    Stimulus s{opts};
    RegressCB cb{this, std::addressof(s)};
    return k()->run(std::addressof(cb));
  }
};

}  // namespace

namespace tb::tests::regress {

void init(tb::TestRegistry* r) { Regress::Builder::init(r); }

}  // namespace tb::tests::regress
