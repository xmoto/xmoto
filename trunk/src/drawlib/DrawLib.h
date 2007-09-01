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

#ifndef __DRAWLIB_H__
#define __DRAWLIB_H__

#ifdef HAVE_SDL_FRAMEWORK
  #include <SDL_ttf.h>
#else
  #include <SDL/SDL_ttf.h>
#endif

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

#include "helpers/VMath.h"

class Img;
class Camera;
class DrawLib;
class Theme;
class Texture;

class FontGlyph {
 public:
  virtual ~FontGlyph() {};
  virtual unsigned int realWidth() const = 0;
  virtual unsigned int realHeight() const = 0;
};

class FontManager {
 public:
  FontManager(DrawLib* i_drawLib, const std::string &i_fontFile, unsigned int i_fontSize);
  virtual ~FontManager();
  
  virtual FontGlyph* getGlyph(const std::string& i_string) = 0;
  
  virtual void printString(FontGlyph* i_glyph, int i_x, int i_y, Color i_color, bool i_shadowEffect = false) = 0;
  virtual void printStringGrad(FontGlyph* i_glyph, int i_x, int i_y,
			       Color c1,Color c2,Color c3,Color c4, bool i_shadowEffect = false) = 0;
  

  static std::string getDrawFontFile();

 protected:
  TTF_Font* m_ttf;
  DrawLib* m_drawLib;
};


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
  /* this block should be removed as soon as possible */
  typedef enum {backend_None, backend_OpenGl, backend_SdlGFX} backendtype;
  static backendtype getBackend() {
    return m_backend;
  }
  /**/
  
  /**
   * initialize the screen
   **/
  virtual void init(unsigned int nDispWidth, unsigned int nDispHeight, unsigned int nDispBPP,
		    bool bWindowed, Theme * ptheme) = 0;
  
  virtual void unInit() = 0;
  
  void setDispWidth(unsigned int width);
  unsigned int getDispWidth();
  void setDispHeight(unsigned int height);
  unsigned int getDispHeight(void);
  void setDispBPP(unsigned int bpp);
  unsigned int getDispBPP();
  void setWindowed(bool windowed);
  bool getWindowed(void);
  void setNoGraphics(bool disable_graphics);  
  bool isNoGraphics();

  /* Methods - low-level */
  //add a vertex given screen coordinates
  virtual void glVertexSP(float x, float y) = 0;

  //add a vertex given opengl coordinates
  virtual void glVertex(float x, float y) = 0;

  //create a vertex based
  void glVertex(Vector2f x);

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
  void setColorRGB(unsigned int r, unsigned int g, unsigned int b);
  void setColorRGBA(unsigned int r, unsigned int g, unsigned int b, unsigned int a);

  /**
   * enables clipping and sets the clipping borders
   **/
  virtual void setClipRect(int x, int y, unsigned int w, unsigned int h) = 0;
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
  virtual void endDrawKeepProperties() = 0; /* to keep textures, ... to render several times the same entity fastly */
  virtual void removePropertiesAfterEnd() = 0; /* remove properties endDraw = endDrawKeepProperties + removePropertiesAfterEnd */

  /**
   * Clears the screen with the configured background
   **/
  virtual void clearGraphics() = 0;

  /**
   * resets the state of the zoom and translation
   **/
  virtual void resetGraphics();

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
  
  virtual bool isExtensionSupported(std::string Ext) = 0;
  void setDontUseGLExtensions(bool dont_use);
  virtual Img *grabScreen() = 0;

  /*
   * set the reference drawing size
   **/
  void setDrawDims(unsigned int nActualW, unsigned int nActualH, unsigned int w, unsigned int h);
  bool useVBOs();
  bool useFBOs();
  bool useShaders();

  /* more open specific */
  /* handle display lists */
  void toogleFullscreen();

  FontManager* getFontSmall();
  FontManager* getFontMedium();
  FontManager* getFontBig();
  virtual FontManager* getFontManager(const std::string &i_fontFile, unsigned int i_fontSize);

  Camera* getMenuCamera();

 protected:    
  unsigned int m_nDrawWidth, m_nDrawHeight;
  unsigned int m_nActualWidth, m_nActualHeight;
  unsigned int m_nDispWidth, m_nDispHeight, m_nDispBPP;	/* Screen stuff */
  unsigned int m_nLScissorX, m_nLScissorY, m_nLScissorW, m_nLScissorH;

  FontManager* m_fontSmall;
  FontManager* m_fontMedium;
  FontManager* m_fontBig;

  bool m_bWindowed;		/* Windowed or not */

  Texture *m_texture;
  BlendMode m_blendMode;

  bool m_bVBOSupported;
  bool m_bFBOSupported;
  bool m_bShadersSupported;
  bool m_bDontUseGLExtensions;
  bool m_bNoGraphics;		/* No-graphics mode */
  
  SDL_Surface *m_screen;
  Camera* m_menuCamera;

 private:
  static backendtype m_backend;
};

#endif
