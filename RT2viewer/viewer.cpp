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
 * @file    RT2viewer/viewer.cpp
 * @brief   RT2viewer main (GLFW referenced from the OptiX SDK examples)
 * @author  CM Lee
 * @date    05/23/2023
 */


#include <optix.h>
#include <optix_function_table_definition.h>
#include <optix_stack_size.h>
#include <optix_stubs.h>

#include <cuda_runtime.h>

#include <sampleConfig.h>

#include <sutil/CUDAOutputBuffer.h>
#include <sutil/Camera.h>
#include <sutil/Exception.h>
#include <sutil/GLDisplay.h>
#include <sutil/Matrix.h>
#include <sutil/Trackball.h>
#include <sutil/sutil.h>
#include <sutil/vec_math.h>
#include <optix_stack_size.h>

#include <GLFW/glfw3.h>

#include "ptx/render.h"
#include "geometry/color.hpp"
#include "geometry/surface.hpp"
#include "mcutil/world/world.hpp"
#include "prompt/prompt.hpp"

#include <iomanip>
#include <iostream>
#include <string>


bool resize_dirty = false;
bool minimized = false;

int target = -1;
int method = 0;
std::vector<std::string> region_name;

// Camera state
bool             camera_changed = true;
sutil::Camera    camera;
sutil::Trackball trackball;

// Mouse state
int32_t mouse_button       = -1;
int32_t samples_per_launch = 16;
bool    mouse_double_click = false;
std::chrono::steady_clock::time_point click_time_1, click_time_2;
sutil::CUDAOutputBuffer<float3>*      hit_position_global;
sutil::CUDAOutputBuffer<int>*         hit_activated_global;

template <typename T>
struct SbtRecord
{
    __align__(OPTIX_SBT_RECORD_ALIGNMENT) char header[OPTIX_SBT_RECORD_HEADER_SIZE];
    T data;
};

typedef SbtRecord<RayGenData>   RayGenSbtRecord;
typedef SbtRecord<MissData>     MissSbtRecord;
typedef SbtRecord<HitGroupData> HitGroupSbtRecord;

struct PathTracerState {
    OptixDeviceContext context = 0;

    OptixTraversableHandle         gas_handle               = 0;  // Traversable handle for triangle AS
    CUdeviceptr                    d_gas_output_buffer      = 0;  // Triangle AS memory
    CUdeviceptr                    d_vertex_buffer          = 0;
    CUdeviceptr                    d_region_idx_backface    = 0;
    CUdeviceptr                    d_region_idx_frontface   = 0;
    CUdeviceptr                    d_color_palette          = 0;

    OptixModule                    module                   = 0;
    OptixPipelineCompileOptions    pipeline_compile_options = {};
    OptixPipeline                  pipeline                 = 0;

    OptixProgramGroup              raygen_prog_group        = 0;
    OptixProgramGroup              miss_prog_group          = 0;
    OptixProgramGroup              hitgroup_prog_group      = 0;

    CUstream                       stream                   = 0;
    Params                         params;
    Params*                        d_params;

    OptixShaderBindingTable        sbt                    = {};
};

static void context_log_cb(unsigned int level, const char* tag, const char* message, void* /*cbdata */)
{
    std::cerr << "[" << std::setw(2) << level << "][" << std::setw(12) << tag << "]: "
        << message << "\n";
}

void initCameraState(const float3& eye, const float3& look, const float& fov) {
    camera.setEye(eye);
    camera.setLookat(look);
    camera.setUp(make_float3(0.0f, 1.0f, 0.0f));
    camera.setFovY(fov);
    camera_changed = true;

    trackball.setCamera(&camera);
    trackball.setMoveSpeed(10.0f);
    trackball.setReferenceFrame(
        make_float3(1.0f, 0.0f, 0.0f),
        make_float3(0.0f, 0.0f, 1.0f),
        make_float3(0.0f, 1.0f, 0.0f)
    );
    trackball.setGimbalLock( false );
}

