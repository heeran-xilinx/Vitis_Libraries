..
   Copyright 2019 Xilinx, Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.


*************************
Internal Design of Quanto
*************************

Overview
========

The Quanto solution is based on the Black Scholes Merton model, where the BSM dividend yield parameter is replaced with a calculation involving various quanto parameters and the resulting BSM call price multiplied by another Quanto parameter.


Design Structure
================

There are two layers to the kernel; the engine itself and the IO wrapper. The engine is the standard Black Scholes Merton engine, and the wrapper maps Quanto parameters to BSM parameter and provides the I/O to/from the engine.

The Engine
===========================

The engine is simply the :ref:`Black Scholes Merton Engine <black_scholes_merton_engine>`
The engine performs a single Black Scholes Merton Closed Form solution for a European Call.

IO Wrapper (quanto_kernel.cpp)
==============================

The kernel is the HLS wrapper level which implements the pipelining and parallelization to allow high throughput. The kernel uses a dataflow methodology with streams to pass the data through the design.

The top level's input and output ports are 512 bit wide, which is designed to match the whole DDR bus width and allowing vector access. In the case of float data type (4 bytes), sixteen parameters can be accessed from the bus in parallel. Each port is connected to its own AXI master with arbitration handled by the AXI switch and DDR controller under the hood.

These ports are interfaced via functions in bus_interface.hpp which convert between the wide bus and a template number of streams. Once input stream form, each stream is passed to a separate instance of the engine. The engine is wrapped inside bsm_stream_wrapper() which handles the stream processing. Here the II and loop unrolling is controlled. One cfBSMEngine engine is instanced per stream allowing for parallel processing of multiple parameter sets. Additionally, the engines are in an II=1 loop, so that each engine can produce one price and associated Greeks on each cycle.

This wrapper also handles the mapping from the Quanto parameters to the BSM parameters, and it multiplies the result of BSM call price by the fixed exchange rate 'E'.

Resource Utilization
====================

The floating point kernel Area Information:

:FF:         97160 (12% of SLR on u200 board)  
:LUT:        83888 (21% of SLR on u200 board)   
:DSP:        554 (24% of SLR on u200 board)
:BRAM:       544 (37% of SLR on u200 board)
:URAM:       0


Throughput
==========

Throughput is composed of two processes: transferring data to/from the FPGA and running the computations. The demo contains options to measure timings as described in the README.md file.

As an example, processing a batche of 19683 call calculations with a floating point kernel breaks down as follows:

Total time (memory transfer time plus calculation time) = 1175us

Calculation time (kernel execution time) = 405us

Memory transfer time = 770us

Throughput = 48.5663 Mega options/sec



.. toctree::
   :maxdepth: 1
