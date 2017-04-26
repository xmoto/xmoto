cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(xmoto)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)

include(CheckCXXCompilerFlag)
include(CPack)

# packaging
set(CPACK_PACKAGE_NAME "xmoto")
set(CPACK_GENERATOR TGZ)
set(CPACK_SOURCE_GENERATOR STGZ)
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 5)
set(CPACK_PACKAGE_VERSION_PATCH 3)
set(CPACK_PACKAGE_FILE_NAME "xmoto-" ${CPACK_PACKAGE_VERSION_MAJOR} "." ${CPACK_PACKAGE_VERSION_MINOR} "." ${CPACK_PACKAGE_VERSION_PATCH})

add_subdirectory(src)

add_subdirectory(bin)
add_subdirectory(extra)
#add_subdirectory(po)
#add_subdirectory(intl)
