#pragma once

#include <utility>
#include <cuda_runtime.h>


namespace geo {


	constexpr double SMALL_COSINE_LIMIT            = 1e-20;
	constexpr double DIRECTION_VECTOR_NORM_MINIMUM = 1e-20;


	double degreeToRadian(double degree);


	double2 vectorToRotationAngle(double3 v);


	std::pair<double, double2> vectorToNormAngle(double3 v);

}