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
 * @file    RT2builder/config.cpp
 * @brief   RT2builder global configs
 * @author  CM Lee
 * @date    05/23/2023
 */


#include <sutil/CUDAOutputBuffer.h>
#include <sutil/Exception.h>
#include <sutil/sutil.h>

#include "geometry.hpp"


namespace mcutil {


    template <>
    ArgumentCard InputCardFactory<geo::GlobalSettings>::_setCard() {
        ArgumentCard arg_card("GEO_GLOBAL");

        arg_card.insertOptionLikeField("correction_method",
            "Correction method of the quadradtic curvature to mesh "
            "approximation.",
            {
                OptionEntry(
                    "quad_encircle",
                    static_cast<int>(mesh::APPROXIMATION_TYPE::QUAD_ENCIRCLE),
                    "Generated mesh is encircled by the quadratic curvature. "
                    "The mesh volume and surface area will be smaller than "
                    "mathematical equation"
                ),
                OptionEntry(
                    "mesh_encircle",
                    static_cast<int>(mesh::APPROXIMATION_TYPE::QUAD_ENCIRCLE),
                    "Quadratic curvature is encircled by the generated mesh. "
                    "The mesh volume and surface area will be larger than "
                    "mathematical equation"
                ),
                OptionEntry(
                    "area_conserve",
                    static_cast<int>(mesh::APPROXIMATION_TYPE::AREA_CONSERVE),
                    "Vertices are scaled to make the surface area of the "
                    "generated mesh to equal against the quadratic curvature."
                ),
                OptionEntry(
                    "volume_conserve",
                    static_cast<int>(mesh::APPROXIMATION_TYPE::VOLUME_CONSERVE),
                    "Vertices are scaled to make the volume of the "
                    "generated mesh to equal against the quadratic curvature."
                )
            },
            "quad_encircle"
        );

        arg_card.insert<double>("stepsize",
            "The step size epsilon for boundary crossing "
            "evaluator [cm].",
            { 1e-6 }, { 2e-7 }, { 1e+0 });

        arg_card.insert<double>("error_tolerance",
            "The geometrical error tolerance for boundary "
            "crossing evaluator. For each surface, if the "
            "area where the error occur is larger than "
            "this parameter compared to the total area, "
            "error is raised.",
            { 1e-3 }, { 0.e0 }, { 1.e0 });

        arg_card.insert<bool>("save_primitive", 
            "If true, all surface primitives are saved to "
            "object file format [off].",
            std::vector<bool>{ false });

        arg_card.insert<bool>("save_corefined", 
            "If true, all corefined surfaces are saved to "
            "object file format [off].",
            std::vector<bool>{ false });

        arg_card.insert<bool>("save_region",
            "If true, all region primitives are saved to "
            "object file format [off].",
            std::vector<bool>{ false });

        return arg_card;
    }

}


namespace geo {

    
    GlobalSettings::GlobalSettings() {
        this->_rt_stepsize    = 1e-6;
        this->_ge_tolerance   = 1e-3;
        this->_save_primitive = false;
        this->_save_corefined = false;
    }


    GlobalSettings::GlobalSettings(mcutil::ArgInput& args) {

        this->_mesh_approximation_type = static_cast<mesh::APPROXIMATION_TYPE>(args["correction_method"].cast<int>()[0]);

        this->_rt_stepsize    = args["stepsize"].cast<double>()[0];
        this->_ge_tolerance   = args["error_tolerance"].cast<double>()[0];
        this->_save_primitive = args["save_primitive"].cast<bool>()[0];
        this->_save_corefined = args["save_corefined"].cast<bool>()[0];
        this->_save_region    = args["save_region"].cast<bool>()[0];

        GlobalSettings& def = GlobalSettings::getInstance();
        def = *this;
    }


}