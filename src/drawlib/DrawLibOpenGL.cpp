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
#include "DrawLibOpenGL.h"
#include "common/Image.h"
#include "common/VFileIO.h"
#include "common/VFileIO_types.h"
#include "common/VTexture.h"
#include "common/XMBuild.h"
#include "helpers/Log.h"
#include "helpers/RenderSurface.h"
#include "helpers/Singleton.h"
#include "helpers/utf8.h"
#include "include/xm_hashmap.h"
#include "xmscene/Camera.h"

#define UTF8_INTERLINE_SPACE 2
#define UTF8_INTERCHAR_SPACE 0

#ifdef ENABLE_OPENGL

class ScrapTextures : public Singleton<ScrapTextures> {
  friend class Singleton<ScrapTextures>;
  // from quake 2, Scrap_AllocBlock in gl_image.c
public:
  ScrapTextures();
  ~ScrapTextures();

  int allocateAndLoadTexture(unsigned int width,
                             unsigned int height,
                             float *ux,
                             float *uy,
                             float *vx,
                             float *vy,
                             SDL_Surface *data);

  bool isDirty();
  void update();

  void display(DrawLib *pDrawLib);

private:
#define MAX_SCRAPS 4
// we need 3dfx first gen compatibility, which has only 256x256 max
// texture size
#define BLOCK_WIDTH 256
#define BLOCK_HEIGHT 256

  // store the first available y
  unsigned int m_scrapsAllocated[MAX_SCRAPS][BLOCK_WIDTH];
  SDL_Surface *m_scrapsTexels[MAX_SCRAPS];
  bool m_scrapsUsed[MAX_SCRAPS];
  unsigned int m_scrapsTextures[MAX_SCRAPS];

  bool m_dirty;
};

class GLFontGlyphLetter;

class GLFontGlyph : public FontGlyph {
public:
  /* for simple glyph */
  GLFontGlyph(const std::string &i_value);

  /* a glyph from other glyphs */
  /*kejo:why not just grrr create a copy contructor*/
  GLFontGlyph(const std::string &i_value,
              HashNamespace::unordered_map<std::string, GLFontGlyphLetter *>
                &i_glyphsLetters);
  virtual ~GLFontGlyph();

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

  static int powerOf2(int i_value);
};

class GLFontGlyphLetter : public GLFontGlyph {
public:
  GLFontGlyphLetter(const std::string &i_value,
                    TTF_Font *i_ttf,
                    unsigned int i_fixedFontSize);
  virtual ~GLFontGlyphLetter();
  GLuint GLID() const;

  Vector2f m_u, m_v;

private:
  GLuint m_GLID;
  bool m_useScrap;
};

class GLFontManager : public FontManager {
public:
  GLFontManager(DrawLib *i_drawLib,
                const std::string &i_fontFile,
                unsigned int i_fontSize,
                unsigned int i_fixedFontSize = 0);
  virtual ~GLFontManager();

  FontGlyph *getGlyph(const std::string &i_string);
  FontGlyph *getGlyphTabExtended(const std::string &i_string);
  void printString(DrawLib *pDrawLib,
                   FontGlyph *i_glyph,
                   int i_x,
                   int i_y,
                   Color i_color,
                   float i_perCentered = -1.0,
                   bool i_shadowEffect = false);
  void printStringGrad(DrawLib *pDrawLib,
                       FontGlyph *i_glyph,
                       int i_x,
                       int i_y,
                       Color c1,
                       Color c2,
                       Color c3,
                       Color c4,
                       float i_perCentered = -1.0,
                       bool i_shadowEffect = false);
  void printStringGradOne(DrawLib *pDrawLib,
                          FontGlyph *i_glyph,
                          int i_x,
                          int i_y,
                          Color c1,
                          Color c2,
                          Color c3,
                          Color c4,
                          float i_perCentered = -1.0);

  virtual unsigned int nbGlyphsInMemory();
  virtual void displayScrap(DrawLib *pDrawLib);

private:
  std::vector<std::string> m_glyphsKeys;
  std::vector<GLFontGlyph *> m_glyphsValues;
  HashNamespace::unordered_map<std::string, GLFontGlyph *> m_glyphs;

  std::vector<std::string> m_glyphsLettersKeys;
  std::vector<GLFontGlyphLetter *> m_glyphsLettersValues;
  HashNamespace::unordered_map<std::string, GLFontGlyphLetter *>
    m_glyphsLetters;

  unsigned int getLonguestLineSize(const std::string &i_value,
                                   unsigned int i_start = 0,
                                   unsigned int i_nbLinesToRead = -1);
};

DrawLibOpenGL::~DrawLibOpenGL() {
  if (m_fontSmall != NULL) {
    delete m_fontSmall;
  }

  if (m_fontMedium != NULL) {
    delete m_fontMedium;
  }

  if (m_fontBig != NULL) {
    delete m_fontBig;
  }

  if (m_fontMonospace != NULL) {
    delete m_fontMonospace;
  }
  if (m_menuCamera != NULL) {
    delete m_menuCamera;
  }
}

