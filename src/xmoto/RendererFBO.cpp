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
#include "GameText.h"
#include "Renderer.h"
#include "common/VFileIO.h"
#include "common/VXml.h"
#include "drawlib/DrawLib.h"
#include "helpers/Log.h"
#include "helpers/RenderSurface.h"

#ifdef ENABLE_OPENGL
#include "drawlib/DrawLibOpenGL.h"
#endif

/*===========================================================================
Init and clean up
===========================================================================*/
void SFXOverlay::init(DrawLib *i_drawLib,
                      RenderSurface *i_screen,
                      unsigned int nWidth,
                      unsigned int nHeight) {
#ifdef ENABLE_OPENGL
  m_drawLib = i_drawLib;
  m_screen = i_screen;
  m_nOverlayWidth = nWidth;
  m_nOverlayHeight = nHeight;

  if (m_drawLib->useFBOs()) {
    /* Create overlay */
    glEnable(GL_TEXTURE_2D);

    glGenFramebuffers(1, &m_FrameBufferID);

    glGenTextures(1, &m_DynamicTextureID);
    glBindTexture(GL_TEXTURE_2D, m_DynamicTextureID);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 nWidth,
                 nHeight,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glDisable(GL_TEXTURE_2D);

    /* Shaders? */
    m_bUseShaders = m_drawLib->useShaders();

    if (m_bUseShaders) {
      unsigned int numVSSrcLines = 0, numFSSrcLines = 0;
      std::string vsSource = _LoadShaderSource("SFXOverlay.vert");
      std::string fsSource = _LoadShaderSource("SFXOverlay.frag");

      m_VertShaderID = _CreateShader(GL_VERTEX_SHADER, vsSource);
      m_FragShaderID = _CreateShader(GL_FRAGMENT_SHADER, fsSource);
      
      if (false)
        m_bUseShaders = false;
      else {
        /* Source loaded good... Now create the program */
        m_ProgramID = glCreateProgram();

        /* Attach our shaders to it */
        glAttachShader(m_ProgramID, m_VertShaderID);
        glAttachShader(m_ProgramID, m_FragShaderID);

        /* Link it */
        glLinkProgram(m_ProgramID);


        if (!_CheckStatus(m_ProgramID, true, GL_LINK_STATUS)) {
        }

        glDetachShader(m_ProgramID, m_VertShaderID);
        glDetachShader(m_ProgramID, m_FragShaderID);

        glDeleteShader(m_VertShaderID);
        glDeleteShader(m_FragShaderID);

        /*
        int nStatus = 0;
        ((DrawLibOpenGL *)m_drawLib)
          ->glGetObjectParameterivARB(
            m_ProgramID, GL_OBJECT_LINK_STATUS_ARB, (GLint *)&nStatus);
        if (!nStatus) {
          LogError("-- Failed to link SFXOverlay shader program --");

          /* Retrieve info-log * /
          int nInfoLogLen = 0;
          ((DrawLibOpenGL *)m_drawLib)
            ->glGetObjectParameterivARB(m_ProgramID,
                                        GL_OBJECT_INFO_LOG_LENGTH_ARB,
                                        (GLint *)&nInfoLogLen);
          char *pcInfoLog = new char[nInfoLogLen];
          int nCharsWritten = 0;
          ((DrawLibOpenGL *)m_drawLib)
            ->glGetInfoLogARB(
              m_ProgramID, nInfoLogLen, (GLsizei *)&nCharsWritten, pcInfoLog);
          LogInfo(pcInfoLog);
          delete[] pcInfoLog;

          m_bUseShaders = false;
        } else {
          /* Linked OK! * /
        }
        */

        glValidateProgram(m_ProgramID);
      }
    }
  }
#endif
}

void SFXOverlay::cleanUp(void) {
#ifdef ENABLE_OPENGL
  if (m_drawLib != NULL) {
    if (m_drawLib->useFBOs()) {
      /* Delete stuff */
      glDeleteTextures(1, &m_DynamicTextureID);
      glDeleteFramebuffers(1, &m_FrameBufferID);
    }
  }
#endif
}

/*===========================================================================
Start/stop rendering
===========================================================================*/
void SFXOverlay::beginRendering(void) {
#ifdef ENABLE_OPENGL
  if (m_drawLib->useFBOs()) {
    glEnable(GL_TEXTURE_2D);

    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                              GL_COLOR_ATTACHMENT0,
                              GL_TEXTURE_2D,
                              m_DynamicTextureID,
                              0);

    glDisable(GL_TEXTURE_2D);

    glViewport(0, 0, m_nOverlayWidth, m_nOverlayHeight);
  }
#endif
}

