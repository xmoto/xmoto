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
  
    if(m_pApp->getDrawLib()->useFBOs()) {
      /* Create overlay */
  	  glEnable(GL_TEXTURE_2D);

	    m_pApp->getDrawLib()->glGenFramebuffersEXT(1,&m_FrameBufferID);
	    glGenTextures(1,&m_DynamicTextureID);
	    glBindTexture(GL_TEXTURE_2D,m_DynamicTextureID);
	    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,nWidth,nHeight,0,GL_RGB,GL_UNSIGNED_BYTE,0);

	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    	
  	  glDisable(GL_TEXTURE_2D);
  	  
  	  /* Shaders? */
  	  m_bUseShaders = m_pApp->getDrawLib()->useShaders();
  	  
  	  if(m_bUseShaders) {
  	    m_VertShaderID = m_pApp->getDrawLib()->glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
  	    m_FragShaderID = m_pApp->getDrawLib()->glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    	  
  	    if(!_SetShaderSource(m_VertShaderID,"SFXOverlay.vert") ||
  	      !_SetShaderSource(m_FragShaderID,"SFXOverlay.frag"))
  	      m_bUseShaders = false;
  	    else {
  	      /* Source loaded good... Now create the program */
  	      m_ProgramID = m_pApp->getDrawLib()->glCreateProgramObjectARB();
    	    
  	      /* Attach our shaders to it */
  	      m_pApp->getDrawLib()->glAttachObjectARB(m_ProgramID,m_VertShaderID);
  	      m_pApp->getDrawLib()->glAttachObjectARB(m_ProgramID,m_FragShaderID);
    	    
  	      /* Link it */
  	      m_pApp->getDrawLib()->glLinkProgramARB(m_ProgramID);

          int nStatus = 0;
          m_pApp->getDrawLib()->glGetObjectParameterivARB(m_ProgramID,
            GL_OBJECT_LINK_STATUS_ARB,(GLint*)&nStatus);
          if(!nStatus) {
            Log("-- Failed to link SFXOverlay shader program --");
            
            /* Retrieve info-log */
            int nInfoLogLen = 0;
            m_pApp->getDrawLib()->glGetObjectParameterivARB(m_ProgramID,
              GL_OBJECT_INFO_LOG_LENGTH_ARB,(GLint*)&nInfoLogLen);
		        char *pcInfoLog = new char[nInfoLogLen];
		        int nCharsWritten = 0;
		        m_pApp->getDrawLib()->glGetInfoLogARB(m_ProgramID,nInfoLogLen,
		          (GLsizei*)&nCharsWritten,pcInfoLog);
		        LogRaw(pcInfoLog);
		        delete [] pcInfoLog;
      			
            m_bUseShaders = false;
          }
          else {
            /* Linked OK! */          
          }
        }
  	  }  	     
  	}
  }
  
  void SFXOverlay::cleanUp(void) {
    if(m_pApp != NULL) {
      if(m_pApp->getDrawLib()->useFBOs()) {
        /* Delete stuff */
        glDeleteTextures(1,&m_DynamicTextureID);
  	    m_pApp->getDrawLib()->glDeleteFramebuffersEXT(1,&m_FrameBufferID);  
  	  }
  	}
  }

  /*===========================================================================
  Start/stop rendering
  ===========================================================================*/
  void SFXOverlay::beginRendering(void) {
    if(m_pApp->getDrawLib()->useFBOs()) {
  	  glEnable(GL_TEXTURE_2D);

	    m_pApp->getDrawLib()->glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,m_FrameBufferID);
	    m_pApp->getDrawLib()->glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_DynamicTextureID,0);

  	  glDisable(GL_TEXTURE_2D);

	    glViewport(0,0,m_nOverlayWidth,m_nOverlayHeight);
	  }
  }
  
  void SFXOverlay::endRendering(void) {
    if(m_pApp->getDrawLib()->useFBOs()) {
	    m_pApp->getDrawLib()->glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	    glViewport(0,0,m_pApp->getDrawLib()->getDispWidth(),m_pApp->getDrawLib()->getDispHeight());
    }
  }

  /*===========================================================================
  Load shader source
  ===========================================================================*/
  char **SFXOverlay::_LoadShaderSource(const std::string &File,int *pnNumLines) {
    FileHandle *pfh = FS::openIFile(std::string("Shaders/") + File);
    if(pfh != NULL) {
      /* Load lines */
      std::string Line;
      std::vector<std::string> Lines;
      while(FS::readNextLine(pfh,Line)) {
        Lines.push_back(Line);
      }
      FS::closeFile(pfh);
      
      /* Convert line array into something OpenGL will eat */
      char **ppc = new char*[Lines.size()];
      for(int i=0;i<Lines.size();i++) {
        char *pc = new char[Lines[i].length() + 1];
        strcpy(pc,Lines[i].c_str());
        ppc[i] = pc;
      }
      *pnNumLines = Lines.size();
      
      return ppc;
    }
    /* Nothing! */
    return NULL;
  }

  bool SFXOverlay::_SetShaderSource(GLhandleARB ShaderID,const std::string &File) {
    /* Try loading file */
    int nNumLines;
    char **ppc = _LoadShaderSource(File,&nNumLines);
    if(ppc == NULL) return false; /* no shader */
    
    /* Pass it to OpenGL */
    m_pApp->getDrawLib()->glShaderSourceARB(ShaderID,nNumLines,(const GLcharARB **)ppc,NULL);
    
    /* Compile it! */
    m_pApp->getDrawLib()->glCompileShaderARB(ShaderID);
    int nStatus = 0;
    m_pApp->getDrawLib()->glGetObjectParameterivARB(ShaderID,
      GL_OBJECT_COMPILE_STATUS_ARB,(GLint*)&nStatus);
    if(!nStatus) {
      _FreeShaderSource(ppc,nNumLines);
      Log("-- Failed to compile shader: %s --",File.c_str());
      
      /* Retrieve info-log */
      int nInfoLogLen = 0;
      m_pApp->getDrawLib()->glGetObjectParameterivARB(ShaderID,
        GL_OBJECT_INFO_LOG_LENGTH_ARB,(GLint*)&nInfoLogLen);
		  char *pcInfoLog = new char[nInfoLogLen];
		  int nCharsWritten = 0;
		  m_pApp->getDrawLib()->glGetInfoLogARB(ShaderID,nInfoLogLen,
		    (GLsizei*)&nCharsWritten,pcInfoLog);
		  LogRaw(pcInfoLog);
		  delete [] pcInfoLog;
			
      return false;
    }
    
    /* Free source */
    _FreeShaderSource(ppc,nNumLines);
    return true;
  }  

  void SFXOverlay::_FreeShaderSource(char **ppc,int nNumLines) {
    for(int i=0;i<nNumLines;i++) {
      delete [] ppc[i];
    }
    delete [] ppc;
  }

  /*===========================================================================
  Fading...
  ===========================================================================*/
  void SFXOverlay::fade(float f) {
    if(m_pApp->getDrawLib()->useFBOs()) {
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
  
  void SFXOverlay::present(void) {
    if(m_pApp->getDrawLib()->useFBOs()) {   
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      glOrtho(0,m_pApp->getDrawLib()->getDispWidth(),0,m_pApp->getDrawLib()->getDispHeight(),-1,1);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      
      if(m_bUseShaders)
        m_pApp->getDrawLib()->glUseProgramObjectARB(m_ProgramID);

      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE,GL_ONE);
      
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D,m_DynamicTextureID);
      glBegin(GL_POLYGON);
      glColor3f(1,1,1);
      glTexCoord2f(0,0);
      glVertex2f(0,0);
      glTexCoord2f(1,0);
      glVertex2f(m_pApp->getDrawLib()->getDispWidth(),0);
      glTexCoord2f(1,1);
      glVertex2f(m_pApp->getDrawLib()->getDispWidth(),m_pApp->getDrawLib()->getDispHeight());
      glTexCoord2f(0,1);
      glVertex2f(0,m_pApp->getDrawLib()->getDispHeight());
      glEnd();
      glDisable(GL_TEXTURE_2D);

      glDisable(GL_BLEND);

      if(m_bUseShaders)
        m_pApp->getDrawLib()->glUseProgramObjectARB(NULL);

      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
    }
  }
}

