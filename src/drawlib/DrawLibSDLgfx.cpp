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
#include "DrawLibSDLgfx.h"
#include "common/Image.h"
#include "helpers/Log.h"
#include "xmoto/Game.h"

#ifdef ENABLE_SDLGFX
#include "PolyDraw.h"
#include "SDL_gfxPrimitives.h"
#include "SDL_rotozoom.h"
#include "helpers/iqsort.h"
// #define islt(a,b) ((*(const int *)a) - (*(const int *)b))
#define islt(a, b) ((*a) < (*b))

int xx_gfxPrimitivesCompareInt(const void *a, const void *b) {
  return (*(const int *)a) - (*(const int *)b);
};

class SDLFontGlyph : public FontGlyph {
public:
  /* for simple glyph */
  SDLFontGlyph(const std::string &i_value);

  virtual ~SDLFontGlyph();

  std::string Value() const;
  unsigned int drawWidth() const;
  unsigned int drawHeight() const;
  unsigned int realWidth() const;
  unsigned int realHeight() const;
  unsigned int firstLineDrawHeight() const;

protected:
  std::string m_value;
  unsigned int m_drawWidth, m_drawHeight;
  unsigned int m_realWidth, m_realHeight;
  unsigned int m_firstLineDrawHeight;
};

SDLFontGlyph::SDLFontGlyph(const std::string &i_value) {
  m_value = i_value; /*c++ */

  m_drawWidth = 1;
  m_drawHeight = 1;
  m_realWidth = 1;
  m_realHeight = 1;
  m_firstLineDrawHeight = 1;
};

SDLFontGlyph::~SDLFontGlyph() {}

std::string SDLFontGlyph::Value() const {
  return m_value;
}

unsigned int SDLFontGlyph::drawWidth() const {
  return m_drawWidth;
}

unsigned int SDLFontGlyph::drawHeight() const {
  return m_drawHeight;
}

unsigned int SDLFontGlyph::realWidth() const {
  return m_realWidth;
}

unsigned int SDLFontGlyph::realHeight() const {
  return m_realHeight;
}

unsigned int SDLFontGlyph::firstLineDrawHeight() const {
  return m_firstLineDrawHeight;
}

class SDLFontManager : public FontManager {
public:
  SDLFontManager(DrawLib *i_drawLib,
                 const std::string &i_fontFile,
                 int i_fontSize);
  virtual ~SDLFontManager();

  FontGlyph *getGlyph(const std::string &i_string);
  void printString(FontGlyph *i_glyph,
                   int i_x,
                   int i_y,
                   Color i_color,
                   float i_perCentered = -1.0,
                   bool i_shadowEffect = false);
  void printStringGrad(FontGlyph *i_glyph,
                       int i_x,
                       int i_y,
                       Color c1,
                       Color c2,
                       Color c3,
                       Color c4,
                       float i_perCentered = -1.0,
                       bool i_shadowEffect = false);

private:
  DrawLib *m_drawlib;
  std::string i_fontFile;
};

SDLFontManager::SDLFontManager(DrawLib *i_drawLib,
                               const std::string &i_fontFile,
                               int i_fontSize)
  : FontManager(i_drawLib, i_fontFile, i_fontSize) {};

FontGlyph *SDLFontManager::getGlyph(const std::string &i_string) {
  return new SDLFontGlyph("test");
};

void SDLFontManager::printStringGrad(FontGlyph *i_glyph,
                                     int i_x,
                                     int i_y,
                                     Color c1,
                                     Color c2,
                                     Color c3,
                                     Color c4,
                                     float i_perCentered,
                                     bool i_shadowEffect) {
  SDL_Color color = { 99, 0, 33, 0 };
  SDL_Surface *text_surface = TTF_RenderText_Blended(
    m_ttf, ((SDLFontGlyph *)i_glyph)->Value().c_str(), color);
  SDL_FreeSurface(text_surface);
}
void SDLFontManager::printString(FontGlyph *i_glyph,
                                 int i_x,
                                 int i_y,
                                 Color i_color,
                                 float i_perCentered,
                                 bool i_shadowEffect) {
  printStringGrad(
    i_glyph, i_x, i_y, i_color, i_color, i_color, i_color, i_perCentered);
}

SDLFontManager::~SDLFontManager() {}

FontManager *DrawLibSDLgfx::getFontManager(const std::string &i_fontFile,
                                           int i_fontSize) {
  return new SDLFontManager(this, i_fontFile, i_fontSize);
};

