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

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        703bd9caab50b139428cea1aaff9974ebee5742e # release-1.10.0  
  )

FetchContent_MakeAvailable(googletest)

list(APPEND TB_SRCS
  "${RTL_SOURCES}"
  "${CMAKE_CURRENT_SOURCE_DIR}/tb.sv")

set(Verilator_SRCS
  "${Verilator_INCLUDE_DIR}/verilated.cpp"
  "${Verilator_INCLUDE_DIR}/verilated_dpi.cpp"
  "${Verilator_INCLUDE_DIR}/verilated_save.cpp"
  "${Verilator_INCLUDE_DIR}/verilated_vcd_c.cpp"
  )
set(Verilator_INCLUDE_DIR
  "${Verilator_INCLUDE_DIR}"
  "${Verilator_INCLUDE_DIR}/vltstd"
  )
add_library(verilated SHARED ${Verilator_SRCS})
target_include_directories(verilated PUBLIC ${Verilator_INCLUDE_DIR})

set(VERILATOR_ARGS
  "-cc"
  "--Mdir Vobj"
  "--trace"
  "--build"
  "--top tb")

if (ENABLE_TRACE)
  list(APPEND VERILATOR_ARGS "--trace")
endif ()

set(VERILATOR_INCLUDES
  "-I${CMAKE_SOURCE_DIR}/design/rtl")

set(VERILATOR_SOURCES
  "${RTL_SOURCES}"
  "${CMAKE_CURRENT_SOURCE_DIR}/tb.sv")

set(VERILATOR_COMMAND_LIST
  "${VERILATOR_ARGS}"
  "${VERILATOR_INCLUDES}"
  "${VERILATOR_SOURCES}")

add_subdirectory(cfg_0)