//------------------------------------------------------------------------------
//
// GLFW callbacks
//
//------------------------------------------------------------------------------

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (action == GLFW_PRESS)
    {
        mouse_button = button;
        click_time_2 = std::chrono::steady_clock::now();
        std::chrono::duration<double> click_interval = click_time_2 - click_time_1;
        click_time_1 = click_time_2;
        trackball.startTracking(static_cast<int>(xpos), static_cast<int>(ypos));

        if (click_interval.count() < 0.2) {
            mouse_double_click = true;
        }
        else {
            mouse_double_click = false;
        }
    }
    else
    {
        mouse_button = -1;
    }
}

static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    Params& params = static_cast<PathTracerState*>(glfwGetWindowUserPointer(window))->params;
    if (mouse_double_click) {
        int map_position = params.width * (params.height - static_cast<int>(ypos)) + static_cast<int>(xpos);
        int* act = hit_activated_global->getHostPointer();
        if (act[map_position]) {
            float3* pos = hit_position_global->getHostPointer();
            trackball.updateViewPoint(pos[map_position]);
            camera_changed = true;
        }
        mouse_double_click = false;
    }
    else {
        if (mouse_button == GLFW_MOUSE_BUTTON_LEFT)
        {
            trackball.setViewMode(sutil::Trackball::LookAtFixed);
            trackball.updateTracking(static_cast<int>(xpos), static_cast<int>(ypos), params.width, params.height);
            camera_changed = true;
        }
        else if (mouse_button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            trackball.setViewMode(sutil::Trackball::EyeFixed);
            trackball.updateTracking(static_cast<int>(xpos), static_cast<int>(ypos), params.width, params.height);
            camera_changed = true;
        }
    }
}

static void windowSizeCallback(GLFWwindow* window, int32_t res_x, int32_t res_y)
{
    // Keep rendering at the current resolution when the window is minimized.
    if (minimized)
        return;

    // Output dimensions must be at least 1 in both x and y.
    sutil::ensureMinimumSize(res_x, res_y);

    Params& params = static_cast<PathTracerState*>(glfwGetWindowUserPointer(window))->params;
    params.width = res_x;
    params.height = res_y;
    camera_changed = true;
    resize_dirty = true;
}

static void windowIconifyCallback(GLFWwindow* window, int32_t iconified)
{
    minimized = (iconified > 0);
}

void updatePipeline(PathTracerState& state) {
    state.params.target = target;
    state.params.method = method;
}

static void keyCallback(GLFWwindow* window, int32_t key, int32_t /*scancode*/, int32_t action, int32_t /*mods*/)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
        {
            glfwSetWindowShouldClose(window, true);
        }
    }
    else if (key == GLFW_KEY_G)
    {
        // toggle UI draw
    }
    else if (key == GLFW_KEY_S)
    {

    }
}

static void charCallback(GLFWwindow* window, unsigned int codepoint)
{
    if (codepoint == '=')
    {
        target++;
        if (target >= region_name.size())
            target = -1;
        updatePipeline(*static_cast<PathTracerState*>(glfwGetWindowUserPointer(window)));
    }
    else if (codepoint == '-')
    {
        target--;
        if (target < -1)
            target = region_name.size() - 1;
        updatePipeline(*static_cast<PathTracerState*>(glfwGetWindowUserPointer(window)));
    }
    else if (codepoint == 'm')
    {
        method = (method + 1) % 2;
        updatePipeline(*static_cast<PathTracerState*>(glfwGetWindowUserPointer(window)));
    }
}

static void scrollCallback(GLFWwindow* window, double xscroll, double yscroll)
{
    if (trackball.wheelEvent((int)yscroll))
        camera_changed = true;
}

