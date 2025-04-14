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

#ifndef __VTEXTURE_H__
#define __VTEXTURE_H__

#include "VCommon.h"
#include "helpers/VExcept.h"
#include "include/xm_SDL.h"
#include "include/xm_hashmap.h"

#ifdef ENABLE_OPENGL
#include "include/xm_OpenGL.h"
#endif

#include <vector>

class Sprite;

enum FilterMode { FM_NEAREST, FM_LINEAR, FM_MIPMAP };

enum class WrapMode {
  Clamp = GL_CLAMP,
  ClampToEdge = GL_CLAMP_TO_EDGE,
  Repeat = GL_REPEAT,
};

// Our friendly texture exception friend
class TextureError : public Exception {
public:
  TextureError(std::string &iMsg)
    : Exception(iMsg) {}
  TextureError(const char *pc)
    : Exception(std::string(pc)) {}
};

// keesj:todo. I experimented with converting this struct to a
// class and extending it so that the opengl version would only
// contain nID and the SDL_based one the Surface pointer. I think it is
// a lot of work and because the game currently depends on SDL
// and nID is not a openGL specific structure
// it's

enum RegistrationStageMode { RSM_PERSISTENT, RSM_NORMAL };

#define PERSISTENT 0
class Texture {
public:
  Texture() {
    nWidth = 0;
    nHeight = 0;
    nID = 0;
    surface = NULL;
    nSize = 0;
    isAlpha = false;
    curRegistrationStageMode = RSM_PERSISTENT;
  }

  std::string Name;
  int nWidth;
  int nHeight;
  // OpenGL name
  unsigned int nID;
  SDL_Surface *surface;
  // size in bytes
  int nSize;
  bool isAlpha;
  unsigned char *pcData;

  // zero for persistent textures
  RegistrationStageMode curRegistrationStageMode;
  std::vector<unsigned int> curRegistrationStage;

  // when the texture is removed, keep the sprites informed so that
  // it invalides its pointer on the texture
  std::vector<Sprite *> associatedSprites;

  void addAssociatedSprite(Sprite *sprite);
  void invalidateSpritesTexture();
  void removeAssociatedSprites();
};

class TextureManager {
public:
  TextureManager() { m_nTexSpaceUsage = 0; }

  ~TextureManager();

  Texture *createTexture(const std::string &Name,
                         unsigned char *pcData,
                         int nWidth,
                         int nHeight,
                         bool bAlpha = false,
                         WrapMode wrapMode = WrapMode::Repeat,
                         FilterMode eFilterMode = FM_MIPMAP);
  void destroyTexture(Texture *pTexture);
  Texture *loadTexture(const std::string &Path,
                       bool bSmall = false,
                       WrapMode wrapMode = WrapMode::Repeat,
                       FilterMode eFilterMode = FM_MIPMAP,
                       bool persistent = false,
                       Sprite *associatedSprite = NULL);
  int getTextureSize(const std::string &p_fileName);
  Texture *getTexture(const std::string &Name);
  void removeAssociatedSpritesFromTextures();
  void unloadTextures(void);

  std::vector<Texture *> &getTextures(void) { return m_Textures; }
  int getTextureUsage(void) { return m_nTexSpaceUsage; }

  // texture registration
  static bool registering();
  static unsigned int currentRegistrationStage();
  unsigned int beginTexturesRegistration();
  void endTexturesRegistration();
  void registerTexture(Texture *i_texture);
  bool isRegisteredTexture(Texture *i_texture);
  void unregister(unsigned int i_registerValue);

private:
  std::vector<Texture *> m_Textures;

  int m_nTexSpaceUsage;

  // texture registration
  static unsigned int m_curRegistrationStage;
  static bool m_registering;

  void cleanUnregistredTextures();

  HashNamespace::unordered_map<std::string, int *> m_textureSizeCache;
  std::vector<std::string> m_textureSizeCacheKeys;
  std::vector<int *> m_textureSizeCacheValues;
};

#endif
