
#include <sutil/CUDAOutputBuffer.h>
#include <sutil/sutil.h>
#include <sutil/Exception.h>

#include "device/memory.hpp"

#include "surface.hpp"


namespace mcutil {


	template <>
	ArgumentCard InputCardFactory<geo::RotDefi>::_setCard() {
		ArgumentCard arg_card("ROT_DEFI");

		arg_card.insert<char>("axis",
			"Axis of rotation. This parameter should be 'x', "
			"'y', or 'z'.",
			{ 'z' }, { 'x' }, { 'z' });

		arg_card.insert<double>("affine", 
			"3x4 Affine transform matrix. If this option is "
            "used, final transformation matrix is overwrittend "
            "by this parameter. The transformation matrix has  "
            "following structure;\n\n"
			"       | a_xx  a_xy  a_xz  a_xt | \n"
			"       | a_yx  a_yy  a_yz  a_yt | \n"
			"       | a_zx  a_zy  a_zz  a_zt | ",
			std::vector<double>
			(12, 0.0));

		arg_card.insert<std::string>("name",
			"The name of rotation definition. The name must "
			"be unique.",
			1);

		arg_card.insert<double>("theta", "Rotation azimuthal angle [degree].", std::vector{0.e0});

		arg_card.insert<double>("delta", "Tanslation vector [cm]", { 0.e0, 0.e0, 0.e0 });

		return arg_card;
	}


	template <>
	ArgumentCard InputCardFactory<geo::SurfaceBegin>::_setCard() {
		ArgumentCard arg_card("SURFACE_BEGIN");
		return arg_card;
	}


	template <>
	ArgumentCard InputCardFactory<geo::SurfaceEnd>::_setCard() {
		ArgumentCard arg_card("SURFACE_END");
		return arg_card;
	}


	template <>
	ArgumentCard InputCardFactory<geo::TransformBegin>::_setCard() {
		ArgumentCard arg_card("TRANSFORM_BEGIN");
		arg_card.insert<std::string>("target", "The name of target transformation definition.", 1);
		return arg_card;
	}


	template <>
	ArgumentCard InputCardFactory<geo::TransformEnd>::_setCard() {
		ArgumentCard arg_card("TRANSFORM_END");
		return arg_card;
	}


	template <>
	ArgumentCard InputCardFactory<geo::VoxelContainer>::_setCard() {
		ArgumentCard arg_card("VOXEL");

		arg_card.insert<std::string>("name", 
			"The name of surface primitive. The surface name "
			"must be unique.",
			1);

		arg_card.insert<std::string>("file", 
			"RT2 in-house voxel file name. Compressed voxel "
			"file can be generated from DICOM or NIFTI file "
			"by using RT2dicom or RT2nifti.",
			1);

		return arg_card;
	}


	template <>
	ArgumentCard InputCardFactory<geo::Revolution>::_setCard() {
		ArgumentCard arg_card("REVOLUTION");

		arg_card.insert<std::string>("name", 
			"The name of surface primitive. The surface name "
			"must be unique.",
			1);

		arg_card.insert<double>("entry",
			"The coordinate entry. Entry list should have form "
            "of {x1, z1, x2, z2,..., xn, zn}. The length of "
            "entry list must even. The axis of rotation is "
            "z-axis, and the start and end points of the entry "
            "must be on the z-axis. The lines connecting each "
            "point in order must not intersect each other and "
            "must be arranged in a clockwise order.");

		arg_card.insert<double>("center", 
			"The center coordinate of this solid [cm]. The "
            "'entry' coordinates are the relative position of "
            "this center variable.",
			3);

		arg_card.insert<double>("direction", 
			"The direction vector of this solid. Initial axis "
            "of rotation is transformed from z-axis to this "
            "variable.",
			{ 0.e0, 0.e0, 1.e0 });

		arg_card.insert<int>("vertices",
			"Polygon resolution of this solid.",
			{ geo::MESH_DEFAULT_VERTICES }, 
			{ geo::MESH_MINIMUM_VERTICES },
			{ geo::MESH_MAXIMUM_VERTICES }
		);

		return arg_card;
	}


