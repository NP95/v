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
#include "cfg.h"
#include "log.h"
#include "model.h"
#include "test.h"
#include "rnd.h"
#include "tests/regress.h"
#include "tests/reset.h"
#include "tests/smoke_cmds.h"
#ifdef ENABLE_VCD
#include "verilated_vcd_c.h"
#endif

namespace {

void set_bool(vluint8_t* v, bool b) { *v = b ? 1 : 0; }

struct VPorts {
  static bool clk(Vtb* tb) { return (tb->clk != 0); }
  static void clk(Vtb* tb, bool v) { set_bool(&tb->clk, v); }

  static bool arst_n(Vtb* tb) { return (tb->arst_n != 0); }
  static void arst_n(Vtb* tb, bool v) { set_bool(&tb->arst_n, v); }

  static std::uint64_t tb_cycle(Vtb* tb) { return tb->o_tb_cycle; }
};

}  // namespace

namespace tb {

void register_tests(TestRegistry& tr) {
  tests::reset::init(tr);
  tests::regress::init(tr);
  tests::smoke_cmds::init(tr);
}

Kernel::Kernel() : tb_time_(0) {
  vctxt_ = std::make_unique<VerilatedContext>();
  vtb_ = std::make_unique<Vtb>(vctxt_.get());
#ifdef ENABLE_VCD
  if (Sim::vcd_on) {
    vctxt_->traceEverOn(true);
    vcd_ = std::make_unique<VerilatedVcdC>();
    vtb_->trace(vcd_.get(), 99);
    vcd_->open(Sim::vcd_fn.c_str());
  }
#endif
  Scope* mdl_logger_scope = nullptr;
  if (Sim::logger) {
    logger_ = Sim::logger->top();
    mdl_logger_scope = logger_->create_child("mdl");
  }
  tb::Sim::model = std::make_unique<Model>(vtb_.get(), mdl_logger_scope);
}

Kernel::~Kernel() {}

bool Kernel::run(KernelCallbacks* cb) {
  if (!cb) return false;

  tb_time_ = 0;

  Vtb* vtb = vtb_.get();

  // Drive all interfaces to a quiescent state.
  VPorts::clk(vtb, false);
  VPorts::arst_n(vtb, false);
  VDriver::issue(vtb, UpdateCommand{});
  VDriver::issue(vtb, QueryCommand{});

  int rundown_n = 5;
  bool do_stepping = true;
  bool failed = false;
  while (do_stepping || --rundown_n > 0) {
    try {
      if (++tb_time_ % 5 == 0) {
        const bool edge = VPorts::clk(vtb);
        if (do_stepping) {
          do_stepping = eval_clock_edge(cb, edge);
        }
        if (tb::Sim::errors >= tb::Sim::error_max) {
          do_stepping = false;
        }
        VPorts::clk(vtb, !edge);
      }
    } catch (const KernelException& ex) {
      failed = true;
      do_stepping = false;
    }

    vtb_->eval();
#ifdef ENABLE_VCD
    if (vcd_) vcd_->dump(tb_time_);
#endif
  }
  end();
  return failed;
}

bool Kernel::eval_clock_edge(KernelCallbacks* cb, bool edge) {
  bool do_stepping;
  if (edge) {
    do_stepping = cb->on_negedge_clk(vtb_.get());
    Sim::model->step();
  } else {
    do_stepping = cb->on_posedge_clk(vtb_.get());
  }
  return do_stepping;
}

void Kernel::end() {
  vtb_->final();
#ifdef ENABLE_VCD
  if (vcd_) {
    vcd_->close();
    vcd_.reset(nullptr);
  }
#endif
}

std::uint64_t Kernel::tb_cycle() const { return VPorts::tb_cycle(vtb_.get()); }

// Drive Update Command Interface
void VDriver::issue(Vtb* tb, const UpdateCommand& up) {
  tb->i_upd_vld = up.vld();
  if (up.vld()) {
    tb->i_upd_prod_id = up.prod_id();
    tb->i_upd_cmd = static_cast<std::underlying_type_t<Cmd>>(up.cmd());
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

void VDriver::reset(Vtb* tb, bool r) { tb->arst_n = r ? 1 : 0; }

}  // namespace tb
