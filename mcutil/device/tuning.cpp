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
 * @file    mcutil/device/prng.cpp
 * @brief   Detect GPU architecture
 * @author  CM Lee
 * @date    05/23/2023
 */


#include "mclog/logger.hpp"

#include "tuning.hpp"
#include "tuning.cuh"


namespace mcutil {


	size_t getShadingUnitsNumber(const cudaDeviceProp& dev_prop) {
		size_t mp = (size_t)dev_prop.multiProcessorCount;
		size_t cpm;  // cores per multi processor
		switch (dev_prop.major) {
		case 2:  // Fermi
			cpm = (dev_prop.minor == 1) ? 48 : 32;
			break;
		case 3:  // Kepler
			cpm = 192;
			break;
		case 5:  // Maxwell
			cpm = 128;
			break;
		case 6:  // Pascal
			if (dev_prop.minor == 1 || dev_prop.minor == 2) cpm = 128;
			else if (dev_prop.minor == 0) cpm = 64;
			else throw std::runtime_error("Unknown device type");
			break;
		case 7:  // Volta - Turing
			if (dev_prop.minor == 0 || dev_prop.minor == 5) cpm = 64;
			else throw std::runtime_error("Unknown device type");
			break;
		case 8:  // Ampere
			if (dev_prop.minor == 0) cpm = 64;
			else if (dev_prop.minor == 6 || dev_prop.minor == 9) cpm = 128;
			else throw std::runtime_error("Unknown device type");
			break;
		case 9:  // Hopper
			if (dev_prop.minor == 0) cpm = 128;
			else throw std::runtime_error("Unknown device type");
			break;
		case 10:  // Ada Lovelace
			cpm = 128;  // Sure?
			break;
		case 12:  // Blackwell
			cpm = 128;
			break;
		default:
			throw std::runtime_error("Unknown device type");
			break;
		}

		return cpm * mp;
	}


	template <>
	ArgumentCard InputCardFactory<DeviceController>::_setCard() {
		ArgumentCard arg_card("CUDA_SETTINGS");

		arg_card.insert<int>("gpu",
			"The ID of the GPU on which the code run. GPU "
			"list can be found in command 'nvidia-smi'.",
			std::vector<int>{0});

		arg_card.insert<int>("block_dim", 
			"The number of threads in a single block. "
			"The total number of threads is product of the "
			"block_dim, block_per_sm, and the number of "
			"stream multiprocessor(SM). Here, the number "
			"of SM is determined by hardware specification.",
			std::vector<int>{128},
			std::vector<int>{MIN_DIMENSION_THREAD}, 
			std::vector<int>{MAX_DIMENSION_THREAD});

		arg_card.insert<int>("block_per_sm", 
			"The number of block per stream multiprocessor "
			"multiplied by the theoretical occupancy. "
			"If the metric 'Volatile GPU-Util' in the "
			"'nvidia-smi' is not saturated, block_dim should "
			"be increased to achieve peak performance.",
			std::vector<int>{8},
			std::vector<int>{1},
			std::vector<int>{100});

		arg_card.insert<double>("buffer_ratio", 
			"The ratio of memory share of particle and "
			"interaction buffer per total GPU memory capacity. "
			"If the error caused by the buffer size is raised, "
			"this parameter should be increased.",
			{ 0.6 }, { 0.1 }, { 0.9 });

		arg_card.insert<double>("block_decay_rate", 
			"The decay rate of the block of launched kernel "
			"at the residual stage. This parameter can be "
			"tuned heuristically to achieve peak performance. "
			"Generally, this parameter should have a value "
			"close to 1 when activating a computationally "
			"intensive interaction (e.g. Doppler broadened "
			"Compton scattering).",
			{ 0.2 }, { 0.01 }, { 0.99 });

		arg_card.insert<int>("block_decay_limit", 
			"The lower bound on the number of the block in the "
			"launched kernel. When the decayed number of "
			"blocks become smaller than this limit, all "
			"remained phase-space of each buffer will be "
			"discarded. Peak performance can be achieved by "
			"increasing this parameter, as small kernel launch "
			"sizes degrade performance. However, accuracy may "
			"decrease because remaining particles are removed.",
			{ 1 }, { 1 }, { 100000 });

		arg_card.insert<int>("seed", 
			"Initial seed of the curand XORWOW random number generator.",
			std::vector<int>{42}, std::vector<int>{1}, std::vector<int>{INT_MAX - 1});

		return arg_card;
	}


	DeviceController::DeviceController() :
		_gpu_id(0), 
		_number_of_shader(0), 
		_number_of_sm(0),
		_global_memory_cap(0), 
		_shared_memory_cap(0),
		_thread(0), 
		_block_per_sm(0),
		_block(0), 
		_thread_qmd(0),
		_block_qmd(0), 
		_seed(42),
		_buffer_ratio(0.0),
		_block_decay_limit(1),
		_block_decay_rate(0.2)
		{}


	DeviceController::DeviceController(ArgInput& args) {
		cudaError_t err;
		int devices_count;
		err = cudaGetDeviceCount(&devices_count);
		if (err != cudaSuccess)
			mclog::fatal() << "Failed to detect Nvidia GPUs";
		
		int gpu_id = args["gpu"].cast<int>()[0];
		err = cudaSetDevice(gpu_id);
		if (err != cudaSuccess) 
			mclog::fatal() << "Failed to set device, '" << gpu_id << "' is invalid device id";
		this->_gpu_id = (size_t)gpu_id;

		cudaDeviceProp dev_prop;
		err = cudaGetDeviceProperties(&dev_prop, (int)this->_gpu_id);
		if (err != cudaSuccess) 
			mclog::fatal() << "Failed to get device properties";

		this->_gpu_name = std::string(dev_prop.name);

		mclog::debug() << "Set card " << this->_gpu_id << " (" << this->_gpu_name << ") to main device";

		this->_number_of_shader = getShadingUnitsNumber(dev_prop);
		this->_number_of_sm     = (size_t)dev_prop.multiProcessorCount;

		// Common
		this->_thread       = args["block_dim"].cast<size_t>()[0];
		size_t mult         = args["block_per_sm"].cast<size_t>()[0];
		this->_block        = (this->_number_of_shader / this->_thread) * mult;
		this->_block_per_sm = mult;

		// QMD
		this->_thread_qmd = BLOCK_DIM_QMD;
		this->_block_qmd  = std::min(this->_thread * this->_block / this->_thread_qmd, 
			(this->_number_of_shader / this->_thread_qmd) * BLOCK_PER_SM_QMD);

		this->_global_memory_cap = dev_prop.totalGlobalMem;
		this->_shared_memory_cap = dev_prop.sharedMemPerBlock / 2;

		this->_buffer_ratio = args["buffer_ratio"].cast<double>()[0];
		this->_seed         = args["seed"].cast<int>()[0];

		this->_block_decay_rate  = args["block_decay_rate"].cast<double>()[0];
		this->_block_decay_limit = args["block_decay_limit"].cast<size_t>()[0];
	}


	size_t DeviceController::freeMemory() const {
		size_t free_byte, total_byte;
		cudaMemGetInfo(&free_byte, &total_byte);
		return free_byte;
	}


}