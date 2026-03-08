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
 * @file    RT2builder/geometry.cpp
 * @brief   RT2builder geometry handler
 * @author  CM Lee
 * @date    05/23/2023
 */


#include <sutil/CUDAOutputBuffer.h>
#include <sutil/Exception.h>
#include <sutil/sutil.h>

#include "geometry.hpp"


namespace geo {


    LogicalSurfaceHandler::LogicalSurfaceHandler(
        mcutil::Input& input,
        OptixDeviceContext optix_context
    ) : _voxel_surface_idx(-1) {
        typedef std::vector<mcutil::ArgumentCard> CardList;

        // read rotation definitions
        std::deque<RotDefi> rot_defi = mcutil::InputCardFactory<RotDefi>::readAll(input);
        for (RotDefi& rot_card : rot_defi)
            rot_card.summary();

        // read voxel hounsfield
        // std::deque<Hounsfield> hounsfield = Hounsfield::getBaseCard().readAll(input);

        // check starting point
        mcutil::InputCardFactory<SurfaceBegin>::readAll(input, 1);
        if (input.eof())
            mclog::fatalNecessary(mcutil::InputCardFactory<SurfaceBegin>::getCardDefinition().key());

        // read surface series
        std::deque<RotDefi*> rot_defi_stack;

        // syntax helper
        if (input.printSyntax()) {
            mclog::info() << "List of Possible Surface Cards";
            mcutil::InputCardFactory<TransformBegin>::getCardDefinition().printHelp();
            mcutil::InputCardFactory<TransformEnd  >::getCardDefinition().printHelp();
            mcutil::InputCardFactory<VoxelContainer>::getCardDefinition().printHelp();
            mcutil::InputCardFactory<Revolution    >::getCardDefinition().printHelp();
            mcutil::InputCardFactory<Toroid        >::getCardDefinition().printHelp();
            mcutil::InputCardFactory<UvSphere      >::getCardDefinition().printHelp();
            mcutil::InputCardFactory<Cylinder      >::getCardDefinition().printHelp();
            mcutil::InputCardFactory<Ellipsoid     >::getCardDefinition().printHelp();
            mcutil::InputCardFactory<Cone          >::getCardDefinition().printHelp();
            mcutil::InputCardFactory<IcoSphere     >::getCardDefinition().printHelp();
            mcutil::InputCardFactory<Cube          >::getCardDefinition().printHelp();
            mcutil::InputCardFactory<Torus         >::getCardDefinition().printHelp();
            mcutil::InputCardFactory<Plate         >::getCardDefinition().printHelp();
            mcutil::InputCardFactory<Model         >::getCardDefinition().printHelp();
        }
        

        while (true) {

            std::pair<std::string, mcutil::ArgInput> card =
                input.nextCard();
            input.printPreviousLine();

            std::string& key = card.first;
            mcutil::ArgInput& args = card.second;

            Surface surf;
            bool is_mesh  = false;
            bool is_voxel = false;

            if (key == mcutil::InputCardFactory<SurfaceEnd>::getCardDefinition().key()) {  // SURFACE_END case
                if (!rot_defi_stack.empty())
                    mclog::fatal() << "'TRANSFORM_END' parenthesis error";
                break;
            }
            else if (key == mcutil::InputCardFactory<TransformBegin>::getCardDefinition().key()) {  // TRANSFORM_BEGIN case
                mcutil::InputCardFactory<TransformBegin>::getCardDefinition().get(args);
                TransformBegin begin(args);
                const std::string& target = begin.target();
                size_t i = 0;
                for (; i < rot_defi.size(); ++i) {
                    if (rot_defi[i].name() == target) {
                        rot_defi_stack.push_back(&rot_defi[i]);
                        break;
                    }
                }
                if (i == rot_defi.size())
                    mclog::fatalNameNotExist(target);
                continue;
            }
            else if (key == mcutil::InputCardFactory<TransformEnd>::getCardDefinition().key()) {  // TRANSFORM_END case
                if (rot_defi_stack.empty())
                    mclog::fatal() << "'TRANSFORM_END' parenthesis error";
                rot_defi_stack.pop_back();
                continue;
            }
            else if (key == mcutil::InputCardFactory<VoxelContainer>::getCardDefinition().key()) {
                mcutil::InputCardFactory<VoxelContainer>::getCardDefinition().get(args);
                VoxelContainer voxel(args);
                // voxel.build(hounsfield);
                surf = Surface(voxel);
                is_mesh  = true;
                is_voxel = true;
                // move voxel data
                this->_voxel_surface_idx = this->_surface.size();
                this->_voxel = voxel.voxel();
                voxel.summary();
            }
            else if (key == mcutil::InputCardFactory<Revolution>::getCardDefinition().key()) {
                mcutil::InputCardFactory<Revolution>::getCardDefinition().get(args);
                Revolution revolution(args);
                surf = Surface(revolution);
                is_mesh = true;
            }
            else if (key == mcutil::InputCardFactory<Toroid>::getCardDefinition().key()) {
                mcutil::InputCardFactory<Toroid>::getCardDefinition().get(args);
                Toroid toroid(args);
                surf = Surface(toroid);
                is_mesh = true;
            }
            else if (key == mcutil::InputCardFactory<UvSphere>::getCardDefinition().key()) {
                mcutil::InputCardFactory<UvSphere>::getCardDefinition().get(args);
                surf = UvSphere(args);
                is_mesh = true;
            }
            else if (key == mcutil::InputCardFactory<Cylinder>::getCardDefinition().key()) {
                mcutil::InputCardFactory<Cylinder>::getCardDefinition().get(args);
                surf = Cylinder(args);
                is_mesh = true;
            }
            else if (key == mcutil::InputCardFactory<Ellipsoid>::getCardDefinition().key()) {
                mcutil::InputCardFactory<Ellipsoid>::getCardDefinition().get(args);
                surf = Ellipsoid(args);
                is_mesh = true;
            }
            else if (key == mcutil::InputCardFactory<Cone>::getCardDefinition().key()) {
                mcutil::InputCardFactory<Cone>::getCardDefinition().get(args);
                surf = Cone(args);
                is_mesh = true;
            }
            else if (key == mcutil::InputCardFactory<IcoSphere>::getCardDefinition().key()) {
                mcutil::InputCardFactory<IcoSphere>::getCardDefinition().get(args);
                surf = IcoSphere(args);
                is_mesh = true;
            }
            else if (key == mcutil::InputCardFactory<Cube>::getCardDefinition().key()) {
                mcutil::InputCardFactory<Cube>::getCardDefinition().get(args);
                surf = Cube(args);
                is_mesh = true;
            }
            else if (key == mcutil::InputCardFactory<Torus>::getCardDefinition().key()) {
                mcutil::InputCardFactory<Torus>::getCardDefinition().get(args);
                surf = Torus(args);
                is_mesh = true;
            }
            else if (key == mcutil::InputCardFactory<Plate>::getCardDefinition().key()) {
                surf = Plate(args);
                is_mesh = true;
            }
            else if (key == mcutil::InputCardFactory<Model>::getCardDefinition().key()) {
                mcutil::InputCardFactory<Model>::getCardDefinition().get(args);
                surf = Model(args);
                is_mesh = true;
            }
            else {
                mclog::fatal() << "SURFACE hit invalid card or EOF, card: " << key;
            }
                
            if (!is_mesh) continue;

            // mesh transformation
            for (std::deque<RotDefi*>::reverse_iterator iter = rot_defi_stack.rbegin();
                iter != rot_defi_stack.rend(); ++iter) {
                surf.transform(**iter);
                if (is_voxel)
                    this->_voxel->transform(**iter);
            }
            // check surface integrity
            /*
            if (surf.checkSelfIntersect()) {
                std::stringstream ss;
                ss << "Surface '" << surf.name() << "' has self-intersecting facets";
                mclog::fatal(ss);
            }
            if (!surf.checkIsClosed()) {
                std::stringstream ss;
                ss << "Surface '" << surf.name() << "' is not closed";
                mclog::fatal(ss);
            }
            */
            // build Optix object
            surf.compressInitialScene(optix_context);
            this->_surface.push_back(surf);
        }
        return;
    }


