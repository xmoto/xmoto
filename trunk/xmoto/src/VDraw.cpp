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

 DrawLib::DrawLib() {
   m_pDefaultFontTex = NULL;
 }

  /*===========================================================================
  Transform an OpenGL vertex to pure 2D 
  ===========================================================================*/
  void DrawLib::glVertex(float x,float y) {
    //float tx = (((float)m_nActualWidth) / 2.0f) - 
	  /*glVertex2f(-1.0f + (x*2.0f + 0.1f)/(float)(m_nActualWidth),
					    1.0f - (y*2.0f + 0.1f)/(float)(m_nActualHeight));
  */
	  //glVertex2f(-1.0f + (x*2.0f + 0.1f)/(float)(m_nDrawWidth),
			//		    1.0f - (y*2.0f + 0.1f)/(float)(m_nDrawHeight));
	  
	  glVertex2f(m_nActualWidth/2 - m_nDrawWidth/2 + x,
	             m_nActualHeight - (m_nActualHeight/2 - m_nDrawHeight/2 + y));
  }

  void DrawLib::screenProjVertex(float *x,float *y) {
    *y = m_nActualHeight - (*y);
  
    //*x = -1.0f + ((*x)*2.0f + 0.1f)/(float)(m_nDrawWidth);
    //*y = 1.0f - ((*y)*2.0f + 0.1f)/(float)(m_nDrawHeight);
  }

  /*===========================================================================
  Primitive: circle
  ===========================================================================*/
  void DrawLib::drawCircle(const Vector2f &Center,float fRadius,float fBorder,Color Back,Color Front) {
    /* Alpha? */
    bool bAlpha = false;
    if(GET_ALPHA(Back)!=255 || GET_ALPHA(Front)!=255) bAlpha=true;
    bAlpha = true;
    
    if(bAlpha) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    }

    /* How many steps? */    
    int nSteps=2.0f*(fRadius / 3.0f);
    if(nSteps<8) nSteps=8;
    if(nSteps>64) nSteps=64;
    
    /* Draw circle background */
    if(GET_ALPHA(Back)>0) {
      glBegin(GL_POLYGON);
      glColor4ub(GET_RED(Back),GET_GREEN(Back),GET_BLUE(Back),GET_ALPHA(Back));
      for(int i=0;i<nSteps;i++) {
        float rads = (3.14159f * 2.0f * (float)i)/ (float)nSteps;            
        glVertex(Center.x + fRadius*sin(rads),Center.y + fRadius*cos(rads));
      }
      glEnd();
    }
    
    /* Draw circle border */
    if(fBorder>0.0f && GET_ALPHA(Front)>0) {
      for(int i=0;i<nSteps;i++) {
        float rads1 = (3.14159f * 2.0f * (float)i)/ (float)nSteps;            
        float rads2 = (3.14159f * 2.0f * (float)(i+1))/ (float)nSteps;      
  
        glBegin(GL_POLYGON);              
        glColor4ub(GET_RED(Front),GET_GREEN(Front),GET_BLUE(Front),GET_ALPHA(Front));
        glVertex(Center.x + fRadius*sin(rads1),Center.y + fRadius*cos(rads1));
        glVertex(Center.x + fRadius*sin(rads2),Center.y + fRadius*cos(rads2));
        glVertex(Center.x + (fRadius-fBorder)*sin(rads2),Center.y + (fRadius-fBorder)*cos(rads2));
        glVertex(Center.x + (fRadius-fBorder)*sin(rads1),Center.y + (fRadius-fBorder)*cos(rads1));
        glEnd();
      }
    }    

    /* Disable alpha again if we enabled it */
    if(bAlpha) glDisable(GL_BLEND);    
  }
  
  /*===========================================================================
  Primitive: box
  ===========================================================================*/
  void DrawLib::drawBox(const Vector2f &A,const Vector2f &B,float fBorder,Color Back,Color Front) {
    /* Alpha? */
    bool bAlpha = false;
    if(GET_ALPHA(Back)!=255 || GET_ALPHA(Front)!=255) bAlpha=true;
  
    if(bAlpha) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    }
  
    /* Draw rectangle background */
    if(GET_ALPHA(Back)>0) {
      glBegin(GL_POLYGON);
      glColor4ub(GET_RED(Back),GET_GREEN(Back),GET_BLUE(Back),GET_ALPHA(Back));
      glVertex(A.x,A.y);
      glVertex(A.x,B.y);
      glVertex(B.x,B.y);
      glVertex(B.x,A.y);
      glEnd();
    }
    
    /* Draw rectangle border */
    if(fBorder>0.0f && GET_ALPHA(Front)>0) {
      glBegin(GL_POLYGON);
      glColor4ub(GET_RED(Front),GET_GREEN(Front),GET_BLUE(Front),GET_ALPHA(Front));
      glVertex(A.x,A.y);
      glVertex(A.x,B.y);
      glVertex(A.x+fBorder,B.y);
      glVertex(A.x+fBorder,A.y);
      glEnd();
      glBegin(GL_POLYGON);
      glColor4ub(GET_RED(Front),GET_GREEN(Front),GET_BLUE(Front),GET_ALPHA(Front));
      glVertex(B.x-fBorder,A.y);
      glVertex(B.x-fBorder,B.y);
      glVertex(B.x,B.y);
      glVertex(B.x,A.y);
      glEnd();
      glBegin(GL_POLYGON);
      glColor4ub(GET_RED(Front),GET_GREEN(Front),GET_BLUE(Front),GET_ALPHA(Front));
      glVertex(A.x,A.y);
      glVertex(A.x,A.y+fBorder);
      glVertex(B.x,A.y+fBorder);
      glVertex(B.x,A.y);
      glEnd();
      glBegin(GL_POLYGON);
      glColor4ub(GET_RED(Front),GET_GREEN(Front),GET_BLUE(Front),GET_ALPHA(Front));
      glVertex(A.x,B.y-fBorder);
      glVertex(A.x,B.y);
      glVertex(B.x,B.y);
      glVertex(B.x,B.y-fBorder);
      glEnd();
      
    }
    
    /* Disable alpha again if we enabled it */
    if(bAlpha) glDisable(GL_BLEND);
  }

  /*===========================================================================
  Primitive: box
  ===========================================================================*/
  void DrawLib::drawImage(const Vector2f &A,const Vector2f &B,Texture *pTexture,Color Tint) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D,pTexture->nID);
    glEnable(GL_TEXTURE_2D);
    
    glBegin(GL_POLYGON);
    glColor4ub(GET_RED(Tint),GET_GREEN(Tint),GET_BLUE(Tint),GET_ALPHA(Tint));
    glTexCoord2f(0.0,0.0);
    glVertex(A.x,A.y);
    glTexCoord2f(0.0,1.0);
    glVertex(A.x,B.y);
    glTexCoord2f(1.0,1.0);
    glVertex(B.x,B.y);
    glTexCoord2f(1.0,0.0);
    glVertex(B.x,A.y);
    glEnd();

    glDisable(GL_TEXTURE_2D);        
    glDisable(GL_BLEND);
  }
        
  /*===========================================================================
  Init of 2D drawing library
  ===========================================================================*/
  void DrawLib::initLib(Theme *p_theme) {
    _InitTextRendering(p_theme);
  }
  
  /*===========================================================================
  Uninit
  ===========================================================================*/
  void DrawLib::uninitLib(Theme *p_theme) {
    _UninitTextRendering(p_theme);
  }
  
};

