cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(libccd C)

add_library(libccd STATIC
  src/alloc.c
  src/ccd.c
  src/mpr.c
  src/polytope.c
  src/support.c
  src/vec3.c
)
target_include_directories(libccd
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