    void LogicalSurfaceHandler::writePrimitives() const {
        mclog::debug() << "Write surface primitives ...";
        for (size_t i = 0; i < this->_surface.size(); ++i) {
            this->_surface[i].writePrimitive(this->_surface[i].name() + ".off");
        }
    }


    bool LogicalSurfaceHandler::hasVoxel() const {
        return this->_voxel_surface_idx >= 0;
    }


    int LogicalSurfaceHandler::voxelSurfaceIndex() const {
        return this->_voxel_surface_idx;
    }


    size_t LogicalSurfaceHandler::size() const {
        return this->_surface.size();
    }


    const Voxel& LogicalSurfaceHandler::voxel() const {
        return *this->_voxel;
    }


    const std::vector<std::string>& LogicalSurfaceHandler::voxelRegion() const {
        return this->_voxel->region();
    }


    std::vector<std::string> LogicalSurfaceHandler::namelist() const {
        std::vector<std::string> list;
        for (size_t i = 0; i < this->_surface.size(); ++i) {
            list.push_back(this->_surface[i].name());
        }
        return list;
    }


    const std::vector<Surface>& LogicalSurfaceHandler::items() const {
        return this->_surface;
    }


    LogicalRegionHandler::LogicalRegionHandler(
        mcutil::Input& input,
        const LogicalSurfaceHandler& surface_handle
    ) {
        // push voxel region (if exist)
        if (surface_handle.hasVoxel()) {
            const std::vector<std::string> reg_namelist = surface_handle.voxelRegion();
            for (const std::string& reg_name : reg_namelist) {
                for (const Region& reg : this->_region) {
                    if (reg_name == reg.name())
                        mclog::fatalNameAlreadyExist(reg_name);
                }
                this->_region.push_back(Region(reg_name));
            }
        }

        std::deque<Region> reg_init = mcutil::InputCardFactory<Region>::readAll(input);

        // build surface namelist
        std::vector<std::string> namelist = surface_handle.namelist();

        for (Region& reg : reg_init) {
            // unfold recursive expressions
            reg.unfold(this->_region);
            // mark surface idx
            reg.mark(namelist);
            // check equation integrity
            if (!reg.good()) {
                mclog::fatal() << "Boolean equation of REGION '" << reg.name() << "' has problem";
            }
            for (const Region& reg_glob : this->_region) {
                if (reg_glob.name() == reg.name())
                    mclog::fatalNameAlreadyExist(reg.name());
            }
            this->_region.push_back(reg);
        }

        return;
    }


