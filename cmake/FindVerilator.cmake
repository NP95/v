## ==================================================================== ##
## Copyright (c) 2022, Stephen Henry
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
##
## * Redistributions of source code must retain the above copyright
##   notice, this list of conditions and the following disclaimer.
##
## * Redistributions in binary form must reproduce the above copyright
##   notice, this list of conditions and the following disclaimer in
##   the documentation and/or other materials provided with the
##   distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
## "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
## LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
## FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
## COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
## INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
## (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
## SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
## HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
## STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
## OF THE POSSIBILITY OF SUCH DAMAGE.
## ==================================================================== ##

find_program(Verilator_EXE verilator
  DOC "Searching for Verilator executable...")

if (Verilator_EXE)
  execute_process(COMMAND ${Verilator_EXE} --version
    OUTPUT_VARIABLE v_version)
  string(REGEX REPLACE "Verilator ([0-9]).([0-9]+).*" "\\1"
    VERILATOR_MAJOR_VERSION ${v_version})
  string(REGEX REPLACE "Verilator ([0-9]).([0-9]+).*" "\\2"
    VERILATOR_MINOR_VERSION ${v_version})
  set(VERILATOR_VERSION
    ${VERILATOR_MAJOR_VERSION}.${VERILATOR_MINOR_VERSION})
  message(STATUS "Found Verilator version: ${VERILATOR_VERSION}")

  execute_process(COMMAND
    ${Verilator_EXE} --getenv VERILATOR_ROOT
    OUTPUT_VARIABLE VERILATOR_ROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  message(STATUS "VERILATOR_ROOT set as ${VERILATOR_ROOT}")

  macro (verilator_build vlib)
    add_library(${vlib} STATIC
      "${VERILATOR_ROOT}/include/verilated.cpp"
      "${VERILATOR_ROOT}/include/verilated_dpi.cpp"
      "${VERILATOR_ROOT}/include/verilated_save.cpp"
      "$<$<BOOL:${ENABLE_VCD}>:${VERILATOR_ROOT}/include/verilated_vcd_c.cpp>"
      )
    target_include_directories(${vlib}
      PRIVATE
      "${VERILATOR_ROOT}/include"
      "${VERILATOR_ROOT}/include/vltstd"
      )
  endmacro ()

else()
  message(WARNING "Verilator not found! Simulation is not supported")
endif ()
