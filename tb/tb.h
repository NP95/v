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
#include <optional>
#include <vector>

#include "cfg.h"
#include "opts.h"
#include "rnd.h"

// Verilator artifacts
class Vtb;
class VerilatedVcdC;
class VerilatedContext;

namespace tb {

class TestRegistry;
class Model;
class UpdateCommand;
class QueryCommand;
class Kernel;
class Logger;
class Scope;

void register_tests(TestRegistry& tr);

struct Sim {
  static void initialize();

  inline static std::optional<std::string> test_name;

  inline static std::vector<std::string> test_args;

#ifdef ENABLE_VCD
  inline static bool vcd_on = false;

  inline static std::string vcd_fn = "v.vcd";
#endif

  //! Global logger
  inline static std::unique_ptr<Logger> logger;

  //! Global randomization state.
  inline static std::unique_ptr<Random> random;

  //! Global simulation kernel.
  inline static std::unique_ptr<Kernel> kernel;

  //! Global validation model.
  inline static std::unique_ptr<Model> model;

  //! Pass indication status (final traced line)
  inline static std::string_view pass_note = "PASS!\n";

  //! Fail indirection status (final traced line)
  inline static std::string_view fail_note = "FAIL!\n";

  inline static int errors = 0;

  inline static int warnings = 0;

  //! Total number of errors encountered before the simulations is terminated.
  inline static int error_max = 1;
};

struct KernelCallbacks {
  virtual ~KernelCallbacks() = default;

  virtual bool on_negedge_clk(Vtb* tb) { return true; }

  virtual bool on_posedge_clk(Vtb* tb) { return true; }
};

class KernelException {
 public:
  KernelException(const char* msg) : msg_(msg) {}
  const char* msg() const { return msg_; }

 private:
  const char* msg_;
};

class Kernel {
 public:
  explicit Kernel();
  ~Kernel();

  bool run(KernelCallbacks* cb);
  void end();

  std::uint64_t tb_time() const { return tb_time_; }
  std::uint64_t tb_cycle() const;

 private:
  bool eval_clock_edge(KernelCallbacks* cb, bool edge);
#ifdef ENABLE_VCD
  std::unique_ptr<VerilatedVcdC> vcd_;
#endif
  std::unique_ptr<VerilatedContext> vctxt_;
  std::unique_ptr<Vtb> vtb_;
  std::uint64_t tb_time_;
  Scope* logger_{nullptr};
};

struct VDriver {
  //
  static void issue(Vtb* tb, const UpdateCommand& uc);

  //
  static void issue(Vtb* tb, const QueryCommand& qc);

  //
  static bool is_busy(Vtb* tb);

  //
  static void reset(Vtb* tb, bool r);
};

}  // namespace tb

#endif
