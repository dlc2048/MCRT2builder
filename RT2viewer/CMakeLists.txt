#
# Copyright (C) 2025 CM Lee, SJ Ye, Seoul Sational University
#
# Licensed to the Apache Software Foundation(ASF) under one
# or more contributor license agreements.See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# 	"License"); you may not use this file except in compliance
#	with the License.You may obtain a copy of the License at
# 
# 	http ://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.See the License for the
# specific language governing permissionsand limitations
# under the License.

list(APPEND source_subdir "geometry"          )
list(APPEND source_subdir "prompt"            )
list(APPEND source_subdir "ptx"               )

include_directories( "${CMAKE_SOURCE_DIR}/mcutil" )
include_directories( "${CMAKE_SOURCE_DIR}/module" )

set(source)

foreach(folder ${source_subdir})
  file(GLOB sub_sources "${folder}/*.h" "${folder}/*.hpp" "${folder}/*.cpp" "${folder}/*.cu")
  set(source ${source} ${sub_sources})
endforeach()

OPTIX_add_sample_executable( RT2viewer target_name
  ${source}
  viewer.cpp
  OPTIONS -rdc=false
)

target_link_libraries( ${target_name}
  ${CUDA_LIBRARIES}
  world
)

install(TARGETS ${target_name} DESTINATION ${INSTALL_ROOT}bin)
