# find SDL2_net includes and library
#
# SDL2_NET_INCLUDE_DIR - where the directory containing the SDL2_net headers can be
#                        found
# SDL2_NET_LIBRARY     - full path to the SDL2_net library
# SDL2_NET_FOUND       - TRUE if SDL2_net was found

IF (NOT SDL2_NET_FOUND)
  FIND_PATH(SDL2_NET_INCLUDE_DIR SDL2/SDL_net.h
    /usr/include
    /usr/local/include
  )
  FIND_LIBRARY(SDL2_NET_LIBRARY
    NAMES SDL2_net
    PATHS
    /usr/lib
    /usr/local/lib
  )

  IF(SDL2_NET_INCLUDE_DIR)
    MESSAGE(STATUS "Found SDL2_net include dir: ${SDL2_NET_INCLUDE_DIR}")
  ELSE(SDL2_NET_INCLUDE_DIR)
    MESSAGE(STATUS "Could NOT find SDL2_net headers.")
  ENDIF(SDL2_NET_INCLUDE_DIR)

  IF(SDL2_NET_LIBRARY)
    MESSAGE(STATUS "Found SDL2_net library: ${SDL2_NET_LIBRARY}")
  ELSE(SDL2_NET_LIBRARY)
    MESSAGE(STATUS "Could NOT find SDL2_net library.")
  ENDIF(SDL2_NET_LIBRARY)

  IF(SDL2_NET_INCLUDE_DIR AND SDL2_NET_LIBRARY)
    SET(SDL2_NET_FOUND TRUE CACHE STRING "Whether SDL2_net was found or not")
  ELSE(SDL2_NET_INCLUDE_DIR AND SDL2_NET_LIBRARY)
    SET(SDL2_NET_FOUND FALSE)
    IF(SDL2_NET_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find SDL2_net. Please install SDL2_net")
    ENDIF(SDL2_NET_FIND_REQUIRED)
  ENDIF(SDL2_NET_INCLUDE_DIR AND SDL2_NET_LIBRARY)
ENDIF (NOT SDL2_NET_FOUND)

