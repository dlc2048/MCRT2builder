#pragma once

#include <string>
#include <fstream>
#include <deque>
#include <vector>
#include <set>

#include <optix.h>
#include <optix_stack_size.h>
#include <optix_stubs.h>

#include <cuda_runtime.h>
#include <sampleConfig.h>

#include "../shared/input/input.h"
#include "../shared/input/parser.h"
#include "mesh_primitives.h"
#include "logic.h"

#define MESH_MINIMUM_SIZE 1e-5
#define MESH_MAXIMUM_SIZE 1e+10

#define MESH_ICOSPHERE_DEFAULT_ORDER 4
#define MESH_ICOSPHERE_MINIMUM_ORDER 0
#define MESH_ICOSPHERE_MAXIMUM_ORDER 8

#define MESH_DEFAULT_VERTICES 100
#define MESH_MINIMUM_VERTICES 3
#define MESH_MAXIMUM_VERTICES 500


namespace geo {

    /*
    Geometry model
    */

    enum class SURFACE_MODEL {
        QUADRATIC_CORRECT,
        POLYGON_PRIMITIVE
    };


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


    class RotDefi : public mcutil::_ArgTypeContext<RotDefi> {
    private:
        SURFACE_TRANSFORM_AXIS _type;
        std::string _name;
        double      _polar;
        double      _azimuthal;
        double3     _translate;
    public:
        RotDefi();
        RotDefi(const std::map<std::string, card::ArgContainer>& data);
        template<typename S> S getRotationMatrix() const;
        std::string name() const;
        SURFACE_TRANSFORM_AXIS type() const;
        double polar() const;
        double azimuthal() const;
        double3 translate() const;
    };


    /*
    Revolution family entry point
    */
    
    class EntryPoints : public mcutil::_ListTypeContext<EntryPoints> {
    private:
        std::string          _name;
        std::vector<double2> _entry;
    public:
        EntryPoints(const std::deque<std::string>& container);
        const std::string& name() const;
        const std::vector<double2>& entry() const;
    };


    /*
    Surface series
    */


    class SurfaceBegin : public mcutil::_ArgTypeContext<SurfaceBegin> {
    private:
    public:
        SurfaceBegin();
        SurfaceBegin(const std::map<std::string, card::ArgContainer>& data);
    };


    class SurfaceEnd : public mcutil::_ArgTypeContext<SurfaceEnd> {
    private:
    public:
        SurfaceEnd();
        SurfaceEnd(const std::map<std::string, card::ArgContainer>& data);
    };


    class TransformBegin : public mcutil::_ArgTypeContext<TransformBegin> {
    private:
        std::string _target;
    public:
        TransformBegin();
        TransformBegin(const std::map<std::string, card::ArgContainer>& data);
        const std::string& target() const;
    };


    class TransformEnd : public mcutil::_ArgTypeContext<TransformEnd> {
    public:
        TransformEnd();
        TransformEnd(const std::map<std::string, card::ArgContainer>& data);
    };


    class _LogicalSurfaceAttributes {
    protected:
        std::string _name;
        std::string _file;
        std::string _entry;
        double3     _center;
        double3     _direction;
        int2        _vertices;
        int         _order;
        double2     _radius;
        double3     _size;
        double      _scale;
        _LogicalSurfaceAttributes();
    public:
        const std::string name() const;
        const std::string file() const;
        const std::string entry() const;
        double3 center() const;
        double3 direction() const;
        int2 vertices() const;
        int order() const;
        double2 radius() const;
        double3 size() const;
        double scale() const;
    };


    class CardVoxel : 
        public mcutil::_ArgTypeContext<CardVoxel>, 
        public _LogicalSurfaceAttributes {
    public:
        CardVoxel();
        CardVoxel(const std::map<std::string, card::ArgContainer>& data);
    };


    class CardRevolution : 
        public mcutil::_ArgTypeContext<CardRevolution>, 
        public _LogicalSurfaceAttributes {
    public:
        CardRevolution();
        CardRevolution(const std::map<std::string, card::ArgContainer>& data);
    };


