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

class Kernel;
class Scope;

#define CREATE_TEST_BUILDER(__name)                       \
  struct Builder : ::tb::TestBuilder {                    \
    static void init(::tb::TestRegistry& tr) {            \
      tr.add(std::make_unique<Builder>());                \
    }                                                     \
    std::string name() const override { return #__name; } \
    ::tb::JsonDict args() const override {                \
      ::tb::JsonDict d;                                   \
      d.add("name", name());                              \
      return d;                                           \
    }                                                     \
    std::unique_ptr<::tb::Test> construct(                \
        ::tb::Scope* logger) const override {             \
      auto t = std::make_unique<__name>();                \
      build(t.get(), logger);                             \
      return std::move(t);                                \
    }                                                     \
  }

#define CREATE_TEST_BUILDER_WITH_ARGS(__name, __args)     \
  struct Builder : ::tb::TestBuilder {                    \
    static void init(::tb::TestRegistry& tr) {            \
      tr.add(std::make_unique<Builder>());                \
    }                                                     \
    std::string name() const override { return #__name; } \
    ::tb::JsonDict args() const override {                \
      ::tb::JsonDict d{__name::args()};                   \
      d.add("name", name());                              \
      return d;                                           \
    }                                                     \
    std::unique_ptr<::tb::Test> construct(                \
        ::tb::Scope* logger) const override {             \
      auto t = std::make_unique<__name>();                \
      build(t.get(), logger);                             \
      return std::move(t);                                \
    }                                                     \
  }

class JsonArray;
class JsonInteger;
class JsonString;

class JsonObject {
protected:
  enum class Type { Object, String, Integer, Array, Dict };
  virtual Type type() const { return Type::Object; }
public:
  explicit JsonObject() = default;
  virtual ~JsonObject() = default;

  virtual JsonObject* clone() const = 0;

  virtual void serialize(std::ostream& os, std::size_t offset = 0) const = 0;

private:
};

class JsonDict : public JsonObject {
  Type type() const override { return Type::Dict; }
public:
  explicit JsonDict() = default;

  JsonObject* clone() const override;
  void serialize(std::ostream& os, std::size_t offset = 0) const;

  void add(const std::string& k, const JsonString& s);
  void add(const std::string& k, const JsonInteger& i);
  void add(const std::string& k, const JsonArray& a);
  void add(const std::string& k, const JsonDict& d);

private:
  std::map<std::string, std::unique_ptr<JsonObject>> m_;
};

class JsonArray : public JsonObject {
  Type type() const override { return Type::Array; }
public:
  explicit JsonArray() = default;

  JsonObject* clone() const override;
  void serialize(std::ostream& os, std::size_t offset = 0) const;

  void add(const JsonString& s);
  void add(const JsonInteger& i);
  void add(const JsonArray& a);
  void add(const JsonDict& d);

private:
  std::vector<std::unique_ptr<JsonObject>> children_;
};

class JsonString : public JsonObject {
  Type type() const override { return Type::String; }
public:
  /* no explicit */ JsonString(const std::string& s) : s_(s) {}
  /* no explicit */ JsonString(const char* s) : s_(s) {}

  JsonObject* clone() const override;
  void serialize(std::ostream& os, std::size_t offset = 0) const;
private:
  std::string s_;
};

class JsonInteger : public JsonObject {
  Type type() const override { return Type::Integer; }
public:
  /* no explicit */ JsonInteger(int i) : i_(i) {}

  JsonObject* clone() const override;
  void serialize(std::ostream& os, std::size_t offset = 0) const;
private:
  int i_;
};

class Test {
  friend class TestBuilder;

 public:
  explicit Test();
  virtual ~Test();

  Scope* logger() const { return logger_; }

  virtual bool run() = 0;

 private:
  Scope* logger_{nullptr};
};

class TestBuilder {
 public:
  explicit TestBuilder() = default;
  virtual ~TestBuilder() = default;

  virtual std::string name() const = 0;
  virtual std::unique_ptr<Test> construct(::tb::Scope*) const = 0;
  virtual JsonDict args() const = 0;

 protected:
  void build(Test* t, Scope* logger) const;
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