DrawLibSDLgfx::DrawLibSDLgfx()
  : DrawLib() {
  m_scale.x = 1;
  m_scale.y = 1;
  m_translate.x = 0;
  m_translate.y = 0;
  m_bg_data = NULL;
  gfxPrimitivesPolyInts = NULL;
  gfxPrimitivesPolyAllocated = 0;
  m_min.x = 0;
  m_min.y = 0;
  m_max.x = 0;
  m_max.y = 0;
  m_polyDraw = NULL;
  m_texture = NULL;

  m_fontSmall =
    getFontManager(FS::FullPath(FontManager::getDrawFontFile()), 14);
  m_fontMedium =
    getFontManager(FS::FullPath(FontManager::getDrawFontFile()), 22);
  m_fontBig = getFontManager(FS::FullPath(FontManager::getDrawFontFile()), 60);
};

DrawLibSDLgfx::~DrawLibSDLgfx() {};

/*===========================================================================
Transform an OpenGL vertex to pure 2D
===========================================================================*/
void DrawLibSDLgfx::glVertexSP(float x, float y) {
  m_drawingPoints.push_back(new Vector2f(x, y));
  if (m_drawingPoints.size() == 0) {
    m_min.x = m_max.x = x;
    m_min.y = m_max.y = y;
  } else {
    if (x < m_min.x) {
      m_min.x = x;
    }
    if (x > m_max.x) {
      m_max.x = x;
    }
    if (y < m_min.y) {
      m_min.y = y;
    }
    if (y > m_max.y) {
      m_max.y = y;
    }
  }
}

void DrawLibSDLgfx::glVertex(float x, float y) {
  Vector2f *v =
    new Vector2f(m_nDrawWidth / 2 + ((x + m_translate.x) * m_scale.x),
                 m_nDrawHeight / 2 - ((y + m_translate.y) * m_scale.y));

  if (m_drawingPoints.size() == 0) {
    m_min.x = m_max.x = v->x;
    m_min.y = m_max.y = v->y;
  } else {
    if (v->x < m_min.x) {
      m_min.x = v->x;
    }
    if (v->x > m_max.x) {
      m_max.x = v->x;
    }
    if (v->y < m_min.y) {
      m_min.y = v->y;
    }
    if (v->y > m_max.y) {
      m_max.y = v->y;
    }
  }
  m_drawingPoints.push_back(v);
}

void DrawLibSDLgfx::glTexCoord(float x, float y) {
  m_texturePoints.push_back(new Vector2f(x, y));
}

void DrawLibSDLgfx::screenProjVertex(float *x, float *y) {
  *y = m_nActualHeight - (*y);
}

void DrawLibSDLgfx::setClipRect(int x, int y, unsigned int w, unsigned int h) {
  SDL_Rect *rect = new SDL_Rect();

  rect->x = x;
  rect->y = y;
  rect->w = w;
  rect->h = h;
  setClipRect(rect);
}

void DrawLibSDLgfx::setClipRect(SDL_Rect *clip_rect) {
  SDL_SetClipRect(SDL_GetWindowSurface(m_window), clip_rect);
}

void DrawLibSDLgfx::getClipRect(int *px, int *py, int *pnWidth, int *pnHeight) {
  *px = m_nLScissorX;
  *py = m_nLScissorY;
  *pnWidth = m_nLScissorW;
  *pnHeight = m_nLScissorH;
}

void DrawLibSDLgfx::setScale(float x, float y) {
  m_scale.x = x * m_nDrawWidth / 2;
  m_scale.y = y * m_nDrawHeight / 2;
}
void DrawLibSDLgfx::setTranslate(float x, float y) {
  m_translate.x = x;
  m_translate.y = y;
}

void DrawLibSDLgfx::setMirrorY() {}

void DrawLibSDLgfx::setRotateZ(float i_angle) {}

void DrawLibSDLgfx::setLineWidth(float width) {}

