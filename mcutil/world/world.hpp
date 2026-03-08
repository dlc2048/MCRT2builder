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
 * @file    mcutil/world/world.hpp
 * @brief   RT2 world definitions
 * @author  CM Lee
 * @date    05/23/2023
 */


#pragma once

#include <string>
#include <vector>

#include <cuda_runtime.h>

#include "parser/input.hpp"


namespace geo {

	/*
	Base triangle segment
	*/

	struct MarkedTriangle {
		float3   vertex[3];
		int      region_idx_backface;
		int      region_idx_frontface;
		float3   center() const;
		double3  d_center() const;
		float3   normal() const;
		float    area() const;
		double   d_area() const;
		double   d_volume() const;
	};


	class BremSplit {
	private:
		std::string _target;
	public:
		BremSplit() = default;
		BremSplit(mcutil::ArgInput& args);
		const std::string& target() const;
	};


	/**
	* @brief Generic region context for module - world link
	*/
	class RegionContext {
	protected:
		std::string _reg_name;
		int         _reg_idx;
	public:


		RegionContext(const std::string& name);


		const std::string& name() const { return this->_reg_name; }


		int index() const { return this->_reg_idx; }


		void setIndex(int idx) { this->_reg_idx = idx; }


	};


	/*
	Unformatted polygon world
	*/

	class World {
	protected:
		std::vector<std::string>    _region;   // region data
		std::vector<int>            _reg_brem_split;  // bremsstrahlung splitting target regions
		std::vector<MarkedTriangle> _mesh;     // mesh data
	public:


		World();


		World(const std::string& file_name);


		void write(const std::string& file_name);


		void readBremSplit(mcutil::Input& input);


		const std::vector<std::string>& region() const;


		const std::vector<int>& regBremSplit() const;


		const std::vector<MarkedTriangle>& mesh() const;


		/* geometrical feature extractors */
		double volume(int region_idx) const;       // volume of region
		double area(int region_idx) const;         // area of region
		double area(int reg1, int reg2) const;     // area of boundary between reg1-reg2 (for boundary crossing)


		void printRegionInfo() const;


		// module - world linker
		bool link(RegionContext& reg_context) const;


	};

}


namespace mcutil {


	template<>
	ArgumentCard InputCardFactory<geo::BremSplit>::_setCard();


}