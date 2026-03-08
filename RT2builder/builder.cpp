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
 * @file    RT2builder/builder.cpp
 * @brief   RT2builder main
 * @author  CM Lee
 * @date    05/23/2023
 */


#include <iostream>
#include <filesystem>

#include <optix_function_table_definition.h>
#include <cuda_runtime.h>

#include <sutil/CUDAOutputBuffer.h>
#include <sutil/sutil.h>
#include <sutil/Exception.h>

#include "mcutil/prompt/prompt.hpp"
#include "mcutil/parser/input.hpp"
#include "mcutil/mclog/logger.hpp"
#include "mcutil/fortran/fortran.hpp"

#include "geometry/geometry.hpp"


static void context_log_cb(unsigned int level, const char* tag, const char* message, void* /*cbdata */)
{
	std::cerr << "[" << std::setw(2) << level << "][" << std::setw(12) << tag << "]: "
		<< message << "\n";
}


int main(int argc, char* argv[]) {

	std::string cache       = ".cache";
	std::string cache_world = "world.bin";

	// Parse command line options
	mcutil::FileTarget target = mcutil::interpretCommandLine(argc, argv);
	std::filesystem::path res = target.result;
	if (!res.is_absolute())
		res = std::filesystem::current_path() / res;

	// cache
	if (!std::filesystem::exists(cache))
		std::filesystem::create_directories(cache);

	// link output file stream
	std::filesystem::path out = std::filesystem::path(target.output);
	if (out.string() == "")
		mclog::setLogger();
	else {
		out = res / out;
		mclog::setLogger(out.string());
	}

	mclog::setLevel(target.level);

	// file startup time
	mclog::time();

	// link input file stream
	mcutil::Input input(target.input, target.syntax);

	// initialize global settings
	mcutil::InputCardFactory<geo::GlobalSettings>::readAll(input);

	// Initialize CUDA and create OptiX context
	OptixDeviceContext context = nullptr;
	{
		// Initialize CUDA
		CUDA_CHECK(cudaFree(0));
		// Initialize the OptiX API, loading all API entry points
		OPTIX_CHECK(optixInit());
		OptixDeviceContextOptions options = {};
		options.logCallbackFunction = &context_log_cb;
		options.logCallbackLevel = 4;

		// Associate a CUDA context (and therefore a specific GPU) with this
		// device context
		CUcontext cuCtx = 0;  // zero means take the current context
		OPTIX_CHECK(optixDeviceContextCreate(cuCtx, &options, &context));
	}

	// read surfaces
	geo::LogicalSurfaceHandler surface_handle(input, context);
	
	// write primitives (if setting activated)
	if (geo::GlobalSettings::getInstance().savePrimitive())
		surface_handle.writePrimitives();

	// read regions
	geo::LogicalRegionHandler region_handle(input, surface_handle);

	// prepare optix world builder
	{
		geo::OptixWorldGenerator generator(context);
		generator.build(surface_handle, region_handle);

		// write cache data
		mclog::info() << "*** Write world data ***";
		geo::WorldFactory world(surface_handle, region_handle, generator);
		world.printRegionInfo();
		world.write(cache + "/" + cache_world);

		if (geo::GlobalSettings::getInstance().saveRegion())
			world.writeRegions(target.project);
	}
	OPTIX_CHECK(optixDeviceContextDestroy(context));
	return 0;
}