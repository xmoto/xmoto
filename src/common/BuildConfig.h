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

#ifndef __BUILDCONFIG_H__
#define __BUILDCONFIG_H__

/*=============================================================================
Build configuration
=============================================================================*/

/* Rendering engine configuration. xmoto currently supports two rendering
 * engines
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

#ifndef ALLOW_DEV
#define ALLOW_DEV 0
#endif
#if ALLOW_DEV == 1
#define ENABLE_DEV
#endif

#if HAVE_GETTEXT == 1
#define USE_GETTEXT
#endif

#endif
