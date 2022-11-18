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

#include "log.h"

#include <sstream>
#include <string>
#include <algorithm>

#include "common.h"
#include "mdl.h"
#include "tb.h"

namespace tb::log {

template<>
void render_to_stream(std::ostream& os, const bool& b) {
  os << (b ? "1" : "0");
}

void StreamRenderer<bool>::write(std::ostream& os, const bool& b) {
  os << (b ? "1" : "0");
}

LoggerScope::LoggerScope(const std::string& name, Logger* logger, LoggerScope* parent)
  : name_(name), logger_(logger), parent_(parent) {
}

std::string LoggerScope::path() {
  if (!path_) {
    path_ = render_path();
  }
  return *path_;
}

void LoggerScope::append(const Message& message) {
  logger_->write(path() + message.to_string());
}

LoggerScope* LoggerScope::create_child(const std::string& scope_name) {
  children_.emplace_back(
    std::unique_ptr<LoggerScope>(new LoggerScope(scope_name, logger_, this)));
  return children_.back().get();
}

std::string LoggerScope::render_path() {
  std::vector<std::string> vs;
  vs.push_back(name_);
  LoggerScope* scope = parent_;
  while (scope != nullptr) {
    vs.push_back(scope->name());
    scope = scope->parent_;
  }
  std::reverse(vs.begin(), vs.end());
  std::string path;
  for (std::size_t i = 0; i < vs.size(); i++) {
    path += vs[i];
    if (i != (vs.size() - 1)) {
      path += scope_separator;
    }
  }
  return path;
}

Logger::Logger(std::ostream& os)
  : os_(os) {}

LoggerScope* Logger::scope() {
  if (!parent_scope_) {
    parent_scope_.reset(new LoggerScope("tb", this));
  }
  return parent_scope_.get();
}

void Logger::write(const std::string& s) {
  os_ << s << "\n";
}




















const char* to_string(bool b) { return b ? "true" : "false"; }

std::string Msg::str() const {
  //  std::stringstream ss;
  //  ss << fn() << ':' << ln() << ': ' << msg_;
  //  return ss.str();
  return msg_;
}

void Msg::pp(const std::string& f, unsigned l) {
  fn(f);
  ln(l);
}

void Msg::append(const std::string& s) { msg_ += s; }

void Msg::append(const UpdateCommand& uc) { msg_ += uc.to_string(); }

void Msg::append(const QueryCommand& qc) { msg_ += qc.to_string(); }

void Msg::append(const QueryResponse& qr) { msg_ += qr.to_string(); }

void Msg::append(const NotifyResponse& nr) { msg_ += nr.to_string(); }

Scope::Scope(Log* log) : log_(log), parent_(nullptr), sn_("tb") {}

Scope::Scope(Scope* parent, const std::string& sn) : parent_(parent), sn_(sn) {
  sn_ = parent->sn() + SEP + sn;
  log_ = parent->log();
}

Scope* Scope::create_child(const std::string& sn) {
  childs_.push_back(std::make_unique<Scope>(this, sn));
  return childs_.back().get();
}

void Scope::write(const Msg& msg) {
  if (log()) {
    log()->write(msg.str());
  }
  if (msg.l() == Level::Error) {
    throw VKernelException{"Error raised"};
  } else if (msg.l() == Level::Fatal) {
    throw VKernelException{"Fatal raised"};
  }
}

Log::Log(std::ostream& os) : os_(std::addressof(os)) {}

void Log::write(const std::string& s) {
  if (os_) *os_ << k_->tb_cycle() << ": " << s << "\n";
}

}  // namespace tb::log
