/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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

/* Warnings we don't like */
#if defined(WIN32) && defined(_MSC_VER)
  #pragma warning(disable : 4244 4018 4101 4244 4267 4305)
#endif

/* Misc. nice-to-have includes */
#if defined(WIN32)
  #include <windows.h>
#endif

//#if defined(__APPLE__) && defined(__MACH__)
//  #include <OpenGL/gl.h>  /* Header File For The OpenGL Library */
//#else
//  #include <GL/gl.h>      /* Header File For The OpenGL Library */
//#endif

#include <stddef.h>

//#if defined(WIN32) && defined(_MSC_VER)
//  #define GL_GLEXT_PROTOTYPES 1
//  #include "glext.h"
//#else
//
//  /* This aren't elegant. It would be nicer to use glext.h, but I can't get
//     it to work :( */
//  #define GL_ARRAY_BUFFER_ARB 0x8892
//  #define GL_STATIC_DRAW_ARB 0x88E4
//  
//  typedef void (*PFNGLBINDBUFFERARBPROC)(GLenum target, GLuint buffer);
//  typedef void (*PFNGLDELETEBUFFERSARBPROC)(GLsizei n, const GLuint *buffers); 
//  typedef void (*PFNGLGENBUFFERSARBPROC)(GLsizei n, GLuint *buffers);
//
//  typedef size_t GLsizeiptrARB; /* is this always true? */
//
//  typedef void (*PFNGLBUFFERDATAARBPROC)(GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
//
//#endif

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
//#include <SDL/SDL_opengl.h>  /* can't use this, it's glext.h is simply too old */
/* Pull in OpenGL headers */
#if defined(WIN32)
  #include <windows.h>
#endif
#if defined(__APPLE__) && defined(__MACH__)
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
#else
  #define __glext_h_
  #include <GL/gl.h>
  #include <GL/glu.h>
  #undef __glext_h_
#endif
#include "glext.h"

#include <string>
#include <vector>
#include <ostream>
#include <istream>

#include <stdio.h>
#include <math.h>

#if !defined(WIN32) && !defined(__APPLE__) && !defined(__MACH__)
  #include <endian.h>
#endif

#if !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
  /* Assume little endian */
  #define LITTLE_ENDIAN
#endif

extern "C" {
  #if defined(WIN32) 
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
  #else
    #if defined(HAVE_LUA50_LUA_H)
      #include <lua50/lua.h>
    #elif defined(HAVE_LUA_LUA_H)
      #include <lua/lua.h>
    #elif defined(HAVE_LUA_H)
      #include <lua.h>
    #else
      #error Missing lua.h
    #endif

    #if defined(HAVE_LUA50_LAUXLIB_H)
      #include <lua50/lauxlib.h>
    #elif defined(HAVE_LUA_LAUXLIB_H)
      #include <lua/lauxlib.h>
    #elif defined(HAVE_LAUXLIB_H)
      #include <lauxlib.h>
    #else
      #error Missing lauxlib.h
    #endif
    
    #if defined(HAVE_LUA50_LUALIB_H)
      #include <lua50/lualib.h>
    #elif defined(HAVE_LUA_LUALIB_H)
      #include <lua/lualib.h>
    #elif defined(HAVE_LUALIB_H)
      #include <lualib.h>
    #else
      #error Missing lualib.h    
    #endif
  #endif
};

#if defined(WIN32)
  #include "ode/ode.h"
#else
  #include <ode/ode.h>
#endif

#if !defined(NOMMGR)
  /* Track memory? */
  #if defined(TRACK_MEMORY)
    #include "mmgr.h"
  #else 
    #include "nommgr.h"
  #endif
#endif

/*===========================================================================
Macros
===========================================================================*/
#define MAKE_COLOR(red,green,blue,alpha) \
	((vapp::Color)(alpha|(((vapp::Color)blue)<<8)|(((vapp::Color)green)<<16)|(((vapp::Color)red)<<24)))
#define GET_ALPHA(color) \
	((vapp::Color)(color&0xff))
#define GET_BLUE(color) \
	((vapp::Color)((color&0xff00)>>8))
#define GET_GREEN(color) \
	((vapp::Color)((color&0xff0000)>>16))
#define GET_RED(color) \
	((vapp::Color)((color&0xff000000)>>24))
#define GETF_ALPHA(color) \
	(((vapp::Color)(color&0xff))/255.0f)
#define GETF_BLUE(color) \
	(((vapp::Color)((color&0xff00)>>8))/255.0f)
#define GETF_GREEN(color) \
	(((vapp::Color)((color&0xff0000)>>16))/255.0f)
#define GETF_RED(color) \
	(((vapp::Color)((color&0xff000000)>>24))/255.0f)
#define INVERT_COLOR(color) \
	MAKE_COLOR(255-GET_RED(color),255-GET_GREEN(color),255-GET_BLUE(color),255)

namespace vapp {

  /*===========================================================================
  Types
  ===========================================================================*/
  typedef unsigned long Color;

};

#endif
