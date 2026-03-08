#pragma once

#include <iostream>
#include <exception>
#include <vector>
#include <string>
#include <cuda_runtime.h>

#include "mcutil/device/algorithm.hpp"
#include "mcutil/fortran/fortran.hpp"
#include "mcutil/mclog/logger.hpp"
#include "mcutil/parser/parser.hpp"
#include "mcutil/parser/input.hpp"
#include "mcutil/world/world.hpp"

namespace geo {


    class Voxel : public mcutil::Affine {
    private:
        int3                     _shape;   // voxel shape
        std::vector<uint16_t>    _array;   // 1-D voxel array
        std::vector<std::string> _region;  // region namelist
    public:
        // constructors
        Voxel();
        /*
        Voxel(
            const std::string& file_name, 
            const std::deque<Hounsfield>& hounsfield
        );
        */
        Voxel(const std::string& file_name);
        

        // interior structure generator
        std::vector<geo::MarkedTriangle> getInterior() const;

        // attribute mirrors
        int3 shape() const;
        const std::vector<uint16_t>& array() const;
        const std::vector<std::string>& region() const;
    };


    float3 make_float3(double3 point);


}