
#include <sutil/CUDAOutputBuffer.h>
#include <sutil/sutil.h>
#include <sutil/Exception.h>

#include "surface.h"


namespace mcutil {
	
	card::ArgumentCard geo::RotDefi::_ARGCARD("ROT_DEFI");


	template <>
	void _ArgTypeContext<geo::RotDefi>::_setCard() {
		char   vc_default = 'x';
		char   vc_minimum = 'x';
		char   vc_maximum = 'z';
		double vd_default[3] = { 0.0 };
		_ARGCARD.push_back<char>("axis", 1, &vc_default, &vc_minimum, &vc_maximum);
		_ARGCARD.push_back<std::string>("name", 1);
		_ARGCARD.push_back<double>("polar", 1, vd_default);
		_ARGCARD.push_back<double>("azm", 1, vd_default);
		_ARGCARD.push_back<double>("delta", 3, vd_default);
	}


	const std::string geo::EntryPoints::_LIST_KEY("ENTRY");
	const std::string geo::EntryPoints::_DELIMITER("");


	card::ArgumentCard geo::SurfaceBegin::_ARGCARD("SURFACE_BEGIN");
	card::ArgumentCard geo::SurfaceEnd::_ARGCARD("SURFACE_END");
	card::ArgumentCard geo::TransformBegin::_ARGCARD("TRANSFORM_BEGIN");
	card::ArgumentCard geo::TransformEnd::_ARGCARD("TRANSFORM_END");

	card::ArgumentCard geo::CardVoxel::_ARGCARD("VOXEL");
	card::ArgumentCard geo::CardRevolution::_ARGCARD("REVOLUTION");
	card::ArgumentCard geo::CardUVSphere::_ARGCARD("UVSPHERE");
	card::ArgumentCard geo::CardCylinder::_ARGCARD("CYLINDER");
	card::ArgumentCard geo::CardCone::_ARGCARD("CONE");
	card::ArgumentCard geo::CardIcoSphere::_ARGCARD("ICOSPHERE");
	card::ArgumentCard geo::CardCube::_ARGCARD("CUBE");
	card::ArgumentCard geo::CardToroid::_ARGCARD("TOROID");

	template <>
	void _ArgTypeContext<geo::SurfaceBegin>::_setCard() {}


	template <>
	void _ArgTypeContext<geo::SurfaceEnd>::_setCard() {}


	template <>
	void _ArgTypeContext<geo::TransformBegin>::_setCard() {
		_ARGCARD.push_back<std::string>("target", 1);
	}


	template <>
	void _ArgTypeContext<geo::TransformEnd>::_setCard() {}


}

namespace geo {

	/*
	Logical surface transform
	*/


	RotDefi::RotDefi() :
		_ArgTypeContext<RotDefi>() {}


	RotDefi::RotDefi(const std::map<std::string, card::ArgContainer>& data) :
	    _ArgTypeContext<RotDefi>() {
		std::vector<double> delta;
		char axis_char = data.find("axis")->second.get<char>()[0];
		switch (axis_char) {
		case 'x':
			this->_type = SURFACE_TRANSFORM_AXIS::X;
			break;
		case 'y':
			this->_type = SURFACE_TRANSFORM_AXIS::Y;
			break;
		case 'z':
			this->_type = SURFACE_TRANSFORM_AXIS::Z;
		default:
			break;
		}
		this->_name      = data.find("name")
			->second.get<std::string>()[0];
		this->_polar     = data.find("polar")
			->second.get<double>()[0];
		this->_azimuthal = data.find("azm")
			->second.get<double>()[0];
		delta            = data.find("delta")
			->second.get<double>();
		this->_translate = make_double3(delta[0], delta[1], delta[2]);
	}


	template<> 
	MeshTransformFtn RotDefi::getRotationMatrix() const {
		MeshTransformFtn ftn;
		switch (this->_type) {
		case SURFACE_TRANSFORM_AXIS::X:
			ftn.rot1 = &mesh::PrimitiveMesh::rotateX;
			ftn.rot2 = &mesh::PrimitiveMesh::rotateZ;
			break;
		case SURFACE_TRANSFORM_AXIS::Y:
			ftn.rot1 = &mesh::PrimitiveMesh::rotateY;
			ftn.rot2 = &mesh::PrimitiveMesh::rotateX;
			break;
		case SURFACE_TRANSFORM_AXIS::Z:
			ftn.rot1 = &mesh::PrimitiveMesh::rotateZ;
			ftn.rot2 = &mesh::PrimitiveMesh::rotateY;
		}
		return ftn;
	}


	template<>
	VectorTransformFtn RotDefi::getRotationMatrix() const {
		VectorTransformFtn ftn;
		switch (this->_type) {
		case SURFACE_TRANSFORM_AXIS::X:
			ftn.rot1 = mesh::rotateX;
			ftn.rot2 = mesh::rotateZ;
			break;
		case SURFACE_TRANSFORM_AXIS::Y:
			ftn.rot1 = mesh::rotateY;
			ftn.rot2 = mesh::rotateX;
			break;
		case SURFACE_TRANSFORM_AXIS::Z:
			ftn.rot1 = mesh::rotateZ;
			ftn.rot2 = mesh::rotateY;
		}
		return ftn;
	}


	std::string RotDefi::name() const {
		return this->_name;
	}


	SURFACE_TRANSFORM_AXIS RotDefi::type() const {
		return this->_type;
	}


	double RotDefi::polar() const {
		return this->_polar;
	}


	double RotDefi::azimuthal() const {
		return this->_azimuthal;
	}


	double3 RotDefi::translate() const {
		return this->_translate;
	}


	/*
	Revolution family entry point
	*/

