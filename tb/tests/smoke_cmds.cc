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

#include "smoke_cmds.h"

#include "../log.h"
#include "../test.h"
#include "cfg.h"
#include "directed.h"

namespace {

struct CheckAddCmd : public tb::tests::Directed {
  CREATE_TEST_BUILDER(CheckAddCmd);

  void program() override {
    // Wait until initialization sequence has completed.
    wait_until_not_busy();
    V_NOTE("Test begins...");

    // Issue simulus; populate prod_id/context in the table a validate correct
    // ordering.
    for (tb::key_t key = 0; key < cfg::ENTRIES_N + 1; key++) {
      const tb::volume_t volume = key;
      push_back(tb::UpdateCommand{0, tb::Cmd::Add, key + 1, volume});
      // Insert wait cycle to account for incomplete forwarding around
      // state-table.
      wait_cycles(1);
    }

    wait_cycles(10);

    // Verify value written is present in the machine and in the expected
    // order. No wait cycle here as this interface can sink a query command each
    // cycle.
    for (tb::level_t level = 0; level < cfg::ENTRIES_N; level++) {
      push_back(tb::QueryCommand{0, level});
    }

    V_NOTE("Test ends...");
  }
};

struct CheckDelCmd : public tb::tests::Directed {
  CREATE_TEST_BUILDER(CheckDelCmd);

  void program() override {
    // Wait until initialization sequence has completed.
    wait_until_not_busy();
    V_NOTE("Test begins...");

    // Issue simulus; populate prod_id/context in the table a validate correct
    // ordering.
    for (tb::key_t key = 0; key < cfg::ENTRIES_N; key++) {
      const tb::volume_t volume = key;
      push_back(tb::UpdateCommand{0, tb::Cmd::Add, key, volume});
      // Insert wait cycle to account for incomplete forwarding around
      // state-table.
      wait_cycles(1);
    }

    for (tb::key_t key = 0; key < cfg::ENTRIES_N; key++) {
      push_back(tb::UpdateCommand{0, tb::Cmd::Del, key, 0});
      wait_cycles(1);
    }
    V_NOTE("Test ends...");
  }
};

struct CheckListSize : public tb::tests::Directed {
  CREATE_TEST_BUILDER(CheckListSize);

  void program() override {
    // Wait until initialization sequence has completed.
    wait_until_not_busy();
    V_NOTE("Test begins...");

    // Check empty status;
    //
    // Expect to return invalid/error status when attempting to query the a
    // context/prod_id with no entries.
    push_back(tb::QueryCommand{0, 0});
    wait_cycles(10);

    // Check that invalid entries will raise an error message.
    //
    // Add one item, check that entry is present and that it is at the correct
    // location.
    push_back(tb::UpdateCommand{0, tb::Cmd::Add, 0, 0});
    wait_cycles(10);
    push_back(tb::QueryCommand{0, 0});
    // Expect error on next commands
    for (int i = 0; i < cfg::ENTRIES_N + 10; i++) {
      const tb::level_t level = i;
      push_back(tb::QueryCommand{0, level});
    }
    wait_cycles(10);

    // Check that an error message will be raised whenever a query is made to an
    // entry for which there is a pending update operation.

    // Ensure that entry contains valid item so that we're hitting the pipeline
    // is busy error conditino.
    push_back(tb::UpdateCommand{0, tb::Cmd::Add, 0, 0});
    wait_cycles(10);
    push_back(tb::UpdateCommand{0, tb::Cmd::Add, 0, 0}, tb::QueryCommand{0, 0});
    wait_cycles(10);

    V_NOTE("Test ends...");
  }
};

struct CheckClrCmd : public tb::tests::Directed {
  CREATE_TEST_BUILDER(CheckClrCmd);

  void program() override {
    // Wait until initialization sequence has completed.
    wait_until_not_busy();
    V_NOTE("Test begins...");

    // Issue simulus; populate prod_id/context in the table a validate correct
    // ordering.
    for (tb::key_t key = 0; key < cfg::ENTRIES_N; key++) {
      const tb::volume_t volume = key;
      push_back(tb::UpdateCommand{0, tb::Cmd::Add, key + 1, volume});
      // Insert wait cycle to account for incomplete forwarding around
      // state-table.
      wait_cycles(1);
    }

    push_back(tb::UpdateCommand{0, tb::Cmd::Clr, 0, 0});
    wait_cycles(10);

    // Tby value written is present in the machine and in the expected
    // order. No wait cycle here as this interface can sink a query command each
    // cycle.
    for (tb::level_t level = 0; level < cfg::ENTRIES_N; level++) {
      push_back(tb::QueryCommand{0, level});
    }

    // Clear again; should already be empty at this point.
    //
    push_back(tb::UpdateCommand{0, tb::Cmd::Clr, 0, 0});
    wait_cycles(10);

    // Verify value written is present in the machine and in the expected
    // order. No wait cycle here as this interface can sink a query command each
    // cycle.
    for (tb::level_t level = 0; level < cfg::ENTRIES_N; level++) {
      push_back(tb::QueryCommand{0, level});
    }

    V_NOTE("Test ends...");
  }
};

struct CheckRplCmd : public tb::tests::Directed {
  CREATE_TEST_BUILDER(CheckRplCmd);

  void program() override {
    // Wait until initialization sequence has completed.
    wait_until_not_busy();
    V_NOTE("Test begins...");

    push_back(tb::UpdateCommand{0, tb::Cmd::Rep, 0, 0});
    wait_cycles(1);

    // Issue simulus; populate prod_id/context in the table a validate correct
    // ordering.
    for (tb::key_t key = 0; key < cfg::ENTRIES_N; key++) {
      const tb::volume_t volume = key;
      push_back(tb::UpdateCommand{0, tb::Cmd::Add, key, volume});
      // Insert wait cycle to account for incomplete forwarding around
      // state-table.
      wait_cycles(1);
    }

    // Replace key that we don't expect to find (non-empty case).
    push_back(tb::UpdateCommand{0, tb::Cmd::Rep, cfg::ENTRIES_N, 1});
    wait_cycles(1);

    // Replace key that we DO expect to find.
    push_back(tb::UpdateCommand{0, tb::Cmd::Rep, 0, 1});
    wait_cycles(1);

    // Validate final state.
    for (tb::level_t level = 0; level < cfg::ENTRIES_N; level++) {
      push_back(tb::QueryCommand{0, level});
    }

    V_NOTE("Test ends...");
  }
};

}  // namespace

namespace tb::tests::smoke_cmds {

void init(tb::TestRegistry* r) {
  CheckAddCmd::Builder::init(r);
  CheckDelCmd::Builder::init(r);
  CheckListSize::Builder::init(r);
  CheckClrCmd::Builder::init(r);
  CheckRplCmd::Builder::init(r);
}

}  // namespace tb::tests::smoke_cmds