	template<>
	ArgumentCard InputCardFactory<geo::UvSphere>::_setCard() {
		ArgumentCard arg_card("UVSPHERE");

		arg_card.insert<std::string>("name",
			"The name of surface primitive. The surface name "
            "must be unique.",
			1);

		arg_card.insert<double>("center",
			"The center coordinate of the UV sphere [cm].",
			3);

		arg_card.insert<double>("direction", 
			"The direction vector of the UV sphere. Initial "
            "axis of rotation is transformed from z-axis to "
            "this variable.",
			{ 0.e0, 0.e0, 1.e0 });

		arg_card.insert<double>("radius",
			"The radius of this sphere [cm].",
			{ geo::MESH_MINIMUM_SIZE },
			{ geo::MESH_MAXIMUM_SIZE }
		);

		arg_card.insert<int>("vertices",
			"Polygon resolution of this solid.",
			{ geo::MESH_DEFAULT_VERTICES, geo::MESH_DEFAULT_VERTICES },
			{ geo::MESH_MINIMUM_VERTICES, geo::MESH_MINIMUM_VERTICES },
			{ geo::MESH_MAXIMUM_VERTICES, geo::MESH_MAXIMUM_VERTICES }
		);

		return arg_card;
	}


	template <>
	ArgumentCard InputCardFactory<geo::Ellipsoid>::_setCard() {
		ArgumentCard arg_card("ELLIPSOID");

		arg_card.insert<std::string>("name", 
			"The name of surface primitive. The surface name "
            "must be unique.",
			1);

		arg_card.insert<double>("center",
			"The center coordinate of the ellipsoidal [cm].",
			3);

		arg_card.insert<double>("direction",
			"The direction vector of the ellipsoidal. Initial "
            "axis of rotation is transformed from z-axis to "
            "this variable.",
			{ 0.e0, 0.e0, 1.e0 });

		arg_card.insert<double>("radius",
			"The radius of this ellipsoid (x, y and z-axis) [cm].",
			{ geo::MESH_MINIMUM_SIZE, geo::MESH_MINIMUM_SIZE, geo::MESH_MINIMUM_SIZE },
			{ geo::MESH_MAXIMUM_SIZE, geo::MESH_MAXIMUM_SIZE, geo::MESH_MAXIMUM_SIZE }
		);

		arg_card.insert<int>("vertices",
			"Polygon resolution of this solid (UV respectively)",
			{ geo::MESH_DEFAULT_VERTICES, geo::MESH_DEFAULT_VERTICES },
			{ geo::MESH_MINIMUM_VERTICES, geo::MESH_MINIMUM_VERTICES },
			{ geo::MESH_MAXIMUM_VERTICES, geo::MESH_MAXIMUM_VERTICES }
		);

		return arg_card;
	}


	template <>
	ArgumentCard InputCardFactory<geo::Cylinder>::_setCard() {
		ArgumentCard arg_card("CYLINDER");

		arg_card.insert<std::string>("name", 
			"The name of surface primitive. The surface name "
            "must be unique.",
			1);

		arg_card.insert<double>("center",
			"The center coordinate of the base of a cylinder[cm].",
			3);

		arg_card.insert<double>("height", 
			"The height vector of the cylinder [cm]. Length "
			"of the entered vector should be 3.",
			3);

		arg_card.insert<double>("radius",
			"The radius of the cylinder [cm].",
			{ geo::MESH_MINIMUM_SIZE },
			{ geo::MESH_MAXIMUM_SIZE }
		);

		arg_card.insert<int>("vertices",
			"Polygon resolution of this solid.",
			{ geo::MESH_DEFAULT_VERTICES },
			{ geo::MESH_MINIMUM_VERTICES },
			{ geo::MESH_MAXIMUM_VERTICES }
		);

		return arg_card;
	}


	template <>
	ArgumentCard InputCardFactory<geo::Cone>::_setCard() {
		ArgumentCard arg_card("CONE");

		arg_card.insert<std::string>("name",
			"The name of surface primitive. The surface name "
            "must be unique.",
			1);

		arg_card.insert<double>("center", 
			"The center coordinate of the base of a cone [cm].",
			3);

		arg_card.insert<double>("height", 
			"The height vector of the cylinder [cm]. Length "
			"of the entered vector should be 3.",
			3);

		arg_card.insert<double>("radius",
			"The radius of the cone [cm].",
			{ geo::MESH_MINIMUM_SIZE },
			{ geo::MESH_MAXIMUM_SIZE }
		);

		arg_card.insert<int>("vertices",
			"Polygon resolution of this solid.",
			{ geo::MESH_DEFAULT_VERTICES },
			{ geo::MESH_MINIMUM_VERTICES },
			{ geo::MESH_MAXIMUM_VERTICES }
		);

		return arg_card;
	}