    const std::vector<Region>& LogicalRegionHandler::items() const {
        return this->_region;
    }


    LogicalNeighborHandler::LogicalNeighborHandler(
        const LogicalSurfaceHandler& surface_handle,
        const LogicalRegionHandler&  region_handle
    ) {
        const std::vector<Surface> slist = surface_handle.items();
        const std::vector<Region>  rlist = region_handle.items();

        this->_neighbor_region_idx_list.resize(slist.size());
        this->_neighbor_surface_idx_list.resize(slist.size());

        for (size_t target_surface_idx = 0; target_surface_idx < slist.size(); ++target_surface_idx) {

            const Surface& target_surface = slist[target_surface_idx];
            const mesh::PrimitiveMesh& mesh_init = target_surface.mesh_init();

            // overlap test by boolean equation
            for (size_t i = 0; i < rlist.size(); ++i) {
                if (rlist[i].related((int)target_surface_idx)) {
                    std::vector<size_t> nei = rlist[i].correspondingIndex();
                    this->_neighbor_surface_idx_list[target_surface_idx].insert(nei.begin(), nei.end());
                    this->_neighbor_region_idx_list[target_surface_idx].insert(i);
                }
            }

            // overlap test by plane of surface
            for (size_t i = 0; i < slist.size(); ++i) {
                if (i == target_surface_idx) continue;
                const mesh::PrimitiveMesh& mesh_other = slist[i].mesh_init();
                bool some_plane_overlapped = false;

                for (size_t j = 0; j < mesh_other.number_of_plane(); ++j) {
                    const mesh::Plane& plane_you = mesh_other.plane(j);
                    for (size_t k = 0; k < mesh_init.number_of_plane(); ++k) {
                        const mesh::Plane& plane_me = mesh_init.plane(k);

                        mesh::PLANAR_OVERLAP_TYPE overlap = plane_me.overlap(plane_you);
                        if (overlap.on_same_plane) {
                            if (!plane_me.isOverlapAllowed()) {
                                mclog::fatal() << "Plane '" << k << "' of surface '" << target_surface.name()
                                               << "' is overlaping with some plane of surface '" << slist[i].name()
                                               << "', and it is prohibited";
                            }
                            else {
                                mclog::debug() << "Plane '" << k << "' of surface '" << target_surface.name()
                                               << "' is overlaping with some plane of surface '" << slist[i].name() << "'";
                                mclog::debug() << "'" << slist[i].name() << "' will be condidered as implicit neighbor";
                            }
                            some_plane_overlapped = true;
                        }

                    }
                }

                // add to neighbor
                if (some_plane_overlapped) {
                    this->_neighbor_surface_idx_list[target_surface_idx].insert(i);
                    for (size_t l = 0; l < rlist.size(); ++l) {
                        if (rlist[l].related((int)i)) {
                            std::vector<size_t> nei = rlist[l].correspondingIndex();
                            this->_neighbor_region_idx_list[target_surface_idx].insert(l);
                        }
                    }
                }
            }

            // discard self index
            if (this->_neighbor_surface_idx_list[target_surface_idx].find(target_surface_idx)
                != this->_neighbor_surface_idx_list[target_surface_idx].end())
                this->_neighbor_surface_idx_list[target_surface_idx].erase(target_surface_idx);
        }

        // mutual reference
        for (size_t i = 0; i < slist.size(); ++i) {
            for (size_t j = i + 1; j < slist.size(); ++j) {
                for (size_t k : this->_neighbor_region_idx_list[j]) {
                    this->_neighbor_region_idx_list[i].insert(k);
                }
            }
        }
        return;
    }


