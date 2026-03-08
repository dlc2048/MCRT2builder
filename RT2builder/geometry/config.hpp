#pragma once

#include "mcutil/singleton/singleton.hpp"
#include "mcutil/parser/input.hpp"
#include "mcutil/parser/parser.hpp"

#include "mcutil/geometry/mesh_primitives.hpp"


namespace geo {


    constexpr int BOOLEAN_MAP_MAXIMUM_SIZE_BYTES = 10240000;

    
	class GlobalSettings : public Singleton<GlobalSettings> {
		friend class Singleton<GlobalSettings>;
	private:
		mesh::APPROXIMATION_TYPE _mesh_approximation_type;
		double _rt_stepsize;
		double _ge_tolerance;
		bool   _save_primitive;
		bool   _save_corefined;
		bool   _save_region;
	public:


		GlobalSettings();


		GlobalSettings(mcutil::ArgInput& args);


		mesh::APPROXIMATION_TYPE meshApproximationType() { 
			return this->_mesh_approximation_type;
		}


		double stepsize()       { return this->_rt_stepsize;    }
		double errorTolerance() { return this->_ge_tolerance;   }
		bool   savePrimitive()  { return this->_save_primitive; }
		bool   saveCorefined()  { return this->_save_corefined; }
		bool   saveRegion()     { return this->_save_region;    }


	};


}