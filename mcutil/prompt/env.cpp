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
 * @file    mcutil/prompt/env.cpp
 * @brief   Environment variable (MCRT2_HOME)
 * @author  CM Lee
 * @date    05/23/2023
 */


#include <stdexcept>
#include <stdlib.h>
#include <filesystem>

#include "env.hpp"


namespace mcutil {


    std::string getMCRT2HomePath() {
        const char* env = getenv("MCRT2_HOME");
        std::string path("");
        static const std::filesystem::path fp_default
#ifdef _WIN32
            = std::filesystem::path{ "C:/Program Files/MCRT2/" };
#elif defined(__unix__) || defined(__unix) || defined(unix)
            = std::filesystem::path{ "/usr/local/MCRT2/" };
#else
            = std::filesystem::path{ "" };
#endif
        if (env) {
            path = std::string(env);
        }
        else {  // Default
            path = fp_default.string();
        }
        return path;
    }

}