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
 * @file    mcutil/world/logic.hpp
 * @brief   Logical conversion in RT2 world builder
 * @author  CM Lee
 * @date    05/23/2023
 */

#pragma once

#include <utility>
#include <cuda_runtime.h>


namespace geo {


	constexpr double SMALL_COSINE_LIMIT            = 1e-20;
	constexpr double DIRECTION_VECTOR_NORM_MINIMUM = 1e-20;


	double degreeToRadian(double degree);


	double2 vectorToRotationAngle(double3 v);


	std::pair<double, double2> vectorToNormAngle(double3 v);

}