DrawLibOpenGL::DrawLibOpenGL()
  : DrawLib() {
  m_fontSmall = getFontManager(
    XMFS::FullPath(FDT_DATA, FontManager::getDrawFontFile()), 14);
  m_fontMedium = getFontManager(
    XMFS::FullPath(FDT_DATA, FontManager::getDrawFontFile()), 22);
  m_fontBig = getFontManager(
    XMFS::FullPath(FDT_DATA, FontManager::getDrawFontFile()), 60);
  m_fontMonospace = getFontManager(
    XMFS::FullPath(FDT_DATA, FontManager::getMonospaceFontFile()), 12, 7);

  m_glContext = NULL;
};

/*===========================================================================
  Transform an OpenGL vertex to pure 2D
  ===========================================================================*/
void DrawLibOpenGL::glVertexSP(float x, float y) {
  glVertex2f(x, m_renderSurf->getDispHeight() - y);
}

void DrawLibOpenGL::glVertex(float x, float y) {
  glVertex2f(x, y);
}

void DrawLibOpenGL::glTexCoord(float x, float y) {
  glTexCoord2f(x, y);
}

void DrawLibOpenGL::setClipRect(int x, int y, unsigned int w, unsigned int h) {
  // glScissor(x, m_nDispHeight - (y+h), w, h);
  glScissor(x, m_renderSurf->upright().y - (y + h), w, h);

  m_nLScissorX = x;
  m_nLScissorY = y;
  m_nLScissorW = w;
  m_nLScissorH = h;
}

void DrawLibOpenGL::setClipRect(SDL_Rect *clip_rect) {
  if (clip_rect != NULL) {
    setClipRect(clip_rect->x, clip_rect->y, clip_rect->w, clip_rect->h);
  }
}

void DrawLibOpenGL::getClipRect(int *px, int *py, int *pnWidth, int *pnHeight) {
  *px = m_nLScissorX;
  *py = m_nLScissorY;
  *pnWidth = m_nLScissorW;
  *pnHeight = m_nLScissorH;
}

void DrawLibOpenGL::setScale(float x, float y) {
  glScalef(x, y, 1);
}
void DrawLibOpenGL::setTranslate(float x, float y) {
  glTranslatef(x, y, 0);
}

void DrawLibOpenGL::setMirrorY() {
  glRotatef(180, 0, 1, 0);
}

void DrawLibOpenGL::setRotateZ(float i_angle) {
  if (i_angle != 0.0) { /* not nice to compare a float, but the main case */
    glRotatef(i_angle, 0, 0, 1);
  }
}

void DrawLibOpenGL::setLineWidth(float width) {
  glLineWidth(width);
}

void DrawLibOpenGL::init(unsigned int nDispWidth,
                         unsigned int nDispHeight,
                         bool bWindowed) {
  DrawLib::init(nDispWidth, nDispHeight, bWindowed);

  /* Set suggestions */
  m_nDispWidth = nDispWidth;
  m_nDispHeight = nDispHeight;
  m_bWindowed = bWindowed;

  /* Get some video info */
  const SDL_RendererInfo *pVidInfo = NULL;

  const int displayIndex = 0;
  int displayModeCount = 0;
  if ((displayModeCount = SDL_GetNumDisplayModes(displayIndex)) < 1) {
    throw Exception("DrawLib: No display modes found.");
  }
  std::vector<SDL_DisplayMode> modes(displayModeCount);

  for (int modeIndex = 0; modeIndex < displayModeCount; ++modeIndex) {
    SDL_DisplayMode mode;
    if (SDL_GetDisplayMode(displayIndex, modeIndex, &mode) != 0) {
      throw Exception("getDisplayModes: SDL_GetDisplayMode failed: " +
                      std::string(SDL_GetError()));
    }
    modes[modeIndex] = mode;
  }

  /* Setup GL stuff */
  /* 2005-10-05 ... note that we no longer ask for explicit settings... it's
     better to do it per auto */
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  /* Create video flags */
  int nFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
  if (!m_bWindowed) {
    nFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  }

  /* At last, try to "set the video mode" */
  std::string title = std::string("X-Moto ") + XMBuild::getVersionString(true);
  if ((m_window = SDL_CreateWindow(title.c_str(),
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED,
                                   m_nDispWidth,
                                   m_nDispHeight,
                                   nFlags)) == NULL) {
    LogWarning("Tried with a resolution of %ix%i, but SDL failed (%s)",
               m_nDispWidth,
               m_nDispHeight,
               SDL_GetError());

    m_nDispWidth = 800;
    m_nDispHeight = 600;
    m_bWindowed = true;
    LogWarning("Trying %ix%i in windowed mode", m_nDispWidth, m_nDispHeight);

    if ((m_window = SDL_CreateWindow(title.c_str(),
                                     SDL_WINDOWPOS_UNDEFINED,
                                     SDL_WINDOWPOS_UNDEFINED,
                                     m_nDispWidth,
                                     m_nDispHeight,
                                     SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL)) ==
        NULL) {
      throw Exception("SDL_CreateWindow: " + std::string(SDL_GetError()));
    }
  }

  SDL_SetWindowMinimumSize(m_window, m_nDispWidth, m_nDispHeight);
  SDL_SetWindowMaximumSize(m_window, m_nDispWidth, m_nDispHeight);

  SDL_SetWindowTitle(m_window, title.c_str());

#if !defined(WIN32) && !defined(__APPLE__) && !defined(__amigaos4__)
  SDL_Surface *v_icon =
    SDL_LoadBMP((XMFS::getSystemDataDir() + std::string("/xmoto.bmp")).c_str());
  if (v_icon != NULL) {
    SDL_SetSurfaceBlendMode(v_icon, SDL_BLENDMODE_BLEND);
    SDL_SetColorKey(v_icon, SDL_TRUE, SDL_MapRGB(v_icon->format, 236, 45, 211));
    SDL_SetWindowIcon(m_window, v_icon);
    SDL_FreeSurface(v_icon);
  }
#endif

  SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

  /* Create an OpenGL context */
  SDL_GLContext m_glContext = SDL_GL_CreateContext(m_window);

  if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
    /* Try without SDL's loader */
    if (!gladLoadGL()) {
      const char *err = "Failed to load OpenGL functions!";
      LogError("%s", err);
      throw Exception(err);
    }
  }

  /* OpenGL fully initialized */

  m_menuCamera =
    new Camera(Vector2i(0, 0), Vector2i(m_nDispWidth, m_nDispHeight));
  m_menuCamera->setCamera2d();

  /* Output some general info */
  LogInfo("GL: %s (%s)", glGetString(GL_RENDERER), glGetString(GL_VENDOR));
  if (glGetString(GL_RENDERER) == NULL || glGetString(GL_VENDOR) == NULL) {
    LogWarning("GL strings NULL!");
    throw Exception("GL strings are NULL!");
  }

