cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(chipmunk C)

file(GLOB chipmunk_source_files "*.c")

add_library(chipmunk STATIC
  ${chipmunk_source_files}
)
target_include_directories(chipmunk
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

