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
 * @file    RT2builder/region.hpp
 * @brief   RT2builder region definitions
 * @author  CM Lee
 * @date    05/23/2023
 */


#pragma once

#include <string>
#include <deque>
#include <vector>
#include <list>
#include <algorithm>

#include "mcutil/parser/input.hpp"


namespace geo {


	enum class BOOLEAN_OPERATOR {
		RECUR   = -1,
		NONE    = -2,
		NOT     = -3,
		AND     = -4,
		OR      = -5,
		BRA     = -6,
		KET     = -7,
		OPERAND = +0
	};


	struct EquationElem {
		BOOLEAN_OPERATOR type;
		std::string      operand_name;
		int              surface_id;
		EquationElem(BOOLEAN_OPERATOR op_type, const std::string& name);
		EquationElem(BOOLEAN_OPERATOR op_type);
		EquationElem(const std::string& name);
	};


	class Region {
	private:
		static std::map<std::string, BOOLEAN_OPERATOR> _OPERATOR;
		std::string             _name;
		std::list<EquationElem> _equation;
		size_t                  _surface_id_end;
		bool                    _good;
		bool _toPostfix();
	public:
		Region(mcutil::ArgInput& args);
		Region(const std::string& name);  // for voxel
		// attribute mirrors
		const std::string& name() const;
		const std::list<EquationElem>& equation() const;

		bool unfold(const std::vector<Region>& others);
		bool mark(const std::vector<std::string>& surface_namelist);
		size_t maximumSurfaceIndex() const;

		bool good() const;
		bool inside(const bool* state) const;
		bool related(int surface_idx) const;
		std::vector<size_t> correspondingIndex() const;
		
	};

}

