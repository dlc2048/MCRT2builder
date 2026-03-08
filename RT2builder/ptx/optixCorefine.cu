//
// Copyright (C) 2025 CM Lee, SJ Ye, Seoul Sational University
//
// Licensed to the Apache Software Foundation(ASF) under one
// or more contributor license agreements.See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// 	"License"); you may not use this file except in compliance
// 	with the License.You may obtain a copy of the License at
// 
// 	http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.See the License for the
// specific language governing permissionsand limitations
// under the License.

/**
 * @file    RT2builder/optixCorefine.cu
 * @brief   RT2builder OptiX SBT
 * @author  CM Lee
 * @date    05/23/2023
 */


#include <optix.h>

#include "optixCorefine.h"
#include <cuda/helpers.h>

extern "C" {
    __constant__ ParamsCorefine params;
}


extern "C" __global__ void __raygen__core() {
    const uint3 idx = optixGetLaunchIndex();
    unsigned int p0;
    float3 beam_pos = params.beam_pos[idx.x];
    float3 beam_dir = params.beam_dir[idx.x];
    optixTrace(
        params.handle,
        beam_pos,
        beam_dir,
        0.0f,
        1e16f,
        0.0f,
        OptixVisibilityMask(1),
        OPTIX_RAY_FLAG_NONE,
        0,
        0,
        0,
        p0
    );
    params.is_inside[idx.x] = (bool)p0;
}

extern "C" __global__ void __miss__core() {
    optixSetPayload_0(0);
}

extern "C" __global__ void __closesthit__core() {
    unsigned int p;
    if (optixIsBackFaceHit())
        p = 1;
    else
        p = 0;
    optixSetPayload_0(p);
}