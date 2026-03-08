
#include "mesh_primitives.hpp"
#include "plane_delaunay.hpp"

#include <fstream>
#include <iostream>


namespace mesh {

    /* planar profile */

    Plane::Plane()
        : _mesh(0x0),
          _center(UINT_MAX),
          _plane(make_double4(0.e0, 0.e0, 0.e0, 0.e0)), 
          _allow_overlap(false),
          _side_angle(0.e0) {

    }


    Plane::Plane(PolygonMesh* mesh, double4 plane, bool allow_overlap, double angle)
        : _mesh(mesh), 
          _center(UINT_MAX),
          _plane(plane), 
          _allow_overlap(allow_overlap), 
          _side_angle(angle) {
    }


    const PolygonMesh::vertex_index& Plane::center() const {
        return this->_center;
    }


    const std::vector<PolygonMesh::halfedge_index>& Plane::corner() const {
        return this->_corner;
    }
    

    double4 Plane::plane() const {
        return this->_plane;
    }


    bool Plane::isOverlapAllowed() const {
        return this->_allow_overlap;
    }


    double Plane::sideAngle() const {
        return this->_side_angle;
    }


    void Plane::setCenter(vertex_index idx) {
        this->_center = idx;
    }


    void Plane::pushCorner(halfedge_index idx) {
        this->_corner.push_back(idx);
    }


    void Plane::transform(const mcutil::Affine& affine) {
        this->_plane = mesh::transform(this->_plane, affine);
    }


    Polygon_2 Plane::polygon() const {
        std::vector<Point_2> point;
        int axis;
        double4 plane = this->plane();
        if (abs(plane.x) > abs(plane.y)) {
            if (abs(plane.x) > abs(plane.z))
                axis = 0;
            else
                axis = 2;
        }
        else {
            if (abs(plane.y) > abs(plane.z))
                axis = 1;
            else
                axis = 2;
        }
        const PolygonMesh* const          mesh   = this->_mesh;
        const vertex_index                center = this->center();
        const std::vector<halfedge_index> corner = this->corner();
        for (size_t i = 0; i < corner.size(); ++i) {
            Point_3 point_3;
            if (mesh->target(corner[i]) == center)  // halfedge target is center
                point_3 = mesh->point(mesh->source(corner[i]));
            else
                point_3 = mesh->point(mesh->target(corner[i]));
            switch (axis) {
            case 0:
                point.push_back(Point_2(point_3.y(), point_3.z()));
                break;
            case 1:
                point.push_back(Point_2(point_3.z(), point_3.x()));
                break;
            case 2:
                point.push_back(Point_2(point_3.x(), point_3.y()));
                break;
            }
        }
        // Polygon_2 p2(point.begin(), point.end());
        return Polygon_2(point.begin(), point.end());
    }


    PLANAR_OVERLAP_TYPE Plane::overlap(const Plane& reference) const {
        PLANAR_OVERLAP_TYPE result;
        // equation test
        double4 plane_eq_ref = normalize(reference.plane());
        double4 plane_eq_tar = normalize(this->plane());

        // check back-to-back status
        if (abs(plane_eq_ref.x - plane_eq_tar.x) < MESH_PLANE_GRADIENT_EPSILON &&
            abs(plane_eq_ref.y - plane_eq_tar.y) < MESH_PLANE_GRADIENT_EPSILON &&
            abs(plane_eq_ref.z - plane_eq_tar.z) < MESH_PLANE_GRADIENT_EPSILON &&
            abs(plane_eq_ref.w - plane_eq_tar.w) < MESH_PLANE_VERTEX_EPSILON)
        {
            result.on_same_plane = true;
            result.face_to_face  = false;
        }

        // check face-to-face status
        plane_eq_tar.x = -plane_eq_tar.x;
        plane_eq_tar.y = -plane_eq_tar.y;
        plane_eq_tar.z = -plane_eq_tar.z;
        plane_eq_tar.w = -plane_eq_tar.w;
        if (abs(plane_eq_ref.x - plane_eq_tar.x) < MESH_PLANE_GRADIENT_EPSILON &&
            abs(plane_eq_ref.y - plane_eq_tar.y) < MESH_PLANE_GRADIENT_EPSILON &&
            abs(plane_eq_ref.z - plane_eq_tar.z) < MESH_PLANE_GRADIENT_EPSILON &&
            abs(plane_eq_ref.w - plane_eq_tar.w) < MESH_PLANE_VERTEX_EPSILON)
        {
            result.on_same_plane = true;
            result.face_to_face  = true;
        }

        if (result.on_same_plane) {
            // vertex test
            Polygon_2 polygon_ref = reference.polygon();
            Polygon_2 polygon_tar = this->polygon();

            bool inside_vertex_found  = false;
            bool outside_vertex_found = false;

            GT::Compute_squared_distance_2 ft;
            for (size_t i = 0; i < polygon_tar.size() && !(inside_vertex_found && outside_vertex_found); ++i) {
                Point_2 point_tar = polygon_tar.vertex(i);
                bool same_vertex_found = false;
                for (size_t j = 0; j < polygon_ref.size() && !same_vertex_found; ++j) {
                    Point_2 point_ref = polygon_ref.vertex(j);
                    if (ft(point_tar, point_ref) < MESH_PLANE_VERTEX_EPSILON_D)
                        same_vertex_found = true;
                }
                if (!same_vertex_found) {
                    switch (CGAL::bounded_side_2(polygon_ref.begin(), polygon_ref.end(), point_tar)) {
                    case CGAL::ON_BOUNDED_SIDE:
                    case CGAL::ON_BOUNDARY:
                        inside_vertex_found = true;
                        break;
                    case CGAL::ON_UNBOUNDED_SIDE:
                        outside_vertex_found = true;
                        break;
                    }
                }
            }

            result.polygon_overlap = POLYGON_OVERLAP_TYPE::INTERSECT;
            if (inside_vertex_found) {
                if (outside_vertex_found);
                else
                    result.polygon_overlap = POLYGON_OVERLAP_TYPE::INSIDE;
            }
            else {
                if (outside_vertex_found)
                    result.polygon_overlap = POLYGON_OVERLAP_TYPE::OUTSIDE;
                else {
                    if (polygon_tar.size() == polygon_ref.size())
                        result.polygon_overlap = POLYGON_OVERLAP_TYPE::IDENTICAL;
                }
            }
        }
        
        return result;
    }