void DrawLibSDLgfx::init(unsigned int nDispWidth,
                         unsigned int nDispHeight,
                         unsigned int nDispBPP,
                         bool bWindowed) {
  DrawLib::init(nDispWidth, nDispHeight, nDispBPP, bWindowed);

  /* Set suggestions */
  m_nDispWidth = nDispWidth;
  m_nDispHeight = nDispHeight;
  m_nDispBPP = nDispBPP;
  m_bWindowed = bWindowed;

  /* Get some video info */
  const SDL_VideoInfo *pVidInfo = SDL_GetVideoInfo();

  if (pVidInfo == NULL)
    throw Exception("(1) SDL_GetVideoInfo : " + std::string(SDL_GetError()));

  /* Determine target bit depth */
  if (m_bWindowed)
    /* In windowed mode we can't tinker with the bit-depth */
    m_nDispBPP = pVidInfo->vfmt->BitsPerPixel;

  int nFlags = SDL_SRCALPHA;

  if (!m_bWindowed)
    nFlags |= SDL_FULLSCREEN;
  /* At last, try to "set the video mode" */
  if ((m_window = SDL_SetVideoMode(
         m_nDispWidth, m_nDispHeight, m_nDispBPP, nFlags)) == NULL) {
    LogWarning(
      "Tried to set video mode %dx%d @ %d-bit, but SDL responded: %s\n"
      "                Now SDL will try determining a proper mode itself.",
      m_nDispWidth,
      m_nDispHeight,
      m_nDispBPP);

    /* Hmm, try letting it decide the BPP automatically */
    if ((m_window = SDL_SetVideoMode(m_nDispWidth, m_nDispHeight, 0, nFlags)) ==
        NULL) {
      /* Still no luck */
      LogWarning("Still no luck, now we'll try 800x600 in a window.");
      m_nDispWidth = 800;
      m_nDispHeight = 600;
      m_bWindowed = true;
      if ((m_window = SDL_SetVideoMode(
             m_nDispWidth, m_nDispHeight, 0, SDL_SRCALPHA)) == NULL) {
        throw Exception("SDL_SetVideoMode : " + std::string(SDL_GetError()));
      }
    }
  }

  /* Retrieve actual configuration */
  pVidInfo = SDL_GetVideoInfo();
  if (pVidInfo == NULL)
    throw Exception("(2) SDL_GetVideoInfo : " + std::string(SDL_GetError()));

  m_nDispBPP = pVidInfo->vfmt->BitsPerPixel;

  /* Enable unicode translation and key repeats */
  SDL_EnableUNICODE(1);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  LogInfo("Using SDL_gfx implementation");

  // screenBuffer.nPixelAlign = m_window->format->BytesPerPixel;
  // screenBuffer.nPixelSize = m_window->format->BytesPerPixel;
  // screenBuffer.nWidth = m_nDispWidth;
  // screenBuffer.nHeight = m_nDispHeight;
  m_polyDraw = new PolyDraw(m_window);

  // setBackgroundColor(0,0,40,255);
}

void DrawLibSDLgfx::unInit() {}

/*===========================================================================
Check for OpenGL extension
===========================================================================*/
bool DrawLibSDLgfx::isExtensionSupported(std::string Ext) {
  return false;
}

/*===========================================================================
Grab screen contents
===========================================================================*/
Img *DrawLibSDLgfx::grabScreen(int i_reduce) {
  Img *pImg = new Img;

  pImg->createEmpty(m_nDispWidth, m_nDispHeight);
  Color *pPixels = pImg->getPixels();

  Uint8 red, green, blue, alpha;

  SDL_Surface *surface = SDL_GetWindowSurface(m_window);
  SDL_LockSurface(surface);

  for (int y = 0; y < m_window->h; y++) {
    for (int x = 0; x < m_window->w; x++) {
      void *pixel = (Uint8 *)m_window->pixels +
                    (x * m_window->format->BytesPerPixel) +
                    ((y) * (m_window->pitch));
      SDL_GetRGB(*(Uint32 *)pixel, m_window->format, &red, &green, &blue);

      *(Uint32 *)pPixels = MAKE_COLOR(red, green, blue, 255);
      pPixels++;
    }
  }

  SDL_UnlockSurface(surface);

  return pImg;
}

void DrawLibSDLgfx::startDraw(DrawMode mode) {
  m_drawMode = mode;
  if (m_drawingPoints.size() != 0 || m_texturePoints.size() != 0) {
    printf(
      "error drawingPoints.size(%i) of texturePoints.size(%i) was not 0 =%i\n",
      m_drawingPoints.size(),
      m_texturePoints.size());
  }
}

