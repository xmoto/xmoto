# Distributed under the OSI-approved BSD 3-Clause License.  See
# https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindSDL_gfx
-----------

Locate SDL_gfx library

This module defines:

::

  SDL_GFX_LIBRARIES, the name of the library to link against
  SDL_GFX_INCLUDE_DIRS, where to find the headers
  SDL_GFX_FOUND, if false, do not try to link against
  SDL_GFX_VERSION_STRING - human-readable string containing the version of SDL_gfx



$SDLDIR is an environment variable that would correspond to the
./configure --prefix=$SDLDIR used in building SDL.

Created by Eric Wing.  Modified by Dusk Banks.  This was heavily based on
the FindSDL_net.cmake module, but with modifications for SDL_gfx instead.
#]=======================================================================]

find_path(SDL_GFX_INCLUDE_DIR SDL_gfxPrimitives.h
  HINTS
    ENV SDLGFXDIR
    ENV SDLDIR
  PATH_SUFFIXES SDL
                # path suffixes to search inside ENV{SDLDIR}
                include/SDL include/SDL12 include/SDL11 include
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VC_LIB_PATH_SUFFIX lib/x64)
else()
  set(VC_LIB_PATH_SUFFIX lib/x86)
endif()

find_library(SDL_GFX_LIBRARY
  NAMES SDL_gfx
  HINTS
    ENV SDLGFXDIR
    ENV SDLDIR
  PATH_SUFFIXES lib ${VC_LIB_PATH_SUFFIX}
)

if(SDL_GFX_INCLUDE_DIR AND EXISTS "${SDL_GFX_INCLUDE_DIR}/SDL_gfxPrimitives.h")
  file(STRINGS "${SDL_GFX_INCLUDE_DIR}/SDL_gfxPrimitives.h" SDL_GFX_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_GFXPRIMITIVES_MAJOR[ \t]+[0-9]+$")
  file(STRINGS "${SDL_GFX_INCLUDE_DIR}/SDL_gfxPrimitives.h" SDL_GFX_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_GFXPRIMITIVES_MINOR[ \t]+[0-9]+$")
  file(STRINGS "${SDL_GFX_INCLUDE_DIR}/SDL_gfxPrimitives.h" SDL_GFX_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_GFXPRIMITIVES_MICRO[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_GFXPRIMITIVES_MAJOR[ \t]+([0-9]+)$" "\\1" SDL_GFX_VERSION_MAJOR "${SDL_GFX_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_GFXPRIMITIVES_MINOR[ \t]+([0-9]+)$" "\\1" SDL_GFX_VERSION_MINOR "${SDL_GFX_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_GFXPRIMITIVES_MICRO[ \t]+([0-9]+)$" "\\1" SDL_GFX_VERSION_PATCH "${SDL_GFX_VERSION_PATCH_LINE}")
  set(SDL_GFX_VERSION_STRING ${SDL_GFX_VERSION_MAJOR}.${SDL_GFX_VERSION_MINOR}.${SDL_GFX_VERSION_PATCH})
  unset(SDL_GFX_VERSION_MAJOR_LINE)
  unset(SDL_GFX_VERSION_MINOR_LINE)
  unset(SDL_GFX_VERSION_PATCH_LINE)
  unset(SDL_GFX_VERSION_MAJOR)
  unset(SDL_GFX_VERSION_MINOR)
  unset(SDL_GFX_VERSION_PATCH)
endif()

set(SDL_GFX_LIBRARIES ${SDL_GFX_LIBRARY})
set(SDL_GFX_INCLUDE_DIRS ${SDL_GFX_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL_gfx
                                  REQUIRED_VARS SDL_GFX_LIBRARIES SDL_GFX_INCLUDE_DIRS
                                  VERSION_VAR SDL_GFX_VERSION_STRING)

mark_as_advanced(SDL_GFX_LIBRARY SDL_GFX_INCLUDE_DIR)

if(SDL_GFX_FOUND AND NOT TARGET SDL::SDL_gfx)
  add_library(SDL::SDL_gfx UNKNOWN IMPORTED)
  set_target_properties(SDL::SDL_gfx PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${SDL_GFX_INCLUDE_DIR}"
    IMPORTED_LOCATION "${SDL_GFX_LIBRARY}"
    VERSION "${SDL_GFX_VERSION_STRING}"
  )
endif()
