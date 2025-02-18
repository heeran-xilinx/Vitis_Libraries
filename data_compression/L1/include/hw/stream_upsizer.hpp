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
#ifndef _XFCOMPRESSION_STREAM_UPSIZER_HPP_
#define _XFCOMPRESSION_STREAM_UPSIZER_HPP_

/**
 * @file stream_upsizer.hpp
 * @brief Header for stream upsizer module.
 *
 * This file is part of Vitis Data Compression Library.
 */

#include "common.h"

namespace xf {
namespace compression {
template <class SIZE_DT, int IN_WIDTH, int OUT_WIDTH>
void streamUpsizer(hls::stream<ap_uint<IN_WIDTH> >& inStream,
                   hls::stream<ap_uint<OUT_WIDTH> >& outStream,
                   SIZE_DT original_size) {
    /**
     * @brief This module reads IN_WIDTH from the input stream and accumulate
     * the consecutive reads until OUT_WIDTH and writes the OUT_WIDTH data to
     * output stream
     *
     * @tparam SIZE_DT stream size class instance
     * @tparam IN_WIDTH input data width
     * @tparam OUT_WIDTH output data width
     *
     * @param inStream input stream
     * @param outStream output stream
     * @param original_size original stream size
     */

    if (original_size == 0) return;
    ap_uint<OUT_WIDTH> shift_register;
    uint8_t factor = OUT_WIDTH / IN_WIDTH;
    uint32_t withAppendedDataSize = (((original_size - 1) / factor) + 1) * factor;

    for (uint32_t i = 0; i < withAppendedDataSize; i++) {
#pragma HLS PIPELINE II = 1
        if (i != 0 && i % factor == 0) {
            outStream << shift_register;
            shift_register = 0;
        }
        if (i < original_size) {
            shift_register.range(OUT_WIDTH - 1, OUT_WIDTH - IN_WIDTH) = inStream.read();
        } else {
            shift_register.range(OUT_WIDTH - 1, OUT_WIDTH - IN_WIDTH) = 0;
        }
        if ((i + 1) % factor != 0) shift_register >>= IN_WIDTH;
    }
    // write last data to stream
    outStream << shift_register;
}

template <class SIZE_DT, int IN_WIDTH, int OUT_WIDTH>
void upsizerEos(hls::stream<ap_uint<IN_WIDTH> >& inStream,
                hls::stream<bool>& inStream_eos,
                hls::stream<ap_uint<OUT_WIDTH> >& outStream,
                hls::stream<bool>& outStream_eos) {
    /**
     * @brief This module reads IN_WIDTH data from input stream based
     * on end of stream and accumulate the consecutive reads until
     * OUT_WIDTH and then writes OUT_WIDTH data to output stream.
     *
     * @tparam SIZE_DT stream size class instance
     * @tparam IN_WIDTH input data width
     * @tparam OUT_WIDTH output data width
     *
     * @param inStream input stream
     * @param inStream_eos input end of stream flag
     * @param outStream output stream
     * @param outStream_eos output end of stream flag
     */
    // Constants
    const int c_byteWidth = IN_WIDTH;
    const int c_upsizeFactor = OUT_WIDTH / c_byteWidth;
    const int c_inSize = IN_WIDTH / c_byteWidth;

    ap_uint<OUT_WIDTH> outBuffer = 0;
    ap_uint<IN_WIDTH> outBuffer_int[c_upsizeFactor];
#pragma HLS array_partition variable = outBuffer_int dim = 1 complete
    uint32_t byteIdx = 0;
    bool done = false;
    ////printme("%s: reading next data=%d outSize=%d c_inSize=%d\n ",__FUNCTION__, size,outSize,c_inSize);
    outBuffer_int[byteIdx] = inStream.read();
stream_upsizer:
    for (bool eos_flag = inStream_eos.read(); eos_flag == false; eos_flag = inStream_eos.read()) {
#pragma HLS PIPELINE II = 1
        for (int j = 0; j < c_upsizeFactor; j += c_inSize) {
#pragma HLS unroll
            outBuffer.range((j + 1) * c_byteWidth - 1, j * c_byteWidth) = outBuffer_int[j];
        }
        byteIdx += 1;
        ////printme("%s: value=%c, chunk_size = %d and byteIdx=%d\n",__FUNCTION__,(char)tmpValue, chunk_size,byteIdx);
        if (byteIdx >= c_upsizeFactor) {
            outStream << outBuffer;
            outStream_eos << 0;
            byteIdx -= c_upsizeFactor;
        }
        outBuffer_int[byteIdx] = inStream.read();
    }

    if (byteIdx) {
        outStream_eos << 0;
        outStream << outBuffer;
    }
    // end of block

    outStream << 0;
    outStream_eos << 1;
    // printme("%s:Ended \n",__FUNCTION__);
}

} // namespace compression
} // namespace xf

#endif
