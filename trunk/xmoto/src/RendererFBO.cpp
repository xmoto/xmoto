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
 *  In-game rendering (FBO stuff)
 */
#include "VXml.h"
#include "VFileIO.h"
#include "MotoGame.h"
#include "Renderer.h"
#include "GameText.h"

namespace vapp {
  
  /*===========================================================================
  Init and clean up
  ===========================================================================*/
  void SFXOverlay::init(App *pApp,int nWidth,int nHeight) {    
    m_pApp = pApp;
    
    m_nOverlayWidth = nWidth;
    m_nOverlayHeight = nHeight;
  
    if(m_pApp->useFBOs()) {
      /* Create overlay */
  	  glEnable(GL_TEXTURE_2D);

	    m_pApp->glGenFramebuffersEXT(1,&m_FrameBufferID);
	    glGenTextures(1,&m_DynamicTextureID);
	    glBindTexture(GL_TEXTURE_2D,m_DynamicTextureID);
	    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,nWidth,nHeight,0,GL_RGB,GL_UNSIGNED_BYTE,0);

	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    	
  	  glDisable(GL_TEXTURE_2D);
  	}
  }
  
  void SFXOverlay::cleanUp(void) {
    if(m_pApp->useFBOs()) {
      /* Delete stuff */
      glDeleteTextures(1,&m_DynamicTextureID);
  	  m_pApp->glDeleteFramebuffersEXT(1,&m_FrameBufferID);  
  	}
  }

  /*===========================================================================
  Start/stop rendering
  ===========================================================================*/
  void SFXOverlay::beginRendering(void) {
    if(m_pApp->useFBOs()) {
  	  glEnable(GL_TEXTURE_2D);

	    m_pApp->glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,m_FrameBufferID);
	    m_pApp->glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_DynamicTextureID,0);

  	  glDisable(GL_TEXTURE_2D);

	    glViewport(0,0,m_nOverlayWidth,m_nOverlayHeight);
	  }
  }
  
  GLuint SFXOverlay::endRendering(void) {
    if(m_pApp->useFBOs()) {
	    m_pApp->glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	    glViewport(0,0,m_pApp->getDispWidth(),m_pApp->getDispHeight());
	    
	    return m_DynamicTextureID;
    }
    return 0;  
  }

  /*===========================================================================
  Fading...
  ===========================================================================*/
  void SFXOverlay::fade(float f) {
    if(m_pApp->useFBOs()) {
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      glOrtho(0,1,0,1,-1,1);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();      
    
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      glBegin(GL_POLYGON);
      glColor4f(0,0,0,f);
      glVertex2f(0,0);
      glVertex2f(1,0);
      glVertex2f(1,1);
      glVertex2f(0,1);
      glEnd();   
      glDisable(GL_BLEND);         

      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);      
    }
  }
};