    bool Plane::extend(double epsilon) {
        bool success = false;
        if (this->isOverlapAllowed()) {
            double4 plane_eq       = normalize(this->plane());
            double3 epsilon_vector =
                make_double3(
                    plane_eq.x * epsilon,
                    plane_eq.y * epsilon,
                    plane_eq.z * epsilon
                );
            if (this->center().idx() > this->_mesh->number_of_vertices()) {  // invalid center
                for (std::vector<halfedge_index>::iterator he = this->_corner.begin();
                    he != this->_corner.end(); ++he) {
                    Point_3 pos = this->_mesh->point(this->_mesh->target(*he));
                    pos = Point_3(
                        pos.x() + epsilon_vector.x,
                        pos.y() + epsilon_vector.y,
                        pos.z() + epsilon_vector.z
                    );
                    this->_mesh->point(this->_mesh->target(*he)) = pos;
                }
            }
            else {  // center exist
                std::vector<vertex_index> vertex_source;
                std::vector<vertex_index> vertex_target;
                std::vector<vertex_index> vertex_extend;
                // relocate center
                {
                    Point_3 pos = this->_mesh->point(this->center());
                    pos = Point_3(
                        pos.x() + epsilon_vector.x,
                        pos.y() + epsilon_vector.y,
                        pos.z() + epsilon_vector.z
                    );
                    this->_mesh->point(this->center()) = pos;
                }
            }
        }
        return success;
    }


    /* geometry primitive with planar profile */

    PrimitiveMesh::PrimitiveMesh() {}


    PrimitiveMesh::PrimitiveMesh(const PrimitiveMesh& pm) {
        typedef PolygonMesh::vertex_index   vertex_index;
        typedef PolygonMesh::halfedge_index halfedge_index;
        this->_mesh  = PolygonMesh(pm._mesh);
        std::vector<Plane> plane = std::vector<Plane>(pm._plane);
        for (size_t i = 0; i < plane.size(); ++i) {
            double4 plane_eq   = plane[i].plane();
            bool allow_overlap = plane[i].isOverlapAllowed();
            double side_normal = plane[i].sideAngle();
            this->pushPlane(Plane(&this->_mesh, plane_eq, allow_overlap, side_normal));
            const vertex_index center = plane[i].center();
            PolygonMesh::vertex_iterator iter = this->_mesh.vertices_begin();
            this->_plane[i].setCenter(*(iter + center.idx()));
            const std::vector<halfedge_index> corner = plane[i].corner();
            for (size_t j = 0; j < corner.size(); ++j) {
                PolygonMesh::halfedge_iterator iter = this->_mesh.halfedges_begin();
                this->_plane[i].pushCorner(*(iter + corner[j].idx()));
            }
        }
    }


    PrimitiveMesh::~PrimitiveMesh() {
        this->_mesh.clear();
    }


    bool PrimitiveMesh::does_self_intersect() const {
        return CGAL::Polygon_mesh_processing::does_self_intersect(this->_mesh);
    }


    void PrimitiveMesh::flip() {
        return CGAL::Polygon_mesh_processing::reverse_face_orientations(this->_mesh);
    }


    bool PrimitiveMesh::is_closed() const {
        return CGAL::is_closed(this->_mesh);
    }


    const PolygonMesh& PrimitiveMesh::mesh() const {
        return this->_mesh;
    }


    PolygonMesh* PrimitiveMesh::meshPointer() {
        return &this->_mesh;
    }


    size_t PrimitiveMesh::number_of_plane() const {
        return this->_plane.size();
    }


    size_t PrimitiveMesh::number_of_faces() const {
        return this->_mesh.number_of_faces();
    }


    size_t PrimitiveMesh::number_of_vertices() const {
        return this->_mesh.number_of_vertices();
    }


    const Plane& PrimitiveMesh::plane(size_t idx) const {
        return this->_plane[idx];
    }


    void PrimitiveMesh::pushPlane(Plane plane) {
        this->_plane.push_back(plane);
    }


    void PrimitiveMesh::transform(const mcutil::Affine& affine) {
        mesh::transform(&this->_mesh, affine);
        for (size_t i = 0; i < this->_plane.size(); ++i)
            this->_plane[i].transform(affine);
    }


    bool PrimitiveMesh::extend(size_t idx, double epsilon) {
        return this->_plane[idx].extend(epsilon);
    }


    void PrimitiveMesh::write(const char* file_name) const {
        std::ofstream out(file_name);
        out << this->_mesh << std::endl;
    }


    /* geometry primitive generator */

    // tessellation family

