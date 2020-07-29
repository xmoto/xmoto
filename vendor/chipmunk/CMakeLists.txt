cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(chipmunk C)

add_library(chipmunk STATIC
  chipmunk.c chipmunk.h
  cpArbiter.c cpArbiter.h
  cpArray.c cpArray.h
  cpBB.c cpBB.h
  cpBody.c cpBody.h
  cpCollision.c cpCollision.h
  cpHashSet.c cpHashSet.h
  cpJoint.c cpJoint.h
  cpPolyShape.c cpPolyShape.h
  cpShape.c cpShape.h
  cpSpace.c cpSpace.h
  cpSpaceHash.c cpSpaceHash.h
  cpVect.c cpVect.h
  prime.h
)
target_include_directories(chipmunk PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)
