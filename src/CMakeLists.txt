cmake_minimum_required(VERSION 3.5)

project(test)
add_executable(test test.cpp)
target_link_libraries(test va va-drm)

project(pylibva)

include_directories (pylibva /usr/local/lib/python3.8/dist-packages/pybind11/include /usr/include/python3.8)
add_library(pylibva SHARED pylibva.cpp)
set_target_properties(pylibva PROPERTIES PREFIX "")