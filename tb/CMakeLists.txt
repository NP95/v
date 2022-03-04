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
# Environment:
include(rtl)
include(lib)

# ---------------------------------------------------------------------------- #
# TB flags:
set(has_vcd false)

set(BUILD_GMOCK OFF CACHE BOOL "Disable GMock")

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
  )

FetchContent_MakeAvailable(googletest)

set(TB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# Build verilator support library
verilator_build(vlib)

set(VERILATOR_OBJ "Vobj")

set(VERILATOR_ARGS
  "-cc"
  "-Wall"
  "--Mdir ${VERILATOR_OBJ}"
  "--build"
  "--top tb")

if (ENABLE_VCD)
  set(has_vcd true)
  list(APPEND VERILATOR_ARGS "--trace")
endif ()

set(TB_SOURCES
  "${RTL_CFG_SOURCES}"
  "${RTL_SOURCES}"
  "${LIB_SOURCES}"
  "${TB_ROOT}/tb.sv")

macro (set_tb_param k v)
  set(${k} ${v})
endmacro ()

macro (verilate_tb cfg)
  set(VERILATOR_INCLUDES "-I${CMAKE_CURRENT_BINARY_DIR}")
  foreach (inc_fn ${RTL_INCLUDE_PATHS})
    list(APPEND VERILATOR_INCLUDES "-I${inc_fn}")
  endforeach ()
  foreach (inc_fn ${LIB_INCLUDE_PATHS})
    list(APPEND VERILATOR_INCLUDES "-I${inc_fn}")
  endforeach ()

  set(VERILATOR_COMMAND_LIST
    "${VERILATOR_ARGS}"
    "${VERILATOR_INCLUDES}"
    "${TB_SOURCES}")

  string(REGEX REPLACE ";" "\n" FILELIST "${VERILATOR_COMMAND_LIST}")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/filelist "${FILELIST}")

  add_custom_target(verilate_${cfg}
    COMMAND ${CMAKE_COMMAND} -E env VERILATOR_ROOT=${VERILATOR_ROOT}
      ${Verilator_EXE} -f ${CMAKE_CURRENT_BINARY_DIR}/filelist
    COMMENT "Verilating (verilate_${cfg})...")

  set(VERILATOR_A "${CMAKE_CURRENT_BINARY_DIR}/${VERILATOR_OBJ}/Vtb__ALL.a")
endmacro ()

macro (build_test_cfg cfg src)
  verilate_tb(${cfg})

  set(target "${cfg}_test")
  add_executable(${target} ${TB_CPP})
  target_include_directories(${target} PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${Verilator_INCLUDE_DIR}")
  target_link_libraries(${target} gtest_main gtest pthread vlib ${VERILATOR_A})
  add_dependencies(${target} verilate_${cfg})

  add_test(NAME ${target} COMMAND ${target})
endmacro ()

set_tb_param("CONTEXT_N" 10)
set_tb_param("ENTRIES_N" 10)

configure_file("${TB_ROOT}/cfg.h.in" cfg.h)

foreach (src_fn "${RTL_GENERATED_SOURCES}")
  cmake_path(GET src_fn STEM LAST_ONLY dst_fn)
  configure_file(${src_fn} ${dst_fn})
endforeach ()

set(TB_CPP
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/smoke_cmds.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/directed.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/reset.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/regress.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/test.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/mdl.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/log.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/tb.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/driver.cc"
  )

build_test_cfg(cfg_0 ${TB_CPP})