void DrawLibSDLgfx::endDraw() {
  int size = m_drawingPoints.size();
  int x0, y0, x1, y1;
  SDL_Rect *clip = NULL;

  SDL_Surface *surface = SDL_GetWindowSurface(m_window);
  // get the current clipping on the screen
  // if null we must expect the be drawing
  // on the full screen
  SDL_GetClipRect(surface, clip);
  if (clip != NULL) {
    x0 = clip->x;
    y0 = clip->y;
    x1 = x0 + clip->w;
    x1 = y0 + clip->h;
  } else {
    x0 = 0;
    y0 = 0;
    x1 = surface->w;
    y1 = surface->h;
  }

  bool draw = true;

  // if all the lines  are "smaller" or larger then the screen
  // we don't need to draw
  if (m_max.x < x0 || m_min.x > x1 || m_max.y < y0 || m_min.y > y1) {
    draw = false;
  }

  if (draw == true) {
    // keesj:TODO , why not just create one x/y array
    // and use that all the time. this is also true
    // for texture point and drawing points
    // it will also remove the need to clean the m_texturePoints
    // and m_drawingPoints collection afterwards

    for (int i = 0; i < size; i++) {
      m_int_drawing_points_x[i] = m_drawingPoints.at(i)->x;
      m_int_drawing_points_y[i] = m_drawingPoints.at(i)->y;
    }
    SDL_Surface *windowSurface = SDL_GetWindowSurface(m_window);
    switch (m_drawMode) {
      case DRAW_MODE_POLYGON:
        if (m_texture != NULL) {
          SDL_LockSurface(windowSurface);
          // TODO:keesj check if the surface is still valid
          // screenBuffer.pcData = (char *)windowSurface->pixels;
          // screenBuffer.nPitch = windowSurface->pitch;
          // SDL_Surface * s = SDL_DisplayFormatAlpha(m_texture->surface);
          SDL_Surface *s = NULL;

          char key[255] = "";

          snprintf(key, 255, "%s--", m_texture->Name.c_str());
          std::map<const char *, SDL_Surface *>::iterator i =
            m_image_cache.find(key);
          if (i != m_image_cache.end()) {
            s = (*i).second;
          } else {
            char *keyName = (char *)malloc(strlen(key) + 1);

            // if (m_texture->isAlpha) {
            // s = SDL_DisplayFormatAlpha(m_texture->surface);
            //} else {
            // PolyDraw only supports textures size up to 256x256
            // pixels if the texture is larger we scale it down
            if (m_texture->surface->w > 256) {
              double zoom = 256.0 / m_texture->surface->w;
              SDL_Surface *a =
                zoomSurface(m_texture->surface, zoom, zoom, SMOOTHING_ON);
              if (m_texture->isAlpha) {
                s = SDL_DisplayFormatAlpha(a);
                // s = SDL_ConvertSurface(a, windowSurface->format,
                // SDL_HWSURFACE);
              } else {
                s = SDL_ConvertSurface(a, windowSurface->format, SDL_HWSURFACE);
              }
              SDL_FreeSurface(a);
            } else {
              s = SDL_ConvertSurface(
                m_texture->surface, windowSurface->format, SDL_HWSURFACE);
              if (m_texture->isAlpha) {
                s = SDL_DisplayFormatAlpha(m_texture->surface);
                // s = SDL_ConvertSurface(m_texture->surface,
                // windowSurface->format, SDL_HWSURFACE);
              } else {
                s = SDL_ConvertSurface(
                  m_texture->surface, windowSurface->format, SDL_HWSURFACE);
              }
            }

            strcpy(keyName, key);
            m_image_cache.insert(std::make_pair<>(keyName, s));
          }

          SDL_LockSurface(s);
          /*
          PolyDraw::Buffer texture;
          texture.pcData = (char *)s->pixels;
          texture.nPitch = s->pitch;
          texture.nPixelAlign = s->format->BytesPerPixel;
          texture.nPixelSize = s->format->BytesPerPixel;
          texture.nWidth = s->w;
          texture.nHeight = s->h;
          */

          for (int i = 0; i < size; i++) {
            screenVerticles[i * 2] = m_drawingPoints.at(i)->x;
            screenVerticles[(2 * i) + 1] = m_drawingPoints.at(i)->y;
          }

          for (int i = 0; i < m_texturePoints.size(); i++) {
            nPolyTextureVertices[2 * i] =
              (int)(m_texturePoints.at(i)->x * 0xff);
            nPolyTextureVertices[(2 * i) + 1] =
              (int)(m_texturePoints.at(i)->y * 0xff);
          }

          m_polyDraw->render(screenVerticles, nPolyTextureVertices, size, s);
          SDL_UnlockSurface(s);
          SDL_UnlockSurface(windowSurface);
        } else {
          xx_filledPolygonColor(windowSurface,
                                m_int_drawing_points_x,
                                m_int_drawing_points_y,
                                size,
                                m_color);
        }

        break;
      case DRAW_MODE_LINE_LOOP:
        polygonRGBA(windowSurface,
                    m_int_drawing_points_x,
                    m_int_drawing_points_y,
                    size,
                    GET_RED(m_color),
                    GET_GREEN(m_color),
                    GET_BLUE(m_color),
                    GET_ALPHA(m_color));
        break;
      case DRAW_MODE_LINE_STRIP:
        for (int f = 0; f < size - 1; f++) {
          lineRGBA(windowSurface,
                   m_int_drawing_points_x[f],
                   m_int_drawing_points_y[f],
                   m_int_drawing_points_x[f + 1],
                   m_int_drawing_points_y[f + 1],
                   GET_RED(m_color),
                   GET_GREEN(m_color),
                   GET_BLUE(m_color),
                   GET_ALPHA(m_color));
        }
        break;
    }
  }
  for (int i = 0; i < m_texturePoints.size(); i++) {
    delete m_drawingPoints.at(i);
  }
  for (int i = 0; i < m_texturePoints.size(); i++) {
    delete m_texturePoints.at(i);
  }
  m_drawingPoints.resize(0);
  m_texturePoints.resize(0);
  m_drawMode = DRAW_MODE_NONE;
  m_texture = NULL;
  m_min.x = 0;
  m_min.y = 0;
  m_max.x = 0;
  m_max.y = 0;
}

