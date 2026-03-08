#pragma once

#include <array>
#include <vector>

#include <cuda_runtime.h>

#include <optix.h>
#include <optix_stack_size.h>
#include <optix_stubs.h>

#include "mcutil/world/world.hpp"

namespace geo {

    /*
    enum class SURFACE_TYPE {
        // have its own OptiX quadratic traversable scene
        SPHERE    = 401,
        CYLINDER  = 701,
        CONE      = 703,
        // no OptiX quadratic traversable scene
        ARBITRARY = 000,
        CUBE      = 602,
        TORUS     = 04
    };


    class OptixQuadratic {
        friend class OptixQuadraticFactory;
    private:
        SURFACE_TYPE           _type;
        float3                 _center;
        float                  _radius;
        float3                 _height;
        OptixTraversableHandle _gas_handle;
        CUdeviceptr            _d_gas_output_buffer;
        OptixQuadratic(
            SURFACE_TYPE type,
            float3       center,
            float        radius,
            float3       height
        );
    public:
        OptixQuadratic() {}
        double3 center();
        double  radius();
        double3 height();
        OptixTraversableHandle getGasAddress();
    };

    class OptixQuadraticFactory {
    private:
        static OptixQuadratic _sphere(OptixDeviceContext* context, float3 center, float radius, float3 height);
        static OptixQuadratic _cylinder(OptixDeviceContext* context, float3 center, float radius, float3 height, bool is_cylinder);
    public:
        static OptixQuadratic generate(
            SURFACE_TYPE        type, 
            OptixDeviceContext* context, 
            float3              center = make_float3(0,0,0), 
            float               radius = 0,
            float3              height = make_float3(0,0,0)
        );
    }; 

    */

    /*
    std::vector<MarkedTriangle> convertVoxelToWorld(
        int                        reg_idx, 
        const std::vector<double>& transformer, 
        double3                    size, 
        int3                       shape, 
        const std::vector<int>&    data
    );
    */

    // std::vector<MarkedTriangle> convertVoxelToWorld(const World& world);

}

