if(APPLE)
  # Fixes a header/dylib version mismatch that can happen if Mono is installed:
  # https://gitlab.kitware.com/cmake/cmake/-/issues/18921#note_601123
  set(CMAKE_FIND_FRAMEWORK LAST)
endif()
