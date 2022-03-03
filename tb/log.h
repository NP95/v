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

#ifndef V_TB_LOG_H
#define V_TB_LOG_H

#include <iostream>
#include <vector>

#include "common.h"

#define V_ASSERT(__lg, __cond)                \
  MACRO_BEGIN                                 \
  if (__lg && !(__cond)) {                    \
    using namespace ::tb::log;                \
    Msg msg(Level::Fatal);                    \
    msg.set_pp(__FILE__, __LINE__);           \
    msg.append("Assertion failed: " #__cond); \
    (__lg)->write(msg);                       \
  }                                           \
  MACRO_END

#define V_EXPECT_EQ(__lg, __lhs, __rhs) \
  MACRO_BEGIN                           \
  if (__lg && !((__lhs) == (__rhs))) {  \
    using namespace ::tb::log;          \
    Msg msg(Level::Error);              \
    msg.set_pp(__FILE__, __LINE__);     \
    msg.append("Mismatch detected: ");  \
    msg.append(#__lhs " (");            \
    msg.append(__lhs);                  \
    msg.append(") != ");                \
    msg.append(#__rhs " (");            \
    msg.append(__rhs);                  \
    msg.append(")");                    \
    (__lg)->write(msg);                 \
  }                                     \
  MACRO_END

#define V_LOG(__lg, __level, __msg) \
  MACRO_BEGIN                       \
  if (__lg) {                       \
    using namespace ::tb::log;      \
    Msg msg(Level::__level);        \
    msg.append(__msg);              \
    (__lg)->write(msg);             \
  }                                 \
  MACRO_END

#define V_LOG_MSG(__lg, __msg) \
  MACRO_BEGIN                  \
  if (__lg) {                  \
    (__lg)->write(__msg);      \
  }                            \
  MACRO_END

// Forwards:
namespace tb {
class UpdateCommand;
class QueryCommand;
class QueryResponse;
class NotifyResponse;
class VKernel;
}  // namespace tb

namespace tb::log {

class Log;
class Scope;

enum class Level { Debug, Info, Warning, Error, Fatal };

class Msg {
 public:
  Msg() = default;
  Msg(Level l) : l_(l) {}

  void set_pp(const std::string& f, unsigned l) {
    fn(f);
    ln(l);
  }

  void fn(const std::string& fn) { fn_ = fn; }
  std::string fn() const { return fn_; }

  void ln(unsigned ln) { ln_ = ln; }
  unsigned ln() const { return ln_; }

  void append(const std::string& s);
  void append(bool b);
  void append(const UpdateCommand& uc);
  void append(const QueryCommand& qc);
  void append(const QueryResponse& qr);
  void append(const NotifyResponse& nr);

 private:
  std::string msg_;
  std::string fn_;
  unsigned ln_;
  Level l_{Level::Fatal};
};

class Scope {
  friend class Log;

  static constexpr const char SEP = '.';

 public:
  Scope(Log* log) : parent_(nullptr), sn_("tb") {}
  Scope(Scope* parent, const std::string& sn) : parent_(parent), sn_(sn) {
    sn_ = parent->sn() + SEP + sn;
  }

  Scope* create_child(const std::string& sn) {
    childs_.push_back(std::make_unique<Scope>(this, sn));
    return childs_.back().get();
  }

  void write(const Msg& msg);

  std::string sn() const { return sn_; }
  void sn(const std::string& sn) { sn_ = sn; }
  Log* log() const { return log_; }

 private:
  std::string sn_;
  Log* log_;
  std::vector<std::unique_ptr<Scope> > childs_;
  Scope* parent_;
};

class Log {
 public:
  explicit Log() = default;
  Log(std::ostream& os);

  void set_os(std::ostream& os) { os_ = std::addressof(os); }
  void set_kernel(VKernel* k) { k_ = k; }

  Scope* create_logger() {
    lgs_.push_back(std::make_unique<Scope>(this));
    return lgs_.back().get();
  }

 private:
  std::ostream* os_{nullptr};
  VKernel* k_{nullptr};
  std::vector<std::unique_ptr<Scope> > lgs_;
};

}  // namespace tb::log

#endif
