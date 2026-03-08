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
 * @file    RT2builder/config.hpp
 * @brief   RT2builder global configs
 * @author  CM Lee
 * @date    05/23/2023
 */


#pragma once

#include "mcutil/singleton/singleton.hpp"
#include "mcutil/parser/input.hpp"
#include "mcutil/parser/parser.hpp"

#include "mcutil/geometry/mesh_primitives.hpp"


namespace geo {


    constexpr int BOOLEAN_MAP_MAXIMUM_SIZE_BYTES = 10240000;

    
	class GlobalSettings : public Singleton<GlobalSettings> {
		friend class Singleton<GlobalSettings>;
	private:
		mesh::APPROXIMATION_TYPE _mesh_approximation_type;
		double _rt_stepsize;
		double _ge_tolerance;
		bool   _save_primitive;
		bool   _save_corefined;
		bool   _save_region;
	public:


		GlobalSettings();


		GlobalSettings(mcutil::ArgInput& args);


		mesh::APPROXIMATION_TYPE meshApproximationType() { 
			return this->_mesh_approximation_type;
		}


		double stepsize()       { return this->_rt_stepsize;    }
		double errorTolerance() { return this->_ge_tolerance;   }
		bool   savePrimitive()  { return this->_save_primitive; }
		bool   saveCorefined()  { return this->_save_corefined; }
		bool   saveRegion()     { return this->_save_region;    }


	};


}