##========================================================================== //
## Copyright (c) 2020, Stephen Henry
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

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/filelist "-cc\n")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/filelist "--Mdir Vobj\n")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/filelist "--trace\n")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/filelist "--top tb\n")

file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/filelist "-I${CMAKE_SOURCE_DIR}/design/rtl\n")
string(REGEX REPLACE ";" "\n" FILELIST "${TB_SRCS}")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/filelist "${FILELIST}")

add_executable(cfg_0 cfg_0.cc)
target_include_directories(cfg_0 PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(cfg_0 gtest_main gtest pthread)
add_test(NAME cfg_0 COMMAND cfg_0)

add_custom_target(verilate_cfg_0
  COMMAND ${Verilator_EXE} -f ${CMAKE_CURRENT_BINARY_DIR}/filelist
  COMMENT "Verilating..."
  )
