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

#include "test.h"

#include <iomanip>
#include "Vobj/Vtb.h"
#include "log.h"
#include "model.h"
#include "opts.h"
#include "tb.h"
#include "rnd.h"

namespace tb {

Test::Test() {}

Test::~Test() {}

JsonObject* JsonDict::clone() const {
  JsonDict *d = new JsonDict;
  for (auto& [k, v]: m_) {
    d->m_.insert(std::make_pair(k, v->clone()));
  }
  return d;
}

void JsonDict::serialize(std::ostream& os, std::size_t offset) const {
  os << "{";
  int i = 0;
  for (auto& [k, v] : m_) {
    if (i++ != 0) os << ", ";
    JsonString{k}.serialize(os);
    os << ":";
    v->serialize(os);
  }
  os << "}";
}

#define DECLARE_ADD(__type) \
void JsonDict::add(const std::string& k, const __type& t) { \
  m_.insert(std::make_pair(k, t.clone())); \
}
DECLARE_ADD(JsonString)
DECLARE_ADD(JsonInteger)
DECLARE_ADD(JsonArray)
DECLARE_ADD(JsonDict)
#undef DECLARE_ADD

JsonObject* JsonArray::clone() const {
  JsonArray* a = new JsonArray;
  for (const std::unique_ptr<JsonObject>& o : children_) {
    a->children_.emplace_back(o->clone());
  }
  return a;
}

void JsonArray::serialize(std::ostream& os, std::size_t offset) const {
  os << "[";
  for (std::size_t i = 0; i < children_.size(); i++) {
    if (i != 0) os << ", ";
    children_[i]->serialize(os);
  }
  os << "]";
}

#define DECLARE_ADD(__type) \
void JsonArray::add(const __type& t) { \
  children_.emplace_back(t.clone()); \
}
DECLARE_ADD(JsonString)
DECLARE_ADD(JsonInteger)
DECLARE_ADD(JsonArray)
DECLARE_ADD(JsonDict)
#undef DECLARE_ADD

JsonObject* JsonString::clone() const {
  return new JsonString(s_);
}

void JsonString::serialize(std::ostream& os, std::size_t offset) const {
  os << std::quoted(s_);
}

JsonObject* JsonInteger::clone() const {
  return new JsonInteger(i_);
}

void JsonInteger::serialize(std::ostream& os, std::size_t offset) const {
  os << i_;
}

void TestBuilder::build(Test* t, Scope* logger) const {
  t->logger_ = logger;
}

void TestRegistry::add(std::unique_ptr<TestBuilder> br) {
  r_.insert(std::make_pair(br->name(), std::move(br)));
}

std::vector<const TestBuilder*> TestRegistry::tests() const {
  std::vector<const TestBuilder*> tbrs;
  for (const auto& [name, tbr] : r_) {
    tbrs.push_back(tbr.get());
  }
  return tbrs;
}

const TestBuilder* TestRegistry::get(const std::string& name) const {
  if (auto it = r_.find(name); it != r_.end()) {
    return it->second.get();
  }
  return nullptr;
}

}  // namespace tb