	template <>
	ArgumentCard InputCardFactory<geo::IcoSphere>::_setCard() {
		ArgumentCard arg_card("ICOSPHERE");

		arg_card.insert<std::string>("name", 
			"The name of surface primitive. The surface name "
			"must be unique.",
			1);

		arg_card.insert<double>("center",
			"The center coordinate of this ico-sphere [cm].",
			3);

		arg_card.insert<double>("radius",
			"The radius of the sphere [cm].",
			{ geo::MESH_MINIMUM_SIZE },
			{ geo::MESH_MAXIMUM_SIZE }
		);

		arg_card.insert<int>("order",
			"The order of ico-sphere tessellation algorithm. "
			"Higher order generate smoother mesh.",
			{ geo::MESH_ICOSPHERE_DEFAULT_ORDER },
			{ geo::MESH_ICOSPHERE_MINIMUM_ORDER },
			{ geo::MESH_ICOSPHERE_MAXIMUM_ORDER }
		);

		return arg_card;
	}


	template <>
	ArgumentCard InputCardFactory<geo::Cube>::_setCard() {
		ArgumentCard arg_card("CUBE");

		arg_card.insert<std::string>("name",
			"The name of surface primitive. The surface name "
			"must be unique.",
			1);

		arg_card.insert<double>("center", 
			"The center coordinate of this cube [cm].",
			3);

		arg_card.insert<double>("size",
			"The length of one side for each axis (xyz) [cm].",
			{ geo::MESH_MINIMUM_SIZE, geo::MESH_MINIMUM_SIZE, geo::MESH_MINIMUM_SIZE },
			{ geo::MESH_MAXIMUM_SIZE, geo::MESH_MAXIMUM_SIZE, geo::MESH_MAXIMUM_SIZE }
		);

		return arg_card;
	}


	template <>
	ArgumentCard InputCardFactory<geo::Toroid>::_setCard() {
		ArgumentCard arg_card("TOROID");

		arg_card.insert<std::string>("name", 
			"The name of surface primitive. The surface name "
            "must be unique.",
			1);

		arg_card.insert<double>("entry",
			"The coordinate entry. Entry list should have form "
            "of {x1, z1, x2, z2,..., xn, zn}. The length of "
            "entry list must even. The axis of rotation is "
            "z-axis, and the start and end points of the entry "
            "are connected. The lines connecting each point in "
            "order must not intersect each other and must be "
            "arranged in a clockwise order.");

		arg_card.insert<double>("center", 
			"The center coordinate of this solid [cm]. The "
            "'entry' coordinates are the relative position of "
            "this center variable.",
			3);

		arg_card.insert<double>("direction",
			"The direction vector of this solid. Initial axis "
            "of rotation is transformed from z-axis to this "
            "variable.",
			{ 0.e0, 0.e0, 1.e0 });

		arg_card.insert<int>("vertices",
			"Polygon resolution of this solid.",
			{ geo::MESH_DEFAULT_VERTICES },
			{ geo::MESH_MINIMUM_VERTICES },
			{ geo::MESH_MAXIMUM_VERTICES }
		);
		return arg_card;
	}


	template <>
	ArgumentCard InputCardFactory<geo::Torus>::_setCard() {
		ArgumentCard arg_card("TORUS");

		arg_card.insert<std::string>("name",
			"The name of surface primitive. The surface name "
			"must be unique.",
			1);

		arg_card.insert<double>("center", 
			"The center coordinate of co-planar circular axis"
            "[cm].",
			3);

		arg_card.insert<double>("direction", 
			"The direction vector of this solid. Initial axis "
            "of rotation is transformed from z-axis to this "
            "variable.",
			{ 0.0, 0.0, 1.0 });

		arg_card.insert<double>("radius",
			"The radius of co-planar circle (first) and "
            "revolving circle (second).",
			{ geo::MESH_MINIMUM_SIZE, geo::MESH_MINIMUM_SIZE },
			{ geo::MESH_MAXIMUM_SIZE, geo::MESH_MAXIMUM_SIZE }
		);
		arg_card.insert<int>("vertices",
			"Polygon resolution of this solid.",
			{ geo::MESH_DEFAULT_VERTICES, geo::MESH_DEFAULT_VERTICES },
			{ geo::MESH_MINIMUM_VERTICES, geo::MESH_MINIMUM_VERTICES },
			{ geo::MESH_MAXIMUM_VERTICES, geo::MESH_MAXIMUM_VERTICES }
		);
		return arg_card;
	}


