cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(glad C)

add_library(glad STATIC
  src/glad.c
)

target_include_directories(glad PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

target_link_libraries(glad PUBLIC
  "${CMAKE_DL_LIBS}"
)
