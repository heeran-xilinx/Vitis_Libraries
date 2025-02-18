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
#ifndef _XFCOMPRESSION_STREAM_DOWNSIZER_HPP_
#define _XFCOMPRESSION_STREAM_DOWNSIZER_HPP_

/**
 * @file stream_downsizer.hpp
 * @brief Header for stream downsizer module.
 *
 * This file is part of Vitis Data Compression Library.
 */

#include "common.h"

namespace xf {
namespace compression {
template <class SIZE_DT, int IN_WIDTH, int OUT_WIDTH>
void streamDownsizer(hls::stream<ap_uint<IN_WIDTH> >& inStream,
                     hls::stream<ap_uint<OUT_WIDTH> >& outStream,
                     SIZE_DT input_size) {
    /**
     * @brief This module reads the IN_WIDTH size from the data stream
     * and downsizes the data to OUT_WIDTH size and writes to output stream
     *
     * @tparam SIZE_DT data size
     * @tparam IN_WIDTH input width
     * @tparam OUT_WIDTH output width
     *
     * @param inStream input stream
     * @param outStream output stream
     * @param input_size input size
     */

    if (input_size == 0) // changed for gzip
        return;
    const int c_byteWidth = 8;
    const int c_inputWord = IN_WIDTH / c_byteWidth;
    const int c_outWord = OUT_WIDTH / c_byteWidth;
    uint32_t sizeOutputV = (input_size - 1) / c_outWord + 1;
    int factor = c_inputWord / c_outWord;
    ap_uint<IN_WIDTH> inBuffer = 0;
convInWidthtoV:
    for (int i = 0; i < sizeOutputV; i++) {
#pragma HLS PIPELINE II = 1
        int idx = i % factor;
        if (idx == 0) inBuffer = inStream.read();
        ap_uint<OUT_WIDTH> tmpValue = inBuffer.range((idx + 1) * OUT_WIDTH - 1, idx * OUT_WIDTH);
        outStream << tmpValue;
    }
}

template <class SIZE_DT, int IN_WIDTH, int OUT_WIDTH>
void streamDownsizerP2P(hls::stream<ap_uint<IN_WIDTH> >& inStream,
                        hls::stream<ap_uint<OUT_WIDTH> >& outStream,
                        SIZE_DT input_size,
                        SIZE_DT input_start_idx) {
    /**
     * @brief This module reads the IN_WIDTH size from the data stream
     * and downsizes the data to OUT_WIDTH size and writes to output stream
     *
     * @tparam SIZE_DT data size
     * @tparam IN_WIDTH input width
     * @tparam OUT_WIDTH output width
     *
     * @param inStream input stream
     * @param outStream output stream
     * @param input_size input size
     * @param input_start_idx input starting index
     */
    const int c_byteWidth = 8;
    const int c_inputWord = IN_WIDTH / c_byteWidth;
    const int c_outWord = OUT_WIDTH / c_byteWidth;
    uint32_t sizeOutputV = (input_size - 1) / c_outWord + 1;
    int factor = c_inputWord / c_outWord;
    ap_uint<IN_WIDTH> inBuffer = 0;
    int offset = input_start_idx % c_inputWord;
convInWidthtoV:
    for (int i = offset; i < (sizeOutputV + offset); i++) {
#pragma HLS PIPELINE II = 1
        int idx = i % factor;
        if (idx == 0 || i == offset) inBuffer = inStream.read();
        ap_uint<OUT_WIDTH> tmpValue = inBuffer.range((idx + 1) * OUT_WIDTH - 1, idx * OUT_WIDTH);
        outStream << tmpValue;
    }
}

} // namespace compression
} // namespace xf

#endif
