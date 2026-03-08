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
 * @file    RT2builder/geometry.hpp
 * @brief   RT2builder geometry handler
 * @author  CM Lee
 * @date    05/23/2023
 */


#pragma once

#include <deque>
#include <map>
#include <set>

#include <optix.h>
#include <optix_stack_size.h>
#include <optix_stubs.h>

#include <cuda_runtime.h>

#include "mcutil/parser/input.hpp"
#include "mcutil/parser/parser.hpp"
#include "mcutil/fortran/fortran.hpp"
#include "mcutil/world/world.hpp"

#include "surface.hpp"
#include "region.hpp"
#include "voxel.hpp"
#include "config.hpp"

#include "../ptx/optixCorefine.h"



namespace geo {


	class LogicalSurfaceHandler {
	private:
		std::vector<Surface>   _surface;
		std::shared_ptr<Voxel> _voxel;
		int _voxel_surface_idx;
	public:
		LogicalSurfaceHandler(
			mcutil::Input&     input, 
			OptixDeviceContext optix_context
		);
		void writePrimitives() const;
		bool hasVoxel() const;
		int voxelSurfaceIndex() const;
		size_t size() const;
		const Voxel& voxel() const;
		const std::vector<std::string>& voxelRegion() const;
		std::vector<std::string> namelist() const;
		const std::vector<Surface>& items() const;
	};


	class LogicalRegionHandler {
	private:
		std::vector<Region> _region;
	public:
		LogicalRegionHandler(
			mcutil::Input& input,
			const LogicalSurfaceHandler& surface_handle
		);
		const std::vector<Region>& items() const;
	};


	class LogicalNeighborHandler {
	private:
		std::vector<std::set<size_t>> _neighbor_surface_idx_list;
		std::vector<std::set<size_t>> _neighbor_region_idx_list;
	public:
		LogicalNeighborHandler(
			const LogicalSurfaceHandler& surface_handle,
			const LogicalRegionHandler&  region_handle
		);

		const std::set<size_t>& neighborSurface(size_t i) const { return this->_neighbor_surface_idx_list[i]; }
		const std::set<size_t>& neighborRegion(size_t i)  const { return this->_neighbor_region_idx_list[i];  }
	};



	/*
	CGAL mesh corefining handler
	*/

	class CorefineStructure {
	private:
		std::string _name;
		std::shared_ptr<mesh::PolygonMesh> _mesh_core;
		std::set<size_t> _neighbor_surface_idx_list;
		std::set<size_t> _neighbor_region_idx_list;
	public:
		CorefineStructure(
			const LogicalSurfaceHandler&  surface_handle,
			const LogicalRegionHandler&   region_handle,
			const LogicalNeighborHandler& neighbor_handle,
			size_t target_surface_idx
		);
		const std::string& name() const;
		const mesh::PolygonMesh& mesh() const;
		const std::set<size_t>& neighbor() const;
		void write() const;
	};


	class OptixWorldGenerator {
	private:
		// Optix related attributes
		OptixModule             _module;
		OptixProgramGroup       _raygen_prog_group;
		OptixProgramGroup       _miss_prog_group;
		OptixProgramGroup       _hit_prog_group;
		OptixPipeline           _pipeline;
		OptixShaderBindingTable _sbt;
		// voxel handling
		int64_t _voxel_region_idx;
		// world data
		std::vector<MarkedTriangle> _world;
	public:
		OptixWorldGenerator(OptixDeviceContext context);
		~OptixWorldGenerator();
		void build(
			const LogicalSurfaceHandler& surface_handle,
			const LogicalRegionHandler&  region_handle
		);
		int64_t voxelRegionIndex() const;
		const std::vector<MarkedTriangle>& world() const;
	};


	class WorldFactory : public World {
	public:


		WorldFactory(
			const LogicalSurfaceHandler& surface_handle,
			const LogicalRegionHandler&  region_handle,
			const OptixWorldGenerator&   generator
		);


		void writeRegions(const std::string& proj_name);


	};


}

