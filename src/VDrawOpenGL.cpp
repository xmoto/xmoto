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
#include "helpers/utf8.h"
#include "xmscene/Camera.h"
#include "helpers/Log.h"

#define UTF8_INTERLINE_SPACE 2
#define UTF8_INTERCHAR_SPACE 0

#ifdef ENABLE_OPENGL
namespace vapp {


class GLFontGlyphLetter;

class GLFontGlyph : public FontGlyph {
 public:
  /* for simple glyph */
  GLFontGlyph(const std::string& i_value);

  /* a glyph from other glyphs */
/*kejo:why not just grrr create a copy contructor*/
  GLFontGlyph(const std::string& i_value,
	      HashNamespace::hash_map<const char*, GLFontGlyphLetter*, HashNamespace::hash<const char*>, hashcmp_str>& i_glyphsLetters);
  virtual ~GLFontGlyph();

  std::string Value() const;
  int drawWidth()     const;
  int drawHeight()    const;
  int realWidth()     const;
  int realHeight()    const;
  int firstLineDrawHeight() const;

 protected:
  std::string m_value;
  int m_drawWidth, m_drawHeight;
  int m_realWidth, m_realHeight;
  int m_firstLineDrawHeight;

  static int powerOf2(int i_value);
};

class GLFontGlyphLetter : public GLFontGlyph {
 public:
  GLFontGlyphLetter(const std::string& i_value, TTF_Font* i_ttf);
  virtual ~GLFontGlyphLetter();
  GLuint GLID() const;  

 private:
  GLuint m_GLID;  
};


class GLFontManager : public FontManager {
 public:
  GLFontManager(DrawLib* i_drawLib, const std::string &i_fontFile, int i_fontSize);
  virtual ~GLFontManager();

  FontGlyph* getGlyph(const std::string& i_string);
  void printString(FontGlyph* i_glyph, int i_x, int i_y, Color i_color);
  void printStringGrad(FontGlyph* i_glyph, int i_x, int i_y,
		       Color c1, Color c2, Color c3, Color c4);
  virtual int nbGlyphsInMemory();

 private:
  std::vector<GLFontGlyph*> m_glyphsList;
  HashNamespace::hash_map<const char*, GLFontGlyph*, HashNamespace::hash<const char*>, hashcmp_str> m_glyphs;

  std::vector<GLFontGlyphLetter*> m_glyphsLettersList;
  HashNamespace::hash_map<const char*, GLFontGlyphLetter*, HashNamespace::hash<const char*>, hashcmp_str> m_glyphsLetters;
};




 DrawLibOpenGL::~DrawLibOpenGL(){
   if(m_fontSmall != NULL) {
     delete m_fontSmall;
   }

   if(m_fontMedium != NULL) {
     delete m_fontMedium;
   }

   if(m_fontBig != NULL) {
     delete m_fontBig;
   }
 }

 DrawLibOpenGL::DrawLibOpenGL() : DrawLib(){
   m_fontSmall  = getFontManager(FS::FullPath(DRAW_FONT_FILE), 14);
   m_fontMedium = getFontManager(FS::FullPath(DRAW_FONT_FILE), 22);
   m_fontBig    = getFontManager(FS::FullPath(DRAW_FONT_FILE), 60);
 };
 
