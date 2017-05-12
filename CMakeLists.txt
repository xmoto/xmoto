cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(xmoto C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

include(CPack)
include(CheckCXXCompilerFlag)
include(CheckPrototypeDefinition)
include(CheckSymbolExists)

# packaging
include(cpack/cpack.cmake)

add_subdirectory(src)

add_subdirectory(bin)
add_subdirectory(extra)
#add_subdirectory(po)
