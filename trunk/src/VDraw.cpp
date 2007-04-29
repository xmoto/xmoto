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

namespace vapp {

  DrawLib::backendtype DrawLib::m_backend = DrawLib::backend_None;

  DrawLib* DrawLib::DrawLibFromName(std::string i_drawLibName) {
#ifdef ENABLE_OPENGL
    if (i_drawLibName == "OPENGL"){
      m_backend = backend_OpenGl;
      return new DrawLibOpenGL();
    }
#endif
#ifdef ENABLE_SDLGFX
    if (i_drawLibName == "SDLGFX"){
      m_backend = backend_SdlGFX;
      return new DrawLibSDLgfx();
    }
#endif

    /* if no name is given, try to force one renderer */
#ifdef ENABLE_OPENGL
    m_backend = backend_OpenGl;
    return new DrawLibOpenGL();
#endif
#ifdef ENABLE_SDLGFX
    m_backend = backend_SdlGFX;
    return new DrawLibSDLgfx();
#endif

    m_backend = backend_None;
    return NULL;
  }

 DrawLib::DrawLib() {
  m_nDispWidth=800;
  m_nDispHeight=600;
  m_nDispBPP=32;
  m_bWindowed=true;
  m_bNoGraphics=false;
  m_bDontUseGLExtensions=false;
  m_bShadersSupported = false;
  m_bVBOSupported = false;
  m_nLScissorX = m_nLScissorY = m_nLScissorW = m_nLScissorH = 0;
  m_bFBOSupported = false;
  m_texture = NULL;
  m_blendMode = BLEND_MODE_NONE;

  m_fontSmall  = NULL;
  m_fontMedium = NULL;
 };

 DrawLib::~DrawLib() {
 }

  int FontManager::nbGlyphsInMemory() {
    return 0;
  }

  FontManager* DrawLib::getFontManager(const std::string &i_fontFile, int i_fontSize) {
    throw Exception("Your DrawLib doesn't manage FontManager");
  }

  FontManager* DrawLib::getFontSmall() {
    if(m_fontSmall == NULL) {
      throw Exception("Invalid font");
    }
    return m_fontSmall;
  }

  FontManager* DrawLib::getFontMedium() {
    if(m_fontMedium == NULL) {
      throw Exception("Invalid font");
    }
    return m_fontMedium;
  }

  FontManager* DrawLib::getFontBig() {
    if(m_fontBig == NULL) {
      throw Exception("Invalid font");
    }
    return m_fontBig;
  }