void DrawLibSDLgfx::endDrawKeepProperties() {
  endDraw();
}

void DrawLibSDLgfx::removePropertiesAfterEnd() {}

void DrawLibSDLgfx::setColor(Color color) {
  m_color = color;
}

void DrawLibSDLgfx::setTexture(Texture *texture, BlendMode blendMode) {
  m_texture = texture;
}

void DrawLibSDLgfx::setBlendMode(BlendMode blendMode) {}

void DrawLibSDLgfx::clearGraphics() {
  SDL_Surface *surface = SDL_GetWindowSurface(m_window);
  SDL_LockSurface(surface);
  if (m_bg_data == NULL) {
    m_bg_data =
      malloc(surface->format->BytesPerPixel * surface->w * surface->h);
    memset(
      m_bg_data, 0, surface->format->BytesPerPixel * surface->w * surface->h);
  }
  memcpy(surface->pixels,
         m_bg_data,
         surface->format->BytesPerPixel * surface->w * surface->h);
  SDL_UnlockSurface(surface);
  // SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
}

void DrawLibSDLgfx::resetGraphics() {
  setClipRect(NULL);
  m_scale.x = 1;
  m_scale.y = 1;
  m_translate.x = 0;
  m_translate.y = 0;
}

/**
 * Flush the graphics. In memory graphics will now be displayed
 **/
void DrawLibSDLgfx::flushGraphics() {
  SDL_UpdateRect(SDL_GetWindowSurface(m_window), 0, 0, 0, 0);
}

int DrawLibSDLgfx::xx_texturedHLineAlpha(SDL_Surface *dst,
                                         Sint16 x1,
                                         Sint16 x2,
                                         Sint16 y,
                                         SDL_Surface *texture,
                                         int texture_dx,
                                         int texture_dy) {
  Sint16 left, right, top, bottom;
  Sint16 w;
  Sint16 xtmp;
  int result = 0;
  int texture_x_walker;
  int texture_y_start;
  SDL_Rect source_rect, dst_rect;
  int pixels_written, write_width;

  /*
   * Get clipping boundary
   */
  left = dst->clip_rect.x;
  right = dst->clip_rect.x + dst->clip_rect.w - 1;
  top = dst->clip_rect.y;
  bottom = dst->clip_rect.y + dst->clip_rect.h - 1;

  /*
   * Check visibility of hline
   */
  if ((x1 < left) && (x2 < left)) {
    return (0);
  }
  if ((x1 > right) && (x2 > right)) {
    return (0);
  }
  if ((y < top) || (y > bottom)) {
    return (0);
  }

  /*
   * Swap x1, x2 if required
   */
  if (x1 > x2) {
    xtmp = x1;
    x1 = x2;
    x2 = xtmp;
  }

  /*
   * Clip x
   */
  if (x1 < left) {
    x1 = left;
  }
  if (x2 > right) {
    x2 = right;
  }

  /*
   * Calculate width
   */
  w = x2 - x1;

  /*
   * Sanity check on width
   */
  if (w < 0) {
    return (0);
  }

  /*
   * Determint where in the texture we start drawing
   **/
  texture_x_walker = (x1 - texture_dx) % texture->w;
  if (texture_x_walker < 0) {
    texture_x_walker = texture->w + texture_x_walker;
  }

  texture_y_start = (y + texture_dy) % texture->h;
  if (texture_y_start < 0) {
    texture_y_start = texture->h + texture_y_start;
  }
  // setup the source rectangle  we are only drawing one horizontal line
  source_rect.y = texture_y_start;
  source_rect.x = texture_x_walker;
  source_rect.h = 1;
  // we will draw to the current y
  dst_rect.y = y;

  // if there are enough pixels left in the current row of the texture
  // draw it all at once
  if (w <= texture->w - texture_x_walker) {
    source_rect.w = w;
    source_rect.x = texture_x_walker;
    dst_rect.x = x1;
    result != SDL_BlitSurface(texture, &source_rect, dst, &dst_rect);
  } else { // we need to draw multiple times
    // draw the first segment
    pixels_written = texture->w - texture_x_walker;
    source_rect.w = pixels_written;
    source_rect.x = texture_x_walker;
    dst_rect.x = x1;
    result != SDL_BlitSurface(texture, &source_rect, dst, &dst_rect);
    write_width = texture->w;

    // now draw the rest
    // set the source x to 0
    source_rect.x = 0;
    while (pixels_written < w) {
      if (write_width >= w - pixels_written) {
        write_width = w - pixels_written;
      }
      source_rect.w = write_width;
      dst_rect.x = x1 + pixels_written;
      result != SDL_BlitSurface(texture, &source_rect, dst, &dst_rect);
      pixels_written += write_width;
    }
  }
  return result;
}

