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

#define MACRO_BEGIN do {
#define MACRO_END \
  }               \
  while (false)

#define V_ASSERT(__lg, __cond)                               \
  MACRO_BEGIN                                                \
  if (__lg && !(__cond)) {                                   \
    (__lg)->log(::tb::log::Level::Fatal, "condition fail!"); \
  }                                                          \
  MACRO_END

namespace tb::log {

class Log;
class Scope;

enum class Level { Debug, Info, Warning, Error, Fatal };

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

  void log(Level l, const char* msg) {}

  std::string sn() const { return sn_; }
  void sn(const std::string& sn) { sn_ = sn; }

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

  Scope* create_logger() {
    lgs_.push_back(std::make_unique<Scope>(this));
    return lgs_.back().get();
  }

 private:
  std::ostream* os_{nullptr};
  std::vector<std::unique_ptr<Scope> > lgs_;
};

}  // namespace tb::log

#endif