   /*===========================================================================
  Primitive: box
  ===========================================================================*/
  void DrawLib::drawBox(const Vector2f &A,const Vector2f &B,float fBorder,Color Back,Color Front) {
    /* Alpha? */
    bool bAlpha = false;
    if(GET_ALPHA(Back)!=255 || GET_ALPHA(Front)!=255) bAlpha=true;
  
    if(bAlpha) {
      setBlendMode(BLEND_MODE_A);
    }
  
    /* Draw rectangle background */
    if(GET_ALPHA(Back)>0) {
      startDraw(DRAW_MODE_POLYGON);
      setColor(Back);
      glVertexSP(A.x,A.y);
      glVertexSP(A.x,B.y);
      glVertexSP(B.x,B.y);
      glVertexSP(B.x,A.y);
      endDraw();
    }
    
    /* Draw rectangle border */
    if(fBorder>0.0f && GET_ALPHA(Front)>0) {
      startDraw(DRAW_MODE_POLYGON);
      setColor(Front);
      if(bAlpha) { setBlendMode(BLEND_MODE_A); }
      glVertexSP(A.x,A.y);
      glVertexSP(A.x,B.y);
      glVertexSP(A.x+fBorder,B.y);
      glVertexSP(A.x+fBorder,A.y);
      endDraw();
      startDraw(DRAW_MODE_POLYGON);
      if(bAlpha) { setBlendMode(BLEND_MODE_A); }
      setColor(Front);
      glVertexSP(B.x-fBorder,A.y);
      glVertexSP(B.x-fBorder,B.y);
      glVertexSP(B.x,B.y);
      glVertexSP(B.x,A.y);
      endDraw();
      startDraw(DRAW_MODE_POLYGON);
      if(bAlpha) { setBlendMode(BLEND_MODE_A); }
      setColor(Front);
      glVertexSP(A.x,A.y);
      glVertexSP(A.x,A.y+fBorder);
      glVertexSP(B.x,A.y+fBorder);
      glVertexSP(B.x,A.y);
      endDraw();
      startDraw(DRAW_MODE_POLYGON);
      if(bAlpha) { setBlendMode(BLEND_MODE_A); }
      setColor(Front);
      setColor(Front);
      glVertexSP(A.x,B.y-fBorder);
      glVertexSP(A.x,B.y);
      glVertexSP(B.x,B.y);
      glVertexSP(B.x,B.y-fBorder);
      endDraw();
      
    }
    
   if(bAlpha) { setBlendMode(BLEND_MODE_NONE); }
  }
  
    
  /*===========================================================================
  Primitive: circle
  ===========================================================================*/
  void DrawLib::drawCircle(const Vector2f &Center,float fRadius,float fBorder,Color Back,Color Front) {
    /* Alpha? */
    bool bAlpha = false;
    if(GET_ALPHA(Back)!=255 || GET_ALPHA(Front)!=255) bAlpha=true;
    bAlpha = true;
    
    if(bAlpha) { setBlendMode(BLEND_MODE_A); }

    /* How many steps? */    
    int nSteps= (int) (2.0f*(fRadius / 3.0f));
    if(nSteps<8) nSteps=8;
    if(nSteps>64) nSteps=64;
    
    /* Draw circle background */
    if(GET_ALPHA(Back)>0) {
      startDraw(DRAW_MODE_POLYGON);
      if(bAlpha) { setBlendMode(BLEND_MODE_A); }
      setColor(Back);
      for(int i=0;i<nSteps;i++) {
        float rads = (3.14159f * 2.0f * (float)i)/ (float)nSteps;            
        glVertexSP(Center.x + fRadius*sin(rads),Center.y + fRadius*cos(rads));
      }
      endDraw();
    }
    
    /* Draw circle border */
    if(fBorder>0.0f && GET_ALPHA(Front)>0) {
      for(int i=0;i<nSteps;i++) {
        float rads1 = (3.14159f * 2.0f * (float)i)/ (float)nSteps;            
        float rads2 = (3.14159f * 2.0f * (float)(i+1))/ (float)nSteps;      
  
        startDraw(DRAW_MODE_POLYGON);              
        if(bAlpha) { setBlendMode(BLEND_MODE_A); }
        setColor(Front);
        glVertexSP(Center.x + fRadius*sin(rads1),Center.y + fRadius*cos(rads1));
        glVertexSP(Center.x + fRadius*sin(rads2),Center.y + fRadius*cos(rads2));
        glVertexSP(Center.x + (fRadius-fBorder)*sin(rads2),Center.y + (fRadius-fBorder)*cos(rads2));
        glVertexSP(Center.x + (fRadius-fBorder)*sin(rads1),Center.y + (fRadius-fBorder)*cos(rads1));
        endDraw();
      }
    }    

    /* Disable alpha again if we enabled it */
   if(bAlpha) { setBlendMode(BLEND_MODE_NONE); }
  }
  

  /*===========================================================================
  Primitive: box
  ===========================================================================*/
  void DrawLib::drawImage(const Vector2f &a,const Vector2f &b,Texture *pTexture,Color Tint) {
    setTexture(pTexture,BLEND_MODE_A);
    startDraw(DRAW_MODE_POLYGON);
    setColor(Tint);
    glTexCoord(0.0, 0.0);
    glVertexSP(a.x, a.y);
    glTexCoord(1.0, 0.0);
    glVertexSP(b.x, a.y);
    glTexCoord(1.0, 1.0);
    glVertexSP(b.x, b.y);
    glTexCoord(0.0, 1.0);
    glVertexSP(a.x, b.y);
    endDraw();
  }

  void DrawLib::toogleFullscreen() {
    if(SDL_WM_ToggleFullScreen(m_screen) != 0) {
      /* hum */
    }
  }

}