void createContext(PathTracerState& state) {
    // Initialize CUDA
    CUDA_CHECK(cudaFree(0));

    OptixDeviceContext context;
    CUcontext          cuCtx = 0;  // zero means take the current context
    OPTIX_CHECK(optixInit());
    OptixDeviceContextOptions options = {};
    options.logCallbackFunction       = &context_log_cb;
    options.logCallbackLevel          = 4;
    OPTIX_CHECK(optixDeviceContextCreate(cuCtx, &options, &context));

    state.context = context;
}

void buildMeshAccel(PathTracerState& state, std::string file_name) {
    // Build quadratic surface list and world
    geo::World world(file_name);

    // set region name
    region_name = world.region();

    OptixAccelBuildOptions accel_options = {};
    accel_options.buildFlags = OPTIX_BUILD_FLAG_ALLOW_COMPACTION | OPTIX_BUILD_FLAG_ALLOW_RANDOM_VERTEX_ACCESS;
    accel_options.operation = OPTIX_BUILD_OPERATION_BUILD;

    // world build input
    std::vector<float3> vertex;
    std::vector<int> region_idx_backface;
    std::vector<int> region_idx_frontface;
    std::vector<float3> color_palette;

    std::vector<geo::MarkedTriangle> mesh;
    // if (world.has_voxel())
    //     mesh = geo::convertVoxelToWorld(world);

    const std::vector<geo::MarkedTriangle>& mesh_world = world.mesh();
    mesh.insert(mesh.end(), mesh_world.begin(), mesh_world.end());

    size_t world_size = mesh.size();

    vertex.reserve(world_size);
    region_idx_backface.reserve(world_size);
    region_idx_frontface.reserve(world_size);

    int max_region_idx = -1;

    for (const geo::MarkedTriangle& facet : mesh) {
        for (int j = 0; j < 3; ++j)
            vertex.push_back(facet.vertex[j]);
        region_idx_backface.push_back(facet.region_idx_backface);
        region_idx_frontface.push_back(facet.region_idx_frontface);
        max_region_idx = max(max_region_idx, region_idx_backface.back());
        max_region_idx = max(max_region_idx, region_idx_frontface.back());
    }
    max_region_idx++;
    // generate color palette
    color_palette.resize(max_region_idx);
    for (int i = 0; i < color_palette.size(); ++i) {
        size_t palette_length = shader::COLOR_PALETTE.size();
        color_palette[i] = shader::COLOR_PALETTE[i % palette_length];
    }

    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&state.d_vertex_buffer),
        sizeof(float3) * world_size * 3));
    CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(state.d_vertex_buffer), &vertex[0],
        sizeof(float3) * world_size * 3, cudaMemcpyHostToDevice));
    state.params.vertices = reinterpret_cast<float3*>(state.d_vertex_buffer);

    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&state.d_region_idx_backface),
        sizeof(int) * world_size));
    CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(state.d_region_idx_backface),
        &region_idx_backface[0], sizeof(int) * world_size, cudaMemcpyHostToDevice));
    state.params.region_idx_backface = reinterpret_cast<int*>(state.d_region_idx_backface);

    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&state.d_region_idx_frontface),
        sizeof(int) * world_size));
    CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(state.d_region_idx_frontface),
        &region_idx_frontface[0], sizeof(int) * world_size, cudaMemcpyHostToDevice));
    state.params.region_idx_frontface = reinterpret_cast<int*>(state.d_region_idx_frontface);

    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&state.d_color_palette),
        sizeof(float3) * color_palette.size()));
    CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(state.d_color_palette),
        &color_palette[0], sizeof(float3) * color_palette.size(), cudaMemcpyHostToDevice));
    state.params.color_palette = reinterpret_cast<float3*>(state.d_color_palette);

    OptixBuildInput triangle_input = {};

    triangle_input.type = OPTIX_BUILD_INPUT_TYPE_TRIANGLES;
    triangle_input.triangleArray.vertexBuffers = &state.d_vertex_buffer;
    triangle_input.triangleArray.vertexFormat = OPTIX_VERTEX_FORMAT_FLOAT3;
    triangle_input.triangleArray.vertexStrideInBytes = sizeof(float3);
    triangle_input.triangleArray.numVertices = world_size * 3;

    uint32_t triangle_input_flags[1] = { OPTIX_GEOMETRY_FLAG_DISABLE_ANYHIT };
    triangle_input.triangleArray.flags = triangle_input_flags;
    triangle_input.triangleArray.numSbtRecords = 1;

    OptixAccelBufferSizes gas_buffer_sizes;
    OPTIX_CHECK(optixAccelComputeMemoryUsage(
        state.context,
        &accel_options,
        &triangle_input,
        1, // Number of build inputs
        &gas_buffer_sizes
    ));
    CUdeviceptr d_temp_buffer_gas;
    CUDA_CHECK(cudaMalloc(
        reinterpret_cast<void**>(&d_temp_buffer_gas),
        gas_buffer_sizes.tempSizeInBytes
    ));
    CUDA_CHECK(cudaMalloc(
        reinterpret_cast<void**>(&state.d_gas_output_buffer),
        gas_buffer_sizes.outputSizeInBytes
    ));

    OPTIX_CHECK(optixAccelBuild(
        state.context,
        0,             // CUDA stream
        &accel_options,
        &triangle_input,
        1,             // num build inputs
        d_temp_buffer_gas,
        gas_buffer_sizes.tempSizeInBytes,
        state.d_gas_output_buffer,
        gas_buffer_sizes.outputSizeInBytes,
        &state.gas_handle,
        nullptr,       // emitted property list
        0              // num emitted properties
    ));

    // We can now free the scratch space buffer used during build and the vertex
    // inputs, since they are not needed by our trivial shading method
    CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_temp_buffer_gas)));
}

