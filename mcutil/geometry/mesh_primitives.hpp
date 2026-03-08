/**
 * @file    mcutil/geometry/mesh_primitives.hpp
 * @brief   Polygon mesh primitives include sphere, cube, cylinder and so on
 * @author  CM Lee
 * @date    05/23/2023
 */

#pragma once

#include <vector>
#include <map>
#include <set>
#include <cuda_runtime.h>

#include "mcutil/device/algorithm.hpp"

#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/transform.h>
#include <CGAL/Polygon_mesh_processing/corefinement.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/measure.h>

#include <CGAL/Polygon_2.h>

#define MESH_PLANE_GRADIENT_X_MINIMUM    1e-20
#define MESH_PLANE_GRADIENT_EPSILON      1e-3
#define MESH_PLANE_VERTEX_EPSILON        1e-6
#define MESH_PLANE_VERTEX_EPSILON_D      1e-12

#define MESH_REVOLUTION_POSITION_EPSILON 1e-20


inline bool operator<(const ulonglong3& lhs, const ulonglong3& rhs) {
	if (lhs.x < rhs.x) return true;
	else if (lhs.x > rhs.x) return false;
	else {
		if (lhs.y < rhs.y) return true;
		else if (lhs.y > rhs.y) return false;
		else{
			if (lhs.z < rhs.z) return true;
			else if (lhs.z > rhs.z) return false;
		}
	}
	return false;
}


inline bool operator>(const ulonglong3& lhs, const ulonglong3& rhs) {
	if (lhs.x > rhs.x) return true;
	else if (lhs.x < rhs.x) return true;
	else {
		if (lhs.y > rhs.y) return true;
		else if (lhs.y < rhs.y) return true;
		else {
			if (lhs.z > rhs.z) return true;
			else if (lhs.z < rhs.z) return true;
		}
	}
	return false;
}


inline bool operator==(const ulonglong3& lhs, const ulonglong3& rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}


namespace mesh {


	enum class APPROXIMATION_TYPE {
		QUAD_ENCIRCLE,
		MESH_ENCIRCLE,
		AREA_CONSERVE,
		VOLUME_CONSERVE
	};


	enum class POLYGON_OVERLAP_TYPE {
		IDENTICAL,
		INSIDE,
		OUTSIDE,
		INTERSECT
	};


	struct PLANAR_OVERLAP_TYPE {
		bool on_same_plane;
		bool face_to_face;
		POLYGON_OVERLAP_TYPE polygon_overlap;
		PLANAR_OVERLAP_TYPE() {
			on_same_plane   = false;
			face_to_face    = false;
			polygon_overlap = 
				POLYGON_OVERLAP_TYPE::INTERSECT;
		}
	};


	// default triangulation for Surface_mesher
	typedef CGAL::Surface_mesh_default_triangulation_3 Tr;
	// geometry traits
	typedef Tr::Geom_traits GT;
	typedef GT::Point_2 Point_2;
	typedef GT::Point_3 Point_3;
	typedef GT::Vector_3 Vector_3;
	typedef GT::FT FT;
	typedef std::array<FT, 3> custom_point;
	typedef std::vector<size_t> CGAL_Polygon;
	// surface and mesh
	typedef CGAL::Surface_mesh<Point_3> PolygonMesh;
	// plane 2-D polygon
	typedef CGAL::Polygon_2<GT> Polygon_2;

	/* polygon soup repair */
	struct Array_traits
	{
		struct Equal_3
		{
			bool operator()(const custom_point& p, const custom_point& q) const {
				return (p == q);
			}
		};
		struct Less_xyz_3
		{
			bool operator()(const custom_point& p, const custom_point& q) const {
				return std::lexicographical_compare(p.begin(), p.end(), q.begin(), q.end());
			}
		};
		Equal_3 equal_3_object() const { return Equal_3(); }
		Less_xyz_3 less_xyz_3_object() const { return Less_xyz_3(); }
	};

	/**
	* @brief Polygon plane for intersecting/overlapping test
	*/
	class Plane {
		typedef PolygonMesh::vertex_index   vertex_index;
		typedef PolygonMesh::halfedge_index halfedge_index;
	private:
		PolygonMesh*                _mesh;           //!< @brief Pointer of parent mesh
		vertex_index                _center;         //!< @brief Center of polygon
		std::vector<halfedge_index> _corner;         //!< @brief Corner halfedge
		double4                     _plane;          //!< @brief Mathematical plane
		bool                        _allow_overlap;  //!< @brief Overlap constraint
		double                      _side_angle;     //!< @brief Angle between plane and side
	public:


