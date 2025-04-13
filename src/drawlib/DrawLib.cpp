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
 *  Simple 2D drawing library, built closely on top of OpenGL.
 */
#include "DrawLib.h"
#include "common/VFileIO.h"
#include "common/VFileIO_types.h"
#include "include/xm_SDL.h"
#include "xmoto/GameText.h"
#include <vector>

#define DRAW_FONT_FILE_GENERAL "Textures/Fonts/DejaVuSans.ttf"
#define DRAW_FONT_FILE_MONOSPACE "Textures/Fonts/DejaVuSansMono.ttf"

#ifdef ASIAN_TTF_FILE
#define DRAW_FONT_FILE_ASIAN ASIAN_TTF_FILE
#else
#define DRAW_FONT_FILE_ASIAN "Textures/Fonts/asian.ttf"
#endif

#ifdef ENABLE_OPENGL
#include "DrawLibOpenGL.h"
#endif

#ifdef ENABLE_SDLGFX
#include "DrawLibSDLgfx.h"
#endif

bool DrawLib::m_initialized = false;

DrawLib::backendtype DrawLib::m_backend = DrawLib::backend_None;

DrawLib *DrawLib::DrawLibFromName(std::string i_drawLibName) {
#ifdef ENABLE_OPENGL
  if (i_drawLibName == "OPENGL") {
    m_backend = backend_OpenGl;
    return new DrawLibOpenGL();
  }
#endif
#ifdef ENABLE_SDLGFX
  if (i_drawLibName == "SDLGFX") {
    m_backend = backend_SdlGFX;
    return new DrawLibSDLgfx();
  }
#endif

/* if no name is given, try to force one renderer */
#ifdef ENABLE_OPENGL
  m_backend = backend_OpenGl;
  return new DrawLibOpenGL();
#endif
#ifdef ENABLE_SDLGFX
  m_backend = backend_SdlGFX;
  return new DrawLibSDLgfx();
#endif

  m_backend = backend_None;
  return NULL;
}

void DrawLib::init(unsigned int nDispWidth,
                   unsigned int nDispHeight,
                   bool bWindowed) {
  m_initialized = true;
}

bool DrawLib::isInitialized() {
  return m_initialized;
}

void DrawLib::checkFontPrerequites() {
  if (XMFS::fileExists(FDT_DATA, FontManager::getDrawFontFile()) == false) {
    throw Exception("File " + FontManager::getDrawFontFile() + " is missing");
  }
  if (XMFS::fileExists(FDT_DATA, FontManager::getMonospaceFontFile()) ==
      false) {
    throw Exception("File " + FontManager::getMonospaceFontFile() +
                    " is missing");
  }
}

DrawLib::DrawLib() {
  m_nDispWidth = 800;
  m_nDispHeight = 600;
  m_bWindowed = true;
  m_bNoGraphics = false;
  m_bDontUseGLExtensions = false;
  m_bDontUseGLVOBS = false;
  m_bShadersSupported = false;
  m_bVBOSupported = false;
  m_nLScissorX = m_nLScissorY = m_nLScissorW = m_nLScissorH = 0;
  m_bFBOSupported = false;
  m_texture = NULL;
  m_blendMode = BLEND_MODE_NONE;
  m_ownsRenderSurface = false;
  m_renderSurf = NULL;
  m_window = NULL;
  m_menuCamera = NULL;

  m_fontSmall = NULL;
  m_fontMedium = NULL;
  m_fontBig = NULL;
  m_fontMonospace = NULL;
};

DrawLib::~DrawLib() {
  if (m_ownsRenderSurface)
    delete m_renderSurf;
}

FontManager *DrawLib::getFontManager(const std::string &i_fontFile,
                                     unsigned int i_fontSize,
                                     unsigned int i_fixedFontSize) {
  throw Exception("Your DrawLib doesn't manage FontManager");
}

FontManager *DrawLib::getFontSmall() {
  if (m_fontSmall == NULL) {
    throw Exception("Invalid font");
  }
  return m_fontSmall;
}

