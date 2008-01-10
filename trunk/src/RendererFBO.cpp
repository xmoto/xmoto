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

/* 
 *  In-game rendering (FBO stuff)
 */
#include "VXml.h"
#include "VFileIO.h"
#include "Renderer.h"
#include "GameText.h"
#include "helpers/Log.h"
#include "drawlib/DrawLib.h"

#ifdef ENABLE_OPENGL
#include "drawlib/DrawLibOpenGL.h"
#endif

  /*===========================================================================
  Init and clean up
  ===========================================================================*/
  void SFXOverlay::init(DrawLib* i_drawLib, unsigned int nWidth, unsigned int nHeight) {
#ifdef ENABLE_OPENGL
    m_drawLib = i_drawLib;
    
    m_nOverlayWidth = nWidth;
    m_nOverlayHeight = nHeight;
  
    if(m_drawLib->useFBOs()) {
      /* Create overlay */
      glEnable(GL_TEXTURE_2D);

      ((DrawLibOpenGL*)m_drawLib)->glGenFramebuffersEXT(1,&m_FrameBufferID);
      glGenTextures(1,&m_DynamicTextureID);
      glBindTexture(GL_TEXTURE_2D,m_DynamicTextureID);
      glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,nWidth,nHeight,0,GL_RGB,GL_UNSIGNED_BYTE,0);

      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    	
      glDisable(GL_TEXTURE_2D);
  	  
      /* Shaders? */
      m_bUseShaders = m_drawLib->useShaders();
  	  
      if(m_bUseShaders) {
	m_VertShaderID = ((DrawLibOpenGL*)m_drawLib)->glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	m_FragShaderID = ((DrawLibOpenGL*)m_drawLib)->glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    	  
	if(!_SetShaderSource(m_VertShaderID,"SFXOverlay.vert") ||
	   !_SetShaderSource(m_FragShaderID,"SFXOverlay.frag"))
	  m_bUseShaders = false;
	else {
	  /* Source loaded good... Now create the program */
	  m_ProgramID = ((DrawLibOpenGL*)m_drawLib)->glCreateProgramObjectARB();
    	    
	  /* Attach our shaders to it */
	  ((DrawLibOpenGL*)m_drawLib)->glAttachObjectARB(m_ProgramID,m_VertShaderID);
	  ((DrawLibOpenGL*)m_drawLib)->glAttachObjectARB(m_ProgramID,m_FragShaderID);
    	    
	  /* Link it */
	  ((DrawLibOpenGL*)m_drawLib)->glLinkProgramARB(m_ProgramID);

          int nStatus = 0;
          ((DrawLibOpenGL*)m_drawLib)->glGetObjectParameterivARB(m_ProgramID,
								 GL_OBJECT_LINK_STATUS_ARB,(GLint*)&nStatus);
          if(!nStatus) {
            Logger::Log("-- Failed to link SFXOverlay shader program --");
            
            /* Retrieve info-log */
            int nInfoLogLen = 0;
            ((DrawLibOpenGL*)m_drawLib)->glGetObjectParameterivARB(m_ProgramID,
								   GL_OBJECT_INFO_LOG_LENGTH_ARB,(GLint*)&nInfoLogLen);
	    char *pcInfoLog = new char[nInfoLogLen];
	    int nCharsWritten = 0;
	    ((DrawLibOpenGL*)m_drawLib)->glGetInfoLogARB(m_ProgramID,nInfoLogLen,
							 (GLsizei*)&nCharsWritten,pcInfoLog);
	    Logger::Log(pcInfoLog);
	    delete [] pcInfoLog;
      			
            m_bUseShaders = false;
          }
          else {
            /* Linked OK! */          
          }
        }
      }  	     
    }
#endif
  }
  
  void SFXOverlay::cleanUp(void) {
#ifdef ENABLE_OPENGL
    if(m_drawLib != NULL) {
      if(m_drawLib->useFBOs()) {
        /* Delete stuff */
        glDeleteTextures(1,&m_DynamicTextureID);
  	    ((DrawLibOpenGL*)m_drawLib)->glDeleteFramebuffersEXT(1,&m_FrameBufferID);  
  	  }
  	}
#endif
  }

  /*===========================================================================
  Start/stop rendering
  ===========================================================================*/
  void SFXOverlay::beginRendering(void) {
#ifdef ENABLE_OPENGL
    if(m_drawLib->useFBOs()) {
  	  glEnable(GL_TEXTURE_2D);

	    ((DrawLibOpenGL*)m_drawLib)->glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,m_FrameBufferID);
	    ((DrawLibOpenGL*)m_drawLib)->glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_DynamicTextureID,0);

  	  glDisable(GL_TEXTURE_2D);

	    glViewport(0,0,m_nOverlayWidth,m_nOverlayHeight);
	  }
