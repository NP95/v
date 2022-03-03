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

#include "Vobj/Vtb.h"
#include "mdl.h"
#include "test.h"
#include "tests/regress.h"
#include "tests/smoke_cmds.h"
#ifdef ENABLE_VCD
#include "verilated_vcd_c.h"
#endif

namespace {

void set_bool(vluint8_t* v, bool b) { *v = b ? 1 : 0; }

struct VPorts {
  static bool clk(Vtb* tb) { return (tb->clk != 0); }
  static void clk(Vtb* tb, bool v) { set_bool(&tb->clk, v); }

  static bool rst(Vtb* tb) { return (tb->rst != 0); }
  static void rst(Vtb* tb, bool v) { set_bool(&tb->rst, v); }
};

}  // namespace

namespace tb {

void init(TestRegistry* tr) {
  tests::regress::init(tr);
  tests::smoke_cmds::init(tr);
}

VKernel::VKernel(const VKernelOptions& opts) : opts_(opts), tb_time_(0) {
  build_verilated_environment();
  mdl_ = std::make_unique<Mdl>(vtb_.get());
}

void VKernel::run(VKernelCB* cb) {
  if (!cb) return;

  tb_time_ = 0;

  Vtb* vtb = vtb_.get();
  VPorts::clk(vtb, false);
  VPorts::rst(vtb, false);

  bool do_stepping = true;
  while (do_stepping) {
    tb_time_++;

    if (tb_time_ % 5 == 0) {
      if (VPorts::clk(vtb)) {
        do_stepping = cb->on_negedge_clk(vtb);
        mdl_->step();
        VPorts::clk(vtb, false);
      } else {
        do_stepping = cb->on_posedge_clk(vtb);
        VPorts::clk(vtb, true);
      }
    }

    vtb_->eval();
#ifdef ENABLE_VCD
    if (vcd_) vcd_->dump(tb_time_);
#endif
  }
}

void VKernel::build_verilated_environment() {
  vctxt_ = std::make_unique<VerilatedContext>();
  vtb_ = std::make_unique<Vtb>(vctxt_.get());
#ifdef ENABLE_VCD
  if (opts_.vcd_on) {
    vctxt_->traceEverOn(true);
    vcd_ = std::make_unique<VerilatedVcdC>();
    vtb_->trace(vcd_.get(), 99);
    vcd_->open(opts_.vcd_fn.c_str());
  }
#endif
}

// Drive Update Command Interface
void VDriver::issue(Vtb* tb, const UpdateCommand& up) {
  tb->i_upd_vld = up.vld();
  if (up.vld()) {
    tb->i_upd_prod_id = up.prod_id();
    switch (up.cmd()) {
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
    tb->i_upd_key = up.key();
    tb->i_upd_size = up.volume();
  }
}

// Drive Query Command Interface
void VDriver::issue(Vtb* tb, const QueryCommand& qc) {
  tb->i_lut_vld = qc.vld();
  if (qc.vld()) {
    tb->i_lut_prod_id = qc.prod_id();
    tb->i_lut_level = qc.level();
  }
}

bool VDriver::is_busy(Vtb* tb) { return (tb->o_busy_r != 0); }

}  // namespace tb

/*
#include <algorithm>
#include <array>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "cfg.h"
#include "tb.h"

namespace verif {

namespace utilities {

bool to_bool(vluint8_t v) { return v != 0; }

} // namespace utilities


UUTHarness::UUTHarness(Vtb* tb) : tb_(tb) {}

bool UUTHarness::busy() const { return tb_->o_busy_r != 0; }

bool UUTHarness::in_reset() const { return tb_->rst == 1; }

std::uint64_t UUTHarness::tb_cycle() const { return tb_->o_tb_cycle; }

struct VDriver {

  // Drive Update Command Interface
  static void Drive(Vtb* tb, const UpdateCommand& up) {
    tb->i_upd_vld = up.vld();
    if (up.vld()) {
      tb->i_upd_prod_id = up.prod_id();
      switch (up.cmd()) {
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
      tb->i_upd_key = up.key();
      tb->i_upd_size = up.volume();
    }
  }

  // Drive Query Command Interface
  static void Drive(Vtb* tb, const QueryCommand& qc) {
    tb->i_lut_vld = qc.vld();
    if (qc.vld()) {
      tb->i_lut_prod_id = qc.prod_id();
      tb->i_lut_level = qc.level();
    }
  }

  // Sample Notify Reponse Interface:
  static void Sample(Vtb* tb, NotifyResponse& nr) {
    if (tb->o_lv0_vld_r) {
      nr = NotifyResponse{
        tb->o_lv0_prod_id_r, tb->o_lv0_key_r, tb->o_lv0_size_r};
    } else {
      nr = NotifyResponse{};
    }
  }

  // Sample Query Response Interface:
  static void Sample(Vtb* tb, QueryResponse& qr) {
    if (tb->o_lut_vld_r) {
      qr = QueryResponse{tb->o_lut_key, tb->o_lut_size,
        utilities::to_bool(tb->o_lut_error), tb->o_lut_listsize};
    } else {
      qr = QueryResponse{};
    }
  }

};

template<typename T, std::size_t N>
class DelayPipeBase {
 public:
  DelayPipeBase() {
    clear();
  }

  std::string to_string() const {
    std::stringstream ss;
    for (std::size_t i = 0; i < p_.size(); i++) {
      const T& t{p_[(rd_ptr_ + i) % p_.size()]};
      ss << t.to_string() << "\n";
    }
    return ss.str();
  }

  void push_back(const T& t) { p_[wr_ptr_] = t; }

  const T& head() const { return p_[rd_ptr_]; }

  void step() {
    wr_ptr_ = (wr_ptr_ + 1) % p_.size();
    rd_ptr_ = (rd_ptr_ + 1) % p_.size();
  }

  void clear() {
    for (T& t : p_) t = T{};

    wr_ptr_ = N;
    rd_ptr_ = 0;
  }

 protected:
  std::size_t wr_ptr_, rd_ptr_;
  std::array<T, N + 1> p_;
};

template<typename T, std::size_t N>
class DelayPipe : public DelayPipeBase<T, N> {
  using base_class_type = DelayPipeBase<T, N>;
};

template<std::size_t N>
class DelayPipe<UpdateResponse, N> : public DelayPipeBase<UpdateResponse, N> {
  using base_class_type = DelayPipeBase<UpdateResponse, N>;

  using base_class_type::p_;
  using base_class_type::wr_ptr_;
 public:
  bool has_prod_id(prod_id_t prod_id) const {
    for (std::size_t i = 0; i < N; i++) {
      const UpdateResponse& ur{p_[(wr_ptr_ + i) % p_.size()]};
      if (ur.vld() && (ur.prod_id() == prod_id))
        return true;
    }
    return false;
  }
};

bool compare_keys(key_t rhs, key_t lhs) {
  return cfg::is_bid_table ? (rhs < lhs) : (rhs > lhs);
}

struct Entry {
  std::string to_string() const {
    std::stringstream ss;
    ss << key << ", " << volume;
    return ss.str();
  }

  key_t key;
  volume_t volume;
};

bool compare_entries(const Entry& lhs, const Entry& rhs) {
  return compare_keys(lhs.key, rhs.key);
}

std::ostream& operator<<(std::ostream& os, const Entry& e) {
  return os << e.to_string();
}

class ValidationModel {
  static constexpr const std::size_t QUERY_PIPE_DELAY = 1;
  static constexpr const std::size_t UPDATE_PIPE_DELAY = 4;

  struct PipeProdId {
    bool vld{false};
    prod_id_t prod_id;
  };

 public:
  ValidationModel(UUTHarness harness)
      : harness_(harness) {
    reset();
  }

  void reset() {
    for (std::vector<Entry>& entries : tbl_) {
      entries.clear();
    }
  }

  void step() {
    nr_pipe_.step();
    ur_pipe_.step();
    qr_pipe_.step();

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
    if (!uc_.vld()) {
      // No command is present at the interface on this cycle, we do not
      // therefore expect a notification.
      nr_pipe_.push_back(NotifyResponse{});
      ur_pipe_.push_back(UpdateResponse{});
      return;
    };

    // Validate that ID provided by stimulus is within [0, cfg::CONTEXT_N).
    ASSERT_LT(uc_.prod_id(), cfg::CONTEXT_N);

    UpdateResponse ur{};
    NotifyResponse nr{};
    std::vector<Entry>& ctxt{tbl_[uc_.prod_id()]};
    switch (uc_.cmd()) {
      case Cmd::Clr: {
        ur = UpdateResponse{uc_.prod_id()};
        if (!ctxt.empty()) {
          nr = NotifyResponse{uc_.prod_id(), 0, 0};
        }
        ctxt.clear();
      } break;
      case Cmd::Add: {
        ur = UpdateResponse{uc_.prod_id()};
        if (ctxt.empty() || compare_keys(uc_.key(), ctxt.begin()->key)) {
          nr = NotifyResponse{uc_.prod_id(), uc_.key(), uc_.volume()};
        }
        ctxt.push_back(Entry{uc_.key(), uc_.volume()});
        std::stable_sort(ctxt.begin(), ctxt.end(), compare_entries);
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
        ur = UpdateResponse{uc_.prod_id()};
        auto find_key = [&](const Entry& e) { return (e.key == uc_.key()); };
        auto it = std::find_if(ctxt.begin(), ctxt.end(), find_key);

        // The context was either empty or the key was not found. The current
        // command becomes a NOP.
        if (it == ctxt.end()) break;

        if (it == ctxt.begin()) {
          // Item to be replaced is first, therefore raise notification of
          // current first item in context.
          nr = NotifyResponse{uc_.prod_id(), it->key, it->volume};
        }

        if (uc_.cmd() == Cmd::Rep) {
          // Perform final replacement of 'volume'.
          it->volume = uc_.volume();
        } else {
          // Delete: Remove entry from context.
          ctxt.erase(it);
        }
      } break;
    }

    // Update predicted notify responses based upon outcome of prior command.
    ur_pipe_.push_back(ur);
    nr_pipe_.push_back(nr);
  }

  void handle_qc() {
    QueryResponse qr;
    if (qc_.vld()) {
      ASSERT_LT(qc_.prod_id(), cfg::CONTEXT_N);
      const std::vector<Entry>& ctxt{tbl_[qc_.prod_id()]};

      if ((qc_.level() >= ctxt.size()) || ur_pipe_.has_prod_id(qc_.prod_id())) {
        // Query is errored, other fields are invalid.
        qr = QueryResponse{0, 0, true, 0};
      } else {
        // Query is valid, populate as necessary.
        const Entry& e{ctxt[qc_.level()]};
        const listsize_t listsize = static_cast<listsize_t>(ctxt.size());
        qr = QueryResponse{e.key, e.volume, false, listsize};
      }
    }
    qr_pipe_.push_back(qr);
  }

  void handle_qr() {
    const QueryResponse& predicted = qr_pipe_.head();
    const QueryResponse& actual = qr_;
    EXPECT_EQ(predicted.vld(), actual.vld());
    if (predicted.vld() && actual.vld()) {
      EXPECT_EQ(predicted.error(), actual.error());
      if (!predicted.error() && !actual.error()) {
        EXPECT_EQ(predicted.key(), actual.key());
        EXPECT_EQ(predicted.volume(), actual.volume());
        EXPECT_EQ(predicted.listsize(), actual.listsize());
      }
    }
  }

  void handle_nr() {
    const NotifyResponse& predicted = nr_pipe_.head();
    const NotifyResponse& actual = nr_;
    EXPECT_EQ(predicted.vld(), actual.vld());
    if (predicted.vld()) {
      EXPECT_EQ(predicted.prod_id(), actual.prod_id());
      EXPECT_EQ(predicted.key(), actual.key());
      EXPECT_EQ(predicted.volume(), actual.volume());
    }
  }

  UpdateCommand uc_;
  QueryCommand qc_;
  QueryResponse qr_;
  NotifyResponse nr_;

  std::array<std::vector<Entry>, cfg::CONTEXT_N> tbl_;
  DelayPipe<NotifyResponse, UPDATE_PIPE_DELAY> nr_pipe_;
  DelayPipe<UpdateResponse, UPDATE_PIPE_DELAY> ur_pipe_;
  DelayPipe<QueryResponse, QUERY_PIPE_DELAY> qr_pipe_;

  // UUT harness
  UUTHarness harness_;
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

void TB::run(Test* t) {
  if (!t) return;

  tb_time_ = 0;

  ValidationModel mdl_{get_harness()};

  UpdateCommand up;
  NotifyResponse nr;
  QueryCommand qc;
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
        up = UpdateCommand{};
        qc = QueryCommand{};

        const Test::Status status = t->on_negedge_clk(up, qc);

        VDriver::Drive(uut_, up);
        VDriver::Drive(uut_, qc);
        VDriver::Sample(uut_, nr);
        VDriver::Sample(uut_, qr);

        mdl_.apply(up);
        mdl_.apply(qc);
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
*/
//} // namespace verif
