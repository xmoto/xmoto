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

#define THEME_SPRITE_FILE_DIR "Textures"
#define THEME_DECORATION_SPRITE_FILE_DIR THEME_SPRITE_FILE_DIR"/Sprites"
#define THEME_EFFECT_SPRITE_FILE_DIR     THEME_SPRITE_FILE_DIR"/Effects"
#define THEME_MISC_SPRITE_FILE_DIR       THEME_SPRITE_FILE_DIR"/Misc"
#define THEME_ANIMATION_SPRITE_FILE_DIR  THEME_SPRITE_FILE_DIR"/Anims"
#define THEME_BIKERPART_SPRITE_FILE_DIR  THEME_SPRITE_FILE_DIR"/Riders"
#define THEME_UI_SPRITE_FILE_DIR         THEME_SPRITE_FILE_DIR"/UI"
#define THEME_TEXTURE_SPRITE_FILE_DIR    THEME_SPRITE_FILE_DIR"/Textures"
#define THEME_FONT_SPRITE_FILE_DIR       THEME_SPRITE_FILE_DIR"/Fonts"

#define THEME_PLAYER_BODY     "PlayerBikerBody"
#define THEME_PLAYER_FRONT    "PlayerBikerFront"
#define THEME_PLAYER_REAR     "PlayerBikerRear"
#define THEME_PLAYER_WHEEL    "PlayerBikerWheel"
#define THEME_PLAYER_LOWERARM "PlayerLowerArm"
#define THEME_PLAYER_LOWERLEG "PlayerLowerLeg"
#define THEME_PLAYER_TORSO    "PlayerTorso"
#define THEME_PLAYER_UPPERARM "PlayerUpperArm"
#define THEME_PLAYER_UPPERLEG "PlayerUpperLeg"
#define THEME_PLAYER_UGLYRIDERCOLOR MAKE_COLOR(0,255,0,255)
#define THEME_PLAYER_UGLYWHEELCOLOR MAKE_COLOR(255,0,0,255)

#if defined(ALLOW_GHOST)
#define THEME_GHOST_BODY     "GhostBikerBody"
#define THEME_GHOST_FRONT    "GhostBikerFront"
#define THEME_GHOST_REAR     "GhostBikerRear"
#define THEME_GHOST_WHEEL    "GhostBikerWheel"
#define THEME_GHOST_LOWERARM "GhostLowerArm"
#define THEME_GHOST_LOWERLEG "GhostLowerLeg"
#define THEME_GHOST_TORSO    "GhostTorso"
#define THEME_GHOST_UPPERARM "GhostUpperArm"
#define THEME_GHOST_UPPERLEG "GhostUpperLeg"
#define THEME_GHOST_UGLYRIDERCOLOR MAKE_COLOR(100,100,128,255)
#define THEME_GHOST_UGLYWHEELCOLOR MAKE_COLOR(100,100,128,255)
#endif

#define THEME_DEFAULT_THEMENAME "Classic"

  enum SpriteType {
    SPRITE_TYPE_ANIMATION,
    SPRITE_TYPE_BIKERPART,
    SPRITE_TYPE_DECORATION,
    SPRITE_TYPE_EFFECT,
    SPRITE_TYPE_FONT,
    SPRITE_TYPE_MISC,
    SPRITE_TYPE_TEXTURE,
    SPRITE_TYPE_UI
  };

class Theme;
class BikerTheme;

class Sprite {
  public:
  Sprite(Theme* p_associated_theme, std::string v_name);
  ~Sprite();

  virtual enum SpriteType getType() = 0;
  std::string getName();
  void load();

  /* 
     the bSmall, bClamp, bFilter parameters are considerated 
     only the first time that getTexture is called for a given sprite
  */
  vapp::Texture* getTexture(bool bSmall=false, bool bClamp=false, bool bFilter=true);

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

 protected:
  std::string getFileDir();

 private:
};

class BikerPartSprite : public SimpleFrameSprite {
 public:
  BikerPartSprite(Theme* p_associated_theme, std::string p_name, std::string p_filename);
  ~BikerPartSprite();
  enum SpriteType getType();

