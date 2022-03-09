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
#include "include/xm_OpenGL.h"

class DrawLibOpenGL : public DrawLib {
public:
  DrawLibOpenGL();
  virtual ~DrawLibOpenGL();

  virtual void init(unsigned int nDispWidth,
                    unsigned int nDispHeight,
                    bool bWindowed);
  virtual void unInit();

  virtual void glVertexSP(float x, float y);
  virtual void glVertex(float x, float y);

  virtual void setCameraDimensionality(CameraDimension dimension);

  // texture coordinate
  virtual void glTexCoord(float x, float y);
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
   * Flush the graphics. In memory graphics will now be displayed
   **/
  virtual void flushGraphics();

  virtual FontManager *getFontManager(const std::string &i_fontFile,
                                      unsigned int i_fontSize,
                                      unsigned int i_fixedFontSize = 0);

  virtual Img *grabScreen(int i_reduce = 1);
  virtual bool isExtensionSupported(std::string Ext);

private:
  SDL_GLContext m_glContext;
};

#endif
