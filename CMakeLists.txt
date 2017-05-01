cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(xmoto)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)

include(CheckCXXCompilerFlag)
include(CPack)

# packaging
include(cpack/cpack.cmake)

add_subdirectory(src)

add_subdirectory(bin)
add_subdirectory(extra)
#add_subdirectory(po)
