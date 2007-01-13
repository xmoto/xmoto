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

#ifdef ENABLE_SDLGFX
#include "SDL_gfxPrimitives.h"
#include "SDL_rotozoom.h"
#include "iqsort.h"
//#define islt(a,b) ((*(const int *)a) - (*(const int *)b))
#define islt(a,b) ((*a)<(*b))


namespace vapp {
  int xx_gfxPrimitivesCompareInt(const void *a, const void *b) {
    return (*(const int *)a) - (*(const int *)b);
  };

DrawLibSDLgfx::DrawLibSDLgfx():DrawLib() {
    m_scale.x = 1;
    m_scale.y = 1;
    m_translate.x = 0;
    m_translate.y = 0;
    m_bg_data = NULL;
    gfxPrimitivesPolyInts = NULL;
    gfxPrimitivesPolyAllocated = 0;
    m_int_drawing_points_x = NULL;
    m_int_drawing_points_y = NULL;
    m_int_drawing_points_allocated = 0;
    m_min.x = 0;
    m_min.y = 0;
    m_max.x = 0;
    m_max.y = 0;
  };

  DrawLibSDLgfx::~DrawLibSDLgfx() {
  };


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

  void DrawLibSDLgfx::setClipRect(int x, int y, int w, int h) {
    SDL_Rect *rect = new SDL_Rect();

    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = h;
    setClipRect(rect);
  }

  void DrawLibSDLgfx::setClipRect(SDL_Rect * clip_rect) {
    SDL_SetClipRect(m_screen, clip_rect);
  }

  void DrawLibSDLgfx::getClipRect(int *px, int *py, int *pnWidth,
				  int *pnHeight) {
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

  void DrawLibSDLgfx::setLineWidth(float width) {
  }

  void DrawLibSDLgfx::init(int nDispWidth, int nDispHeight, int nDispBPP,
			   bool bWindowed, Theme * ptheme) {

    /* Set suggestions */
    m_nDispWidth = nDispWidth;
    m_nDispHeight = nDispHeight;
    m_nDispBPP = nDispBPP;
    m_bWindowed = bWindowed;

    /* Get some video info */
    const SDL_VideoInfo *pVidInfo = SDL_GetVideoInfo();

    if (pVidInfo == NULL)
      throw Exception("(1) SDL_GetVideoInfo : " +
		      std::string(SDL_GetError()));

    /* Determine target bit depth */
    if (m_bWindowed)
      /* In windowed mode we can't tinker with the bit-depth */
      m_nDispBPP = pVidInfo->vfmt->BitsPerPixel;

    int nFlags = SDL_SRCALPHA;

    if (!m_bWindowed)
      nFlags |= SDL_FULLSCREEN;
    /* At last, try to "set the video mode" */
    if ((m_screen =
	 SDL_SetVideoMode(m_nDispWidth, m_nDispHeight, m_nDispBPP,
			  nFlags)) == NULL) {
      Log
	("** Warning ** : Tried to set video mode %dx%d @ %d-bit, but SDL responded: %s\n"
	 "                Now SDL will try determining a proper mode itself.",
	 m_nDispWidth, m_nDispHeight, m_nDispBPP);

      /* Hmm, try letting it decide the BPP automatically */
      if ((m_screen =
	   SDL_SetVideoMode(m_nDispWidth, m_nDispHeight, 0,
			    nFlags)) == NULL) {
	/* Still no luck */
	Log
	  ("** Warning ** : Still no luck, now we'll try 800x600 in a window.");
	m_nDispWidth = 800;
	m_nDispHeight = 600;
	m_bWindowed = true;
	if ((m_screen =
	     SDL_SetVideoMode(m_nDispWidth, m_nDispHeight, 0,
			      SDL_SRCALPHA)) == NULL) {
	  throw Exception("SDL_SetVideoMode : " +
			  std::string(SDL_GetError()));
	}
      }
    }

    /* Retrieve actual configuration */
    pVidInfo = SDL_GetVideoInfo();
    if (pVidInfo == NULL)
      throw Exception("(2) SDL_GetVideoInfo : " +
		      std::string(SDL_GetError()));

    m_nDispBPP = pVidInfo->vfmt->BitsPerPixel;


    /* Enable unicode translation and key repeats */
    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
			SDL_DEFAULT_REPEAT_INTERVAL);

    Log("Using SDL_gfx implementation");

    //setBackgroundColor(0,0,40,255);

    /* Init the Text drawing library */
    _InitTextRendering(ptheme);

  }