/**
 * Draws a polygon filled with the given texture. this operation use
 *SDL_BlitSurface. It supports
 * alpha drawing.
 * to get the best performance of this operation you need to make sure the
 *texture and the dst surface have the same format
 * see
 *http://docs.mandragor.org/files/Common_libs_documentation/SDL/SDL_Documentation_project_en/sdlblitsurface.html
 *
 * dest the destination surface,
 * vx array of x vector components
 * vy array of x vector components
 * n the amount of vectors in the vx and vy array
 * texture the sdl surface to use to fill the polygon
 * texture_dx the offset of the texture relative to the screeen. if you move the
 *polygon 10 pixels to the left and want the texture
 * to apear the same you need to increase the texture_dx value
 * texture_dy see texture_dx
 **/
int DrawLibSDLgfx::xx_texturedPolygonAlpha(SDL_Surface *dst,
                                           const Sint16 *vx,
                                           const Sint16 *vy,
                                           int n,
                                           SDL_Surface *texture,
                                           int texture_dx,
                                           int texture_dy) {
  int result;
  int i;
  int y, xa, xb;
  int minx, maxx, miny, maxy;
  int x1, y1;
  int x2, y2;
  int ind1, ind2;
  int ints;

  /*
   * Sanity check
   */
  if (n < 3) {
    return -1;
  }

  /*
   * Allocate temp array, only grow array
   */
  if (!gfxPrimitivesPolyAllocated) {
    gfxPrimitivesPolyInts = (int *)malloc(sizeof(int) * n);
    gfxPrimitivesPolyAllocated = n;
  } else {
    if (gfxPrimitivesPolyAllocated < n) {
      gfxPrimitivesPolyInts =
        (int *)realloc(gfxPrimitivesPolyInts, sizeof(int) * n);
      gfxPrimitivesPolyAllocated = n;
    }
  }

  miny = vy[0];
  maxy = vy[0];
  minx = vx[0];
  maxx = vx[0];
  for (i = 1; (i < n); i++) {
    if (vy[i] < miny) {
      miny = vy[i];
    } else if (vy[i] > maxy) {
      maxy = vy[i];
    }
    if (vx[i] < minx) {
      minx = vx[i];
    } else if (vx[i] > maxx) {
      maxx = vx[i];
    }
  }
  if (maxx < 0 || minx > dst->w) {
    return 0;
  }
  if (maxy < 0 || miny > dst->h) {
    return 0;
  }

  /*
   * Draw, scanning y
   */
  result = 0;
  for (y = miny; (y <= maxy); y++) {
    ints = 0;
    for (i = 0; (i < n); i++) {
      if (!i) {
        ind1 = n - 1;
        ind2 = 0;
      } else {
        ind1 = i - 1;
        ind2 = i;
      }
      y1 = vy[ind1];
      y2 = vy[ind2];
      if (y1 < y2) {
        x1 = vx[ind1];
        x2 = vx[ind2];
      } else if (y1 > y2) {
        y2 = vy[ind1];
        y1 = vy[ind2];
        x2 = vx[ind1];
        x1 = vx[ind2];
      } else {
        continue;
      }
      if (((y >= y1) && (y < y2)) || ((y == maxy) && (y > y1) && (y <= y2))) {
        gfxPrimitivesPolyInts[ints++] =
          ((65536 * (y - y1)) / (y2 - y1)) * (x2 - x1) + (65536 * x1);
      }
    }

    QSORT(int, gfxPrimitivesPolyInts, ints, islt);

    //      qsort(gfxPrimitivesPolyInts, ints, sizeof(int),
    //          xx_gfxPrimitivesCompareInt);

    for (i = 0; (i < ints); i += 2) {
      xa = gfxPrimitivesPolyInts[i] + 1;
      xa = (xa >> 16) + ((xa & 32768) >> 15);
      xb = gfxPrimitivesPolyInts[i + 1] - 1;
      xb = (xb >> 16) + ((xb & 32768) >> 15);
      result |=
        xx_texturedHLineAlpha(dst, xa, xb, y, texture, texture_dx, texture_dy);
    }
  }

  return (result);
}