void createModule(PathTracerState& state) {
    // Create module
    OptixModuleCompileOptions module_compile_options = {};
#if !defined( NDEBUG )
    module_compile_options.optLevel   = OPTIX_COMPILE_OPTIMIZATION_LEVEL_0;
    module_compile_options.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_FULL;
#endif

    state.pipeline_compile_options.usesMotionBlur        = false;
    state.pipeline_compile_options.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_GAS;
    state.pipeline_compile_options.numPayloadValues      = 6;
    state.pipeline_compile_options.numAttributeValues    = 0;
#ifdef DEBUG // Enables debug exceptions during optix launches. This may incur significant performance cost and should only be done during development.
    state.pipeline_compile_options.exceptionFlags = OPTIX_EXCEPTION_FLAG_DEBUG | OPTIX_EXCEPTION_FLAG_TRACE_DEPTH | OPTIX_EXCEPTION_FLAG_STACK_OVERFLOW;
#else
    state.pipeline_compile_options.exceptionFlags        = OPTIX_EXCEPTION_FLAG_NONE;
#endif
    state.pipeline_compile_options.pipelineLaunchParamsVariableName = "params";
    state.pipeline_compile_options.usesPrimitiveTypeFlags           = OPTIX_PRIMITIVE_TYPE_FLAGS_TRIANGLE;

    size_t      inputSize = 0;
    const char* input = sutil::getInputData(OPTIX_SAMPLE_NAME, OPTIX_SAMPLE_DIR, "render.cu", inputSize);

    if (!inputSize)
        mclog::fatalFileNotExist("render.cu");

    OPTIX_CHECK_LOG(optixModuleCreateFromPTX(
        state.context,
        &module_compile_options,
        &state.pipeline_compile_options,
        input,
        inputSize,
        LOG, &LOG_SIZE,
        &state.module
    ));
}

