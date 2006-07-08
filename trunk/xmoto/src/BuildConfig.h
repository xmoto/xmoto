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
#define BUILD_VERSION         1
#define BUILD_MINORVERSION    17
#define BUILD_EXTRAINFO       "test1"

/*=============================================================================
Build configuration
=============================================================================*/

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

/* ENABLE_ZOOMING - Allow user to zoom in and out while playing */
//#define ENABLE_ZOOMING

/* ALLOW_GHOST - Allow user to see replay ghosts while playing */
/* (BUG: disabling this will make it impossible to compile) */
#define ALLOW_GHOST

/* DO_NOT_ALLOW_WEBACCESS - Don't compile with web-access support, 
   regardless of whether libcurl is available or not */
//#define DO_NOT_ALLOW_WEBACCESS

/*=============================================================================
Misc, don't touch
=============================================================================*/
#if defined(XMOTO_EDITOR) && defined(EMUL_800x600)
  #undef EMUL_800x600 /* editor don't want that */
#endif

#if defined(HAVE_LIBCURL) && !defined(DO_NOT_ALLOW_WEBACCESS)
  #define SUPPORT_WEBACCESS
#endif

#endif

