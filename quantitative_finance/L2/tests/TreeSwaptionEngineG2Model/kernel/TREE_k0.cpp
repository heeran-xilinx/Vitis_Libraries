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

#include "tree_engine_kernel.hpp"
#ifndef __SYNTHESIS__
#include <iostream>
using namespace std;
#endif

extern "C" void TREE_k0(int type,
                        DT fixedRate,
                        int timestep,
                        DT initTime[LEN],
                        int initSize,
                        int exerciseCnt[ExerciseLen],
                        int floatingCnt[FloatingLen],
                        int fixedCnt[FixedLen],
                        DT NPV[N]) {
#ifndef HLS_TEST
#pragma HLS INTERFACE m_axi port = NPV bundle = gmem0 offset = slave
#pragma HLS INTERFACE m_axi port = initTime bundle = gmem1 offset = slave
#pragma HLS INTERFACE m_axi port = exerciseCnt bundle = gmem2 offset = slave
#pragma HLS INTERFACE m_axi port = floatingCnt bundle = gmem3 offset = slave
#pragma HLS INTERFACE m_axi port = fixedCnt bundle = gmem4 offset = slave

#pragma HLS INTERFACE s_axilite port = type bundle = control
#pragma HLS INTERFACE s_axilite port = fixedRate bundle = control
#pragma HLS INTERFACE s_axilite port = timestep bundle = control
#pragma HLS INTERFACE s_axilite port = NPV bundle = control
#pragma HLS INTERFACE s_axilite port = initTime bundle = control
#pragma HLS INTERFACE s_axilite port = exerciseCnt bundle = control
#pragma HLS INTERFACE s_axilite port = floatingCnt bundle = control
#pragma HLS INTERFACE s_axilite port = fixedCnt bundle = control
#pragma HLS INTERFACE s_axilite port = initSize bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control
#endif

    DT a = 0.050055733653096922;
    DT sigma = 0.0094424342056787739;
    DT b = 0.050052910248222851;
    DT eta = 0.0094424313463861171;
    DT rho = -0.76300324120391616;
    DT flatRate = 0.04875825;
    DT x0 = 0.0;
    DT nominal = 1000.0;
    DT spread = 0.0;
    int exercise_cnt[ExerciseLen];
    int floating_cnt[FloatingLen];
    int fixed_cnt[FixedLen];

    for (int i = 0; i < ExerciseLen; i++) exercise_cnt[i] = exerciseCnt[i];
    for (int i = 0; i < FloatingLen; i++) floating_cnt[i] = floatingCnt[i];
    for (int i = 0; i < FixedLen; i++) fixed_cnt[i] = fixedCnt[i];

#ifndef __SYNTHESIS__
    cout << "type=" << type << ",fixedRate=" << NPV[0] << endl;
    cout << "timestep=" << timestep << ",initSize=" << initSize << endl;
#endif

    Model model;
    model.initialization(flatRate, a, sigma, b, eta, rho);
    Process process1, process2;
    process1.init(a, sigma, 0.0, 0.0);
    process2.init(b, eta, 0.0, 0.0);

    treeSwaptionEngine<DT, Model, Process, DIM, LEN, LEN2>(model, process1, process2, type, fixedRate, timestep,
                                                           initTime, initSize, exercise_cnt, floating_cnt, fixed_cnt,
                                                           flatRate, nominal, x0, spread, rho, NPV);

#ifndef __SYNTHESIS__
    cout << "type=" << type << ",NPV=" << NPV[0] << endl;
#endif
}
