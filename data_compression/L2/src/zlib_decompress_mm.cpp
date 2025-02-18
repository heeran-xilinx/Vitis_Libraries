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

/**
 * @file xil_zlib_decompress_kernel.cpp
 * @brief Source for zlib decompression kernel.
 *
 * This file is part of Vitis Data Compression Library.
 */
#include "zlib_decompress_mm.hpp"

// LZ specific Defines
#define BIT 8
#define MIN_OFFSET 1
#define MIN_MATCH 4
#define LZ_MAX_OFFSET_LIMIT 32768
#define HISTORY_SIZE LZ_MAX_OFFSET_LIMIT
#define LOW_OFFSET 10

void xil_inflate(const xf::compression::uintMemWidth_t* in,
                 xf::compression::uintMemWidth_t* out,
                 uint32_t* encoded_size,
                 uint32_t input_size) {
    hls::stream<xf::compression::uintMemWidth_t> inStream512("inputStream");
    hls::stream<ap_uint<16> > outDownStream("outDownStream");
    hls::stream<ap_uint<8> > uncompOutStream("unCompOutStream");
    hls::stream<xf::compression::uintMemWidth_t> outStream512("outputStream");
    hls::stream<bool> outStream512_eos("outputStreamSize");
    hls::stream<bool> byte_eos("byteEndOfStream");

    hls::stream<xf::compression::compressd_dt> bitUnPackStream("bitUnPackStream");
    hls::stream<bool> bitEndOfStream("bitEndOfStream");
#pragma HLS STREAM variable = bitUnPackStream depth = 32
#pragma HLS STREAM variable = bitEndOfStream depth = 32

#pragma HLS STREAM variable = inStream512 depth = 32
#pragma HLS STREAM variable = outDownStream depth = 32
#pragma HLS STREAM variable = uncompOutStream depth = 32
#pragma HLS STREAM variable = outStream512 depth = 32
#pragma HLS STREAM variable = outStream512_eos depth = 32
#pragma HLS STREAM variable = byte_eos depth = 32

    hls::stream<uint32_t> outsize_val;
    hls::stream<bool> outStreamWidth_eos[PARALLEL_BLOCK];

#pragma HLS dataflow
    xf::compression::mm2sSimple<xf::compression::kGMemDWidth, xf::compression::kGMemBurstSize>(in, inStream512,
                                                                                               input_size);
    xf::compression::streamDownsizer<uint32_t, xf::compression::kGMemDWidth, 16>(inStream512, outDownStream,
                                                                                 input_size);

    xf::compression::huffBitUnPacker(outDownStream, bitUnPackStream, bitEndOfStream, input_size);

    xf::compression::lzDecompressZlibEos_new<HISTORY_SIZE, LOW_OFFSET>(bitUnPackStream, bitEndOfStream, uncompOutStream,
                                                                       byte_eos, outsize_val);

    xf::compression::upsizerEos<uint16_t, 8, xf::compression::kGMemDWidth>(uncompOutStream, byte_eos, outStream512,
                                                                           outStream512_eos);
    xf::compression::s2mmEosSimple<uint32_t, xf::compression::kGMemBurstSize, xf::compression::kGMemDWidth, 1>(
        out, outStream512, outStream512_eos, outsize_val, encoded_size);
}

extern "C" {
void xilDecompressZlib(xf::compression::uintMemWidth_t* in,
                       xf::compression::uintMemWidth_t* out,
                       uint32_t* encoded_size,
                       uint32_t input_size) {
#pragma HLS INTERFACE m_axi port = in offset = slave bundle = gmem0
#pragma HLS INTERFACE m_axi port = out offset = slave bundle = gmem0
#pragma HLS INTERFACE m_axi port = encoded_size offset = slave bundle = gmem1
#pragma HLS INTERFACE s_axilite port = in bundle = control
#pragma HLS INTERFACE s_axilite port = out bundle = control
#pragma HLS INTERFACE s_axilite port = encoded_size bundle = control
#pragma HLS INTERFACE s_axilite port = input_size bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control

    ////printme("In decompress kernel \n");
    // Call for parallel compression
    xil_inflate(in, out, encoded_size, input_size);
}
}