    std::shared_ptr<PrimitiveMesh> PrimitiveMeshFactory::sphereIco(double radius, size_t order) {
        typedef std::map<PolygonMesh::vertex_index,   PolygonMesh::vertex_index> vPair;
        typedef std::map<PolygonMesh::halfedge_index, PolygonMesh::vertex_index> ePair;
        std::shared_ptr<PrimitiveMesh> pm = std::make_shared<PrimitiveMesh>();
        PolygonMesh                    sm;
        // set up 20-triangle icosahedron
        double r_hedron = radius; // / sqrt(1 + (1 + sqrt(5)) * (1 + sqrt(5)) * 0.25);
        CGAL::make_icosahedron(sm, Point_3(0.0, 0.0, 0.0), r_hedron);
        for (size_t i = 0; i < order; ++i) {
            // copy vertex data to the new mesh
            // and pairing original mesh vertex and new mesh vertex
            PolygonMesh sm_new;
            vPair v_pairs;
            ePair e_pairs;
            for (PolygonMesh::vertex_iterator vi = sm.vertices_begin();
                vi != sm.vertices_end(); ++vi) {
                PolygonMesh::vertex_index nvi = sm_new.add_vertex(sm.point(*vi));
                v_pairs.insert({ *vi, nvi });
            }
            // add midpoint vertex for all edges, iterate over halfedges
            for (PolygonMesh::halfedge_iterator hei = sm.halfedges_begin();
                hei != sm.halfedges_end(); ++hei) {
                if (e_pairs.find(*hei) == e_pairs.end()) {  // paired vertex is not found
                    ePair::iterator op = e_pairs.find(sm.opposite(*hei));
                    if (op == e_pairs.end()) {  // opposite also
                        double norm;
                        Point_3 target, prev, center;
                        target = sm.point(sm.target(*hei));
                        prev = sm.point(sm.target(sm.prev(*hei)));
                        center = Point_3(
                            (target.x() + prev.x()) * 0.5,
                            (target.y() + prev.y()) * 0.5,
                            (target.z() + prev.z()) * 0.5
                        );
                        // normalize
                        norm =
                            center.x() * center.x() +
                            center.y() * center.y() +
                            center.z() * center.z();
                        norm = radius / sqrt(norm);
                        center = Point_3(
                            center.x() * norm,
                            center.y() * norm,
                            center.z() * norm
                        );
                        // push new vertex to new mesh space
                        PolygonMesh::vertex_index nvi = sm_new.add_vertex(center);
                        e_pairs.insert({ *hei, nvi });
                    }
                    else
                        e_pairs.insert({ *hei, op->second });
                }
            }
            // connect edge vertices and  midpoint vertices, iterate over face
            std::set<PolygonMesh::face_index> f_set;
            for (PolygonMesh::halfedge_iterator hei = sm.halfedges_begin();
                hei != sm.halfedges_end(); ++hei) {
                PolygonMesh::face_index fi = sm.face(*hei);
                if (f_set.find(fi) == f_set.end()) {
                    f_set.insert(fi);
                    PolygonMesh::vertex_index p0, p1, p2, c0, c1, c2;
                    p0 = v_pairs.find(sm.target(sm.prev(*hei)))->second;
                    p1 = v_pairs.find(sm.target(*hei))->second;
                    p2 = v_pairs.find(sm.target(sm.next(*hei)))->second;
                    c0 = e_pairs.find(sm.prev(*hei))->second;
                    c1 = e_pairs.find(*hei)->second;
                    c2 = e_pairs.find(sm.next(*hei))->second;
                    sm_new.add_face(c0, c1, c2);
                    sm_new.add_face(c1, c0, p0);
                    sm_new.add_face(c2, c1, p1);
                    sm_new.add_face(c0, c2, p2);
                }
            }
            // set new mesh to old mesh
            sm.clear();
            sm = sm_new;
        }
        pm->_mesh = sm;
        return pm;
    }


