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
 * @file    RT2viewer/color.cpp
 * @brief   RT2viewer color definitions
 * @author  CM Lee
 * @date    05/23/2023
 */


#include "color.hpp"

namespace shader {

    std::vector<float3> COLOR_PALETTE;

    inline float rgbLinearize(uint8_t value) {
        float vf = (float)value / 255.f;
        return vf > 0.04045 ? std::pow((vf + 0.055) / 1.055, 2.4) : vf / 12.92;
    }

    inline float3 rgbToLinearRgb(uint8_t r, uint8_t g, uint8_t b) {
        return make_float3(
            rgbLinearize(r),
            rgbLinearize(g),
            rgbLinearize(b)
        );
    }

    void initializeColorSpace() {
        COLOR_PALETTE.push_back(rgbToLinearRgb(255, 000, 000));
        COLOR_PALETTE.push_back(rgbToLinearRgb(255, 128, 000));
        COLOR_PALETTE.push_back(rgbToLinearRgb(255, 255, 000));
        COLOR_PALETTE.push_back(rgbToLinearRgb(128, 255, 000));
        COLOR_PALETTE.push_back(rgbToLinearRgb(000, 255, 000));
        COLOR_PALETTE.push_back(rgbToLinearRgb(000, 255, 128));
        COLOR_PALETTE.push_back(rgbToLinearRgb(000, 255, 255));
        COLOR_PALETTE.push_back(rgbToLinearRgb(000, 128, 255));
        COLOR_PALETTE.push_back(rgbToLinearRgb(000, 000, 255));
        COLOR_PALETTE.push_back(rgbToLinearRgb(128, 000, 255));
        COLOR_PALETTE.push_back(rgbToLinearRgb(255, 000, 255));
        COLOR_PALETTE.push_back(rgbToLinearRgb(255, 000, 128));
    }

}