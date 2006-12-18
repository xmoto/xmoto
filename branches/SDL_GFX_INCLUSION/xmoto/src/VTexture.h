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

#ifndef __VTEXTURE_H__
#define __VTEXTURE_H__

#include "VCommon.h"
#include "helpers/VExcept.h"

namespace vapp {

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
      TextureError() {}
      TextureError(std::string &iMsg)
        : Exception(iMsg) {}
      TextureError(const char *pc)
        : Exception(std::string(pc)) {}
    private:
  };

  /*===========================================================================
  Texture
  ===========================================================================*/    
  struct Texture {
    Texture() {
      nWidth = nHeight = 0;
#ifdef ENABLE_OPENGL
      nID = 0;
#endif
      nSize = 0;
    }
  
    std::string Name;       /* Name */
    int nWidth,nHeight;     /* Size */
#ifdef ENABLE_OPENGL
    unsigned int nID;       /* OpenGL name */
#endif
    std::string Tag;        /* Optional tag */
    int nSize;              /* Size in bytes */
  };

  /*===========================================================================
  Texture manager
  ===========================================================================*/    
  class TextureManager {
    public:
      TextureManager() {m_nTexSpaceUsage=0;}
    
      /* Methods */
      Texture *createTexture(std::string Name,unsigned char *pcData,int nWidth,int nHeight,bool bAlpha=false,bool bClamp=false, FilterMode eFilterMode = FM_LINEAR);
      void destroyTexture(Texture *pTexture);
      Texture *loadTexture(std::string Path,bool bSmall=false,bool bClamp=false, FilterMode eFilterMode = FM_LINEAR);
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
  
}

//#include "VCommon.h"
//#include "VExcept.h"
//
//namespace vapp {
//
//  class Texture;
//  class TextureManager;
//
//  /*===========================================================================
//  Types
//  ===========================================================================*/    
//  class TextureError : public Exception {
//    public:
//      TextureError() {}
//      TextureError(std::string &iMsg)
//        : Exception(iMsg) {}
//      TextureError(const char *pc)
//        : Exception(std::string(pc)) {}
//    private:
//  };
//
//  /*===========================================================================
//  Types
//  ===========================================================================*/    
//  typedef struct _TexInfo {
//    std::string Name;               /* Name of texture */
//    std::string FilePath;           /* File path to texture */
//    int nWidth,nHeight;             /* Size of texture */
//    unsigned int nID;               /* OpenGL identifier */
//    std::string Tag;                /* Texture tag. Optional */
//  } TexInfo;  
//
//  /*===========================================================================
//  Texture class
//  ===========================================================================*/    
//  class Texture {
//    public:
//      friend class TextureManager; /* allow TextureManager to access us */
//
//      Texture() {m_TI.Name=""; m_TI.nWidth=m_TI.nHeight=0; m_TI.nID=0; m_TI.FilePath="";}
//
//      /* Virtual methods */
//      virtual void load(std::string FilePath,bool bSmall=false);
//      virtual void unload(void);
//                              
//      /* Data interface */
//      unsigned int getID(void) {return m_TI.nID;}
//      int getWidth(void) {return m_TI.nWidth;}
//      int getHeight(void) {return m_TI.nHeight;}
//      std::string &getName(void) {return m_TI.Name;}
//      std::string &getFilePath(void) {return m_TI.FilePath;}
//      std::string &getTag(void) {return m_TI.Tag;}
//      void setTag(std::string Tag) {m_TI.Tag = Tag;}
//      
//    protected:
//      /* Data */
//      TexInfo m_TI;
//  };    
//
//  /*===========================================================================
//  Texture manager class
//  ===========================================================================*/    
//  class TextureManager {
//    public:    
//      TextureManager() {m_DefaultTextureName = "";}
//    
//      friend class Texture; /* allow Texture to access us */
//          
//      /* Methods */
//      Texture *loadTexture(Texture *pTexture,std::string FilePath,bool bSmall=false);      
//      void unloadTextures(void) {_Shutdown();}
//      Texture *getTexture(std::string Name);
//      std::vector<Texture *> fetchTaggedTextures(std::string Tag);
//      
//      /* Data interface */
//      std::vector<Texture *> &getTextures(void) {return m_Textures;}
//      std::string &getDefaultTextureName(void) {return m_DefaultTextureName;}
//      void setDefaultTextureName(std::string Name) {m_DefaultTextureName=Name;}
//      
//    private:
//      /* Private methods */
//      void _Unmanage(Texture *pTexture);
//      void _Shutdown(void);
//    
//      /* Data */
//      std::vector<Texture *> m_Textures;      /* Textures */
//      std::string m_DefaultTextureName;       /* Name of default texture */
//  };
//  
//};

#endif
