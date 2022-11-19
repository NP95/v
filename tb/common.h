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

#ifndef V_TB_COMMON_H
#define V_TB_COMMON_H

#include <string>

namespace tb {

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

} // namespace tb

// Macro helpers:
#define MACRO_BEGIN do {
#define MACRO_END \
  }               \
  while (false)

struct RecordBuilder {
  static constexpr const char LPAREN = '{';
  static constexpr const char RPAREN = '}';

  RecordBuilder(std::string& s) : s_(s) { s_ += LPAREN; }

  ~RecordBuilder() { s_ += RPAREN; }

  template <typename T>
  void append(const char* k, T&& v) {
    s_ += k;
    s_ += ':';
    s_ += to_string(v);
  }

 private:
  std::string& s_;
};

#endif
