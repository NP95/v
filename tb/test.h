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

#ifndef V_VERIF_TEST_H
#define V_VERIF_TEST_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "opts.h"

namespace tb {

class VKernel;

namespace log {
class Scope;
}

#define CREATE_TEST_BUILDER(__name)                       \
  struct Builder : ::tb::TestBuilder {                    \
    static void init(::tb::TestRegistry* tr) {            \
      tr->add(std::make_unique<Builder>());               \
    }                                                     \
    std::string name() const override { return #__name; } \
    std::unique_ptr<::tb::Test> construct(                \
        const ::tb::TestOptions& opts) const override {   \
      auto t = std::make_unique<__name>();                \
      build(t.get(), opts);                               \
      return std::move(t);                                \
    }                                                     \
  }

class Test {
  friend class TestBuilder;

 public:
  explicit Test() = default;
  virtual ~Test() = default;

  log::Scope* lg() const { return opts_.l; }
  const TestOptions& opts() const { return opts_; }
  VKernel* k() const { return k_.get(); }

  virtual bool run() = 0;

 private:
  TestOptions opts_;
  std::unique_ptr<VKernel> k_;
};

class TestBuilder {
 public:
  explicit TestBuilder() = default;
  virtual ~TestBuilder() = default;

  virtual std::string name() const = 0;
  virtual std::unique_ptr<Test> construct(
      const TestOptions& opts = TestOptions{}) const = 0;

 protected:
  void build(Test* t, const TestOptions& opts) const;
};

class TestRegistry {
  std::map<std::string, std::unique_ptr<TestBuilder>> r_;

 public:
  explicit TestRegistry() = default;

  void add(std::unique_ptr<TestBuilder> br);

  std::vector<const TestBuilder*> tests() const;

  const TestBuilder* get(const std::string& name) const;
};

}  // namespace tb

#endif
