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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string_view>

#include "log.h"
#include "model.h"
#include "rnd.h"
#include "tb.h"
#include "test.h"

namespace {

template <typename... Ts>
bool is_one_of(std::string_view in, Ts&&... ts) {
  for (std::string_view opt : {ts...}) {
    if (in == opt) return true;
  }
  return false;
}

class Driver {
  enum class ArgResult : int { Bad, Good, Exit };
 public:
  explicit Driver() = default;

  int run(int argc, char** argv);
  int status() const { return status_; }

 private:
  void init();
  ArgResult parse_args(int argc, char** argv);
  void finalize();
  void execute();
  void print_usage(std::ostream& os) const;
  void print_tests(std::ostream& os, bool as_json = false) const;
  int report(bool failed = false) const;

  tb::TestRegistry tr_;
  int status_ = 0;
  std::unique_ptr<std::ofstream> ofs_;
};

void Driver::init() {
  tb::Sim::random = std::make_unique<tb::Random>();
  tb::register_tests(tr_); 
}

int Driver::run(int argc, char** argv) {
  bool failed = false;
  try {
    init();
    switch (parse_args(argc, argv)) {
      case ArgResult::Bad:
        failed = true;
        break;
      case ArgResult::Good:
        finalize();
        execute();
        break;
      case ArgResult::Exit:
        std::exit(1);
        break;
    }
  } catch (std::exception& ex) {
    std::cout << "Driver execution failed with:" << ex.what() << "!\n";
    failed = true;
  }
  return report(failed);
}

auto Driver::parse_args(int argc, char** argv) -> ArgResult {
  bool as_json = false;
  std::vector<std::string_view> vs{argv, argv + argc};
  for (int i = 1; i < vs.size(); ++i) {
    const std::string_view argstr{vs.at(i)};
    if (is_one_of(argstr, "-h", "--help")) {
      // -h|--help: Print help information
      print_usage(std::cout);
      return ArgResult::Exit;
    } else if (is_one_of(argstr, "-v", "--verbose")) {
      // -v|--vebose: Enable verbose tracing.
      tb::Sim::logger = std::make_unique<tb::Logger>();
    } else if (is_one_of(argstr, "-f", "--file")) {
      // -f|--file: Trace to file.
      ofs_ = std::make_unique<std::ofstream>(std::filesystem::path(vs.at(++i)));
      tb::Sim::logger->set_os(ofs_.get());
    } else if (is_one_of(argstr, "-s", "--seed")) {
      // -s|--seed: Randomization seed (integer)
      const std::string sstr{vs.at(++i)};
      tb::Sim::random->seed(std::stoi(sstr));
    } else if (is_one_of(argstr, "-j", "--json")) {
      as_json = true;
    } else if (is_one_of(argstr, "--list")) {
      // --list: List set of currently registered tests.
      print_tests(std::cout, as_json);
      std::exit(0);
    } else if (is_one_of(argstr, "--vcd")) {
      // --vcd: emit VCD of simulation.
#ifdef ENABLE_VCD
      tb::Sim::vcd_on = true;
#else
      // VCD support has not been compiled into driver. Fail
      std::cout
          << "Waveform tracing has not been enabled in current build.\n";
      status_ = 1;
      return ArgResult::Bad;
#endif
    } else if (is_one_of(argstr, "--run")) {
      // -r|--run: Testname to run.
      tb::Sim::test_name = vs.at(++i);
    } else if (is_one_of(argstr, "-e", "--errors")) {
      // -e|--errors: Tolerated error count (integer)
      const std::string sstr{vs.at(++i)};
      tb::Sim::error_max = std::stoi(sstr);
    } else if (is_one_of(argstr, "-a", "--args")) {
      // -a|--args: Arguments passed to test.
      tb::Sim::test_args.emplace_back(vs.at(++i));
    } else {
      std::cout << "Unknown argument: " << argstr << "\n";
      print_usage(std::cout);
      return ArgResult::Exit;
    }
  }
  return ArgResult::Good;
}

void Driver::finalize() {
  tb::Sim::kernel = std::make_unique<tb::Kernel>();
}

void Driver::execute() {
  if (!tb::Sim::test_name) {
    std::cout << "No testname provided!\n";
    print_usage(std::cout);
    status_ = 1;
  } else if (const tb::TestBuilder* tb = tr_.get(*tb::Sim::test_name);
             tb != nullptr) {
    tb::Scope* test_scope{nullptr};
    if (tb::Sim::logger) {
      test_scope = tb::Sim::logger->top()->create_child("test");
    }
    std::unique_ptr<tb::Test> t{tb->construct(test_scope)};
    status_ = t->run() ? 1 : 0;
  } else {
    std::cout << "Unknown test: " << *tb::Sim::test_name << "\n";
    status_ = 1;
  }
}

void Driver::print_usage(std::ostream& os) const {
  os << "Usage is:\n"
     << "   -h|--help         Print help and quit.\n"
     << "   -v                Verbose\n"
     << "   -f|--file         Trace to file\n"
     << "   -s|--seed         Randomization seed.\n"
     << "   -j|--json         Testcases listed as JSON (for --list option)\n"
     << "   --list            List testcases and quit\n"
#ifndef ENABLE_VCD
     << "   --vcd             Enable waveform tracing (VCD)\n"
#endif
     << "   --run <test>      Run testcase\n"
     << "   -e|--errors <arg> Tolerated error count\n"
     << "   -a|--args <arg>   Append testcase argument\n";
}

void Driver::print_tests(std::ostream& os, bool as_json) const {
  if (as_json) {
    // Render as JSON.
    tb::JsonArray a;
    for (const tb::TestBuilder* tb : tr_.tests()) {
      a.add(tb->args());
    }
    a.serialize(os);
    os << "\n";
  } else {
    // Otherwise, simply echo the registered test names.
    for (const tb::TestBuilder* tb : tr_.tests()) {
      os << tb->name() << "\n";
    }
  }
}

int Driver::report(bool failed) const {
  std::string thick_row(80, '=');
  std::string thin_row(80, '-');
  tb::Sim::logger->write(thick_row);
  tb::Sim::logger->write("Simulation terminates: ");
  tb::Sim::logger->write(thin_row);
  tb::Sim::logger->write("   Error(s)   - ", tb::Sim::errors);
  tb::Sim::logger->write("   Warning(s) - ", tb::Sim::warnings);
  tb::Sim::logger->write(thick_row);

  int issue_n = failed ? 1 : 0;
  issue_n += tb::Sim::errors;
  issue_n += tb::Sim::warnings;

  tb::Sim::logger->write(issue_n ? tb::Sim::fail_note : tb::Sim::pass_note);
  return issue_n;
}

}  // namespace

int main(int argc, char** argv) { return Driver{}.run(argc, argv); }
