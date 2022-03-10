# Introduction

"v" is a solution to an interview problem set (some years ago) by a trading
company in Manhattan. The problem statement was as follows:

Maintain a table of 'N' Contexts, where each Context maintains some number 'M'
of 2-tuples. Each 2-tuple (Entry) consists of a 64b key and a 32b value (the
'volume'). An 'Update' interface is present to which the following commands are
issued:

- Add; which inserts a new Entry into a given Context.
- Delete; which deletes a Entry by key in a given Context.
- Replace; which changes the value (volume) of a Entry given a key in a given
  Context.
- Clear; which removes all Entry from a given Context.

The 'Update' interface must accept a command on each cycle, but is constrained
to such that back-to-back commands to the same Context will not occur.

A second 'Lookup' interface is also present. This second interface, for a given
Context, returns the n-th largest/smallest Entry in the Context (ordered on
key), where 'n' is an operand to the command. The interface must accept a
command on each cycle and should respond with only 1-cycle latency. The
interface is not constrained on Context, therefore back-to-back commands to the
same Context are permissible.

The block shall emit a 'Notify' message each time the largest/smallest tuple in
a Context is modified by the Update interface. The latency after this is emitted
in response to the initial Update command is not constrained.

The solution was to be targeted towards an FPGA-context and was to be clocked at
a clock frequency of at least 175 MHz.

# Build Instructions

A basic smoke test which runs 1M randomized commands can be built and run using
the following commands:

```shell
git clone git@github.com:stephenry/v.git
pushd m
mkdir build
pushd build
# Configuration with pointer to local build of verilator at path 'verilator path'
cmake .. -DVERILATOR_ROOT='verilator path'
# Or, configuration using system verilator installation. This will attempt to find
# a pre-existing installation of Verilator on the host system. The explicit approach
# is preferred as a recent version of Verilator is required (>= 4.210).
cmake ..
make smoke
```

The RTL is fully parameterized on Context count (CONTEXT_N) and Entry count
(ENTRIES_N). Parameterization is performed during configuration as follows:

```shell
cmake .. -DVERILATOR_ROOT='verilator path' -DCONTEXT_N=5, -DENTRIES_N=4
```

which will generate RTL (and verification collateral) for a machine with 5
Contexts, each containing update 4 Entries.

# Dependencies

* A fairly recent version of Verilator (>= 4.210), specifically a version
  including the recent (~2020) VerilatedContext improvements. Ideally, project
  configuration should be performed against a local build of Verilator (pointing
  towards the current HEAD of main) that has been locally built
  in-place. Otherwise, an attempt will be made to find the system installation
  which may not succeed if not present at known paths on the file-system, or
  when using an older version.
* A C++17 compliant compiler (developed on Clang 12).
* A recent version of CMake (>= 3.20)

# Discussion

* The problem describes the basic behaviour of a low-latency order book, where
  incoming orders are tabulated based upon their bid-/ask- price (in this case,
  represented as some opaque 64b signed quantity).
* The problem requires two concurrent accesses to Context state; a
  read-modify-write style update for the Update interface, and a read-only
  access for the Lookup interface. This state can be implemented using either
  Block RAM (BRAM) or in flops. Flops allows state to be accessed using some
  arbitrary number of read/write ports, but is costly from a utilization
  perspective. BRAM offer improved utilization and performance, but suffer from
  a limitation on port count (upto 1r1w). In this solution, table state was
  maintained using two BRAM banks; the first for the read-modify-write operation
  on the Update pipeline, and the second for the read-only operation in the
  Lookup pipeline. Writebacks to the table were broadcast to both banks such
  that both maintained a coherent view of the machines state.
* The Lookup pipeline required the ability to query the 'n'-th largest/smallest
  entry in a given Context and to do so on a cycle-by-cycle basis with only one
  cycle of latency. This very restrictive latency requirement essentially
  ruled-out any ability to process state at the output of the Lookup BRAM other
  than to simply mux. out the desired Entry. Therefore, an implicit requirement
  of this problem was that state should be ordered by the time it had been
  inserted into the table.
* The [Update pipeline](./rtl/v_pipe_update.sv) was responsible for updates to
  the state table. A bitonic-sorting network is the canonical approach used to
  order elements in hardware, but is unsuitable in this challenge as it requires
  some number of cycles to compute. A key simplifying observation to this
  problem is that the state table need not be sorted on each cycle, only that it
  be maintained in sorted order. The table itself is initially empty and on each
  cycle a new element may be added or removed. It is therefore only necessary to
  compute the appropriate location into/from which this operation is to take
  place. For this task, the [Execution Unit](./rtl/v_pipe_update_exe.sv) used
  the Insertion Sort algorithm.  In this approach, the sorting process can be
  computed entirely combinatorially over one clock cycle and is largely
  invariant to the number of elements to be sorted (with some small dependency
  on prioritization logic proportional to the number of elements in the
  Context).
* The Update pipeline need not support updates to the same Context on
  back-to-back cycles. This relaxation allows the read-modify-write operation to
  be pipelined across two cycles if necessary to attain the desired clock
  frequency. In this solution, the update is performed across only one cycle as
  this should already be sufficient. The Update pipeline however implements only
  partial forwarding of state, such that all forwarded state can be derived from
  the output of flops.
* The [Query pipeline](./rtl/v_pipe_query.sv) consisted of two stages: a zeroth
  stage to simply stage the lookup into the BRAM macro, and a output stage to
  mux out the selected Entry from the state. Some additional error logic is
  present to compute error cases such as to error out whenever an in-flight
  access is made to the same Context in the update pipeline, or when an attempt
  is made to select an invalid Entry from the table (all conditions specified in
  the original problem statement).
* The latency through the Query pipeline is one-cycle (the time taken to lookup
  the BRAM). In practice this is unrealistic as although the data becomes
  available at this point, often it arrives quite late into the cycle for it to
  be useful to downstream logic. In practice, if this design was to be part of a
  larger subsystem, it would be necessary to flop the Update response interface
  at the output before its value could be used elsewhere. This is outside of the
  scope however of our current objective.
* The presented solution has been written in a more
  [ASIC-y](./rtl/common/cmp.sv) fashion than what would typically be found on an
  FPGA platform. In this style, I have paid more attention to X-propagation and
  physical concerns (implicit flop instantiation), than would have otherwise
  been typical in an FPGA setting. This is purely stylistic and has little
  meaningful difference on an FPGA.
* The problem statement did not discuss expected behaviour under error
  conditions; when, for example, back-to-back Update commands are issued to the
  same Context, or when an attempt is made to address a Context which is not
  present (Context 14, on a 10 Context machine where the Context itself is
  represented as a 4b digit). These conditions have not been exhaustively
  explored due to time constraints.
* Verification of the solution is carried out using Verilator and C++17. Random
  stimulus is presented to the RTL and compared against a C++-based behavioural
  [model](./tb/mdl.cc). The simulation is failed if a mismatch is observed
  between the models. A [Driver](./tb/driver.cc) utility is also provided to
  execute interesting testcases.
* Configuration and build management is carried out using
  [CMake](./tb/CMakeLists.txt).
