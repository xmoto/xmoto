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

#ifndef __THEME_H__
#define __THEME_H__

#include <string>
#include <vector>
#include "VTexture.h"
#include "VFileIO.h"

#define SPRITE_FILE_DIR "Textures"
#define DECORATION_SPRITE_FILE_DIR SPRITE_FILE_DIR"/Sprites"
#define EFFECT_SPRITE_FILE_DIR     SPRITE_FILE_DIR"/Effects"
#define MISC_SPRITE_FILE_DIR       SPRITE_FILE_DIR"/Misc"
#define ANIMATION_SPRITE_FILE_DIR  SPRITE_FILE_DIR"/Anims"

  enum SpriteType {
    SPRITE_TYPE_ANIMATION,
    SPRITE_TYPE_BIKERPART,
    SPRITE_TYPE_DECORATION,
    SPRITE_TYPE_EFFECT,
    SPRITE_TYPE_FONT,
    SPRITE_TYPE_MISC,
    SPRITE_TYPE_TEXTURE
  };

class Theme;

class Sprite {
  public:
  Sprite(Theme* p_associated_theme, std::string v_name);
  ~Sprite();

  virtual enum SpriteType getType() = 0;
  std::string getName();
  void load();
  vapp::Texture* getTexture();

 protected:
  virtual vapp::Texture* getCurrentTexture() = 0;
  virtual std::string getCurrentTextureFileName() = 0;
  virtual void setCurrentTexture(vapp::Texture *p_texture) = 0;
  virtual std::string getFileDir();

  private:
  Theme* m_associated_theme;
  std::string m_name;
};

class SimpleFrameSprite : public Sprite {
 public:
  SimpleFrameSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName);
  ~SimpleFrameSprite();

 protected:
  vapp::Texture* getCurrentTexture();
  std::string getCurrentTextureFileName();
  void setCurrentTexture(vapp::Texture *p_texture);

 private:
  std::string m_fileName;
  vapp::Texture* m_texture;
};

class TextureSprite : public SimpleFrameSprite {
 public:
  TextureSprite(Theme* p_associated_theme, std::string p_name, std::string p_filename);
  ~TextureSprite();
  enum SpriteType getType();

 private:
};

class BikerPartSprite : public SimpleFrameSprite {
 public:
  BikerPartSprite(Theme* p_associated_theme, std::string p_name, std::string p_filename);
  ~BikerPartSprite();
  enum SpriteType getType();

 private:
};

class AnimationSprite;

class AnimationSpriteFrame {
 public:
  AnimationSpriteFrame(AnimationSprite *p_associatedAnimationSprite,
		       float p_centerX,
		       float p_centerY,
		       float p_width,
		       float p_height,
		       float p_delay
		       );
  ~AnimationSpriteFrame();
  vapp::Texture *getTexture();
  void  setTexture(vapp::Texture *p_texture);
  float getCenterX() const;
  float getCenterY() const;
  float getWidth() const;
  float getHeight() const;
  float getDelay() const;

 private:
  AnimationSprite *m_associatedAnimationSprite;

  vapp::Texture* m_texture;
  float m_centerX;
  float m_centerY;
  float m_width;
  float m_height;
  float m_delay;
};

class AnimationSprite : public Sprite {
 public:
  AnimationSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileBase, std::string p_fileExtention);
  ~AnimationSprite();
  enum SpriteType getType();
  float getCenterX();
  float getCenterY();
  float getWidth();
  float getHeight();
  void  addFrame(float p_centerX, float p_centerY, float p_width, float p_height, float p_delay);

 protected:
  vapp::Texture* getCurrentTexture();
  std::string getCurrentTextureFileName();
  void setCurrentTexture(vapp::Texture *p_texture);
  std::string getFileDir();

 private:
  int getCurrentFrame();

  std::string m_fileBase;
  std::string m_fileExtension;
  int m_current_frame;
  void AnimationSprite::cleanFrames();
  std::vector<AnimationSpriteFrame*> m_frames;
  float m_fFrameTime;
};

class EffectSprite : public SimpleFrameSprite {
 public:
  EffectSprite(Theme* p_associated_theme, std::string p_name, std::string p_filename);
  ~EffectSprite();
  enum SpriteType getType();

 protected:
  std::string getFileDir();

 private:
};

class FontSprite : public SimpleFrameSprite {
 public:
  FontSprite(Theme* p_associated_theme, std::string p_name, std::string p_filename);
  ~FontSprite();
  enum SpriteType getType();

 private:
};

class MiscSprite : public SimpleFrameSprite {
 public:
  MiscSprite(Theme* p_associated_theme, std::string p_name, std::string p_filename);
  ~MiscSprite();
  enum SpriteType getType();

 protected:
  std::string getFileDir();

 private:
};

class DecorationSprite : public SimpleFrameSprite {
 public:
  DecorationSprite(Theme* p_associated_theme, std::string p_name, std::string p_filename, float p_width, float p_height, float p_centerX, float p_centerY);
  ~DecorationSprite();
  enum SpriteType getType();

  float getWidth();
  float getHeight();
  float getCenterX();
  float getCenterY();

 protected:
  std::string getFileDir();

 private:
  float m_width;
  float m_height;
  float m_centerX;
  float m_centerY;

};

//class CompositionTheme {
// public:
//  CompositionTheme();
//  ~CompositionTheme();
//
// private:
//  void cleanComponents()
//  vector<vapp::Texture*> m_components;
//};
//
//class RiderTheme : CompositionSprite {
// public:
//  vapp::Texture* getBikerBody();
//  vapp::Texture* getBikerFront();
//  vapp::Texture* getBikerRear();
//  vapp::Texture* getWheel();
//  vapp::Texture* getLowerArm();
//  vapp::Texture* getLowerLeg();
//  vapp::Texture* getTorso();
//  vapp::Texture* getUpperArm();
//  vapp::Texture* getUpperLeg();
//
// private:
//};

class Theme {
  public:
  Theme();
  ~Theme();
 
  void load();
  void load(std::string p_themeFile);

  std::string getDefaultThemeFile();
  Sprite* getSprite(enum SpriteType pSpriteType, std::string pName);
  vapp::Texture* loadTexture(std::string p_fileName);

  private:
  vapp::TextureManager m_texMan;
  std::string m_name;
  std::vector<Sprite*> m_sprites;

  void loadSprites();
  void cleanSprites();

  void newAnimationSpriteFromXML(TiXmlElement *pVarElem);
  void newBikerPartSpriteFromXML(TiXmlElement *pVarElem);
  void newDecorationSpriteFromXML(TiXmlElement *pVarElem);
  void newEffectSpriteFromXML(TiXmlElement *pVarElem);
  void newFontSpriteFromXML(TiXmlElement *pVarElem);
  void newMiscSpriteFromXML(TiXmlElement *pVarElem);
  void newTextureSpriteFromXML(TiXmlElement *pVarElem);
};

#endif /* __THEME_H__ */