FontManager *DrawLib::getFontMedium() {
  if (m_fontMedium == NULL) {
    throw Exception("Invalid font");
  }
  return m_fontMedium;
}

FontManager *DrawLib::getFontBig() {
  if (m_fontBig == NULL) {
    throw Exception("Invalid font");
  }
  return m_fontBig;
}

FontManager *DrawLib::getFontMonospace() {
  if (m_fontMonospace == NULL) {
    throw Exception("Invalid font");
  }
  return m_fontMonospace;
}

/*===========================================================================
Primitive: box
===========================================================================*/
void DrawLib::drawBox(const Vector2f &A,
                      const Vector2f &B,
                      float fBorder,
                      Color Back,
                      Color Front) {
  // remove current texture if any
  setTexture(NULL, BLEND_MODE_NONE);

  /* Alpha? */
  bool bAlpha = false;
  if (GET_ALPHA(Back) != 255 || GET_ALPHA(Front) != 255)
    bAlpha = true;

  if (bAlpha) {
    setBlendMode(BLEND_MODE_A);
  }

  /* Draw rectangle background */
  if (GET_ALPHA(Back) > 0) {
    startDraw(DRAW_MODE_POLYGON);
    setColor(Back);
    glVertexSP(A.x, A.y);
    glVertexSP(A.x, B.y);
    glVertexSP(B.x, B.y);
    glVertexSP(B.x, A.y);
    endDraw();
  }

  /* Draw rectangle border */
  if (fBorder > 0.0f && GET_ALPHA(Front) > 0) {
    startDraw(DRAW_MODE_POLYGON);

    setColor(Front);
    if (bAlpha)
      setBlendMode(BLEND_MODE_A);
    glVertexSP(A.x, A.y);
    glVertexSP(A.x, B.y);
    glVertexSP(A.x + fBorder, B.y);
    glVertexSP(A.x + fBorder, A.y);
    endDraw();
    startDraw(DRAW_MODE_POLYGON);

    if (bAlpha)
      setBlendMode(BLEND_MODE_A);
    setColor(Front);
    glVertexSP(B.x - fBorder, A.y);
    glVertexSP(B.x - fBorder, B.y);
    glVertexSP(B.x, B.y);
    glVertexSP(B.x, A.y);
    endDraw();
    startDraw(DRAW_MODE_POLYGON);

    if (bAlpha)
      setBlendMode(BLEND_MODE_A);
    setColor(Front);
    glVertexSP(A.x, A.y);
    glVertexSP(A.x, A.y + fBorder);
    glVertexSP(B.x, A.y + fBorder);
    glVertexSP(B.x, A.y);
    endDraw();
    startDraw(DRAW_MODE_POLYGON);

    if (bAlpha)
      setBlendMode(BLEND_MODE_A);
    setColor(Front);
    glVertexSP(A.x, B.y - fBorder);
    glVertexSP(A.x, B.y);
    glVertexSP(B.x, B.y);
    glVertexSP(B.x, B.y - fBorder);
    endDraw();
  }

  if (bAlpha)
    setBlendMode(BLEND_MODE_NONE);
}

/*===========================================================================
Primitive: polygon
===========================================================================*/
void DrawLib::drawPolygon(const std::vector<Vector2f> &Points,
                          Color PolyColor) {
  // remove current texture if any
  setTexture(NULL, BLEND_MODE_NONE);

  /* Alpha? */
  bool bAlpha = false;
  if (GET_ALPHA(PolyColor) != 255)
    bAlpha = true;

  if (bAlpha) {
    setBlendMode(BLEND_MODE_A);
  }

  /* Draw rectangle background */
  if (GET_ALPHA(PolyColor) > 0) {
    startDraw(DRAW_MODE_POLYGON);
    setColor(PolyColor);
    for (const Vector2f &vertex : Points) {
      glVertexSP(vertex.x, vertex.y);
    }
    endDraw();
  }

  if (bAlpha)
    setBlendMode(BLEND_MODE_NONE);
}

