# Introduce

"v" is a solution to an interview problem for an FPGA role in Manhattan.

```shell
git clone git@github.com:stephenry/v.git
pushd m
mkdir build
pushd build
# Configuration with pointer to local build of verilator at path 'verilator path'
cmake .. -DVERILATOR_ROOT='verilator path'
# Or, configuration using system verilator installation.
cmake ..
make run_regress
```

# Dependencies

# Discussion

*
