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

#include "mdl.h"

#include <array>
#include <sstream>
#include <vector>

#include "Vobj/Vtb.h"
#include "cfg.h"

namespace tb {

UpdateCommand::UpdateCommand() : vld_(false) {}

UpdateCommand::UpdateCommand(prod_id_t prod_id, Cmd cmd, key_t key,
                             volume_t volume)
    : vld_(true), prod_id_(prod_id), cmd_(cmd), key_(key), volume_(volume) {}

UpdateResponse::UpdateResponse() : vld_(false) {}

UpdateResponse::UpdateResponse(prod_id_t prod_id)
    : vld_(true), prod_id_(prod_id) {}

std::string UpdateResponse::to_string() const {
  std::stringstream ss;
  ss << (int)vld_ << " " << (int)prod_id_;
  return ss.str();
}

QueryCommand::QueryCommand() : vld_(false) {}

QueryCommand::QueryCommand(prod_id_t prod_id, level_t level)
    : vld_(true), prod_id_(prod_id), level_(level) {}

QueryResponse::QueryResponse() : vld_(false) {}

QueryResponse::QueryResponse(key_t key, volume_t volume, bool error,
                             listsize_t listsize) {
  vld_ = true;
  key_ = key;
  volume_ = volume;
  error_ = error;
  listsize_ = listsize;
}

NotifyResponse::NotifyResponse() : vld_(false) {}

NotifyResponse::NotifyResponse(prod_id_t prod_id, key_t key, volume_t volume) {
  vld_ = true;
  prod_id_ = prod_id;
  key_ = key;
  volume_ = volume;
}

std::string NotifyResponse::to_string() const {
  std::stringstream ss;
  ss << vld_ << " " << (int)prod_id_ << " " << key_ << " " << volume_;
  return ss.str();
}

struct VSampler {
  static UpdateCommand uc(Vtb* tb) {
    if (to_bool(tb->i_upd_vld)) {
      return UpdateCommand{tb->i_upd_prod_id, to_cmd(tb->i_upd_cmd),
                           tb->i_upd_key, tb->i_upd_size};
    } else {
      return UpdateCommand{};
    }
  }

  static QueryCommand qc(Vtb* tb) {
    if (to_bool(tb->i_lut_vld)) {
      return QueryCommand{tb->i_lut_prod_id, tb->i_lut_level};
    } else {
      return QueryCommand{};
    }
  }

  // Sample Notify Reponse Interface:
  static NotifyResponse nr(Vtb* tb) {
    if (to_bool(tb->o_lv0_vld_r)) {
      return NotifyResponse{tb->o_lv0_prod_id_r, tb->o_lv0_key_r,
                            tb->o_lv0_size_r};
    } else {
      return NotifyResponse{};
    }
  }

  // Sample Query Response Interface:
  static QueryResponse qr(Vtb* tb) {
    if (to_bool(tb->o_lut_vld_r)) {
      return QueryResponse{tb->o_lut_key, tb->o_lut_size,
                           to_bool(tb->o_lut_error), tb->o_lut_listsize};
    } else {
      return QueryResponse{};
    }
  }

 private:
  static bool to_bool(vluint8_t v) { return (v != 0); }

  static Cmd to_cmd(vluint8_t c) { return Cmd{c}; }
};

template <typename T, std::size_t N>
class DelayPipeBase {
 public:
  DelayPipeBase() { clear(); }

  std::string to_string() const {
    std::stringstream ss;
    for (std::size_t i = 0; i < p_.size(); i++) {
      const T& t{p_[(rd_ptr_ + i) % p_.size()]};
      ss << t.to_string() << "\n";
    }
    return ss.str();
  }

  void push_back(const T& t) { p_[wr_ptr_] = t; }

  const T& head() const { return p_[rd_ptr_]; }

  void step() {
    wr_ptr_ = (wr_ptr_ + 1) % p_.size();
    rd_ptr_ = (rd_ptr_ + 1) % p_.size();
  }

  void clear() {
    for (T& t : p_) t = T{};

    wr_ptr_ = N;
    rd_ptr_ = 0;
  }

 protected:
  std::size_t wr_ptr_, rd_ptr_;
  std::array<T, N + 1> p_;
};

template <typename T, std::size_t N>
class DelayPipe : public DelayPipeBase<T, N> {
  using base_class_type = DelayPipeBase<T, N>;
};

template <std::size_t N>
class DelayPipe<UpdateResponse, N> : public DelayPipeBase<UpdateResponse, N> {
  using base_class_type = DelayPipeBase<UpdateResponse, N>;

  using base_class_type::p_;
  using base_class_type::wr_ptr_;

 public:
  bool has_prod_id(prod_id_t prod_id) const {
    for (std::size_t i = 0; i < N; i++) {
      const UpdateResponse& ur{p_[(wr_ptr_ + i) % p_.size()]};
      if (ur.vld() && (ur.prod_id() == prod_id)) return true;
    }
    return false;
  }
};

struct Entry {
  std::string to_string() const {
    std::stringstream ss;
    ss << key << ", " << volume;
    return ss.str();
  }

  key_t key;
  volume_t volume;
};

bool compare_keys(key_t rhs, key_t lhs) {
  return cfg::is_bid_table ? (rhs < lhs) : (rhs > lhs);
}

bool compare_entries(const Entry& lhs, const Entry& rhs) {
  return compare_keys(lhs.key, rhs.key);
}

std::ostream& operator<<(std::ostream& os, const Entry& e) {
  return os << e.to_string();
}

class Mdl::Impl {
  static constexpr const std::size_t QUERY_PIPE_DELAY = 1;
  static constexpr const std::size_t UPDATE_PIPE_DELAY = 4;

