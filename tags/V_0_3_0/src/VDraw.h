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
#include "PolyDraw.h"

#include <SDL/SDL_ttf.h>

#ifdef __GNUC__
#if (__GNUC__ >= 3)
#include <ext/hash_map>
  namespace HashNamespace=__gnu_cxx;
#else
#include <hash_map>
#define HashNamespace std
#endif
#else // #ifdef __GNUC__
#include <hash_map>
namespace HashNamespace=std;
#endif
struct hashcmp_str {
  bool operator()(std::string s1, std::string s2) {
    return s1 == s2;
  }
};

class Camera;

namespace vapp {

class DrawLib;
class GLFontGlyphLetter;

class FontGlyph {
  public:
  virtual int realWidth() const = 0;
  virtual int realHeight() const = 0;

  private:
};

class FontManager {
 public:
  FontManager(DrawLib* i_drawLib, const std::string &i_fontFile, int i_fontSize);
  virtual ~FontManager();

  virtual FontGlyph* getGlyph(const std::string& i_string) = 0;
  virtual void printString(FontGlyph* i_glyph, int i_x, int i_y, Color i_color) = 0;
  virtual void printStringGrad(FontGlyph* i_glyph, int i_x, int i_y,
			       Color c1,Color c2,Color c3,Color c4) = 0;
  virtual int nbGlyphsInMemory();

 protected:
  TTF_Font* m_ttf;
  DrawLib* m_drawLib;
};

#ifdef ENABLE_OPENGL
class GLFontGlyph : public FontGlyph {
 public:
  /* for simple glyph */
  GLFontGlyph(const std::string& i_value);

  /* a glyph from other glyphs */
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

#endif

  /**
   * VApp draw modes to be used as argument in startDraw
   * DRAW_MODE_POLYGON is a filled polygon drawing mode.
   * If a texture was given this texture will be used as filling
   * DRAW_MODE_LINE_LOOP is a polygon drawing mode (non filled)
   * DRAW_MODE_LINE_STRIP draws lines 
   **/
  enum DrawMode {
    DRAW_MODE_NONE,
    DRAW_MODE_POLYGON,
    DRAW_MODE_LINE_LOOP,
    DRAW_MODE_LINE_STRIP
  };

  /**
   * Definition of the blending mode
   **/
  enum BlendMode {
    BLEND_MODE_NONE,		//no blending
    BLEND_MODE_A,		//GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA
    BLEND_MODE_B		//GL_ONE,GL_ONE
  };

  /*===========================================================================
  Class with various drawing functions
  ===========================================================================*/
  class DrawLib {
  public:
    DrawLib();
    virtual ~DrawLib();

    /* initialize a drawLib from a name */
    static DrawLib* DrawLibFromName(std::string i_drawLibName);

    /* Rendering is different depending on the backend. */
    /* it's bad design issue, for sure, but we're talking about performance here !*/
    typedef enum {backend_None, backend_OpenGl, backend_SdlGFX} backendtype;
    static backendtype getBackend() {
      return m_backend;
    }

    /**
   * initialize the screen
    **/
    virtual void init(int nDispWidth, int nDispHeight, int nDispBPP,
		      bool bWindowed, Theme * ptheme) = 0;

    virtual void unInit() = 0;

    void setDispWidth(int width) {
      m_nDispWidth = width;
    };

    int getDispWidth(void) {
      return m_nDispWidth;
    };

    void setNoGraphics(bool disable_graphics) {
      m_bNoGraphics = disable_graphics;
    };

    bool isNoGraphics() {
      return m_bNoGraphics;
    };

    void setDispHeight(int height) {
      m_nDispHeight = height;
    };
    int getDispHeight(void) {
      return m_nDispHeight;
    }
    void setDispBPP(int bpp) {
      m_nDispBPP = bpp;
    }
    int getDispBPP(void) {
      return m_nDispBPP;
    }

    void setWindowed(bool windowed) {
      m_bWindowed = windowed;
    };
    bool getWindowed(void) {
      return m_bWindowed;
    }

    /* Methods - low-level */
    //add a vertex given screen coordinates
    virtual void glVertexSP(float x, float y) = 0;

    //add a vertex given opengl coordinates
    virtual void glVertex(float x, float y) = 0;

    //create a vertex based on a opengl 
    void glVertex(Vector2f x) {
      glVertex(x.x, x.y);
    };

    //texture coordinate
    virtual void glTexCoord(float x, float y) = 0;
    virtual void screenProjVertex(float *x, float *y) = 0;

    virtual void setColor(Color color) = 0;

