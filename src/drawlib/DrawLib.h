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

#include "helpers/Color.h"
#include "helpers/RenderSurface.h"
#include "helpers/VMath.h"
#include "include/xm_SDL_ttf.h"
#include <vector>

class Img;
class Camera;
class DrawLib;
class Theme;
class Texture;
class RenderSurface;

class FontGlyph {
public:
  virtual ~FontGlyph() {};
  virtual unsigned int realWidth() const = 0;
  virtual unsigned int realHeight() const = 0;
};

class FontManager {
public:
  FontManager(DrawLib *i_drawLib,
              const std::string &i_fontFile,
              unsigned int i_fontSize,
              unsigned int i_fixedFontSize = 0 /* if > 0, force width */);
  virtual ~FontManager();

  virtual FontGlyph *getGlyph(const std::string &i_string) = 0;
  virtual FontGlyph *getGlyphTabExtended(
    const std::string &i_string) = 0; // extends \t

  virtual void printString(DrawLib *pDrawLib,
                           FontGlyph *i_glyph,
                           int i_x,
                           int i_y,
                           Color i_color,
                           float i_perCentered = -1.0,
                           bool i_shadowEffect = false) = 0;
  virtual void printStringGrad(DrawLib *pDrawLib,
                               FontGlyph *i_glyph,
                               int i_x,
                               int i_y,
                               Color c1,
                               Color c2,
                               Color c3,
                               Color c4,
                               float i_perCentered = -1.0,
                               bool i_shadowEffect = false) = 0;

  static std::string getDrawFontFile();
  static std::string getMonospaceFontFile();

  virtual void displayScrap(DrawLib *pDrawLib) = 0;

protected:
  TTF_Font *m_ttf;
  DrawLib *m_drawLib;
  unsigned int m_fixedFontSize;
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
  BLEND_MODE_NONE, // no blending
  BLEND_MODE_A, // GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA
  BLEND_MODE_B // GL_ONE,GL_ONE
};

/**
 * Dimensionality of the camera
 **/
enum CameraDimension { CAMERA_2D, CAMERA_3D };

/*===========================================================================
  Class with various drawing functions
  ===========================================================================*/
class DrawLib {
public:
  DrawLib();
  virtual ~DrawLib();

  /* initialize a drawLib from a name */
  static DrawLib *DrawLibFromName(std::string i_drawLibName);

  /* Rendering is different depending on the backend. */
  /* it's bad design issue, for sure, but we're talking about performance here
   * !*/
  /* this block should be removed as soon as possible */
  typedef enum { backend_None, backend_OpenGl, backend_SdlGFX } backendtype;
  static backendtype getBackend() { return m_backend; }
  /**/

  /**
   * initialize the screen
   **/
  static bool isInitialized();
  virtual void init(unsigned int nDispWidth,
                    unsigned int nDispHeight,
                    bool bWindowed);

  virtual void unInit() = 0;

  void setDispWidth(unsigned int width);
  unsigned int getDispWidth();
  void setDispHeight(unsigned int height);
  unsigned int getDispHeight(void);
  void setWindowed(bool windowed);
  bool getWindowed(void);
  void setNoGraphics(bool disable_graphics);
  bool isNoGraphics();

  SDL_Window *getWindow() const { return m_window; }

  virtual void setCameraDimensionality(CameraDimension dimension);

  void setRenderSurface(RenderSurface *renderSurf, bool i_own);
  RenderSurface *getRenderSurface();

  /* Methods - low-level */
  // add a vertex given screen coordinates
  virtual void glVertexSP(float x, float y) = 0;

  // add a vertex given opengl coordinates
  virtual void glVertex(float x, float y) = 0;

  // create a vertex based
  void glVertex(Vector2f x);

  // texture coordinate
  virtual void glTexCoord(float x, float y) = 0;

  virtual void setColor(Color color) = 0;

  /**
   * set the texture for drawing
   * the value may be NULL to disable texture
   * every end draw will reset the texture to NULL
   **/
  virtual void setTexture(Texture *texture, BlendMode blendMode) = 0;
  virtual void setBlendMode(BlendMode blendMode) = 0;
  void setColorRGB(unsigned int r, unsigned int g, unsigned int b);
  void setColorRGBA(unsigned int r,
                    unsigned int g,
                    unsigned int b,
                    unsigned int a);

