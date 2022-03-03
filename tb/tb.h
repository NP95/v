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

// Verilator artifacts
class Vtb;
#ifdef ENABLE_VCD
class VerilatedVcdC;
#endif
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

struct VKernelOptions {
  bool vcd_on = false;

  std::string vcd_fn = "sim.vcd";
};

struct VKernelCB {
  virtual ~VKernelCB() = default;

  virtual bool on_negedge_clk(Vtb* tb) { return false; }

  virtual bool on_posedge_clk(Vtb* tb) { return false; }
};

class VKernel {
 public:
  explicit VKernel(const VKernelOptions& opts, log::Scope* l = nullptr);

  void run(VKernelCB* cb);

  std::uint64_t tb_time() const { return tb_time_; }
  std::uint64_t tb_cycle() const;

 private:
  void build_verilated_environment();

  std::unique_ptr<Mdl> mdl_;
  std::unique_ptr<VerilatedContext> vctxt_;
  std::unique_ptr<Vtb> vtb_;
#ifdef ENABLE_VCD
  std::unique_ptr<VerilatedVcd> vcd_;
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
};

}  // namespace tb

/*
#include <optional>
#include <string>

#include "gtest/gtest.h"
#include "verilated.h"

#define ENABLE_VCD

class Vtb;

class VerilatedContext;
#ifdef ENABLE_VCD
class VerilatedVcdC;
#endif

namespace verif {

struct Options {
#ifdef ENABLE_VCD
  std::optional<std::string> vcd_filename;

  bool enable_vcd = false;
#endif
};

class UUTHarness {
 public:
  explicit UUTHarness(Vtb* tb);

  bool busy() const;

  bool in_reset() const;

  std::uint64_t tb_cycle() const;

 private:
  Vtb* tb_ = nullptr;
};

class Test {
 public:
  enum class Status {
    ApplyReset,
    RescindReset,
    Continue,
    Terminate
  };

  Test() {}

  virtual ~Test() {}

  // Called before the negative clock edge
  virtual Status on_negedge_clk(UpdateCommand& up, QueryCommand& qp) = 0;

  virtual bool is_passed() const { return true; }
};

class TB {
 public:
  explicit TB(const Options& opts);

  virtual ~TB();

  // Accessors:
  std::uint64_t tb_time() const { return tb_time_; }

  UUTHarness get_harness() const { return UUTHarness{uut_}; }

  void run(Test* s);

 private:

  void build_verilated_environment();

  // Current runtime options.
  Options opts_;

  // Unit Under Test
  Vtb* uut_ = nullptr;

  VerilatedContext* ctxt_ = nullptr;
#ifdef ENABLE_VCD
  VerilatedVcdC* vcd_ = nullptr;
#endif

  // Current Testbench simulation time.
  std::uint64_t tb_time_ = 0;
};

} // namespace verif
*/
#endif