    std::shared_ptr<PrimitiveMesh> PrimitiveMeshFactory::cube(double dx, double dy, double dz) {
        std::shared_ptr<PrimitiveMesh> pm = std::make_shared<PrimitiveMesh>();
        PolygonMesh* const             sm = pm->meshPointer();
        std::vector<Plane> plane;
        vertex_index       v[2][2][2];

        // initialize planar profile
        plane.push_back(Plane(sm, make_double4(+1.e0, +0.e0, +0.e0, -dx * 0.5), true, 90.e0));
        plane.push_back(Plane(sm, make_double4(-1.e0, +0.e0, +0.e0, -dx * 0.5), true, 90.e0));
        plane.push_back(Plane(sm, make_double4(+0.e0, +1.e0, +0.e0, -dy * 0.5), true, 90.e0));
        plane.push_back(Plane(sm, make_double4(+0.e0, -1.e0, +0.e0, -dy * 0.5), true, 90.e0));
        plane.push_back(Plane(sm, make_double4(+0.e0, +0.e0, +1.e0, -dz * 0.5), true, 90.e0));
        plane.push_back(Plane(sm, make_double4(+0.e0, +0.e0, -1.e0, -dz * 0.5), true, 90.e0));

        // generate vertices
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < 2; k++) {
                    Point_3 point((-1 + 2 * i) * dx * 0.5,
                                  (-1 + 2 * j) * dy * 0.5,
                                  (-1 + 2 * k) * dz * 0.5);
                    v[i][j][k] = sm->add_vertex(point);
                }
            }
        }
        // draw surface
        sm->add_face(v[0][0][0], v[0][0][1], v[0][1][0]);
        sm->add_face(v[0][0][1], v[0][1][1], v[0][1][0]);
        sm->add_face(v[0][0][0], v[1][0][0], v[0][0][1]);
        sm->add_face(v[0][0][1], v[1][0][0], v[1][0][1]);
        sm->add_face(v[1][0][0], v[1][1][1], v[1][0][1]);
        sm->add_face(v[1][0][0], v[1][1][0], v[1][1][1]);
        sm->add_face(v[0][1][0], v[0][1][1], v[1][1][1]);
        sm->add_face(v[0][1][0], v[1][1][1], v[1][1][0]);
        sm->add_face(v[0][0][0], v[0][1][0], v[1][0][0]);
        sm->add_face(v[0][1][0], v[1][1][0], v[1][0][0]);
        sm->add_face(v[0][0][1], v[1][1][1], v[0][1][1]);
        sm->add_face(v[0][0][1], v[1][0][1], v[1][1][1]);

        plane[0].pushCorner(sm->halfedge(v[0][0][0], v[1][0][0]));
        plane[0].pushCorner(sm->halfedge(v[0][1][0], v[1][1][0]));
        plane[0].pushCorner(sm->halfedge(v[0][1][1], v[1][1][1]));
        plane[0].pushCorner(sm->halfedge(v[0][0][1], v[1][0][1]));

        plane[1].pushCorner(sm->halfedge(v[1][0][0], v[0][0][0]));
        plane[1].pushCorner(sm->halfedge(v[1][0][1], v[0][0][1]));
        plane[1].pushCorner(sm->halfedge(v[1][1][1], v[0][1][1]));
        plane[1].pushCorner(sm->halfedge(v[1][1][0], v[0][1][0]));

        plane[2].pushCorner(sm->halfedge(v[0][0][0], v[0][1][0]));
        plane[2].pushCorner(sm->halfedge(v[0][0][1], v[0][1][1]));
        plane[2].pushCorner(sm->halfedge(v[1][0][1], v[1][1][1]));
        plane[2].pushCorner(sm->halfedge(v[1][0][0], v[1][1][0]));

        plane[3].pushCorner(sm->halfedge(v[0][1][0], v[0][0][0]));
        plane[3].pushCorner(sm->halfedge(v[1][1][0], v[1][0][0]));
        plane[3].pushCorner(sm->halfedge(v[1][1][1], v[1][0][1]));
        plane[3].pushCorner(sm->halfedge(v[0][1][1], v[0][0][1]));

        plane[4].pushCorner(sm->halfedge(v[0][0][0], v[0][0][1]));
        plane[4].pushCorner(sm->halfedge(v[1][0][0], v[1][0][1]));
        plane[4].pushCorner(sm->halfedge(v[1][1][0], v[1][1][1]));
        plane[4].pushCorner(sm->halfedge(v[0][1][0], v[0][1][1]));

        plane[5].pushCorner(sm->halfedge(v[0][0][1], v[0][0][0]));
        plane[5].pushCorner(sm->halfedge(v[0][1][1], v[0][1][0]));
        plane[5].pushCorner(sm->halfedge(v[1][1][1], v[1][1][0]));
        plane[5].pushCorner(sm->halfedge(v[1][0][1], v[1][0][0]));

        for (size_t i = 0; i < plane.size(); ++i)
            pm->pushPlane(plane[i]);
        return pm;
    }


    std::shared_ptr<PrimitiveMesh> PrimitiveMeshFactory::voxel(
        size_t nx, size_t ny, size_t nz
    ) {
        std::shared_ptr<PrimitiveMesh> pm = std::make_shared<PrimitiveMesh>();
        PolygonMesh* const             sm = pm->meshPointer();
        std::vector<Plane> plane;

        // sparse vertex matrix
        std::map<ulonglong3, vertex_index> v;

        // initialize planar profile
        plane.push_back(Plane(sm, make_double4(+1.e0, +0.e0, +0.e0, 0.5             ), true, 90.e0));
        plane.push_back(Plane(sm, make_double4(-1.e0, +0.e0, +0.e0, (double)nx - 0.5), true, 90.e0));
        plane.push_back(Plane(sm, make_double4(+0.e0, +1.e0, +0.e0, 0.5             ), true, 90.e0));
        plane.push_back(Plane(sm, make_double4(+0.e0, -1.e0, +0.e0, (double)ny - 0.5), true, 90.e0));
        plane.push_back(Plane(sm, make_double4(+0.e0, +0.e0, +1.e0, 0.5             ), true, 90.e0));
        plane.push_back(Plane(sm, make_double4(+0.e0, +0.e0, -1.e0, (double)nz - 0.5), true, 90.e0));

        // generate vertices
        for (size_t i = 0; i <= nx; i++) {
            for (size_t j = 0; j <= ny; j++) {
                for (size_t k = 0; k <= nz; k++) {
                    if (!i || i == nx || 
                        !j || j == ny || 
                        !k || k == nz) {  // skin condition
                        Point_3 point(
                            -0.5 + (double)i,
                            -0.5 + (double)j,
                            -0.5 + (double)k
                        );
                        v.insert({ ulonglong3{i,j,k}, sm->add_vertex(point) });
                    }
                }
            }
        }
        // draw surface +-z
        for (size_t i = 0; i < nx; i++) {
            for (size_t j = 0; j < ny; j++) {
                ulonglong3 v1, v2, v3, v4;
                // negative face
                v1 = { i,     j,     0 };
                v2 = { i,     j + 1, 0 };
                v3 = { i + 1, j + 1, 0 };
                v4 = { i + 1, j,     0 };
                sm->add_face(v[v1], v[v2], v[v3]);
                sm->add_face(v[v3], v[v4], v[v1]);
                // positive face
                v1.z = nz;
                v2.z = nz;
                v3.z = nz;
                v4.z = nz;
                sm->add_face(v[v1], v[v3], v[v2]);
                sm->add_face(v[v3], v[v1], v[v4]);
            }
        }
        // draw surface +-y
        for (size_t i = 0; i < nx; i++) {
            for (size_t k = 0; k < nz; k++) {
                ulonglong3 v1, v2, v3, v4;
                // negative face
                v1 = { i,     0, k     };
                v2 = { i + 1, 0, k     };
                v3 = { i + 1, 0, k + 1 };
                v4 = { i ,    0, k + 1 };
                sm->add_face(v[v1], v[v2], v[v3]);
                sm->add_face(v[v3], v[v4], v[v1]);
                // positive face
                v1.y = ny;
                v2.y = ny;
                v3.y = ny;
                v4.y = ny;
                sm->add_face(v[v1], v[v3], v[v2]);
                sm->add_face(v[v3], v[v1], v[v4]);
            }
        }
        // draw surface +-x
        for (size_t j = 0; j < ny; j++) {
            for (size_t k = 0; k < nz; k++) {
                ulonglong3 v1, v2, v3, v4;
                // negative face
                v1 = { 0, j,     k     };
                v2 = { 0, j,     k + 1 };
                v3 = { 0, j + 1, k + 1 };
                v4 = { 0, j + 1, k     };
                sm->add_face(v[v1], v[v2], v[v3]);
                sm->add_face(v[v3], v[v4], v[v1]);
                // positive face
                v1.x = nx;
                v2.x = nx;
                v3.x = nx;
                v4.x = nx;
                sm->add_face(v[v1], v[v3], v[v2]);
                sm->add_face(v[v3], v[v1], v[v4]);
            }
        }

        for (size_t i = 0; i < plane.size(); ++i)
            pm->pushPlane(plane[i]);
        return pm;
    }


    // revolution family

    std::shared_ptr<PrimitiveMesh> PrimitiveMeshFactory::revolution(const Entry2& entry, size_t n_azimuthal) {
        double                         phi;
        size_t                         n_vertical = entry.size();
        std::shared_ptr<PrimitiveMesh> pm = std::make_shared<PrimitiveMesh>();
        PolygonMesh* const             sm = pm->meshPointer();
        std::vector<Plane>             plane;
        std::vector<size_t>            plane_entry_idx;

        bool has_plane_top = false;
        bool has_plane_bot = false;

        // check entry integrity
        {
            if (n_vertical < 3)
                goto REVOLUTION_EXCEPTION;
            if (entry[0].y - entry[n_vertical - 1].y < MESH_REVOLUTION_POSITION_EPSILON)
                goto REVOLUTION_EXCEPTION;
            if (abs(entry[0].x > MESH_REVOLUTION_POSITION_EPSILON))
                goto REVOLUTION_EXCEPTION;
            if (abs(entry[n_vertical - 1].x > MESH_REVOLUTION_POSITION_EPSILON))
                goto REVOLUTION_EXCEPTION;

            for (size_t i = 0; i < n_vertical - 1; ++i) {
                double gradient_x = abs(entry[i].x - entry[i + 1].x);
                double gradient_z = abs(entry[i].y - entry[i + 1].y);
                if (gradient_x < MESH_PLANE_GRADIENT_X_MINIMUM)
                    continue;
                if (gradient_z / gradient_x > MESH_PLANE_GRADIENT_EPSILON)
                    continue;
                if (i == 0)
                    has_plane_top = true;
                if (i == n_vertical - 2)
                    has_plane_bot = true;
                plane_entry_idx.push_back(i);
                double direction = (entry[i].x > entry[i + 1].x) ? -1.e0 : 1.e0;
                bool allow_overlap = (i == 0 || i == n_vertical - 2) ? true : false;
                size_t ei = (i == n_vertical - 2) ? i - 1 : i;
                double angle = angleBetweenEntryPoints(entry[ei], entry[ei + 1], entry[ei + 2]);
                plane.push_back(Plane(sm, make_double4(0.e0, 0.e0, direction, -direction * entry[i].y), allow_overlap, angle));
            }
        }

        // generate solid of revolution
        {
            std::vector<vertex_index> v_eo;  // revolution edge (origin)
            v_eo.resize(n_vertical);
            for (size_t i = 0; i < n_vertical; ++i)
                v_eo[i] = sm->add_vertex(Point_3(entry[i].x, 0.e0, entry[i].y));

            phi = 360.0 / (double)n_azimuthal;

            std::vector<vertex_index> v_el(v_eo), v_en;  // revolution edge (last, new)
            v_en.resize(n_vertical);
            v_en[0] = v_eo[0];
            v_en[n_vertical - 1] = v_eo[n_vertical - 1];

            for (size_t i = 1; i < n_azimuthal; ++i) {
                for (size_t j = 1; j < n_vertical - 1; ++j) {
                    mcutil::Affine affine;
                    affine.rotate(phi, mcutil::AFFINE_AXIS::Z);

                    Point_3 new_p = transform(sm->point(v_el[j]), affine);
                    v_en[j] = sm->add_vertex(new_p);
                }
                // add face
                sm->add_face(v_el[0], v_el[1], v_en[1]);
                for (size_t j = 1; j < n_vertical - 2; ++j) {
                    sm->add_face(v_el[j], v_el[j + 1], v_en[j]);
                    sm->add_face(v_el[j + 1], v_en[j + 1], v_en[j]);
                }
                sm->add_face(v_el[n_vertical - 1], v_en[n_vertical - 2], v_el[n_vertical - 2]);
                // set plane corner
                for (size_t j = 0; j < plane_entry_idx.size(); ++j) {
                    size_t entry_idx = plane_entry_idx[j];
                    plane[j].pushCorner(sm->halfedge(v_el[entry_idx], v_el[entry_idx + 1]));
                }
                // update current vertices index
                for (size_t j = 0; j < n_vertical; ++j)
                    v_el[j] = v_en[j];
            }
            // connect last vertices and initial vertices
            sm->add_face(v_el[0], v_el[1], v_eo[1]);
            for (size_t j = 1; j < n_vertical - 2; ++j) {
                sm->add_face(v_el[j], v_el[j + 1], v_eo[j]);
                sm->add_face(v_el[j + 1], v_eo[j + 1], v_eo[j]);
            }
            sm->add_face(v_el[n_vertical - 1], v_eo[n_vertical - 2], v_el[n_vertical - 2]);
            for (size_t j = 0; j < plane_entry_idx.size(); ++j) {
                size_t entry_idx = plane_entry_idx[j];
                plane[j].pushCorner(sm->halfedge(v_el[entry_idx], v_el[entry_idx + 1]));
            }
            // intialize primitive mesh
            if (has_plane_top) plane[0].setCenter(v_eo[0]);
            if (has_plane_bot) plane[plane.size() - 1].setCenter(v_eo[n_vertical - 1]);
            for (size_t j = 0; j < plane.size(); ++j) {
                pm->pushPlane(plane[j]);
            }
        }
        
    REVOLUTION_EXCEPTION:
        return pm;
    }


    std::shared_ptr<PrimitiveMesh> PrimitiveMeshFactory::sphereUV(double radius, size_t n_vertical, size_t n_azimuthal) {
        Entry2 entry;
        entry.resize(n_vertical);
        for (size_t i = 0; i < n_vertical; ++i) {
            double theta;
            theta = M_PI / (double)(n_vertical - 1) * (double)i;
            entry[i] = make_double2(radius * sin(theta), radius * cos(theta));
        }
        entry[n_vertical - 1].x = 0.e0;
        return PrimitiveMeshFactory::revolution(entry, n_azimuthal);
    }


    std::shared_ptr<PrimitiveMesh> PrimitiveMeshFactory::cylinder(double radius, double height, size_t n_azimuthal) {
        Entry2 entry;
        entry.resize(4);
        entry[0] = make_double2(0.e0,   height);
        entry[1] = make_double2(radius, height);
        entry[2] = make_double2(radius, 0.e0  );
        entry[3] = make_double2(0.e0,   0.e0  );
        return PrimitiveMeshFactory::revolution(entry, n_azimuthal);
    }


    std::shared_ptr<PrimitiveMesh> PrimitiveMeshFactory::cone(double radius, double height, size_t n_azimuthal) {
        Entry2 entry;
        entry.resize(3);
        entry[0] = make_double2(0.e0,   height);
        entry[1] = make_double2(radius, 0.e0  );
        entry[2] = make_double2(0.e0,   0.e0  );
        return PrimitiveMeshFactory::revolution(entry, n_azimuthal);
    }


    // toroid family

    std::shared_ptr<PrimitiveMesh> PrimitiveMeshFactory::toroid(const Entry2& entry, size_t n_azimuthal) {
        double                         phi;
        size_t                         n_ring = entry.size();
        std::shared_ptr<PrimitiveMesh> pm = std::make_shared<PrimitiveMesh>();
        PolygonMesh* const             sm = pm->meshPointer();
        std::vector<Plane>             plane;
        std::vector<size_t>            plane_idx;

        // check entry integrity
        if (n_ring < 3)
            goto TOROID_EXCEPTION;

        {
            // check surface candidate
            for (size_t i = 0; i < n_ring + 1; ++i) {
                double gradient_x = abs(entry[i % n_ring].x - entry[(i + 1) % n_ring].x);
                double gradient_y = abs(entry[i % n_ring].y - entry[(i + 1) % n_ring].y);
                if (gradient_x < MESH_PLANE_GRADIENT_X_MINIMUM)
                    continue;
                else if (gradient_y / gradient_x > MESH_PLANE_GRADIENT_EPSILON)
                    continue;
                double direction =
                    (abs(entry[(i + 1) % n_ring].x) > abs(entry[i % n_ring].x))
                    ? 1.e0
                    : -1.e0;
                double4 plane_eq = make_double4(
                    0.e0, 0.e0, direction,
                    -entry[i % n_ring].y * direction
                );
                plane_idx.push_back(i % n_ring);
                plane.push_back(Plane(sm, plane_eq, false, 0.e0));
            }

            std::vector<vertex_index> v_eo;       // initial ring vertices

            // initial ring vertices
            v_eo.resize(n_ring + 1);
            for (size_t i = 0; i < n_ring; ++i)
                v_eo[i] = sm->add_vertex(Point_3(entry[i].x, 0.e0, entry[i].y));
            v_eo[n_ring] = v_eo[0];

            std::vector<vertex_index> v_el(v_eo);  // last ring vertices
            std::vector<vertex_index> v_en;        // new ring vertices
            v_en.resize(n_ring + 1);

            phi = 360.0 / (double)n_azimuthal;
            for (size_t i = 1; i < n_azimuthal; ++i) {
                // push new vertices
                for (size_t j = 0; j < n_ring; ++j) {
                    mcutil::Affine affine;
                    affine.rotate(phi, mcutil::AFFINE_AXIS::Z);
                    Point_3 new_p = transform(sm->point(v_el[j]), affine);
                    v_en[j] = sm->add_vertex(new_p);
                }
                v_en[n_ring] = v_en[0];
                // add face
                for (size_t j = 0; j < n_ring; ++j) {
                    sm->add_face(v_en[j + 1], v_en[j], v_el[j]);
                    sm->add_face(v_en[j + 1], v_el[j], v_el[j + 1]);
                }
                // set planar corner
                for (size_t j = 0; j < plane_idx.size(); ++j) {
                    size_t idx = plane_idx[j];
                    plane[j].pushCorner(sm->halfedge(v_en[idx % n_ring], v_en[(idx + 1) % n_ring]));
                }
                // update current vertices index
                for (size_t j = 0; j < n_ring; ++j)
                    v_el[j] = v_en[j];
                v_el[n_ring] = v_el[0];
            }
            // connect last vertices and initial vertices
            for (size_t i = 0; i < n_ring; ++i) {
                sm->add_face(v_eo[i + 1], v_eo[i], v_el[i]);
                sm->add_face(v_eo[i + 1], v_el[i], v_el[i + 1]);
            }

            // intialize primitive mesh
            for (size_t i = 0; i < plane_idx.size(); ++i)
                pm->pushPlane(plane[i]);
        }

    TOROID_EXCEPTION:
        return pm;
    }


    std::shared_ptr<PrimitiveMesh> PrimitiveMeshFactory::torus(double a, double c, size_t n_ring, size_t n_azimuthal) {
        Entry2 entry;
        entry.resize(n_ring);
        for (size_t i = 0; i < n_ring; ++i) {
            double theta = 2.e0 * M_PI / (double)(n_ring) * (double)i;
            entry[i].x = sin(theta) * c + a;
            entry[i].y = cos(theta) * c;
        }
        return PrimitiveMeshFactory::toroid(entry, n_azimuthal);
    }


    std::shared_ptr<PrimitiveMesh> PrimitiveMeshFactory::plate(const Entry2& entry, double thickness) {
        size_t                         n_entry = entry.size();
        std::shared_ptr<PrimitiveMesh> pm = std::make_shared<PrimitiveMesh>();
        PolygonMesh* const             sm = pm->meshPointer();
        std::vector<Plane>             plane;

        // check entry integrity
        {
            if (n_entry < 3)
                return pm;

            plane.push_back(Plane(sm, make_double4(0.e0, 0.e0, -1.e0,       0.e0), true, 90.e0));
            plane.push_back(Plane(sm, make_double4(0.e0, 0.e0, +1.e0, -thickness), true, 90.e0));
        }

        // generate 2d polygon (convex partitioning)
        std::vector<Point_2> points;
        double2              center = { 0.e0, 0.e0 };
        for (size_t i = 0; i < n_entry; ++i) {
            points.push_back(Point_2(entry[i].x, entry[i].y));
            center.x += entry[i].x;
            center.y += entry[i].y;
        }
        center.x /= (double)n_entry;
        center.y /= (double)n_entry;
            
        Polygon_2 polygon_2(points.begin(), points.end());

        if (polygon_2.is_convex()) {  // convex case
            vertex_index v_bot;
            vertex_index v_top;

            v_bot = sm->add_vertex(Point_3(center.x, center.y, 0.e0));
            v_top = sm->add_vertex(Point_3(center.x, center.y, thickness));

            std::vector<std::pair<vertex_index, vertex_index>> v_side;  // side edge (bot, top)
            v_side.resize(n_entry + 1);

            // first element
            v_side.front().first  = sm->add_vertex(Point_3(entry[0].x, entry[0].y, 0.e0));
            v_side.front().second = sm->add_vertex(Point_3(entry[0].x, entry[0].y, thickness));

            // last is replica of the first
            v_side.back().first  = v_side.front().first;
            v_side.back().second = v_side.front().second;

            for (size_t i = 1; i <= n_entry; ++i) {
                if (i < n_entry) {
                    v_side[i].first  = sm->add_vertex(Point_3(entry[i].x, entry[i].y, 0.e0));
                    v_side[i].second = sm->add_vertex(Point_3(entry[i].x, entry[i].y, thickness));
                }
                // add face (bot)
                sm->add_face(v_bot, v_side[i].first, v_side[i - 1].first);

                // add face (top)
                sm->add_face(v_top, v_side[i - 1].second, v_side[i].second);

                // add face (side)
                sm->add_face(v_side[i].second, v_side[i - 1].second, v_side[i - 1].first);
                sm->add_face(v_side[i].second, v_side[i - 1].first,  v_side[i].first);
            }
        }
        else {  // non convex -> 2D triangulation

            CDT cdt;
            Delaunay::insertPolygonEdgeConstraint(cdt, points);  // Delaunay triangulation with constraint

            std::unordered_map<VH, int> vh2idx;
            int running = 0;
            for (auto vh : cdt.finite_vertex_handles())
                vh2idx.emplace(vh, running++);

            auto boundary_idx = Delaunay::extractOuterBoundaryCCW(cdt, vh2idx);

            typedef CGAL::internal::CC_iterator<CGAL::Compact_container<
                CGAL::Triangulation_vertex_base_2<K, CGAL::Triangulation_ds_vertex_base_2<Tds>>>, false> TD_HANDLE;

            std::map<TD_HANDLE, std::pair<vertex_index, vertex_index>> v_face;  // face vertex (Delaunay vertex, (bot, top))
            std::vector<std::pair<vertex_index, vertex_index>> v_side(boundary_idx.size() + 1);  // side edge vertex (bot, top)

            int pos = 0;
            for (auto vertex = cdt.finite_vertices_begin(); vertex != cdt.finite_vertices_end(); ++vertex) {
                auto point = vertex->point();

                std::pair<vertex_index, vertex_index> bt_pair;
                bt_pair.first  = sm->add_vertex(Point_3(point.x(), point.y(), 0.e0));
                bt_pair.second = sm->add_vertex(Point_3(point.x(), point.y(), thickness));

                v_face.insert({ vertex , bt_pair });

                auto   it    = std::find(boundary_idx.begin(), boundary_idx.end(), pos);
                size_t index = std::distance(boundary_idx.begin(), it);
                if (index < boundary_idx.size())
                    v_side[index] = bt_pair;

                pos++;
            }
            // replicate first element
            v_side.back() = v_side.front();

            // fill bot & top face
            for (auto facet = cdt.finite_faces_begin(); facet != cdt.finite_faces_end(); ++facet) {
                if (!facet->info().in_domain()) continue;

                auto v0 = facet->vertex(0);
                auto v1 = facet->vertex(1);
                auto v2 = facet->vertex(2);

                // bot -> reorient
                sm->add_face(
                    v_face.find(v0)->second.first, 
                    v_face.find(v2)->second.first,
                    v_face.find(v1)->second.first
                );

                // top -> inherit
                sm->add_face(
                    v_face.find(v0)->second.second,
                    v_face.find(v1)->second.second,
                    v_face.find(v2)->second.second
                );
            }

            // fill edge
            for (size_t i = 1; i <= n_entry; ++i) {
                // add face (side)
                sm->add_face(v_side[i].second, v_side[i - 1].second, v_side[i - 1].first);
                sm->add_face(v_side[i].second, v_side[i - 1].first, v_side[i].first);
            }

            return pm;
        }

        // initialize plane
        for (size_t j = 0; j < plane.size(); ++j) {
            pm->pushPlane(plane[j]);
        }

        return pm;
    }


    std::shared_ptr<PrimitiveMesh> PrimitiveMeshFactory::file(const char* file_name) {
        std::shared_ptr<PrimitiveMesh> pm = std::make_shared<PrimitiveMesh>();
        PolygonMesh*                   sm = pm->meshPointer();
        if (!CGAL::Polygon_mesh_processing::IO::read_polygon_mesh(file_name, *sm)) {
            pm.~shared_ptr();  // initialize
            pm = std::make_shared<PrimitiveMesh>();
        }
        return pm;
    }


    void transform(PolygonMesh* sm, const mcutil::Affine& affine) {
        mcutil::Affine::matrix mat = affine.affine();
        CGAL::Aff_transformation_3<GT>
            trans(
                mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                mat[2][0], mat[2][1], mat[2][2], mat[2][3]
            );
        CGAL::Polygon_mesh_processing::transform(trans, *sm);
    }


    Point_3 transform(Point_3 point, const mcutil::Affine& affine) {
        mcutil::Affine::matrix mat = affine.affine();
        double pn[3] = { 0.0, 0.0, 0.0 };
        double po[3] = { point.x(), point.y(), point.z() };
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 3; ++j)
                pn[i] += mat[i][j] * po[j];
            pn[i] += mat[i][3];
        }
        return Point_3(pn[0], pn[1], pn[2]);
    }


    double4 transform(double4 plane, const mcutil::Affine& affine) {
        mcutil::Affine         inv = affine.inverse();
        mcutil::Affine::matrix mat = inv.affine();
        double pn[4] = { 0.0, 0.0, 0.0, plane.w };
        double po[4] = { plane.x, plane.y, plane.z, plane.w };
        for (size_t i = 0; i < 4; ++i) {
            for (size_t j = 0; j < 3; ++j)
                pn[i] += mat[j][i] * po[j];
        }
        return { pn[0], pn[1], pn[2], pn[3] };
    }


    double4 normalize(double4 plane) {
        double norm = sqrt(
            plane.x * plane.x +
            plane.y * plane.y +
            plane.z * plane.z
        );
        norm = 1.e0 / norm;
        return make_double4(
            plane.x * norm,
            plane.y * norm,
            plane.z * norm,
            plane.w * norm
        );
    }


    double angleBetweenEntryPoints(double2 p0, double2 p1, double2 p2) {
        double norm, theta, curl;
        double3 v1 = make_double3(p1.x - p0.x, 0.e0, p1.y - p0.y);
        double3 v2 = make_double3(p2.x - p1.x, 0.e0, p2.y - p1.y);
        // normalize
        norm = sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
        v1.x /= norm;
        v1.y /= norm;
        v1.z /= norm;
        norm = sqrt(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z);
        v2.x /= norm;
        v2.y /= norm;
        v2.z /= norm;

        theta = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
        theta = std::max(std::min(theta, 1.e0), -1.e0);
        theta = acos(theta) * 180.e0 / M_PI;

        curl = v1.z * v2.x - v1.x * v2.z;
        if (curl < 0.e0)
            theta += 180.e0;

        return theta;
    }


    double calculatePolygonCircleRadius(
        double radius,
        double height,
        uint32_t n_vertices,
        APPROXIMATION_TYPE method
    ) {
        double rad_eff;
        if (method == APPROXIMATION_TYPE::QUAD_ENCIRCLE) {
            rad_eff = radius;
        }
        else if (method == APPROXIMATION_TYPE::MESH_ENCIRCLE) {
            rad_eff = M_PI / (double)n_vertices;
            rad_eff = radius / cos(rad_eff);
        }
        else if (method == APPROXIMATION_TYPE::AREA_CONSERVE) {
            double a = (double)n_vertices * sin(2.0 * M_PI / (double)n_vertices);
            double b = (double)n_vertices * 2.0 * height * sin(M_PI / (double)n_vertices);
            double c = -2.0 * M_PI * radius * (radius + height);
            rad_eff  = (-b + sqrt(b * b - 4.0 * a * c)) / 2.0 / a;
        }
        else if (method == APPROXIMATION_TYPE::VOLUME_CONSERVE) {
            rad_eff = 2.0 * M_PI / (double)n_vertices;
            rad_eff = radius * sqrt(rad_eff / sin(rad_eff));
        }
        else
            assert(false);
        return rad_eff;
    }


    Point_3 centerOfMass(Point_3 p0, Point_3 p1, Point_3 p2) {
        return Point_3(
            (p0.x() + p1.x() + p2.x()) / 3.0,
            (p0.y() + p1.y() + p2.y()) / 3.0,
            (p0.z() + p1.z() + p2.z()) / 3.0
        );
    }


    void correctPolygonSphereRadius(
        PolygonMesh*       mesh,
        double             radius,
        APPROXIMATION_TYPE method
    ) {
        double norm;
        if (method == APPROXIMATION_TYPE::QUAD_ENCIRCLE) {
            norm = 1.0;
        }
        else if (method == APPROXIMATION_TYPE::MESH_ENCIRCLE) {
            double min_dist = radius * radius * 1e8;
            GT::Compute_squared_distance_3 ft;
            for (PolygonMesh::halfedge_iterator he = mesh->halfedges_begin();
                he != mesh->halfedges_end(); ++he) {
                double dist = ft(centerOfMass(
                    mesh->point(mesh->target(*he)),
                    mesh->point(mesh->target(mesh->next(*he))),
                    mesh->point(mesh->target(mesh->prev(*he)))
                ), Point_3(0.0, 0.0, 0.0));
                min_dist = (dist < min_dist) ? dist : min_dist;
            }
            norm = radius / sqrt(min_dist);
        }
        else if (method == APPROXIMATION_TYPE::AREA_CONSERVE) {
            double area = 0;
            GT::Compute_area_3 ft;
            for (auto f : mesh->faces()) {
                auto he = mesh->halfedge(f);
                const auto& p0 = mesh->point(mesh->target(he));
                const auto& p1 = mesh->point(mesh->target(mesh->next(he)));
                const auto& p2 = mesh->point(mesh->target(mesh->prev(he)));
                area += ft(p0, p1, p2);
            }
            norm = sqrt(4.0 * radius * radius * M_PI / area);
        }
        else if (method == APPROXIMATION_TYPE::VOLUME_CONSERVE) {
            double volume = 0.;
            for (auto f : mesh->faces()) {
                auto he = mesh->halfedge(f);
                const auto& p0 = mesh->point(mesh->target(he));
                const auto& p1 = mesh->point(mesh->target(mesh->next(he)));
                const auto& p2 = mesh->point(mesh->target(mesh->prev(he)));

                volume += p0.x() * p1.y() * p2.z();
                volume += p1.x() * p2.y() * p0.z();
                volume += p2.x() * p0.y() * p1.z();
                volume -= p2.x() * p1.y() * p0.z();
                volume -= p1.x() * p0.y() * p2.z();
                volume -= p0.x() * p2.y() * p1.z();
            }
            volume = std::fabs(volume) / 6.0;
            norm = cbrt(4.0 / 3.0 * radius * radius * radius * M_PI / volume);
        }
        else
            assert(false);

        mcutil::Affine affine(
            norm, 0.0, 0.0, 0.0,
            0.0, norm, 0.0, 0.0,
            0.0, 0.0, norm, 0.0
        );
        transform(mesh, affine);
    }


    std::vector<float3> extractVertices(const PolygonMesh& sm, double epsilon) {
        epsilon *= epsilon;
        GT::Compute_squared_distance_3 ft;
        GT::Compute_area_3 fa;
        std::set<PolygonMesh::face_index> f_set;
        std::vector<float3> vertices;
        vertices.reserve(sm.number_of_faces());
        for (PolygonMesh::halfedge_iterator hei = sm.halfedges_begin();
            hei != sm.halfedges_end(); ++hei) {
            PolygonMesh::face_index fi = sm.face(*hei);
            if (f_set.find(fi) == f_set.end()) {
                f_set.insert(fi);
                Point_3 v[3];
                v[0] = sm.point(sm.target(sm.prev(*hei)));
                v[1] = sm.point(sm.target(*hei));
                v[2] = sm.point(sm.target(sm.next(*hei)));
                if (ft(v[0], v[1]) < epsilon  ||
                    ft(v[0], v[2]) < epsilon  ||
                    ft(v[1], v[2]) < epsilon  || 
                    fa(v[1], v[2], v[3]) < epsilon)
                    continue;
                for (uint32_t i=0; i<3; ++i)
                    vertices.push_back(make_float3(v[i].x(), v[i].y(), v[i].z()));
            }
        }
        return vertices;
    }


    void repairCorefinedMesh(PolygonMesh* sm, double epsilon) {
        epsilon *= epsilon;
        // generate new mesh
        GT::Compute_squared_distance_3 ft;
        PolygonMesh sm_new;
        std::set<PolygonMesh::face_index> target_face_list;
        for (PolygonMesh::halfedge_iterator he = sm->halfedges_begin();
            he != sm->halfedges_end(); ++he) {
            if (target_face_list.find(sm->face(*he)) != target_face_list.end())
                continue;
            target_face_list.insert(sm->face(*he));
            // search duplicated point
            PolygonMesh::vertex_index vi[3];
            Point_3 point[3];
            point[0] = sm->point(sm->target(sm->prev(*he)));
            point[1] = sm->point(sm->target(*he));
            point[2] = sm->point(sm->target(sm->next(*he)));
            for (size_t i = 0; i < 3; ++i) {
                bool duplicated_point_found = false;
                for (PolygonMesh::vertex_iterator ve = sm_new.vertices_begin();
                    ve != sm_new.vertices_end() && !duplicated_point_found; ++ve) {
                    if (ft(point[i], sm_new.point(*ve)) < epsilon) {
                        vi[i] = *ve;
                        duplicated_point_found = true;
                    }
                }
                if (!duplicated_point_found)
                    vi[i] = sm_new.add_vertex(point[i]);
            }
            sm_new.add_face(vi[0], vi[1], vi[2]);
        }
        // update mesh
        sm->clear();
        *sm = sm_new;
    }
    
}