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
#include <memory>
#include <sstream>
#include <optional>
#include "verilated.h"

#include "common.h"

#define V_ASSERT(__lg, __cond)

#define V_EXPECT_EQ(__lg, __lhs, __rhs)


#define V_EXPECT_TRUE(__lg, __cond)


#define V_LOG(__lg, __level, __msg)


#define V_LOG_MSG(__lg, __msg)


namespace tb::log {

class Msg {};

class Logger;

template<typename T>
struct AsHex {
  explicit AsHex(T t) : t(t) {}
  T t;
};

template<typename T>
struct StreamRenderer;

template<>
struct StreamRenderer<bool> {
  static void write(std::ostream& os, const bool& t);
};

template<>
struct StreamRenderer<const char*> {
  static void write(std::ostream& os, const char* msg);
};

#define VERILATOR_TYPES(__func) \
  __func(vlsint64_t) \
  __func(vluint32_t) \
  __func(vluint8_t)

#define DECLARE_HANDLER(__type) \
template<> \
struct StreamRenderer<__type> { \
  static void write(std::ostream& os, const __type& t) { \
    os << t; \
  } \
};
VERILATOR_TYPES(DECLARE_HANDLER)
#undef DECLARE_HANDLER

class RecordRenderer {
  static constexpr const char* LPAREN = "{";
  static constexpr const char* RPAREN = "}";
public:
  explicit RecordRenderer(std::ostream& os, const std::string& type = "unk")
    : os_(os) {
    os_ << type << LPAREN;
  }

  ~RecordRenderer() {
    if (!finalized_) finalize();
  }

  template<typename T>
  void add(const std::string& k, const T& t) {
    if (finalized_) return;
    preamble(k);
    writekey(t);
  }

private:
  void preamble(const std::string& k) {
    if (entries_n_++) os_ << ", ";
    os_ << k;
    os_ << ":";    
  }

  template<typename T>
  void writekey(const AsHex<T>& h) {
    os_ << std::hex << "0x" << h.t;
  }
  
  template<typename T>
  void writekey(T& t) {
    StreamRenderer<std::decay_t<T>>::write(os_, t);
  }

  void finalize() {
    os_ << RPAREN;
    finalized_ = true;
  }
  //! Record has been serialized.
  bool finalized_{false};
  //! Number of previously rendered key/value pairs.
  std::size_t entries_n_{0};
  //! Output stream.
  std::ostream& os_;
};

class Message {
  friend class Scope;
public:
#define __declare_message_builder(__level) \
  template<typename ...T> \
  static Message __level(T&& ...ts) { \
    return Build(Level::__level, std::forward<T>(ts)...); \
  }
  LOG_LEVELS(__declare_message_builder)
#undef __declare_message_builder

  Level level() const { return level_; }
  
  void to(std::string& s) const { s = ss_.str(); }
  void to(std::ostream& os) const { os << ss_.rdbuf(); }

private:
  template<typename ...T>
  static Message Build(Level level, T&& ...ts) {
    Message message{level};
    message.append(std::forward<T>(ts)...);
    return message;
  }

  explicit Message(Level level)
    : level_(level) {}

  template<typename ...T>
  void append(T&& ...ts) { 
    (StreamRenderer<std::decay_t<T>>::write(ss_, std::forward<T>(ts)), ...);
  }
  //! Current accumulated message.
  std::stringstream ss_;
  Level level_;
};


class Scope {
  friend class Logger;

  static constexpr const char* scope_separator = ".";

  explicit Scope(
    const std::string& name, Logger* logger, Scope* parent = nullptr);
public:
  //! Current scope name
  std::string name() { return name_; }
  //! Current scope path
  std::string path() const;

#define __declare_message(__level) \
  template<typename ...Ts> \
  void __level(Ts&& ...ts) const { \
    write(Level::__level, std::forward<Ts>(ts)...); \
  }
  LOG_LEVELS(__declare_message)
#undef __declare_message

  Scope* create_child(const std::string& scope_name);

private:
  template<typename ...Ts>
  void write(Level l, Ts&& ...ts) const;

  //!
  std::string render_path() const;
  //! Name of current scope.
  std::string name_;
  //! Pointer to parent scope (nullptr if root scope)
  Scope* parent_{nullptr};
  //! Owning pointer to child logger scopes.
  std::vector<std::unique_ptr<Scope>> children_;
  //! Path of scope within logger hierarchy.
  mutable std::optional<std::string> path_;
  //! Parent Logger instance.
  Logger* logger_{nullptr};
};

class Logger {
  friend class Scope;
  friend class Context;
public:
  class Context {
    friend class Logger;

    static constexpr const char* PATH_LPAREN = "{";
    static constexpr const char* PATH_RPAREN = "}";
    static constexpr const char* PATH_COLON = ":";

    explicit Context(const Scope* s, Logger* logger)
     : s_(s), logger_(logger)
    {}
  public:
    void write(const Message& message) const {
      std::ostream& os{logger_->os()};
      os << PATH_LPAREN << s_->path() << PATH_RPAREN << PATH_COLON << " ";
      message.to(os);
    }

  private:
    const Scope* s_;
    Logger* logger_;
  };

  explicit Logger(std::ostream& os);

  Scope* top();

  Context create_context(const Scope* s) { return Context{s, this}; }

private:
  //! 
  std::ostream& os() const { return os_; }
  //!
  std::unique_ptr<Scope> parent_scope_;
  //! Output logging stream.
  std::ostream& os_;
};

template<typename ...Ts>
void Scope::write(Level l, Ts&& ...ts) const {
  auto context{logger_->create_context(this)};
  context.write(Message::Build(l, std::forward<Ts>(ts)...));
}

}  // namespace tb::log

#endif