	EntryPoints::EntryPoints(const std::deque<std::string>& container) {
		mcutil::CallResult res;
		// check length
		size_t str_size = container.size();
		if (str_size < 2 || str_size % 2)
			mcutil::output.fatalInsufficient(_LIST_KEY);
		// check name is valid
		const std::string& name = container[1];
		if (card::isHasSpecialSymbol(name))
			mcutil::output.fatalNameInvalid(name);
		if (card::isHasDigitHeader(name))
			mcutil::output.fatalNameInvalid(name);
		this->_name = name;
		// push entry points
		for (size_t i = 2; i < str_size; i += 2) {
			double point[2];
			for (size_t j = 0; j < 2; ++j) {
				point[j] = card::stringTo<double>(container[i + j], &res);
				if (res.level <= mcutil::LOG_LEVEL::ERR) {
					res.message << " in entry point '";
					res.message << container[i + j] << "'";
					mcutil::output.write(res);
				}
			}
			this->_entry.push_back(make_double2(point[0], point[1]));
		}
	}


	const std::string& EntryPoints::name() const {
		return this->_name;
	}


	const std::vector<double2>& EntryPoints::entry() const {
		return this->_entry;
	}


	/*
	Surface series
	*/

	SurfaceBegin::SurfaceBegin() :
		_ArgTypeContext<SurfaceBegin>() {}


	SurfaceBegin::SurfaceBegin(const std::map<std::string, card::ArgContainer>& data) :
		_ArgTypeContext<SurfaceBegin>() {}


	SurfaceEnd::SurfaceEnd() :
		_ArgTypeContext<SurfaceEnd>() {}


	SurfaceEnd::SurfaceEnd(const std::map<std::string, card::ArgContainer>& data) :
		_ArgTypeContext<SurfaceEnd>() {}


	TransformBegin::TransformBegin() :
		_ArgTypeContext<TransformBegin>() {}


	TransformBegin::TransformBegin(const std::map<std::string, card::ArgContainer>& data) :
		_ArgTypeContext<TransformBegin>() {
		this->_target = data.find("target")->second.get<std::string>()[0];
	}


	TransformEnd::TransformEnd() :
		_ArgTypeContext<TransformEnd>() {}


	TransformEnd::TransformEnd(const std::map<std::string, card::ArgContainer>& data) :
		_ArgTypeContext<TransformEnd>() {}


	card::ArgumentCard SurfaceSeries::_ARGCARD_SURFACE_BEGIN("SURFACE_BEGIN");
	std::vector<card::ArgumentCard> SurfaceSeries::_ARGCARD_SURFACE_SERIES;

