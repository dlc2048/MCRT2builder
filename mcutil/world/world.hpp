#pragma once

#include <string>
#include <vector>

#include <cuda_runtime.h>

#include "parser/input.hpp"


namespace geo {

	/*
	Base triangle segment
	*/

	struct MarkedTriangle {
		float3   vertex[3];
		int      region_idx_backface;
		int      region_idx_frontface;
		float3   center() const;
		double3  d_center() const;
		float3   normal() const;
		float    area() const;
		double   d_area() const;
		double   d_volume() const;
	};


	class BremSplit {
	private:
		std::string _target;
	public:
		BremSplit() = default;
		BremSplit(mcutil::ArgInput& args);
		const std::string& target() const;
	};


	/**
	* @brief Generic region context for module - world link
	*/
	class RegionContext {
	protected:
		std::string _reg_name;
		int         _reg_idx;
	public:


		RegionContext(const std::string& name);


		const std::string& name() const { return this->_reg_name; }


		int index() const { return this->_reg_idx; }


		void setIndex(int idx) { this->_reg_idx = idx; }


	};


	/*
	Unformatted polygon world
	*/

	class World {
	protected:
		std::vector<std::string>    _region;   // region data
		std::vector<int>            _reg_brem_split;  // bremsstrahlung splitting target regions
		std::vector<MarkedTriangle> _mesh;     // mesh data
	public:


		World();


		World(const std::string& file_name);


		void write(const std::string& file_name);


		void readBremSplit(mcutil::Input& input);


		const std::vector<std::string>& region() const;


		const std::vector<int>& regBremSplit() const;


		const std::vector<MarkedTriangle>& mesh() const;


		/* geometrical feature extractors */
		double volume(int region_idx) const;       // volume of region
		double area(int region_idx) const;         // area of region
		double area(int reg1, int reg2) const;     // area of boundary between reg1-reg2 (for boundary crossing)


		void printRegionInfo() const;


		// module - world linker
		bool link(RegionContext& reg_context) const;


	};

}


namespace mcutil {


	template<>
	ArgumentCard InputCardFactory<geo::BremSplit>::_setCard();


}