 public:
  Impl(Vtb* tb) : tb_(tb) {}

  void step() {
    handle(VSampler::uc(tb_));
    handle(VSampler::qc(tb_));
    handle(VSampler::nr(tb_));
    handle(VSampler::qr(tb_));
  }

 private:
  void handle(const UpdateCommand& uc) {
    if (!uc.vld()) {
      // No command is present at the interface on this cycle, we do not
      // therefore expect a notification.
      nr_pipe_.push_back(NotifyResponse{});
      ur_pipe_.push_back(UpdateResponse{});
      return;
    };

    // Validate that ID provided by stimulus is within [0, cfg::CONTEXT_N).
    //    ASSERT_LT(uc.prod_id(), cfg::CONTEXT_N);

    UpdateResponse ur{};
    NotifyResponse nr{};
    std::vector<Entry>& ctxt{tbl_[uc.prod_id()]};
    switch (uc.cmd()) {
      case Cmd::Clr: {
        ur = UpdateResponse{uc.prod_id()};
        if (!ctxt.empty()) {
          nr = NotifyResponse{uc.prod_id(), 0, 0};
        }
        ctxt.clear();
      } break;
      case Cmd::Add: {
        ur = UpdateResponse{uc.prod_id()};
        if (ctxt.empty() || compare_keys(uc.key(), ctxt.begin()->key)) {
          nr = NotifyResponse{uc.prod_id(), uc.key(), uc.volume()};
        }
        ctxt.push_back(Entry{uc.key(), uc.volume()});
        std::stable_sort(ctxt.begin(), ctxt.end(), compare_entries);
        if (ctxt.size() > cfg::ENTRIES_N) {
          // Entry has been spilled on this Add.
          //
          // TODO: Raise some error notification to indicate that the context
          // has been truncated due to a capacity conflict and an entry has been
          // dropped.
          ctxt.pop_back();
        }
      } break;
      case Cmd::Rep:
      case Cmd::Del: {
        ur = UpdateResponse{uc.prod_id()};
        auto find_key = [&](const Entry& e) { return (e.key == uc.key()); };
        auto it = std::find_if(ctxt.begin(), ctxt.end(), find_key);

        // The context was either empty or the key was not found. The current
        // command becomes a NOP.
        if (it == ctxt.end()) break;

        if (it == ctxt.begin()) {
          // Item to be replaced is first, therefore raise notification of
          // current first item in context.
          nr = NotifyResponse{uc.prod_id(), it->key, it->volume};
        }

        if (uc.cmd() == Cmd::Rep) {
          // Perform final replacement of 'volume'.
          it->volume = uc.volume();
        } else {
          // Delete: Remove entry from context.
          ctxt.erase(it);
        }
      } break;
    }

    // Update predicted notify responses based upon outcome of prior command.
    ur_pipe_.push_back(ur);
    nr_pipe_.push_back(nr);
  }

  void handle(const NotifyResponse& nr) {
    const NotifyResponse& predicted = nr_pipe_.head();
    const NotifyResponse& actual = nr;
    //    EXPECT_EQ(predicted.vld(), actual.vld());
    if (predicted.vld()) {
      //      EXPECT_EQ(predicted.prod_id(), actual.prod_id());
      //      EXPECT_EQ(predicted.key(), actual.key());
      //      EXPECT_EQ(predicted.volume(), actual.volume());
    }
  }

  void handle(const QueryCommand& qc) {
    QueryResponse qr;
    if (qc.vld()) {
      //      ASSERT_LT(qc.prod_id(), cfg::CONTEXT_N);
      const std::vector<Entry>& ctxt{tbl_[qc.prod_id()]};

      if ((qc.level() >= ctxt.size()) || ur_pipe_.has_prod_id(qc.prod_id())) {
        // Query is errored, other fields are invalid.
        qr = QueryResponse{0, 0, true, 0};
      } else {
        // Query is valid, populate as necessary.
        const Entry& e{ctxt[qc.level()]};
        const listsize_t listsize = static_cast<listsize_t>(ctxt.size());
        qr = QueryResponse{e.key, e.volume, false, listsize};
      }
    }
    qr_pipe_.push_back(qr);
  }

  void handle(const QueryResponse& qr) {
    const QueryResponse& predicted = qr_pipe_.head();
    const QueryResponse& actual = qr;
    //    EXPECT_EQ(predicted.vld(), actual.vld());
    if (predicted.vld() && actual.vld()) {
      //      EXPECT_EQ(predicted.error(), actual.error());
      if (!predicted.error() && !actual.error()) {
        //        EXPECT_EQ(predicted.key(), actual.key());
        //        EXPECT_EQ(predicted.volume(), actual.volume());
        //        EXPECT_EQ(predicted.listsize(), actual.listsize());
      }
    }
  }

  std::array<std::vector<Entry>, cfg::CONTEXT_N> tbl_;
  DelayPipe<NotifyResponse, UPDATE_PIPE_DELAY> nr_pipe_;
  DelayPipe<UpdateResponse, UPDATE_PIPE_DELAY> ur_pipe_;
  DelayPipe<QueryResponse, QUERY_PIPE_DELAY> qr_pipe_;

  Vtb* tb_;
};

Mdl::Mdl(Vtb* tb) { impl_ = std::make_unique<Impl>(tb); }

Mdl::~Mdl() {}

void Mdl::step() { impl_->step(); }

}  // namespace tb