	void SurfaceSeries::setCard() {
		if (!_ARGCARD_SURFACE_SERIES.empty())
			return;

		_ARGCARD_SURFACE_SERIES.push_back(card::ArgumentCard("SURFACE_END"));
		_ARGCARD_SURFACE_SERIES.push_back(card::ArgumentCard("TRANSFORM_BEGIN"));
		_ARGCARD_SURFACE_SERIES.push_back(card::ArgumentCard("TRANSFORM_END"));
		_ARGCARD_SURFACE_SERIES.push_back(card::ArgumentCard("VOXEL"));
		_ARGCARD_SURFACE_SERIES.push_back(card::ArgumentCard("REVOLUTION"));
		_ARGCARD_SURFACE_SERIES.push_back(card::ArgumentCard("UVSPHERE"));
		_ARGCARD_SURFACE_SERIES.push_back(card::ArgumentCard("CYLINDER"));
		_ARGCARD_SURFACE_SERIES.push_back(card::ArgumentCard("CONE"));
		_ARGCARD_SURFACE_SERIES.push_back(card::ArgumentCard("ICOSPHERE"));
		_ARGCARD_SURFACE_SERIES.push_back(card::ArgumentCard("CUBE"));
		_ARGCARD_SURFACE_SERIES.push_back(card::ArgumentCard("TOROID"));
		_ARGCARD_SURFACE_SERIES.push_back(card::ArgumentCard("TORUS"));
		_ARGCARD_SURFACE_SERIES.push_back(card::ArgumentCard("MODEL"));

		int    vi_default[2];
		int    vi_minimum[2];
		int    vi_maximum[2];
		double vd_default[3] = { 0.0, 0.0, 1.0 };
		double vd_minimum[3];
		double vd_maximum[3];

		card::ArgumentCard& transform_begin = _ARGCARD_SURFACE_SERIES[1];
		transform_begin.push_back<std::string>("target", 1);

		card::ArgumentCard& voxel           = _ARGCARD_SURFACE_SERIES[3];
		voxel.push_back<std::string>("name", 1);
		voxel.push_back<std::string>("file", 1);

		card::ArgumentCard& revolution      = _ARGCARD_SURFACE_SERIES[4];
		revolution.push_back<std::string>("name", 1);
		revolution.push_back<std::string>("entry", 1);
		revolution.push_back<double>("center", 3);
		revolution.push_back<double>("direction", 3, vd_default);
		vi_default[0] = MESH_DEFAULT_VERTICES;
		vi_minimum[0] = MESH_MINIMUM_VERTICES;
		vi_maximum[0] = MESH_MAXIMUM_VERTICES;
		revolution.push_back<int>("vertices", 1, 
			vi_default, vi_minimum, vi_maximum);

		card::ArgumentCard& uv_sphere        = _ARGCARD_SURFACE_SERIES[5];
		uv_sphere.push_back<std::string>("name", 1);
		uv_sphere.push_back<double>("center", 3);
		uv_sphere.push_back<double>("direction", 3, vd_default);
		vd_minimum[0] = MESH_MINIMUM_SIZE;
		vd_maximum[0] = MESH_MAXIMUM_SIZE;
		uv_sphere.push_back<double>("radius", 1, vd_minimum, vd_maximum);
		vi_default[0] = MESH_DEFAULT_VERTICES;
		vi_minimum[0] = MESH_MINIMUM_VERTICES;
		vi_maximum[0] = MESH_MAXIMUM_VERTICES;
		vi_default[1] = MESH_DEFAULT_VERTICES;
		vi_minimum[1] = MESH_MINIMUM_VERTICES;
		vi_maximum[1] = MESH_MAXIMUM_VERTICES;
		uv_sphere.push_back<int>("vertices", 2,
			vi_default, vi_minimum, vi_maximum);

		card::ArgumentCard& cylinder        = _ARGCARD_SURFACE_SERIES[6];
		cylinder.push_back<std::string>("name", 1);
		cylinder.push_back<double>("center", 3);
		cylinder.push_back<double>("height", 3);
		vd_minimum[0] = MESH_MINIMUM_SIZE;
		vd_maximum[0] = MESH_MAXIMUM_SIZE;
		cylinder.push_back<double>("radius", 1, vd_minimum, vd_maximum);
		vi_default[0] = MESH_DEFAULT_VERTICES;
		vi_minimum[0] = MESH_MINIMUM_VERTICES;
		vi_maximum[0] = MESH_MAXIMUM_VERTICES;
		cylinder.push_back<int>("vertices", 1,
			vi_default, vi_minimum, vi_maximum);

		card::ArgumentCard& cone            = _ARGCARD_SURFACE_SERIES[7];
		cone.push_back<std::string>("name", 1);
		cone.push_back<double>("center", 3);
		cone.push_back<double>("height", 3);
		vd_minimum[0] = MESH_MINIMUM_SIZE;
		vd_maximum[0] = MESH_MAXIMUM_SIZE;
		cone.push_back<double>("radius", 1, vd_minimum, vd_maximum);
		vi_default[0] = MESH_DEFAULT_VERTICES;
		vi_minimum[0] = MESH_MINIMUM_VERTICES;
		vi_maximum[0] = MESH_MAXIMUM_VERTICES;
		cone.push_back<int>("vertices", 1,
			vi_default, vi_minimum, vi_maximum);

		card::ArgumentCard& ico_sphere      = _ARGCARD_SURFACE_SERIES[8];
		ico_sphere.push_back<std::string>("name", 1);
		ico_sphere.push_back<double>("center", 3);
		vd_minimum[0] = MESH_MINIMUM_SIZE;
		vd_maximum[0] = MESH_MAXIMUM_SIZE;
		ico_sphere.push_back<double>("radius", 1, vd_minimum, vd_maximum);
		vi_default[0] = MESH_ICOSPHERE_DEFAULT_ORDER;
		vi_minimum[0] = MESH_ICOSPHERE_MINIMUM_ORDER;
		vi_maximum[0] = MESH_ICOSPHERE_MAXIMUM_ORDER;
		ico_sphere.push_back<int>("order", 1,
			vi_default, vi_minimum, vi_maximum);

		card::ArgumentCard& cube            = _ARGCARD_SURFACE_SERIES[9];
		cube.push_back<std::string>("name", 1);
		cube.push_back<double>("center", 3);
		vd_minimum[0] = MESH_MINIMUM_SIZE;
		vd_maximum[0] = MESH_MAXIMUM_SIZE;
		vd_minimum[1] = MESH_MINIMUM_SIZE;
		vd_maximum[1] = MESH_MAXIMUM_SIZE;
		vd_minimum[2] = MESH_MINIMUM_SIZE;
		vd_maximum[2] = MESH_MAXIMUM_SIZE;
		cube.push_back<double>("size", 3, vd_minimum, vd_maximum);

		card::ArgumentCard& toroid          = _ARGCARD_SURFACE_SERIES[10];
		toroid.push_back<std::string>("name", 1);
		toroid.push_back<std::string>("entry", 1);
		toroid.push_back<double>("center", 3);
		toroid.push_back<double>("direction", 3, vd_default);
		vi_default[0] = MESH_DEFAULT_VERTICES;
		vi_minimum[0] = MESH_MINIMUM_VERTICES;
		vi_maximum[0] = MESH_MAXIMUM_VERTICES;
		vi_default[1] = MESH_DEFAULT_VERTICES;
		vi_minimum[1] = MESH_MINIMUM_VERTICES;
		vi_maximum[1] = MESH_MAXIMUM_VERTICES;
		toroid.push_back<int>("vertices", 2,
			vi_default, vi_minimum, vi_maximum);

		card::ArgumentCard& torus            = _ARGCARD_SURFACE_SERIES[11];
		torus.push_back<std::string>("name", 1);
		torus.push_back<double>("center", 3);
		torus.push_back<double>("direction", 3, vd_default);
		vd_minimum[0] = MESH_MINIMUM_SIZE;
		vd_maximum[0] = MESH_MAXIMUM_SIZE;
		vd_minimum[1] = MESH_MINIMUM_SIZE;
		vd_maximum[1] = MESH_MAXIMUM_SIZE;
		torus.push_back<double>("radius", 2, vd_minimum, vd_maximum);
		vi_default[0] = MESH_DEFAULT_VERTICES;
		vi_minimum[0] = MESH_MINIMUM_VERTICES;
		vi_maximum[0] = MESH_MAXIMUM_VERTICES;
		vi_default[1] = MESH_DEFAULT_VERTICES;
		vi_minimum[1] = MESH_MINIMUM_VERTICES;
		vi_maximum[1] = MESH_MAXIMUM_VERTICES;
		torus.push_back<int>("vertices", 2,
			vi_default, vi_minimum, vi_maximum);

		card::ArgumentCard& model             = _ARGCARD_SURFACE_SERIES[12];
		model.push_back<std::string>("name", 1);
		model.push_back<std::string>("file", 1);
		vd_default[0] = 1.0;
		model.push_back<double>("scale", 1, vd_default);

		return;
	}


