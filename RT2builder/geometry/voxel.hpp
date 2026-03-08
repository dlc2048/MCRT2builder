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
 * @file    RT2builder/voxel.hpp
 * @brief   RT2builder voxel (CT) definitions
 * @author  CM Lee
 * @date    05/23/2023
 */


#pragma once

#include <iostream>
#include <exception>
#include <vector>
#include <string>
#include <cuda_runtime.h>

#include "mcutil/device/algorithm.hpp"
#include "mcutil/fortran/fortran.hpp"
#include "mcutil/mclog/logger.hpp"
#include "mcutil/parser/parser.hpp"
#include "mcutil/parser/input.hpp"
#include "mcutil/world/world.hpp"

namespace geo {


    class Voxel : public mcutil::Affine {
    private:
        int3                     _shape;   // voxel shape
        std::vector<uint16_t>    _array;   // 1-D voxel array
        std::vector<std::string> _region;  // region namelist
    public:
        // constructors
        Voxel();
        /*
        Voxel(
            const std::string& file_name, 
            const std::deque<Hounsfield>& hounsfield
        );
        */
        Voxel(const std::string& file_name);
        

        // interior structure generator
        std::vector<geo::MarkedTriangle> getInterior() const;

        // attribute mirrors
        int3 shape() const;
        const std::vector<uint16_t>& array() const;
        const std::vector<std::string>& region() const;
    };


    float3 make_float3(double3 point);


}