/* Windows: check whether we are using the standard GDI OpenGL software
   driver... If so make sure the user is warned */
#if defined(WIN32)
  if (!strcmp(reinterpret_cast<const char *>(glGetString(GL_RENDERER)),
              "GDI Generic") &&
      !strcmp(reinterpret_cast<const char *>(glGetString(GL_VENDOR)),
              "Microsoft Corporation")) {
    LogWarning("No GL hardware acceleration!");
    // m_UserNotify = "It seems that no OpenGL hardware acceleration is
    // available!\n"
    //               "Please make sure OpenGL is configured properly.";
  }
#endif

  /* Init OpenGL extensions */
  if (m_bDontUseGLExtensions == true) {
    m_bVBOSupported = false;
    m_bFBOSupported = false;
    m_bShadersSupported = false;
  } else {
    if (m_bDontUseGLVOBS) {
      m_bVBOSupported = false;
    } else {
      m_bVBOSupported = isExtensionSupported("GL_ARB_vertex_buffer_object");
    }

    m_bFBOSupported = isExtensionSupported("GL_EXT_framebuffer_object");

    m_bShadersSupported = isExtensionSupported("GL_ARB_fragment_shader") &&
                          isExtensionSupported("GL_ARB_vertex_shader") &&
                          isExtensionSupported("GL_ARB_shader_objects");
  }

  if (m_bVBOSupported == true) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    LogInfo("GL: using ARB_vertex_buffer_object");
  } else
    LogInfo("GL: not using ARB_vertex_buffer_object");

  if (m_bFBOSupported == true) {
    LogInfo("GL: using EXT_framebuffer_object");
  } else {
    LogInfo("GL: not using EXT_framebuffer_object");
  }

  if (m_bShadersSupported == true) {
    LogInfo(
      "GL: using ARB_fragment_shader/ARB_vertex_shader/ARB_shader_objects");
  } else {
    LogInfo(
      "GL: not using ARB_fragment_shader/ARB_vertex_shader/ARB_shader_objects");
  }

  /* Set background color to black */
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(m_window);
}

void DrawLibOpenGL::unInit() {
  if (m_glContext) {
    SDL_GL_DeleteContext(m_glContext);
    m_glContext = NULL;
  }
  if (m_window) {
    SDL_DestroyWindow(m_window);
    m_window = NULL;
  }
}

/*===========================================================================
  Check for OpenGL extension
  ===========================================================================*/
bool DrawLibOpenGL::isExtensionSupported(std::string Ext) {
  const unsigned char *pcExtensions = NULL;
  const unsigned char *pcStart;
  unsigned char *pcWhere, *pcTerminator;

  pcExtensions = glGetString(GL_EXTENSIONS);
  if (pcExtensions == NULL) {
    LogError(
      "Failed to determine OpenGL extensions. Try stopping all other\n"
      "applications that might use your OpenGL hardware.\n"
      "If it still doesn't work, please create a detailed bug report.\n");
    throw Exception("glGetString() : NULL");
  }

  pcStart = pcExtensions;
  while (1) {
    pcWhere = (unsigned char *)strstr((const char *)pcExtensions, Ext.c_str());
    if (pcWhere == NULL)
      break;
    pcTerminator = pcWhere + Ext.length();
    if (pcWhere == pcStart || *(pcWhere - 1) == ' ')
      if (*pcTerminator == ' ' || *pcTerminator == '\0')
        return true;
    pcStart = pcTerminator;
  }
  return false;
}

/*===========================================================================
  Grab screen contents
  ===========================================================================*/