  /**
   * enables clipping and sets the clipping borders
   **/
  virtual void setClipRect(int x, int y, unsigned int w, unsigned int h) = 0;
  virtual void setClipRect(SDL_Rect *i_clip_rect) = 0;
  virtual void setScale(float x, float y) = 0;
  virtual void setTranslate(float x, float y) = 0;
  virtual void setMirrorY() = 0;
  virtual void setRotateZ(float i_angle) = 0;
  virtual void setLineWidth(float width) = 0;

  /**
   * returns the current screen clipping
   **/
  virtual void getClipRect(int *o_px,
                           int *o_py,
                           int *o_pnWidth,
                           int *o_pnHeight) = 0;

  /**
   * Start drawing ... used in combination with glVertex
   **/
  virtual void startDraw(DrawMode mode) = 0;

  /**
   * End draw
   **/
  virtual void endDraw() = 0;
  virtual void endDrawKeepProperties() = 0; /* to keep textures, ... to render
                                               several times the same entity
                                               fastly */
  virtual void removePropertiesAfterEnd() = 0; /* remove properties endDraw =
                                                  endDrawKeepProperties +
                                                  removePropertiesAfterEnd */

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

  /* For Ghost Trail */
  void DrawFilledCircle(unsigned int nSteps,
                        Color CircleColor,
                        const Vector2f &C,
                        float fRadius);
  void DrawLine(Vector2f &i_p1,
                Vector2f &i_p2,
                const Color &i_color,
                float i_thickness1,
                float i_thickness2,
                bool i_rounded);
  /* Methods - primitives */
  virtual void drawCircle(const Vector2f &Center,
                          float fRadius,
                          float fBorder = 1.0f,
                          Color Back = 0,
                          Color Front = 0xFFFFFFFF);
  virtual void drawBox(const Vector2f &A,
                       const Vector2f &B,
                       float fBorder = 1.0f,
                       Color Back = 0,
                       Color Front = 0xFFFFFFFF);
  virtual void drawPolygon(const std::vector<Vector2f> &Points,
                           Color PolyColor = 0xFFFFFFFF);
  virtual void drawImage(const Vector2f &a,
                         const Vector2f &b,
                         Texture *pTexture,
                         Color Tint = 0xFFFFFFFF,
                         bool i_coordsReversed = false);
  virtual void drawImage(const Vector2f &a,
                         const Vector2f &b,
                         const Vector2f &c,
                         const Vector2f &d,
                         Texture *pTexture,
                         Color Tint = 0xFFFFFFFF,
                         bool i_coordsReversed = false,
                         BlendMode i_blendMode = BLEND_MODE_A);
  virtual void drawImageTextureSet(const Vector2f &a,
                                   const Vector2f &b,
                                   const Vector2f &c,
                                   const Vector2f &d,
                                   Color Tint,
                                   bool i_coordsReversed = false,
                                   bool i_keepDrawProperties = true);

  virtual bool isExtensionSupported(std::string Ext) = 0;
  void setDontUseGLExtensions(bool dont_use);
  void setDontUseGLVOBS(bool dont_use);
  virtual Img *grabScreen(int i_reduce = 1) = 0;

  /*
   * set the reference drawing size
   **/
  bool useVBOs();
  bool useFBOs();
  bool useShaders();

  /* more open specific */
  /* handle display lists */
  void toogleFullscreen();

  FontManager *getFontSmall();
  FontManager *getFontMedium();
  FontManager *getFontBig();
  FontManager *getFontMonospace();
  virtual FontManager *getFontManager(
    const std::string &i_fontFile,
    unsigned int i_fontSize,
    unsigned int i_fixedFontSize = 0 /* if > 0, force width */);
  static void checkFontPrerequites();

  Camera *getMenuCamera();

protected:
  unsigned int m_nDispWidth, m_nDispHeight; /* Screen stuff */
  unsigned int m_nLScissorX, m_nLScissorY, m_nLScissorW, m_nLScissorH;

  FontManager *m_fontSmall;
  FontManager *m_fontMedium;
  FontManager *m_fontBig;
  FontManager *m_fontMonospace;

  bool m_bWindowed; /* Windowed or not */

  Texture *m_texture;
  BlendMode m_blendMode;

  bool m_bVBOSupported;
  bool m_bFBOSupported;
  bool m_bShadersSupported;
  bool m_bDontUseGLExtensions;
  bool m_bDontUseGLVOBS;
  bool m_bNoGraphics; /* No-graphics mode */

  SDL_Window *m_window;
  Camera *m_menuCamera;
  RenderSurface *m_renderSurf;
  bool m_ownsRenderSurface;

private:
  static backendtype m_backend;
  static bool m_initialized;
};

#endif
