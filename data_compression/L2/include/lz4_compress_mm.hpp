/*
 * (c) Copyright 2019 Xilinx, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef _XFCOMPRESSION_LZ4_COMPRESS_KERNEL_HPP_
#define _XFCOMPRESSION_LZ4_COMPRESS_KERNEL_HPP_

/**
 * @file lz4_compress_kernel.hpp
 * @brief Header for LZ4 compression kernel.
 *
 * This file is part of Vitis Data Compression Library.
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "hls_stream.h"
#include <ap_int.h>

#include "lz_compress.hpp"
#include "lz_optional.hpp"
#include "mm2s.hpp"
#include "s2mm.hpp"
#include "stream_downsizer.hpp"
#include "stream_upsizer.hpp"

#include "lz4_compress.hpp"

#define MAX_LIT_COUNT 4096
#define PARALLEL_BLOCK 8

// Kernel top functions
extern "C" {
/**
 * @brief LZ4 compression kernel takes the raw data as input and compresses the data
 * in block based fashion and writes the output to global memory.
 *
 * @param in input raw data
 * @param out output compressed data
 * @param compressd_size compressed output size of each block
 * @param in_block_size input block size of each block
 * @param block_size_in_kb input block size in bytes
 * @param input_size input data size
 */
void xilLz4Compress(const xf::compression::uintMemWidth_t* in,
                    xf::compression::uintMemWidth_t* out,
                    uint32_t* compressd_size,
                    uint32_t* in_block_size,
                    uint32_t block_size_in_kb,
                    uint32_t input_size);
}
#endif
