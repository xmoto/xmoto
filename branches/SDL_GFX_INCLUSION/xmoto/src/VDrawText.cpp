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
 *  This file is the part of the simple 2D drawing functionality, which
 *  handles text output.  
 */

#include "VDraw.h"
#include "BuiltInFont.h"

namespace vapp {

  /*===========================================================================
  Text dim probing
  ===========================================================================*/  
  int DrawLib::getTextHeight(std::string Text) {
    int cx = 0,cy = 0,c;
    int h=0;
    for(unsigned int i=0;i<Text.size();i++) {
      c = Text[i];
      if(c==' ') {
        cx += 8;
      }
      else if(c=='\r') {
        cx = 0;
      }
      else if(c=='\n') {
        cx = 0;
        cy += 12;
      }
      else {
        cx += 8;
      }
      if(cy > h) h=cx;
    }
    return h+12;
  }
  
  int DrawLib::getTextWidth(std::string Text) {
    int cx = 0,cy = 0,c;
    int w=0;
    for(unsigned int i=0;i<Text.size();i++) {
      c = Text[i];
      if(c==' ') {
        cx += 8;
      }
      else if(c=='\r') {
        cx = 0;
      }
      else if(c=='\n') {
        cx = 0;
        cy += 12;
      }
      else {
        cx += 8;
      }
      if(cx > w) w=cx;
    }
    return w;
  }

  /*===========================================================================
  Text drawing
  ===========================================================================*/  
  void DrawLib::drawText(const Vector2f &Pos,std::string Text,Color Back,Color Front,bool bEdge) {
		if(bEdge) {
			/* SLOOOOW */
			drawText(Pos + Vector2f(1,1),Text,0,MAKE_COLOR(0,0,0,255),false);
			drawText(Pos + Vector2f(-1,1),Text,0,MAKE_COLOR(0,0,0,255),false);
			drawText(Pos + Vector2f(1,-1),Text,0,MAKE_COLOR(0,0,0,255),false);
			drawText(Pos + Vector2f(-1,-1),Text,0,MAKE_COLOR(0,0,0,255),false);
		}
  
    int cx = (int) Pos.x, cy = (int) Pos.y, c;
    if(m_pDefaultFontTex != NULL) {
      glBindTexture(GL_TEXTURE_2D,m_pDefaultFontTex->nID);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    int nCharsPerLine = 256 / 8;
    for(unsigned int i=0;i<Text.size();i++) {
      c = Text[i];
      if(c==' ') {
        glBegin(GL_POLYGON);
        glColor4ub(GET_RED(Back),GET_GREEN(Back),GET_BLUE(Back),GET_ALPHA(Back));
        glVertexSP(cx,cy);
        glColor4ub(GET_RED(Back),GET_GREEN(Back),GET_BLUE(Back),GET_ALPHA(Back));
        glVertexSP(cx+8,cy);
        glColor4ub(GET_RED(Back),GET_GREEN(Back),GET_BLUE(Back),GET_ALPHA(Back));
        glVertexSP(cx+8,cy+12);
        glColor4ub(GET_RED(Back),GET_GREEN(Back),GET_BLUE(Back),GET_ALPHA(Back));
        glVertexSP(cx,cy+12);
        glEnd();        
        cx += 8;
      }
      else if(c=='\r') {
        cx = (int) Pos.x;
      }
      else if(c=='\n') {
        cx = (int) Pos.x;
        cy += 12;
      }
      else {
        int y1 = (c / nCharsPerLine) * 12;
        int x1 = (c % nCharsPerLine) * 8;
        int y2 = y1 + 12;
        int x2 = x1 + 8;
        glBegin(GL_POLYGON);
        glColor4ub(GET_RED(Back),GET_GREEN(Back),GET_BLUE(Back),GET_ALPHA(Back));
        glVertexSP(cx,cy);
        glColor4ub(GET_RED(Back),GET_GREEN(Back),GET_BLUE(Back),GET_ALPHA(Back));
        glVertexSP(cx+8,cy);
        glColor4ub(GET_RED(Back),GET_GREEN(Back),GET_BLUE(Back),GET_ALPHA(Back));
        glVertexSP(cx+8,cy+12);
        glColor4ub(GET_RED(Back),GET_GREEN(Back),GET_BLUE(Back),GET_ALPHA(Back));
        glVertexSP(cx,cy+12);
        glEnd();        
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_POLYGON);
        glTexCoord2f((float)x1/256.0f,(float)y1/256.0f);
        glColor4ub(GET_RED(Front),GET_GREEN(Front),GET_BLUE(Front),GET_ALPHA(Front));
        glVertexSP(cx,cy);
        glTexCoord2f((float)x2/256.0f,(float)y1/256.0f);
        glColor4ub(GET_RED(Front),GET_GREEN(Front),GET_BLUE(Front),GET_ALPHA(Front));
        glVertexSP(cx+8,cy);
        glTexCoord2f((float)x2/256.0f,(float)y2/256.0f);
        glColor4ub(GET_RED(Front),GET_GREEN(Front),GET_BLUE(Front),GET_ALPHA(Front));
        glVertexSP(cx+8,cy+12);
        glTexCoord2f((float)x1/256.0f,(float)y2/256.0f);
        glColor4ub(GET_RED(Front),GET_GREEN(Front),GET_BLUE(Front),GET_ALPHA(Front));
        glVertexSP(cx,cy+12);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        cx += 8;
      }
    }
    glDisable(GL_BLEND);
  }
  
  /*===========================================================================
  Init of text rendering
  ===========================================================================*/  
  void DrawLib::_InitTextRendering(Theme *p_theme) {   
    m_pDefaultFontTex = p_theme->getDefaultFont();
          
    /* Create font texture (default) */
    //m_pDefaultFontTexture = (DefaultFontTexture *)pTextureManager->loadTexture(new DefaultFontTexture,"default-font");
  }
  
  /*===========================================================================
  Uninit of text rendering
  ===========================================================================*/  
  void DrawLib::_UninitTextRendering(Theme *p_theme) {    
  }

  /*===========================================================================
  Create a default font texture
  ===========================================================================*/  
  //void DefaultFontTexture::load(std::string Name,bool bSmall) {
  //  /* Create font object */
  //  
  //  /* Pass it to GL */
  //  GLuint N;
  //  glEnable(GL_TEXTURE_2D);
  //  glGenTextures(1,&N);    
  //  glBindTexture(GL_TEXTURE_2D,N);
  //  glTexImage2D(GL_TEXTURE_2D,0,4,256,256,0,GL_RGBA,GL_UNSIGNED_BYTE,(void *)pImgData);
  //  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  //  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  //  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
  //  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
  //  glDisable(GL_TEXTURE_2D);
  //  
  //  m_TI.nID = N;
//  }

  /*===========================================================================
  Unload a default font texture
  ===========================================================================*/    
  //void DefaultFontTexture::unload(void) {
 // }
      
}

