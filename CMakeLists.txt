cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(xmoto C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

set(APP_NAME "X-Moto")
set(APP_COPYRIGHT_YEAR "2023")

set(APP_VERSION_MAJOR 0)
set(APP_VERSION_MINOR 6)
set(APP_VERSION_PATCH 2)

set(APP_VERSION_STRING "\
${APP_VERSION_MAJOR}.\
${APP_VERSION_MINOR}.\
${APP_VERSION_PATCH}")

include(CheckCXXCompilerFlag)
include(CheckPrototypeDefinition)
include(CheckSymbolExists)
include(TestBigEndian)

include(CMake/macos.cmake)
include(CMake/mxe.cmake)

add_subdirectory(src)

# we set some cpack variables in src/CMakeLists.txt so include this after src/
include(cpack/cpack.cmake)
include(CPack)

add_subdirectory(bin)
add_subdirectory(doc)
add_subdirectory(extra)
#add_subdirectory(po)