Img *DrawLibOpenGL::grabScreen(int i_reduce) {
  unsigned int v_imgH = m_nDispHeight / i_reduce;
  unsigned int v_imgW = m_nDispWidth / i_reduce;

  Img *pImg = new Img;

  pImg->createEmpty(v_imgW, v_imgH);
  Color *pPixels = pImg->getPixels();
  unsigned char *pcTemp = new unsigned char[m_nDispWidth * 3];

  /* Select frontbuffer */
  glReadBuffer(GL_FRONT);

  for (unsigned int i = 0; i < v_imgH; i++) {
    glReadPixels(
      0, i * i_reduce, m_nDispWidth, 1, GL_RGB, GL_UNSIGNED_BYTE, pcTemp);
    for (unsigned int j = 0; j < v_imgW; j++) {
      pPixels[(v_imgH - i - 1) * v_imgW + j] =
        MAKE_COLOR(pcTemp[j * i_reduce * 3],
                   pcTemp[j * i_reduce * 3 + 1],
                   pcTemp[j * i_reduce * 3 + 2],
                   255);
    }
  }

  delete[] pcTemp;
  return pImg;
}

void DrawLibOpenGL::startDraw(DrawMode mode) {
  switch (mode) {
    case DRAW_MODE_POLYGON:
      glBegin(GL_POLYGON);
      break;
    case DRAW_MODE_LINE_LOOP:
      glBegin(GL_LINE_LOOP);
      break;
    case DRAW_MODE_LINE_STRIP:
      glBegin(GL_LINE_STRIP);
      break;
    default:
      break;
  };
}

void DrawLibOpenGL::endDraw() {
  glEnd();
  if (m_blendMode != BLEND_MODE_NONE) {
    glDisable(GL_BLEND);
  }
}

void DrawLibOpenGL::endDrawKeepProperties() {
  glEnd();
}

void DrawLibOpenGL::removePropertiesAfterEnd() {
  if (m_texture != NULL) {
    glDisable(GL_TEXTURE_2D);
    m_texture = NULL;
  }
  if (m_blendMode != BLEND_MODE_NONE) {
    glDisable(GL_BLEND);
  }
}

void DrawLibOpenGL::setColor(Color color) {
  glColor4ub(
    GET_RED(color), GET_GREEN(color), GET_BLUE(color), GET_ALPHA(color));
}

void DrawLibOpenGL::setTexture(Texture *texture, BlendMode blendMode) {
  setBlendMode(blendMode);
  if (texture != NULL) {
    /* bind texture only if different than the current one */
    if (m_texture == NULL || texture->Name != m_texture->Name) {
      glBindTexture(GL_TEXTURE_2D, texture->nID);
    }
    glEnable(GL_TEXTURE_2D);
  } else {
    // so the texture is set to null
    // if the texture was not null we need
    // to disable the current texture
    if (m_texture != NULL) {
      glDisable(GL_TEXTURE_2D);
    }
  }
  m_texture = texture;
}

void DrawLibOpenGL::setBlendMode(BlendMode blendMode) {
  if (blendMode != BLEND_MODE_NONE) {
    glEnable(GL_BLEND);
    if (blendMode == BLEND_MODE_A) {
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
      glBlendFunc(GL_ONE, GL_ONE);
    }
  } else {
    glDisable(GL_BLEND);
  }
  m_blendMode = blendMode;
}

