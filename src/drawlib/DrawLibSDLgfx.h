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

#ifndef __DRAWLIBSDLGFX_H__
#define __DRAWLIBSDLGFX_H__

#include "DrawLib.h"
class PolyDraw;

class DrawLibSDLgfx : public DrawLib {
public:
  DrawLibSDLgfx();
  virtual ~DrawLibSDLgfx();

  virtual void init(unsigned int nDispWidth,
                    unsigned int nDispHeight,
                    unsigned int nDispBPP,
                    bool bWindowed);
  virtual void unInit();

  virtual void glVertexSP(float x, float y);
  virtual void glVertex(float x, float y);

  // texture coordinate
  virtual void glTexCoord(float x, float y);
  virtual void screenProjVertex(float *x, float *y);

  virtual void setColor(Color color);
  /**
   * set the texture for drawing
   * the value may be NULL to disable texture
   * every end draw will reset the texture to NULL
   **/
  virtual void setTexture(Texture *texture, BlendMode blendMode);
  virtual void setBlendMode(BlendMode blendMode);

  /**
   * enables clipping and sets the clipping borders
   **/
  virtual void setClipRect(int x, int y, unsigned int w, unsigned int h);
  virtual void setClipRect(SDL_Rect *i_clip_rect);
  virtual void setScale(float x, float y);
  virtual void setTranslate(float x, float y);
  virtual void setMirrorY();
  virtual void setRotateZ(float i_angle);
  virtual void setLineWidth(float width);

  /**
   * returns the current screen clipping
   **/
  virtual void getClipRect(int *o_px,
                           int *o_py,
                           int *o_pnWidth,
                           int *o_pnHeight);

  /**
   * Start drawing ... used in combination with glVertex
   **/
  virtual void startDraw(DrawMode mode);

  /**
   * End draw
   **/
  virtual void endDraw();
  virtual void endDrawKeepProperties(); /* to keep textures, ... to render
                                           several times the same entity fastly
                                           */
  virtual void removePropertiesAfterEnd(); /* remove properties endDraw =
                                              endDrawKeepProperties +
                                              removePropertiesAfterEnd */

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

  virtual Img *grabScreen(int i_reduce = 1);
  virtual bool isExtensionSupported(std::string Ext);

  virtual FontManager *getFontManager(const std::string &i_fontFile,
                                      int i_fontSize);

private:
  int xx_texturedHLine(SDL_Surface *dst,
                       Sint16 x1,
                       Sint16 x2,
                       Sint16 y,
                       SDL_Surface *texture,
                       int texture_dx,
                       int texture_dy);
  int xx_texturedPolygon(SDL_Surface *dst,
                         const Sint16 *vx,
                         const Sint16 *vy,
                         int n,
                         SDL_Surface *texture,
                         int texture_dx,
                         int texture_dy);

  int xx_texturedHLineAlpha(SDL_Surface *dst,
                            Sint16 x1,
                            Sint16 x2,
                            Sint16 y,
                            SDL_Surface *texture,
                            int texture_dx,
                            int texture_dy);
  int xx_texturedPolygonAlpha(SDL_Surface *dst,
                              const Sint16 *vx,
                              const Sint16 *vy,
                              int n,
                              SDL_Surface *texture,
                              int texture_dx,
                              int texture_dy);

  int xx_filledPolygonColor(SDL_Surface *dst,
                            const Sint16 *vx,
                            const Sint16 *vy,
                            int n,
                            Uint32 color);

  // the mode used when drawing
  DrawMode m_drawMode;

  // the current scale
  Vector2f m_scale;

  // the current translate
  Vector2f m_translate;

  // data buffer for drawing the background
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

  // Vector for creating polygons
  std::vector<Vector2f *> m_drawingPoints;
  // Vector for keeping track of texture coordinates
  std::vector<Vector2f *> m_texturePoints;

  // map to hold converted images
  std::map<const char *, SDL_Surface *, ltstr> m_image_cache;

  // dynamic allocated array for the
  // that that is fed to the polygon filling
  // algorithms
  Sint16 m_int_drawing_points_x[100];
  Sint16 m_int_drawing_points_y[100];

  int *gfxPrimitivesPolyInts;
  int gfxPrimitivesPolyAllocated;
  PolyDraw *m_polyDraw;
  //    PolyDraw* m_polyDraw;
  int screenVerticles[100];
  int nPolyTextureVertices[100];
};

#endif