    CorefineStructure::CorefineStructure(
        const LogicalSurfaceHandler&  surface_handle,
        const LogicalRegionHandler&   region_handle,
        const LogicalNeighborHandler& neighbor_handle,
        size_t target_surface_idx
    ) {
        const std::vector<Surface> slist = surface_handle.items();
        const std::vector<Region>  rlist = region_handle.items();

        const Surface& target_surface = slist[target_surface_idx];

        // copy original non-corefined mesh
        this->_name = target_surface.name();
        const mesh::PrimitiveMesh& mesh_init = target_surface.mesh_init();
        this->_mesh_core =
            std::make_shared<mesh::PolygonMesh>(
                mesh_init.mesh()
                );

        this->_neighbor_surface_idx_list = neighbor_handle.neighborSurface(target_surface_idx);
        this->_neighbor_region_idx_list  = neighbor_handle.neighborRegion(target_surface_idx);

        // corefine
        for (std::set<size_t>::iterator ns = this->_neighbor_surface_idx_list.begin();
            ns != this->_neighbor_surface_idx_list.end(); ++ns) {
            bool core_flag = true;
            mesh::PrimitiveMesh  mesh_target(slist[*ns].mesh_init());  // copy target mesh
            for (size_t i = 0; i < mesh_init.number_of_plane(); ++i) {
                for (size_t j = 0; j < mesh_target.number_of_plane(); ++j) {
                    mesh::PLANAR_OVERLAP_TYPE type = mesh_target.plane(j).overlap(mesh_init.plane(i));
                    if (type.on_same_plane) {
                        switch (type.polygon_overlap) {
                        case mesh::POLYGON_OVERLAP_TYPE::IDENTICAL:
                            if (!type.face_to_face) {
                                /*
                                if (abs(this->_mesh_core->plane(i).sideAngle() - mesh_target.plane(j).sideAngle()) < 1e-3)
                                    mcutil::output.fatal("");
                                */
                            }
                            core_flag = false;
                            break;
                        case mesh::POLYGON_OVERLAP_TYPE::INTERSECT:
                        case mesh::POLYGON_OVERLAP_TYPE::INSIDE:
                            mesh_target.extend(j, 1e-4);
                        case mesh::POLYGON_OVERLAP_TYPE::OUTSIDE:
                            mesh_target.extend(j, 1e-4);
                            break;
                        }
                    }
                }
            }
            mesh::PolygonMesh mesh_target_mesh = mesh_target.mesh();
            if (core_flag) {
                if (CGAL::Polygon_mesh_processing::do_intersect(
                    *this->_mesh_core,
                    mesh_target_mesh,
                    CGAL::Polygon_mesh_processing::parameters::do_overlap_test_of_bounded_sides(false),
                    CGAL::Polygon_mesh_processing::parameters::do_overlap_test_of_bounded_sides(false)
                )) {
                    CGAL::Polygon_mesh_processing::corefine(
                        *this->_mesh_core,
                        mesh_target_mesh
                    );
                    mesh::repairCorefinedMesh(&(*this->_mesh_core));
                }
            }
        }
    }


    const std::string& CorefineStructure::name() const {
        return this->_name;
    }


    const mesh::PolygonMesh& CorefineStructure::mesh() const {
        return *this->_mesh_core;
    }


    const std::set<size_t>& CorefineStructure::neighbor() const {
        return this->_neighbor_region_idx_list;
    }


    void CorefineStructure::write() const {
        std::string name = this->_name;
        name += "_core.off";
        std::ofstream out(name);
        out << *this->_mesh_core << std::endl;
    }


