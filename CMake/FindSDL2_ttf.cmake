# find SDL2_ttf includes and library
#
# SDL2_TTF_INCLUDE_DIR - where the directory containing the SDL2_ttf headers can be
#                        found
# SDL2_TTF_LIBRARY     - full path to the SDL2_ttf library
# SDL2_TTF_FOUND       - TRUE if SDL2_ttf was found

IF (NOT SDL2_TTF_FOUND)
  FIND_PATH(SDL2_TTF_INCLUDE_DIR SDL2/SDL_ttf.h
    /usr/include
    /usr/local/include
  )
  FIND_LIBRARY(SDL2_TTF_LIBRARY
    NAMES SDL2_ttf
    PATHS
    /usr/lib
    /usr/local/lib
  )

  IF(SDL2_TTF_INCLUDE_DIR)
    MESSAGE(STATUS "Found SDL2_ttf include dir: ${SDL2_TTF_INCLUDE_DIR}")
  ELSE(SDL2_TTF_INCLUDE_DIR)
    MESSAGE(STATUS "Could NOT find SDL2_ttf headers.")
  ENDIF(SDL2_TTF_INCLUDE_DIR)

  IF(SDL2_TTF_LIBRARY)
    MESSAGE(STATUS "Found SDL2_ttf library: ${SDL2_TTF_LIBRARY}")
  ELSE(SDL2_TTF_LIBRARY)
    MESSAGE(STATUS "Could NOT find SDL2_ttf library.")
  ENDIF(SDL2_TTF_LIBRARY)

  IF(SDL2_TTF_INCLUDE_DIR AND SDL2_TTF_LIBRARY)
    SET(SDL2_TTF_FOUND TRUE CACHE STRING "Whether SDL2_ttf was found or not")
  ELSE(SDL2_TTF_INCLUDE_DIR AND SDL2_TTF_LIBRARY)
    SET(SDL2_TTF_FOUND FALSE)
    IF(SDL2_TTF_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find SDL2_ttf. Please install SDL2_ttf")
    ENDIF(SDL2_TTF_FIND_REQUIRED)
  ENDIF(SDL2_TTF_INCLUDE_DIR AND SDL2_TTF_LIBRARY)
ENDIF (NOT SDL2_TTF_FOUND)

