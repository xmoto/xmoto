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

#ifndef __VDRAW_H__
#define __VDRAW_H__

#include "VCommon.h"

#include "helpers/VMath.h"
#include "VTexture.h"
#include "Theme.h"
#include "Image.h"
namespace vapp {
  //keesj:TODO why do I need to declare the Log function here?
  void Log(const char *pcFmt,...);

  /**
   * VApp draw modes to be used as argument in startDraw
   * DRAW_MODE_POLYGON is a filled polygon drawing mode
   * DRAW_MODE_LINE_LOOP is a polygon drawing mode (non filled)
   * DRAW_MODE_LINE_STRIP draws lines 
   **/
  enum DrawMode {
    DRAW_MODE_NONE, 
    DRAW_MODE_POLYGON,
    DRAW_MODE_LINE_LOOP,
    DRAW_MODE_LINE_STRIP
  };

  enum BlendMode {
    BLEND_MODE_NONE,//no blending
    BLEND_MODE_A,   //GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA
    BLEND_MODE_B,   //GL_ONE,GL_ONE

  };

  /*===========================================================================
  Class with various drawing functions
  ===========================================================================*/
  class DrawLib {
    public:
      DrawLib();

      /**
       * initialize the screen
       **/
      void init(int nDispWidth,int nDispHeight,int nDispBPP,bool bWindowed,Theme * ptheme);
      void unInit();
      
      void setDispWidth(int width){m_nDispWidth = width;};
      int getDispWidth(void) {return m_nDispWidth;}
      
      void setNoGraphics(bool disable_graphics){ m_bNoGraphics =disable_graphics;};
      bool isNoGraphics(){return m_bNoGraphics;};
      void setDispHeight(int height){m_nDispHeight =  height;};
      int getDispHeight(void) {return m_nDispHeight;}
      void setDispBPP(int bpp) {m_nDispBPP = bpp;}
      int getDispBPP(void) {return m_nDispBPP;}
      
      void setWindowed(bool windowed){ m_bWindowed = windowed;};
      bool getWindowed(void) {return m_bWindowed;}
      
      /* Methods - low-level */
      //add a vertex given screen coordinates
      void glVertexSP(float x,float y);
      //add a vertex given opengl coordinates
      void glVertex(float x,float y);
      void glVertex(Vector2f x){glVertex(x.x,x.y);};
      void screenProjVertex(float *x,float *y); 
      
      void setColor(Color color);
      /**
       * set the texure for drawing
       * the value may be NULL to disable texure
       * every end draw will reset the texture to NULL
       **/
      void setTexture(Texture * texture,BlendMode blendMode );
      void setBlendMode(BlendMode blendMode );
      void setColorRGB(int r,int g,int b){ setColor(MAKE_COLOR(r,g,b,255));};
      void setColorRGBA(int r,int g,int b,int a){ setColor(MAKE_COLOR(r,g,b,a));};

      /**
       * enables clipping and sets the clipping borders
       **/     
      void setClipRect(int x , int y , int w , int h);
      void setClipRect(SDL_Rect * i_clip_rect);
      
      /**
       * returns the current screen clipping
       **/
      void getClipRect(int *o_px,int *o_py,int *o_pnWidth,int *o_pnHeight);
      
      /**
       * Start drawing ... used in combination with glVertex
       **/
      void startDraw(DrawMode mode);
      
      /**
       * End draw
       **/
      void endDraw();

      /**
       * Clears the screen with the configured background
       **/
      void clearGraphics();
      
      /**
       * Flush the graphics. In memory graphics will now be displayed
       **/
      void flushGraphics();
      
      /* Methods - primitives */
      void drawCircle(const Vector2f &Center,float fRadius,float fBorder=1.0f,Color Back=0,Color Front=-1);
      void drawBox(const Vector2f &A,const Vector2f &B,float fBorder=1.0f,Color Back=0,Color Front=-1);      
      void drawImage(const Vector2f &A,const Vector2f &B,Texture *pTexture,Color Tint=-1);
      
      
      /* Methods - text */
      void drawText(const Vector2f &Pos,std::string Text,Color Back=0,Color Front=-1,bool bEdge=false);
      int getTextWidth(std::string Text);
      int getTextHeight(std::string Text);

      void setDontUseGLExtensions(bool dont_use){ m_bDontUseGLExtensions = dont_use;};
      Img * grabScreen(void); 
      /*
       * set the refecence drawing size
       **/
      void setDrawDims(int nActualW,int nActualH,int w,int h) {m_nDrawWidth=w; m_nDrawHeight=h;
                                                               m_nActualWidth=nActualW; m_nActualHeight=nActualH;}
//opengl specific to be removed
      bool useVBOs(void) {return m_bVBOSupported;};
      bool useFBOs(void) {return m_bFBOSupported;};
      bool useShaders(void) {return m_bShadersSupported;};
#ifdef ENABLE_OPENGL
      /**
       *keesj:TODO
       *I am not happy about all these public members:)
       **/
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
      PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT;
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
#endif
      
private:
    

      /* Data */
      int m_nDrawWidth,m_nDrawHeight;     
      int m_nActualWidth,m_nActualHeight;     
      bool isExtensionSupported(std::string Ext);

      Texture *m_pDefaultFontTex;
      Texture *m_texture;
      BlendMode m_blendMode;
      
      /* Public helper methods */
      void _InitTextRendering(Theme *p_theme);
      void _UninitTextRendering(Theme *p_theme);
      
      	
      int m_nDispWidth,m_nDispHeight,m_nDispBPP; /* Screen stuff */
      bool m_bWindowed;         /* Windowed or not */

      int m_nLScissorX,m_nLScissorY,m_nLScissorW,m_nLScissorH;
      bool m_bVBOSupported;
      bool m_bFBOSupported;
      bool m_bShadersSupported;
      bool m_bDontUseGLExtensions;
      bool m_bNoGraphics;       /* No-graphics mode */
  };
  
}

#endif