	void SurfaceSeries::setPosition(mcutil::Input& input) {
		input.init();
		input.findArg(SurfaceSeries::_ARGCARD_SURFACE_BEGIN);
	}


	const std::vector<card::ArgumentCard>& SurfaceSeries::argSeries() {
		return SurfaceSeries::_ARGCARD_SURFACE_SERIES;
	}


	/*
	Base triangle segment
	*/

	float3 MarkedTriangle::center() {
		float3 center;
		center.x = (this->vertex[0].x + this->vertex[1].x + this->vertex[2].x) / 3.f;
		center.y = (this->vertex[0].y + this->vertex[1].y + this->vertex[2].y) / 3.f;
		center.z = (this->vertex[0].z + this->vertex[1].z + this->vertex[2].z) / 3.f;
		return center;
	}


	float3 MarkedTriangle::normal() {
		float3 v1, v2, nv;
		float norm;
		v1 = make_float3(
			this->vertex[1].x - this->vertex[0].x,
			this->vertex[1].y - this->vertex[0].y,
			this->vertex[1].z - this->vertex[0].z);
		v2 = make_float3(
			this->vertex[2].x - this->vertex[1].x,
			this->vertex[2].y - this->vertex[1].y,
			this->vertex[2].z - this->vertex[1].z);
		nv = make_float3(
			v1.y * v2.z - v2.y * v1.z,
			v1.z * v2.x - v2.z * v1.x,
			v1.x * v2.y - v2.x * v1.y
		);
		norm = sqrt(nv.x * nv.x + nv.y * nv.y + nv.z * nv.z);
		nv.x /= norm;
		nv.y /= norm;
		nv.z /= norm;
		return nv;
	}


	float MarkedTriangle::area() {
		float s, r[3];
		for (int i = 0; i < 3; ++i) {
			float3 v;
			v = make_float3(
				this->vertex[(i + 1) % 3].x - this->vertex[i % 3].x,
				this->vertex[(i + 1) % 3].y - this->vertex[i % 3].y,
				this->vertex[(i + 1) % 3].z - this->vertex[i % 3].z);
			r[i] = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		}
		s = (r[0] + r[1] + r[2]) * 0.5f;
		return sqrt(max(0.f, s * (s - r[0]) * (s - r[1]) * (s - r[2])));
	}


	/*
	OptiX surface, for the corefining
	*/

	OptixInitialScene::OptixInitialScene() 
		: _has_device_mem(false) {}


	OptixInitialScene::~OptixInitialScene() {
		this->free();
	}


