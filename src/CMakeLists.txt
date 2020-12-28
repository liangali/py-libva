cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_FLAGS "-std=c++11 -g -O0")

project(test)
add_executable(test test.cpp)
target_link_libraries(test va va-drm)

project(pylibva)

include_directories (pylibva /usr/local/lib/python3.6/dist-packages/pybind11/include /usr/include/python3.6m)
add_library(pylibva SHARED pylibva.cpp)
target_link_libraries(pylibva va va-drm)
set_target_properties(pylibva PROPERTIES PREFIX "")