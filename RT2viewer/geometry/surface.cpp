
#include <sutil/CUDAOutputBuffer.h>
#include <sutil/sutil.h>
#include <sutil/Exception.h>

#include "surface.hpp"

namespace geo {

    /*
    OptixQuadratic::OptixQuadratic(
        SURFACE_TYPE type,
        float3       center,
        float        radius,
        float3       height
    ) : _type(type),
        _center(center),
        _radius(radius),
        _height(height),
        _gas_handle(OptixTraversableHandle(0x0)),
        _d_gas_output_buffer(0x0) {}

    OptixQuadratic OptixQuadraticFactory::generate(
        SURFACE_TYPE        type, 
        OptixDeviceContext* context,
        float3              center,
        float               radius,
        float3              height
    ) {
        switch (type) {
        case SURFACE_TYPE::SPHERE:
            return _cylinder(context, center, radius, 
                make_float3(0.f, 0.f, 1e-8f), true);
        case SURFACE_TYPE::CYLINDER:
            return _cylinder(context, center, radius, height, true);
        case SURFACE_TYPE::CONE:
            return _cylinder(context, center, radius, height, false);
        }
        return OptixQuadratic(type, center, radius, height);
    }

    OptixQuadratic OptixQuadraticFactory::_sphere(
        OptixDeviceContext* context,
        float3              center,
        float               radius,
        float3              height
        ) {
        OptixQuadratic quad(SURFACE_TYPE::SPHERE, center, radius, height);

        OptixAccelBuildOptions accel_options = {};
        accel_options.buildFlags = OPTIX_BUILD_FLAG_PREFER_FAST_TRACE | OPTIX_BUILD_FLAG_ALLOW_RANDOM_VERTEX_ACCESS;
        accel_options.operation  = OPTIX_BUILD_OPERATION_BUILD;

        // sphere build input
        CUdeviceptr d_vertex_buffer;
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_vertex_buffer), sizeof(float3)));
        CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_vertex_buffer), &center,
            sizeof(float3), cudaMemcpyHostToDevice));

        CUdeviceptr d_radius_buffer;
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_radius_buffer), sizeof(float)));
        CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_radius_buffer), &radius, 
            sizeof(float), cudaMemcpyHostToDevice));

        OptixBuildInput sphere_input = {};

        sphere_input.type                      = OPTIX_BUILD_INPUT_TYPE_SPHERES;
        sphere_input.sphereArray.vertexBuffers = &d_vertex_buffer;
        sphere_input.sphereArray.numVertices   = 1;
        sphere_input.sphereArray.radiusBuffers = &d_radius_buffer;

        uint32_t sphere_input_flags[1]         = { OPTIX_GEOMETRY_FLAG_NONE };
        sphere_input.sphereArray.flags         = sphere_input_flags;
        sphere_input.sphereArray.numSbtRecords = 1;

        OptixAccelBufferSizes gas_buffer_sizes;
        OPTIX_CHECK(optixAccelComputeMemoryUsage(*context, &accel_options, 
            &sphere_input, 1, &gas_buffer_sizes));

        CUdeviceptr d_temp_buffer_gas;
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_temp_buffer_gas), 
            gas_buffer_sizes.tempSizeInBytes));
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&quad._d_gas_output_buffer), 
            gas_buffer_sizes.outputSizeInBytes));

        OPTIX_CHECK(optixAccelBuild(
            *context,
            0,  // CUDA stream
            &accel_options,
            &sphere_input,
            1,  // num build inputs
            d_temp_buffer_gas,
            gas_buffer_sizes.tempSizeInBytes,
            quad._d_gas_output_buffer,
            gas_buffer_sizes.outputSizeInBytes,
            &quad._gas_handle,
            nullptr,  // emitted property list
            0  // num emitted properties
        ));

        // We can now free the scratch space buffer used during build and the vertex
        // inputs, since they are not needed by our trivial shading method
        CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_temp_buffer_gas)));
        CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_vertex_buffer)));
        CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_radius_buffer)));

        return quad;
    }

    OptixQuadratic OptixQuadraticFactory::_cylinder(
        OptixDeviceContext* context,
        float3              center,
        float               radius,
        float3              height,
        bool                is_cylinder
    ) {
        OptixQuadratic quad = is_cylinder
            ? OptixQuadratic(SURFACE_TYPE::CYLINDER, center, radius, height)
            : OptixQuadratic(SURFACE_TYPE::CONE, center, radius, height);

        OptixAccelBuildOptions accel_options = {};
        accel_options.buildFlags = OPTIX_BUILD_FLAG_PREFER_FAST_TRACE | OPTIX_BUILD_FLAG_ALLOW_RANDOM_VERTEX_ACCESS;
        accel_options.operation = OPTIX_BUILD_OPERATION_BUILD;

        float3 vertices[2];
        float  widths[2];
        vertices[0]    = center;
        vertices[1]    = center;
        vertices[1].x += height.x;
        vertices[1].y += height.y;
        vertices[1].z += height.z;
        widths[0]      = radius;
        widths[1]      = is_cylinder ? radius : 0.f;

        CUdeviceptr d_vertices = 0x0;
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_vertices), sizeof(float3) * 2));
        CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_vertices), vertices, 
            sizeof(float3) * 2, cudaMemcpyHostToDevice));
        CUdeviceptr d_widths = 0x0;
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_widths), sizeof(float) * 2));
        CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_widths), widths,
            sizeof(float) * 2, cudaMemcpyHostToDevice));

        // Curve build intput: with a single segment the index array
        // contains index of first vertex.
        const std::array<int, 1> segmentIndices = { 0 };
        const size_t             segmentIndicesSize = sizeof(int) * segmentIndices.size();
        CUdeviceptr              d_segmentIndices = 0;
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_segmentIndices), segmentIndicesSize));
        CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_segmentIndices), segmentIndices.data(),
            segmentIndicesSize, cudaMemcpyHostToDevice));

        // Curve build input.
        OptixBuildInput curve_input = {};
        curve_input.type                            = OPTIX_BUILD_INPUT_TYPE_CURVES;
        curve_input.curveArray.curveType            = OPTIX_PRIMITIVE_TYPE_ROUND_LINEAR;
        curve_input.curveArray.numPrimitives        = 1;
        curve_input.curveArray.vertexBuffers        = &d_vertices;
        curve_input.curveArray.numVertices          = 2;
        curve_input.curveArray.vertexStrideInBytes  = sizeof(float3);
        curve_input.curveArray.widthBuffers         = &d_widths;
        curve_input.curveArray.widthStrideInBytes   = sizeof(float);
        curve_input.curveArray.normalBuffers        = 0;
        curve_input.curveArray.normalStrideInBytes  = 0;
        curve_input.curveArray.indexBuffer          = d_segmentIndices;
        curve_input.curveArray.indexStrideInBytes   = sizeof(int);
        curve_input.curveArray.flag                 = OPTIX_GEOMETRY_FLAG_NONE;
        curve_input.curveArray.primitiveIndexOffset = 0;

        OptixAccelBufferSizes gas_buffer_sizes;
        OPTIX_CHECK(optixAccelComputeMemoryUsage(*context, &accel_options,
            &curve_input, 1, &gas_buffer_sizes));

        CUdeviceptr d_temp_buffer_gas;
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_temp_buffer_gas),
            gas_buffer_sizes.tempSizeInBytes));
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&quad._d_gas_output_buffer),
            gas_buffer_sizes.outputSizeInBytes));

        OPTIX_CHECK(optixAccelBuild(
            *context,
            0,  // CUDA stream
            &accel_options,
            &curve_input,
            1,  // num build inputs
            d_temp_buffer_gas,
            gas_buffer_sizes.tempSizeInBytes,
            quad._d_gas_output_buffer,
            gas_buffer_sizes.outputSizeInBytes,
            &quad._gas_handle,
            nullptr,  // emitted property list
            0  // num emitted properties
        ));

        // We can now free the scratch space buffer used during build and the vertex
        // inputs, since they are not needed by our trivial shading method
        CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_temp_buffer_gas)));
        CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_vertices)));
        CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_widths)));
        CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_segmentIndices)));

        return quad;
    }
    */