void DrawLibOpenGL::setCameraDimensionality(CameraDimension dimension) {
  glViewport(m_renderSurf->downleft().x,
             m_renderSurf->downleft().y,
             m_renderSurf->size().x,
             m_renderSurf->size().y);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (dimension == CAMERA_2D) {
    glOrtho(0, m_renderSurf->size().x, 0, m_renderSurf->size().y, -1, 1);
  }
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void DrawLibOpenGL::clearGraphics() {
  /* Clear screen */
  glClear(GL_COLOR_BUFFER_BIT);
}

/**
 * Flush the graphics. In memory graphics will now be displayed
 **/
void DrawLibOpenGL::flushGraphics() {
  /* Swap buffers */
  SDL_GL_SwapWindow(m_window);
}

// little helper to avoid code duplication
SDL_Surface *createSDLSurface(unsigned int width, unsigned int height) {
  SDL_Surface *surf = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                           width,
                                           height,
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
  if (surf == NULL) {
    SDL_FreeSurface(surf);
    throw Exception("SDL_CreateRGBSurface failed");
  }

  return surf;
}

ScrapTextures::ScrapTextures() {
  m_dirty = false;
  memset(m_scrapsAllocated, 0, sizeof(unsigned int) * BLOCK_WIDTH * MAX_SCRAPS);
  for (unsigned int i = 0; i < MAX_SCRAPS; i++) {
    m_scrapsTexels[i] = createSDLSurface(BLOCK_WIDTH, BLOCK_HEIGHT);
    m_scrapsUsed[i] = false;
  }
  glGenTextures(MAX_SCRAPS, (GLuint *)&m_scrapsTextures);
}

ScrapTextures::~ScrapTextures() {
  for (unsigned int i = 0; i < MAX_SCRAPS; i++) {
    SDL_FreeSurface(m_scrapsTexels[i]);
  }
  glDeleteTextures(MAX_SCRAPS, (GLuint *)&m_scrapsTextures);
}

int ScrapTextures::allocateAndLoadTexture(unsigned int width,
                                          unsigned int height,
                                          float *ux,
                                          float *uy,
                                          float *vx,
                                          float *vy,
                                          SDL_Surface *data) {
  unsigned int x = 0, y = 0, firstAvailable = 0, scrap = 0;
  bool useScrap = false;

  while (scrap < MAX_SCRAPS && useScrap == false) {
    for (unsigned int i = 0; i < BLOCK_WIDTH - width && useScrap == false;
         i++) {
      firstAvailable = 0;

      unsigned int j;
      for (j = 0; j < width; j++) {
        if (m_scrapsAllocated[scrap][i + j] + height >= BLOCK_HEIGHT) {
          break;
        }
        if (m_scrapsAllocated[scrap][i + j] > firstAvailable)
          firstAvailable = m_scrapsAllocated[scrap][i + j];
      }

      if (j == width) {
        // enough free space
        x = i;
        y = firstAvailable;
        useScrap = true;
      }
    }

    if (useScrap == false)
      scrap++;
  }

  if (useScrap == true) {
    m_scrapsUsed[scrap] = true;

    for (unsigned int i = 0; i < width; i++)
      m_scrapsAllocated[scrap][x + i] = firstAvailable + height;

    *ux = (x + 0.01) / (float)BLOCK_WIDTH;
    *vx = (x + width - 0.01) / (float)BLOCK_HEIGHT;
    *uy = (y + 0.01) / (float)BLOCK_WIDTH;
    *vy = (y + height - 0.01) / (float)BLOCK_HEIGHT;

    m_dirty = true;
    SDL_Rect v_area_dest;

    v_area_dest.x = x;
    v_area_dest.y = y;

    SDL_BlitSurface(data, NULL, m_scrapsTexels[scrap], &v_area_dest);

    return m_scrapsTextures[scrap];
  } else {
    // there's four scraps, approximatly less than 100 characters, this
    // should not happen
    LogError("Scrap is full.");
    throw Exception("Scrap is full !");
  }
}

bool ScrapTextures::isDirty() {
  return m_dirty;
}

void ScrapTextures::update() {
  if (isDirty() == true) {
    for (unsigned int i = 0; i < MAX_SCRAPS; i++) {
      if (m_scrapsUsed[i] == false)
        continue;

      glBindTexture(GL_TEXTURE_2D, m_scrapsTextures[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   GL_RGBA,
                   BLOCK_WIDTH,
                   BLOCK_HEIGHT,
                   0,
                   GL_RGBA,
                   GL_UNSIGNED_BYTE,
                   m_scrapsTexels[i]->pixels);
    }
    m_dirty = false;
  }
}

void ScrapTextures::display(DrawLib *pDrawLib) {
  for (unsigned int i = 0; i < MAX_SCRAPS; i++) {
    if (m_scrapsUsed[i] == false)
      continue;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, m_scrapsTextures[i]);
    pDrawLib->drawImageTextureSet(
      Vector2f(0.0 + i * BLOCK_WIDTH, 200.0),
      Vector2f(BLOCK_WIDTH + i * BLOCK_WIDTH, 200.0),
      Vector2f(BLOCK_WIDTH + i * BLOCK_WIDTH, 200 + BLOCK_HEIGHT),
      Vector2f(0.0 + i * BLOCK_WIDTH, 200 + BLOCK_HEIGHT),
      MAKE_COLOR(255, 255, 255, 255),
      true);
  }
}

FontManager *DrawLibOpenGL::getFontManager(const std::string &i_fontFile,
                                           unsigned int i_fontSize,
                                           unsigned int i_fixedFontSize) {
  return new GLFontManager(this, i_fontFile, i_fontSize, i_fixedFontSize);
}

GLFontGlyphLetter::GLFontGlyphLetter(const std::string &i_value,
                                     TTF_Font *i_ttf,
                                     unsigned int i_fixedFontSize)
  : GLFontGlyph(i_value) {
  SDL_Surface *v_surf;
  SDL_Surface *v_image;
  SDL_Color v_color = { 0xFF, 0xFF, 0xFF, 0x00 };

  v_surf = TTF_RenderUTF8_Blended(i_ttf, i_value.c_str(), v_color);
  if (v_surf == NULL) {
    // throw Exception("GLFontGlyphLetter: " + std::string(TTF_GetError()));
    // sometimes, it happends with some special char, give a 2nd chance

    v_surf = TTF_RenderUTF8_Blended(i_ttf, " ", v_color);
    if (v_surf == NULL) {
      throw Exception("GLFontGlyphLetter: " + std::string(TTF_GetError()));
    }
  }
  SDL_SetSurfaceAlphaMod(v_surf, 255);

  // This gets rid of aliasing, don't know why
  SDL_SetSurfaceBlendMode(v_surf, SDL_BLENDMODE_NONE);

  int v_realWidth, v_realHeight;
  TTF_SizeUTF8(i_ttf, i_value.c_str(), &v_realWidth, &v_realHeight);
  m_realWidth = (unsigned int)v_realWidth;
  m_realHeight = (unsigned int)v_realHeight;

  m_drawWidth = m_realWidth;
  m_drawHeight = m_realHeight;
  m_firstLineDrawHeight = m_drawHeight;

  // hack because ttf even width fixed fonts return not perfectly fixed fonts
  if (i_fixedFontSize != 0) {
    if (TTF_FontFaceIsFixedWidth(i_ttf)) {
      m_drawWidth = m_realWidth = i_fixedFontSize;
    }
  }

  // maximum width/heigth allowed, 80, when bigger, create a texture
  if (m_drawWidth < 80 && m_drawHeight < 80) {
    try {
      m_GLID = ScrapTextures::instance()->allocateAndLoadTexture(
        m_drawWidth, m_drawHeight, &m_u.x, &m_u.y, &m_v.x, &m_v.y, v_surf);
      SDL_FreeSurface(v_surf);

      m_useScrap = true;
    } catch (Exception) {
      goto dont_use_crap;
    }
  } else {
  dont_use_crap:
    m_useScrap = false;

    m_drawWidth = powerOf2(m_realWidth);
    m_drawHeight = powerOf2(m_realHeight);
    v_image = createSDLSurface(m_drawWidth, m_drawHeight);

    /* Copy the surface into the GL texture image */
    SDL_BlitSurface(v_surf, NULL, v_image, NULL);
    SDL_FreeSurface(v_surf);

    /* Create the OpenGL texture */
    glGenTextures(1, &m_GLID);
    glBindTexture(GL_TEXTURE_2D, m_GLID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 m_drawWidth,
                 m_drawHeight,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 v_image->pixels);
    SDL_FreeSurface(v_image);

    m_u.x = 0.0;
    m_u.y = 0.0;
    m_v.x = 1.0;
    m_v.y = 1.0;
  }
}

GLFontGlyphLetter::~GLFontGlyphLetter() {
  glDeleteTextures(1, &m_GLID);
}

GLuint GLFontGlyphLetter::GLID() const {
  return m_GLID;
}

GLFontGlyph::GLFontGlyph(const std::string &i_value) {
  m_value = i_value;
  m_drawWidth = m_drawHeight = 0;
  m_realWidth = m_realHeight = 0;
  m_firstLineDrawHeight = 0;
}

GLFontGlyph::GLFontGlyph(
  const std::string &i_value,
  HashNamespace::unordered_map<std::string, GLFontGlyphLetter *>
    &i_glyphsLetters) {
  GLFontGlyph *v_glyph;
  std::string v_char;

  m_value = i_value;
  m_realWidth = m_realHeight = 0;

  if (i_value == "")
    return; /* do nothing for empty strings */

  unsigned int n = 0;
  unsigned int v_maxHeight = 0;
  unsigned int v_curWidth = 0;
  bool v_firstLineInitialized = false;
  while (n < i_value.size()) {
    v_char = utf8::getNextChar(i_value, n);
    if (v_char == "\n") {
      if (v_firstLineInitialized == false) {
        m_firstLineDrawHeight = v_maxHeight;
        v_firstLineInitialized = true;
      }
      m_realHeight += UTF8_INTERLINE_SPACE + v_maxHeight;
      v_maxHeight = 0;
      v_curWidth = 0;
    } else {
      v_glyph = i_glyphsLetters[v_char];
      if (v_glyph != NULL) {
        if (v_glyph->realHeight() > v_maxHeight)
          v_maxHeight = v_glyph->realHeight();
        if (v_curWidth != 0)
          v_curWidth += UTF8_INTERCHAR_SPACE;
        v_curWidth += v_glyph->realWidth();
        if (v_curWidth > m_realWidth)
          m_realWidth = v_curWidth;
      }
    }
  }
  /* last line */
  if (v_char != "\n")
    m_realHeight += v_maxHeight;
  if (v_firstLineInitialized == false) {
    m_firstLineDrawHeight = v_maxHeight;
  }

  m_drawWidth = m_realWidth;
  m_drawHeight = m_realHeight;
  m_firstLineDrawHeight = m_firstLineDrawHeight;
}

GLFontGlyph::~GLFontGlyph() {}

std::string GLFontGlyph::Value() const {
  return m_value;
}

unsigned int GLFontGlyph::drawWidth() const {
  return m_drawWidth;
}

unsigned int GLFontGlyph::drawHeight() const {
  return m_drawHeight;
}

unsigned int GLFontGlyph::realWidth() const {
  return m_realWidth;
}

unsigned int GLFontGlyph::realHeight() const {
  return m_realHeight;
}

unsigned int GLFontGlyph::firstLineDrawHeight() const {
  return m_firstLineDrawHeight;
}

GLFontManager::GLFontManager(DrawLib *i_drawLib,
                             const std::string &i_fontFile,
                             unsigned int i_fontSize,
                             unsigned int i_fixedFontSize)
  : FontManager(i_drawLib, i_fontFile, i_fontSize, i_fixedFontSize) {}

unsigned int GLFontManager::nbGlyphsInMemory() {
  return m_glyphsLettersValues.size();
}

void GLFontManager::displayScrap(DrawLib *pDrawLib) {
  ScrapTextures::instance()->display(pDrawLib);
}

GLFontManager::~GLFontManager() {
  for (unsigned int i = 0; i < m_glyphsValues.size(); i++) {
    delete m_glyphsValues[i];
  }

  for (unsigned int i = 0; i < m_glyphsLettersValues.size(); i++) {
    delete m_glyphsLettersValues[i];
  }

  /* i added the m_glyphsList because the iterator on the hashmap
     often produces segfault on delete it->second */
  // printf("BEGIN ~GLFontManager()\n");
  // HashNamespace::unordered_map<std::string, GLFontGlyph*>::iterator it;
  // for (it = m_glyphs.begin(); it != m_glyphs.end(); it++) {
  //  delete it->second;
  //}
  // printf("END ~GLFontManager()\n");
}

int GLFontGlyph::powerOf2(int i_value) {
  int v_value = 1;

  while (v_value < i_value) {
    v_value <<= 1;
  }
  return v_value;
}

FontGlyph *GLFontManager::getGlyphTabExtended(const std::string &i_string) {
  std::string v_extstr;
  unsigned int v_tab_size = 5;

  unsigned int n = 0;
  std::string v_char;
  while (n < i_string.size()) {
    v_char = utf8::getNextChar(i_string, n);
    if (v_char == "\t") {
      for (unsigned int i = n % v_tab_size; i < v_tab_size; i++) {
        v_extstr += " ";
      }
    } else {
      v_extstr += v_char;
    }
  }

  return getGlyph(v_extstr);
}

FontGlyph *GLFontManager::getGlyph(const std::string &i_string) {
  GLFontGlyph *v_glyph;
  GLFontGlyphLetter *v_glyphLetter;

  v_glyph = m_glyphs[i_string];
  if (v_glyph != NULL)
    return v_glyph;

  /* make sure that chars exists into the hashmap before continuing */
  unsigned int n = 0;
  std::string v_char;
  while (n < i_string.size()) {
    v_char = utf8::getNextChar(i_string, n);
    if (v_char != "\n") {
      if (m_glyphsLetters[v_char] == NULL) {
        v_glyphLetter = new GLFontGlyphLetter(v_char, m_ttf, m_fixedFontSize);
        m_glyphsLettersKeys.push_back(v_char);
        m_glyphsLettersValues.push_back(v_glyphLetter);
        m_glyphsLetters[m_glyphsLettersKeys[m_glyphsLettersKeys.size() - 1]] =
          v_glyphLetter;
      }
    }
  }

  v_glyph = new GLFontGlyph(i_string, m_glyphsLetters);
  m_glyphsKeys.push_back(i_string);
  m_glyphsValues.push_back(v_glyph);
  m_glyphs[m_glyphsKeys[m_glyphsKeys.size() - 1]] = v_glyph;

  return v_glyph;
}

void GLFontManager::printStringGrad(DrawLib *pDrawLib,
                                    FontGlyph *i_glyph,
                                    int i_x,
                                    int i_y,
                                    Color c1,
                                    Color c2,
                                    Color c3,
                                    Color c4,
                                    float i_perCentered,
                                    bool i_shadowEffect) {
  if (i_shadowEffect) {
    printStringGradOne(pDrawLib,
                       i_glyph,
                       i_x,
                       i_y,
                       INVERT_COLOR(c1),
                       INVERT_COLOR(c2),
                       INVERT_COLOR(c3),
                       INVERT_COLOR(c4),
                       i_perCentered);
    printStringGradOne(
      pDrawLib, i_glyph, i_x + 1, i_y + 1, c1, c2, c3, c4, i_perCentered);
  } else {
    printStringGradOne(
      pDrawLib, i_glyph, i_x, i_y, c1, c2, c3, c4, i_perCentered);
  }
}

void GLFontManager::printStringGradOne(DrawLib *pDrawLib,
                                       FontGlyph *i_glyph,
                                       int i_x,
                                       int i_y,
                                       Color c1,
                                       Color c2,
                                       Color c3,
                                       Color c4,
                                       float i_perCentered) {
  GLFontGlyph *v_glyph = (GLFontGlyph *)i_glyph;
  GLFontGlyphLetter *v_glyphLetter;
  int v_x, v_y;
  unsigned int n;
  std::string v_value, v_char;
  unsigned int v_lineHeight;
  unsigned int v_longuest_linesize;
  unsigned int v_current_linesize;
  unsigned int v_size;

  bool prevNewline = false; // indicates whether the previous char was a newline

  int oldTextureId = -1;
  int newTextureId;

  char r1, r2, r3, r4;
  char g1, g2, g3, g4;
  char b1, b2, b3, b4;
  char a1, a2, a3, a4;

  r1 = GET_RED(c1);
  r2 = GET_RED(c2);
  r3 = GET_RED(c3);
  r4 = GET_RED(c4);

  g1 = GET_GREEN(c1);
  g2 = GET_GREEN(c2);
  g3 = GET_GREEN(c3);
  g4 = GET_GREEN(c4);

  b1 = GET_BLUE(c1);
  b2 = GET_BLUE(c2);
  b3 = GET_BLUE(c3);
  b4 = GET_BLUE(c4);

  a1 = GET_ALPHA(c1);
  a2 = GET_ALPHA(c2);
  a3 = GET_ALPHA(c3);
  a4 = GET_ALPHA(c4);

  ScrapTextures::instance()->update();

  v_value = v_glyph->Value();
  if (v_value == "")
    return;

  try {
    /* draw the glyph */
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // the first glBegin/glEnd will draw nothing
    glBegin(GL_QUADS);

    v_longuest_linesize = getLonguestLineSize(v_value);
    v_size = v_value.size();
    v_current_linesize = getLonguestLineSize(v_value, 0, 1);
    v_x = i_x + (v_longuest_linesize - v_current_linesize) / 2 *
                  (i_perCentered + 1.0);

    /* vertical position of the first line */
    v_y = -i_y + pDrawLib->getRenderSurface()->size().y -
          v_glyph->firstLineDrawHeight();
    v_lineHeight = 0;

    n = 0;

    while (n < v_size) {
      utf8::getNextChar(v_value, n, v_char);
      if (v_char == "\n") {
        v_current_linesize = getLonguestLineSize(v_value, n, 1);
        v_x = i_x + (v_longuest_linesize - v_current_linesize) / 2 *
                      (i_perCentered + 1.0);
        v_y -= v_lineHeight + UTF8_INTERLINE_SPACE;
        /* check if the previous character was a newline too */
        if (prevNewline) {
          v_glyphLetter = m_glyphsLetters[" "];
          v_lineHeight = v_glyphLetter->realHeight();
          v_y -= v_lineHeight;
        }
        prevNewline = true;
        v_lineHeight = 0;
      } else {
        prevNewline = false;
        v_glyphLetter = m_glyphsLetters[v_char];
        if (v_glyphLetter != NULL) {
          if (v_glyphLetter->realHeight() > v_lineHeight)
            v_lineHeight = v_glyphLetter->realHeight();

          newTextureId = v_glyphLetter->GLID();
          if (newTextureId != oldTextureId) {
            glEnd();
            glBindTexture(GL_TEXTURE_2D, newTextureId);
            glBegin(GL_QUADS);
            oldTextureId = newTextureId;
          }

          glColor4ub(r1, g1, b1, a1);
          glTexCoord2f(v_glyphLetter->m_u.x, v_glyphLetter->m_v.y);
          glVertex2i(v_x, v_y);

          glColor4ub(r2, g2, b2, a2);
          glTexCoord2f(v_glyphLetter->m_v.x, v_glyphLetter->m_v.y);
          glVertex2i(v_x + v_glyphLetter->drawWidth(), v_y);

          glColor4ub(r4, g4, b4, a4);
          glTexCoord2f(v_glyphLetter->m_v.x, v_glyphLetter->m_u.y);
          glVertex2i(v_x + v_glyphLetter->drawWidth(),
                     v_y + v_glyphLetter->drawHeight());

          glColor4ub(r3, g3, b3, a3);
          glTexCoord2f(v_glyphLetter->m_u.x, v_glyphLetter->m_u.y);
          glVertex2i(v_x, v_y + v_glyphLetter->drawHeight());

          v_x += v_glyphLetter->realWidth() + UTF8_INTERCHAR_SPACE;
        }
      }
    }

    glEnd();
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

  } catch (Exception &e) {
    /* ok, forget this one */
  }
}

unsigned int GLFontManager::getLonguestLineSize(const std::string &i_value,
                                                unsigned int i_start,
                                                unsigned int i_nbLinesToRead) {
  unsigned int v_longuest_linesize = 0;
  unsigned int v_current_linesize;
  unsigned int n = i_start;
  unsigned int v_size = i_value.size();
  std::string v_char;
  unsigned int v_line_n = 0;
  GLFontGlyphLetter *v_glyphLetter;

  v_current_linesize = -UTF8_INTERCHAR_SPACE;
  while (n < v_size) {
    utf8::getNextChar(i_value, n, v_char);
    if (v_char == "\n") {
      if (v_current_linesize > v_longuest_linesize) {
        v_longuest_linesize = v_current_linesize;
      }
      v_current_linesize = 0;
      v_line_n++;

      // enough lines read
      if (i_nbLinesToRead > 0 && v_line_n == i_nbLinesToRead) {
        return v_longuest_linesize;
      }
    } else {
      v_glyphLetter = m_glyphsLetters[v_char];
      if (v_glyphLetter != NULL) {
        v_current_linesize += v_glyphLetter->realWidth() + UTF8_INTERCHAR_SPACE;
      }
    }
  }

  if (v_current_linesize > v_longuest_linesize) {
    v_longuest_linesize = v_current_linesize;
  }

  return v_longuest_linesize;
}

void GLFontManager::printString(DrawLib *pDrawLib,
                                FontGlyph *i_glyph,
                                int i_x,
                                int i_y,
                                Color i_color,
                                float i_perCentered,
                                bool i_shadowEffect) {
  printStringGrad(pDrawLib,
                  i_glyph,
                  i_x,
                  i_y,
                  i_color,
                  i_color,
                  i_color,
                  i_color,
                  i_perCentered,
                  i_shadowEffect);
}

#endif
