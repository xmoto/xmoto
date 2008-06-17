/*=============================================================================
XMOTO

This file is part of XMOTO.

XMOTO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

XMOTO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XMOTO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
=============================================================================*/

#ifndef __VCOMMON_H__
#define __VCOMMON_H__

/* Load in configuration */
#include "BuildConfig.h"

/* Misc. nice-to-have includes */
#if defined(WIN32)
  #include <windows.h>
#endif

#include <stddef.h>

#ifdef HAVE_SDL_FRAMEWORK
  #include <SDL.h>
  #include <SDL_mixer.h>
  #include <SDL_thread.h>
  #include <SDL_mutex.h>
#else
  #define USE_RWOPS
  #include <SDL/SDL.h>
  #include <SDL/SDL_mixer.h>
  #include <SDL/SDL_thread.h>
  #include <SDL/SDL_mutex.h>
#endif

// Some places #define _T, which we want for a template parameter
#undef _T

#ifdef ENABLE_OPENGL
/* Pull in OpenGL headers */
//#define NO_SDL_GLEXT
//#include <SDL/SDL_opengl.h>
/* following scissored from SDL_opengl.h */
#define __glext_h_  /* Don't let gl.h include glext.h */
#ifdef HAVE_APPLE_OPENGL_FRAMEWORK
#include <OpenGL/gl.h>	/* Header File For The OpenGL Library */
#include <OpenGL/glu.h>	/* Header File For The GLU Library */
#elif defined(__MACOS__)
#include <gl.h>		/* Header File For The OpenGL Library */
#include <glu.h>	/* Header File For The GLU Library */
#else
#include <GL/gl.h>	/* Header File For The OpenGL Library */
#include <GL/glu.h>	/* Header File For The GLU Library */
#endif
#undef __glext_h_

#include "glext.h"

#endif

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <iostream>
#include <sstream>

#include <stdio.h>
#include <math.h>

#if !defined(WIN32) && !defined(__APPLE__) && !defined(__MACH__)
  #if defined(__FreeBSD__) || defined(__OpenBSD__)
    #include <sys/endian.h>
  #else
    #include <endian.h>
  #endif
#endif

#if !defined(XMOTO_LITTLE_ENDIAN) && !defined(XMOTO_BIG_ENDIAN)
  /* Assume little endian */
  #define XMOTO_LITTLE_ENDIAN 1
#endif

#if defined(WIN32)
  #include "ode/ode.h"
#else
  #include <ode/ode.h>
#endif

//used a comparator in sdt::map
struct ltstr {
  bool operator()(const char* s1, const char* s2) const {
    return strcmp(s1, s2) < 0;
  }
};

#endif