      /**
       * set the texture for drawing
       * the value may be NULL to disable texture
       * every end draw will reset the texture to NULL
       **/
    virtual void setTexture(Texture * texture, BlendMode blendMode) = 0;
    virtual void setBlendMode(BlendMode blendMode) = 0;
    void setColorRGB(int r, int g, int b) {
      setColor(MAKE_COLOR(r, g, b, 255));
    };

    void setColorRGBA(int r, int g, int b, int a) {
      setColor(MAKE_COLOR(r, g, b, a));
    };

      /**
       * enables clipping and sets the clipping borders
       **/
    virtual void setClipRect(int x, int y, int w, int h) = 0;
    virtual void setClipRect(SDL_Rect * i_clip_rect) = 0;
    virtual void setScale(float x, float y) = 0;
    virtual void setTranslate(float x, float y) = 0;
    virtual void setMirrorY() = 0;
    virtual void setRotateZ(float i_angle) = 0;
    virtual void setLineWidth(float width) = 0;

      /**
       * returns the current screen clipping
       **/
    virtual void getClipRect(int *o_px, int *o_py, int *o_pnWidth,
			     int *o_pnHeight) = 0;

      /**
       * Start drawing ... used in combination with glVertex
       **/
    virtual void startDraw(DrawMode mode) = 0;

      /**
       * End draw
       **/
    virtual void endDraw() = 0;

      /**
       * Clears the screen with the configured background
       **/
    virtual void clearGraphics() = 0;

    /**
      * resets the state of the zoom and translation
     **/
    virtual void resetGraphics() {
    };

      /**
       * Flush the graphics. In memory graphics will now be displayed
       **/
    virtual void flushGraphics() = 0;

    /* Methods - primitives */
    virtual void drawCircle(const Vector2f & Center, float fRadius,
			    float fBorder = 1.0f, Color Back =
			    0, Color Front = -1);
    virtual void drawBox(const Vector2f & A, const Vector2f & B,
			 float fBorder = 1.0f, Color Back =
			 0, Color Front = -1);
    virtual void drawImage(const Vector2f & a, const Vector2f & b,
			   Texture * pTexture, Color Tint = -1);

    void setDontUseGLExtensions(bool dont_use) {
      m_bDontUseGLExtensions = dont_use;
    };
    virtual Img *grabScreen(void) = 0;

    /*
     * set the reference drawing size
     **/
    void setDrawDims(int nActualW, int nActualH, int w, int h) {
      m_nDrawWidth = w;
      m_nDrawHeight = h;
      m_nActualWidth = nActualW;
      m_nActualHeight = nActualH;
    }
//opengl specific to be removed
    bool useVBOs() {
      return m_bVBOSupported;
    };
    bool useFBOs() {
      return m_bFBOSupported;
    };
    bool useShaders() {
      return m_bShadersSupported;
    };

    /* more open specific */
    /* handle display lists */
    
    void toogleFullscreen();
    FontManager* getFontSmall();
    FontManager* getFontMedium();
    FontManager* getFontBig();
    virtual FontManager* getFontManager(const std::string &i_fontFile, int i_fontSize);
    
    /* Data */
    int m_nDrawWidth, m_nDrawHeight;
    int m_nActualWidth, m_nActualHeight;
    int m_nDispWidth, m_nDispHeight, m_nDispBPP;	/* Screen stuff */
    int m_nLScissorX, m_nLScissorY, m_nLScissorW, m_nLScissorH;

    FontManager* m_fontSmall;
    FontManager* m_fontMedium;
    FontManager* m_fontBig;

    bool m_bWindowed;		/* Windowed or not */

    virtual bool isExtensionSupported(std::string Ext) = 0;
    Texture *m_texture;
    BlendMode m_blendMode;

    bool m_bVBOSupported;
    bool m_bFBOSupported;
    bool m_bShadersSupported;
    bool m_bDontUseGLExtensions;
    bool m_bNoGraphics;		/* No-graphics mode */

		Camera* getMenuCamera(){
			return m_menuCamera;
		}
  protected:
    SDL_Surface *m_screen;
		Camera* m_menuCamera;

  private:
    static backendtype m_backend;
  };

  class DrawLibOpenGL:public DrawLib {
  public:
    DrawLibOpenGL();
    virtual ~DrawLibOpenGL();

    virtual void init(int nDispWidth, int nDispHeight, int nDispBPP,
											bool bWindowed, Theme * ptheme);
    virtual void unInit();
    /* Methods - low-level */
    //add a vertex given screen coordinates
    virtual void glVertexSP(float x, float y);
    //add a vertex given opengl coordinates
    virtual void glVertex(float x, float y);
    //texture coordinate
    virtual void glTexCoord(float x, float y);
    virtual void screenProjVertex(float *x, float *y);