	template<>
	ArgumentCard InputCardFactory<geo::Plate>::_setCard() {
		ArgumentCard arg_card("PLATE");
		
		arg_card.insert<double>("entry",
			"The coordinate entry. Entry list should have form "
            "of {x1, y1, x2, y2,..., xn, zn}. The length of "
            "entry list must even. The axis of rotation is "
            "z-axis, and the start and end points of the entry "
            "are connected. The lines connecting each point in "
            "order must not intersect each other and must be "
            "arranged in a clockwise order.");

		arg_card.insert<double>("height",
			"The height thickness vector of the plate [cm]. Length "
			"of the entered vector should be 3.",
			3);

		arg_card.insert<double>("center", 
			"The center coordinate of this solid [cm]. The "
            "'entry' coordinates are the relative position of "
            "this center variable.",
			3);

		return arg_card;
	}


	template <>
	ArgumentCard InputCardFactory<geo::Model>::_setCard() {
		ArgumentCard arg_card("MODEL");

		arg_card.insert<std::string>("name",
			"The name of surface primitive. The surface name "
            "must be unique.",
			1);

		arg_card.insert<std::string>("file", 
			"The name of 3D mesh file. Supported file "
            "extension are .off and .stl. Mesh data should not "
            "have self-intersecting facets and satisfy manifoldness.",
			1);

		arg_card.insert<double>("scale",
			"The origin mesh data is scaled by this factor. "
            "It requires three values, each a scaling factor "
            "for the x, y, and z axes.",
			{ 1e+0, 1e+0, 1e+0 }, 
			{ 1e-5, 1e-5, 1e-5 },
			{ 1e+5, 1e+5, 1e+5 }
		);

		return arg_card;
	}


}


namespace geo {


	/*
	Logical surface transform
	*/


	RotDefi::RotDefi(mcutil::ArgInput& args) :
		mcutil::Affine() {
		this->_name = args["name"].cast<std::string>()[0];
		char axis_char = args["axis"].cast<char>()[0];
		std::vector<double> affine = args["affine"].cast<double>();
		double theta  = args["theta"].cast<double>()[0];
		std::vector<double> delta = args["delta"].cast<double>();
		
		bool all_zero = true;
		for (size_t i = 0; i < 12; ++i) {
			if (affine[i] != 0.e0) {
				all_zero = false;
				break;
			}
		}

		if (all_zero) {  // Rotation & translation mode
			mcutil::AFFINE_AXIS axis;
			switch (axis_char) {
			case 'x':
				axis = mcutil::AFFINE_AXIS::X;
				break;
			case 'y':
				axis = mcutil::AFFINE_AXIS::Y;
				break;
			case 'z':
				axis = mcutil::AFFINE_AXIS::Z;
				break;
			default:
				break;
			}
			this->rotate(theta, axis);
			this->translate(delta[0], delta[1], delta[2]);
		}
		else {
			this->transform(mcutil::Affine(
				affine[0], affine[1], affine[2],  affine[3],
				affine[4], affine[5], affine[6],  affine[7],
				affine[8], affine[9], affine[10], affine[11]
			));
		}
	}


	std::string RotDefi::name() const {
		return this->_name;
	}


	void RotDefi::summary() const {
		mclog::printName(this->name());
		mclog::print() << "*** Affine matrix ***";
		mclog::print() << this->_tostr();
	}


	/*
	Optix traversable object, for the corefining
	*/


