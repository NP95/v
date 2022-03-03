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

#ifndef V_TB_MDL_H
#define V_TB_MDL_H

#include "verilated.h"

class Vtb;

namespace tb {

namespace log {
class Scope;
};

using prod_id_t = vluint8_t;
enum class Cmd : vluint8_t { Clr = 0, Add = 1, Del = 2, Rep = 3 };
using key_t = vluint64_t;
using volume_t = vluint32_t;
using level_t = vluint8_t;
using listsize_t = vluint8_t;

class UpdateCommand {
 public:
  UpdateCommand();
  UpdateCommand(prod_id_t prod_id, Cmd cmd, key_t key, volume_t volume);

  std::string to_string() const;

  bool vld() const { return vld_; }
  prod_id_t prod_id() const { return prod_id_; }
  Cmd cmd() const { return cmd_; }
  key_t key() const { return key_; }
  volume_t volume() const { return volume_; }

 private:
  bool vld_;
  prod_id_t prod_id_;
  Cmd cmd_;
  key_t key_;
  volume_t volume_;
};

bool operator==(const UpdateCommand& lhs, const UpdateCommand& rhs);

class UpdateResponse {
 public:
  UpdateResponse();
  UpdateResponse(prod_id_t prod_id);

  std::string to_string() const;

  bool vld() const { return vld_; }
  prod_id_t prod_id() const { return prod_id_; }

 private:
  bool vld_;
  prod_id_t prod_id_;
};

bool operator==(const UpdateResponse& lhs, const UpdateResponse& rhs);

class QueryCommand {
 public:
  QueryCommand();
  QueryCommand(prod_id_t prod_id, level_t level);

  std::string to_string() const;

  bool vld() const { return vld_; }
  prod_id_t prod_id() const { return prod_id_; }
  level_t level() const { return level_; }

 private:
  bool vld_;
  prod_id_t prod_id_;
  level_t level_;
};

bool operator==(const QueryCommand& lhs, const QueryCommand& rhs);

class QueryResponse {
 public:
  QueryResponse();
  QueryResponse(key_t key, volume_t volume, bool error, listsize_t listsize);

  std::string to_string() const;

  bool vld() const { return vld_; }
  key_t key() const { return key_; }
  volume_t volume() const { return volume_; }
  bool error() const { return error_; }
  listsize_t listsize() const { return listsize_; }

 private:
  bool vld_;
  key_t key_;
  volume_t volume_;
  bool error_;
  listsize_t listsize_;
};

bool operator==(const QueryResponse& lhs, const QueryResponse& rhs);

class NotifyResponse {
 public:
  NotifyResponse();
  NotifyResponse(prod_id_t prod_id, key_t key, volume_t volume);

  std::string to_string() const;

  bool vld() const { return vld_; }
  prod_id_t prod_id() const { return prod_id_; }
  key_t key() const { return key_; }
  volume_t volume() const { return volume_; }

 private:
  bool vld_;
  prod_id_t prod_id_;
  key_t key_;
  volume_t volume_;
};

bool operator==(const NotifyResponse& lhs, const NotifyResponse& rhs);

class Mdl {
  class Impl;
  std::unique_ptr<Impl> impl_;

 public:
  explicit Mdl(Vtb* tb, log::Scope* lg);
  ~Mdl();

  void step();
};

}  // namespace tb

#endif
