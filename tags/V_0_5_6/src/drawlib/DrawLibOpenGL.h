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

#ifndef __DRAWLIBOPENGL_H__
#define __DRAWLIBOPENGL_H__

#include "DrawLib.h"
#include "../include/xm_OpenGL.h"

class DrawLibOpenGL : public DrawLib {
  public:
  DrawLibOpenGL();
  virtual ~DrawLibOpenGL();

  virtual void init(unsigned int nDispWidth, unsigned int nDispHeight, unsigned int nDispBPP,
		    bool bWindowed);
  virtual void unInit();

  virtual void glVertexSP(float x, float y);
  virtual void glVertex(float x, float y);
  
  //texture coordinate
  virtual void glTexCoord(float x, float y);
  virtual void setColor(Color color);

  /**
   * set the texture for drawing
   * the value may be NULL to disable texture
   * every end draw will reset the texture to NULL
   **/
  virtual void setTexture(Texture* texture, BlendMode blendMode);
  virtual void setBlendMode(BlendMode blendMode);

  /**
   * enables clipping and sets the clipping borders
   **/
  virtual void setClipRect(int x, int y, unsigned int w, unsigned int h);
  virtual void setClipRect(SDL_Rect* i_clip_rect);
  virtual void setScale(float x, float y);
  virtual void setTranslate(float x, float y);
  virtual void setMirrorY();
  virtual void setRotateZ(float i_angle);
  virtual void setLineWidth(float width);
  
  /**
   * returns the current screen clipping
   **/
  virtual void getClipRect(int *o_px, int *o_py, int *o_pnWidth,
			   int *o_pnHeight);

  /**
   * Start drawing ... used in combination with glVertex
   **/
  virtual void startDraw(DrawMode mode);

  /**
   * End draw
   **/
  virtual void endDraw();
  virtual void endDrawKeepProperties(); /* to keep textures, ... to render several times the same entity fastly */
  virtual void removePropertiesAfterEnd(); /* remove properties endDraw = endDrawKeepProperties + removePropertiesAfterEnd */

  /**
   * Clears the screen with the configured background
   **/
  virtual void clearGraphics();
  
  /**
   * Flush the graphics. In memory graphics will now be displayed
   **/
  virtual void flushGraphics();
  
  virtual FontManager* getFontManager(const std::string &i_fontFile, unsigned int i_fontSize, unsigned int i_fixedFontSize = 0);
  
  virtual Img *grabScreen(int i_reduce = 1);
  virtual bool isExtensionSupported(std::string Ext);

  /* Extensions */
  PFNGLGENBUFFERSARBPROC glGenBuffersARB;
  PFNGLBINDBUFFERARBPROC glBindBufferARB;
  PFNGLBUFFERDATAARBPROC glBufferDataARB;
  PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;

  /* Extensions (for render-to-texture) */
  PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXT;
  PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
  PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT;
  PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
  PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
  PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT;
  PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXT;
  PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
  PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
  PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
  PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
  PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT;
  PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
  PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT;
  PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
  PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC
  glGetFramebufferAttachmentParameterivEXT;
  PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT;
  
  /* Extensions (for shaders) */
  PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocationARB;
  PFNGLGETACTIVEATTRIBARBPROC glGetActiveAttribARB;
  PFNGLGETATTRIBLOCATIONARBPROC glGetAttribLocationARB;
  PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
  PFNGLGETHANDLEARBPROC glGetHandleARB;
  PFNGLDETACHOBJECTARBPROC glDetachObjectARB;
  PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
  PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
  PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
  PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
  PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
  PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
  PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;
  PFNGLVALIDATEPROGRAMARBPROC glValidateProgramARB;
  PFNGLUNIFORM1FARBPROC glUniform1fARB;
  PFNGLUNIFORM2FARBPROC glUniform2fARB;
  PFNGLUNIFORM3FARBPROC glUniform3fARB;
  PFNGLUNIFORM4FARBPROC glUniform4fARB;
  PFNGLUNIFORM1IARBPROC glUniform1iARB;
  PFNGLUNIFORM2IARBPROC glUniform2iARB;
  PFNGLUNIFORM3IARBPROC glUniform3iARB;
  PFNGLUNIFORM4IARBPROC glUniform4iARB;
  PFNGLUNIFORM1FVARBPROC glUniform1fvARB;
  PFNGLUNIFORM2FVARBPROC glUniform2fvARB;
  PFNGLUNIFORM3FVARBPROC glUniform3fvARB;
  PFNGLUNIFORM4FVARBPROC glUniform4fvARB;
  PFNGLUNIFORM1IVARBPROC glUniform1ivARB;
  PFNGLUNIFORM2IVARBPROC glUniform2ivARB;
  PFNGLUNIFORM3IVARBPROC glUniform3ivARB;
  PFNGLUNIFORM4IVARBPROC glUniform4ivARB;
  PFNGLUNIFORMMATRIX2FVARBPROC glUniformMatrix2fvARB;
  PFNGLUNIFORMMATRIX3FVARBPROC glUniformMatrix3fvARB;
  PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fvARB;
  PFNGLGETOBJECTPARAMETERFVARBPROC glGetObjectParameterfvARB;
  PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
  PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
  PFNGLGETATTACHEDOBJECTSARBPROC glGetAttachedObjectsARB;
  PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;
  PFNGLGETACTIVEUNIFORMARBPROC glGetActiveUniformARB;
  PFNGLGETUNIFORMFVARBPROC glGetUniformfvARB;
  PFNGLGETUNIFORMIVARBPROC glGetUniformivARB;
  PFNGLGETSHADERSOURCEARBPROC glGetShaderSourceARB;
};

#endif