void createProgramGroup(PathTracerState& state) {

    OptixProgramGroupOptions program_group_options = {}; // Initialize to zeros
    OptixProgramGroupDesc raygen_prog_group_desc = {}; //
    raygen_prog_group_desc.kind = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;
    raygen_prog_group_desc.raygen.module = state.module;
    raygen_prog_group_desc.raygen.entryFunctionName = "__raygen__rg";
    OPTIX_CHECK_LOG(optixProgramGroupCreate(
        state.context,
        &raygen_prog_group_desc,
        1,   // num program groups
        &program_group_options,
        LOG, &LOG_SIZE,
        &state.raygen_prog_group
    ));

    OptixProgramGroupDesc miss_prog_group_desc = {};
    miss_prog_group_desc.kind = OPTIX_PROGRAM_GROUP_KIND_MISS;
    miss_prog_group_desc.miss.module = state.module;
    miss_prog_group_desc.miss.entryFunctionName = "__miss__ms";
    OPTIX_CHECK_LOG(optixProgramGroupCreate(
        state.context,
        &miss_prog_group_desc,
        1,   // num program groups
        &program_group_options,
        LOG, &LOG_SIZE,
        &state.miss_prog_group
    ));

    OptixProgramGroupDesc hitgroup_prog_group_desc = {};
    hitgroup_prog_group_desc.kind = OPTIX_PROGRAM_GROUP_KIND_HITGROUP;
    hitgroup_prog_group_desc.hitgroup.moduleCH = state.module;
    hitgroup_prog_group_desc.hitgroup.entryFunctionNameCH = "__closesthit__ch";
    OPTIX_CHECK_LOG(optixProgramGroupCreate(
        state.context,
        &hitgroup_prog_group_desc,
        1,   // num program groups
        &program_group_options,
        LOG, &LOG_SIZE,
        &state.hitgroup_prog_group
    ));
}

void createPipeline(PathTracerState& state) {
    const uint32_t    max_trace_depth = 1;
    OptixProgramGroup program_groups[] = { 
        state.raygen_prog_group, 
        state.miss_prog_group, 
        state.hitgroup_prog_group 
    };

    OptixPipelineLinkOptions pipeline_link_options = {};
    pipeline_link_options.maxTraceDepth = max_trace_depth;
#if !defined( NDEBUG )
    pipeline_link_options.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_FULL;
#endif
    OPTIX_CHECK_LOG(optixPipelineCreate(
        state.context,
        &state.pipeline_compile_options,
        &pipeline_link_options,
        program_groups,
        sizeof(program_groups) / sizeof(program_groups[0]),
        LOG, &LOG_SIZE,
        &state.pipeline
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
    OPTIX_CHECK(optixPipelineSetStackSize(state.pipeline, direct_callable_stack_size_from_traversal,
        direct_callable_stack_size_from_state, continuation_stack_size,
        1  // maxTraversableDepth
    ));
}

void createSBT(PathTracerState& state) {
    CUdeviceptr  raygen_record;
    const size_t raygen_record_size = sizeof(RayGenSbtRecord);
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&raygen_record), raygen_record_size));
    RayGenSbtRecord rg_sbt;
    OPTIX_CHECK(optixSbtRecordPackHeader(state.raygen_prog_group, &rg_sbt));
    CUDA_CHECK(cudaMemcpy(
        reinterpret_cast<void*>(raygen_record),
        &rg_sbt,
        raygen_record_size,
        cudaMemcpyHostToDevice
    ));

    CUdeviceptr miss_record;
    size_t      miss_record_size = sizeof(MissSbtRecord);
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&miss_record), miss_record_size));
    MissSbtRecord ms_sbt;
    OPTIX_CHECK(optixSbtRecordPackHeader(state.miss_prog_group, &ms_sbt));
    CUDA_CHECK(cudaMemcpy(
        reinterpret_cast<void*>(miss_record),
        &ms_sbt,
        miss_record_size,
        cudaMemcpyHostToDevice
    ));

    CUdeviceptr hitgroup_record;
    size_t      hitgroup_record_size = sizeof(HitGroupSbtRecord);
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&hitgroup_record), hitgroup_record_size));
    HitGroupSbtRecord hg_sbt;
    OPTIX_CHECK(optixSbtRecordPackHeader(state.hitgroup_prog_group, &hg_sbt));
    CUDA_CHECK(cudaMemcpy(
        reinterpret_cast<void*>(hitgroup_record),
        &hg_sbt,
        hitgroup_record_size,
        cudaMemcpyHostToDevice
    ));

    state.sbt.raygenRecord = raygen_record;
    state.sbt.missRecordBase = miss_record;
    state.sbt.missRecordStrideInBytes = sizeof(MissSbtRecord);
    state.sbt.missRecordCount = 1;
    state.sbt.hitgroupRecordBase = hitgroup_record;
    state.sbt.hitgroupRecordStrideInBytes = sizeof(HitGroupSbtRecord);
    state.sbt.hitgroupRecordCount = 1;
}