#endif
  }
  
  void SFXOverlay::endRendering(void) {
#ifdef ENABLE_OPENGL
    if(m_drawLib->useFBOs()) {
	    ((DrawLibOpenGL*)m_drawLib)->glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	    glViewport(0,0,m_drawLib->getDispWidth(),m_drawLib->getDispHeight());
    }
#endif
  }

#ifdef ENABLE_OPENGL
  /*===========================================================================
  Load shader source
  ===========================================================================*/
  char **SFXOverlay::_LoadShaderSource(const std::string &File, unsigned int *pnNumLines) {
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
      for(unsigned int i=0;i<Lines.size();i++) {
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

  bool SFXOverlay::_SetShaderSource(GLhandleARB ShaderID, const std::string &File) {
    /* Try loading file */
    unsigned int nNumLines;
    char **ppc = _LoadShaderSource(File, &nNumLines);
    if(ppc == NULL)
      return false; /* no shader */

    /* Pass it to OpenGL */
    ((DrawLibOpenGL*)m_drawLib)->glShaderSourceARB(ShaderID,nNumLines,(const GLcharARB **)ppc,NULL);

    /* Compile it! */
    ((DrawLibOpenGL*)m_drawLib)->glCompileShaderARB(ShaderID);
    int nStatus = 0;
    ((DrawLibOpenGL*)m_drawLib)->glGetObjectParameterivARB(ShaderID,
      GL_OBJECT_COMPILE_STATUS_ARB,(GLint*)&nStatus);
    if(!nStatus) {
      _FreeShaderSource(ppc,nNumLines);
      Logger::Log("-- Failed to compile shader: %s --",File.c_str());

      /* Retrieve info-log */
      int nInfoLogLen = 0;
      ((DrawLibOpenGL*)m_drawLib)->glGetObjectParameterivARB(ShaderID,
        GL_OBJECT_INFO_LOG_LENGTH_ARB,(GLint*)&nInfoLogLen);
		  char *pcInfoLog = new char[nInfoLogLen];
		  int nCharsWritten = 0;
		  ((DrawLibOpenGL*)m_drawLib)->glGetInfoLogARB(ShaderID,nInfoLogLen,
		    (GLsizei*)&nCharsWritten,pcInfoLog);
		  Logger::Log(pcInfoLog);
		  delete [] pcInfoLog;
	
      return false;
    }

    /* Free source */
    _FreeShaderSource(ppc,nNumLines);
    return true;
  }  

  void SFXOverlay::_FreeShaderSource(char **ppc, unsigned int nNumLines) {
    for(unsigned int i=0;i<nNumLines;i++) {
      delete [] ppc[i];
    }
    delete [] ppc;
  }
#endif

  /*===========================================================================
  Fading...
  ===========================================================================*/
  void SFXOverlay::fade(float f) {
#ifdef ENABLE_OPENGL
    if(m_drawLib->useFBOs()) {
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      glOrtho(0,1,0,1,-1,1);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();      
    
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      m_drawLib->startDraw(DRAW_MODE_POLYGON);
      m_drawLib->setColorRGBA(0, 0, 0, (int)(f * 255));
      glVertex2f(0,0);
      glVertex2f(1,0);
      glVertex2f(1,1);
      glVertex2f(0,1);
      m_drawLib->endDraw();
      glDisable(GL_BLEND);         

      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);      
    }
#endif
  }
  
  void SFXOverlay::present(void) {
#ifdef ENABLE_OPENGL
    if(m_drawLib->useFBOs()) {   
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      glOrtho(0,m_drawLib->getDispWidth(),0,m_drawLib->getDispHeight(),-1,1);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      
      if(m_bUseShaders)
        ((DrawLibOpenGL*)m_drawLib)->glUseProgramObjectARB(m_ProgramID);

      m_drawLib->setBlendMode(BLEND_MODE_B);
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D,m_DynamicTextureID);

      // 0.01 and 0.99 not nice, should be replaced by DrawLib::drawImage, but m_dynamictextureid is not a texture
      m_drawLib->startDraw(DRAW_MODE_POLYGON);
      m_drawLib->setColorRGB(255,255,255);
      glTexCoord2f(0.01,0.01);
      glVertex2f(0,0);
      glTexCoord2f(0.99,0.01);
      glVertex2f(m_drawLib->getDispWidth(),0);
      glTexCoord2f(0.99,0.99);
      glVertex2f(m_drawLib->getDispWidth(),m_drawLib->getDispHeight());
      glTexCoord2f(0.01,0.99);
      glVertex2f(0,m_drawLib->getDispHeight());
      m_drawLib->endDraw();

      glDisable(GL_TEXTURE_2D);

      if(m_bUseShaders)
        ((DrawLibOpenGL*)m_drawLib)->glUseProgramObjectARB(0);

      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
    }
#endif
  }