  /*===========================================================================
  Transform an OpenGL vertex to pure 2D 
  ===========================================================================*/
  void DrawLibOpenGL::glVertexSP(float x,float y) {
	  glVertex2f(x, m_nActualHeight - y);
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

  void DrawLibOpenGL::setMirrorY() {
    glRotatef(180, 0, 1, 0);
  }

  void DrawLibOpenGL::setRotateZ(float i_angle) {
    if(i_angle != 0.0) { /* not nice to compare a float, but the main case */
      glRotatef(i_angle, 0, 0, 1);
    }
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
      Logger::Log("** Warning ** : Tried to set video mode %ix%i @ %i-bit, but SDL failed (%s)\n"
		  "                Now SDL will try determining a proper mode itself.", m_nDispWidth, m_nDispHeight, m_nDispBPP, SDL_GetError());
      m_nDispBPP = 0;

      /* Hmm, try letting it decide the BPP automatically */
      if((m_screen=SDL_SetVideoMode(m_nDispWidth,m_nDispHeight,0,nFlags))==NULL) {       
        /* Still no luck */
        Logger::Log("** Warning ** : Still no luck, now we'll try 800x600 in a window.");
        m_nDispWidth = 800; m_nDispHeight = 600; m_nDispBPP = 0;       
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
  
		m_menuCamera = new Camera(Vector2i(0,0),
															Vector2i(m_nDispWidth,m_nDispHeight));
		m_menuCamera->setCamera2d();
    
    glClearDepth(1);
    glDepthFunc(GL_LEQUAL);
    
    /* Enable unicode translation and key repeats */
    SDL_EnableUNICODE(1);         
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
     
    /* Output some general info */
    Logger:: Log("GL: %s (%s)",glGetString(GL_RENDERER),glGetString(GL_VENDOR));
    if(glGetString(GL_RENDERER) == NULL || 
       glGetString(GL_VENDOR) == NULL) {
      Logger::Log("** Warning ** : GL strings NULL!");
      throw Exception("GL strings are NULL!");
    }
    
    /* Windows: check whether we are using the standard GDI OpenGL software driver... If
       so make sure the user is warned */
    #if defined(WIN32) 
      if(!strcmp(reinterpret_cast<const char *>(glGetString(GL_RENDERER)),"GDI Generic") &&
         !strcmp(reinterpret_cast<const char *>(glGetString(GL_VENDOR)),"Microsoft Corporation")) {
        Logger::Log("** Warning ** : No GL hardware acceleration!");
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
          
      Logger::Log("GL: using ARB_vertex_buffer_object");    
    }
    else
      Logger::Log("GL: not using ARB_vertex_buffer_object");    
      
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
          
      Logger::Log("GL: using EXT_framebuffer_object");
    }
    else
      Logger::Log("GL: not using EXT_framebuffer_object");
      
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
        
      Logger::Log("GL: using ARB_fragment_shader/ARB_vertex_shader/ARB_shader_objects");
    }
    else
      Logger::Log("GL: not using ARB_fragment_shader/ARB_vertex_shader/ARB_shader_objects");
    
    /* Set background color to black */
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapBuffers();  
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
      Logger::Log("Failed to determine OpenGL extensions. Try stopping all other\n"
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

GLFontGlyphLetter::GLFontGlyphLetter(const std::string& i_value, TTF_Font* i_ttf)
 : GLFontGlyph(i_value) {
  SDL_Surface* v_surf;
  SDL_Surface* v_image;
  SDL_Rect v_area;
  SDL_Color v_color = {0xFF, 0xFF, 0xFF, 0x00};

  v_surf = TTF_RenderUTF8_Blended(i_ttf, i_value.c_str(), v_color);
  if (v_surf == NULL) {
    throw Exception("GLFontGlyphLetter: " + std::string(TTF_GetError()));
  }

  TTF_SizeUTF8(i_ttf, i_value.c_str(), &m_realWidth, &m_realHeight);

  m_drawWidth  = powerOf2(m_realWidth);
  m_drawHeight = powerOf2(m_realHeight);
  m_firstLineDrawHeight = m_drawHeight;

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

  /* Copy the surface into the GL texture image */
  SDL_SetAlpha(v_surf, 0, 0);
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
  
GLFontGlyphLetter::~GLFontGlyphLetter() {
  glDeleteTextures(1, &m_GLID);
}

GLuint GLFontGlyphLetter::GLID() const {
  return m_GLID;
}

GLFontGlyph::GLFontGlyph(const std::string& i_value) {
  m_value = i_value;
  m_drawWidth = m_drawHeight = 0;
  m_realWidth = m_realHeight = 0;
  m_firstLineDrawHeight = 0;
}

GLFontGlyph::GLFontGlyph(const std::string& i_value,
			 HashNamespace::hash_map<const char*, GLFontGlyphLetter*, HashNamespace::hash<const char*>, hashcmp_str>& i_glyphsLetters) {
  GLFontGlyph* v_glyph;
  std::string v_char;

  m_value = i_value;
  m_realWidth = m_realHeight = 0;

  if(i_value == "") return; /* do nothing for empty strings */

  int n=0;
  int v_maxHeight = 0;
  int v_curWidth = 0;
  bool v_firstLineInitialized = false;
  while(n < i_value.size()) {
    v_char = utf8::getNextChar(i_value, n);
    if(v_char == "\n") {
      if(v_firstLineInitialized == false) {	
	m_firstLineDrawHeight = v_maxHeight;
	v_firstLineInitialized = true;
      }
      m_realHeight += UTF8_INTERLINE_SPACE + v_maxHeight;
      v_maxHeight = 0;
      v_curWidth  = 0;
    } else {
      v_glyph = i_glyphsLetters[v_char.c_str()];
      if(v_glyph != NULL) {
	if(v_glyph->realHeight() > v_maxHeight) v_maxHeight = v_glyph->realHeight();
	if(v_curWidth != 0) v_curWidth += UTF8_INTERCHAR_SPACE;
	v_curWidth += v_glyph->realWidth();
	if(v_curWidth > m_realWidth) m_realWidth = v_curWidth;
      }
    }
  }
  /* last line */
  if(v_char != "\n") m_realHeight += v_maxHeight;
  if(v_firstLineInitialized == false) {	
    m_firstLineDrawHeight = v_maxHeight;
  }

  m_drawWidth  = powerOf2(m_realWidth);
  m_drawHeight = powerOf2(m_realHeight);
  m_firstLineDrawHeight = powerOf2(m_firstLineDrawHeight);
}

GLFontGlyph::~GLFontGlyph() {
}

std::string GLFontGlyph::Value() const {
  return m_value;
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

int GLFontGlyph::firstLineDrawHeight() const {
  return m_firstLineDrawHeight;
}



GLFontManager::GLFontManager(DrawLib* i_drawLib, const std::string &i_fontFile, int i_fontSize)
  : FontManager(i_drawLib, i_fontFile, i_fontSize) {
}

int GLFontManager::nbGlyphsInMemory() {
  return m_glyphsLettersList.size();
}

GLFontManager::~GLFontManager() {
  for(unsigned int i=0; i<m_glyphsList.size(); i++) {
    delete m_glyphsList[i];
  }

  for(unsigned int i=0; i<m_glyphsLettersList.size(); i++) {
    delete m_glyphsLettersList[i];
  }

  /* i added the m_glyphsList because the iterator on the hashmap
     often produces segfault on delete it->second */
  //printf("BEGIN ~GLFontManager()\n");
  //HashNamespace::hash_map<const char*, GLFontGlyph*, HashNamespace::hash<const char*>, hashcmp_str, std::allocator<GLFontGlyph*> >::iterator it;
  //for (it = m_glyphs.begin(); it != m_glyphs.end(); it++) {
  //  delete it->second;
  //}
  //printf("END ~GLFontManager()\n");
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
  GLFontGlyphLetter *v_glyphLetter;

  v_glyph = m_glyphs[i_string.c_str()];
  if(v_glyph != NULL) return v_glyph;

  /* make sure that chars exists into the hashmap before continuing */
  int n = 0;
  std::string v_char;
  while(n < i_string.size()) {
    v_char = utf8::getNextChar(i_string, n);
    if(v_char != "\n") {
      if(m_glyphsLetters[v_char.c_str()] == NULL) {
	v_glyphLetter = new GLFontGlyphLetter(v_char, m_ttf);
	m_glyphsLetters[v_char.c_str()] = v_glyphLetter;
	m_glyphsLettersList.push_back(v_glyphLetter);
      }
    }
  }
  /* */

  v_glyph = new GLFontGlyph(i_string, m_glyphsLetters);
  
  m_glyphs[i_string.c_str()] = v_glyph;
  m_glyphsList.push_back(v_glyph);

  return v_glyph;
}

void GLFontManager::printStringGrad(FontGlyph* i_glyph, int i_x, int i_y,
      Color c1, Color c2, Color c3, Color c4) {

  GLFontGlyph* v_glyph = (GLFontGlyph*) i_glyph;
  GLFontGlyphLetter* v_glyphLetter;
  int v_x, v_y;
  int n = 0;
  std::string v_char;
  int v_lineHeight;

  if(v_glyph->Value() == "") return;

  v_y = -i_y + m_drawLib->getDispHeight() - v_glyph->firstLineDrawHeight();/* la taille de la 1ere ligne */
  v_x = i_x;
  v_lineHeight = 0;

  try {
    /* draw the glyph */
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while(n < v_glyph->Value().size()) {
      v_char = utf8::getNextChar(v_glyph->Value(), n);
      if(v_char == "\n") {
	v_x  = i_x;
	v_y -= v_lineHeight + UTF8_INTERLINE_SPACE;
	v_lineHeight = 0;
      } else {
	v_glyphLetter = m_glyphsLetters[v_char.c_str()];
	if(v_glyphLetter != NULL) {
	  if(v_glyphLetter->realHeight() > v_lineHeight) v_lineHeight = v_glyphLetter->realHeight();

	  glBindTexture(GL_TEXTURE_2D, v_glyphLetter->GLID());
	  glBegin(GL_TRIANGLE_STRIP);
	  glColor4ub(GET_RED(c1),GET_GREEN(c1),GET_BLUE(c1),GET_ALPHA(c1));
	  glTexCoord2f(0.0, 1.0);
	  glVertex2i(v_x, v_y);
	  glColor4ub(GET_RED(c2),GET_GREEN(c2),GET_BLUE(c2),GET_ALPHA(c2));
	  glTexCoord2f(1.0, 1.0);
	  glVertex2i(v_x + v_glyphLetter->drawWidth(), v_y);
	  glColor4ub(GET_RED(c3),GET_GREEN(c3),GET_BLUE(c3),GET_ALPHA(c3));
	  glTexCoord2f(0.0, 0.0);
	  glVertex2i(v_x, v_y + v_glyphLetter->drawHeight());
	  glColor4ub(GET_RED(c4),GET_GREEN(c4),GET_BLUE(c4),GET_ALPHA(c4));
	  glTexCoord2f(1.0, 0.0);
	  glVertex2i(v_x + v_glyphLetter->drawWidth(), v_y + v_glyphLetter->drawHeight());
	  v_x += v_glyphLetter->realWidth() + UTF8_INTERCHAR_SPACE;
	  glEnd();
	}
      }
    }
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    /* */

  } catch(Exception &e) {
    /* ok, forget this one */
  }
}

void GLFontManager::printString(FontGlyph* i_glyph, int i_x, int i_y, Color i_color) {
  printStringGrad(i_glyph, i_x, i_y, i_color, i_color, i_color, i_color);
}

}
#endif
