#ifdef HAVE_SDL_FRAMEWORK
#include <SDL.h>
#include <SDL_mutex.h>
#include <SDL_thread.h>
#else
#include <SDL/SDL.h>
#include <SDL/SDL_mutex.h>
#include <SDL/SDL_thread.h>
#endif
