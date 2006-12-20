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

/* 
 *  Simple 2D drawing library, built closely on top of OpenGL.
 */
#include "VDraw.h"
#include "BuiltInFont.h"

#ifdef ENABLE_OPENGL
namespace vapp {

 DrawLib::DrawLib() {
  m_nDispWidth=800;
  m_nDispHeight=600;
  m_nDispBPP=32;
  m_bWindowed=true;
  m_bNoGraphics=false;
  m_bDontUseGLExtensions=false;
  m_bShadersSupported = false;
  m_nLScissorX = m_nLScissorY = m_nLScissorW = m_nLScissorH = 0;
  m_bFBOSupported = false;
  m_pDefaultFontTex = NULL;
  m_texture = NULL;
  m_blendMode = BLEND_MODE_NONE;
 };

 DrawLib::~DrawLib() {
 }

}
#endif