void initLaunchParams(PathTracerState& state) {
    
    state.params.image = nullptr;  // Will be set when output buffer is mapped

    state.params.subframe_index = 0u;

    state.params.handle = state.gas_handle;

    CUDA_CHECK(cudaStreamCreate(&state.stream));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&state.d_params), sizeof(Params)));

}

void handleCameraUpdate(Params& params) {
    if (!camera_changed)
        return;
    camera_changed = false;

    camera.setAspectRatio(static_cast<float>(params.width) / static_cast<float>(params.height));
    params.eye = camera.eye();
    camera.UVWFrame(params.U, params.V, params.W);
}

void handleResize(
    sutil::CUDAOutputBuffer<uchar4>& output_buffer, 
    sutil::CUDAOutputBuffer<float3>& hit_position,
    sutil::CUDAOutputBuffer<int>& hit_activated,
    Params& params
) {
    if (!resize_dirty)
        return;
    resize_dirty = false;

    output_buffer.resize(params.width, params.height);
    hit_position.resize(params.width, params.height);
    hit_activated.resize(params.width, params.height);
}

void updateState(
    sutil::CUDAOutputBuffer<uchar4>& output_buffer, 
    sutil::CUDAOutputBuffer<float3>& hit_position,
    sutil::CUDAOutputBuffer<int>& hit_activated,
    Params& params
) {
    // Update params on device
    if (camera_changed || resize_dirty)
        params.subframe_index = 0;

    handleCameraUpdate(params);
    handleResize(output_buffer, hit_position, hit_activated, params);
}

void launchSubframe(
    sutil::CUDAOutputBuffer<uchar4>& output_buffer, 
    sutil::CUDAOutputBuffer<float3>& hit_position,
    sutil::CUDAOutputBuffer<int>& hit_activated,
    PathTracerState& state
) {
    // Launch
    uchar4* result_buffer_data = output_buffer.map();
    float3* result_hit_position = hit_position.map();
    int* reslut_hit_activated = hit_activated.map();
    state.params.image = result_buffer_data;
    state.params.hit_position = result_hit_position;
    state.params.hit_activated = reslut_hit_activated;
    CUDA_CHECK(cudaMemcpyAsync(
        reinterpret_cast<void*>(state.d_params),
        &state.params, sizeof(Params),
        cudaMemcpyHostToDevice, state.stream
    ));

    OPTIX_CHECK(optixLaunch(
        state.pipeline,
        state.stream,
        reinterpret_cast<CUdeviceptr>(state.d_params),
        sizeof(Params),
        &state.sbt,
        state.params.width,   // launch width
        state.params.height,  // launch height
        1                     // launch depth
    ));
    output_buffer.unmap();
    hit_position.unmap();
    hit_activated.unmap();
    CUDA_SYNC_CHECK();
}

void displaySubframe(sutil::CUDAOutputBuffer<uchar4>& output_buffer, sutil::GLDisplay& gl_display, GLFWwindow* window)
{
    // Display
    int framebuf_res_x = 0;  // The display's resolution (could be HDPI res)
    int framebuf_res_y = 0;  //
    glfwGetFramebufferSize(window, &framebuf_res_x, &framebuf_res_y);
    gl_display.display(
        output_buffer.width(),
        output_buffer.height(),
        framebuf_res_x,
        framebuf_res_y,
        output_buffer.getPBO()
    );
}

