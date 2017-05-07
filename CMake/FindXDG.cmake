# find XDG (Open Dynamics Engine) includes and library
#
# XDG_INCLUDE_DIR - where the directory containing the XDG headers can be
#                   found
# XDG_LIBRARY     - full path to the XDG library
# XDG_FOUND       - TRUE if XDG was found

IF (NOT XDG_FOUND)

  FIND_PATH(XDG_INCLUDE_DIR basedir.h
    /usr/include
    /usr/local/include
  )
  FIND_LIBRARY(XDG_LIBRARY
    NAMES xdg-basedir
    PATHS
    /usr/lib
    /usr/local/lib
  )

  IF(XDG_INCLUDE_DIR)
    MESSAGE(STATUS "Found xdg-basedir include dir: ${XDG_INCLUDE_DIR}")
  ELSE(XDG_INCLUDE_DIR)
    MESSAGE(STATUS "Could NOT find xdg-basedir headers.")
  ENDIF(XDG_INCLUDE_DIR)

  IF(XDG_LIBRARY)
    MESSAGE(STATUS "Found xdg-basedir library: ${XDG_LIBRARY}")
  ELSE(XDG_LIBRARY)
    MESSAGE(STATUS "Could NOT find xdg-basedir library.")
  ENDIF(XDG_LIBRARY)

  IF(XDG_INCLUDE_DIR AND XDG_LIBRARY)
     SET(XDG_FOUND TRUE CACHE STRING "Whether libxdg-basedir was found or not")
   ELSE(XDG_INCLUDE_DIR AND XDG_LIBRARY)
     SET(XDG_FOUND FALSE)
     IF(XDG_FIND_REQUIRED)
       MESSAGE(FATAL_ERROR "Could not find libxdg-basedir Please install libxdg-basedir")
     ENDIF(XDG_FIND_REQUIRED)
   ENDIF(XDG_INCLUDE_DIR AND XDG_LIBRARY)
ENDIF (NOT XDG_FOUND)
