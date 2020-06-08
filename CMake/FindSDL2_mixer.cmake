# find SDL2_mixer includes and library
#
# SDL2_MIXER_INCLUDE_DIR - where the directory containing the SDL2_mixer headers can be
#                          found
# SDL2_MIXER_LIBRARY     - full path to the SDL2_mixer library
# SDL2_MIXER_FOUND       - TRUE if SDL2_mixer was found

IF (NOT SDL2_MIXER_FOUND)
  FIND_PATH(SDL2_MIXER_INCLUDE_DIR SDL2/SDL_mixer.h
    /usr/include
    /usr/local/include
  )
  FIND_LIBRARY(SDL2_MIXER_LIBRARY
    NAMES SDL2_mixer
    PATHS
    /usr/lib
    /usr/local/lib
  )

  IF(SDL2_MIXER_INCLUDE_DIR)
    MESSAGE(STATUS "Found SDL2_mixer include dir: ${SDL2_MIXER_INCLUDE_DIR}")
  ELSE(SDL2_MIXER_INCLUDE_DIR)
    MESSAGE(STATUS "Could NOT find SDL2_mixer headers.")
  ENDIF(SDL2_MIXER_INCLUDE_DIR)

  IF(SDL2_MIXER_LIBRARY)
    MESSAGE(STATUS "Found SDL2_mixer library: ${SDL2_MIXER_LIBRARY}")
  ELSE(SDL2_MIXER_LIBRARY)
    MESSAGE(STATUS "Could NOT find SDL2_mixer library.")
  ENDIF(SDL2_MIXER_LIBRARY)

  IF(SDL2_MIXER_INCLUDE_DIR AND SDL2_MIXER_LIBRARY)
    SET(SDL2_MIXER_FOUND TRUE CACHE STRING "Whether SDL2_mixer was found or not")
  ELSE(SDL2_MIXER_INCLUDE_DIR AND SDL2_MIXER_LIBRARY)
    SET(SDL2_MIXER_FOUND FALSE)
    IF(SDL2_MIXER_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find SDL2_mixer. Please install SDL2_mixer")
    ENDIF(SDL2_MIXER_FIND_REQUIRED)
  ENDIF(SDL2_MIXER_INCLUDE_DIR AND SDL2_MIXER_LIBRARY)
ENDIF (NOT SDL2_MIXER_FOUND)