    class CardUVSphere :
        public mcutil::_ArgTypeContext<CardUVSphere>,
        public _LogicalSurfaceAttributes {
        CardUVSphere();
        CardUVSphere(const std::map<std::string, card::ArgContainer>& data);
    };


    class CardCylinder :
        public mcutil::_ArgTypeContext<CardCylinder>,
        public _LogicalSurfaceAttributes {
        CardCylinder();
        CardCylinder(const std::map<std::string, card::ArgContainer>& data);
    };


    class CardCone :
        public mcutil::_ArgTypeContext<CardCone>,
        public _LogicalSurfaceAttributes {
        CardCone();
        CardCone(const std::map<std::string, card::ArgContainer>& data);
    };


    class CardIcoSphere :
        public mcutil::_ArgTypeContext<CardIcoSphere>,
        public _LogicalSurfaceAttributes {
        CardIcoSphere();
        CardIcoSphere(const std::map<std::string, card::ArgContainer>& data);
    };


    class CardCube :
        public mcutil::_ArgTypeContext<CardCube>,
        public _LogicalSurfaceAttributes {
        CardCube();
        CardCube(const std::map<std::string, card::ArgContainer>& data);
    };


    class CardToroid :
        public mcutil::_ArgTypeContext<CardToroid>,
        public _LogicalSurfaceAttributes {
        CardToroid();
        CardToroid(const std::map<std::string, card::ArgContainer>& data);
    };


    class CardTorus :
        public mcutil::_ArgTypeContext<CardTorus>,
        public _LogicalSurfaceAttributes {
        CardTorus();
        CardTorus(const std::map<std::string, card::ArgContainer>& data);
    };


    class CardModel :
        public mcutil::_ArgTypeContext<CardModel>,
        public _LogicalSurfaceAttributes {
        CardModel();
        CardModel(const std::map<std::string, card::ArgContainer>& data);
    };


    class SurfaceSeries {
    private:
        static card::ArgumentCard _ARGCARD_SURFACE_BEGIN;
        static std::vector<card::ArgumentCard> _ARGCARD_SURFACE_SERIES;
    public:
        static void setCard();
        static void setPosition(mcutil::Input& input);
        static const std::vector<card::ArgumentCard>& argSeries();
    };


    /*
    Base triangle segment
    */

    struct MarkedTriangle {
        float3   vertex[3];
        int32_t  surface_idx;
        int32_t  region_idx_backface;
        int32_t  region_idx_frontface;
        float3   center();
        float3   normal();
        float    area();
    };


    /*
    Surface types
    */

    // If surface type is odd number, there's no quadratic definition
    // of triangle mesh primitive. Elsewise, primitive has quadratic.
    enum class SURFACE_TYPE {
        // have its own OptiX quadratic traversable scene
        SPHERE    = 401,
        CYLINDER  = 701,
        CONE      = 703,
        // no OptiX quadratic traversable scene
        ARBITRARY = 004,
        CUBE      = 602,
        TORUS     = 302,
        VOXEL     = 000
    };


    /*
    Optix surface, for the corefining
    */

    class OptixInitialScene {
        friend class Surface;
    private:
        OptixTraversableHandle _gas_handle;
        CUdeviceptr            _d_gas_output_buffer;
        bool                   _has_device_mem;
    public:
        OptixInitialScene();
        ~OptixInitialScene();
        void load(
            const mesh::PolygonMesh& mesh_init,
            OptixDeviceContext       context
        );
        void free();
    };


    /*
    OptiX surface, for quadratic track-length correction handling
    */

    class OptixQuadratic {
    private:
        SURFACE_TYPE    _type;
        double3         _center;
        double          _radius;
        double3         _height;
    public:
        OptixQuadratic();
        OptixQuadratic(
            SURFACE_TYPE type, 
            double3      center, 
            double       radius, 
            double3      height
        );

        double3 center() const;
        double  radius() const;
        double3 height() const;

        void transform(RotDefi& rot);
    };

    /*
    Logical surface primitives
    */

    class Surface {
        friend class SurfaceFactory;
    private:
        SURFACE_TYPE                _type;            // surface type
        std::string                 _name;            // surface name, marked on MC input
        size_t                      _id;              // surface id, for the internal process
        mesh::PrimitiveMesh         _mesh_init;       // mesh, initial
        std::vector<MarkedTriangle> _mesh_core;       // mesh, after corefine and internal boundary exclusion

