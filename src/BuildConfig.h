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

#ifndef __BUILDCONFIG_H__
#define __BUILDCONFIG_H__

/*=============================================================================
Info
=============================================================================*/

#define BUILD_MAJORVERSION    0
#define BUILD_VERSION         3
#define BUILD_MINORVERSION    3
#define BUILD_EXTRAINFO       "test1"

/*=============================================================================
Build configuration
=============================================================================*/

/* Rendering engine configuration. xmoto currently supports two rendering engines
 * one is openGl based and the other one is based on SDL_gfx. Here we define
 * ENABLE_OPENGL , ENABLE_SDLGFX for easy use in 
 * the code.
 */
#ifndef USE_OPENGL
  #define USE_OPENGL 1
#endif
#if USE_OPENGL == 1
  #define ENABLE_OPENGL
#endif

#ifndef USE_SDLGFX
  #define USE_SDLGFX 0
#endif
#if USE_SDLGFX == 1
  #define ENABLE_SDLGFX
#endif

/* DO_NOT_LOAD_TEXTURES - Do not load texture files. This speeds up the init
   time enormously */
//#define DO_NOT_LOAD_TEXTURES

/* TRACK_MEMORY - Track memory usage using mmgr */
#if defined(_DEBUG) /* Per default only do this in debug builds. */
  #define TRACK_MEMORY
#endif

/* EMUL_800x600 - Let everything think that the resolution is 800x600 */
#define EMUL_800x600

/* HIDE_JOYSTICK_SUPPORT - Disable joystick config UI */
#define HIDE_JOYSTICK_SUPPORT

/* BREAK_ON_EXCEPTION - (Visual C++ debug-mode only) Will break program if an
   exception occurs */
//#define BREAK_ON_EXCEPTION

/* PROFILE_MAIN_LOOP - Will profile main loop (Currently only Win32) */
//#define PROFILE_MAIN_LOOP

/*=============================================================================
Misc, don't touch
=============================================================================*/
#if defined(XMOTO_EDITOR) && defined(EMUL_800x600)
  #undef EMUL_800x600 /* editor don't want that */
#endif

#ifndef ALLOW_ZOOMING
#define ALLOW_ZOOMING 0
#endif
#if ALLOW_ZOOMING == 1
  #define ENABLE_ZOOMING
#endif

#if HAVE_GETTEXT == 1
  #define USE_GETTEXT
#endif



#endif

