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
#include "VApp.h"
#include "BuiltInFont.h"

#define DRAW_FONT_FILE "Textures/Fonts/DejaVuSans.ttf"

#ifdef ENABLE_OPENGL
namespace vapp {


 DrawLibOpenGL::~DrawLibOpenGL(){
 }

 DrawLibOpenGL::DrawLibOpenGL() : DrawLib(){
   m_fontSmall  = getFontManager(FS::FullPath(DRAW_FONT_FILE), 14);
   m_fontMedium = getFontManager(FS::FullPath(DRAW_FONT_FILE), 22);
 };
 
  /*===========================================================================
  Transform an OpenGL vertex to pure 2D 
  ===========================================================================*/
  void DrawLibOpenGL::glVertexSP(float x,float y) {
	  glVertex2f(m_nActualWidth/2 - m_nDrawWidth/2 + x,
	             m_nActualHeight - (m_nActualHeight/2 - m_nDrawHeight/2 + y));
  }

  void DrawLibOpenGL::glVertex(float x,float y) {
	  glVertex2f(x,y);
  }

  void DrawLibOpenGL::glTexCoord(float x,float y){
    glTexCoord2f(x,y);
  } 

  void DrawLibOpenGL::screenProjVertex(float *x,float *y) {
    *y = m_nActualHeight - (*y);
  }

  void DrawLibOpenGL::setClipRect(int x , int y , int w , int h){
    glScissor(x,m_nDispHeight - (y+h),w,h);
    
    m_nLScissorX = x;
    m_nLScissorY = y;
    m_nLScissorW = w;
    m_nLScissorH = h;
  }

  void DrawLibOpenGL::setClipRect(SDL_Rect * clip_rect){
        if (clip_rect != NULL){
  	  setClipRect(clip_rect->x,clip_rect->y,clip_rect->w,clip_rect->h);
	}
  }

  void DrawLibOpenGL::getClipRect(int *px,int *py,int *pnWidth,int *pnHeight) {
    *px = m_nLScissorX;
    *py = m_nLScissorY;
    *pnWidth = m_nLScissorW;
    *pnHeight = m_nLScissorH;
  }  
  
  void DrawLibOpenGL::setScale(float x,float y){
    glScalef(x,y,1);
  }
  void DrawLibOpenGL::setTranslate(float x,float y){
    glTranslatef(x,y, 0);
  }
  void DrawLibOpenGL::setLineWidth(float width){
    glLineWidth(width);
  }
 
  void DrawLibOpenGL::init(int nDispWidth,int nDispHeight,int nDispBPP,bool bWindowed,Theme * ptheme){

    
    /* Set suggestions */
    m_nDispWidth = nDispWidth;
    m_nDispHeight = nDispHeight;
    m_nDispBPP = nDispBPP;
    m_bWindowed = bWindowed;

    /* Get some video info */
    const SDL_VideoInfo *pVidInfo=SDL_GetVideoInfo();
    if(pVidInfo==NULL)
      throw Exception("(1) SDL_GetVideoInfo : " + std::string(SDL_GetError()));
  
    /* Determine target bit depth */
    if(m_bWindowed) 
      /* In windowed mode we can't tinker with the bit-depth */
      m_nDispBPP=pVidInfo->vfmt->BitsPerPixel;      

    /* Setup GL stuff */
    /* 2005-10-05 ... note that we no longer ask for explicit settings... it's
                      better to do it per auto */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1); 
  
    /* Create video flags */
    int nFlags = SDL_OPENGL;
    if(!m_bWindowed) nFlags|=SDL_FULLSCREEN;
  
    /* At last, try to "set the video mode" */
    if((m_screen=SDL_SetVideoMode(m_nDispWidth,m_nDispHeight,m_nDispBPP,nFlags))==NULL) {
      vapp::Log("** Warning ** : Tried to set video mode %dx%d @ %d-bit, but SDL responded: %s\n"
          "                Now SDL will try determining a proper mode itself.",m_nDispWidth,m_nDispHeight,m_nDispBPP);
    
      /* Hmm, try letting it decide the BPP automatically */
      if((m_screen=SDL_SetVideoMode(m_nDispWidth,m_nDispHeight,0,nFlags))==NULL) {       
        /* Still no luck */
        Log("** Warning ** : Still no luck, now we'll try 800x600 in a window.");
        m_nDispWidth = 800; m_nDispHeight = 600;        
        m_bWindowed = true;
        if((m_screen=SDL_SetVideoMode(m_nDispWidth,m_nDispHeight,0,SDL_OPENGL))==NULL) {       
          throw Exception("SDL_SetVideoMode : " + std::string(SDL_GetError()));
        }       
      }
    }
    
