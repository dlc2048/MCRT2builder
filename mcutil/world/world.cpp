//
// Copyright (C) 2025 CM Lee, SJ Ye, Seoul Sational University
//
// Licensed to the Apache Software Foundation(ASF) under one
// or more contributor license agreements.See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// 	"License"); you may not use this file except in compliance
// 	with the License.You may obtain a copy of the License at
// 
// 	http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.See the License for the
// specific language governing permissionsand limitations
// under the License.

/**
 * @file    mcutil/world/world.cpp
 * @brief   RT2 world definitions
 * @author  CM Lee
 * @date    05/23/2023
 */


#include <cmath>

#include "fortran/fortran.hpp"

#include "world.hpp"


namespace mcutil {


    template <>
    ArgumentCard InputCardFactory<geo::BremSplit>::_setCard() {
        ArgumentCard arg_card("BREM_SPLIT");
        arg_card.insert<std::string>("target",
            "Name of the target region of bremsstrahlung splitting",
            1);
        return arg_card;
    }


}


namespace geo {


    /*
    Base triangle segment
    */

    float3 MarkedTriangle::center() const {
        float3 center;
        center.x = (this->vertex[0].x + this->vertex[1].x + this->vertex[2].x) / 3.f;
        center.y = (this->vertex[0].y + this->vertex[1].y + this->vertex[2].y) / 3.f;
        center.z = (this->vertex[0].z + this->vertex[1].z + this->vertex[2].z) / 3.f;
        return center;
    }


    double3 MarkedTriangle::d_center() const {
        double3 center;
        center.x = (
            (double)this->vertex[0].x + 
            (double)this->vertex[1].x + 
            (double)this->vertex[2].x
            ) / 3.e0;
        center.y = (
            (double)this->vertex[0].y +
            (double)this->vertex[1].y +
            (double)this->vertex[2].y
            ) / 3.e0;
        center.z = (
            (double)this->vertex[0].z +
            (double)this->vertex[1].z +
            (double)this->vertex[2].z
            ) / 3.e0;
        return center;
    }


    float3 MarkedTriangle::normal() const {
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
        norm = std::sqrt(nv.x * nv.x + nv.y * nv.y + nv.z * nv.z);
        nv.x /= norm;
        nv.y /= norm;
        nv.z /= norm;
        return nv;
    }


    float MarkedTriangle::area() const {
        float s, r[3];
        for (int i = 0; i < 3; ++i) {
            float3 v;
            v = {
                this->vertex[(i + 1) % 3].x - this->vertex[i % 3].x,
                this->vertex[(i + 1) % 3].y - this->vertex[i % 3].y,
                this->vertex[(i + 1) % 3].z - this->vertex[i % 3].z
            };
            r[i] = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        }
        s = (r[0] + r[1] + r[2]) * 0.5f;
        return std::sqrt(std::max(0.f, s * (s - r[0]) * (s - r[1]) * (s - r[2])));
    }


    double MarkedTriangle::d_area() const {
        double s, r[3];
        for (int i = 0; i < 3; ++i) {
            double3 v;
            v = {
                (double)this->vertex[(i + 1) % 3].x - (double)this->vertex[i % 3].x,
                (double)this->vertex[(i + 1) % 3].y - (double)this->vertex[i % 3].y,
                (double)this->vertex[(i + 1) % 3].z - (double)this->vertex[i % 3].z
            };
            r[i] = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        }
        s = (r[0] + r[1] + r[2]) * 5.e-1;
        return std::sqrt(std::max(0.e0, s * (s - r[0]) * (s - r[1]) * (s - r[2])));
    }


    double MarkedTriangle::d_volume() const {
        double v = 0.e0;
        v += (double)vertex[2].x * (double)vertex[1].y * (double)vertex[0].z;
        v += (double)vertex[1].x * (double)vertex[0].y * (double)vertex[2].z;
        v += (double)vertex[0].x * (double)vertex[2].y * (double)vertex[1].z;
        v -= (double)vertex[0].x * (double)vertex[1].y * (double)vertex[2].z;
        v -= (double)vertex[1].x * (double)vertex[2].y * (double)vertex[0].z;
        v -= (double)vertex[2].x * (double)vertex[0].y * (double)vertex[1].z;
        return std::abs(v) / 6.e0;
    }


    BremSplit::BremSplit(mcutil::ArgInput& args) {
        this->_target = args["target"].cast<std::string>()[0];
    }


    const std::string& BremSplit::target() const {
        return this->_target;
    }


    RegionContext::RegionContext(const std::string& name) 
        : _reg_name(name), _reg_idx(-1) {}


