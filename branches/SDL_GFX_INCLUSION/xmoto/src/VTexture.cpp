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
 *  Handling of textures.
 */
#include "VApp.h"
#include "VTexture.h"
#include "Image.h"
#include "VFileIO.h"

namespace vapp {

  /*===========================================================================
  Create texture from memory
  ===========================================================================*/
  Texture *TextureManager::createTexture(std::string Name,unsigned char *pcData,int nWidth,int nHeight,bool bAlpha,bool bClamp, FilterMode eFilterMode) {
    /* Name free? */
    if(getTexture(Name) != NULL) {
      Log("** Warning ** : TextureManager::createTexture() : Name '%s' already in use",Name.c_str());
      throw TextureError("texture naming conflict");
    }

    /* Allocate */
    Texture *pTexture = new Texture;
    pTexture->Name = Name;
    pTexture->nWidth = nWidth;
    pTexture->nHeight = nHeight;
    pTexture->Tag = "";
#ifdef ENABLE_OPENGL
    pTexture->nID = 0;
#endif
    
#ifdef ENABLE_OPENGL
    /* OpenGL magic */
    GLuint N;
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1,&N);    
    glBindTexture(GL_TEXTURE_2D,N);
    
    if(bAlpha) {
      /* Got alpha channel */
      glTexImage2D(GL_TEXTURE_2D,0,4,nWidth,nHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,pcData);
      pTexture->nSize = nWidth * nHeight * 4;
    }
    else {
      /* Plain RGB */
      glTexImage2D(GL_TEXTURE_2D,0,3,nWidth,nHeight,0,GL_RGB,GL_UNSIGNED_BYTE,pcData);
      pTexture->nSize = nWidth * nHeight * 3;
    }
    
    switch(eFilterMode) {
      /* require openGL 1.4 */
      case FM_MIPMAP:
#if defined(GL_GENERATE_MIPMAP)      
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_NEAREST);
      glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP, GL_TRUE);
#else
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);    
#endif      
      break;
      
      case FM_LINEAR:
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      break;

      case FM_NEAREST:
      default:
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
      break;
    }
        
    if(bClamp) {
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
    }
    else {
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    }
    glDisable(GL_TEXTURE_2D);
    
    pTexture->nID = N;
#endif
    
    m_nTexSpaceUsage += pTexture->nSize;
  
    /* Do it captain */
    m_Textures.push_back( pTexture );
    
    return pTexture;
  }
  
  /*===========================================================================
  Destroy texture - i.e. free it from video memory and our list
  ===========================================================================*/  
  void TextureManager::destroyTexture(Texture *pTexture) {
    if(pTexture != NULL) {
      for(unsigned int i=0;i<m_Textures.size();i++) {
        if(m_Textures[i] == pTexture) {
#ifdef ENABLE_OPENGL
          glDeleteTextures(1,(GLuint *)&pTexture->nID);
#endif
          m_nTexSpaceUsage -= pTexture->nSize;
          delete pTexture;
          m_Textures.erase(m_Textures.begin() + i);
          return;
        }
      }      
    }
    
    throw TextureError("can't destroy unmanaged texture object");
  }
  
  /*===========================================================================
  Shortcut to loading textures from image files
  ===========================================================================*/  
  Texture *TextureManager::loadTexture(std::string Path,bool bSmall,bool bClamp, FilterMode eFilterMode) {
    /* Check file validity */
    image_info_t ii;
    Img TextureImage;
    Texture *pTexture = NULL;
    
    /* Name it */
    std::string TexName = FS::getFileBaseName(Path);

    /* if the texture is already loaded, return it */
    pTexture = getTexture(TexName);
    if(pTexture != NULL) {
      return pTexture;
    }

    if(TextureImage.checkFile( Path,&ii )) {
      /* Valid texture size? */
      if(ii.nWidth != ii.nHeight) {
        Log("** Warning ** : TextureManager::loadTexture() : texture '%s' is not square",Path.c_str());
        throw TextureError("texture not square");
      }
      if(!(ii.nWidth == 1 ||
         ii.nWidth == 2 ||
         ii.nWidth == 4 ||
         ii.nWidth == 8 ||
         ii.nWidth == 16 ||
         ii.nWidth == 32 ||
         ii.nWidth == 64 ||
         ii.nWidth == 128 ||
         ii.nWidth == 256 ||
         ii.nWidth == 512 ||
         ii.nWidth == 1024)) {
        Log("** Warning ** : TextureManager::loadTexture() : texture '%s' size is not power of two",Path.c_str());
        throw TextureError("texture size not power of two");
      }
         
      /* Load it into system memory */
      TextureImage.loadFile(Path,bSmall);
      
      /* Copy it into video memory */
      unsigned char *pc;
      bool bAlpha = TextureImage.isAlpha();
      if(bAlpha)
        pc = TextureImage.convertToRGBA32();
      else
        pc = TextureImage.convertToRGB24();
      
      pTexture = createTexture(TexName,pc,TextureImage.getWidth(),TextureImage.getHeight(),bAlpha,bClamp, eFilterMode);
      
      delete [] pc;
    }
    else {
      Log("** Warning ** : TextureManager::loadTexture() : texture '%s' not found or invalid",Path.c_str());
      throw TextureError(std::string("invalid or missing texture file (" + Path + ")").c_str());
    }    
    
    return pTexture;
  }
  
  /*===========================================================================
  Get loaded texture by name
  ===========================================================================*/  
  Texture *TextureManager::getTexture(std::string Name) {
    for(unsigned int i=0;i<m_Textures.size();i++)
      if(m_Textures[i]->Name == Name) return m_Textures[i];
    return NULL;
  }

  /*===========================================================================
  Fetch all textures with the given tag 
  ===========================================================================*/
  std::vector<Texture *> TextureManager::fetchTaggedTextures(std::string Tag) {
    std::vector<Texture *> Ret;
    
    for(unsigned int i=0;i<m_Textures.size();i++) {
      if(m_Textures[i]->Tag == Tag) Ret.push_back(m_Textures[i]);
    }
    return Ret;
  }
  
  /*===========================================================================
  Unload everything in a very hateful manner
  ===========================================================================*/
  void TextureManager::unloadTextures(void) {
    while(!m_Textures.empty())
      destroyTexture(m_Textures[0]);
  }
  
}

