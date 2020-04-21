cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(xdgbasedir C)

add_library(xdgbasedir STATIC
  include/xdgbasedir/basedir.h
  include/xdgbasedir/basedir_fs.h

  src/basedir.c
)

set(MS_MKDIR $<BOOL:${MSYS}>)
target_compile_definitions(xdgbasedir PUBLIC MS_MKDIR=$<BOOL:${MS_MKDIR}>)

target_include_directories(xdgbasedir
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
