/*
 * Copyright 2019 Xilinx, Inc.
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
 */

#ifndef _XF_BPC_CONFIG_H_
#define _XF_BPC_CONFIG_H_
#include "hls_stream.h"
#include <ap_int.h>
#include "xf_config_params.h"
#include "common/xf_common.hpp"
#include "imgproc/xf_bpc.hpp"

// Set the image height and width
#define HEIGHT 2160 // 2160
#define WIDTH 3840  // 3840

#define WB_TYPE 1

#if NO
#define NPC1 XF_NPPC1
#endif
#if RO
#define NPC1 XF_NPPC2
#endif

#define IN_TYPE XF_8UC1
#define OUT_TYPE XF_8UC1

void badpixelcorrection_accel(xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, NPC1>& imgInput1,
                              xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPC1>& imgOutput);
#endif //_XF_BPC_CONFIG_H_
