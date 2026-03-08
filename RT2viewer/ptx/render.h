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
 * @file    RT2viewer/render.h
 * @brief   RT2viewer OptiX data handle & SBT
 * @author  CM Lee
 * @date    05/23/2023
 */


struct Params {
    unsigned int           subframe_index;
    uchar4*                image;
    float3*                hit_position;
    int*                   hit_activated;
    unsigned int           width;
    unsigned int           height;
    int                    target;
    int                    method;

    int*                   surface_idx;
    int*                   region_idx_backface;
    int*                   region_idx_frontface;
    float3*                vertices;
    float3*                color_palette;

    float3                 eye;
    float3                 U;
    float3                 V;
    float3                 W;

    OptixTraversableHandle handle;
};

struct RayGenData
{
    // No data needed
};


struct MissData
{
    // No data needed
};


struct HitGroupData
{
    // No data needed
};
