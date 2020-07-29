target_sources_relative(xmoto PRIVATE
  ./DrawLib.cpp ./DrawLib.h
)
if(USE_OPENGL)
  target_sources_relative(xmoto PRIVATE
    ./DrawLibOpenGL.cpp ./DrawLibOpenGL.h
  )
endif()
if(USE_SDL_GFX)
  target_sources_relative(xmoto PRIVATE
    ./DrawLibSDLgfx.cpp ./DrawLibSDLgfx.h
  )
endif()
