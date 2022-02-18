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
#include "cfg.h"
#include "Vobj/Vtb.h"
#ifdef ENABLE_VCD
#  include "verilated_vcd_c.h"
#endif
#include <string>
#include <array>
#include <vector>
#include <algorithm>

namespace verif {

UpdateCommand::UpdateCommand() {
  reset();
  prod_id = 0;
  cmd = Cmd::Clr;
  key = 0;
  volume = 0;
}

void UpdateCommand::reset() {
  vld = false;
}

QueryCommand::QueryCommand() {
  reset();
}

void QueryCommand::reset() {
  vld = false;
}

QueryResponse::QueryResponse() {
  vld = false;
}

QueryResponse::QueryResponse(
    key_t key, volume_t volume, bool error, listsize_t listsize) {
  vld = true;
  key = key;
  volume = volume;
  error = error;
  listsize = listsize;
}

NotifyResponse::NotifyResponse() {
  vld = false;
}

NotifyResponse::NotifyResponse(id_t id, key_t key, volume_t volume) {
  vld = true;
  prod_id = id;
  key = key;
  volume = volume;
}

UUTHarness::UUTHarness(Vtb* tb) : tb_(tb) {}

bool UUTHarness::busy() const { return tb_->o_busy_r != 0; }

bool UUTHarness::in_reset() const { return tb_->rst == 1; }

std::uint64_t UUTHarness::tb_cycle() const { return tb_->o_tb_cycle; }

struct VDriver {

  // Drive Update Command Interface
  static void Drive(Vtb* tb, const UpdateCommand& up) {
    tb->i_upd_vld = up.vld;
    if (up.vld) {
      tb->i_upd_prod_id = up.prod_id;
      switch (up.cmd) {
        case Cmd::Clr:
          tb->i_upd_cmd = 0;
          break;
        case Cmd::Add:
          tb->i_upd_cmd = 1;
          break;
        case Cmd::Del:
          tb->i_upd_cmd = 2;
          break;
        case Cmd::Rep:
          tb->i_upd_cmd = 3;
          break;
      }
      tb->i_upd_key = up.key;
      tb->i_upd_size = up.volume;
    }
  }

  // Drive Query Command Interface
  static void Drive(Vtb* tb, const QueryCommand& qp) {
    tb->i_lut_vld = qp.vld;
    if (qp.vld) {
      tb->i_lut_prod_id = qp.prod_id;
      tb->i_lut_level = qp.level;
    }
  }

  // Sample Notify Reponse Interface:
  static void Sample(Vtb* tb, NotifyResponse& nr) {
  }

  // Sample Query Response Interface:
  static void Sample(Vtb* tb, QueryResponse& qr) {
  }

};

template<typename T, std::size_t N>
class DelayPipe {
 public:
  DelayPipe() {
    clear();
  }

  void push_back(const T& t) { p_[wr_ptr_] = t; }

  const T& head() const { return p_[rd_ptr_]; }

  void step() {
    wr_ptr_ = (wr_ptr_ + 1) % (N + 1);
    rd_ptr_ = (rd_ptr_ + 1) % (N + 1);
  }

  void clear() {
    for (T& t : p_) t = T{};

    wr_ptr_ = N;
    rd_ptr_ = 0;
  }

 private:
  std::size_t wr_ptr_, rd_ptr_;
  std::array<T, N + 1> p_;
};

class ValidationModel {
  static constexpr const std::size_t QUERY_PIPE_DELAY = 2;
  static constexpr const std::size_t UPDATE_PIPE_DELAY = 4;

  struct Entry {
    bool operator<(const Entry& rhs) const {
      return (key < rhs.key);
    }

    key_t key;
    volume_t volume;
  };

 public:
  ValidationModel() {
    reset();
  }

  void reset() {
    for (std::vector<Entry>& entries : tbl_) {
      entries.clear();
    }
  }

  void step() {
    notify_.step();
    queries_.step();

    handle_uc();
    handle_qc();
    handle_qr();
    handle_nr();
  }

  void apply(const UpdateCommand& uc) { uc_ = uc; }
  void apply(const QueryCommand& qc) { qc_ = qc; }
  void apply(const QueryResponse& qr) { qr_ = qr; }
  void apply(const NotifyResponse& nr) { nr_ = nr; }

