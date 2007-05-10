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

#include "BuildConfig.h"
#include "VTexture.h"
#include "VFileIO.h"
#include "WWW.h"

class ProxySettings;
class WebThemes;
class xmDatabase;

#define THEMES_DIRECTORY "Themes"
#define THEME_SPRITE_FILE_DIR "Textures"
#define THEME_DECORATION_SPRITE_FILE_DIR THEME_SPRITE_FILE_DIR"/Sprites"
#define THEME_EFFECT_SPRITE_FILE_DIR     THEME_SPRITE_FILE_DIR"/Effects"
#define THEME_MISC_SPRITE_FILE_DIR       THEME_SPRITE_FILE_DIR"/Misc"
#define THEME_ANIMATION_SPRITE_FILE_DIR  THEME_SPRITE_FILE_DIR"/Anims"
#define THEME_BIKERPART_SPRITE_FILE_DIR  THEME_SPRITE_FILE_DIR"/Riders"
#define THEME_UI_SPRITE_FILE_DIR         THEME_SPRITE_FILE_DIR"/UI"
#define THEME_TEXTURE_SPRITE_FILE_DIR    THEME_SPRITE_FILE_DIR"/Textures"
#define THEME_FONT_SPRITE_FILE_DIR       THEME_SPRITE_FILE_DIR"/Fonts"
#define THEME_EDGEEFFECT_SPRITE_FILE_DIR THEME_SPRITE_FILE_DIR"/Effects"
#define THEME_MUSICS_FILE_DIR            THEME_SPRITE_FILE_DIR"/Musics"
#define THEME_SOUNDS_FILE_DIR            THEME_SPRITE_FILE_DIR"/Sounds"

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

#define THEME_DEFAULT_THEMENAME "Classic"

  enum SpriteType {
    SPRITE_TYPE_ANIMATION,
    SPRITE_TYPE_BIKERPART,
    SPRITE_TYPE_DECORATION,
    SPRITE_TYPE_EFFECT,
    SPRITE_TYPE_FONT,
    SPRITE_TYPE_MISC,
    SPRITE_TYPE_TEXTURE,
    SPRITE_TYPE_UI,
    SPRITE_TYPE_EDGEEFFECT,
  };
  
  enum SpriteBlendMode {
    SPRITE_BLENDMODE_DEFAULT,
    SPRITE_BLENDMODE_ADDITIVE
  };

class Theme;
class BikerTheme;

class Music {
 public:
  Music(Theme* p_associated_theme, std::string i_name, std::string i_fileName);
  ~Music();

  std::string Name() const;
  std::string FileName() const;
  std::string FilePath() const;

 private:
  Theme* m_associated_theme;
  std::string m_name;
  std::string m_fileName;
};

class Sound {
 public:
  Sound(Theme* p_associated_theme, std::string i_name, std::string i_fileName);
  ~Sound();

  std::string Name() const;
  std::string FileName() const;
  std::string FilePath() const;

 private:
  Theme* m_associated_theme;
  std::string m_name;
  std::string m_fileName;
};

class Sprite {
  public:
  Sprite(Theme* p_associated_theme, std::string v_name);
  virtual ~Sprite();

  /* no inline with virtual functions, so, no longer virtual */
  inline enum SpriteType getType() {
    return m_type;
  }
  /* called many many many times, so we inline it, and make it return a ref */
  inline std::string& getName() {
    return m_name;
  }
  SpriteBlendMode getBlendMode();
  void setBlendMode(SpriteBlendMode Mode);
  void load();

  /* 
     the bSmall, bClamp, bFilter parameters are considerated 
     only the first time that getTexture is called for a given sprite
  */
  vapp::Texture* getTexture(bool bSmall=false, bool bClamp=false, vapp::FilterMode eFilterMode = vapp::FM_LINEAR);

 protected:
  virtual vapp::Texture* getCurrentTexture() = 0;
  virtual std::string getCurrentTextureFileName() = 0;
  virtual void setCurrentTexture(vapp::Texture *p_texture) = 0;
  virtual std::string getFileDir();
  enum SpriteType m_type;

  private:
  Theme* m_associated_theme;
  std::string m_name;
  SpriteBlendMode m_blendmode;
};

class SimpleFrameSprite : public Sprite {
 public:
  SimpleFrameSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName);
  virtual ~SimpleFrameSprite();

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
  virtual ~TextureSprite();

 protected:
  std::string getFileDir();

 private:
};

