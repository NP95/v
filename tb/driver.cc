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
#include <string_view>

#include "log.h"
#include "rnd.h"
#include "tb.h"
#include "test.h"

namespace {

class Driver {
 public:
  explicit Driver() {
    status_ = 0;
    tb::init(&tr_);
  }

  int status() const { return status_; }

  void execute(int argc, char** argv) {
    bool got_testname = false;
    tb::TestOptions topts;
    tb::log::Log log;
    topts.l = log.create_logger();
    tb::Rnd rnd;
    topts.rnd = std::addressof(rnd);
    for (int i = 1; i < argc; ++i) {
      const std::string_view argstr{argv[i]};
      if (argstr == "--help" || argstr == "-h") {
        print_usage();
        status_ = 1;
        return;
      } else if (argstr == "-v") {
        log.set_os(std::cout);
      } else if (argstr == "-s" || argstr == "--seed") {
        const std::string sstr{argv[++i]};
        std::size_t pos = 0;
        rnd.seed(std::stoi(sstr, &pos));
      } else if (argstr == "--list") {
        for (const tb::TestBuilder* tb : tr_.tests()) {
          std::cout << tb->name() << "\n";
        }
        status_ = 1;
        return;
      } else if (argstr == "--vcd") {
#ifdef ENABLE_VCD
        topts.vcd_on = true;
#else
        // VCD support has not been compiled into driver. Fail
        std::cout
            << "Waveform tracing has not been enabled in current build.\n";
        status_ = 1;
#endif
      } else if (argstr == "--run") {
        topts.test_name = argv[++i];
        got_testname = true;
      } else if (argstr == "--args") {
        topts.args = argv[++i];
      } else {
        std::cout << "Unknown argument: " << argstr << "\n";
        status_ = 1;
        return;
      }
    }

    // Try to run test.
    if (!got_testname || !run(topts)) {
      status_ = 1;
      return;
    }
  }

 private:
  bool run(const tb::TestOptions& opts) {
    if (const tb::TestBuilder* tb = tr_.get(opts.test_name); tb != nullptr) {
      return run(opts, tb);
    } else {
      return false;
    }
  }

  bool run(const tb::TestOptions& opts, const tb::TestBuilder* tb) {
    std::unique_ptr<tb::Test> t{tb->construct(opts)};
    return t->run();
  }

  void print_usage() {
    std::cout << " -h|--help         Print help and quit.\n"
              << " -v                Verbose\n"
              << " -s|--seed         Randomization seed.\n"
              << " --list            List testcases\n"
#ifndef ENABLE_VCD
              << " --vcd             Enable waveform tracing (VCD)\n"
#endif
              << " --run <test>      Run testcase\n"
              << " --args <args>     Testcase arguments.\n";
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