	void OptixInitialScene::load(
		const mesh::PolygonMesh&  mesh_init, 
		OptixDeviceContext        context
	) {

		if (this->_has_device_mem) {
			CUDA_CHECK(cudaFree(reinterpret_cast<void*>(this->_d_gas_output_buffer)));
		}

		std::vector<float3> vertices = mesh::extractVertices(mesh_init);

		OptixAccelBuildOptions accel_options = {};
		accel_options.buildFlags = OPTIX_BUILD_FLAG_NONE;
		accel_options.operation  = OPTIX_BUILD_OPERATION_BUILD;

		const size_t vertices_size = sizeof(float3) * vertices.size();
		CUdeviceptr d_vertices = 0;
		CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_vertices), vertices_size));
		CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_vertices), vertices.data(), 
			vertices_size, cudaMemcpyHostToDevice));

		const uint32_t flags[1] = { OPTIX_GEOMETRY_FLAG_NONE };
		OptixBuildInput input = {};
		input.type                              = OPTIX_BUILD_INPUT_TYPE_TRIANGLES;
		input.triangleArray.vertexFormat        = OPTIX_VERTEX_FORMAT_FLOAT3;
		input.triangleArray.vertexStrideInBytes = sizeof(float3);
		input.triangleArray.numVertices         = static_cast<uint32_t>(vertices.size());
		input.triangleArray.vertexBuffers       = &d_vertices;
		input.triangleArray.flags               = flags;
		input.triangleArray.numSbtRecords       = 1;

		OptixAccelBufferSizes gas_buffer_sizes;
		OPTIX_CHECK(optixAccelComputeMemoryUsage(
			context,
			&accel_options,
			&input,
			1, // Number of build inputs
			&gas_buffer_sizes
		));
		CUdeviceptr d_temp_buffer_gas;
		CUDA_CHECK(cudaMalloc(
			reinterpret_cast<void**>(&d_temp_buffer_gas),
			gas_buffer_sizes.tempSizeInBytes
		));
		this->_has_device_mem = true;
		CUDA_CHECK(cudaMalloc(
			reinterpret_cast<void**>(&this->_d_gas_output_buffer),
			gas_buffer_sizes.outputSizeInBytes
		));

		OPTIX_CHECK(optixAccelBuild(
			context,
			0,                  // CUDA stream
			&accel_options,
			&input,
			1,                  // num build inputs
			d_temp_buffer_gas,
			gas_buffer_sizes.tempSizeInBytes,
			this->_d_gas_output_buffer,
			gas_buffer_sizes.outputSizeInBytes,
			&this->_gas_handle,
			nullptr,            // emitted property list
			0                   // num emitted properties
		));

		// We can now free the scratch space buffer used during build and the vertex
		// inputs, since they are not needed by our trivial shading method
		CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_temp_buffer_gas)));
		CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_vertices)));
	}


	void OptixInitialScene::free() {
		if (this->_has_device_mem) {
			this->_has_device_mem = false;
			CUDA_CHECK(cudaFree(reinterpret_cast<void*>(this->_d_gas_output_buffer)));
		}
	}


	/*
	OptiX surface, for quadratic track-length correction handling
	*/

	OptixQuadratic::OptixQuadratic(
	) : _type(SURFACE_TYPE::ARBITRARY),
		_center(make_double3(0, 0, 0)),
		_radius(0),
		_height(make_double3(0, 0, 0)) {}


	OptixQuadratic::OptixQuadratic(
		SURFACE_TYPE type,
		double3      center,
		double       radius,
		double3      height
	) : _type(type), 
		_center(center), 
		_radius(radius),
		_height(height)
	{}


	double3 OptixQuadratic::center() const {
		return this->_center;
	}
	

	double OptixQuadratic::radius() const {
		return this->_radius;
	}


	double3 OptixQuadratic::height() const {
		return this->_height;
	}


	void OptixQuadratic::transform(RotDefi& rot) {
		// move center position
		double3 t = rot.translate();
		this->_center.x += t.x;
		this->_center.y += t.y;
		this->_center.z += t.z;
		// rotate vectors
		VectorTransformFtn vf = rot.getRotationMatrix<VectorTransformFtn>();
		double3* pointer[2];
		pointer[0] = &this->_center;
		pointer[1] = &this->_height;
		for (int i = 0; i < 2; ++i) {
			mesh::Point_3 vector(pointer[i]->x, pointer[i]->y, pointer[i]->z);
			vector = vf.rot1(vector, rot.azimuthal());
			vector = vf.rot2(vector, rot.polar());
			pointer[i]->x = vector.x();
			pointer[i]->y = vector.y();
			pointer[i]->z = vector.z();
		}
	}


	/*
	Logical surface primitives
	*/

	Surface::Surface(SURFACE_TYPE type, std::string name, size_t id) :
		_type(type),
		_name(name),
		_id(id),
		_transformer{
		    mesh::Point_3(1.0, 0.0, 0.0),
		    mesh::Point_3(0.0, 1.0, 0.0),
		    mesh::Point_3(0.0, 0.0, 1.0),
			mesh::Point_3(0.0, 0.0, 0.0)
		} {};


	Surface::~Surface() {
		// this->_mesh_init.clear();
	}


	SURFACE_TYPE Surface::type() const {
		return this->_type;
	}


	bool Surface::isCurve() const {
		return static_cast<int>(this->_type) % 2;
	}


	std::string Surface::name() const {
		return this->_name;
	}


	mesh::PrimitiveMesh& Surface::mesh_init() {
		return this->_mesh_init;
	}


	const std::vector<MarkedTriangle>& Surface::mesh_core() const {
		return this->_mesh_core;
	}


	const std::set<size_t>& Surface::neiSurface() const {
		return this->_rel_surf;
	}


	const std::set<size_t>& Surface::neiRegion() const {
		return this->_rel_cell;
	}


	std::vector<double> Surface::transformer() const {
		std::vector<double> transformer;
		for (size_t i = 0; i < 4; ++i) {
			transformer.push_back(this->_transformer[i].x());
			transformer.push_back(this->_transformer[i].y());
			transformer.push_back(this->_transformer[i].z());
		}
		return transformer;
	}


	void Surface::free() {
		// this->_mesh_init.clear();
		this->_mesh_core.clear();
	}


	size_t Surface::nInitVertices() const {
		return this->_mesh_init.number_of_vertices();
	}


	void Surface::writePrimitive(const std::string& file_name) const {
		std::ofstream out(file_name);
		this->_mesh_init.write(file_name.c_str());
	}


	void Surface::transform(RotDefi& rot) {
		double3 t = rot.translate();
		// transform mesh data
		MeshTransformFtn mf = rot.getRotationMatrix<MeshTransformFtn>();
		(this->_mesh_init.*mf.rot1)(rot.azimuthal());
		(this->_mesh_init.*mf.rot2)(rot.polar());
		this->_mesh_init.translate(t);
		// transform transformer data
		VectorTransformFtn vf = rot.getRotationMatrix<VectorTransformFtn>();
		for (size_t i = 0; i < 4; ++i) {
			this->_transformer[i] = vf.rot1(this->_transformer[i], rot.azimuthal());
		}
		this->_transformer[3] = 
			mesh::translate(this->_transformer[3], mesh::Vector_3(t.x, t.y, t.z));
		// transform quadratic data if exist
		if (static_cast<int>(this->_type) % 2)
			this->_quadratic.transform(rot);
	}


	void Surface::pushNeiSurface(size_t id) {
		if (id != this->_id)
			this->_rel_surf.insert(id);
	}


	void Surface::pushNeiRegion(size_t id) {
		this->_rel_cell.insert(id);
	}


	bool Surface::checkSelfIntersect() const {
		return this->_mesh_init.does_self_intersect();
	}


	bool Surface::checkIsClosed() const {
		return this->_mesh_init.is_closed();
	}


	bool Surface::compressInitialScene(OptixDeviceContext context) {
		this->_scene_init.load(*this->_mesh_init.mesh(), context);
		return true;
	}


	OptixTraversableHandle Surface::getInitialSceneHandle() const {
		OptixTraversableHandle gas_handle = 0x0;
		if (this->_scene_init._has_device_mem)
			gas_handle = this->_scene_init._gas_handle;
		return gas_handle;
	}


	void Surface::pushCorefinedFacet(const std::vector<MarkedTriangle>& facets) {
		this->_mesh_core.reserve(this->_mesh_core.size() + facets.size());
		for (uint64_t i = 0; i < facets.size(); ++i) {
			if (facets[i].region_idx_backface != facets[i].region_idx_frontface) {
				this->_mesh_core.push_back(facets[i]);
			}
		}
	}


	uint64_t Surface::numberOfCoreFacets() const {
		return this->_mesh_core.size();
	}


	void Surface::markCurveFacets() {
		if (!this->isCurve()) return;
		std::vector<double4> plane;
		for (size_t i = 0; i < this->_mesh_init.number_of_plane(); ++i) {
			plane.push_back(this->_mesh_init.plane(i).plane());
		}
		for (uint64_t i = 0; i < this->_mesh_core.size(); ++i) {
			float3 center = this->_mesh_core[i].center();
			this->_mesh_core[i].surface_idx = this->_id;
			for (uint32_t j = 0; j < plane.size(); ++j) {
				double pos_epsilon =
					plane[j].x * center.x +
					plane[j].y * center.y +
					plane[j].z * center.z +
					plane[j].w;
				if (abs(pos_epsilon) < MESH_PLANE_VERTEX_EPSILON) {
					this->_mesh_core[i].surface_idx = -1;
					break;
				}
			}
		}
	}


	void Surface::writeCorefined(const std::string& file_name) const {
		using namespace mesh;
		std::vector<Point_3> points;
		std::vector<CGAL_Polygon> polygons;
		for (uint32_t i = 0; i < this->_mesh_core.size(); ++i) {
			CGAL_Polygon p;
			for (uint32_t j = 0; j < 3; ++j) {
				points.push_back(Point_3(
					this->_mesh_core[i].vertex[j].x,
					this->_mesh_core[i].vertex[j].y,
					this->_mesh_core[i].vertex[j].z
					));
				p.push_back(points.size() - 1);
			}
			polygons.push_back(p);
		}
		CGAL::IO::write_OFF(file_name.c_str(), points, polygons);
	}


	const std::vector<unsigned char> Surface::compressQuadratic() {
		float buffer[7];
		double3 center = this->_quadratic.center();
		double  radius = this->_quadratic.radius();
		double3 height = this->_quadratic.height();
		buffer[0] = center.x;
		buffer[1] = center.y;
		buffer[2] = center.z;
		buffer[3] = radius;
		buffer[4] = height.x;
		buffer[5] = height.y;
		buffer[6] = height.z;
		std::vector<unsigned char> out;
		out.resize(sizeof(float) * 7);
		std::memcpy(&out[0], buffer, sizeof(float) * 7);
		return out;
	}


	/* surface factory */

	Surface SurfaceFactory::sphereUV(std::string name, SURFACE_MODEL model, size_t id, const double* args) {
		Surface surface(SURFACE_TYPE::SPHERE, name, id);
		double3 c = make_double3(args[0], args[1], args[2]);
		double r = args[3];
		size_t nv1 = (size_t)reinterpret_cast<const int64_t&>(args[4]);
		size_t nv2 = (size_t)reinterpret_cast<const int64_t&>(args[5]);
		surface._quadratic = OptixQuadratic(SURFACE_TYPE::SPHERE, c, r, make_double3(0, 0, 0));
		surface._mesh_init = mesh::PrimitiveMeshFactory::sphereUV(r, nv1, nv2);
		// surface correction method
		if (model == SURFACE_MODEL::POLYGON_PRIMITIVE)
			mesh::correctPolygonSphereRadius(surface._mesh_init.mesh(), r, mesh::APPROXIMATION_TYPE::AREA_CONSERVE);
		else
			mesh::correctPolygonSphereRadius(surface._mesh_init.mesh(), r, mesh::APPROXIMATION_TYPE::QUAD_ENCIRCLE);
		mesh::Vector_3 vector(c.x, c.y, c.z);
		mesh::translate(surface._mesh_init.mesh(), vector);
		surface._transformer[3] = mesh::translate(surface._transformer[3], vector);
		return surface;
	}


	Surface SurfaceFactory::sphereICO(std::string name, SURFACE_MODEL model, size_t id, const double* args) {
		Surface surface(SURFACE_TYPE::SPHERE, name, id);
		double3 c = make_double3(args[0], args[1], args[2]);
		double r = args[3];
		size_t order = (size_t)reinterpret_cast<const int64_t&>(args[6]);
		surface._quadratic = OptixQuadratic(SURFACE_TYPE::SPHERE, c, r, make_double3(0, 0, 0));
		surface._mesh_init = mesh::PrimitiveMeshFactory::sphereIco(r, order);
		// surface correction method
		if (model == SURFACE_MODEL::POLYGON_PRIMITIVE)
			mesh::correctPolygonSphereRadius(surface._mesh_init.mesh(), r, mesh::APPROXIMATION_TYPE::AREA_CONSERVE);
		else
			mesh::correctPolygonSphereRadius(surface._mesh_init.mesh(), r, mesh::APPROXIMATION_TYPE::QUAD_ENCIRCLE);
		mesh::Vector_3 vector(c.x, c.y, c.z);
		mesh::translate(surface._mesh_init.mesh(), vector);
		surface._transformer[3] = mesh::translate(surface._transformer[3], vector);
		return surface;
	}


	Surface SurfaceFactory::cylinder(std::string name, SURFACE_MODEL model, size_t id, const double* args) {
		Surface surface(SURFACE_TYPE::CYLINDER, name, id);
		double3 c = make_double3(args[0], args[1], args[2]);
		double r = args[3];
		double3 h = make_double3(args[4], args[5], args[6]);
		size_t nv = (size_t)reinterpret_cast<const int64_t&>(args[7]);
		surface._quadratic = OptixQuadratic(SURFACE_TYPE::CYLINDER, c, r, h);
		double height_distance = sqrt(h.x * h.x + h.y * h.y + h.z * h.z);
		h.x /= height_distance;  // unit vector of cylinder axis 
		h.y /= height_distance;
		h.z /= height_distance;

		// surface correction method
		if (model == SURFACE_MODEL::POLYGON_PRIMITIVE)
			r = mesh::calculatePolygonCircleRadius(r, nv, mesh::APPROXIMATION_TYPE::AREA_CONSERVE);
		else
			r = mesh::calculatePolygonCircleRadius(r, nv, mesh::APPROXIMATION_TYPE::QUAD_ENCIRCLE);

		surface._mesh_init = mesh::PrimitiveMeshFactory::cylinder(r, height_distance, nv);

		// rotating z-aligned cylinder
		double2 angle;
		angle = vectorToRotationAngle(h);
		// transform mesh
		surface._mesh_init.rotateX(angle.x);
		surface._mesh_init.rotateY(angle.y);
		surface._mesh_init.translate(c);
		// voxel transformer
		for (size_t i = 0; i < 4; ++i) {
			surface._transformer[i] = mesh::rotateX(surface._transformer[i], angle.x);
			surface._transformer[i] = mesh::rotateY(surface._transformer[i], angle.y);
		}
		surface._transformer[3] = mesh::translate(surface._transformer[3], mesh::Vector_3(c.x, c.y, c.z));
		return surface;
	}


	Surface SurfaceFactory::cone(std::string name, SURFACE_MODEL model, size_t id, const double* args) {
		Surface surface(SURFACE_TYPE::CONE, name, id);
		double3 c = make_double3(args[0], args[1], args[2]);
		double r = args[3];
		double3 h = make_double3(args[4], args[5], args[6]);
		size_t nv = (size_t)reinterpret_cast<const int64_t&>(args[7]);
		surface._quadratic = OptixQuadratic(SURFACE_TYPE::CONE, c, r, h);
		double height_distance = sqrt(h.x * h.x + h.y * h.y + h.z * h.z);
		h.x /= height_distance;  // unit vector of cylinder axis 
		h.y /= height_distance;
		h.z /= height_distance;

		// surface correction method
		if (model == SURFACE_MODEL::POLYGON_PRIMITIVE)
			r = mesh::calculatePolygonCircleRadius(r, nv, mesh::APPROXIMATION_TYPE::AREA_CONSERVE);
		else
			r = mesh::calculatePolygonCircleRadius(r, nv, mesh::APPROXIMATION_TYPE::QUAD_ENCIRCLE);

		surface._mesh_init = mesh::PrimitiveMeshFactory::cone(r, height_distance, nv);

		// rotating z-aligned cylinder
		double2 angle;
		angle = vectorToRotationAngle(h);
		// transform mesh
		surface._mesh_init.rotateX(angle.x);
		surface._mesh_init.rotateY(angle.y);
		surface._mesh_init.translate(c);
		// voxel transformer
		for (size_t i = 0; i < 4; ++i) {
			surface._transformer[i] = mesh::rotateX(surface._transformer[i], angle.x);
			surface._transformer[i] = mesh::rotateY(surface._transformer[i], angle.y);
		}
		surface._transformer[3] = mesh::translate(surface._transformer[3], mesh::Vector_3(c.x, c.y, c.z));
		return surface;
	}


	Surface SurfaceFactory::cube(std::string name, size_t id, const double* args) {
		Surface surface(SURFACE_TYPE::CUBE, name, id);
		double3 c = make_double3(args[0], args[1], args[2]);
		double3 d = make_double3(args[3], args[4], args[5]);
		surface._mesh_init = mesh::PrimitiveMeshFactory::cube(d.x, d.y, d.z);
		surface._mesh_init.translate(c);
		surface._transformer[3] = mesh::translate(surface._transformer[3], mesh::Vector_3(c.x, c.y, c.z));
		return surface;
	}


	Surface SurfaceFactory::torus(std::string name, size_t id, const double* args) {
		Surface surface(SURFACE_TYPE::TORUS, name, id);
		double3 c = make_double3(args[0], args[1], args[2]);
		double r1 = args[3];
		double r2 = args[4];
		uint32_t nv1 = (uint32_t)reinterpret_cast<const int64_t&>(args[5]);
		uint32_t nv2 = (uint32_t)reinterpret_cast<const int64_t&>(args[6]);

		r1 = mesh::calculatePolygonCircleRadius(r1, nv1, mesh::APPROXIMATION_TYPE::AREA_CONSERVE);
		r2 = mesh::calculatePolygonCircleRadius(r2, nv2, mesh::APPROXIMATION_TYPE::AREA_CONSERVE);
		surface._mesh_init = mesh::PrimitiveMeshFactory::torus(r1, r2, nv1, nv2);
		surface._mesh_init.translate(c);
		surface._transformer[3] = mesh::translate(surface._transformer[3], mesh::Vector_3(c.x, c.y, c.z));
		return surface;
	}


	Surface SurfaceFactory::model(std::string name, std::string file_name, size_t id, const double* args) {
		Surface surface(SURFACE_TYPE::ARBITRARY, name, id);
		surface._mesh_init = mesh::PrimitiveMeshFactory::file(file_name.c_str());
		mesh::scaling(surface._mesh_init.mesh(), args[0]);
		return surface;
	}


	Surface SurfaceFactory::voxel(std::string name, size_t id, double3 origin, double3 size) {
		Surface surface(SURFACE_TYPE::VOXEL, name, id);
		double3 center;
		center.x = origin.x + size.x * 0.5;
		center.y = origin.y + size.y * 0.5;
		center.z = origin.z + size.z * 0.5;
		surface._mesh_init = mesh::PrimitiveMeshFactory::cube(size.x, size.y, size.z);
		surface._mesh_init.translate(center);
		surface._transformer[3] = mesh::translate(surface._transformer[3],
			mesh::Vector_3(origin.x, origin.y, origin.z));
		return surface;
	}


	Surface SurfaceFactory::error() {
		Surface surface(SURFACE_TYPE::ARBITRARY, "", 0);
		return surface;
	}


	/*
	template <typename T>
	void SurfaceBuildInput<T>::_getVertexAndRadius(size_t n, T* surfaces, float3** vertex, float** radius) {
		uint32_t data_per_instance = surfaces->DATA_POINT_PER_INSTANCE;
		*vertex = new float3[(uint64_t)n * data_per_instance];
		if (surfaces->DATA_POINT_HAS_RADIUS)
			*radius = new float[(uint64_t)n * data_per_instance];
		for (uint32_t i = 0; i < n; ++i) {
			for (uint32_t j = 0; j < data_per_instance; ++j) {
				(*vertex)[i * data_per_instance + j].x = surfaces[i].vertex[j].x;
				(*vertex)[i * data_per_instance + j].y = surfaces[i].vertex[j].y;
				(*vertex)[i * data_per_instance + j].z = surfaces[i].vertex[j].z;
				if (surfaces->DATA_POINT_HAS_RADIUS)
					(*radius)[i * data_per_instance + j] = surfaces[i].radius[j];
			}
		}
	}

	template <typename T>
	uint32_t* SurfaceBuildInput<T>::_getSbtTable(uint32_t n, T* surfaces) {
		uint32_t* mat_indices = new uint32_t[n];
		for (uint32_t i = 0; i < n; ++i)
			mat_indices[i] = surfaces[i].internal_id;
		return mat_indices;
	}

	template <typename T>
	void SurfaceBuildInput<T>::_setTypeAndBuffer(
		uint32_t    n_vertices, 
		uint32_t    n_sbt, 
		CUdeviceptr vertex_buffer, 
		CUdeviceptr radius_buffer ) {}

	template <>
	void SurfaceBuildInput<OptixSphere>::_setTypeAndBuffer(
		uint32_t    n_vertices, 
		uint32_t    n_sbt, 
		CUdeviceptr vertex_buffer, 
		CUdeviceptr radius_buffer ) {
		this->_input.type                      = OPTIX_BUILD_INPUT_TYPE_SPHERES;
		this->_input.sphereArray.numVertices   = n_vertices;
		this->_input.sphereArray.vertexBuffers = &vertex_buffer;
		this->_input.sphereArray.radiusBuffers = &radius_buffer;

		cudaFree((void*)vertex_buffer);
		cudaFree((void*)radius_buffer);

		uint32_t* input_flags;  // One per SBT record for this build input
		input_flags = new uint32_t[n_sbt]{ OPTIX_GEOMETRY_FLAG_DISABLE_ANYHIT };
		this->_input.sphereArray.flags         = input_flags;
		this->_input.sphereArray.numSbtRecords = n_sbt;
	}

	template <>
	void SurfaceBuildInput<OptixPolygon>::_setTypeAndBuffer(
		uint32_t    n_vertices, 
		uint32_t    n_sbt, 
		CUdeviceptr vertex_buffer, 
		CUdeviceptr radius_buffer) {
		this->_input.type                        = OPTIX_BUILD_INPUT_TYPE_TRIANGLES;
		this->_input.triangleArray.numVertices   = n_vertices;
		this->_input.triangleArray.vertexBuffers = &vertex_buffer;

		uint32_t* input_flags;
		input_flags = new uint32_t[n_sbt]{ OPTIX_GEOMETRY_FLAG_DISABLE_ANYHIT };
		this->_input.triangleArray.flags         = input_flags;
		this->_input.triangleArray.numSbtRecords = n_sbt;
	}

	// Constructor
	template <typename T>
	SurfaceBuildInput<T>::SurfaceBuildInput(uint32_t n_surfaces, uint32_t n_sbt, T* surfaces) {
		// get vertex and radius data
		uint32_t n_data_points;
		float3* vertex;
		float* radius;
		this->_getVertexAndRadius(n_surfaces, surfaces, &vertex, &radius);
		n_data_points = n_surfaces * surfaces->DATA_POINT_PER_INSTANCE;

		// get SBT data
		uint32_t* mat_indices;
		mat_indices = this->_getSbtTable(n_surfaces, surfaces);

		CUdeviceptr d_vertex_buffer;
		CUdeviceptr d_radius_buffer;

		CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_vertex_buffer),
			sizeof(float3) * n_data_points));
		CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_vertex_buffer), vertex,
			sizeof(float3) * n_data_points, cudaMemcpyHostToDevice));
		delete[] vertex;
		
		if (surfaces->DATA_POINT_HAS_RADIUS) {
			CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_radius_buffer),
				sizeof(float) * n_data_points));
			CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_radius_buffer), radius,
				sizeof(float) * n_data_points, cudaMemcpyHostToDevice));
			delete[] radius;
		}
		
		// set the build input
		this->_setTypeAndBuffer(n_data_points, n_sbt, d_vertex_buffer, d_radius_buffer);
	}

	// Destructor
	template <typename T>
	SurfaceBuildInput<T>::~SurfaceBuildInput() {
		int x = 1;
	}

	template <>
	SurfaceBuildInput<OptixSphere>::~SurfaceBuildInput() {
		CUdeviceptr d_vertex_buffer;
		CUdeviceptr d_radius_buffer;
		d_vertex_buffer = *this->_input.sphereArray.vertexBuffers;
		d_radius_buffer = *this->_input.sphereArray.radiusBuffers;
		CUDA_CHECK(cudaFree((void*)d_vertex_buffer));
		CUDA_CHECK(cudaFree((void*)d_radius_buffer));
	}

	template <>
	SurfaceBuildInput<OptixPolygon>::~SurfaceBuildInput() {
		CUDA_CHECK(cudaFree((void*)*this->_input.triangleArray.vertexBuffers));
	}

	// Get build input
	template <typename T>
	OptixBuildInput SurfaceBuildInput<T>::getBuildInputPtr() {
		return this->_input;
	}

	// Explicitly instantiate
	template class SurfaceBuildInput<OptixSphere>;
	template class SurfaceBuildInput<OptixPolygon>;
	*/

}
