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
#include "BuiltInFont.h"

#ifdef ENABLE_SDLGFX
#include "SDL_gfxPrimitives.h"
#include "SDL_rotozoom.h"
namespace vapp {
  int xx_gfxPrimitivesCompareInt(const void *a, const void *b) {
    return (*(const int *)a) - (*(const int *)b);
  };

  DrawLibSDLgfx::~DrawLibSDLgfx() {
    m_scale.x = 1;
    m_scale.y = 1;
    m_translate.x = 0;
    m_translate.y = 0;
    m_bg_data = NULL;
    gfxPrimitivesPolyInts = NULL;
    gfxPrimitivesPolyAllocated = 0;
  };

DrawLibSDLgfx::DrawLibSDLgfx():DrawLib() {
  };

  /*===========================================================================
  Transform an OpenGL vertex to pure 2D 
  ===========================================================================*/
  void DrawLibSDLgfx::glVertexSP(float x, float y) {
    m_drawingPoints.push_back(new Vector2f(x, y));
  }

  void DrawLibSDLgfx::glVertex(float x, float y) {
    m_drawingPoints.
      push_back(new
		Vector2f(m_nDrawWidth / 2 +
			 ((x + m_translate.x) * m_scale.x),
			 m_nDrawHeight / 2 -
			 ((y + m_translate.y) * m_scale.y)));
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
			      SDL_OPENGL)) == NULL) {
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

    Log("Using OPEN SDL_gfx implementation");

    //setBackgroundColor(0,0,40,255);

    /* Init the Text drawing library */
    _InitTextRendering(ptheme);

  }

  void DrawLibSDLgfx::unInit() {
  }
  /*===========================================================================
  Primitive: circle
  ===========================================================================*/
  void DrawLibSDLgfx::drawCircle(const Vector2f & Center, float fRadius,
				 float fBorder, Color Back, Color Front) {
    circleRGBA(m_screen, (Sint16) Center.x, (Sint16) Center.y,
	       (Sint16) fRadius, GET_RED(Back), GET_GREEN(Back),
	       GET_BLUE(Back), GET_ALPHA(Back));
  }

  /*===========================================================================
  Primitive: box
  ===========================================================================*/
  void DrawLibSDLgfx::drawBox(const Vector2f & A, const Vector2f & B,
			      float fBorder, Color Back, Color Front) {
    if (GET_ALPHA(Back) > 0) {
      boxRGBA(m_screen, A.x, A.y, B.x, B.y, GET_RED(Back),
	      GET_GREEN(Back), GET_BLUE(Back), GET_ALPHA(Back));
    }

    if (fBorder > 0.0f && GET_ALPHA(Front) > 0) {
      boxRGBA(m_screen, A.x, A.y, A.x + fBorder, B.y, GET_RED(Front),
	      GET_GREEN(Front), GET_BLUE(Front), GET_ALPHA(Front));
    }
  }

  /*===========================================================================
  Primitive: box
  ===========================================================================*/
  void DrawLibSDLgfx::drawImage(const Vector2f & a, const Vector2f & b,
				Texture * pTexture, Color tint) {
    setTexture(pTexture, BLEND_MODE_A);
    startDraw(DRAW_MODE_POLYGON);
    setColor(tint);
    glTexCoord(0.0, 0.0);
    glVertexSP(a.x, a.y);
    glTexCoord(1.0, 0.0);
    glVertexSP(b.x, a.y);
    glTexCoord(1.0, 1.0);
    glVertexSP(b.x, b.y);
    glTexCoord(0.0, 1.0);
    glVertexSP(a.x, b.y);
    endDraw();
    return;

    if (pTexture->surface != NULL) {
      SDL_Rect *dest = new SDL_Rect();
      if (m_scale.x != 1 < m_scale.x < 10) {
	dest->x =
	  m_nDrawWidth / 2 + (Sint16) ((a.x + m_translate.x) * m_scale.x);
	dest->y =
	  m_nDrawHeight / 2 - (Sint16) ((a.y + m_translate.y) * m_scale.y);
	int orig_w = (Sint16) abs(pTexture->surface->w * m_scale.y);
	int w = (Sint16) abs((b.x - a.x) * m_scale.x);
	int h = (Sint16) abs((b.y - a.y) * m_scale.y);
	double scalex = 1.0 * w / pTexture->surface->w;
	double scaley = 1.0 * h / pTexture->surface->h;

	char key[255] = "";
	sprintf(key, "%s_%i_%f_%f", pTexture->Name.c_str(), 0, scalex,
		scaley);
	SDL_Surface *s = NULL;
	std::map < const char *, SDL_Surface * >::iterator i =
	  m_image_cache.find(key);
	if (i != m_image_cache.end()) {
	  s = (*i).second;
	} else {
	  char *keyName = (char *)malloc(strlen(key) + 1);
	  s =
	    rotozoomSurfaceXY(pTexture->surface, 0, scalex, scaley,
			      SMOOTHING_ON);
	  if (scalex < 2) {	//only cache smaller images
	    strcpy(keyName, key);
	    printf("addding image with key %s\n", key);
	    m_image_cache.insert(std::make_pair <> (keyName, s));
	  }
	}
	dest->y -= s->h;
	SDL_BlitSurface(s, NULL /*copy all */ , m_screen, dest);
	if (scalex >= 2) {	// free image if it was not cached
	  SDL_FreeSurface(s);
	}
      } else {
	dest->x = (int)a.x;
	dest->y = (int)b.y;
	SDL_BlitSurface(pTexture->surface, NULL /*copy all */ , m_screen,
			dest);
      }
    } else {
      printf("texture surface was not loaded\n");
    }
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

    Sint16 x[size];
    Sint16 y[size];
    for (int i = 0; i < size; i++) {
      x[i] = m_drawingPoints.at(i)->x;
      y[i] = m_drawingPoints.at(i)->y;
    }
    switch (m_drawMode) {
    case DRAW_MODE_POLYGON:
      if (m_texture != NULL) {
	if (m_texturePoints.size() > 0) {
	  if (m_texturePoints.size() == 4 && m_drawingPoints.size() == 4) {
	    Vector2f *t1, *t2, *t3, *t4;
	    Vector2f *p1, *p2, *p3, *p4;
	    t1 = m_texturePoints.at(0);
	    t2 = m_texturePoints.at(1);
	    t3 = m_texturePoints.at(2);
	    t4 = m_texturePoints.at(3);
	    p1 = m_drawingPoints.at(0);
	    p2 = m_drawingPoints.at(1);
	    p3 = m_drawingPoints.at(2);
	    p4 = m_drawingPoints.at(3);
	    //lets first just try to draw the start
	    //of the pixmap on the right place
	    if (t1->x != t2->x && t2->y != t3->y && p1->y == p2->y) {
	      float dx_t = t2->x - t1->x;	//delta x of texture
	      float dx_p = p2->x - p1->x;	//delta x of pixels
	      int texture_w = m_texture->surface->w;
	      float x_pixel_usage = dx_t * texture_w;
	      float x_zoom_factor = dx_p / x_pixel_usage;
	      float x_start_pixel = (t1->x * texture_w) * x_zoom_factor;

	      float dy_t = t3->y - t2->y;	//delta x of texture
	      float dy_p = p3->y - p2->y;	//delta x of pixels
	      int texture_h = m_texture->surface->h;
	      float y_pixel_usage = dy_t * texture_h;
	      float y_zoom_factor = dy_p / y_pixel_usage;
	      float y_start_pixel = (t2->y * texture_w) * y_zoom_factor;

	      //x scale =
	      //  dx_t / w = dx_p / ff
	      if (texture_w == 1122) {
		printf("tex %s\n", m_texture->Name.c_str());
		for (int z = 0; z < m_texturePoints.size(); z++) {
		  printf("tex %i %3f %3f\n", z,
			 m_texturePoints.at(z)->x,
			 m_texturePoints.at(z)->y);
		}
		for (int z = 0; z < m_drawingPoints.size(); z++) {
		  printf("draw %i %3f %3f\n", z,
			 m_drawingPoints.at(z)->x,
			 m_drawingPoints.at(z)->y);
		}
		printf
		  ("dx dt %3f %3f == w %i %f pixels start_pixel %3f\n",
		   dx_p, dx_t, texture_w, x_pixel_usage, x_start_pixel);
		printf
		  ("dy p,t %3f %3f == w %i %f pixels start_pixel %3f\n",
		   dy_p, dy_t, texture_h, y_pixel_usage, y_start_pixel);

		printf("%f\n", y_zoom_factor);
	      }
	      if (x_zoom_factor < 30) {
		char key[255] = "";
		sprintf(key, "%s_%i_%.3f_%.3f",
			m_texture->Name.c_str(), 0,
			x_zoom_factor, y_zoom_factor);
		SDL_Surface *s = NULL;
		std::map < const char *,
		  SDL_Surface * >::iterator i = m_image_cache.find(key);
		if (i != m_image_cache.end()) {
		  s = (*i).second;
		} else {
		  char *keyName = (char *)malloc(strlen(key) + 1);
		  s =
		    rotozoomSurfaceXY(m_texture->surface, 0,
				      x_zoom_factor,
				      y_zoom_factor, SMOOTHING_ON);
		  strcpy(keyName, key);
		  printf("addding image with key %s\n", key);
		  m_image_cache.insert(std::make_pair <> (keyName, s));
		}
		//SDL_Surface * surface = rotozoomSurface(m_texture->surface,0,x_zoom_factor,1);
		//texturedPolygon(m_screen,x,y,size,surface,(int) p1->x -x_start_pixel ,(int) (p2->y + y_start_pixel));
		//texturedPolygon(m_screen,x,y,size,surface,p1->x + x_start_pixel,-p1->y);
		texturedPolygon(m_screen, x, y, size, s,
				-x_start_pixel + p1->x,
				y_start_pixel - p1->y);
	      } else {
		printf
		  ("skipping because zoom factor is to large %f\n",
		   x_zoom_factor);
	      }
	    } else {
	      texturedPolygon(m_screen, x, y, size,
			      m_texture->surface,
			      (int)m_texturePoints.at(0)->x,
			      (int)m_texturePoints.at(0)->y);
	    }
	  } else {
	    texturedPolygon(m_screen, x, y, size, m_texture->surface,
			    (int)m_texturePoints.at(0)->x,
			    (int)m_texturePoints.at(0)->y);
	  }
	} else {
	  texturedPolygon(m_screen, x, y, size, m_texture->surface, 0, 0);
	}
      } else {
	filledPolygonRGBA(m_screen, x, y, size, GET_RED(m_color),
			  GET_GREEN(m_color), GET_BLUE(m_color),
			  GET_ALPHA(m_color));
      }

      break;
    case DRAW_MODE_LINE_LOOP:
      polygonRGBA(m_screen, x, y, size, GET_RED(m_color),
		  GET_GREEN(m_color), GET_BLUE(m_color),
		  GET_ALPHA(m_color));
      break;
    case DRAW_MODE_LINE_STRIP:
      for (int f = 0; f < size - 1; f++) {
	lineRGBA(m_screen, x[f], y[f], x[f + 1], y[f + 1],
		 GET_RED(m_color), GET_GREEN(m_color),
		 GET_BLUE(m_color), GET_ALPHA(m_color));
      }
      break;
    }
    m_drawingPoints.resize(0);
    m_texturePoints.resize(0);
    m_drawMode = DRAW_MODE_NONE;
    m_texture = NULL;
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
    setClipRect(NULL);
    m_scale.x = 1;
    m_scale.y = 1;
    m_translate.x = 0;
    m_translate.y = 0;
    /*
       SDL_LockSurface(m_screen);
       if (m_bg_data ==NULL){
       m_bg_data = malloc(m_screen->format->BytesPerPixel * m_screen->w * m_screen->h);
       memset(m_bg_data,0,m_screen->format->BytesPerPixel * m_screen->w * m_screen->h);
       }
       memcpy(m_screen->pixels,m_bg_data,m_screen->format->BytesPerPixel * m_screen->w * m_screen->h);
       SDL_UnlockSurface(m_screen);
     */
    SDL_FillRect(m_screen, NULL, SDL_MapRGB(m_screen->format, 0, 0, 0));
  }

      /**
       * Flush the graphics. In memory graphics will now be displayed
       **/
  void DrawLibSDLgfx::flushGraphics() {
    SDL_UpdateRect(m_screen, 0, 0, 0, 0);
  }


  int DrawLibSDLgfx::xx_texturedHLine(SDL_Surface * dst, Sint16 x1,
				      Sint16 x2, Sint16 y,
				      SDL_Surface * texture,
				      int texture_dx, int texture_dy) {
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

      qsort(gfxPrimitivesPolyInts, ints, sizeof(int),
	    xx_gfxPrimitivesCompareInt);

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


    return (result);
  }
}
#endif
