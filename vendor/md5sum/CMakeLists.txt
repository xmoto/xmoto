cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(md5sum C CXX)

add_library(md5sum STATIC
  include/md5sum/md5.h
  include/md5sum/md5file.h

  src/md5.c
  src/md5file.cpp
)
target_include_directories(md5sum
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
