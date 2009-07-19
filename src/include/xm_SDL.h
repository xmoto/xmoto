#ifdef HAVE_SDL_FRAMEWORK
  #include <SDL.h>
  #include <SDL_thread.h>
  #include <SDL_mutex.h>
#else
  #include <SDL/SDL.h>
  #include <SDL/SDL_thread.h>
  #include <SDL/SDL_mutex.h>
#endif
