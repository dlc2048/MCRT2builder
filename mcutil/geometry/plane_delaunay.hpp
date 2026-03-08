/**
 * @file    mcutil/geometry/plane_delaunay.hpp
 * @brief   2D Delaunay triangulation for generating non-convex plate
 * @author  CM Lee
 * @date    10/28/2025
 */

#pragma once

#include <vector>
#include <map>
#include <set>
#include <cuda_runtime.h>

#include "mcutil/device/algorithm.hpp"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>


using K  = CGAL::Exact_predicates_inexact_constructions_kernel;
using P2 = K::Point_2;

struct FaceInfo { int nesting_level = 0; bool in_domain() const { return nesting_level % 2 == 1; } };

using Vb  = CGAL::Triangulation_vertex_base_2<K>;
using Fb  = CGAL::Constrained_triangulation_face_base_2<K>;
using Fbi = CGAL::Triangulation_face_base_with_info_2<FaceInfo, K, Fb>;

using Tds  = CGAL::Triangulation_data_structure_2<Vb, Fbi>;
using Itag = CGAL::Exact_predicates_tag;
using CDT  = CGAL::Constrained_Delaunay_triangulation_2<K, Tds, Itag>;

using VH = CDT::Vertex_handle;
using FH = CDT::Face_handle;

namespace mesh {
    namespace Delaunay {


        void markDomain(CDT& cdt);


        void insertPolygonEdgeConstraint(CDT& cdt, const std::vector<P2>& outer);


        std::vector<int> extractOuterBoundaryCCW(CDT& cdt, const std::unordered_map<VH, int>& vh2idx);


    }
}