    OptixWorldGenerator::OptixWorldGenerator(OptixDeviceContext context) :
        _module(nullptr),
        _raygen_prog_group(nullptr),
        _miss_prog_group(nullptr),
        _hit_prog_group(nullptr),
        _pipeline(nullptr),
        _sbt({}),
        _voxel_region_idx(-1) {
        // Create module
        OptixPipelineCompileOptions pipeline_compile_options = {};
        {
            OptixModuleCompileOptions module_compile_options = {};
#if !defined( NDEBUG )
            module_compile_options.optLevel = OPTIX_COMPILE_OPTIMIZATION_LEVEL_0;
            module_compile_options.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_FULL;
#endif
            pipeline_compile_options.usesMotionBlur = false;
            pipeline_compile_options.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_ANY;
            pipeline_compile_options.numPayloadValues = 1;
            pipeline_compile_options.numAttributeValues = 2;
            pipeline_compile_options.exceptionFlags = OPTIX_EXCEPTION_FLAG_NONE;
            pipeline_compile_options.pipelineLaunchParamsVariableName = "params";
            pipeline_compile_options.usesPrimitiveTypeFlags = OPTIX_PRIMITIVE_TYPE_FLAGS_TRIANGLE;

            size_t      inputSize = 0;
            const char* input;

            input = sutil::getInputData(OPTIX_SAMPLE_NAME, OPTIX_SAMPLE_DIR, "optixCorefine.cu", inputSize);

            if (!inputSize)
                mclog::fatalFileNotExist("optixCorefine.cu");

            OPTIX_CHECK_LOG(optixModuleCreateFromPTX(
                context,
                &module_compile_options,
                &pipeline_compile_options,
                input,
                inputSize,
                LOG, &LOG_SIZE,
                &_module
            ));
        }
        // Create progeam groups
        {
            OptixProgramGroupOptions program_group_options = {}; // Initialize to zeros

            OptixProgramGroupDesc raygen_prog_group_desc = {}; //
            raygen_prog_group_desc.kind = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;
            raygen_prog_group_desc.raygen.module = _module;
            raygen_prog_group_desc.raygen.entryFunctionName = "__raygen__core";
            OPTIX_CHECK_LOG(optixProgramGroupCreate(
                context,
                &raygen_prog_group_desc,
                1,   // num program groups
                &program_group_options,
                LOG, &LOG_SIZE,
                &_raygen_prog_group
            ));

            OptixProgramGroupDesc miss_prog_group_desc = {};
            miss_prog_group_desc.kind = OPTIX_PROGRAM_GROUP_KIND_MISS;
            miss_prog_group_desc.miss.module = _module;
            miss_prog_group_desc.miss.entryFunctionName = "__miss__core";
            OPTIX_CHECK_LOG(optixProgramGroupCreate(
                context,
                &miss_prog_group_desc,
                1,   // num program groups
                &program_group_options,
                LOG, &LOG_SIZE,
                &_miss_prog_group
            ));

            OptixProgramGroupDesc hitgroup_prog_group_desc = {};
            hitgroup_prog_group_desc.kind = OPTIX_PROGRAM_GROUP_KIND_HITGROUP;
            hitgroup_prog_group_desc.hitgroup.moduleCH = _module;
            hitgroup_prog_group_desc.hitgroup.entryFunctionNameCH = "__closesthit__core";
            OPTIX_CHECK_LOG(optixProgramGroupCreate(
                context,
                &hitgroup_prog_group_desc,
                1,   // num program groups
                &program_group_options,
                LOG, &LOG_SIZE,
                &_hit_prog_group
            ));
        }
        // Link pipeline
        {
            const uint32_t    max_trace_depth = 1;
            OptixProgramGroup program_groups[] = { _raygen_prog_group, _miss_prog_group, _hit_prog_group };

            OptixPipelineLinkOptions pipeline_link_options = {};
            pipeline_link_options.maxTraceDepth = max_trace_depth;
#if !defined( NDEBUG )
            pipeline_link_options.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_FULL;
#endif
            OPTIX_CHECK_LOG(optixPipelineCreate(
                context,
                &pipeline_compile_options,
                &pipeline_link_options,
                program_groups,
                sizeof(program_groups) / sizeof(program_groups[0]),
                LOG, &LOG_SIZE,
                &_pipeline
            ));

            OptixStackSizes stack_sizes = {};
            for (auto& prog_group : program_groups)
            {
                OPTIX_CHECK(optixUtilAccumulateStackSizes(prog_group, &stack_sizes));
            }

            uint32_t direct_callable_stack_size_from_traversal;
            uint32_t direct_callable_stack_size_from_state;
            uint32_t continuation_stack_size;
            OPTIX_CHECK(optixUtilComputeStackSizes(&stack_sizes, max_trace_depth,
                0,  // maxCCDepth
                0,  // maxDCDEpth
                &direct_callable_stack_size_from_traversal,
                &direct_callable_stack_size_from_state, &continuation_stack_size));
            OPTIX_CHECK(optixPipelineSetStackSize(_pipeline, direct_callable_stack_size_from_traversal,
                direct_callable_stack_size_from_state, continuation_stack_size,
                1  // maxTraversableDepth
            ));
        }
        // Shader binding table
        {
            CUdeviceptr  raygen_record;
            const size_t raygen_record_size = sizeof(RayGenSbtRecordCorefine);
            CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&raygen_record), raygen_record_size));
            RayGenSbtRecordCorefine rg_sbt;
            OPTIX_CHECK(optixSbtRecordPackHeader(_raygen_prog_group, &rg_sbt));
            CUDA_CHECK(cudaMemcpy(
                reinterpret_cast<void*>(raygen_record),
                &rg_sbt,
                raygen_record_size,
                cudaMemcpyHostToDevice
            ));

            CUdeviceptr miss_record;
            size_t      miss_record_size = sizeof(MissSbtRecordCorefine);
            CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&miss_record), miss_record_size));
            MissSbtRecordCorefine ms_sbt;
            OPTIX_CHECK(optixSbtRecordPackHeader(_miss_prog_group, &ms_sbt));
            CUDA_CHECK(cudaMemcpy(
                reinterpret_cast<void*>(miss_record),
                &ms_sbt,
                miss_record_size,
                cudaMemcpyHostToDevice
            ));

            CUdeviceptr hitgroup_record;
            size_t      hitgroup_record_size = sizeof(HitGroupSbtRecordCorefine);
            CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&hitgroup_record), hitgroup_record_size));
            HitGroupSbtRecordCorefine hg_sbt;
            OPTIX_CHECK(optixSbtRecordPackHeader(_hit_prog_group, &hg_sbt));
            CUDA_CHECK(cudaMemcpy(
                reinterpret_cast<void*>(hitgroup_record),
                &hg_sbt,
                hitgroup_record_size,
                cudaMemcpyHostToDevice
            ));

            _sbt.raygenRecord = raygen_record;
            _sbt.missRecordBase = miss_record;
            _sbt.missRecordStrideInBytes = sizeof(MissSbtRecordCorefine);
            _sbt.missRecordCount = 1;
            _sbt.hitgroupRecordBase = hitgroup_record;
            _sbt.hitgroupRecordStrideInBytes = sizeof(HitGroupSbtRecordCorefine);
            _sbt.hitgroupRecordCount = 1;
        }
    }


    OptixWorldGenerator::~OptixWorldGenerator() {
        OPTIX_CHECK(optixPipelineDestroy(_pipeline));
        OPTIX_CHECK(optixProgramGroupDestroy(_hit_prog_group));
        OPTIX_CHECK(optixProgramGroupDestroy(_miss_prog_group));
        OPTIX_CHECK(optixProgramGroupDestroy(_raygen_prog_group));
        OPTIX_CHECK(optixModuleDestroy(_module));

        CUDA_CHECK(cudaFree(reinterpret_cast<void*>(_sbt.raygenRecord)));
        CUDA_CHECK(cudaFree(reinterpret_cast<void*>(_sbt.missRecordBase)));
        CUDA_CHECK(cudaFree(reinterpret_cast<void*>(_sbt.hitgroupRecordBase)));
    }


    void OptixWorldGenerator::build(
        const LogicalSurfaceHandler& surface_handle,
        const LogicalRegionHandler& region_handle
    ) {
        std::vector<MarkedTriangle> world;
        const std::vector<Surface>& slist = surface_handle.items();
        const std::vector<Region>&  rlist = region_handle.items();

        // build relationship matrix
        LogicalNeighborHandler neighbor_handle(surface_handle, region_handle);

        mclog::FormattedTable pos_fmt({ 12, 12, 12 });

        CUstream stream;
        CUDA_CHECK(cudaStreamCreate(&stream));
        // Generate corefined geometry, and launch Optix program
        for (size_t i = 0; i < slist.size(); ++i) {
            // for voxel geometry handling
            bool is_atomic = false;  // voxel geometry cannot be divided by other region
            if (surface_handle.hasVoxel()) {
                if (i == (size_t)surface_handle.voxelSurfaceIndex())
                    is_atomic = true;
            }
            int64_t voxel_backface_idx = -1;
            CorefineStructure core(surface_handle, region_handle, neighbor_handle, i);
            std::vector<float3> core_vertices = mesh::extractVertices(core.mesh(), 0);
            std::set<size_t> neighbor = core.neighbor();
            size_t n_final_facets = 0;

            if (GlobalSettings::getInstance().saveCorefined())
                core.write();

            // summarize corefining result
            {
                mclog::info()  << "[ " << std::setw(12) << mcutil::truncate(slist[i].name(), 12) << " ] corefinement results";
                mclog::print() << "*** Region boundary crossing candidates (namelist) ***";
                std::stringstream ss;
                for (const size_t& idx : neighbor)
                    ss << " " << rlist[idx].name();
                mclog::print() << ss.str();
                mclog::print() << "Number of facet (initial)   : " << slist[i].mesh_init().number_of_faces();
                mclog::print() << "Number of facet (corefined) : " << core_vertices.size() / 3;
            }
            if (neighbor.empty()) {
                mclog::warning() << "No neighboring region, skipped";
                continue;
            }

            // Build marked triangle list
            double area_error_limit = 0.e0;
            double area_error_cumul = 0.e0;
            std::vector<geo::MarkedTriangle> marked_facet;
            marked_facet.resize(core_vertices.size() / 3);

            for (size_t j = 0; j < marked_facet.size(); ++j) {
                for (size_t k = 0; k < 3; ++k) {
                    marked_facet[j].vertex[k].x = core_vertices[3 * j + k].x;
                    marked_facet[j].vertex[k].y = core_vertices[3 * j + k].y;
                    marked_facet[j].vertex[k].z = core_vertices[3 * j + k].z;
                }
                area_error_limit += (double)marked_facet[j].area();
            }

            // summarize area
            mclog::print() << "Total area of surface       : " 
                           << std::setprecision(8) << area_error_limit;
            mclog::print() << "Region error tolerance      : "
                           << (area_error_limit *= GlobalSettings::getInstance().errorTolerance());

            // build region-surface BC boolean map
            std::set<size_t> rel_sur;
            for (const size_t& j : neighbor) {
                std::vector<size_t> rel_sur_seg = rlist[j].correspondingIndex();
                for (const size_t& k : rel_sur_seg)
                    rel_sur.insert(k);
            }
            size_t map_size = *rel_sur.rbegin() + 1;
            size_t current_facet_index = 0;
            double stepsize = GlobalSettings::getInstance().stepsize();
            while (current_facet_index < marked_facet.size()) {
                size_t next_facet_index, stride;
                // Calculate OptiX calculation stride (image width)
                stride = BOOLEAN_MAP_MAXIMUM_SIZE_BYTES / map_size;
                next_facet_index = std::min(marked_facet.size(), current_facet_index + stride);
                stride = next_facet_index - current_facet_index;

                bool* geo_boolean_map_host[2];  // outward - inward
                std::vector<float3> beam_pos_host[2], beam_dir_host[2];

                for (size_t side = 0; side < 2; ++side) {
                    geo_boolean_map_host[side] = new bool[stride * map_size];
                    beam_pos_host[side].resize(stride);
                    beam_dir_host[side].resize(stride);
                }

                // calculate beam position and beam direction
                for (size_t f = 0; f < stride; ++f) {
                    float3 center = marked_facet[current_facet_index + f].center();
                    float3 normal = marked_facet[current_facet_index + f].normal();
                    for (size_t side = 0; side < 2; ++side) {
                        double epsilon;
                        geo_boolean_map_host[side][f * map_size + i] = (bool)side;
                        beam_dir_host[side][f] = normal;
                        beam_pos_host[side][f] = center;
                        epsilon = std::max(
                            std::abs((double)beam_pos_host[side][f].x), 
                            std::abs((double)beam_pos_host[side][f].y)
                        );
                        epsilon = std::max(epsilon, std::abs((double)beam_pos_host[side][f].z));
                        epsilon *= stepsize;
                        beam_pos_host[side][f].x += normal.x * epsilon;
                        beam_pos_host[side][f].y += normal.y * epsilon;
                        beam_pos_host[side][f].z += normal.z * epsilon;
                        normal.x = -normal.x;
                        normal.y = -normal.y;
                        normal.z = -normal.z;
                    }
                }

                ParamsCorefine param_host;
                CUdeviceptr param_device;
                bool* is_inside_dev;
                float3* beam_pos_dev, * beam_dir_dev;

                bool* is_inside_host;
                is_inside_host = new bool[stride];

                CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&is_inside_dev),
                    sizeof(bool) * stride));
                CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&beam_pos_dev),
                    sizeof(float3) * stride));
                CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&beam_dir_dev),
                    sizeof(float3) * stride));
                param_host.is_inside = is_inside_dev;
                param_host.size = stride;

                CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&param_device),
                    sizeof(ParamsCorefine)));

                // Iterate over surface
                for (size_t side = 0; side < 2; ++side) {
                    CUDA_CHECK(cudaMemcpy(beam_pos_dev, &beam_pos_host[side][0],
                        sizeof(float3) * stride, cudaMemcpyHostToDevice));
                    CUDA_CHECK(cudaMemcpy(beam_dir_dev, &beam_dir_host[side][0],
                        sizeof(float3) * stride, cudaMemcpyHostToDevice));
                    param_host.beam_pos = beam_pos_dev;
                    param_host.beam_dir = beam_dir_dev;
                    for (const size_t& target_sur : rel_sur) {
                        if (target_sur == i) {
                            for (size_t f = 0; f < stride; ++f) {
                                is_inside_host[f] = (bool)side;
                            }
                        }
                        else {
                            param_host.handle = slist[target_sur].getInitialSceneHandle();
                            CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(param_device), &param_host,
                                sizeof(ParamsCorefine), cudaMemcpyHostToDevice));
                            // Launch OptiX
                            OPTIX_CHECK(optixLaunch(_pipeline, stream, param_device, sizeof(ParamsCorefine), &_sbt, stride, 1, 1));
                            CUDA_SYNC_CHECK();
                            // Get traced result
                            CUDA_CHECK(cudaMemcpy(is_inside_host, is_inside_dev,
                                sizeof(bool)* stride, cudaMemcpyDeviceToHost));
                        }
                        // Set region-surface boolean map
                        for (size_t f = 0; f < stride; ++f) {
                            geo_boolean_map_host[side][f * map_size + target_sur] = is_inside_host[f];
                        }
                    }
                }

                // Set facet-region index
                for (size_t f = 0; f < stride; ++f) {
                    size_t count_out = 0, count_in = 0;
                    for (const size_t& reg : neighbor) {
                        bool is_in_out = rlist[reg].inside(&geo_boolean_map_host[0][f * map_size]);
                        bool is_in_in = rlist[reg].inside(&geo_boolean_map_host[1][f * map_size]);
                        if (is_in_out) {
                            marked_facet[current_facet_index + f].region_idx_frontface = reg;
                            count_out++;
                        }
                        if (is_in_in) {
                            marked_facet[current_facet_index + f].region_idx_backface = reg;
                            count_in++;
                            if (is_atomic) {
                                if (voxel_backface_idx < 0)
                                    voxel_backface_idx = reg;
                                else if (voxel_backface_idx != (int64_t)reg) {
                                    mclog::fatal() << slist[i].name()
                                                   << " is atomic boundary surface, its inside must be a single region";
                                }
                            }
                        }
                    }
                    if (!count_out && !count_in) {
                        marked_facet[current_facet_index + f].region_idx_frontface = 0;
                        marked_facet[current_facet_index + f].region_idx_backface = 0;
                    }
                    else if (count_out == 0) {
                        float3 cc = marked_facet[current_facet_index + f].center();
                        pos_fmt.clear();
                        pos_fmt << cc.x << cc.y << cc.z;
                        mclog::warning() << "Region determination error at position ("
                                         << pos_fmt.str()
                                         << ") cm, in facet "
                                         << std::to_string(current_facet_index + f)
                                         << ", this facet has no adjecent outward region";
                        area_error_cumul += (double)marked_facet[current_facet_index + f].area();
                    }
                    else if (count_out > 1) {
                        float3 cc = marked_facet[current_facet_index + f].center();
                        pos_fmt.clear();
                        pos_fmt << cc.x << cc.y << cc.z;
                        mclog::warning() << "Region determination error at position ("
                                         << pos_fmt.str()
                                         << ") cm, in facet "
                                         << std::to_string(current_facet_index + f)
                                         << ", this facet has more than two adjecent outward region";
                        area_error_cumul += (double)marked_facet[current_facet_index + f].area();
                    }
                    else if (count_in == 0) {
                        float3 cc = marked_facet[current_facet_index + f].center();
                        pos_fmt.clear();
                        pos_fmt << cc.x << cc.y << cc.z;
                        mclog::warning() << "Region determination error at position ("
                                         << pos_fmt.str()
                                         << ") cm, in facet "
                                         << std::to_string(current_facet_index + f)
                                         << ", this facet has no adjecent inward region";
                        area_error_cumul += (double)marked_facet[current_facet_index + f].area();
                    }
                    else if (count_in > 1) {
                        float3 cc = marked_facet[current_facet_index + f].center();
                        pos_fmt.clear();
                        pos_fmt << cc.x << cc.y << cc.z;
                        mclog::warning() << "Region determination error at position ("
                                         << pos_fmt.str()
                                         << ") cm, in facet "
                                         << std::to_string(current_facet_index + f)
                                         << ", this facet has more than two adjecent inward region";
                        area_error_cumul += (double)marked_facet[current_facet_index + f].area();
                    }

                    if (area_error_cumul > area_error_limit)
                        mclog::fatal() << "Too many region error";
                }

                delete[] geo_boolean_map_host[0];
                delete[] geo_boolean_map_host[1];

                CUDA_CHECK(cudaFree(is_inside_dev));
                CUDA_CHECK(cudaFree(beam_pos_dev));
                CUDA_CHECK(cudaFree(beam_dir_dev));
                CUDA_CHECK(cudaFree(reinterpret_cast<void*>(param_device)));

                delete[] is_inside_host;

                // Set corefined surface data
                for (const geo::MarkedTriangle& tri : marked_facet) {
                    if (tri.region_idx_backface != tri.region_idx_frontface) {
                        world.push_back(tri);
                        n_final_facets++;
                    }
                }
                current_facet_index = next_facet_index;
            }

            if (surface_handle.hasVoxel() && is_atomic)
                this->_voxel_region_idx = (int64_t)voxel_backface_idx;

            // summarize Optix BC result
            mclog::print() << "Region error                : "
                            << std::setprecision(8)
                            << area_error_cumul;
            mclog::print() << "Number of facet (selfBC ex) : " << n_final_facets;
        }

        this->_world = world;
    }


    int64_t OptixWorldGenerator::voxelRegionIndex() const {
        return this->_voxel_region_idx;
    }


    const std::vector<MarkedTriangle>& OptixWorldGenerator::world() const {
        return this->_world;
    }


    WorldFactory::WorldFactory(
        const LogicalSurfaceHandler& surface_handle,
        const LogicalRegionHandler& region_handle,
        const OptixWorldGenerator& generator
    ) : World() {
        const std::vector<geo::Region>&  rlist = region_handle.items();

        // region
        for (const geo::Region& region : rlist)
            this->_region.push_back(region.name());

        // mesh
        this->_mesh = generator.world();

        // voxel
        if (surface_handle.hasVoxel()) {
            const geo::Voxel& voxel   = surface_handle.voxel();
            size_t voxel_region_index = (size_t)generator.voxelRegionIndex();
            int3 voxel_shape          = voxel.shape();
            const std::vector<uint16_t>& 
                voxel_data            = voxel.array();
            mcutil::Affine inv_trans  = voxel.inverse();

            // convert voxel skin region idx to region idx
            for (MarkedTriangle& facet : this->_mesh) {
                if (facet.region_idx_backface  == voxel_region_index ||
                    facet.region_idx_frontface == voxel_region_index) {
                    double3 pos = mcutil::transform(facet.d_center(), inv_trans);
                    pos.x += 0.5;
                    pos.y += 0.5;
                    pos.z += 0.5;  // offset
                    int3    idx;
                    size_t  idx_1d;

                    idx.x = (int)pos.x;
                    idx.y = (int)pos.y;
                    idx.z = (int)pos.z;
                    idx.x = std::max(std::min(voxel_shape.x - 1, idx.x), 0);
                    idx.y = std::max(std::min(voxel_shape.y - 1, idx.y), 0);
                    idx.z = std::max(std::min(voxel_shape.z - 1, idx.z), 0);
                    idx_1d = idx.z + voxel_shape.z * (idx.y + voxel_shape.y * idx.x);
                    int reg_new = voxel_data[idx_1d];
                    if (facet.region_idx_backface  == voxel_region_index)
                        facet.region_idx_backface  = reg_new;
                    if (facet.region_idx_frontface == voxel_region_index)
                        facet.region_idx_frontface = reg_new;
                }
            }

            // build voxel internal planes
            std::vector<MarkedTriangle> vin;
            vin = voxel.getInterior();

            this->_mesh.insert(this->_mesh.end(), vin.begin(), vin.end());
        }
        
    }


    void WorldFactory::writeRegions(const std::string& proj_name) {

        typedef CGAL::Simple_cartesian<double> Kernel;
        typedef Kernel::Point_3 Point;
        typedef CGAL::Surface_mesh<Point> Mesh;

        for (int i = 0; i < this->_region.size(); ++i) {
            // polygon soup
            std::vector<Point> points;
            std::vector<std::vector<std::size_t>> polygons;

            // triangles
            for (const auto& facet : this->_mesh) {
                if (facet.region_idx_frontface == i) {
                    size_t offset = points.size();
                    for (int k = 0; k < 3; ++k)
                        points.push_back(Point(facet.vertex[k].x, facet.vertex[k].y, facet.vertex[k].z));
                    polygons.push_back({ offset + 2, offset + 1, offset });
                }
                if (facet.region_idx_backface == i) {
                    size_t offset = points.size();
                    for (int k = 0; k < 3; ++k)
                        points.push_back(Point(facet.vertex[k].x, facet.vertex[k].y, facet.vertex[k].z));
                    polygons.push_back({ offset, offset + 1, offset + 2 });
                }
            }

            if (!points.empty()) {
                Mesh mesh;
                CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(points, polygons, mesh);
                std::string name = this->_region[i];
                name += "_reg.off";
                std::ofstream out(name);
                out << mesh << std::endl;
            }
        }
    }


}