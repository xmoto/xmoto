#[=======================================================================[.rst:
FindXDG
-----------

Locate xdgbasedir library

This module defines:

::

  XDG_LIBRARIES, the name of the library to link against
  XDG_INCLUDE_DIRS, where to find the headers
  XDG_FOUND, if false, do not try to link against



#]=======================================================================]

find_path(XDG_INCLUDE_DIR basedir.h)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VC_LIB_PATH_SUFFIX lib/x64)
else()
  set(VC_LIB_PATH_SUFFIX lib/x86)
endif()

find_library(XDG_LIBRARY
  NAMES xdg-basedir
  PATH_SUFFIXES lib ${VC_LIB_PATH_SUFFIX}
)

set(XDG_LIBRARIES ${XDG_LIBRARY})
set(XDG_INCLUDE_DIRS ${XDG_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(XDG
  REQUIRED_VARS XDG_LIBRARIES XDG_INCLUDE_DIRS)

mark_as_advanced(XDG_LIBRARY XDG_INCLUDE_DIR)

if(XDG_FOUND AND NOT TARGET XDG::Basedir)
  add_library(XDG::Basedir UNKNOWN IMPORTED)
  set_target_properties(XDG::Basedir PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${XDG_INCLUDE_DIR}"
    IMPORTED_LOCATION "${XDG_LIBRARY}"
  )
endif()
