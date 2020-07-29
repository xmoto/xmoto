# Match the current Debian stable's CMake version.
cmake_minimum_required(VERSION 3.13.4 FATAL_ERROR) # Debian 10
project(xmoto C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

include(CheckCXXCompilerFlag)
include(CheckPrototypeDefinition)
include(CheckSymbolExists)

add_subdirectory(src)

# we set some cpack variables in src/CMakeLists.txt so include this after src/
include(cpack/cpack.cmake)
include(CPack)

add_subdirectory(bin)
add_subdirectory(doc)
add_subdirectory(extra)
#add_subdirectory(po)
