#
# Copyright 2019 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

source settings.tcl

set PROJ "prj"
set SOLN "solution1"
set CLKP 3.33

open_project -reset $PROJ

add_files -tb tb.cpp -cflags "-I${XF_PROJ_ROOT}/L1/include/xf_fintech -I${XF_PROJ_ROOT}/L1/include/"
add_files dut.cpp -cflags "-I${XF_PROJ_ROOT}/L1/include/xf_fintech -I${XF_PROJ_ROOT}/L1/include/"

set_top dut

open_solution -reset $SOLN

set_part $XPART
create_clock -period $CLKP -name default
config_export -format ip_catalog -rtl verilog
set_clock_uncertainty 0.9

if {$CSIM == 1} {
  csim_design -O -stdmath
}

if {$CSYNTH == 1} {
  csynth_design
}

if {$COSIM == 1} {
  cosim_design
}

if {$VIVADO_SYN == 1} {
  export_design -flow syn -rtl verilog
}

if {$VIVADO_IMPL == 1} {
  export_design -flow impl -rtl verilog
}

if {$QOR_CHECK == 1} {
  puts "QoR check not implemented yet"
}

exit
