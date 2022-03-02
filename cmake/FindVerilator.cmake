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

if (DEFINED VERILATOR_ROOT)

  # Explicit Verilator installation at VERILATOR_ROOT

  # Expand tilde if present in command line argument. If present, this
  # can break the verilator scripts.
  file(REAL_PATH ${VERILATOR_ROOT} VERILATOR_ROOT EXPAND_TILDE)

  find_path(Verilator_INCLUDE_DIR verilated.h
    HINTS ${VERILATOR_ROOT}/include
    DOC "Searching for Verilator installation."
    )

  find_path(VerilatorDpi_INCLUDE_DIR svdpi.h
    HINTS ${VERILATOR_ROOT}/include/vltstd
    DOC "Searching for Verilator installation."
    )

  find_program(Verilator_EXE
    verilator
    HINTS ${VERILATOR_ROOT}/bin
    DOC "Searching for Verilator executable."
    )

else ()

  # Otherwise, search for system installation of Verilator

  find_path(Verilator_INCLUDE_DIR verilated.h
    PATH_SUFFIXES include
    HINTS /usr/share/verilator/include
    HINTS /opt/verilator/latest/share/verilator/include
    DOC "Searching for Verilator installation."
    )

  find_path(VerilatorDpi_INCLUDE_DIR svdpi.h
    PATH_SUFFIXES include
    HINTS /usr/share/verilator/include/vltstd
    HINTS /opt/verilator/latest/share/verilator/include/vltstd
    DOC "Searching for Verilator installation."
    )

  find_program(Verilator_EXE
    verilator
    HINTS /usr/bin/verilator
    HINTS /opt/verilator/latest/bin
    DOC "Searching for Verilator executable."
    )

  if (NOT DEFINED ENV{VERILATOR_ROOT})
    message(FATAL_ERROR
      "Environment does not define VERILATOR_ROOT, which is "
      "required by the Verilator installation.")
  endif()

endif()

if (Verilator_EXE)
  execute_process(COMMAND ${Verilator_EXE} "--version"
    OUTPUT_VARIABLE v_version)
  string(REGEX REPLACE "Verilator ([0-9]).([0-9]+).*" "\\1"
    VERILATOR_MAJOR_VERSION ${v_version})
  string(REGEX REPLACE "Verilator ([0-9]).([0-9]+).*" "\\2"
    VERILATOR_MINOR_VERSION ${v_version})
  set(VERILATOR_VERSION
    ${VERILATOR_MAJOR_VERSION}.${VERILATOR_MINOR_VERSION})

  message(STATUS "Found Verilator version: ${VERILATOR_VERSION}")

  macro (verilator_build vlib)
    set(Verilator_SRCS
      "${Verilator_INCLUDE_DIR}/verilated.cpp"
      "${Verilator_INCLUDE_DIR}/verilated_dpi.cpp"
      "${Verilator_INCLUDE_DIR}/verilated_save.cpp")
    if (ENABLE_VCD)
      list(APPEND Verilator_SRCS
        "${Verilator_INCLUDE_DIR}/verilated_vcd_c.cpp")
    endif ()

    add_library(${vlib} SHARED "${Verilator_SRCS}")
    list(APPEND Verilator_INCLUDE_DIR "${VerilatorDpi_INCLUDE_DIR}")
    target_include_directories(${vlib} PUBLIC "${Verilator_INCLUDE_DIR}")
  endmacro ()

else()
  message(WARNING "Verilator not found! Simulation is not supported")
endif ()
