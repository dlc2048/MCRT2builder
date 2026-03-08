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
 * @file    mcutil/world/logic.cpp
 * @brief   Logical conversion in RT2 world builder
 * @author  CM Lee
 * @date    05/23/2023
 */

#include <cmath>

#include "logic.hpp"


namespace geo {

	double degreeToRadian(double degree) {
		return M_PI / 180.0 * degree;
	}


	double2 vectorToRotationAngle(double3 v) {
		double theta, phi;
		double sint, cost, cosp;

		sint = -v.y;
		cost = sqrt(1.e0 - sint * sint);

		if (cost < SMALL_COSINE_LIMIT) {
			theta = (sint > 0) ? 90.0 : 270.0;
			phi = 0.0;
		}
		else {
			if (v.x < 0) 
				cost = -cost;
			cosp = v.z / cost;
			cosp = std::max(-1.0, std::min(cosp, 1.0));

			theta = acos(cost) * 180.0 / M_PI;
			phi = acos(cosp) * 180.0 / M_PI;
			if (sint < 0)
				theta = 360.0 - theta;
		}
		
		return make_double2(theta, phi);
	}


	std::pair<double, double2> vectorToNormAngle(double3 v) {
		double norm = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		double3 nv;
		if (norm < DIRECTION_VECTOR_NORM_MINIMUM)
			nv = { 0.0, 0.0, 1.0 };
		else
			nv = { v.x / norm, v.y / norm, v.z / norm };
		return { norm, vectorToRotationAngle(nv) };
	}

}