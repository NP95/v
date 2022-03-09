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
#include "tb.h"
#include "test.h"

namespace {

class Driver {
 public:
  Driver(int argc, char** argv) {
    status_ = 0;
    argc_ = argc;
    argv_ = argv;
    tb::init(&tr_);
  }

  int status() const { return status_; }

  void execute() {
    topts_.vcd_on = true;
    for (int i = 1; i < argc_; ++i) {
      const std::string_view argstr{argv_[i]};
      if (argstr == "--help" || argstr == "-h") {
        print_usage();
        status_ = 1;
        return;
      } else if (argstr == "-v") {
        log_.set_os(std::cout);
      } else if (argstr == "--list") {
        for (const tb::TestBuilder* tb : tr_.tests()) {
          std::cout << tb->name() << "\n";
        }
        status_ = 1;
        return;
      } else if (argstr == "--vcd") {
#ifdef ENABLE_VCD
        topts_.vcd_on = true;
#else
        // VCD support has not been compiled into driver. Fail
        std::cout
            << "Waveform tracing has not been enabled in current build.\n";
        status_ = 1;
#endif
      } else if (argstr == "--run") {
        const std::string targs{argv_[++i]};
        if (!run(targs)) {
          status_ = 1;
          return;
        }
      } else {
        std::cout << "Unknown argument: " << argstr << "\n";
        status_ = 1;
        return;
      }
    }
  }

 private:
  bool run(const std::string& targs) {
    if (const tb::TestBuilder* tb = tr_.get(targs); tb != nullptr) {
      return run(tb);
    } else {
      return false;
    }
  }

  bool run(const tb::TestBuilder* tb) {
    topts_.l = log_.create_logger();
    std::unique_ptr<tb::Test> t{tb->construct(topts_)};
    return t->run();
  }

  void print_usage() {
    std::cout << " -h|--help         Print help and quit.\n"
              << " -v                Verbose\n"
              << " --list            List testcases\n"
#ifndef ENABLE_VCD
              << " --vcd             Enable waveform tracing (VCD)\n"
#endif
              << " --run <args>      Run testcase\n"
              << " --run_all         Run all testcases\n";
  }

  int argc_;
  char** argv_;
  tb::TestRegistry tr_;
  int status_;
  tb::log::Log log_;
  tb::TestOptions topts_;
};

}  // namespace

int main(int argc, char** argv) {
  Driver drv{argc, argv};
  drv.execute();
  return drv.status();
}