int DrawLibSDLgfx::xx_texturedHLine(SDL_Surface *dst,
                                    Sint16 x1,
                                    Sint16 x2,
                                    Sint16 y,
                                    SDL_Surface *texture,
                                    int texture_dx,
                                    int texture_dy) {
  Sint16 left, right, top, bottom;
  Uint8 *pixel, *pixellast;
  int dx;
  int pixx, pixy;
  Sint16 w;
  Sint16 xtmp;
  int result = -1;

  int texture_start;
  int texture_x_walker;
  void *texture_start_pixel;

  /*
   * Get clipping boundary
   */
  left = dst->clip_rect.x;
  right = dst->clip_rect.x + dst->clip_rect.w - 1;
  top = dst->clip_rect.y;
  bottom = dst->clip_rect.y + dst->clip_rect.h - 1;

  /*
   * Check visibility of hline
   */
  if ((x1 < left) && (x2 < left)) {
    return 0;
  }
  if ((x1 > right) && (x2 > right)) {
    return 0;
  }
  if ((y < top) || (y > bottom)) {
    return 0;
  }

  /*
   * Swap x1, x2 if required
   */
  if (x1 > x2) {
    xtmp = x1;
    x1 = x2;
    x2 = xtmp;
  }

  /*
   * Clip x
   */
  if (x1 < left) {
    x1 = left;
  }
  if (x2 > right) {
    x2 = right;
  }

  /*
   * Calculate width
   */
  w = x2 - x1;

  /*
   * Sanity check on width
   */
  if (w < 0) {
    return (0);
  }

  /*
   * More variable setup
   */
  dx = w;
  pixx = dst->format->BytesPerPixel;
  pixy = dst->pitch;
  pixel = ((Uint8 *)dst->pixels) + pixx * (int)x1 + pixy * (int)y;

  // find out a what index the texture starts
  // texture_start = (abs(y + texture_dy) % texture->h) * texture->w;

  texture_start = (y + texture_dy) % texture->h;
  if (texture_start < 0) {
    texture_start = texture->h + texture_start;
  }
  // printf("texture start (h=)%i %i\n",texture_start,texture->h);
  // texture_start =texture_start * texture->pitch;
  texture_start = texture_start * texture->pitch / pixx;

  texture_x_walker = (x1 - texture_dx) % texture->w;
  if (texture_x_walker < 0) {
    texture_x_walker = texture->w + texture_x_walker;
  }
  texture_start_pixel = &((Uint8 *)texture->pixels)[texture_start * pixx];
  pixellast = pixel + w * pixx;

  // method 2 faster
  // option 1 the whole line fits in one draw
  if (w <= texture->w - texture_x_walker) {
    memcpy(pixel,
           &((Uint8 *)texture_start_pixel)[texture_x_walker * pixx],
           w * pixx);
  } else {
    int pixels_written = texture->w - texture_x_walker;

    // draw the first segment
    memcpy(pixel,
           &((Uint8 *)texture_start_pixel)[texture_x_walker * pixx],
           pixels_written * pixx);
    int write_width = texture->w;

    // now draw the rest
    while (pixels_written < w) {
      if (write_width >= w - pixels_written) {
        write_width = w - pixels_written;
      }
      memcpy(&((Uint8 *)pixel)[pixels_written * pixx],
             texture_start_pixel,
             write_width * pixx);
      pixels_written += write_width;
    }
  }
  /*
     //methods 1
     for (; pixel <= pixellast; pixel += pixx) {
     *(Uint16 *) pixel = ((Uint16*)texture_start_pixel)[texture_x_walker] ;
     texture_x_walker++;
     if (texture_x_walker >= texture->w ){ texture_x_walker=0; }
     }
     break;
   */

  /*
   * Set result code
   */
  result = 0;
  return (result);
}

