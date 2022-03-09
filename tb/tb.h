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

#ifndef V_TB_TB_H
#define V_TB_TB_H

#include <exception>
#include <memory>
#include <string>

#include "cfg.h"
#include "opts.h"

// Verilator artifacts
class Vtb;
class VerilatedVcdC;
class VerilatedContext;

namespace tb {

class TestRegistry;
class Mdl;
class UpdateCommand;
class QueryCommand;
namespace log {
class Scope;
}  // namespace log

void init(TestRegistry* tr);

struct VKernelCB {
  virtual ~VKernelCB() = default;

  virtual bool on_negedge_clk(Vtb* tb) { return true; }

  virtual bool on_posedge_clk(Vtb* tb) { return true; }
};

class VKernelException {
 public:
  VKernelException(const char* msg) : msg_(msg) {}
  const char* msg() const { return msg_; }

 private:
  const char* msg_;
};

class VKernel {
 public:
  explicit VKernel(const VKernelOptions& opts);
  ~VKernel();

  bool run(VKernelCB* cb);
  void end();

  std::uint64_t tb_time() const { return tb_time_; }
  std::uint64_t tb_cycle() const;
  const Mdl* mdl() const { return mdl_.get(); }

 private:
  void build_verilated_environment();
  bool eval_clock_edge(VKernelCB* cb, bool edge);

  std::unique_ptr<Mdl> mdl_;
  std::unique_ptr<VerilatedContext> vctxt_;
  std::unique_ptr<Vtb> vtb_;
#ifdef ENABLE_VCD
  std::unique_ptr<VerilatedVcdC> vcd_;
#endif
  VKernelOptions opts_;
  std::uint64_t tb_time_;
  log::Scope* l_;
};

struct VDriver {
  //
  static void issue(Vtb* tb, const UpdateCommand& uc);

  //
  static void issue(Vtb* tb, const QueryCommand& qc);

  //
  static bool is_busy(Vtb* tb);

  static void reset(Vtb* tb, bool r);
};

}  // namespace tb

#endif
