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


  enum FilterMode {
    FM_NEAREST,
    FM_LINEAR,
    FM_MIPMAP
  };

  /*===========================================================================
  Our friendly texture exception friend
  ===========================================================================*/    
  class TextureError : public Exception {
    public:
      TextureError(std::string &iMsg)
        : Exception(iMsg) {}
      TextureError(const char *pc)
        : Exception(std::string(pc)) {}
    private:
  };

  /*===========================================================================
  Texture
  ===========================================================================*/    
  //keesj:todo. I experimented with converting this struct to a
  //class and extending it so that the opengl version would only
  //contain nID and the SDL_based one the Surface pointer. I think it is
  //a lot of work and because the game currently depends on SDL
  //and nID is not a openGL specific structure
  //it's 
  struct Texture {
    public:
    Texture() {
      nWidth = nHeight = 0;
      nID = 0;
      surface = NULL;
      nSize = 0;
      isAlpha = false;
    }
 
    std::string Name;       /* Name */
    int nWidth,nHeight;     /* Size */
    unsigned int nID;       /* OpenGL name */
    SDL_Surface * surface;  /* SDL_surface */
    std::string Tag;        /* Optional tag */
    int nSize;              /* Size in bytes */
    bool isAlpha;           /* Whether the texture contains an alpha channel */
    unsigned char *pcData;
  };

  /*===========================================================================
  Texture manager
  ===========================================================================*/    
  class TextureManager {
    public:
      TextureManager() {m_nTexSpaceUsage=0;}
    
      /* Methods */
      Texture *createTexture(std::string Name,unsigned char *pcData,int nWidth,int nHeight,bool bAlpha=false,bool bClamp=false, FilterMode eFilterMode = FM_MIPMAP);
      void destroyTexture(Texture *pTexture);
      Texture *loadTexture(std::string Path,bool bSmall=false,bool bClamp=false, FilterMode eFilterMode = FM_MIPMAP);
      Texture *getTexture(std::string Name);
      std::vector<Texture *> fetchTaggedTextures(std::string Tag);
      void unloadTextures(void);
    
      /* Data interface */
      std::vector<Texture *> &getTextures(void) {return m_Textures;}
      int getTextureUsage(void) {return m_nTexSpaceUsage;}

    private:
      /* Data */
      std::vector<Texture *> m_Textures;      /* Textures */
      
      int m_nTexSpaceUsage;                   /* Bytes of textures resident */
  };

#endif
