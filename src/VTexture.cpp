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
 *  Handling of textures.
 */
#include "VTexture.h"
#include "Image.h"
#include "VFileIO.h"
#include "helpers/Log.h"
#include "Renderer.h"
#include "Theme.h"
#include "XMSession.h"

void Texture::addAssociatedSprite(Sprite* sprite)
{
  bool found = false;
  std::vector<Sprite*>::iterator it = associatedSprites.begin();
  while(it != associatedSprites.end()){
    if((*it) == sprite){
      found = true;
      break;
    }

    ++it;
  }
  if(found == false){
    associatedSprites.push_back(sprite);
  }
}

void Texture::invalidateSpritesTexture()
{
  std::vector<Sprite*>::iterator it = associatedSprites.begin();
  while(it != associatedSprites.end()){
    (*it)->invalidateTextures();

    LogDebug("associated sprite [%x]", (*it));

    ++it;
  }
}

  /*===========================================================================
  Create texture from memory
  ===========================================================================*/
  Texture *TextureManager::createTexture(std::string Name,unsigned char *pcData,int nWidth,int nHeight,bool bAlpha,bool bClamp, FilterMode eFilterMode) {
    /* Name free? */
    if(getTexture(Name) != NULL) {
      LogInfo("** Warning ** : TextureManager::createTexture() : Name '%s' already in use",Name.c_str());
      throw TextureError("texture naming conflict");
    }

    /* Allocate */
    Texture *pTexture = new Texture;
    pTexture->Name = Name;
    pTexture->nWidth = nWidth;
    pTexture->nHeight = nHeight;
    pTexture->isAlpha = bAlpha;

#ifdef ENABLE_OPENGL
    pTexture->nID = 0;

    /* OpenGL magic */
    GLuint N;
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1,&N);    
    glBindTexture(GL_TEXTURE_2D,N);

    switch(eFilterMode) {
      /* require openGL 1.4 */
      case FM_MIPMAP:
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
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

    /* Adapted from Extreme Tuxracer by Antti Harri and Lasse Collin */
    GLint max_texture_size;
    int depth = bAlpha?4:3;
    /* Check if we need to scale image */
    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &max_texture_size );
    if ( nWidth > max_texture_size || nHeight > max_texture_size ) {

      LogInfo("** Warning ** : TextureManager::createTexture() : Texture '%s' too large -- scaling to %i x %i",Name.c_str(), max_texture_size, max_texture_size );

      unsigned char *newdata = new unsigned char[max_texture_size * max_texture_size * depth];

      /*
       * In the case of large- or small-aspect ratio textures, this
       * could end up using *more* space... oh well.
       */
      GLint retval;
      retval = gluScaleImage(bAlpha?GL_RGBA:GL_RGB,nWidth,nHeight,GL_UNSIGNED_BYTE,pcData,max_texture_size,max_texture_size,GL_UNSIGNED_BYTE,newdata );

      if (retval!=0) {
        throw TextureError("Failed to scale image in TextureManagerc::createTexture()");
      }
      delete[] pcData;
      pcData = newdata;

      pTexture->nWidth = nWidth = max_texture_size;
      pTexture->nHeight = nHeight = max_texture_size;
    }

      if(eFilterMode == FM_MIPMAP){
        gluBuild2DMipmaps(GL_TEXTURE_2D,depth,nWidth,nHeight,bAlpha?GL_RGBA:GL_RGB,GL_UNSIGNED_BYTE,pcData);
      }else{
      glTexImage2D(GL_TEXTURE_2D,0,depth,nWidth,nHeight,0,bAlpha?GL_RGBA :GL_RGB,GL_UNSIGNED_BYTE,pcData);
      }

    glDisable(GL_TEXTURE_2D);
    
    pTexture->nID = N;
#endif
    
    m_nTexSpaceUsage += pTexture->nSize;
#ifdef ENABLE_SDLGFX
        #if SDL_BYTEORDER == SDL_BIG_ENDIAN
            Uint32 rmask = 0xff000000;
            Uint32 gmask = 0x00ff0000;
            Uint32 bmask = 0x0000ff00;
            Uint32 amask = 0x000000ff;
        #else
            Uint32 rmask = 0x000000ff;
            Uint32 gmask = 0x0000ff00;
            Uint32 bmask = 0x00ff0000;
            Uint32 amask = 0xff000000;
        #endif

      if(bAlpha){
        pTexture->surface  = SDL_CreateRGBSurfaceFrom(pcData,nWidth,nHeight,32 /*bitsPerPixel */, nWidth * 4 /*pitch*/,rmask,gmask,bmask,amask);
      } else {
        pTexture->surface  = SDL_CreateRGBSurfaceFrom(pcData,nWidth,nHeight,24 /*bitsPerPixel */, nWidth * 3 /*pitch*/,rmask,gmask,bmask,0);

      }

      pTexture->pcData = pcData;
#else
  pTexture->surface = NULL;
  pTexture->pcData = NULL;
  delete[] pcData;
#endif
  
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
#ifdef ENABLE_SDLGFX
	  SDL_FreeSurface(pTexture->surface);
#endif
#ifdef ENABLE_OPENGL
          glDeleteTextures(1,(GLuint *)&pTexture->nID);
#endif
      
	  //keesj:todo when using SDL surface we cannot delete the image data
	  //this is a problem.
	  //delete [] pc; => it's why i keep pTexture->pcData
#ifdef ENABLE_SDLGFX
	  delete [] pTexture->pcData;
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
  Texture* TextureManager::loadTexture(std::string Path,bool bSmall,bool bClamp,
				       FilterMode eFilterMode, bool persistent,
				       Sprite* associatedSprite) {
    /* Check file validity */
    image_info_t ii;
    Img TextureImage;
    Texture *pTexture = NULL;
    
    /* Name it */
    std::string TexName = FS::getFileBaseName(Path);

    /* if the texture is already loaded, return it */
    pTexture = getTexture(TexName);
    if(pTexture != NULL) {
      pTexture->addAssociatedSprite(associatedSprite);
      return pTexture;
    }

    if(TextureImage.checkFile( Path,&ii )) {
      /* Valid texture size? */
      if(ii.nWidth != ii.nHeight) {
        LogInfo("** Warning ** : TextureManager::loadTexture() : texture '%s' is not square",Path.c_str());
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
        LogInfo("** Warning ** : TextureManager::loadTexture() : texture '%s' size is not power of two",Path.c_str());
        throw TextureError("texture size not power of two");
      }
         
      /* Load it into system memory */
      TextureImage.loadFile(Path,bSmall);
      
      /* Copy it into video memory */
      unsigned char *pc;
      bool bAlpha = TextureImage.isAlpha();
      if(bAlpha){
        pc = TextureImage.convertToRGBA32();
      } else {
        pc = TextureImage.convertToRGB24();
      }
      
      pTexture = createTexture(TexName,pc,TextureImage.getWidth(),TextureImage.getHeight(),bAlpha,bClamp, eFilterMode);
      pTexture->addAssociatedSprite(associatedSprite);
    }
    else {
      LogInfo("** Warning ** : TextureManager::loadTexture() : texture '%s' not found or invalid",Path.c_str());
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
  Unload everything in a very hateful manner
  ===========================================================================*/
  void TextureManager::unloadTextures(void) {
    while(!m_Textures.empty())
      destroyTexture(m_Textures[0]);
  }
