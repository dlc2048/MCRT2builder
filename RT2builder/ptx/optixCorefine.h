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
 * @file    RT2builder/optixCorefine.h
 * @brief   RT2builder OptiX SBT
 * @author  CM Lee
 * @date    05/23/2023
 */


struct ParamsCorefine {
    bool*                  is_inside;
    float3*                beam_pos;
    float3*                beam_dir;
    unsigned int           size;
    OptixTraversableHandle handle;
};

struct RayGenDataCorefine {
    // No data needed
};

struct MissDataCorefine {
    // No data needed
};

struct HitGroupDataCorefine {
    // No data needed
};

template <typename T>
struct SbtRecord
{
    __align__(OPTIX_SBT_RECORD_ALIGNMENT) char header[OPTIX_SBT_RECORD_HEADER_SIZE];
    T data;
};

typedef SbtRecord<RayGenDataCorefine>     RayGenSbtRecordCorefine;
typedef SbtRecord<MissDataCorefine>       MissSbtRecordCorefine;
typedef SbtRecord<HitGroupDataCorefine>   HitGroupSbtRecordCorefine;