    /*
    std::vector<MarkedTriangle> convertVoxelToWorld(const World& world) {
        std::vector<MarkedTriangle> out;
        const std::vector<int>& data = world.voxel_data();
        int3 shape = world.voxel_shape();
        double3 size = world.voxel_size();
        int reg_idx = world.voxel_region_index();
        const std::vector<double>& transformer = world.transformer();
        // build planes
        for (size_t i = 1; i < shape.x; ++i) {
            for (size_t j = 1; j < shape.y; ++j) {
                for (size_t k = 1; k < shape.z; ++k) {
                    double3 pos;
                    pos.x = (double)i * size.x;
                    pos.y = (double)j * size.y;
                    pos.z = (double)k * size.z;
                    int current_reg = data[(i * shape.y + j) * shape.z + k];
                    // z plane
                    {
                        MarkedTriangle zm1, zm2;
                        int idx_front, idx_back;
                        if (k == 0) idx_front = reg_idx;
                        else {
                            idx_front = data[(i * shape.y + j) * shape.z + k - 1];
                        }
                        if (k == shape.z) idx_back = reg_idx;
                        else {
                            idx_back = current_reg;
                        }
                        zm1.region_idx_backface  = idx_back;
                        zm2.region_idx_backface  = idx_back;
                        zm1.region_idx_frontface = idx_front;
                        zm2.region_idx_frontface = idx_front;

                        zm1.vertex[0] = make_float3(pos.x,          pos.y,          pos.z);
                        zm1.vertex[1] = make_float3(pos.x + size.x, pos.y + size.y, pos.z);
                        zm1.vertex[2] = make_float3(pos.x + size.x, pos.y,          pos.z);
                        zm2.vertex[0] = make_float3(pos.x,          pos.y,          pos.z);
                        zm2.vertex[1] = make_float3(pos.x,          pos.y + size.y, pos.z);
                        zm2.vertex[2] = make_float3(pos.x + size.x, pos.y + size.y, pos.z);
                        if (idx_back != idx_front) {
                            out.push_back(zm1);
                            out.push_back(zm2);
                        }
                    }
                    // y plane
                    {
                        MarkedTriangle ym1, ym2;
                        int idx_front, idx_back;
                        if (j == 0) idx_front = reg_idx;
                        else {
                            idx_front = data[(i * shape.y + j - 1) * shape.z + k];
                        }
                        if (j == shape.y) idx_back = reg_idx;
                        else {
                            idx_back = current_reg;
                        }
                        ym1.region_idx_backface = idx_back;
                        ym2.region_idx_backface = idx_back;
                        ym1.region_idx_frontface = idx_front;
                        ym2.region_idx_frontface = idx_front;

                        ym1.vertex[0] = make_float3(pos.x,          pos.y, pos.z         );
                        ym1.vertex[1] = make_float3(pos.x + size.x, pos.y, pos.z + size.z);
                        ym1.vertex[2] = make_float3(pos.x,          pos.y, pos.z + size.z);
                        ym2.vertex[0] = make_float3(pos.x,          pos.y, pos.z         );
                        ym2.vertex[1] = make_float3(pos.x + size.x, pos.y, pos.z         );
                        ym2.vertex[2] = make_float3(pos.x + size.x, pos.y, pos.z + size.z);
                        if (idx_back != idx_front) {
                            out.push_back(ym1);
                            out.push_back(ym2);
                        }
                    }
                    // x plane
                    {
                        MarkedTriangle xm1, xm2;
                        int idx_front, idx_back;
                        if (i == 0) idx_front = reg_idx;
                        else {
                            idx_front = data[((i - 1) * shape.y + j) * shape.z + k];
                        }
                        if (i == shape.x) idx_back = reg_idx;
                        else {
                            idx_back = current_reg;
                        }
                        xm1.region_idx_backface = idx_back;
                        xm2.region_idx_backface = idx_back;
                        xm1.region_idx_frontface = idx_front;
                        xm2.region_idx_frontface = idx_front;

                        xm1.vertex[0] = make_float3(pos.x, pos.y,          pos.z         );
                        xm1.vertex[1] = make_float3(pos.x, pos.y + size.y, pos.z + size.z);
                        xm1.vertex[2] = make_float3(pos.x, pos.y + size.y, pos.z         );
                        xm2.vertex[0] = make_float3(pos.x, pos.y,          pos.z         );
                        xm2.vertex[1] = make_float3(pos.x, pos.y,          pos.z + size.z);
                        xm2.vertex[2] = make_float3(pos.x, pos.y + size.y, pos.z + size.z);
                        if (idx_back != idx_front) {
                            out.push_back(xm1);
                            out.push_back(xm2);
                        }
                    }
                }
            }
        }
        // translate planes
        double3 rot[4];
        for (size_t i = 0; i < 4; ++i)
            rot[i] = make_double3(
                transformer[3 * i],
                transformer[3 * i + 1],
                transformer[3 * i + 2]
            );
        for (size_t i = 0; i < out.size(); ++i) {
            for (size_t j = 0; j < 3; ++j) {
                float3 nv;
                float3 ov = out[i].vertex[j];
                nv.x = rot[0].x * ov.x + rot[1].x * ov.y + rot[2].x * ov.z + rot[3].x;
                nv.y = rot[0].y * ov.x + rot[1].y * ov.y + rot[2].y * ov.z + rot[3].y;
                nv.z = rot[0].z * ov.x + rot[1].z * ov.y + rot[2].z * ov.z + rot[3].z;
                out[i].vertex[j] = nv;
            }
        }
        return out;
    }
    */

}