 protected:
  std::string getFileDir();

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
  void cleanFrames();
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

 protected:
  std::string getFileDir();

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

class UISprite : public SimpleFrameSprite {
 public:
  UISprite(Theme* p_associated_theme, std::string p_name, std::string p_filename);
  ~UISprite();
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

class Theme {
  public:
  Theme();
  ~Theme();
 
  void load(std::string p_themeFile);

  Sprite* getSprite(enum SpriteType pSpriteType, std::string pName);
  vapp::Texture* loadTexture(std::string p_fileName,
			     bool bSmall=false,
			     bool bClamp=false,
			     bool bFilter=true);

  vapp::Texture* getDefaultFont();
  std::vector<Sprite*> getSpritesList();

  BikerTheme* getPlayerTheme();
#if defined(ALLOW_GHOST)
  BikerTheme* getGhostTheme();
#endif

  private:
  void initDefaultFont();

  vapp::TextureManager m_texMan;
  std::string m_name;
  std::vector<Sprite*> m_sprites;

  vapp::Texture *m_pDefaultFontTexture;

  BikerTheme *m_player;
#if defined(ALLOW_GHOST)
  BikerTheme *m_ghost;
#endif

  void cleanSprites();

  void loadSpritesFromXML(TiXmlElement *p_ThemeXmlDataElement);

  void newAnimationSpriteFromXML(TiXmlElement *pVarElem);
  void newBikerPartSpriteFromXML(TiXmlElement *pVarElem);
  void newDecorationSpriteFromXML(TiXmlElement *pVarElem);
  void newEffectSpriteFromXML(TiXmlElement *pVarElem);
  void newFontSpriteFromXML(TiXmlElement *pVarElem);
  void newMiscSpriteFromXML(TiXmlElement *pVarElem);
  void newTextureSpriteFromXML(TiXmlElement *pVarElem);
  void newUISpriteFromXML(TiXmlElement *pVarElem);
};

class BikerTheme {
 public:
  BikerTheme(Theme* p_associated_theme,
	     std::string p_Body,
	     std::string p_Front,
	     std::string p_Rear,
	     std::string p_Wheel,
	     std::string p_LowerArm,
	     std::string p_LowerLeg,
	     std::string p_Torso,
	     std::string p_UpperArm,
	     std::string p_UpperLeg,
	     vapp::Color p_UglyRiderColor,
	     vapp::Color p_UglyWheelColor
	     );
  ~BikerTheme();

  Sprite* getBody();
  Sprite* getFront();
  Sprite* getRear();
  Sprite* getWheel();
  Sprite* getLowerArm();
  Sprite* getLowerLeg();
  Sprite* getTorso();
  Sprite* getUpperArm();
  Sprite* getUpperLeg();

  vapp::Color getUglyRiderColor();
  vapp::Color getUglyWheelColor();

 private:
  Theme* m_associated_theme;

  vapp::Color m_UglyRiderColor;
  vapp::Color m_UglyWheelColor;
  
  std::string m_Body;
  std::string m_Front;
  std::string m_Rear;
  std::string m_Wheel;
  std::string m_LowerArm;
  std::string m_LowerLeg;
  std::string m_Torso;
  std::string m_UpperArm;
  std::string m_UpperLeg;
};

class ThemeChoice {
 public:
  ThemeChoice(std::string p_themeName, std::string p_themeFile);
  ~ThemeChoice();
  std::string ThemeName();
  std::string ThemeFile();

 private:
  std::string m_themeName;
  std::string m_themeFile;
};

class ThemeChoicer {
 public:
  ThemeChoicer();
  ~ThemeChoicer();

  bool ExistThemeName(std::string p_themeName);
  std::string getFileName(std::string p_themeName);

 private:
  void cleanList();
  void initList();
  std::string getThemeNameFromFile(std::string p_themeFile);

  std::vector<ThemeChoice*> m_choices;
};

#endif /* __THEME_H__ */