    virtual void setColor(Color color);
      /**
       * set the texture for drawing
       * the value may be NULL to disable texture
       * every end draw will reset the texture to NULL
       **/
    virtual void setTexture(Texture * texture, BlendMode blendMode);
    virtual void setBlendMode(BlendMode blendMode);

      /**
       * enables clipping and sets the clipping borders
       **/
    virtual void setClipRect(int x, int y, int w, int h);
    virtual void setClipRect(SDL_Rect * i_clip_rect);
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

      /**
       * Clears the screen with the configured background
       **/
    virtual void clearGraphics();

      /**
       * Flush the graphics. In memory graphics will now be displayed
       **/
    virtual void flushGraphics();

    virtual FontManager* getFontManager(const std::string &i_fontFile, int i_fontSize);

    virtual Img *grabScreen(void);
    virtual bool isExtensionSupported(std::string Ext);
      /**
       *keesj:TODO
       *I am not happy about all these public members:)
       **/
#ifdef ENABLE_OPENGL
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
#endif

  };

  class DrawLibSDLgfx:public DrawLib {
  public:
    DrawLibSDLgfx();
    virtual ~DrawLibSDLgfx();

    virtual void init(int nDispWidth, int nDispHeight, int nDispBPP,
		      bool bWindowed, Theme * ptheme);
    virtual void unInit();
    /* Methods - low-level */
    //add a vertex given screen coordinates
    virtual void glVertexSP(float x, float y);
    //add a vertex given opengl coordinates
    virtual void glVertex(float x, float y);
    //texture coordinate
    virtual void glTexCoord(float x, float y);
    virtual void screenProjVertex(float *x, float *y);

    virtual void setColor(Color color);
      /**
       * set the texture for drawing
       * the value may be NULL to disable texture
       * every end draw will reset the texture to NULL
       **/
    virtual void setTexture(Texture * texture, BlendMode blendMode);
    virtual void setBlendMode(BlendMode blendMode);

      /**
       * enables clipping and sets the clipping borders
       **/
    virtual void setClipRect(int x, int y, int w, int h);
    virtual void setClipRect(SDL_Rect * i_clip_rect);
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

      /**
       * Clears the screen with the configured background
       **/
    virtual void clearGraphics();
      /**
       * resets the variables like translation and rotation
       **/
    virtual void resetGraphics();

      /**
       * Flush the graphics. In memory graphics will now be displayed
       **/
    virtual void flushGraphics();

    virtual Img *grabScreen(void);
    virtual bool isExtensionSupported(std::string Ext);
  private:
    int xx_texturedHLine(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y,
			 SDL_Surface * texture, int texture_dx,
			 int texture_dy);
    int xx_texturedPolygon(SDL_Surface * dst, const Sint16 * vx,
			   const Sint16 * vy, int n, SDL_Surface * texture,
			   int texture_dx, int texture_dy);

    int xx_texturedHLineAlpha(SDL_Surface * dst, Sint16 x1, Sint16 x2,
			      Sint16 y, SDL_Surface * texture,
			      int texture_dx, int texture_dy);
    int xx_texturedPolygonAlpha(SDL_Surface * dst, const Sint16 * vx,
				const Sint16 * vy, int n,
				SDL_Surface * texture, int texture_dx,
				int texture_dy);

    int xx_filledPolygonColor(SDL_Surface * dst, const Sint16 * vx,
			      const Sint16 * vy, int n, Uint32 color);

    //the mode used when drawing
    DrawMode m_drawMode;

    //the current scale
    Vector2f m_scale;

    //the current translate
    Vector2f m_translate;

    //data buffer for drawing the background
    void *m_bg_data;

    Vector2f m_min;
    Vector2f m_max;

    //
    Vector2f m_textureAnchorPoint;
    Texture *m_texture;

      /**
       * Color used to clear the screen
       **/
    Color m_backgroundColor;
      /**
       * color used for the current drawing
       **/
    Color m_color;

    //Vector for creating polygons
     std::vector < Vector2f * >m_drawingPoints;
    //Vector for keeping track of texture coordinates
     std::vector < Vector2f * >m_texturePoints;

    //map to hold converted images
     std::map < const char *, SDL_Surface *, ltstr > m_image_cache;

    //dynamic allocated array for the 
    //that that is fed to the polygon filling
    //algorithms
    Sint16 m_int_drawing_points_x[100];
    Sint16 m_int_drawing_points_y[100];

    int *gfxPrimitivesPolyInts;
    int gfxPrimitivesPolyAllocated;
    PolyDraw * polyDraw;
    int screenVerticles[100];
    int nPolyTextureVertices[100];

  };
};

#endif
