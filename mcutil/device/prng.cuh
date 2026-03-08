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
 * @file    mcutil/device/prng.cuh
 * @brief   Device-side random number generator
 * @author  CM Lee
 * @date    05/23/2023
 */


#pragma once

#include <cuda_runtime.h>
#include <curand_kernel.h>


namespace mcutil {


	__global__ void __kernel__initialize(curandState* state, int* seed, int* offset);


	/**
	* @brief Allocate memory and initialize rand state by seed and offset
	* @param block  Dimension of device kernel block
	* @param thread Dimension of device kernel thread
	* @param state  XORWOW PRNG state
	* @param seed   XORWOW PRNG seed
	* @param offset Absolute offset of sequence
	*/
	void __host__initialize(int block, int thread, curandState* state, int* seed, int* offset);


}