int DrawLibSDLgfx::xx_texturedPolygon(SDL_Surface *dst,
                                      const Sint16 *vx,
                                      const Sint16 *vy,
                                      int n,
                                      SDL_Surface *texture,
                                      int texture_dx,
                                      int texture_dy) {
  int result;
  int i;
  int y, xa, xb;
  int minx, maxx, miny, maxy;
  int x1, y1;
  int x2, y2;
  int ind1, ind2;
  int ints;

  /*
   * Sanity check
   */
  if (n < 3) {
    return -1;
  }

  /*
   * Allocate temp array, only grow array
   */
  if (!gfxPrimitivesPolyAllocated) {
    gfxPrimitivesPolyInts = (int *)malloc(sizeof(int) * n);
    gfxPrimitivesPolyAllocated = n;
  } else {
    if (gfxPrimitivesPolyAllocated < n) {
      gfxPrimitivesPolyInts =
        (int *)realloc(gfxPrimitivesPolyInts, sizeof(int) * n);
      gfxPrimitivesPolyAllocated = n;
    }
  }

  miny = vy[0];
  maxy = vy[0];
  minx = vx[0];
  maxx = vx[0];
  for (i = 1; (i < n); i++) {
    if (vy[i] < miny) {
      miny = vy[i];
    } else if (vy[i] > maxy) {
      maxy = vy[i];
    }
    if (vx[i] < minx) {
      minx = vx[i];
    } else if (vx[i] > maxx) {
      maxx = vx[i];
    }
  }
  if (maxx < 0 || minx > dst->w) {
    return 0;
  }
  if (maxy < 0 || miny > dst->h) {
    return 0;
  }

  SDL_LockSurface(dst);
  /*
   * Draw, scanning y
   */
  result = 0;
  for (y = miny; (y <= maxy); y++) {
    ints = 0;
    for (i = 0; (i < n); i++) {
      if (!i) {
        ind1 = n - 1;
        ind2 = 0;
      } else {
        ind1 = i - 1;
        ind2 = i;
      }
      y1 = vy[ind1];
      y2 = vy[ind2];
      if (y1 < y2) {
        x1 = vx[ind1];
        x2 = vx[ind2];
      } else if (y1 > y2) {
        y2 = vy[ind1];
        y1 = vy[ind2];
        x2 = vx[ind1];
        x1 = vx[ind2];
      } else {
        continue;
      }
      if (((y >= y1) && (y < y2)) || ((y == maxy) && (y > y1) && (y <= y2))) {
        gfxPrimitivesPolyInts[ints++] =
          ((65536 * (y - y1)) / (y2 - y1)) * (x2 - x1) + (65536 * x1);
      }
    }

    QSORT(int, gfxPrimitivesPolyInts, ints, islt);

    // qsort(gfxPrimitivesPolyInts, ints, sizeof(int),
    //          xx_gfxPrimitivesCompareInt);

    for (i = 0; (i < ints); i += 2) {
      xa = gfxPrimitivesPolyInts[i] + 1;
      xa = (xa >> 16) + ((xa & 32768) >> 15);
      xb = gfxPrimitivesPolyInts[i + 1] - 1;
      xb = (xb >> 16) + ((xb & 32768) >> 15);
      result |=
        xx_texturedHLine(dst, xa, xb, y, texture, texture_dx, texture_dy);
    }
  }

  SDL_UnlockSurface(dst);
  return (result);
}

int DrawLibSDLgfx::xx_filledPolygonColor(SDL_Surface *dst,
                                         const Sint16 *vx,
                                         const Sint16 *vy,
                                         int n,
                                         Uint32 color) {
  int result;
  int i;
  int y, xa, xb;
  int miny, maxy;
  int x1, y1;
  int x2, y2;
  int ind1, ind2;
  int ints;

  /*
   * Sanity check
   */
  if (n < 3) {
    return -1;
  }

  /*
   * Allocate temp array, only grow array
   */
  if (!gfxPrimitivesPolyAllocated) {
    gfxPrimitivesPolyInts = (int *)malloc(sizeof(int) * n);
    gfxPrimitivesPolyAllocated = n;
  } else {
    if (gfxPrimitivesPolyAllocated < n) {
      gfxPrimitivesPolyInts =
        (int *)realloc(gfxPrimitivesPolyInts, sizeof(int) * n);
      gfxPrimitivesPolyAllocated = n;
    }
  }

  /*
   * Determine Y maxima
   */
  miny = vy[0];
  maxy = vy[0];
  for (i = 1; (i < n); i++) {
    if (vy[i] < miny) {
      miny = vy[i];
    } else if (vy[i] > maxy) {
      maxy = vy[i];
    }
  }

  /*
   * Draw, scanning y
   */
  result = 0;
  for (y = miny; (y <= maxy); y++) {
    ints = 0;
    for (i = 0; (i < n); i++) {
      if (!i) {
        ind1 = n - 1;
        ind2 = 0;
      } else {
        ind1 = i - 1;
        ind2 = i;
      }
      y1 = vy[ind1];
      y2 = vy[ind2];
      if (y1 < y2) {
        x1 = vx[ind1];
        x2 = vx[ind2];
      } else if (y1 > y2) {
        y2 = vy[ind1];
        y1 = vy[ind2];
        x2 = vx[ind1];
        x1 = vx[ind2];
      } else {
        continue;
      }
      if (((y >= y1) && (y < y2)) || ((y == maxy) && (y > y1) && (y <= y2))) {
        gfxPrimitivesPolyInts[ints++] =
          ((65536 * (y - y1)) / (y2 - y1)) * (x2 - x1) + (65536 * x1);
      }
    }

    //      qsort(gfxPrimitivesPolyInts, ints, sizeof(int),
    //      gfxPrimitivesCompareInt);
    QSORT(int, gfxPrimitivesPolyInts, ints, islt);

    for (i = 0; (i < ints); i += 2) {
      xa = gfxPrimitivesPolyInts[i] + 1;
      xa = (xa >> 16) + ((xa & 32768) >> 15);
      xb = gfxPrimitivesPolyInts[i + 1] - 1;
      xb = (xb >> 16) + ((xb & 32768) >> 15);
      result |= hlineColor(dst, xa, xb, y, color);
    }
  }

  return (result);
}

#endif
