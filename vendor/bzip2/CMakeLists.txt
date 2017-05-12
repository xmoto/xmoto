cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(bzip2 C)

add_library(bzip2 STATIC
  bzlib.h
  bzlib_private.h

  blocksort.c
  bzlib.c
  compress.c
  crctable.c
  decompress.c
  huffman.c
  randtable.c
)
target_include_directories(bzip2
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)