void SFXOverlay::endRendering(void) {
#ifdef ENABLE_OPENGL
  if (m_drawLib->useFBOs()) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(m_screen->downleft().x,
               m_screen->downleft().y,
               m_screen->size().x,
               m_screen->size().y);
  }
#endif
}

#ifdef ENABLE_OPENGL

std::string SFXOverlay::_LoadShaderSource(const std::string &File) {
  std::string result;

  FILE* file = fopen(File.c_str(), "r");

  if (!file) {
    LogError("-- Failed to load file \"%s\" --", File.c_str());
    return "";
  } else {
    int res = fseek(file, 0L, SEEK_END);
    if (res != 0) {
      LogError("-- Failed to seek file end (\"%s\") --", File.c_str());
    } else {
      long nBytes = ftell(file);
      res = fseek(file, 0L, SEEK_SET);
      if (res != 0) {
        rewind(file);
      }

      size_t bufSz = (nBytes + 1) * sizeof(char);
      char *buf = (char *)malloc(bufSz);
      if (!buf) {
        LogError("-- malloc failed! --");
        return "";
      }

      size_t nBytesRead = fread(buf, sizeof(char), nBytes, file);

      buf = (char *)realloc(buf, nBytesRead + 1);
      if (!buf) {
        LogError("-- realloc failed! --");
        return "";
      }

      buf[nBytesRead] = '\0';

      result.append(buf);

      free(buf);
    }

    fclose(file);
  }

  return result;
}

GLuint SFXOverlay::_CreateShader(GLenum Type, const std::string &Code) {
  GLuint handle = 0;

  handle = glCreateShader(Type);
  const GLchar *code = (const GLchar *)Code.c_str();
  glShaderSource(handle, 1, &code, 0);
  glCompileShader(handle);

  if (0 == handle) {
    LogError("-- Failed to create shader: (type: %s) --",
      GL_VERTEX_SHADER == Type ? "vertex" :
      (GL_FRAGMENT_SHADER == Type ? "fragment" : "unknown"));
    return 0;
  }

  return handle;
}

bool SFXOverlay::_CheckStatus(GLuint Handle, bool IsProg, GLenum Pname, std::string *OutLog) {
  void (*fn1)(GLuint, GLenum, GLint *) = IsProg ? glGetProgramiv : glGetShaderiv;
  void (*fn2)(GLuint, GLsizei, GLsizei *, GLchar *) = IsProg ? glGetProgramInfoLog : glGetShaderInfoLog;

  GLint status = 0;
  
  fn1(Handle, Pname, &status);
  
  if (GL_FALSE == status) {
    GLint maxLen = 0;
    fn1(Handle, GL_INFO_LOG_LENGTH, &maxLen);
    size_t bufLen = maxLen + 1;
    char *infoLog = (char *)malloc(bufLen * sizeof(char));
    if (!infoLog) {
      LogWarning("-- malloc failed! --");
      return false;
    }

    fn2(Handle, maxLen, &maxLen, &infoLog[0]);
    infoLog[maxLen] = '\0';

    if (OutLog) {
      OutLog->append(infoLog);
    } else {
      LogInfo("%s", infoLog);
    }

    free(infoLog);
  }
}
#endif

/*===========================================================================
Fading...
===========================================================================*/
void SFXOverlay::fade(float f, unsigned int i_frameNumber) {
  if (i_frameNumber < 5) {
    f = 1.0; // fade 100% at the beginning of the ghost rendering to avoid bad
    // visual effects
  }

#ifdef ENABLE_OPENGL
  if (m_drawLib->useFBOs()) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_drawLib->startDraw(DRAW_MODE_POLYGON);
    m_drawLib->setColorRGBA(0, 0, 0, (int)(f * 255));
    glVertex2f(0, 0);
    glVertex2f(1, 0);
    glVertex2f(1, 1);
    glVertex2f(0, 1);
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
  if (m_drawLib->useFBOs()) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, m_screen->getDispWidth(), 0, m_screen->getDispHeight(), -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    if (m_bUseShaders)
      glUseProgram(m_ProgramID);

    m_drawLib->setBlendMode(BLEND_MODE_B);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_DynamicTextureID);

    m_drawLib->drawImageTextureSet(
      Vector2f(0.0, 0.0),
      Vector2f(m_screen->getDispWidth(), 0.0),
      Vector2f(m_screen->getDispWidth(), m_screen->getDispHeight()),
      Vector2f(0.0, m_screen->getDispHeight()),
      MAKE_COLOR(255, 255, 255, 255));
    glDisable(GL_TEXTURE_2D);

    if (m_bUseShaders)
      glUseProgram(0);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
  }
#endif
}