    /* Retrieve actual configuration */
    pVidInfo=SDL_GetVideoInfo();
    if(pVidInfo==NULL)
      throw Exception("(2) SDL_GetVideoInfo : " + std::string(SDL_GetError()));
                    
    m_nDispBPP=pVidInfo->vfmt->BitsPerPixel;

    /* Did we get a z-buffer? */        
    int nDepthBits;
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE,&nDepthBits);
    if(nDepthBits == 0)
      throw Exception("no depth buffer");  
  


    /* Force OpenGL to talk 2D */
    glViewport(0,0,m_nDispWidth,m_nDispHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,m_nDispWidth,0,m_nDispHeight,-1,1);
    
    glClearDepth(1);
    glDepthFunc(GL_LEQUAL);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();   
    
    /* Enable unicode translation and key repeats */
    SDL_EnableUNICODE(1);         
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
     
    /* Output some general info */
    Log("GL: %s (%s)",glGetString(GL_RENDERER),glGetString(GL_VENDOR));
    if(glGetString(GL_RENDERER) == NULL || 
       glGetString(GL_VENDOR) == NULL) {
      Log("** Warning ** : GL strings NULL!");
      throw Exception("GL strings are NULL!");
    }
    
    /* Windows: check whether we are using the standard GDI OpenGL software driver... If
       so make sure the user is warned */
    #if defined(WIN32) 
      if(!strcmp(reinterpret_cast<const char *>(glGetString(GL_RENDERER)),"GDI Generic") &&
         !strcmp(reinterpret_cast<const char *>(glGetString(GL_VENDOR)),"Microsoft Corporation")) {
        Log("** Warning ** : No GL hardware acceleration!");
        //m_UserNotify = "It seems that no OpenGL hardware acceleration is available!\n"
        //               "Please make sure OpenGL is configured properly.";
      }
    #endif
    
    /* Init OpenGL extensions */
    if(m_bDontUseGLExtensions) {
      m_bVBOSupported = false;
      m_bFBOSupported = false;
      m_bShadersSupported = false;
    }
    else {
      m_bVBOSupported = isExtensionSupported("GL_ARB_vertex_buffer_object");
      m_bFBOSupported = isExtensionSupported("GL_EXT_framebuffer_object");
      
      m_bShadersSupported = isExtensionSupported("GL_ARB_fragment_shader") &&
                            isExtensionSupported("GL_ARB_vertex_shader") &&
                            isExtensionSupported("GL_ARB_shader_objects");
    }
    
    if(m_bVBOSupported) {
      glGenBuffersARB=(PFNGLGENBUFFERSARBPROC)SDL_GL_GetProcAddress("glGenBuffersARB");
      glBindBufferARB=(PFNGLBINDBUFFERARBPROC)SDL_GL_GetProcAddress("glBindBufferARB");
      glBufferDataARB=(PFNGLBUFFERDATAARBPROC)SDL_GL_GetProcAddress("glBufferDataARB");
      glDeleteBuffersARB=(PFNGLDELETEBUFFERSARBPROC)SDL_GL_GetProcAddress("glDeleteBuffersARB");      

      glEnableClientState( GL_VERTEX_ARRAY );   
      glEnableClientState( GL_TEXTURE_COORD_ARRAY );
          
      Log("GL: using ARB_vertex_buffer_object");    
    }
    else
      Log("GL: not using ARB_vertex_buffer_object");    
      
    if(m_bFBOSupported) {
      glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glIsRenderbufferEXT");
      glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindRenderbufferEXT");
      glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffersEXT");
      glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenRenderbuffersEXT");
      glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)SDL_GL_GetProcAddress("glRenderbufferStorageEXT");
      glGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)SDL_GL_GetProcAddress("glGetRenderbufferParameterivEXT");
      glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC)SDL_GL_GetProcAddress("glIsFramebufferEXT");
      glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindFramebufferEXT");
      glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteFramebuffersEXT");
      glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenFramebuffersEXT");
      glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT");
      glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture1DEXT");
      glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture2DEXT");
      glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture3DEXT");
      glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glFramebufferRenderbufferEXT");
      glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)SDL_GL_GetProcAddress("glGetFramebufferAttachmentParameterivEXT");
      glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)SDL_GL_GetProcAddress("glGenerateMipmapEXT");
          
      Log("GL: using EXT_framebuffer_object");
    }
    else
      Log("GL: not using EXT_framebuffer_object");
      
    if(m_bShadersSupported) {
      glBindAttribLocationARB = (PFNGLBINDATTRIBLOCATIONARBPROC)SDL_GL_GetProcAddress("glBindAttribLocationARB");
      glGetActiveAttribARB = (PFNGLGETACTIVEATTRIBARBPROC)SDL_GL_GetProcAddress("glGetActiveAttribARB");
      glGetAttribLocationARB = (PFNGLGETATTRIBLOCATIONARBPROC)SDL_GL_GetProcAddress("glGetAttribLocationARB");
      glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)SDL_GL_GetProcAddress("glDeleteObjectARB");
      glGetHandleARB = (PFNGLGETHANDLEARBPROC)SDL_GL_GetProcAddress("glGetHandleARB");
      glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)SDL_GL_GetProcAddress("glDetachObjectARB");
      glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)SDL_GL_GetProcAddress("glCreateShaderObjectARB");
      glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)SDL_GL_GetProcAddress("glShaderSourceARB");
      glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)SDL_GL_GetProcAddress("glCompileShaderARB");
      glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)SDL_GL_GetProcAddress("glCreateProgramObjectARB");
      glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)SDL_GL_GetProcAddress("glAttachObjectARB");
      glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)SDL_GL_GetProcAddress("glLinkProgramARB");
      glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)SDL_GL_GetProcAddress("glUseProgramObjectARB");
      glValidateProgramARB = (PFNGLVALIDATEPROGRAMARBPROC)SDL_GL_GetProcAddress("glValidateProgramARB");
      glUniform1fARB = (PFNGLUNIFORM1FARBPROC)SDL_GL_GetProcAddress("glUniform1fARB");
      glUniform2fARB = (PFNGLUNIFORM2FARBPROC)SDL_GL_GetProcAddress("glUniform2fARB");
      glUniform3fARB = (PFNGLUNIFORM3FARBPROC)SDL_GL_GetProcAddress("glUniform3fARB");
      glUniform4fARB = (PFNGLUNIFORM4FARBPROC)SDL_GL_GetProcAddress("glUniform4fARB");
      glUniform1iARB = (PFNGLUNIFORM1IARBPROC)SDL_GL_GetProcAddress("glUniform1iARB");
      glUniform2iARB = (PFNGLUNIFORM2IARBPROC)SDL_GL_GetProcAddress("glUniform2iARB");
      glUniform3iARB = (PFNGLUNIFORM3IARBPROC)SDL_GL_GetProcAddress("glUniform3iARB");
      glUniform4iARB = (PFNGLUNIFORM4IARBPROC)SDL_GL_GetProcAddress("glUniform4iARB");
      glUniform1fvARB = (PFNGLUNIFORM1FVARBPROC)SDL_GL_GetProcAddress("glUniform1fvARB");
      glUniform2fvARB = (PFNGLUNIFORM2FVARBPROC)SDL_GL_GetProcAddress("glUniform2fvARB");
      glUniform3fvARB = (PFNGLUNIFORM3FVARBPROC)SDL_GL_GetProcAddress("glUniform3fvARB");
      glUniform4fvARB = (PFNGLUNIFORM4FVARBPROC)SDL_GL_GetProcAddress("glUniform4fvARB");
      glUniform1ivARB = (PFNGLUNIFORM1IVARBPROC)SDL_GL_GetProcAddress("glUniform1ivARB");
      glUniform2ivARB = (PFNGLUNIFORM2IVARBPROC)SDL_GL_GetProcAddress("glUniform2ivARB");
      glUniform3ivARB = (PFNGLUNIFORM3IVARBPROC)SDL_GL_GetProcAddress("glUniform3ivARB");
      glUniform4ivARB = (PFNGLUNIFORM4IVARBPROC)SDL_GL_GetProcAddress("glUniform4ivARB");
      glUniformMatrix2fvARB = (PFNGLUNIFORMMATRIX2FVARBPROC)SDL_GL_GetProcAddress("glUniformMatrix2fvARB");
      glUniformMatrix3fvARB = (PFNGLUNIFORMMATRIX3FVARBPROC)SDL_GL_GetProcAddress("glUniformMatrix3fvARB");
      glUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC)SDL_GL_GetProcAddress("glUniformMatrix4fvARB");
      glGetObjectParameterfvARB = (PFNGLGETOBJECTPARAMETERFVARBPROC)SDL_GL_GetProcAddress("glGetObjectParameterfvARB");
      glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)SDL_GL_GetProcAddress("glGetObjectParameterivARB");
      glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)SDL_GL_GetProcAddress("glGetInfoLogARB");
      glGetAttachedObjectsARB = (PFNGLGETATTACHEDOBJECTSARBPROC)SDL_GL_GetProcAddress("glGetAttachedObjectsARB");
      glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)SDL_GL_GetProcAddress("glGetUniformLocationARB");
      glGetActiveUniformARB = (PFNGLGETACTIVEUNIFORMARBPROC)SDL_GL_GetProcAddress("glGetActiveUniformARB");
      glGetUniformfvARB = (PFNGLGETUNIFORMFVARBPROC)SDL_GL_GetProcAddress("glGetUniformfvARB");
      glGetUniformivARB = (PFNGLGETUNIFORMIVARBPROC)SDL_GL_GetProcAddress("glGetUniformivARB");
      glGetShaderSourceARB = (PFNGLGETSHADERSOURCEARBPROC)SDL_GL_GetProcAddress("glGetShaderSourceARB");    
        
      Log("GL: using ARB_fragment_shader/ARB_vertex_shader/ARB_shader_objects");
    }
    else
      Log("GL: not using ARB_fragment_shader/ARB_vertex_shader/ARB_shader_objects");
    
    /* Set background color to black */
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapBuffers();  

    /* Init the Text drawing library */
    _InitTextRendering(ptheme);

}

  void DrawLibOpenGL::unInit(){
  }
        
  
  /*===========================================================================
  Check for OpenGL extension
  ===========================================================================*/
  bool DrawLibOpenGL::isExtensionSupported(std::string Ext) {
    const unsigned char *pcExtensions = NULL;
    const unsigned char *pcStart;
    unsigned char *pcWhere,*pcTerminator;
    
    pcExtensions = glGetString(GL_EXTENSIONS);
    if(pcExtensions == NULL) {
      Log("Failed to determine OpenGL extensions. Try stopping all other\n"
          "applications that might use your OpenGL hardware.\n"
          "If it still doesn't work, please create a detailed bug report.\n"
          );
      throw Exception("glGetString() : NULL");
    }
    
    pcStart = pcExtensions;
    while(1) {
      pcWhere = (unsigned char *)strstr((const char*)pcExtensions,Ext.c_str());
      if(pcWhere == NULL) break;
      pcTerminator = pcWhere + Ext.length();
      if(pcWhere == pcStart || *(pcWhere-1) == ' ')
        if(*pcTerminator == ' ' || *pcTerminator == '\0')
          return true;
      pcStart = pcTerminator;
    }
    return false;
  }
  
    /*===========================================================================
  Grab screen contents
  ===========================================================================*/
  Img *DrawLibOpenGL::grabScreen(void) {
    Img *pImg = new Img;
    
    pImg->createEmpty(m_nDispWidth,m_nDispHeight);
    Color *pPixels = pImg->getPixels();
    unsigned char *pcTemp = new unsigned char [m_nDispWidth*3];
  
    /* Select frontbuffer */
    glReadBuffer(GL_FRONT);

    /* Read the pixels (reversed) */
    for(int i=0;i<m_nDispHeight;i++) {          
      glReadPixels(0,i,m_nDispWidth,1,GL_RGB,GL_UNSIGNED_BYTE,pcTemp);
      for(int j=0;j<m_nDispWidth;j++) {
        pPixels[(m_nDispHeight - i - 1)*m_nDispWidth + j] = MAKE_COLOR(
          pcTemp[j*3],pcTemp[j*3+1],pcTemp[j*3+2],255
        );
      }
    }           
    
    delete [] pcTemp;
    return pImg;            
  }
  
  void DrawLibOpenGL::startDraw(DrawMode mode){
	  switch(mode){
		  case DRAW_MODE_POLYGON:
		    glBegin(GL_POLYGON);
		    break;
		  case DRAW_MODE_LINE_LOOP:
		    glBegin(GL_LINE_LOOP);
		    break;
		  case DRAW_MODE_LINE_STRIP:
		   glBegin(GL_LINE_STRIP);
		   break;
	  };
  }
  
  void DrawLibOpenGL::endDraw(){
	glEnd();
        if (m_texture != NULL){
          glDisable(GL_TEXTURE_2D);
          m_texture = NULL;
        }
	if (m_blendMode != BLEND_MODE_NONE){
         glDisable(GL_BLEND);
	}
  }

  void DrawLibOpenGL::setColor(Color color){
    glColor4ub(GET_RED(color),GET_GREEN(color),GET_BLUE(color),GET_ALPHA(color));
  }

  void DrawLibOpenGL::setTexture(Texture *texture,BlendMode blendMode){
    setBlendMode(blendMode);
    if (texture != NULL){
      /* bind texture only if different than the current one */
      if(m_texture == NULL || texture->Name != m_texture->Name){
	glBindTexture(GL_TEXTURE_2D,texture->nID);
      }
      glEnable(GL_TEXTURE_2D);
    } else {
       //so the texture is set to null
       //if the texture was not null we need
       //to disable the current texture
       if (m_texture != NULL){
         glDisable(GL_TEXTURE_2D);
       }
    }
    m_texture = texture;
  } 
  
  void DrawLibOpenGL::setBlendMode(BlendMode blendMode){
    if (blendMode != BLEND_MODE_NONE){
      glEnable(GL_BLEND);
      if (blendMode == BLEND_MODE_A){
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      } else {
        glBlendFunc(GL_ONE,GL_ONE);
      }
    } else {
       if (m_blendMode != NULL){
         glDisable(GL_BLEND);
       }
    }
    m_blendMode = blendMode;
  }

  /*===========================================================================
  Text dim probing
  ===========================================================================*/  
  int DrawLibOpenGL::getTextHeight(std::string Text) {
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
  
  int DrawLibOpenGL::getTextWidth(std::string Text) {
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
  void DrawLibOpenGL::drawText(const Vector2f &Pos,std::string Text,Color Back,Color Front,bool bEdge) {
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
  void DrawLibOpenGL::_InitTextRendering(Theme *p_theme) {   
    m_pDefaultFontTex = p_theme->getDefaultFont();
          
    /* Create font texture (default) */
    //m_pDefaultFontTexture = (DefaultFontTexture *)pTextureManager->loadTexture(new DefaultFontTexture,"default-font");
  }
  
  /*===========================================================================
  Uninit of text rendering
  ===========================================================================*/  
  void DrawLibOpenGL::_UninitTextRendering(Theme *p_theme) {    
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
      

      void DrawLibOpenGL::clearGraphics(){
           /* Clear screen */
          glClear(GL_COLOR_BUFFER_BIT);
      }
      
      /**
       * Flush the graphics. In memory graphics will now be displayed
       **/
      void DrawLibOpenGL::flushGraphics(){
           /* Swap buffers */
          SDL_GL_SwapBuffers();
      }

  FontManager* DrawLibOpenGL::getFontManager(const std::string &i_fontFile, int i_fontSize) {
    return new GLFontManager(this, i_fontFile, i_fontSize);
  }


GLFontGlyph::GLFontGlyph(const std::string& i_value, TTF_Font* i_ttf) {
  SDL_Surface* v_surf;
  SDL_Surface* v_image;
  SDL_Rect v_area;
  SDL_Color v_color = {0xFF, 0xFF, 0xFF, 0x00};

  m_value = i_value;
  m_drawX = 0;
  m_drawY = 0;

  v_surf = TTF_RenderUTF8_Blended(i_ttf, i_value.c_str(), v_color);
  if (v_surf == NULL) {
    throw Exception("GLFontGlyph: " + std::string(TTF_GetError()));
  }

  TTF_SizeUTF8(i_ttf, i_value.c_str(), &m_realWidth, &m_realHeight);
  m_drawWidth  = powerOf2(m_realWidth);
  m_drawHeight = powerOf2(m_realHeight);

  v_image = SDL_CreateRGBSurface(SDL_SWSURFACE,
				 m_drawWidth, m_drawHeight,
				 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
				 0x000000FF, 
				 0x0000FF00, 
				 0x00FF0000, 
				 0xFF000000
#else
				 0xFF000000,
				 0x00FF0000, 
				 0x0000FF00, 
				 0x000000FF
#endif
				 );
  if(v_image == NULL) {
    SDL_FreeSurface(v_surf);
    throw Exception("SDL_CreateRGBSurface failed");
  }

  SDL_SetAlpha(v_surf, 0, 0);

  /* Copy the surface into the GL texture image */
  v_area.x = 0;
  v_area.y = 0;
  v_area.w = m_drawWidth;
  v_area.h = m_drawHeight;
  SDL_BlitSurface(v_surf, &v_area, v_image, &v_area);
  SDL_FreeSurface(v_surf);

  /* Create the OpenGL texture */
  glGenTextures(1, &m_GLID);
  glBindTexture(GL_TEXTURE_2D, m_GLID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,
	       0,
	       GL_RGBA,
	       m_drawWidth, m_drawHeight,
	       0,
	       GL_RGBA,
	       GL_UNSIGNED_BYTE,
	       v_image->pixels);
  SDL_FreeSurface(v_image);
}

GLFontGlyph::~GLFontGlyph() {
 glDeleteTextures(1, &m_GLID);
}

std::string GLFontGlyph::Value() const {
  return m_value;
}

GLuint GLFontGlyph::GLID() const {
  return m_GLID;
}

int GLFontGlyph::drawX() const {
  return m_drawX;
}

int GLFontGlyph::drawY() const {
  return m_drawY;
}

int GLFontGlyph::drawWidth() const {
  return m_drawWidth;
}

int GLFontGlyph::drawHeight() const {
  return m_drawHeight;
}

int GLFontGlyph::realWidth() const {
  return m_realWidth;
}

int GLFontGlyph::realHeight() const {
  return m_realHeight;
}

FontManager::FontManager(DrawLib* i_drawLib, const std::string &i_fontFile, int i_fontSize) {
  m_drawLib = i_drawLib;
  m_ttf = TTF_OpenFont(i_fontFile.c_str(), i_fontSize);
  if (m_ttf == NULL) {
    throw Exception("FontManager: " + std::string(TTF_GetError()));
  }
  TTF_SetFontStyle(m_ttf, TTF_STYLE_NORMAL);
}

FontManager::~FontManager() {
  TTF_CloseFont(m_ttf);
}

GLFontManager::GLFontManager(DrawLib* i_drawLib, const std::string &i_fontFile, int i_fontSize)
  : FontManager(i_drawLib, i_fontFile, i_fontSize) {

}

GLFontManager::~GLFontManager() {
  for(unsigned int i=0; i<m_glyphs.size(); i++) {
    delete m_glyphs[i];
  }
}

int GLFontGlyph::powerOf2(int i_value) {
  int v_value = 1;

  while (v_value < i_value) {
    v_value <<= 1;
  }
  return v_value;
}

FontGlyph* GLFontManager::getGlyph(const std::string& i_string) {
  GLFontGlyph *v_glyph;

  for(unsigned int i=0; i<m_glyphs.size(); i++) {
    if(i_string == m_glyphs[i]->Value()) {
      return m_glyphs[i];
    }
  }

  v_glyph = new GLFontGlyph(i_string, m_ttf);
  m_glyphs.push_back(v_glyph);
  return v_glyph;
}

void GLFontManager::printString(FontGlyph* i_glyph, int i_x, int i_y, Color i_color) {
  GLFontGlyph* v_glyph = (GLFontGlyph*) i_glyph;

  try {
    /* draw the glyph */
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, v_glyph->GLID());
    glBegin(GL_TRIANGLE_STRIP);
    glColor4ub(GET_RED(i_color),GET_GREEN(i_color),GET_BLUE(i_color),GET_ALPHA(i_color));
    glTexCoord2f(0.0, 1.0);
    glVertex2i(i_x, i_y);
    glTexCoord2f(1.0, 1.0);
    glVertex2i(i_x + v_glyph->drawWidth(), i_y);
    glTexCoord2f(0.0, 0.0);
    glVertex2i(i_x, i_y + v_glyph->drawHeight());
    glTexCoord2f(1.0, 0.0);
    glVertex2i(i_x + v_glyph->drawWidth(), i_y + v_glyph->drawHeight());
    glEnd();
    glDisable(GL_TEXTURE_2D);
    /* */

  } catch(Exception &e) {
    /* ok, forget this one */
  }

}

}
#endif