	OptixInitialScene::OptixInitialScene(
		const mesh::PolygonMesh& mesh_init,
		OptixDeviceContext       context
	) : _gas_handle(0x0), _d_gas_output_buffer(0x0) {
		std::vector<float3> vertices = mesh::extractVertices(mesh_init);

		OptixAccelBuildOptions accel_options = {};
		accel_options.buildFlags = OPTIX_BUILD_FLAG_NONE;
		accel_options.operation = OPTIX_BUILD_OPERATION_BUILD;

		const size_t vertices_size = sizeof(float3) * vertices.size();
		CUdeviceptr d_vertices = 0;
		CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_vertices), vertices_size));
		CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_vertices), vertices.data(),
			vertices_size, cudaMemcpyHostToDevice));

		const uint32_t flags[1] = { OPTIX_GEOMETRY_FLAG_NONE };
		OptixBuildInput input = {};
		input.type = OPTIX_BUILD_INPUT_TYPE_TRIANGLES;
		input.triangleArray.vertexFormat = OPTIX_VERTEX_FORMAT_FLOAT3;
		input.triangleArray.vertexStrideInBytes = sizeof(float3);
		input.triangleArray.numVertices = static_cast<uint32_t>(vertices.size());
		input.triangleArray.vertexBuffers = &d_vertices;
		input.triangleArray.flags = flags;
		input.triangleArray.numSbtRecords = 1;

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


	OptixInitialScene::~OptixInitialScene() {
		CUDA_CHECK(cudaFree(reinterpret_cast<void*>(this->_d_gas_output_buffer)));
	}


	OptixTraversableHandle OptixInitialScene::gas_handle() {
		return this->_gas_handle;
	}


	/*
	Surface series
	*/


	TransformBegin::TransformBegin(mcutil::ArgInput& args) {
		this->_target = args["target"].cast<std::string>()[0];
	}


	const std::string& TransformBegin::target() const {
		return this->_target;
	}


	/*
	Logical surface primitives
	*/


	Surface::Surface() :
		_name(""),
		mcutil::Affine() {};


	const std::string& Surface::name() const {
		return this->_name;
	}


	const mesh::PrimitiveMesh& Surface::mesh_init() const {
		return *this->_mesh_init;
	}


	size_t Surface::n_vertices() const {
		return this->_mesh_init->number_of_vertices();
	}


	void Surface::writePrimitive(const std::string& file_name) const {
		this->_mesh_init->write(file_name.c_str());
	}


	void Surface::transform(const mcutil::Affine& affine) {
		// transform mesh data
		this->_mesh_init->transform(affine);
		if (affine.determinant() < 0)  // face reverse condition
			this->_mesh_init->flip();
		// transform affine data
		Affine::transform(affine);
	}


	bool Surface::meshNotExist() const {
		return !_mesh_init;
	}


	bool Surface::checkSelfIntersect() const {
		return this->_mesh_init->does_self_intersect();
	}


	bool Surface::checkIsClosed() const {
		return this->_mesh_init->is_closed();
	}


	bool Surface::compressInitialScene(OptixDeviceContext context) {
		this->_scene_init = 
			std::make_shared<OptixInitialScene>(
			    this->_mesh_init->mesh(), context
			);
		return true;
	}


	OptixTraversableHandle Surface::getInitialSceneHandle() const {
		return this->_scene_init->gas_handle();
	}


	VoxelContainer::VoxelContainer(mcutil::ArgInput& args) {
		this->_name           = args["name"].cast<std::string>()[0];
		// this->_file_name = args["file"].data<std::string>()[0];
		std::string file_name = args["file"].cast<std::string>()[0];

		this->_voxel = std::make_shared<Voxel>(file_name);

		int3    shape    = this->_voxel->shape();
		this->_mesh_init = mesh::PrimitiveMeshFactory::voxel(
			(size_t)shape.x, (size_t)shape.y, (size_t)shape.z
		);
		Surface::transform(*this->_voxel);
	}


	void VoxelContainer::transform(const mcutil::Affine& affine) {
		Surface::transform(affine);
		this->_voxel->transform(affine);
	}


	/*
	void VoxelContainer::build(const std::deque<Hounsfield>& houns_bins) {
		this->_voxel = std::make_shared<Voxel>(this->_file_name, houns_bins);
		double3 center, size;

		double3 origin   = this->_voxel->origin();
		double3 bin_size = this->_voxel->size();
		int3    shape    = this->_voxel->shape();

		size.x = bin_size.x * (double)shape.x;
		size.y = bin_size.y * (double)shape.y;
		size.z = bin_size.z * (double)shape.z;

		center.x = origin.x + size.x * 0.5;
		center.y = origin.y + size.y * 0.5;
		center.z = origin.z + size.z * 0.5;

		this->_mesh_init = mesh::PrimitiveMeshFactory::voxel(
			size.x, size.y, size.z,
			(size_t)shape.x, (size_t)shape.y, (size_t)shape.z
		);
		this->_mesh_init->translate(center);
		this->_transformer[3] = mesh::translate(this->_transformer[3],
			mesh::Vector_3(origin.x, origin.y, origin.z));
	}
	*/


	std::shared_ptr<Voxel>& VoxelContainer::voxel() {
		return this->_voxel;
	}


	void VoxelContainer::summary() const {
		mclog::printName(this->name());
		std::stringstream ss;
		int3 shape = this->_voxel->shape();
		const std::vector<std::string>& reg 
			= this->_voxel->region();
		ss << shape.x << "," << shape.y << "," << shape.z;
		mclog::printVar("Shape", ss.str());
		mclog::printVar("Regions", reg.size());

		ss.str(""); ss.clear();
		for (const std::string& reg_str : reg)
			ss << " " << reg_str;
		mclog::print() << "*** Interior regions (namelist) ***";
		mclog::print() << ss;
		mclog::print() << "*** Affine matrix ***";
		mclog::print() << this->_tostr();
	}


	Revolution::Revolution(mcutil::ArgInput& args)  {
		this->_name     = args["name"].cast<std::string>()[0];
		this->_vertices = args["vertices"].cast<int>()[0];

		std::vector<double> center = args["center"].cast<double>();
		this->_center = { center[0], center[1], center[2] };

		std::vector<double> direction = args["direction"].cast<double>();
		this->_direction = { direction[0], direction[1], direction[2] };

		// read entry
		std::vector<double2> points;
		try {
			points = mcutil::cvtVectorDoubleToDouble2(args["entry"].cast<double>());
		}
		catch (std::length_error& e) {
			mclog::fatal() << "Fail to interpret 'entry' list. " << e.what();
		}
		// calculate direction normal vector
		std::pair<double, double2> norm_angle = vectorToNormAngle(this->_direction);
		double2 angle = norm_angle.second;
		if (norm_angle.first < DIRECTION_VECTOR_NORM_MINIMUM)
			mclog::fatal() << "Norm of direction vector is too small";

		// surface correction
		mesh::APPROXIMATION_TYPE approximation_type = GlobalSettings::getInstance().meshApproximationType();
		if (approximation_type == mesh::APPROXIMATION_TYPE::AREA_CONSERVE)
			approximation_type = mesh::APPROXIMATION_TYPE::VOLUME_CONSERVE;

		for (size_t i = 0; i < points.size(); ++i)
			points[i].x = mesh::calculatePolygonCircleRadius(points[i].x, norm_angle.first, this->_vertices, approximation_type);

		this->_mesh_init = mesh::PrimitiveMeshFactory::revolution(points, this->_vertices);

		// transform mesh
		mcutil::Affine affine;
		affine.rotate(angle.x, mcutil::AFFINE_AXIS::X);
		affine.rotate(angle.y, mcutil::AFFINE_AXIS::Y);
		affine.translate(this->_center.x, this->_center.y, this->_center.z);
		this->transform(affine);
	}


	UvSphere::UvSphere(mcutil::ArgInput& args) {
		std::vector<double> center    = args["center"].cast<double>();
		std::vector<double> direction = args["direction"].cast<double>();
		std::vector<int>    vertices  = args["vertices"].cast<int>();

		this->_name   = args["name"].cast<std::string>()[0];
		double radius = args["radius"].cast<double>()[0];
		
		this->_mesh_init = mesh::PrimitiveMeshFactory::sphereUV(
			radius, (size_t)vertices[0], (size_t)vertices[1]
		);
		double3 c = { center[0], center[1], center[2] };
		double3 d = { direction[0], direction[1], direction[2] };
		// calculate direction normal vector
		std::pair<double, double2> norm_angle = vectorToNormAngle(d);
		double2 angle = norm_angle.second;
		if (norm_angle.first < DIRECTION_VECTOR_NORM_MINIMUM)
			mclog::fatal() << "Norm of direction vector is too small";

		// surface correction
		mesh::correctPolygonSphereRadius(this->_mesh_init->meshPointer(), 
			radius, GlobalSettings::getInstance().meshApproximationType());

		// transform mesh
		mcutil::Affine affine;
		affine.rotate(angle.x, mcutil::AFFINE_AXIS::X);
		affine.rotate(angle.y, mcutil::AFFINE_AXIS::Y);
		affine.translate(c.x, c.y, c.z);
		this->transform(affine);
	}


	Ellipsoid::Ellipsoid(mcutil::ArgInput& args) {
		std::vector<double> center    = args["center"].cast<double>();
		std::vector<double> direction = args["direction"].cast<double>();
		std::vector<int>    vertices  = args["vertices"].cast<int>();
		std::vector<double> radius    = args["radius"].cast<double>();

		this->_name = args["name"].cast<std::string>()[0];

		double rm = std::min(std::min(radius[0], radius[1]), radius[2]);
		double3 radius_scaler = { radius[0] / rm, radius[1] / rm, radius[2] / rm };

		this->_mesh_init = mesh::PrimitiveMeshFactory::sphereUV(
			rm, (size_t)vertices[0], (size_t)vertices[1]
		);
		double3 c = { center[0], center[1], center[2] };
		double3 d = { direction[0], direction[1], direction[2] };
		// calculate direction normal vector
		std::pair<double, double2> norm_angle = vectorToNormAngle(d);
		double2 angle = norm_angle.second;
		if (norm_angle.first < DIRECTION_VECTOR_NORM_MINIMUM) {
			mclog::fatal() << "Norm of direction vector is too small";
		}

		// surface correction
		mesh::correctPolygonSphereRadius(this->_mesh_init->meshPointer(), 
			rm, GlobalSettings::getInstance().meshApproximationType());

		// transform mesh
		mcutil::Affine aff(
			radius_scaler.x, 0.0, 0.0, 0.0,
			0.0, radius_scaler.y, 0.0, 0.0,
			0.0, 0.0, radius_scaler.z, 0.0
		);
		aff.rotate(angle.x, mcutil::AFFINE_AXIS::X);
		aff.rotate(angle.y, mcutil::AFFINE_AXIS::Y);
		aff.translate(c.x, c.y, c.z);
		this->transform(aff);

	}


	Cylinder::Cylinder(mcutil::ArgInput& args) {
		std::vector<double> center = args["center"].cast<double>();
		std::vector<double> height = args["height"].cast<double>();

		int vertices  = args["vertices"].cast<int>()[0];
		this->_name   = args["name"].cast<std::string>()[0];
		double radius = args["radius"].cast<double>()[0];

		double3 h = { height[0], height[1], height[2] };
		double3 c = { center[0], center[1], center[2] };
		// calculate direction normal vector
		std::pair<double, double2> norm_angle = vectorToNormAngle(h);
		double  nh    = norm_angle.first;
		double2 angle = norm_angle.second;

		// surface correction
		radius = mesh::calculatePolygonCircleRadius(radius, nh, (size_t)vertices,
			GlobalSettings::getInstance().meshApproximationType());
		this->_mesh_init = mesh::PrimitiveMeshFactory::cylinder(radius, nh, (size_t)vertices);

		// transform mesh
		mcutil::Affine affine;
		affine.rotate(angle.x, mcutil::AFFINE_AXIS::X);
		affine.rotate(angle.y, mcutil::AFFINE_AXIS::Y);
		affine.translate(c.x, c.y, c.z);
		this->transform(affine);
	}


	Cone::Cone(mcutil::ArgInput& args) {
		std::vector<double> center = args["center"].cast<double>();
		std::vector<double> height = args["height"].cast<double>();

		int vertices  = args["vertices"].cast<int>()[0];
		this->_name   = args["name"].cast<std::string>()[0];
		double radius = args["radius"].cast<double>()[0];

		double3 h = { height[0], height[1], height[2] };
		double3 c = { center[0], center[1], center[2] };
		// calculate direction normal vector
		std::pair<double, double2> norm_angle = vectorToNormAngle(h);
		double  nh = norm_angle.first;
		double2 angle = norm_angle.second;

		// surface correction
		radius = mesh::calculatePolygonCircleRadius(radius, nh, (size_t)vertices,
			GlobalSettings::getInstance().meshApproximationType());
		this->_mesh_init = mesh::PrimitiveMeshFactory::cone(radius, nh, (size_t)vertices);

		// transform mesh
		mcutil::Affine affine;
		affine.rotate(angle.x, mcutil::AFFINE_AXIS::X);
		affine.rotate(angle.y, mcutil::AFFINE_AXIS::Y);
		affine.translate(c.x, c.y, c.z);
		this->transform(affine);
	}


	IcoSphere::IcoSphere(mcutil::ArgInput& args) {
		std::vector<double> center = args["center"].cast<double>();

		int order     = args["order"].cast<int>()[0];
		this->_name   = args["name"].cast<std::string>()[0];
		double radius = args["radius"].cast<double>()[0];

		double3 c = { center[0], center[1], center[2] };

		this->_mesh_init = mesh::PrimitiveMeshFactory::sphereIco(radius, (size_t)order);

		// surface correction
		mesh::correctPolygonSphereRadius(this->_mesh_init->meshPointer(), 
			radius, GlobalSettings::getInstance().meshApproximationType());

		// transform mesh
		mcutil::Affine affine;
		affine.translate(c.x, c.y, c.z);
		this->transform(affine);
	}


	Cube::Cube(mcutil::ArgInput& args) {
		std::vector<double> center = args["center"].cast<double>();
		std::vector<double> size   = args["size"].cast<double>();

		this->_name = args["name"].cast<std::string>()[0];
		
		double3 c = { center[0], center[1], center[2] };

		this->_mesh_init = mesh::PrimitiveMeshFactory::cube(size[0], size[1], size[2]);

		// transform mesh
		mcutil::Affine affine;
		affine.translate(c.x, c.y, c.z);
		this->transform(affine);
	}


	Toroid::Toroid(mcutil::ArgInput& args) {
		this->_name       = args["name"].cast<std::string>()[0];
		this->_vertices   = args["vertices"].cast<int>()[0];

		std::vector<double> center    = args["center"].cast<double>();
		std::vector<double> direction = args["direction"].cast<double>();
		this->_center    = { center[0], center[1], center[2] };
		this->_direction = { direction[0], direction[1], direction[2] };

		// read entry
		std::vector<double2> points;
		try {
			points = mcutil::cvtVectorDoubleToDouble2(args["entry"].cast<double>());
		}
		catch (std::length_error& e) {
			mclog::fatal() << "Fail to interpret 'entry' list. " << e.what();
		}
		// calculate direction normal vector
		std::pair<double, double2> norm_angle = vectorToNormAngle(this->_direction);
		double2 angle = norm_angle.second;
		if (norm_angle.first < DIRECTION_VECTOR_NORM_MINIMUM) {
			mclog::fatal() << "Norm of direction vector is too small";
		}

		this->_mesh_init = mesh::PrimitiveMeshFactory::toroid(points, this->_vertices);

		// transform mesh
		mcutil::Affine affine;
		affine.rotate(angle.x, mcutil::AFFINE_AXIS::X);
		affine.rotate(angle.y, mcutil::AFFINE_AXIS::Y);
		affine.translate(_center.x, _center.y, _center.z);
		this->transform(affine);
	}


	Torus::Torus(mcutil::ArgInput& args) {
		std::vector<double> center    = args["center"].cast<double>();
		std::vector<double> direction = args["direction"].cast<double>();
		std::vector<double> radius    = args["radius"].cast<double>();
		std::vector<int>    vertices  = args["vertices"].cast<int>();

		this->_name = args["name"].cast<std::string>()[0];

		this->_mesh_init = mesh::PrimitiveMeshFactory::torus(
			radius[0], radius[1], (size_t)vertices[0], (size_t)vertices[1]
		);
		double3 c = { center[0], center[1], center[2] };
		double3 d = { direction[0], direction[1], direction[2] };

		std::pair<double, double2> norm_angle = vectorToNormAngle(d);
		double2 angle = norm_angle.second;
		if (norm_angle.first < DIRECTION_VECTOR_NORM_MINIMUM) {
			mclog::fatal() << "Norm of direction vector is too small";
		}

		// transform mesh
		mcutil::Affine affine;
		affine.rotate(angle.x, mcutil::AFFINE_AXIS::X);
		affine.rotate(angle.y, mcutil::AFFINE_AXIS::Y);
		affine.translate(c.x, c.y, c.z);
		this->transform(affine);
	}


	Plate::Plate(mcutil::ArgInput& args) {
		this->_name = args["name"].cast<std::string>()[0];

		std::vector<double> center = args["center"].cast<double>();
		this->_center = { center[0], center[1], center[2] };

		std::vector<double> direction = args["height"].cast<double>();
		this->_direction = { direction[0], direction[1], direction[2] };

		// read entry
		std::vector<double2> points;
		try {
			points = mcutil::cvtVectorDoubleToDouble2(args["entry"].cast<double>());
		}
		catch (std::length_error& e) {
			mclog::fatal() << "Fail to interpret 'entry' list. " << e.what();
		}
		// calculate direction normal vector
		std::pair<double, double2> norm_angle = vectorToNormAngle(this->_direction);
		double  nh    = norm_angle.first;
		double2 angle = norm_angle.second;
		if (norm_angle.first < DIRECTION_VECTOR_NORM_MINIMUM)
			mclog::fatal() << "Norm of direction vector is too small";

		this->_mesh_init = mesh::PrimitiveMeshFactory::plate(points, nh);
		
		// transform mesh
		mcutil::Affine affine;
		affine.rotate(angle.x, mcutil::AFFINE_AXIS::X);
		affine.rotate(angle.y, mcutil::AFFINE_AXIS::Y);
		affine.translate(this->_center.x, this->_center.y, this->_center.z);
		this->transform(affine);
	}


	Model::Model(mcutil::ArgInput& args) {
		this->_name               = args["name"].cast<std::string>()[0];
		const std::string  file   = args["file"].cast<std::string>()[0];
		std::vector<double> scale = args["scale"].cast<double>();

		this->_mesh_init = mesh::PrimitiveMeshFactory::file(
			file.c_str()
		);

		// transform mesh
		mcutil::Affine aff(
			scale[0], 0.0, 0.0, 0.0,
			0.0, scale[1], 0.0, 0.0,
			0.0, 0.0, scale[2], 0.0
		);
		this->transform(aff);
	}

}