void displaySpecializationInfo(GLFWwindow* window)
{
    static char display_text[1024];

    sutil::beginFrameImGui();
    std::string target_name;
    if (target == -1)
        target_name = "all";
    else
        target_name = region_name[target];
    std::string render_mode;
    if (method)
        render_mode = "tr";
    else
        render_mode = "fh";
    sprintf(display_text,
        "Region name  [+/-]: %s\nRendering mode [m]: %s", target_name.c_str(), render_mode.c_str()
        );
    Params& params = static_cast<PathTracerState*>(glfwGetWindowUserPointer(window))->params;
    sutil::displayText(display_text, 10.0f, (float)params.height - 40.f);
    sutil::endFrameImGui();
}

int main(int argc, char* argv[]) {
    RenderSettings settings = interpretCommandLine(argc, argv);

    PathTracerState state;
    click_time_1 = std::chrono::steady_clock::now();
    // Initialize camera
    state.params.target = -1;
    state.params.method = 0;
    state.params.width  = settings.resolution.x;
    state.params.height = settings.resolution.y;
    sutil::CUDAOutputBufferType output_buffer_type = sutil::CUDAOutputBufferType::CUDA_DEVICE;
    
    shader::initializeColorSpace();
    
    createContext(state);
    try {
        buildMeshAccel(state, settings.input);
    }
    catch (std::exception& e) {
        std::cout << "Fail to open world file '" 
                  << settings.input << "'" << std::endl;
        exit(1);
    }
    createModule(state);
    createProgramGroup(state);
    createPipeline(state);
    createSBT(state);
    initLaunchParams(state);

    initCameraState(settings.eye, settings.look, settings.fov);

    GLFWwindow* window = sutil::initUI("geoViewer", state.params.width, state.params.height);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetWindowSizeCallback(window, windowSizeCallback);
    glfwSetWindowIconifyCallback(window, windowIconifyCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCharCallback(window, charCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetWindowUserPointer(window, &state);

    // Render loop
    {
        sutil::CUDAOutputBuffer<uchar4> output_buffer(
            output_buffer_type,
            state.params.width,
            state.params.height
        );

        sutil::CUDAOutputBuffer<float3> hit_position(
            output_buffer_type,
            state.params.width,
            state.params.height
        );

        sutil::CUDAOutputBuffer<int> hit_activated(
            output_buffer_type,
            state.params.width,
            state.params.height
        );

        hit_position_global  = &hit_position;
        hit_activated_global = &hit_activated;

        output_buffer.setStream(state.stream);
        sutil::GLDisplay gl_display;

        std::chrono::duration<double> state_update_time(0.0);
        std::chrono::duration<double> render_time(0.0);
        std::chrono::duration<double> display_time(0.0);

        do
        {
            auto t0 = std::chrono::steady_clock::now();
            glfwPollEvents();

            updateState(output_buffer, hit_position, hit_activated, state.params);
            auto t1 = std::chrono::steady_clock::now();
            state_update_time += t1 - t0;
            t0 = t1;

            launchSubframe(output_buffer, hit_position, hit_activated, state);
            t1 = std::chrono::steady_clock::now();
            render_time += t1 - t0;
            t0 = t1;

            displaySubframe(output_buffer, gl_display, window);
            t1 = std::chrono::steady_clock::now();
            display_time += t1 - t0;

            sutil::displayStats(state_update_time, render_time, display_time);

            displaySpecializationInfo(window);

            glfwSwapBuffers(window);

            ++state.params.subframe_index;
        } while (!glfwWindowShouldClose(window));
        CUDA_SYNC_CHECK();
    }
    sutil::cleanupUI(window);

    return 0;
}