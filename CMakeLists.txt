cmake_minimum_required (VERSION 3.14...3.18)

# Workaround to allow us to support the new VERSION syntax introduced > 3.12
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project (rankcpp-experiments VERSION 1.0
                 DESCRIPTION "key rank experiments"
                 LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(FetchContent)

# common cmake
include(cmake/StdProjectSettings.cmake)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(app_options INTERFACE)
target_compile_features(app_options INTERFACE cxx_std_17)

# Clang: option to enable -ftime-trace
if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE
    "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    add_compile_definitions(app_options INTERFACE -ftime-trace)
  endif()
endif()

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(app_warnings INTERFACE)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(app_warnings)

# Application binaries
add_subdirectory("src")