		Plane();


		/**
		* @brief Polygon plane for intersecting/overlapping test
		* @param mesh          Pointer of parent mesh
		* @param plane         Mathematical plane (ax+by+cz+d=0)
		* @param allwo_overlap Overlap is prohibited if true
		* @param angle         Angle between this plane and side
		*/
		Plane(
			PolygonMesh*       mesh, 
			double4            plane, 
			bool               allow_overlap, 
			double             angle
		);
		

		/**
		* @brief Center of polygon plane
		* @return Vertex index of plane center
		*/
		const vertex_index& center() const;


		/**
		* @brief Get all polygon halfedge
		* @return Vector of halfedge of polygon side
		*/
		const std::vector<halfedge_index>& corner() const;


		double4 plane() const;


		bool isOverlapAllowed() const;


		double sideAngle() const;


		/**
		* @brief Set the center of polygon plane
		* @param idx Vertex index of polygon center. idx must be part of parent mesh
		*/
		void setCenter(vertex_index idx);


		/**
		* @brief Set the side of polygon plane
		* @param idx Halfedge index of polygon side. idx must be part of parent mesh
		*/
		void pushCorner(halfedge_index idx);


		/**
		* @brief Transform whole polygon and plane equation
		* @param affine Cartesian affine transform matrix
		*/
		void transform(const mcutil::Affine& affine);


		/**
		* @brief Extract the CGAL polygon object
		* 
		* @return CGAL polygon object
		*/
		Polygon_2 polygon() const;
		

		/**
		* @brief Test the overlap condition
		* @param reference opponent plane object
		* 
		* @return PLANAR_OVERLAP_TYPE
		*/
		PLANAR_OVERLAP_TYPE overlap(const Plane& reference) const;

		/**
		* @brief Extend plane to corefine operation
		* @param epsilon Extend epsilon (cm)
		*/
		bool extend(double epsilon);
	};


	/**
	* @brief Geometry primitive with planar profile
	*/
	class PrimitiveMesh {
		friend class PrimitiveMeshFactory;
	private:
		PolygonMesh        _mesh;   //!< @brief Polygon mesh
		std::vector<Plane> _plane;  //!< @brief Included planes
	public:


		PrimitiveMesh();


		PrimitiveMesh(const PrimitiveMesh& pm);


		~PrimitiveMesh();


		/**
		* @brief Check self intersecting condition
		*/
		bool does_self_intersect() const;


		/**
		* @brief Flip face orientation
		*/
		void flip();


		/**
		* @brief Check closed condition
		*/
		bool is_closed() const;


		/**
		* @brief Get CGAL mesh reference
		*/
		const PolygonMesh& mesh() const;


		/**
		* @brief Get CGAL mesh pointer
		*/
		PolygonMesh* meshPointer();


		/**
		* @brief Get the total number of included planes
		*/
		size_t number_of_plane() const;


		/**
		* @brief Get the total number of included faces
		*/
		size_t number_of_faces() const;


		/**
		* @brief Get the total number of vertices
		*/
		size_t number_of_vertices() const;


		/**
		* @brief Get the child plane object
		* @param idx Index of plane
		*/
		const Plane& plane(size_t idx) const;


		/*
		* @brief Push single plane
		* @param plane Target plane
		*/
		void pushPlane(Plane plane);


		/**
		* @brief Transform mesh and all planes
		* @param affine Cartesian affine transform matrix
		*/
		void transform(const mcutil::Affine& affine);


		/**
		* @brief Extend target plane to corefine operation
		* @param idx     Index of target plane
		* @param epsilon Extend epsilon (cm)
		*/
		bool extend(size_t idx, double epsilon);


		/**
		* @brief Write mesh to 3-D object file
		* @param file_name Output file name
		*/
		void write(const char* file_name) const;
	};


	/**
	* @brief Geometrical primitive factory
	*/
	class PrimitiveMeshFactory {
		typedef std::vector<double2> Entry2;
		typedef PolygonMesh::vertex_index vertex_index;
	private:
	public:


		/**
		* @brief Generate Icosphere object. Center of sphere is (0,0,0)
		* @param radius Radius of sphere
		* @param order  Order of icosphere
		* 
		* @return Shared pointer of primitive mesh
		*/
		static std::shared_ptr<PrimitiveMesh> sphereIco(double radius, size_t order);


		/**
		* @brief Generate axis-aligned cube object. Center of cube is (0,0,0)
		* @param dx X-axis size
		* @param dy Y-axis size
		* @param dz Z-axis size
		*
		* @return Shared pointer of primitive mesh
		*/
		static std::shared_ptr<PrimitiveMesh> cube(double dx, double dy, double dz);


