cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_FLAGS "-std=c++11 -g -O0")

project(test)
add_executable(test test.cpp)
target_link_libraries(test va va-drm)

project(pylibva)

if(DEFINED ENV{PYTHON_INCLUDE} AND DEFINED ENV{PYBIND11_INCLUDE})
    message(STATUS "INFO: PYTHON_INCLUDE=" $ENV{PYTHON_INCLUDE})
    message(STATUS "INFO: PYBIND11_INCLUDE=" $ENV{PYBIND11_INCLUDE})
else()
    message(FATAL_ERROR "####ERROR: PYTHON_INCLUDE & PYBIND11_INCLUDE is not set, run \"python3 -m pybind11 --includes\" to get path and set the two env variables")
endif()

include_directories (pylibva $ENV{PYTHON_INCLUDE} $ENV{PYBIND11_INCLUDE})
add_library(pylibva SHARED pylibva.cpp)
target_link_libraries(pylibva va va-drm)
set_target_properties(pylibva PROPERTIES PREFIX "")