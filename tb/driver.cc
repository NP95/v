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
// ========================================================================== //

#include <iostream>
#include <fstream>
#include <string_view>
#include <memory>

#include "log.h"
#include "rnd.h"
#include "tb.h"
#include "test.h"
#include "sim.h"
#include "mdl.h"

namespace {

class Driver {
 public:
  explicit Driver() {
    status_ = 0;
    tb::init(&tr_);
  }

  int status() const { return status_; }

  void execute(int argc, char** argv) {
    std::ostream& msgos{std::cout};
    std::unique_ptr<std::ofstream> ofs;
    for (int i = 1; i < argc; ++i) {
      const std::string_view argstr{argv[i]};
      if (argstr == "-h" || argstr == "--help") {
        // -h|--help: Print help information
        print_usage(msgos);
        status_ = 1;
        return;
      } else if (argstr == "-v" || argstr == "--verbose") {
        // -v|--vebose: Enable verbose tracing.
        tb::Sim::log = std::make_unique<tb::log::Logger>(msgos);
      } else if (argstr == "-f" || argstr == "--file") {
        // -f|--file: Trace to file.
        ofs = std::make_unique<std::ofstream>(argv[++i]);
        tb::Sim::log = std::make_unique<tb::log::Logger>(*ofs);
      } else if (argstr == "-s" || argstr == "--seed") {
        // -s|--seed: Randomization seed (integer)
        const std::string sstr{argv[++i]};
        std::size_t pos = 0;
        tb::Sim::rnd->seed(std::stoi(sstr, &pos));
      } else if (argstr == "--list") {
        // --list: List set of currently registered tests.
        print_tests(msgos);
        status_ = 1;
        return;
      } else if (argstr == "--vcd") {
        // --vcd: emit VCD of simulation.
#ifdef ENABLE_VCD
        tb::Sim::vcd_on = true;
#else
        // VCD support has not been compiled into driver. Fail
        msgos
            << "Waveform tracing has not been enabled in current build.\n";
        status_ = 1;
        return;
#endif
      } else if (argstr == "--run") {
        // -r|--run: Testname to run.
        tb::Sim::test_name = argv[++i];
      } else if (argstr == "-a" || argstr == "--args") {
        // -a|--args: Arguments passed to test.
        tb::Sim::test_args.push_back(argv[++i]);
      } else {
        msgos << "Unknown argument: " << argstr << "\n";
        print_usage(msgos);
        status_ = 1;
        return;
      }
    }

    if (!tb::Sim::test_name) {
      msgos << "No testname provided!\n";
      print_usage(msgos);
      status_ = 1;
    } else if (const tb::TestBuilder* tb = tr_.get(*tb::Sim::test_name); tb != nullptr) {
      tb::log::Scope* test_scope{nullptr};
      if (tb::Sim::log) {
        test_scope = tb::Sim::log->top()->create_child("test");
      }
      std::unique_ptr<tb::Test> t{tb->construct(test_scope)};
      status_ = t->run() ? 1 : 0;
    } else {
      msgos << "Unknown test: " << *tb::Sim::test_name << "\n";
      status_ = 1;
    }
  }

 private:
  void print_usage(std::ostream& os) {
    os  << " -h|--help         Print help and quit.\n"
        << " -v                Verbose\n"
        << " -f|--file         Trace to file\n"
        << " -s|--seed         Randomization seed.\n"
        << " --list            List testcases\n"
#ifndef ENABLE_VCD
        << " --vcd             Enable waveform tracing (VCD)\n"
#endif
        << " --run <test>      Run testcase\n"
        << " -a|--args <arg>   Append testcase argument\n";
  }
  void print_tests(std::ostream& os) {
    os << "Available tests:\n";
    for (const tb::TestBuilder* tb : tr_.tests()) {
      os << tb->name() << "\n";
    }
  }

  tb::TestRegistry tr_;
  int status_;
};

}  // namespace

int main(int argc, char** argv) {
  Driver drv{};
  drv.execute(argc, argv);
  return drv.status();
}
