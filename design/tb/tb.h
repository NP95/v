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

#ifndef V_DESIGN_VERIF_TB_H
#define V_DESIGN_VERIF_TB_H

#include "gtest/gtest.h"
#include "verilated.h"
#include <optional>
#include <string>

#define ENABLE_VCD

class Vtb;

class VerilatedContext;
#ifdef ENABLE_VCD
class VerilatedVcdC;
#endif

namespace verif {

using prod_id_t = vluint8_t;
enum class Cmd : vluint8_t { Clr = 0, Add = 1, Del = 2, Rep = 3 };
using key_t = vluint64_t;
using volume_t = vluint32_t;
using level_t = vluint8_t;
using listsize_t = vluint8_t;

class UpdateCommand {
 public:
  UpdateCommand();

  UpdateCommand(prod_id_t prod_id, Cmd cmd, key_t key, volume_t volume);

  bool vld() const { return vld_; }
  prod_id_t prod_id() const { return prod_id_; }
  Cmd cmd() const { return cmd_; }
  key_t key() const { return key_; }
  volume_t volume() const { return volume_; }

 private:
  bool vld_;
  prod_id_t prod_id_;
  Cmd cmd_;
  key_t key_;
  volume_t volume_;
};

class UpdateResponse {
 public:
  UpdateResponse();
  UpdateResponse(prod_id_t prod_id);

  bool vld() const { return vld_; }
  prod_id_t prod_id() const { return prod_id_; }

 private:
  bool vld_;
  prod_id_t prod_id_;
};

class QueryCommand {
 public:
  QueryCommand();

  QueryCommand(prod_id_t prod_id, level_t level);

  bool vld() const { return vld_; }
  prod_id_t prod_id() const { return prod_id_; }
  level_t level() const { return level_; }

 private:
  bool vld_;
  prod_id_t prod_id_;
  level_t level_;
};

class QueryResponse {
 public:
  QueryResponse();
  QueryResponse(key_t key, volume_t volume, bool error, listsize_t listsize);

  bool vld() const { return vld_; }
  key_t key() const { return key_; }
  volume_t volume() const { return volume_; }
  bool error() const { return error_; }
  listsize_t listsize() const { return listsize_; }

 private:
  bool vld_;
  key_t key_;
  volume_t volume_;
  bool error_;
  listsize_t listsize_;
};

class NotifyResponse {
 public:
  NotifyResponse();
  NotifyResponse(prod_id_t prod_id, key_t key, volume_t volume);

  std::string to_string() const;

  bool vld() const { return vld_; }
  prod_id_t prod_id() const { return prod_id_; }
  key_t key() const { return key_; }
  volume_t volume() const { return volume_; }

 private:
  bool vld_;
  prod_id_t prod_id_;
  key_t key_;
  volume_t volume_;
};

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

#endif