 private:
  void handle_uc() {
    if (!uc_.vld) {
      // No command is present at the interface on this cycle, we do not
      // therefore expect a notification.
      notify_.push_back(NotifyResponse{});
    };

    // Validate that ID provided by stimulus is within [0, cfg::CONTEXT_N).
    ASSERT_LT(uc_.prod_id, cfg::CONTEXT_N);

    bool raise_notify = false;
    std::vector<Entry>& ctxt{tbl_[uc_.prod_id]};
    switch (uc_.cmd) {
      case Cmd::Clr: {
        raise_notify = !ctxt.empty();
        ctxt.clear();
      } break;
      case Cmd::Add: {
        ctxt.push_back(Entry{uc_.key, uc_.volume});
        std::stable_sort(ctxt.begin(), ctxt.end());
        if (ctxt.size() > cfg::ENTRIES_N) {
          // Entry has been spilled on this Add.
          //
          // TODO: Raise some error notification to indicate that the context
          // has been truncated due to a capacity conflict and an entry has been
          // dropped.
          ctxt.pop_back();
        }
      } break;
      case Cmd::Rep:
      case Cmd::Del: {
        auto find_key = [&](const Entry& e) { return (e.key == uc_.key); };
        auto it = std::find_if(ctxt.begin(), ctxt.end(), find_key);

        // The context was either empty or the key was not found. The current
        // command becomes a NOP.
        if (it == ctxt.end()) return;

        const bool is_first_entry = (it == ctxt.begin());
        raise_notify = is_first_entry;

        if (uc_.cmd == Cmd::Rep) {
          // Replace: Update volume for current entry.
          it->volume = uc_.volume;
        } else {
          // Delete: Remove entry from context.
          ctxt.erase(it);
        }

      } break;
    }

    if (raise_notify) {
      // Notification expected as a consequence of current command.
      notify_.push_back(NotifyResponse{uc_.prod_id, uc_.key, uc_.volume});
    } else {
      // Otherwise, no notification resonse expected for this command.
      notify_.push_back(NotifyResponse{});
    }
  }

  void handle_qc() {
    QueryResponse qr;
    if (qc_.vld) {

      ASSERT_LT(qc_.prod_id, cfg::CONTEXT_N);
      const std::vector<Entry>& ctxt{tbl_[qc_.prod_id]};

      bool error = (qc_.level >= ctxt.size()); // TODO: in-flight transactions.
      if (error) {
        // Query is errored, other fields are invalid.
        qr = QueryResponse{0, 0, true, 0};
      } else {
        // Query is valid, populate as necessary.
        const Entry& e{ctxt[qc_.level]};
        const listsize_t listsize = static_cast<listsize_t>(ctxt.size());
        qr = QueryResponse{e.key, e.volume, false, listsize};
      }
    }
    queries_.push_back(qr);
  }

  void handle_qr() {
  }

  void handle_nr() {
  }

  UpdateCommand uc_;
  QueryCommand qc_;
  QueryResponse qr_;
  NotifyResponse nr_;

  std::array<std::vector<Entry>, cfg::CONTEXT_N> tbl_;

  DelayPipe<NotifyResponse, QUERY_PIPE_DELAY> notify_;
  DelayPipe<QueryResponse, UPDATE_PIPE_DELAY> queries_;
};


TB::TB(const Options& opts) : opts_(opts) {
  build_verilated_environment();
}

TB::~TB() {
#ifdef ENABLE_VCD
  if (vcd_) {
    vcd_->close();
    delete vcd_;
  }
#endif
  delete uut_;
  delete ctxt_;
}

void TB::build_verilated_environment() {
  ctxt_ = new VerilatedContext();
  uut_ = new Vtb(ctxt_);
#ifdef ENABLE_VCD
  if (opts_.enable_vcd) {
    Verilated::traceEverOn(true);
    vcd_ = new VerilatedVcdC();
    uut_->trace(vcd_, 99);
    if (!opts_.vcd_filename) {
      auto test_info = ::testing::UnitTest::GetInstance()->current_test_info();
      const std::string vcd_name{test_info->name()};
      opts_.vcd_filename = vcd_name + ".vcd";
    }
    vcd_->open(opts_.vcd_filename->c_str());
  }
#endif

}

void TB::run(Test* t) {
  if (!t) return;

  tb_time_ = 0;

  ValidationModel mdl_;

  UpdateCommand up;
  NotifyResponse nr;
  QueryCommand qp;
  QueryResponse qr;

  uut_->clk = 0;
  uut_->rst = 0;

  bool do_stepping = true;
  while (do_stepping) {
    tb_time_++;

    if (tb_time_ % 5 == 0) {
      if (uut_->clk) {
        // Immediately before negative clock edge.

        // Clear stimulus
        up.reset();
        qp.reset();

        const Test::Status status = t->on_negedge_clk(up, qp);

        VDriver::Drive(uut_, up);
        VDriver::Drive(uut_, qp);
        VDriver::Sample(uut_, nr);
        VDriver::Sample(uut_, qr);

        mdl_.apply(up);
        mdl_.apply(qp);
        mdl_.apply(nr);
        mdl_.apply(qr);
        mdl_.step();

        switch (status) {
          case Test::Status::ApplyReset:
            mdl_.reset();
            uut_->rst = true;
            break;
          case Test::Status::RescindReset:
            uut_->rst = false;
            break;
          case Test::Status::Terminate:
            do_stepping = false;
            break;
          case Test::Status::Continue:
            break;
          default:
            // Unhandled command
            break;
        }

      }
      uut_->clk = !uut_->clk;
    }

    uut_->eval();
#ifdef ENABLE_VCD
    if (vcd_)
      vcd_->dump(tb_time_);
#endif
    if (uut_->clk && uut_->rst) {
      // On rising-edge, if machine is in reset, reset the validation model back
      // to its initial state.
      mdl_.reset();
    }
  }

}

} // namespace verif