class BikerPartSprite : public SimpleFrameSprite {
 public:
  BikerPartSprite(Theme* p_associated_theme, std::string p_name, std::string p_filename);
  virtual ~BikerPartSprite();

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
  virtual ~AnimationSpriteFrame();
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
  virtual ~AnimationSprite();
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
  unsigned int m_current_frame;
  void cleanFrames();
  std::vector<AnimationSpriteFrame*> m_frames;
  float m_fFrameTime;
};

class EffectSprite : public SimpleFrameSprite {
 public:
  EffectSprite(Theme* p_associated_theme, std::string p_name, std::string p_filename);
  virtual ~EffectSprite();

 protected:
  std::string getFileDir();

 private:
};

class EdgeEffectSprite : public SimpleFrameSprite {
 public:
  EdgeEffectSprite(Theme* p_associated_theme, std::string p_name, std::string p_filename, float fScale, float fDepth);
  virtual ~EdgeEffectSprite();

  float getScale() const;
  float getDepth() const;

 protected:
  std::string getFileDir();

 private:
  float m_fScale;
  float m_fDepth;
};

class FontSprite : public SimpleFrameSprite {
 public:
  FontSprite(Theme* p_associated_theme, std::string p_name, std::string p_filename);
  virtual ~FontSprite();

 protected:
  std::string getFileDir();

 private:
};

class MiscSprite : public SimpleFrameSprite {
 public:
  MiscSprite(Theme* p_associated_theme, std::string p_name, std::string p_filename);
  virtual ~MiscSprite();

 protected:
  std::string getFileDir();

 private:
};

class UISprite : public SimpleFrameSprite {
 public:
  UISprite(Theme* p_associated_theme, std::string p_name, std::string p_filename);
  virtual ~UISprite();

 protected:
  std::string getFileDir();

 private:
};

class DecorationSprite : public SimpleFrameSprite {
 public:
  DecorationSprite(Theme* p_associated_theme, std::string p_name, std::string p_filename, float p_width, float p_height, float p_centerX, float p_centerY, SpriteBlendMode p_blendmode);
  virtual ~DecorationSprite();

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

  std::string Name() const;
  Sprite* getSprite(enum SpriteType pSpriteType, std::string pName);
  Music* getMusic(std::string i_name);
  Sound* getSound(std::string i_name);
  vapp::Texture* loadTexture(std::string p_fileName,
			     bool bSmall=false,
			     bool bClamp=false,
			     vapp::FilterMode eFilterMode = vapp::FM_LINEAR);

  std::vector<Sprite*> getSpritesList();
  std::vector<Sound*> getSoundsList();
  std::vector<std::string>* getRequiredFiles();

  BikerTheme* getPlayerTheme();
  BikerTheme* getGhostTheme();

  private:
  vapp::TextureManager m_texMan;
  std::string m_name;
  std::vector<Sprite*> m_sprites;
  std::vector<Music*> m_musics;
  std::vector<Sound*> m_sounds;
  std::vector<std::string> m_requiredFiles;

  BikerTheme *m_player;
  BikerTheme *m_ghost;

  void cleanSprites();
  void cleanMusics();
  void cleanSounds();

  void loadSpritesFromXML(TiXmlElement *p_ThemeXmlDataElement);

  void newAnimationSpriteFromXML(TiXmlElement *pVarElem);
  void newBikerPartSpriteFromXML(TiXmlElement *pVarElem);
  void newDecorationSpriteFromXML(TiXmlElement *pVarElem);
  void newEffectSpriteFromXML(TiXmlElement *pVarElem);
  void newEdgeEffectSpriteFromXML(TiXmlElement *pVarElem);
  void newFontSpriteFromXML(TiXmlElement *pVarElem);
  void newMiscSpriteFromXML(TiXmlElement *pVarElem);
  void newTextureSpriteFromXML(TiXmlElement *pVarElem);
  void newUISpriteFromXML(TiXmlElement *pVarElem);
  
  SpriteBlendMode strToBlendMode(const std::string &s);
  std::string blendModeToStr(SpriteBlendMode Mode);
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

class ThemeChoicer {
 public:

  ThemeChoicer(vapp::WWWAppInterface *p_WebApp = NULL,
	       const ProxySettings *p_proxy_settings = NULL);	       
  ~ThemeChoicer();

  void updateFromWWW(xmDatabase *i_db);
  void updateThemeFromWWW(xmDatabase *i_db, const std::string& i_id_theme);

  void setURL(const std::string &p_url);
  void setURLBase(const std::string &p_urlBase);
  void initThemesFromDir(xmDatabase *i_db);

 private:
  WebThemes *m_webThemes;
  std::string getThemeNameFromFile(std::string p_themeFile);
};

#endif /* __THEME_H__ */
