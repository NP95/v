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

#include "common.h"

#define V_ASSERT(__lg, __cond)                \
  MACRO_BEGIN                                 \
  if (__lg && !(__cond)) {                    \
    using namespace ::tb::log;                \
    Msg msg(Level::Fatal);                    \
    msg.pp(__FILE__, __LINE__);               \
    msg.append("Assertion failed: " #__cond); \
    (__lg)->write(msg);                       \
  }                                           \
  MACRO_END

#define V_EXPECT_EQ(__lg, __lhs, __rhs)               \
  MACRO_BEGIN                                         \
  if (__lg && !((__lhs) == (__rhs))) {                \
    using namespace ::tb::log;                        \
    Msg msg(Level::Error);                            \
    msg.pp(__FILE__, __LINE__);                       \
    msg.trace_mismatch(#__lhs, __lhs, #__rhs, __rhs); \
    (__lg)->write(msg);                               \
  }                                                   \
  MACRO_END

#define V_EXPECT_TRUE(__lg, __cond)             \
  MACRO_BEGIN                                   \
  if (__lg && !(__cond)) {                      \
    using namespace ::tb::log;                  \
    Msg msg(Level::Error);                      \
    msg.pp(__FILE__, __LINE__);                 \
    msg.append("Condition is false: " #__cond); \
    (__lg)->write(msg);                         \
  }                                             \
  MACRO_END

#define V_LOG(__lg, __level, __msg) \
  MACRO_BEGIN                       \
  if (__lg) {                       \
    using namespace ::tb::log;      \
    Msg msg(Level::__level);        \
    msg.pp(__FILE__, __LINE__);     \
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
class Logger;

#define LOG_LEVELS(__func)  \
  __func(Debug)\
  __func(Info)\
  __func(Warning)\
  __func(Error)\
  __func(Fatal)

enum class Level {
#define __declare_level(__level) __level,
  LOG_LEVELS(__declare_level)
#undef __declare_level
};


template<typename T>
struct StreamRenderer {
  static void write(std::ostream& os, const T& t) { os << t; }
};

template<>
struct StreamRenderer<bool> {
  static void write(std::ostream& os, const bool& t);
};

class RecordRenderer {
public:
  explicit RecordRenderer(std::ostream& os, const std::string& type = "unk")
    : os_(os) {
    os_ << type << "{";
  }

  ~RecordRenderer() {
    if (!finalized_) finalize();
  }

  template<typename T>
  void add(const std::string& k, const T& t) {
    if (entries_n_++) os_ << ", ";
    os_ << k;
    os_ << ":";
    StreamRenderer::write(os_, t);
  }

private:
  void finalize() {
    os_ << "}";
    finalized_ = true;
  }
  //!
  bool finalized_{false};
  //! Number of previously rendered key/value pairs.
  std::size_t entries_n_{0};
  //! Output stream.
  std::ostream& os_;
};

class Message {
  friend class LoggerScope;
public:
#define __declare_message_builder(__level) \
  template<typename ...T> \
  static Message __level(T&& ...ts) { \
    return Build(Level::__level, std::forward<T>(ts)...); \
  }
  LOG_LEVELS(__declare_message_builder)
#undef __declare_message_builder

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
     (StreamRenderer<T>::write(ss_, std::forward<T>(ts)), ...);
  }

  Level level() const { return level_; }
  std::string to_string() const { return ss_.str(); }

  std::stringstream ss_;

  Level level_;
};


class LoggerScope {
  friend class Logger;

  static constexpr const char* scope_separator = ".";

  explicit LoggerScope(
    const std::string& name, Logger* logger, LoggerScope* parent = nullptr);
public:
  //! Current scope name
  std::string name() { return name_; }
  //! Current scope path
  std::string path();

  void append(const Message& m);

  LoggerScope* create_child(const std::string& scope_name);

private:
  //!
  std::string render_path();

  //! Name of current scope.
  std::string name_;
  //! Pointer to parent scope (nullptr if root scope)
  LoggerScope* parent_{nullptr};
  //! Owning pointer to child logger scopes.
  std::vector<std::unique_ptr<LoggerScope>> children_;
  //! Path of scope within logger hierarchy.
  std::optional<std::string> path_;
  //!
  Logger* logger_{nullptr};
};

class Logger {
  friend class LoggerScope;
public:
  explicit Logger(std::ostream& os);

  LoggerScope* scope();
private:
  //! Write composed message to logger output stream.
  void write(const std::string& s);
  //!
  std::unique_ptr<LoggerScope> parent_scope_;
  //! Output logging stream.
  std::ostream& os_;
};

const char* to_string(bool b);



class Msg {
public:




public:
  explicit Msg() = default;
  explicit Msg(Level l) : l_(l) {}

  std::string str() const;

  void pp(const std::string& f, unsigned l);
  void fn(const std::string& fn) { fn_ = fn; }
  std::string fn() const { return fn_; }
  Level l() const { return l_; }

  void ln(unsigned ln) { ln_ = ln; }
  unsigned ln() const { return ln_; }




  template <typename T>
  void trace_mismatch(const char* lhs_s, T lhs, const char* rhs_s, T rhs) {
    append("Mismatch detected: ");
    append(lhs_s);
    append(" (");
    append(lhs);
    append(") != ");
    append(rhs_s);
    append(" (");
    append(rhs);
    append(")");
  }

  void trace_mismatch(const char* lhs_s, bool lhs, const char* rhs_s,
                      bool rhs) {
    trace_mismatch(lhs_s, to_string(lhs), rhs_s, to_string(rhs));
  }

  // Specializations:
  void append(const std::string& s);
  void append(const UpdateCommand& uc);
  void append(const QueryCommand& qc);
  void append(const QueryResponse& qr);
  void append(const NotifyResponse& nr);

  std::string msg_;
  std::string fn_;
  unsigned ln_;
  Level l_{Level::Fatal};
};

class Scope {
  friend class Log;

  static constexpr const char SEP = '.';

 public:
  Scope(Log* log);
  Scope(Scope* parent, const std::string& sn);

  Scope* create_child(const std::string& sn);
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
  friend class Scope;

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
  void write(const std::string& s);

  std::ostream* os_{nullptr};
  VKernel* k_{nullptr};
  std::vector<std::unique_ptr<Scope> > lgs_;
};

}  // namespace tb::log

#endif