		/**
		* @brief Generate axis-aliend cube object with rectangular
		*        tessellation for voxel handling. Center of first voxel is (0,0,0)
		*        and voxel size is (1,1,1)
		* @param nx Number of voxel in x-axis
		* @param ny Number of voxel in y-axis
		* @param nz Number of voxel in z-axis
		* 
		* @return Shared pointer of primitive mesh
		*/
		static std::shared_ptr<PrimitiveMesh> voxel(size_t nx, size_t ny, size_t nz);


		/**
		* @brief Generate z-axis revolution mesh
		* @param entry       Set of revolution coordinates (x,y). 
		*                    Entry must be clockwise around the z-axis
		* @param n_azimuthal Number of vertices in azimuthal direction
		* 
		* @return Shared pointer of primitive mesh
		*/
		static std::shared_ptr<PrimitiveMesh> revolution(const Entry2& entry, size_t n_azimuthal);


		/**
		* @brief Generate z-axis aliend uvsphere. Center of sphere is (0,0,0)
		* @param radius      Radius of sphere
		* @param n_vertical  Number of vertices in vertical direction
		* @param n_azimuthal Number of vertices in azimuthal direction
		* 
		* @return Shared pointer of primitive mesh
		*/
		static std::shared_ptr<PrimitiveMesh> sphereUV(double radius, size_t n_vertical, size_t n_azimuthal);


		/**
		* @brief Generate z-axis aliend cylinder. Center of base circle is (0,0,0)
		* @param radius      Radius of cylinder
		* @param height      Height of cylinder
		* @param n_azimuthal Number of vertices in azimuthal direction
		* 
		* @return Shared pointer of primitive mesh
		*/
		static std::shared_ptr<PrimitiveMesh> cylinder(double radius, double height, size_t n_azimuthal);


		/**
		* @brief Generate z-axis aliend cone. Center of base circle is (0,0,0)
		* @param radius      Radius of cone
		* @param height      Height of cone
		* @param n_azimuthal Number of vertices in azimuthal direction
		*
		* @return Shared pointer of primitive mesh
		*/
		static std::shared_ptr<PrimitiveMesh> cone(double radius, double height, size_t n_azimuthal);

		static std::shared_ptr<PrimitiveMesh> toroid(const Entry2& entry, size_t n_azimuthal);
		static std::shared_ptr<PrimitiveMesh> torus(double a, double c, size_t n_ring, size_t n_azimuthal);


		/**
		* @brief Generate z-axis aliend plate. Coordinates (entry) center is (0,0,0)
		* @param entry     Coordinate set of the base projection image of plate (x,y).
		*                  Entry must be clockwise around the z-axis
		* @param thickness Thickness of the plate [cm]
		*
		* @return Shared pointer of primitive mesh
		*/
		static std::shared_ptr<PrimitiveMesh> plate(const Entry2& entry, double thickness);


		static std::shared_ptr<PrimitiveMesh> file(const char* file_name);

	};


	/**
	* @brief Transform polygon mesh
	* @param sm     Pointer of polygon mesh
	* @param affine Cartesian affine transform matrix
	*/
	void transform(PolygonMesh* sm, const mcutil::Affine& affine);


	Point_3 transform(Point_3 point, const mcutil::Affine& affine);


	double4 transform(double4 plane, const mcutil::Affine& affine);


	/**
	* @brief Normalize mathematical plane equation (ax+by+cz+d=0)
	*        with constraint (a**2+b**2+c**2=1)
	* @param plane Original plane equation
	* 
	* @return Normalized plane
	*/
	double4 normalize(double4 plane);

	/* geometry correction */

	double angleBetweenEntryPoints(double2 p0, double2 p1, double2 p2);

	double calculatePolygonCircleRadius(
		double radius,
		double height,
		uint32_t n_vertices,
		APPROXIMATION_TYPE method = APPROXIMATION_TYPE::QUAD_ENCIRCLE
	);

	Point_3 centerOfMass(Point_3 p0, Point_3 p1, Point_3 p2);
	void correctPolygonSphereRadius(
		PolygonMesh*       mesh,
		double             radius,
		APPROXIMATION_TYPE method = APPROXIMATION_TYPE::QUAD_ENCIRCLE
	);

	std::vector<float3> extractVertices(const PolygonMesh& sm, double epsilon=0.e0);

	void repairCorefinedMesh(PolygonMesh* sm, double epsilon=MESH_PLANE_VERTEX_EPSILON);

}