// another function for circle (that other one didn't work when I tried using
// it.)
void DrawLib::DrawFilledCircle(unsigned int nSteps,
                               Color CircleColor,
                               const Vector2f &C,
                               float fRadius) {
  // DrawLib* pDrawlib = GameApp::instance()->getDrawLib();
  setTexture(NULL, BLEND_MODE_NONE);
  bool bAlpha = false; /* Alpha? */
  if (GET_ALPHA(CircleColor) != 255) {
    bAlpha = true;
  }
  if (bAlpha) {
    setBlendMode(BLEND_MODE_A);
  }
  startDraw(DRAW_MODE_POLYGON);
  if (bAlpha) {
    setBlendMode(BLEND_MODE_A);
  }
  setColor(CircleColor);
  for (unsigned int i = 0; i < nSteps; i++) {
    float r = (M_PI * 2.0f * (float)i) / (float)nSteps;
    glVertex(Vector2f(C.x + fRadius * sinf(r), C.y + fRadius * cosf(r)));
  }
  endDraw();
  if (bAlpha) {
    setBlendMode(BLEND_MODE_NONE);
  }
}
// a function to draw a fancy line. (used to render the ghost trail) by
// tuhoojabotti
void DrawLib::DrawLine(Vector2f &i_p1,
                       Vector2f &i_p2,
                       const Color &i_color,
                       float i_thickness1,
                       float i_thickness2,
                       bool i_rounded) {
  setTexture(NULL, BLEND_MODE_NONE);
  // declare
  float xd = i_p1.x - i_p2.x;
  float yd = i_p1.y - i_p2.y; // get differences
  float ang =
    atan2(yd, xd) + (90 / 57.29578f); // calculate angle to make thickness
  Vector2f p1, p2, p3, p4; // declare line draw points
  // calculate!
  p1 = Vector2f(i_p1.x + cos(ang) * -i_thickness1 / 2,
                i_p1.y + sin(ang) * -i_thickness1 / 2);
  p2 = Vector2f(i_p1.x + cos(ang + M_PI * 2) * i_thickness1 / 2,
                i_p1.y + sin(ang + M_PI * 2) * i_thickness1 / 2);
  p3 = Vector2f(i_p2.x + cos(ang + M_PI * 2) * i_thickness2 / 2,
                i_p2.y + sin(ang + M_PI * 2) * i_thickness2 / 2);
  p4 = Vector2f(i_p2.x + cos(ang) * -i_thickness2 / 2,
                i_p2.y + sin(ang) * -i_thickness2 / 2);
  // DRAW!
  startDraw(DRAW_MODE_POLYGON);
  setColor(i_color);
  glVertex(p1);
  glVertex(p2);
  glVertex(p3);
  glVertex(p4);
  endDraw();
  // round the ends
  if (i_rounded == true) {
    if (i_thickness1 > 0.045) {
      DrawFilledCircle(20, i_color, i_p1, i_thickness1 / 2);
    }
    if (i_thickness2 > 0.045) {
      DrawFilledCircle(20, i_color, i_p2, i_thickness2 / 2);
    }
  }
}

