
#include <cmath>

#include "logic.hpp"


namespace geo {

	double degreeToRadian(double degree) {
		return M_PI / 180.0 * degree;
	}


	double2 vectorToRotationAngle(double3 v) {
		double theta, phi;
		double sint, cost, cosp;

		sint = -v.y;
		cost = sqrt(1.e0 - sint * sint);

		if (cost < SMALL_COSINE_LIMIT) {
			theta = (sint > 0) ? 90.0 : 270.0;
			phi = 0.0;
		}
		else {
			if (v.x < 0) 
				cost = -cost;
			cosp = v.z / cost;
			cosp = std::max(-1.0, std::min(cosp, 1.0));

			theta = acos(cost) * 180.0 / M_PI;
			phi = acos(cosp) * 180.0 / M_PI;
			if (sint < 0)
				theta = 360.0 - theta;
		}
		
		return make_double2(theta, phi);
	}


	std::pair<double, double2> vectorToNormAngle(double3 v) {
		double norm = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		double3 nv;
		if (norm < DIRECTION_VECTOR_NORM_MINIMUM)
			nv = { 0.0, 0.0, 1.0 };
		else
			nv = { v.x / norm, v.y / norm, v.z / norm };
		return { norm, vectorToRotationAngle(nv) };
	}

}