//
//  /*===========================================================================
//  Default texture loader
//  ===========================================================================*/
//  void Texture::load(std::string FilePath,bool bSmall) {
//    /* Load file as image */
//    Img TextureImage;
//    image_info_t ii;
//    bool bLoadDummyTexture = false;
//    //printf("[%s]\n",FilePath.c_str());
//    if(TextureImage.checkFile( FilePath,&ii )) {
//    //  printf("   [%d %d]\n",ii.nWidth,ii.nHeight);
//      /* Seems ok, let's load! */
//      TextureImage.loadFile( FilePath,bSmall );
//      
////      printf("OK %d %d %d\n",TextureImage.getWidth(),TextureImage.getHeight(),TextureImage.isAlpha());
//
//      /* Obtain RGBA format */
//      unsigned char *pcData = TextureImage.convertToRGBA32();
//
//  //    printf("...\n");
//
//      /* Pass it to GL */
//      GLuint N;
//      glEnable(GL_TEXTURE_2D);
//      glGenTextures(1,&N);    
//      glBindTexture(GL_TEXTURE_2D,N);
//      glTexImage2D(GL_TEXTURE_2D,0,4,TextureImage.getWidth(),TextureImage.getHeight(),
//                   0,GL_RGBA,GL_UNSIGNED_BYTE,pcData);
//      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
//      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
//      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
//      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
//      glDisable(GL_TEXTURE_2D);
//            
//      m_TI.nID = N;            
//      delete [] pcData;
//    }
//    else {
//      Log("** Warning ** : Texture::load() - invalid file: %s",FilePath.c_str());
//      bLoadDummyTexture = true;
//    }
//    
//    /* Should we load a dummy checkerboard pattern? */
//    if(bLoadDummyTexture) {
//      /* TODO: create checkerboard pattern */
//    }
//  }
//
//  /*===========================================================================
//  Default texture unloader
//  ===========================================================================*/
//  void Texture::unload(void) {
//    /* Delete GL name */
//    glDeleteTextures(1,(GLuint *)&m_TI.nID);
//  }
//
//  /*===========================================================================
//  Texture loader 
//  ===========================================================================*/
//  Texture *TextureManager::loadTexture(Texture *pTexture,std::string FilePath,bool bSmall) {   
//    /* Determine the "name" */
//    std::string TexName = FS::getFileBaseName(FilePath);
//    pTexture->m_TI.Name = TexName;
//
//    /* Load and manage texture */
//    pTexture->load(FilePath,bSmall);  /* TODO:  FIX DET HER!! MÅSKE SKRIV EN NY TEXTUREMANAGER :( */
//    //printf("%x  %x  L!! %s\n",this,pTexture,FilePath.c_str());
//    m_Textures.push_back( pTexture );
//    return pTexture;
//  }
//
//  /*===========================================================================
//  Unload texture
//  ===========================================================================*/
//  void TextureManager::_Unmanage(Texture *pTexture) {
//    /* Find texture in list */
//    for(int i=0;i<m_Textures.size();i++) {
//      if(m_Textures[i] == pTexture) {
//        delete m_Textures[i];
//        m_Textures.erase(m_Textures.begin() + i);
//        return;
//      }
//    }    
//    
//    /* Not found */
//    throw TextureError("texture not found");
//  }
//  
//  /*===========================================================================
//  Unload all textures and stuff
//  ===========================================================================*/
//  Texture *TextureManager::getTexture(std::string Name) {
//    /* Look up this texture */
//    if(Name == "default") return getTexture( m_DefaultTextureName );
//    
//    for(int i=0;i<m_Textures.size();i++) {
//      if(m_Textures[i]->getName() == Name) return m_Textures[i];
//    }
//    return NULL;
//  }
//
//  /*===========================================================================
//  Unload all textures and stuff
//  ===========================================================================*/
//  void TextureManager::_Shutdown(void) {
//    for(int i=0;i<m_Textures.size();i++) {
//      delete m_Textures[i];
//    }
//  }
//  
//  /*===========================================================================
//  Fetch all textures with the given tag 
//  ===========================================================================*/
//  std::vector<Texture *> TextureManager::fetchTaggedTextures(std::string Tag) {
//    std::vector<Texture *> Ret;
//    
//    for(int i=0;i<m_Textures.size();i++) {
//      if(m_Textures[i]->getTag() == Tag) Ret.push_back(m_Textures[i]);
//    }
//    return Ret;
//  }
//  
//};