/*===========================================================================
Primitive: circle
===========================================================================*/
void DrawLib::drawCircle(const Vector2f &Center,
                         float fRadius,
                         float fBorder,
                         Color Back,
                         Color Front) {
  setTexture(NULL, BLEND_MODE_NONE);

  /* Alpha? */
  bool bAlpha = false;
  if (GET_ALPHA(Back) != 255 || GET_ALPHA(Front) != 255)
    bAlpha = true;

  if (bAlpha) {
    setBlendMode(BLEND_MODE_A);
  }

  /* How many steps? */
  int nSteps = (int)(2.0f * (fRadius / 3.0f));
  if (nSteps < 8)
    nSteps = 8;
  if (nSteps > 64)
    nSteps = 64;

  /* Draw circle background */
  if (GET_ALPHA(Back) > 0) {
    startDraw(DRAW_MODE_POLYGON);
    if (bAlpha)
      setBlendMode(BLEND_MODE_A);
    setColor(Back);
    for (int i = 0; i < nSteps; i++) {
      float rads = (M_PI * 2.0f * (float)i) / (float)nSteps;
      glVertexSP(Center.x + fRadius * sin(rads),
                 Center.y + fRadius * cos(rads));
    }
    endDraw();
  }

  /* Draw circle border */
  if (fBorder > 0.0f && GET_ALPHA(Front) > 0) {
    for (int i = 0; i < nSteps; i++) {
      float rads1 = (M_PI * 2.0f * (float)i) / (float)nSteps;
      float rads2 = (M_PI * 2.0f * (float)(i + 1)) / (float)nSteps;

      startDraw(DRAW_MODE_POLYGON);
      if (bAlpha)
        setBlendMode(BLEND_MODE_A);
      setColor(Front);
      glVertexSP(Center.x + fRadius * sin(rads1),
                 Center.y + fRadius * cos(rads1));
      glVertexSP(Center.x + fRadius * sin(rads2),
                 Center.y + fRadius * cos(rads2));
      glVertexSP(Center.x + (fRadius - fBorder) * sin(rads2),
                 Center.y + (fRadius - fBorder) * cos(rads2));
      glVertexSP(Center.x + (fRadius - fBorder) * sin(rads1),
                 Center.y + (fRadius - fBorder) * cos(rads1));
      endDraw();
    }
  }

  /* Disable alpha again if we enabled it */
  if (bAlpha)
    setBlendMode(BLEND_MODE_NONE);
}

void DrawLib::setCameraDimensionality(CameraDimension dimension) {}

void DrawLib::setRenderSurface(RenderSurface *renderSurf, bool i_own) {
  if (m_renderSurf != renderSurf && m_ownsRenderSurface)
    delete m_renderSurf;
  m_renderSurf = renderSurf;
  m_ownsRenderSurface = i_own;
}

RenderSurface *DrawLib::getRenderSurface() {
  return m_renderSurf;
}

void DrawLib::setDispWidth(unsigned int width) {
  m_nDispWidth = width;
}

unsigned int DrawLib::getDispWidth(void) {
  return m_nDispWidth;
}

void DrawLib::setNoGraphics(bool disable_graphics) {
  m_bNoGraphics = disable_graphics;
}

bool DrawLib::isNoGraphics() {
  return m_bNoGraphics;
}

void DrawLib::setDispHeight(unsigned int height) {
  m_nDispHeight = height;
}

unsigned int DrawLib::getDispHeight(void) {
  return m_nDispHeight;
}

void DrawLib::setWindowed(bool windowed) {
  m_bWindowed = windowed;
}

bool DrawLib::getWindowed(void) {
  return m_bWindowed;
}

void DrawLib::glVertex(Vector2f x) {
  glVertex(x.x, x.y);
}

void DrawLib::setColorRGB(unsigned int r, unsigned int g, unsigned int b) {
  setColor(MAKE_COLOR(r, g, b, 255));
}

void DrawLib::setColorRGBA(unsigned int r,
                           unsigned int g,
                           unsigned int b,
                           unsigned int a) {
  setColor(MAKE_COLOR(r, g, b, a));
}

void DrawLib::resetGraphics() {}

void DrawLib::setDontUseGLExtensions(bool dont_use) {
  m_bDontUseGLExtensions = dont_use;
}

void DrawLib::setDontUseGLVOBS(bool dont_use) {
  m_bDontUseGLVOBS = dont_use;
}

bool DrawLib::useVBOs() {
  return m_bVBOSupported;
};

bool DrawLib::useFBOs() {
  return m_bFBOSupported;
};

bool DrawLib::useShaders() {
  return m_bShadersSupported;
};

Camera *DrawLib::getMenuCamera() {
  return m_menuCamera;
}

