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
 * @file    RT2viewer/prompt.cpp
 * @brief   RT2viewer command prompt
 * @author  CM Lee
 * @date    05/23/2023
 */


#include "prompt.hpp"


RenderSettings interpretCommandLine(int argc, char* argv[]) {
    RenderSettings settings;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h")
            printHelp(1);
        else if (arg == "--input" || arg == "-i") {
            if (i == argc - 1) {
                std::cout << "Option '" << argv[i] << "' needs following argument" << std::endl;
                printHelp(1);
            }
            settings.input = argv[++i];
        }
        else if (arg == "--resolution" || arg == "-r") {
            if (i + 2 >= argc) {
                std::cout << "Option '" << argv[i] << "' needs following two arguments" << std::endl;
                printHelp(1);
            }
            float res[2];
            for (int j = 0; j < 2; ++j) {
                std::stringstream ss(argv[++i]);
                ss >> res[j];
                if (ss.fail()) {
                    std::cout << "Option '" << argv[i] << "' needs numeric arguments" << std::endl;
                    printHelp(1);
                }
            }
            settings.resolution.x = res[0];
            settings.resolution.y = res[1];
        }
        else if (arg == "--eye" || arg == "-e") {
            if (i + 3 >= argc) {
                std::cout << "Option '" << argv[i] << "' needs following three arguments" << std::endl;
                printHelp(1);
            }
            float eye[3];
            for (int j = 0; j < 3; ++j) {
                std::stringstream ss(argv[++i]);
                ss >> eye[j];
                if (ss.fail()) {
                    std::cout << "Option '" << argv[i] << "' needs numeric arguments" << std::endl;
                    printHelp(1);
                }
            }
            settings.eye.x = eye[0];
            settings.eye.y = eye[1];
            settings.eye.z = eye[2];
        }
        else if (arg == "--look" || arg == "-l") {
            if (i + 3 >= argc) {
                std::cout << "Option '" << argv[i] << "' needs following three arguments" << std::endl;
                printHelp(1);
            }
            float look[3];
            for (int j = 0; j < 3; ++j) {
                std::stringstream ss(argv[++i]);
                ss >> look[j];
                if (ss.fail()) {
                    std::cout << "Option '" << argv[i] << "' needs numeric arguments" << std::endl;
                    printHelp(1);
                }
            }
            settings.look.x = look[0];
            settings.look.y = look[1];
            settings.look.z = look[2];
        }
        else if (arg == "--fov" || arg == "-f") {
            if (i == argc - 1) {
                std::cout << "Option '" << argv[i] << "' needs following argument" << std::endl;
                printHelp(1);
            }
            std::stringstream ss(argv[++i]);
            ss >> settings.fov;
            if (ss.fail()) {
                std::cout << "Option '" << argv[i] << "' needs numeric arguments" << std::endl;
                printHelp(1);
            }
        }
        else {
            std::cout << "Unknown option '" << argv[i] << "'" << std::endl;
            printHelp(1);
        }
    }

    return settings;
}


void printHelp(int err) {
    std::cout << "Parameters: --input      | -i <filename>     Geometry file                " << std::endl;
    std::cout << "            --resolution | -r <int2>         Renderer resolution          " << std::endl;
    std::cout << "            --eye        | -e <float3>       Camera eye position [cm]     " << std::endl;
    std::cout << "            --look       | -l <float3>       Camera look-at position [cm] " << std::endl;
    std::cout << "            --fov        | -f <float>        Camera field of view [degree]" << std::endl;
    std::cout << "            --help       | -h                Print this message           " << std::endl;
    exit(err);
}