#
# Copyright (C) 2025 CM Lee, SJ Ye, Seoul Sational University
#
# Licensed to the Apache Software Foundation(ASF) under one
# or more contributor license agreements.See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# 	"License"); you may not use this file except in compliance
# 	with the License.You may obtain a copy of the License at
# 
# 	http ://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.See the License for the
# specific language governing permissionsand limitations
# under the License.

# Packaging

set(CPACK_THREADS 4)
set(CPACK_PACKAGE_NAME    "MCRT2")
set(CPACK_PACKAGE_VENDOR  "SNU RPLAB")
set(CPACK_PACKAGE_VERSION ${MCRT2_VERSION})
set(CPACK_PACKAGE_CONTACT "dlc2048@snu.ac.kr")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MCRT2")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Ray-Tracing accelerated Radiation Transport Monte Carlo, RT2, geometry builder standalone")

if( WIN32 )
  set(CPACK_GENERATOR "WIX")
  
  set(CPACK_WIX_PRODUCT_GUID "6ba77041-f358-4482-b3ae-e1092502120d")
  set(CPACK_WIX_UPGRADE_GUID "6f9daf2f-5156-4f7e-b323-57c439c0103e")
  
  set(CPACK_WIX_EXTRA_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/rt2_cpack.wxs")
  set(CPACK_WIX_LICENSE_RTF   "${CMAKE_CURRENT_SOURCE_DIR}/license.rtf")
  
  # install dll
  install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/Release/ DESTINATION ${INSTALL_ROOT}bin FILES_MATCHING PATTERN "*.dll")
      
  # install optixIR & PTX files
  file(GLOB ptx_files "${CMAKE_BINARY_DIR}/lib/ptx/*.ptx")
  install(FILES     ${ptx_files} DESTINATION ${INSTALL_ROOT}lib/ptx)
  install(DIRECTORY ${CMAKE_BINARY_DIR}/lib/ptx/Release/ DESTINATION ${INSTALL_ROOT}lib/ptx FILES_MATCHING PATTERN "*.optixir")
  
  # install doc
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/docs DESTINATION ${INSTALL_ROOT}docs FILES_MATCHING PATTERN "*.pdf")
  
  # install license
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/license.rtf" DESTINATION ${INSTALL_ROOT}.) 
else( WIN32 )
  if( UNIX AND NOT APPLE )  # No Apple
    set(OS_RELEASE_FILE "/etc/os-release")
    if (EXISTS ${OS_RELEASE_FILE})
      file(READ ${OS_RELEASE_FILE} OS_RELEASE_CONTENTS)
      
      if(OS_RELEASE_CONTENTS MATCHES "ID=debian")
        set(IS_DEBIAN TRUE)
      elseif(OS_RELEASE_CONTENTS MATCHES "ID=ubuntu")
        set(IS_DEBIAN TRUE)
      else()  # FUTURE
        set(IS_DEBIAN FALSE)
      endif()
      
      if( IS_DEBIAN )
        set(CPACK_GENERATOR "DEB")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64) 
        set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
        
        # install shared lib
        file(GLOB so_files "${CMAKE_BINARY_DIR}/lib/*.so")
        install(FILES ${so_files} DESTINATION ${INSTALL_ROOT}lib)
      
        # install PTX files
        install(DIRECTORY ${CMAKE_BINARY_DIR}/lib/ptx DESTINATION ${INSTALL_ROOT}lib)
      
        # install doc
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/docs DESTINATION ${INSTALL_ROOT} FILES_MATCHING PATTERN "*.pdf")
      
        # install license
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/license.rtf" DESTINATION ${INSTALL_ROOT}.) 
        
        # symbolic link
        set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/packaging/debian/postinst")
      endif()
    endif()
  endif()
endif()

include(CPack)