        std::set<size_t>            _rel_surf;        // surface id, overlap candidates
        std::set<size_t>            _rel_cell;        // cell id, boundary candidates

        OptixQuadratic              _quadratic;       // quadratic handler
        OptixInitialScene           _scene_init;      // optix initial scene handler

        mesh::Point_3               _transformer[4];  // particle transformer (voxel)
 

        Surface(SURFACE_TYPE type, std::string name, size_t id);
    public:
        ~Surface();

        // attributes mirror

        SURFACE_TYPE type() const;
        bool isCurve() const;
        std::string name() const;
        mesh::PrimitiveMesh& mesh_init();
        const std::vector<MarkedTriangle>& mesh_core() const;
        const std::set<size_t>& neiSurface() const;
        const std::set<size_t>& neiRegion() const;
        std::vector<double> transformer() const;

        void free();
        size_t nInitVertices() const;
        void writePrimitive(const std::string& file_name) const;
        void transform(RotDefi& rot);

        void pushNeiSurface(size_t id);
        void pushNeiRegion(size_t id);
        
        bool checkSelfIntersect() const;
        bool checkIsClosed() const;

        bool compressInitialScene(OptixDeviceContext context);
        OptixTraversableHandle getInitialSceneHandle() const;
        void pushCorefinedFacet(const std::vector<MarkedTriangle>& facets);
        size_t numberOfCoreFacets() const;
        void markCurveFacets();
        void writeCorefined(const std::string& file_name) const;

        // pack up the quadratic
        const std::vector<unsigned char> compressQuadratic();

    };

    /* surface factory */

    class SurfaceFactory {
    public:
        static Surface sphereUV (std::string name, SURFACE_MODEL model,   size_t id, const double* args);
        static Surface sphereICO(std::string name, SURFACE_MODEL model,   size_t id, const double* args);
        static Surface cylinder (std::string name, SURFACE_MODEL model,   size_t id, const double* args);
        static Surface cone     (std::string name, SURFACE_MODEL model,   size_t id, const double* args);
        static Surface cube     (std::string name,                        size_t id, const double* args);
        static Surface torus    (std::string name,                        size_t id, const double* args);
        static Surface model    (std::string name, std::string file_name, size_t id, const double* args);
        static Surface voxel    (std::string name,                        size_t id, double3 origin, double3 size);
        static Surface error    ();
    };

    /*
    OptiX shader binding table
    to determine the surface id of closest hit instance
    */

    struct HitGroupData {
        uint32_t surface_id;
    };

    /*
    OptiX surface instances
    */

    struct OptixSphere {
        const static uint8_t DATA_POINT_PER_INSTANCE = 1;
        const static bool    DATA_POINT_HAS_RADIUS   = true;
        size_t               internal_id;
        float3               vertex[1];
        float                radius[1];
    };

    struct OptixPolygon {
        const static uint8_t DATA_POINT_PER_INSTANCE = 3;
        const static bool    DATA_POINT_HAS_RADIUS   = false;
        size_t               internal_id;
        float3               vertex[3];
        float                radius[1];
    };

    struct OptixCylinder {
        const static uint8_t DATA_POINT_PER_INSTANCE = 2;
        const static bool    DATA_POINT_HAS_RADIUS   = true;
        size_t               internal_id;
        float3               vertex[2];
        float                radius[2];
    };

    /*
    Optix surface build input
    */

    /*
    template<typename T>
    class SurfaceBuildInput {
    private:
        OptixBuildInput _input = {};
        void _getVertexAndRadius(size_t n, T* surfaces, float3** vertex, float** radius);
        size_t* _getSbtTable(size_t n, T* surfaces);
        void _setTypeAndBuffer(size_t n_vertices, size_t n_sbt, CUdeviceptr vertex_buffer, CUdeviceptr radius_buffer);
    public:
        SurfaceBuildInput(size_t n_surfaces, size_t n_sbt, T* surfaces);
        ~SurfaceBuildInput();
        OptixBuildInput getBuildInputPtr();
    };
    */

}