/*===========================================================================
Primitive: box
===========================================================================*/
void DrawLib::drawImage(const Vector2f &a,
                        const Vector2f &b,
                        Texture *pTexture,
                        Color Tint,
                        bool i_coordsReversed) {
  drawImage(a,
            Vector2f(b.x, a.y),
            b,
            Vector2f(a.x, b.y),
            pTexture,
            Tint,
            i_coordsReversed);
}

void DrawLib::drawImage(const Vector2f &a,
                        const Vector2f &b,
                        const Vector2f &c,
                        const Vector2f &d,
                        Texture *pTexture,
                        Color Tint,
                        bool i_coordsReversed,
                        BlendMode i_blendMode) {
  setTexture(pTexture, i_blendMode);
  drawImageTextureSet(a, b, c, d, Tint, i_coordsReversed);
  setTexture(NULL, i_blendMode);
}

void DrawLib::drawImageTextureSet(const Vector2f &a,
                                  const Vector2f &b,
                                  const Vector2f &c,
                                  const Vector2f &d,
                                  Color Tint,
                                  bool i_coordsReversed,
                                  bool i_keepDrawProperties) {
  float v_absorb = 0.0;

  startDraw(DRAW_MODE_POLYGON);
  setColor(Tint);

  if ((a.x == d.x && a.y == b.y && c.x == b.x && c.y == d.y) ||
      (d.x == c.x && d.y == a.y && b.x == a.x && b.y == c.y) ||
      (c.x == b.x && c.y == d.y && a.x == d.x && a.y == b.y) ||
      (b.x == a.x && b.y == c.y && d.x == c.x &&
       d.y == a.y)) { // simple case, no rotation, 90, 180, 270
    v_absorb = 0.00;
  } else {
    /* because rotation can make approximation error and 1 pixel of one side of
       the
       picture could be map on the other side, */
    v_absorb = 0.001;
  }

  if (i_coordsReversed) {
    glTexCoord(v_absorb, v_absorb);
    glVertexSP(a.x, a.y);
    glTexCoord(1.00 - v_absorb, v_absorb);
    glVertexSP(b.x, b.y);
    glTexCoord(1.00 - v_absorb, 1.00 - v_absorb);
    glVertexSP(c.x, c.y);
    glTexCoord(v_absorb, 1.0 - v_absorb);
    glVertexSP(d.x, d.y);
  } else {
    glTexCoord(v_absorb, v_absorb);
    glVertex(a.x, a.y);
    glTexCoord(1.00 - v_absorb, v_absorb);
    glVertex(b.x, b.y);
    glTexCoord(1.00 - v_absorb, 1.00 - v_absorb);
    glVertex(c.x, c.y);
    glTexCoord(v_absorb, 1.0 - v_absorb);
    glVertex(d.x, d.y);
  }

  if (i_keepDrawProperties) {
    endDrawKeepProperties();
  } else {
    endDraw();
  }
}

void DrawLib::toogleFullscreen() {}

FontManager::FontManager(DrawLib *i_drawLib,
                         const std::string &i_fontFile,
                         unsigned int i_fontSize,
                         unsigned int i_fixedFontSize) {
  m_drawLib = i_drawLib;
  m_ttf = TTF_OpenFont(i_fontFile.c_str(), i_fontSize);
  if (m_ttf == NULL) {
    throw Exception("FontManager: " + std::string(TTF_GetError()));
  }
  TTF_SetFontStyle(m_ttf, TTF_STYLE_NORMAL);
  m_fixedFontSize = i_fixedFontSize;
}

FontManager::~FontManager() {
  TTF_CloseFont(m_ttf);
}

std::string FontManager::getDrawFontFile() {
  if (std::string(_("FontGroup:GENERAL")) == std::string("FontGroup:GENERAL"))
    return DRAW_FONT_FILE_GENERAL;
  if (std::string(_("FontGroup:GENERAL")) == std::string("FontGroup:ASIAN"))
    return DRAW_FONT_FILE_ASIAN;

  return DRAW_FONT_FILE_GENERAL;
}

std::string FontManager::getMonospaceFontFile() {
  return DRAW_FONT_FILE_MONOSPACE;
}