  void DrawLibSDLgfx::unInit() {
  }



  /*===========================================================================
  Check for OpenGL extension
  ===========================================================================*/
  bool DrawLibSDLgfx::isExtensionSupported(std::string Ext) {
    return false;
  }

    /*===========================================================================
  Grab screen contents
  ===========================================================================*/
  Img *DrawLibSDLgfx::grabScreen(void) {
    Img *pImg = new Img;

    pImg->createEmpty(m_nDispWidth, m_nDispHeight);
    Color *pPixels = pImg->getPixels();

    Uint8 red, green, blue, alpha;

    SDL_LockSurface(m_screen);

    for (int y = 0; y < m_screen->h; y++) {
      for (int x = 0; x < m_screen->w; x++) {
	void *pixel =
	  (Uint8 *) m_screen->pixels +
	  (x * m_screen->format->BytesPerPixel) +
	  ((y) * (m_screen->pitch));
	SDL_GetRGB(*(Uint32 *) pixel, m_screen->format, &red, &green,
		   &blue);

	*(Uint32 *) pPixels = MAKE_COLOR(red, green, blue, 255);
	pPixels++;
      }
    }

    SDL_UnlockSurface(m_screen);

    return pImg;
  }

  void DrawLibSDLgfx::startDraw(DrawMode mode) {
    m_drawMode = mode;
    if (m_drawingPoints.size() != 0 || m_texturePoints.size() != 0) {
      printf
	("error drawingPoints.size(%i) of texturePoints.size(%i) was not 0 =%i\n",
	 m_drawingPoints.size(), m_texturePoints.size());
    }
  }

