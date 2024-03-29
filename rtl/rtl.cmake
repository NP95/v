##========================================================================== //
## Copyright (c) 2022, Stephen Henry
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of source code must retain the above copyright notice, this
##   list of conditions and the following disclaimer.
##
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation
##   and/or other materials provided with the distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##========================================================================== //

# ---------------------------------------------------------------------------- #
# RTL Parameterizations

# The number of supported contexts
set(CONTEXT_N 10)

# The number of unique entries per context
set(ENTRIES_N 10)

# Allow duplicate keys within a given context.
declare_flag_option(ALLOW_DUPLICATES "Allow duplicate keys." ON)

# ---------------------------------------------------------------------------- #
# Sources
set(RTL_ROOT "${CMAKE_SOURCE_DIR}/rtl")

set(RTL_SOURCES
  "${RTL_ROOT}/common/mask.sv"
  "${RTL_ROOT}/common/lzd.sv"
  "${RTL_ROOT}/common/pri.sv"
  "${RTL_ROOT}/common/cla.sv"
  "${RTL_ROOT}/common/cmp.sv"
  "${RTL_ROOT}/common/dec.sv"
  "${RTL_ROOT}/common/mux.sv"
  "${RTL_ROOT}/v_pipe_update_cmp.sv"
  "${RTL_ROOT}/v_pipe_update_exe.sv"
  "${RTL_ROOT}/v_pipe_update.sv"
  "${RTL_ROOT}/v_pipe_query.sv"
  "${RTL_ROOT}/v_init.sv"
  "${RTL_ROOT}/v.sv"
  )

set(RTL_GENERATED_SOURCES
  "${RTL_ROOT}/cfg_pkg.vh.in"
  )

set(RTL_INCLUDE_PATHS
  "${RTL_ROOT}/common"
  "${RTL_ROOT}"
  )

set(RTL_CFG_SOURCES "${RTL_ROOT}/cfg.vlt")
