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

void StreamRenderer<bool>::write(std::ostream& os, const bool& b) {
  os << (b ? "1" : "0");
}

Scope::Scope(const std::string& name, Logger* logger, Scope* parent)
  : name_(name), logger_(logger), parent_(parent) {
}

std::string Scope::path() {
  if (!path_) {
    path_ = render_path();
  }
  return *path_;
}

void Scope::append(const Message& message) {
  logger_->write(path(), message);
}

Scope* Scope::create_child(const std::string& scope_name) {
  children_.emplace_back(
    std::unique_ptr<Scope>(new Scope(scope_name, logger_, this)));
  return children_.back().get();
}

std::string Scope::render_path() {
  std::vector<std::string> vs;
  vs.push_back(name_);
  Scope* scope = parent_;
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

Scope* Logger::scope() {
  if (!parent_scope_) {
    parent_scope_.reset(new Scope("tb", this));
  }
  return parent_scope_.get();
}

void Logger::write(const std::string& path, const Message& message) {
  os_ << path << ":" << message.to_string() << "\n";
//  Sim::log_event(message.level());
}

}  // namespace tb::log
