#ifdef HAVE_SDL_FRAMEWORK
  #include <SDL_mutex.h>
#else
  #define USE_RWOPS
  #include <SDL/SDL_mixer.h>
#endif
