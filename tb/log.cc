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

#include "mdl.h"

namespace tb::log {

void Msg::pp(const std::string& f, unsigned l) {
  fn(f);
  ln(l);
}

void Msg::append(const std::string& s) { msg_ += s; }

void Msg::append(bool b) { msg_ += (b ? "true" : "false"); }

void Msg::append(const UpdateCommand& uc) { msg_ += uc.to_string(); }

void Msg::append(const QueryCommand& qc) { msg_ += qc.to_string(); }

void Msg::append(const QueryResponse& qr) { msg_ += qr.to_string(); }

void Msg::append(const NotifyResponse& nr) { msg_ += nr.to_string(); }

Scope::Scope(Log* log) : parent_(nullptr), sn_("tb") {}

Scope::Scope(Scope* parent, const std::string& sn) : parent_(parent), sn_(sn) {
  sn_ = parent->sn() + SEP + sn;
}

Scope* Scope::create_child(const std::string& sn) {
  childs_.push_back(std::make_unique<Scope>(this, sn));
  return childs_.back().get();
}

void Scope::write(const Msg& msg) {}

Log::Log(std::ostream& os) : os_(std::addressof(os)) {}

}  // namespace tb::log