  void DrawLibSDLgfx::endDraw() {
    int size = m_drawingPoints.size();
    int x0, y0, x1, y1;
    SDL_Rect *clip = NULL;

    //get the current clipping on the screen
    //if null we must expect the be drawing 
    //on the full screen
    SDL_GetClipRect(m_screen, clip);
    if (clip != NULL) {
      x0 = clip->x;
      y0 = clip->y;
      x1 = x0 + clip->w;
      x1 = y0 + clip->h;
    } else {
      x0 = 0;
      y0 = 0;
      x1 = m_screen->w;
      y1 = m_screen->h;
    }

    bool draw = true;

    //if all the lines  are "smaller" or larger then the screen
    //we don't need to draw
    if (m_max.x < x0 || m_min.x > x1 || m_max.y < y0 || m_min.y > y1) {
      draw = false;
    }

    if (draw == true) {
      //keesj:TODO , why not just create one x/y array
      //and use that all the time. this is also true
      //for texture point and drawing points
      //it will also remove the need to clean the m_texturePoints 
      //and m_drawingPoints collection afterwards

      if (size > m_int_drawing_points_allocated) {
	if (m_int_drawing_points_allocated == 0) {
	  m_int_drawing_points_x =
	    (Sint16 *) malloc(sizeof(Sint16) * size * 2);
	  m_int_drawing_points_y =
	    (Sint16 *) malloc(sizeof(Sint16) * size * 2);
	  m_int_drawing_points_allocated = size * 2;
	} else {
	  m_int_drawing_points_x =
	    (Sint16 *) realloc(m_int_drawing_points_x,
			       sizeof(Sint16) *
			       m_int_drawing_points_allocated * 2);
	  m_int_drawing_points_y =
	    (Sint16 *) realloc(m_int_drawing_points_y,
			       sizeof(Sint16) *
			       m_int_drawing_points_allocated * 2);
	  m_int_drawing_points_allocated *= 2;
	}
      }


      for (int i = 0; i < size; i++) {
	m_int_drawing_points_x[i] = m_drawingPoints.at(i)->x;
	m_int_drawing_points_y[i] = m_drawingPoints.at(i)->y;
      }
      switch (m_drawMode) {
      case DRAW_MODE_POLYGON:
	if (m_texture != NULL && m_texturePoints.size() > 2
	    && m_drawingPoints.size() > 2) {
	  //texture points texture point 1 2 and 3
	  Vector2f t1, t2, t3;

	  //polygon points 1 2 3
	  Vector2f p1, p2, p3;

	  //texture line 1 2  and texture diagonal line 1
	  Vector2f tl1, tl2, tdl1;

	  //polygon line 1 2  and diagonal 1
	  Vector2f pl1, pl2, pdl1;

	  //the texture verctor texture points are expected to be given in 
	  //values between 0 and 1 where 1 it a full with of the texture
	  Vector2f texture_v =
	    Vector2f(m_texture->surface->w, m_texture->surface->h);

	  ///anchor point for the texture
	  float x_start_pixel, y_start_pixel;

	  //we only support scaling , rotation and flipping of textures
	  //so we only need 3 points to determine the scale and rotation

	  //convert the texture points to pixel value by
	  //multiplying the point value with the texture size
	  t1 = *m_texturePoints.at(0) * texture_v;
	  t2 = *m_texturePoints.at(1) * texture_v;
	  t3 = *m_texturePoints.at(2) * texture_v;

	  //also get the 3 first points of the polygon
	  p1 = *m_drawingPoints.at(0);
	  p2 = *m_drawingPoints.at(1);
	  p3 = *m_drawingPoints.at(2);

	  //convert the texture points to line vector
	  //that is the value of the l* describes a line 
	  //because we substract the two points
	  //for a polygon we at least need 3 points
	  //I try to think of the polygon as a rectangle
	  //this is why the two first lines are called tl 
	  //and the third one texture diagonal line
	  tl1 = t2 - t1;
	  tl2 = t3 - t2;
	  tdl1 = t1 - t3;

	  pl1 = p2 - p1;
	  pl2 = p3 - p2;
	  pdl1 = p1 - p3;

	  float p1Scale = pl1.length() / tl1.length();
	  float p2Scale = pl2.length() / tl2.length();
	  float p3Scale = pdl1.length() / tdl1.length();

	  float x_zoom, y_zoom, angle, angle2;


	  //know
	  //the amount of zoom that the lines must perform

	  angle = tl1.angle() - pl1.angle();
	  //TODO check???
	  angle2 = tdl1.angle() - pdl1.angle();

	  //we now have two scales and two angles
	  //and we are going to try to define a scaling that can match both
	  //for scaling we can say that (acording to pythagoras)
	  //scale ^2 = dx_scale^2 + dy_scale^2
	  //and because we have two line and those lines must use the same images
	  //we can say
	  //line1_x_scale ^2 + line1_y_scale ^2 = line2_x_scale + ^2 line2_y_scale ^2
	  //determine the x and y factor of the first angle
	  //x_zoom = sin(angle1 * 3.14159265/ 180) * p1Scale + cos(angle1 * 3.14159265/ 180) * p2Scale;
	  //y_zoom = sin(angle1 * 3.14159265/ 180) * p1Scale + cos(angle1 * 3.14159265/ 180) * p2Scale;

	  if (angle == 0) {
	    if (tl1.angle() == 0 || tl1.angle() == 180) {
	      x_zoom = p1Scale;
	      y_zoom = p2Scale;
	    } else {
	      x_zoom = p2Scale;
	      y_zoom = p1Scale;
	    }
	  } else {
	    x_zoom = p1Scale;
	    y_zoom = p2Scale;
	  }

	  float fff = ((int)(pl1.angle() - pl2.angle()) + 360) % 360;
	  float ggg = ((int)(tl1.angle() - tl2.angle()) + 360) % 360;

	  //keesj:misterious code to determin if the texure
	  //is following the vertex points.and if not reverse
	  //this makes the bike look left and right
	  if (fff > 180 && ggg < 180) {
	    angle += 180;
	    x_zoom = -x_zoom;
	  }
//          //SDL does not support very large images this code is located in SDL_surface.c line 48
//          //
//          //        /* Next time I write a library like SDL, I'll use int for size. :) */
//          //        if ( width >= 16384 || height >= 65536 ) {
//          //                SDL_SetError("Width or height is too large");
//          //               return(NULL);
//          //        }
	  if (x_zoom * m_texture->surface->w < 16384
	      && y_zoom * m_texture->surface->h < 65536) {
	    char key[255] = "";

	    if (m_drawingPoints.size() > 4) {
	      angle = 0;
	    }
	    sprintf(key, "rotate-%s_%.0f_%.2f_%.2f",
		    m_texture->Name.c_str(), angle, x_zoom, y_zoom);
	    //sprintf(key, "rotate-%s_%i_%i_%i", m_texture->Name.c_str(), (int)angle, (int)60 * x_zoom , (int)60 * y_zoom );
	    SDL_Surface *s = NULL;
	    std::map < const char *,
	      SDL_Surface * >::iterator i = m_image_cache.find(key);
	    if (i != m_image_cache.end()) {
	      s = (*i).second;
	    } else {
	      char *keyName = (char *)malloc(strlen(key) + 1);

	      //printf("addding image with key %s\n", key);
	      SDL_Surface *s1 =
		rotozoomSurfaceXY(m_texture->surface, 0, x_zoom,
				  y_zoom, SMOOTHING_OFF);
	      SDL_Surface *s2 =
		rotozoomSurfaceXY(s1, angle, 1, 1, SMOOTHING_ON);

	      if (m_texture->isAlpha) {
		s = SDL_DisplayFormatAlpha(s2);
	      } else {
		s =
		  SDL_ConvertSurface(s2, m_screen->format, SDL_HWSURFACE);
	      }

	      SDL_FreeSurface(s1);
	      SDL_FreeSurface(s2);
	      strcpy(keyName, key);
	      m_image_cache.insert(std::make_pair <> (keyName, s));
	    }

	    Vector2f middle = (p1 + p3) / 2;
	    Vector2f middle2 =
	      (*m_texturePoints.at(0) + *m_texturePoints.at(2)) / 2;

	    x_start_pixel = middle.x - (s->w * middle2.x);
	    y_start_pixel = middle.y - (s->h * middle2.y);



	    if (m_texture->Name == "XMOTO") {
	      printf("-----------------%s---------------\n",
		     m_texture->Name.c_str());
	      printf("texture\t%f %f %f %f %f %f\n", t1.x, t1.y,
		     t2.x, t2.y, t3.x, t3.y);
	      printf("drawing points %f %f %f %f %f %f\n", p1.x, p1.y,
		     p2.x, p2.y, p3.x, p3.y);
	      printf("scale\t%f %f %f \n", p1Scale, p2Scale, p3Scale);
	      printf("angle\t%f %f %f \n", tl1.angle(),
		     tl2.angle(), tdl1.angle());
	      printf("angle  point\t%f %f %f \n", pl1.angle(),
		     pl2.angle(), pdl1.angle());
	      printf("angle  total\t%f %f %f \n",
		     tl1.angle() - pl1.angle(), tl2.angle() - pl2.angle(),
		     tdl1.angle() - pdl1.angle());
	      printf("tr\t%f %f \n", x_start_pixel, y_start_pixel);
	      printf("zoom\t%f %f \n", x_zoom, y_zoom);
	      printf("rotate\t%f %f\n", angle, angle2);

	      printf("starx,starty\t%f %f\n", x_start_pixel,
		     y_start_pixel);
	      printf("direction\t%f \n", pl1.angle() - pl2.angle());
	      printf("direction2\t%f \n", fff);
	    }
	    if (m_texture->isAlpha) {
	      xx_texturedPolygonAlpha(m_screen, m_int_drawing_points_x,
				      m_int_drawing_points_y, size, s,
				      x_start_pixel, -y_start_pixel);
	    } else {
	      xx_texturedPolygon(m_screen, m_int_drawing_points_x,
				 m_int_drawing_points_y, size, s,
				 x_start_pixel, -y_start_pixel);
	    }
	  } else {
	    //zoom to large
	    printf("Zoom to large\n");
	  }

	} else {
	  xx_filledPolygonColor(m_screen, m_int_drawing_points_x,
				m_int_drawing_points_y, size, m_color);
	}

	break;
      case DRAW_MODE_LINE_LOOP:
	polygonRGBA(m_screen, m_int_drawing_points_x,
		    m_int_drawing_points_y, size, GET_RED(m_color),
		    GET_GREEN(m_color), GET_BLUE(m_color),
		    GET_ALPHA(m_color));
	break;
      case DRAW_MODE_LINE_STRIP:
	for (int f = 0; f < size - 1; f++) {
	  lineRGBA(m_screen, m_int_drawing_points_x[f],
		   m_int_drawing_points_y[f],
		   m_int_drawing_points_x[f + 1],
		   m_int_drawing_points_y[f + 1], GET_RED(m_color),
		   GET_GREEN(m_color), GET_BLUE(m_color),
		   GET_ALPHA(m_color));
	}
	break;
      }
    }
    for (int i = 0; i < m_drawingPoints.size(); i++) {
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

  void DrawLibSDLgfx::setColor(Color color) {
    m_color = color;
  }

  void DrawLibSDLgfx::setTexture(Texture * texture, BlendMode blendMode) {
    m_texture = texture;
  }

  void DrawLibSDLgfx::setBlendMode(BlendMode blendMode) {
  }



  /*===========================================================================
  Text dim probing
  ===========================================================================*/
  int DrawLibSDLgfx::getTextHeight(std::string Text) {
    int cx = 0, cy = 0, c;
    int h = 0;

    for (unsigned int i = 0; i < Text.size(); i++) {
      c = Text[i];
      if (c == ' ') {
	cx += 8;
      } else if (c == '\r') {
	cx = 0;
      } else if (c == '\n') {
	cx = 0;
	cy += 12;
      } else {
	cx += 8;
      }
      if (cy > h)
	h = cx;
    }
    return h + 12;
  }

  int DrawLibSDLgfx::getTextWidth(std::string Text) {
    int cx = 0, cy = 0, c;
    int w = 0;

    for (unsigned int i = 0; i < Text.size(); i++) {
      c = Text[i];
      if (c == ' ') {
	cx += 8;
      } else if (c == '\r') {
	cx = 0;
      } else if (c == '\n') {
	cx = 0;
	cy += 12;
      } else {
	cx += 8;
      }
      if (cx > w)
	w = cx;
    }
    return w;
  }

  /*===========================================================================
  Text drawing
  ===========================================================================*/
  void DrawLibSDLgfx::drawText(const Vector2f & Pos, std::string Text,
			       Color Back, Color Front, bool bEdge) {
    if (bEdge) {
      /* SLOOOOW */
      drawText(Pos + Vector2f(1, 1), Text, 0, MAKE_COLOR(0, 0, 0, 255),
	       false);
      drawText(Pos + Vector2f(-1, 1), Text, 0, MAKE_COLOR(0, 0, 0, 255),
	       false);
      drawText(Pos + Vector2f(1, -1), Text, 0, MAKE_COLOR(0, 0, 0, 255),
	       false);
      drawText(Pos + Vector2f(-1, -1), Text, 0, MAKE_COLOR(0, 0, 0, 255),
	       false);
    }

    int cx = (int)Pos.x, cy = (int)Pos.y, c;


    int nCharsPerLine = 256 / 8;

    for (unsigned int i = 0; i < Text.size(); i++) {
      c = Text[i];
      if (c == ' ') {

	startDraw(DRAW_MODE_POLYGON);
	setColor(Back);
	glVertexSP(cx, cy);
	glVertexSP(cx + 8, cy);
	glVertexSP(cx + 8, cy + 12);
	glVertexSP(cx, cy + 12);
	endDraw();
	cx += 8;
      } else if (c == '\r') {
	cx = (int)Pos.x;
      } else if (c == '\n') {
	cx = (int)Pos.x;
	cy += 12;
      } else {
	int y1 = (c / nCharsPerLine) * 12;
	int x1 = (c % nCharsPerLine) * 8;
	int y2 = y1 + 12;
	int x2 = x1 + 8;

	startDraw(DRAW_MODE_POLYGON);
	setColor(Back);
	glVertexSP(cx, cy);
	glVertexSP(cx + 8, cy);
	glVertexSP(cx + 8, cy + 12);
	glVertexSP(cx, cy + 12);
	endDraw();
	if (m_pDefaultFontTex == NULL) {
	  printf("Can't draw with empy texture!\n");
	}
	setTexture(m_pDefaultFontTex, BLEND_MODE_A);
	startDraw(DRAW_MODE_POLYGON);
	setColor(Front);
	glTexCoord((float)x1 / 256.0f, (float)y1 / 256.0f);

	glVertexSP(cx, cy);
	glTexCoord((float)x2 / 256.0f, (float)y1 / 256.0f);
	glVertexSP(cx + 8, cy);
	glTexCoord((float)x2 / 256.0f, (float)y2 / 256.0f);
	glVertexSP(cx + 8, cy + 12);
	glTexCoord((float)x1 / 256.0f, (float)y2 / 256.0f);
	glVertexSP(cx, cy + 12);
	endDraw();
	cx += 8;
      }
    }
  }

  /*===========================================================================
  Init of text rendering
  ===========================================================================*/
  void DrawLibSDLgfx::_InitTextRendering(Theme * p_theme) {
    m_pDefaultFontTex = p_theme->getDefaultFont();
  }

  /*===========================================================================
  Uninit of text rendering
  ===========================================================================*/
  void DrawLibSDLgfx::_UninitTextRendering(Theme * p_theme) {
  }

  void DrawLibSDLgfx::clearGraphics() {
    SDL_LockSurface(m_screen);
    if (m_bg_data == NULL) {
      m_bg_data =
	malloc(m_screen->format->BytesPerPixel * m_screen->w *
	       m_screen->h);
      memset(m_bg_data, 0,
	     m_screen->format->BytesPerPixel * m_screen->w * m_screen->h);
    }
    memcpy(m_screen->pixels, m_bg_data,
	   m_screen->format->BytesPerPixel * m_screen->w * m_screen->h);
    SDL_UnlockSurface(m_screen);
    //SDL_FillRect(m_screen, NULL, SDL_MapRGB(m_screen->format, 0, 0, 0));
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
    SDL_UpdateRect(m_screen, 0, 0, 0, 0);
  }


  int DrawLibSDLgfx::xx_texturedHLineAlpha(SDL_Surface * dst, Sint16 x1,
					   Sint16 x2, Sint16 y,
					   SDL_Surface * texture,
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
    //setup the source rectangle  we are only drawing one horizontal line
    source_rect.y = texture_y_start;
    source_rect.x = texture_x_walker;
    source_rect.h = 1;
    //we will draw to the current y
    dst_rect.y = y;

    //if there are enough pixels left in the current row of the texture
    //draw it all at once
    if (w <= texture->w - texture_x_walker) {
      source_rect.w = w;
      source_rect.x = texture_x_walker;
      dst_rect.x = x1;
      result != SDL_BlitSurface(texture, &source_rect, dst, &dst_rect);
    } else {			//we need to draw multiple times
      //draw the first segment
      pixels_written = texture->w - texture_x_walker;
      source_rect.w = pixels_written;
      source_rect.x = texture_x_walker;
      dst_rect.x = x1;
      result != SDL_BlitSurface(texture, &source_rect, dst, &dst_rect);
      write_width = texture->w;

      //now draw the rest
      //set the source x to 0
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
 * Draws a polygon filled with the given texture. this operation use SDL_BlitSurface. It supports
 * alpha drawing.
 * to get the best performance of this operation you need to make sure the texture and the dst surface have the same format
 * see  http://docs.mandragor.org/files/Common_libs_documentation/SDL/SDL_Documentation_project_en/sdlblitsurface.html
 *
 * dest the destination surface, 
 * vx array of x vector components
 * vy array of x vector components
 * n the amount of vectors in the vx and vy array
 * texture the sdl surface to use to fill the polygon
 * texture_dx the offset of the texture relative to the screeen. if you move the polygon 10 pixels to the left and want the texture
 * to apear the same you need to increase the texture_dx value
 * texture_dy see texture_dx
 **/
  int DrawLibSDLgfx::xx_texturedPolygonAlpha(SDL_Surface * dst,
					     const Sint16 * vx,
					     const Sint16 * vy, int n,
					     SDL_Surface * texture,
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
	if (((y >= y1) && (y < y2))
	    || ((y == maxy) && (y > y1) && (y <= y2))) {
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
	  xx_texturedHLineAlpha(dst, xa, xb, y, texture, texture_dx,
				texture_dy);
      }
    }


    return (result);
  }

  int DrawLibSDLgfx::xx_texturedHLine(SDL_Surface * dst, Sint16 x1,
				      Sint16 x2, Sint16 y,
				      SDL_Surface * texture,
				      int texture_dx, int texture_dy) {
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
    pixel = ((Uint8 *) dst->pixels) + pixx * (int)x1 + pixy * (int)y;


    //find out a what index the texture starts
    //texture_start = (abs(y + texture_dy) % texture->h) * texture->w;

    texture_start = (y + texture_dy) % texture->h;
    if (texture_start < 0) {
      texture_start = texture->h + texture_start;
    }
    //printf("texture start (h=)%i %i\n",texture_start,texture->h);
    //texture_start =texture_start * texture->pitch;
    texture_start = texture_start * texture->pitch / pixx;

    texture_x_walker = (x1 - texture_dx) % texture->w;
    if (texture_x_walker < 0) {
      texture_x_walker = texture->w + texture_x_walker;
    }
    texture_start_pixel =
      &((Uint8 *) texture->pixels)[texture_start * pixx];
    pixellast = pixel + w * pixx;

    //method 2 faster 
    //option 1 the whole line fits in one draw
    if (w <= texture->w - texture_x_walker) {
      memcpy(pixel,
	     &((Uint8 *) texture_start_pixel)[texture_x_walker * pixx],
	     w * pixx);
    } else {
      int pixels_written = texture->w - texture_x_walker;

      //draw the first segment
      memcpy(pixel,
	     &((Uint8 *) texture_start_pixel)[texture_x_walker * pixx],
	     pixels_written * pixx);
      int write_width = texture->w;

      //now draw the rest
      while (pixels_written < w) {
	if (write_width >= w - pixels_written) {
	  write_width = w - pixels_written;
	}
	memcpy(&((Uint8 *) pixel)[pixels_written * pixx],
	       texture_start_pixel, write_width * pixx);
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

  int DrawLibSDLgfx::xx_texturedPolygon(SDL_Surface * dst,
					const Sint16 * vx,
					const Sint16 * vy, int n,
					SDL_Surface * texture,
					int texture_dx, int texture_dy) {
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
	if (((y >= y1) && (y < y2))
	    || ((y == maxy) && (y > y1) && (y <= y2))) {
	  gfxPrimitivesPolyInts[ints++] =
	    ((65536 * (y - y1)) / (y2 - y1)) * (x2 - x1) + (65536 * x1);
	}

      }

      QSORT(int, gfxPrimitivesPolyInts, ints, islt);

      //qsort(gfxPrimitivesPolyInts, ints, sizeof(int),
//          xx_gfxPrimitivesCompareInt);

      for (i = 0; (i < ints); i += 2) {
	xa = gfxPrimitivesPolyInts[i] + 1;
	xa = (xa >> 16) + ((xa & 32768) >> 15);
	xb = gfxPrimitivesPolyInts[i + 1] - 1;
	xb = (xb >> 16) + ((xb & 32768) >> 15);
	result |=
	  xx_texturedHLine(dst, xa, xb, y, texture, texture_dx,
			   texture_dy);
      }
    }


    SDL_UnlockSurface(dst);
    return (result);
  }

  int DrawLibSDLgfx::xx_filledPolygonColor(SDL_Surface * dst,
					   const Sint16 * vx,
					   const Sint16 * vy, int n,
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
	if (((y >= y1) && (y < y2))
	    || ((y == maxy) && (y > y1) && (y <= y2))) {
	  gfxPrimitivesPolyInts[ints++] =
	    ((65536 * (y - y1)) / (y2 - y1)) * (x2 - x1) + (65536 * x1);
	}

      }

//      qsort(gfxPrimitivesPolyInts, ints, sizeof(int), gfxPrimitivesCompareInt);
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
}
#endif