    World::World() {
        // Dummy triangle
        MarkedTriangle triangle;
        triangle.vertex[0] = { 0.f, 0.f, 1.f };
        triangle.vertex[1] = { 0.f, 1.f, 0.f };
        triangle.vertex[2] = { 1.f, 0.f, 0.f };
        triangle.region_idx_backface  = -1;
        triangle.region_idx_frontface = -1;
        this->_mesh.push_back(triangle);
    };


    World::World(const std::string& file_name) {
        mcutil::FortranIfstream file(file_name);
        int length = file.read<int>()[0];
        for (int i = 0; i < length; ++i) {
            std::vector<char> name = file.read<char>();
            this->_region.push_back(std::string(name.begin(), name.end()));
        }
        length = file.read<int>()[0];
        this->_mesh = file.read<MarkedTriangle>();
    }


    void World::write(const std::string& file_name) {
        mcutil::FortranOfstream file(file_name);
        int length = (int)_region.size();
        file.write(reinterpret_cast<unsigned char*>(&length), sizeof(int));
        for (const std::string& name : _region) {
            file.write(reinterpret_cast<const unsigned char*>(name.c_str()), 
                name.size());
        }
        length = (int)_mesh.size();
        file.write(reinterpret_cast<unsigned char*>(&length), sizeof(int));
        file.write(reinterpret_cast<unsigned char*>(&_mesh[0]),
            sizeof(geo::MarkedTriangle) * _mesh.size());
    }


    void World::readBremSplit(mcutil::Input& input) {
        std::deque<BremSplit> brem_split_cards = mcutil::InputCardFactory<BremSplit>::readAll(input);
        for (BremSplit& brem_split_card : brem_split_cards) {
            int i;
            for (i = 0; i < (int)this->_region.size(); ++i) {
                if (brem_split_card.target() == this->_region[i]) {
                    this->_reg_brem_split.push_back(i);
                    break;
                }
            }
            if (i == (int)this->_region.size())
                mclog::fatalNameNotExist(brem_split_card.target());
        }
    }


    const std::vector<std::string>& World::region() const {
        return this->_region;
    }


    const std::vector<int>& World::regBremSplit() const {
        return this->_reg_brem_split;
    }

    
    const std::vector<MarkedTriangle>& World::mesh() const {
        return this->_mesh;
    }


    double World::volume(int region_idx) const {
        double volume = 0.e0;
        for (const MarkedTriangle& facet : this->_mesh) {
            float3 center = facet.center();
            float3 normal = facet.normal();
            float orient = 
                center.x * normal.x + 
                center.y * normal.y + 
                center.z * normal.z;
            int reg_back  = facet.region_idx_backface;
            int reg_front = facet.region_idx_frontface;
            if (orient < 0) {  // swap
                int temp  = reg_back;
                reg_back  = reg_front;
                reg_front = temp;
            }
            if (reg_back == region_idx) {
                volume += facet.d_volume();
            }
            else if (reg_front == region_idx) {
                volume -= facet.d_volume();
            }
        }
        return volume;
    }


    double World::area(int region_idx) const {
        double area = 0.e0;
        for (const MarkedTriangle& facet : this->_mesh) {
            if (facet.region_idx_backface  == region_idx ||
                facet.region_idx_frontface == region_idx) {
                area += facet.d_area();
            }
        }
        return area;
    }


    double World::area(int reg1, int reg2) const {
        double area = 0.e0;
        for (const MarkedTriangle& facet : this->_mesh) {
            if (facet.region_idx_backface  == reg1 &&
                facet.region_idx_frontface == reg2) {
                area += facet.d_area();
            }
            else if (facet.region_idx_backface == reg2 &&
                     facet.region_idx_frontface == reg1) {
                area += facet.d_area();
            }
        }
        return area;
    }


    void World::printRegionInfo() const {
        mclog::print() << "*** Final region infomations ***";
        mclog::printVar("Number of the region", this->_region.size());
        mclog::print() << " <Region List> ";

        mclog::FormattedTable table({ 5, 16, 24, 24 }, 2);
        table << "Index" << "Name" << "Surface Area [cm^2]" << "Volume [cm^3]";
        mclog::print() << table.str();

        for (size_t i = 0; i < this->_region.size(); ++i) {
            double volume = this->volume(i);
            double area   = this->area(i);
            
            table.clear();
            table << i << this->region()[i] << area << volume;
            mclog::print() << table.str();
        }
    }


    bool World::link(RegionContext& reg_context) const {
        for (size_t i = 0; i < this->_region.size(); ++i) {
            if (reg_context.name() == this->_region[i]) {
                reg_context.setIndex((int)i);
                return true;
            }
        }
        return false;
    }


}
