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
 * @file    RT2builder/surface.hpp
 * @brief   RT2builder surface definitions
 * @author  CM Lee
 * @date    05/23/2023
 */


#pragma once

#include <string>
#include <fstream>
#include <deque>
#include <vector>
#include <set>
#include <algorithm>
#include <limits>

#include <optix.h>
#include <optix_stack_size.h>
#include <optix_stubs.h>

#include <cuda_runtime.h>
#include <sampleConfig.h>

#include "mcutil/device/algorithm.hpp"
#include "mcutil/parser/input.hpp"
#include "mcutil/parser/parser.hpp"
#include "mcutil/world/logic.hpp"
#include "mcutil/geometry/mesh_primitives.hpp"

#include "voxel.hpp"
#include "config.hpp"


namespace geo {


    constexpr double MESH_MINIMUM_SIZE = 1e-5;
    constexpr double MESH_MAXIMUM_SIZE = 1e+10;

    constexpr int MESH_ICOSPHERE_DEFAULT_ORDER = 4;
    constexpr int MESH_ICOSPHERE_MINIMUM_ORDER = 0;
    constexpr int MESH_ICOSPHERE_MAXIMUM_ORDER = 8;

    constexpr int MESH_DEFAULT_VERTICES = 100;
    constexpr int MESH_MINIMUM_VERTICES = 3;
    constexpr int MESH_MAXIMUM_VERTICES = 500;


    /*
    Logical surface transform
    */

    enum class SURFACE_TRANSFORM_AXIS {
        X,
        Y,
        Z
    };


    struct MeshTransformFtn {
        void (mesh::PrimitiveMesh::* rot1)(double);
        void (mesh::PrimitiveMesh::* rot2)(double);
    };


    struct VectorTransformFtn {
        mesh::Point_3(*rot1)(mesh::Point_3, double);
        mesh::Point_3(*rot2)(mesh::Point_3, double);
    };


    /**
    * @brief Geometrical affine transform
    */
    class RotDefi : public mcutil::Affine {
    private:
        std::string  _name;          //!< @brief Card name
    public:
        RotDefi() {}
        RotDefi(mcutil::ArgInput& args);
        std::string name() const;
        void summary() const;
    };


    /*
    Optix traversable object, for the corefining
    */

    class OptixInitialScene {
    private:
        OptixTraversableHandle _gas_handle;
        CUdeviceptr            _d_gas_output_buffer;
    public:
        OptixInitialScene(
            const mesh::PolygonMesh& mesh_init,
            OptixDeviceContext       context
        );
        ~OptixInitialScene();
        OptixTraversableHandle gas_handle();
    };


    /*
    Surface series
    */

    class SurfaceBegin {
    private:
    public:
        SurfaceBegin() {}
        SurfaceBegin(mcutil::ArgInput& args) {}
    };


    class SurfaceEnd {
    private:
    public:
        SurfaceEnd() {}
        SurfaceEnd(mcutil::ArgInput& args) {}
    };


    class TransformBegin {
    private:
        std::string _target;
    public:
        TransformBegin() {}
        TransformBegin(mcutil::ArgInput& args);
        const std::string& target() const;
    };


    class TransformEnd {
    public:
        TransformEnd() {}
        TransformEnd(mcutil::ArgInput& args) {}
    };


    /*
    Logical surface primitives
    */


    class Surface : public mcutil::Affine {
    protected:
        std::string                          _name;            // surface name, marked on MC input
        std::shared_ptr<mesh::PrimitiveMesh> _mesh_init;       // mesh, initial
        std::shared_ptr<OptixInitialScene>   _scene_init;      // optix initial scene handler
    public:
        Surface();
        const std::string& name() const;
        const mesh::PrimitiveMesh& mesh_init() const;

        size_t n_vertices() const;
        void writePrimitive(const std::string& file_name) const;
        virtual void transform(const mcutil::Affine& affine);

        bool meshNotExist() const;
        bool checkSelfIntersect() const;
        bool checkIsClosed() const;

        bool compressInitialScene(OptixDeviceContext context);
        OptixTraversableHandle getInitialSceneHandle() const;
    };

    
    class VoxelContainer : public Surface { 
    private:
        std::shared_ptr<Voxel> _voxel;
        // std::string         _file_name;
    public:
        VoxelContainer() {}
        VoxelContainer(mcutil::ArgInput& args);
        void transform(const mcutil::Affine& affine);
        // void build(const std::deque<Hounsfield>& houns_bins);
        std::shared_ptr<Voxel>& voxel();

        void summary() const;
    };


    class Revolution : public Surface {
    private:
        double3 _center;
        double3 _direction;
        int     _vertices;
    public:
        Revolution() {}
        Revolution(mcutil::ArgInput& args);
    };


    class UvSphere : public Surface {
    public:
        UvSphere() {}
        UvSphere(mcutil::ArgInput& args);
    };


    class Ellipsoid : public Surface {
    public:
        Ellipsoid() {}
        Ellipsoid(mcutil::ArgInput& args);
    };


    class Cylinder : public Surface {
    public:
        Cylinder() {}
        Cylinder(mcutil::ArgInput& args);
    };


    class Cone : public Surface {
    public:
        Cone() {}
        Cone(mcutil::ArgInput& args);
    };


    class IcoSphere : public Surface {
    public:
        IcoSphere() {}
        IcoSphere(mcutil::ArgInput& args);
    };


    class Cube : public Surface {
    public:
        Cube() {}
        Cube(mcutil::ArgInput& args);
    };


    class Toroid : public Surface {
    private:
        double3 _center;
        double3 _direction;
        int     _vertices;
    public:
        Toroid() {}
        Toroid(mcutil::ArgInput& args);
    };


    class Torus : public Surface {
    public:
        Torus() {}
        Torus(mcutil::ArgInput& args);
    };


    class Plate : public Surface {
    private:
        double3 _center;
        double3 _direction;
    public:
        Plate() {}
        Plate(mcutil::ArgInput& args);
    };


    class Model : public Surface {
    public:
        Model() {}
        Model(mcutil::ArgInput& args);
    };


}
