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

#ifndef __THEME_H__
#define __THEME_H__

#include <string>
#include <vector>

#include "VTexture.h"
#include "VXml.h"
#include "helpers/Singleton.h"
#include "helpers/Color.h"
#include "VFileIO_types.h"

class Texture;
class WWWAppInterface;
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
#define THEME_PLAYER_GRAPHICS_LOW_BIKER MAKE_COLOR(0,0,0,255)
#define THEME_PLAYER_GRAPHICS_LOW_FILL MAKE_COLOR(180,180,180,255)
#define THEME_PLAYER_GRAPHICS_LOW_WHEEL MAKE_COLOR(0,0,0,255)


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
#define THEME_GHOST_GRAPHICS_LOW_BIKER MAKE_COLOR(120,120,120,255)
#define THEME_GHOST_GRAPHICS_LOW_FILL MAKE_COLOR(245,245,245,255)
#define THEME_GHOST_GRAPHICS_LOW_WHEEL MAKE_COLOR(120,120,120,255)


  enum SpriteType {
    SPRITE_TYPE_ANIMATION,
    SPRITE_TYPE_ANIMATION_TEXTURE,
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




class ThemeMusic {
 public:
  ThemeMusic(Theme* p_associated_theme, std::string i_name, std::string i_fileName);
  ~ThemeMusic();

  std::string Name() const;
  std::string FileName() const;
  std::string FilePath() const;

 private:
  Theme* m_associated_theme;
  std::string m_name;
  std::string m_fileName;
};

class ThemeSound {
 public:
  ThemeSound(Theme* p_associated_theme, std::string i_name, std::string i_fileName);
  ~ThemeSound();

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

  inline unsigned int getOrder() {
    return m_order;
  }
  void setOrder(unsigned int order);

  SpriteBlendMode getBlendMode();
  void setBlendMode(SpriteBlendMode Mode);
  void load();

  /* 
     the bSmall, bClamp, bFilter parameters are considerated 
     only the first time that getTexture is called for a given sprite
  */
  Texture* getTexture(bool bSmall=false, bool bClamp=false, FilterMode eFilterMode = FM_MIPMAP);
  // do not load the texture in opengl, just get its size.
  // returns 0 on error
  int getTextureSize();

  // prefetch textures at level loading time
  virtual void loadTextures() = 0;
  virtual void invalidateTextures() = 0;
  virtual std::string getCurrentTextureFileName() = 0;

 protected:
  virtual Texture* getCurrentTexture() = 0;
  virtual void setCurrentTexture(Texture *p_texture) = 0;
  virtual std::string getFileDir();
  enum SpriteType m_type;
  // to sort entities by sprite for rendering
  unsigned int m_order;
  bool m_persistent;

  private:
  Theme* m_associated_theme;
  std::string m_name;
  SpriteBlendMode m_blendmode;
};




class SimpleFrameSprite : public Sprite {
 public:
  SimpleFrameSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName);
  virtual ~SimpleFrameSprite();

  void loadTextures();
  void invalidateTextures();
  std::string getCurrentTextureFileName();

 protected:
  Texture* getCurrentTexture();
  void setCurrentTexture(Texture *p_texture);

 private:
  std::string m_fileName;
  Texture* m_texture;
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
  Texture *getTexture();
  void  setTexture(Texture *p_texture);
  float getCenterX() const;
  float getCenterY() const;
  float getWidth() const;
  float getHeight() const;
  float getDelay() const;

 private:
  AnimationSprite *m_associatedAnimationSprite;

  Texture* m_texture;
  float m_centerX;
  float m_centerY;
  float m_width;
  float m_height;
  float m_delay;
};





class AnimationSprite : public Sprite {
 public:
  AnimationSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileBase, std::string p_fileExtention, bool p_isTexture);
  virtual ~AnimationSprite();
  float getCenterX();
  float getCenterY();
  float getWidth();
  float getHeight();
  void  addFrame(float p_centerX, float p_centerY, float p_width, float p_height, float p_delay);

  void loadTextures();
  void invalidateTextures();
  std::string getCurrentTextureFileName();

 protected:
  Texture* getCurrentTexture();
  void setCurrentTexture(Texture *p_texture);
  std::string getFileDir();

 private:
  int getCurrentFrame();

  std::string m_fileBase;
  std::string m_fileExtension;
  bool m_isTexture;
  unsigned int m_current_frame;
  void cleanFrames();
  std::vector<AnimationSpriteFrame*> m_frames;
  float m_fFrameTime;
  bool  m_animation;
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
  Texture* getTexture(bool bSmall=false, bool bClamp=false, FilterMode eFilterMode = FM_LINEAR);

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



struct ThemeFile {
  std::string filepath;
  std::string filemd5;
};






class Theme : public Singleton<Theme> {
  friend class Singleton<Theme>;

private:
  Theme();
  ~Theme();

public:
  void load(FileDataType i_fdt, std::string p_themeFile);

  std::string Name() const;
  Sprite* getSprite(enum SpriteType pSpriteType, std::string pName);
  std::string getHashMusic(const std::string& i_key);
  ThemeMusic* getMusic(std::string i_name);
  ThemeSound* getSound(std::string i_name);
  Texture* loadTexture(std::string p_fileName,
		       bool bSmall=false,
		       bool bClamp=false,
		       FilterMode eFilterMode = FM_MIPMAP,
		       bool persistent=false,
		       Sprite* associateSprite=NULL);
  int getTextureSize(std::string p_fileName);

  std::vector<Sprite*>& getSpritesList();
  std::vector<ThemeSound*>& getSoundsList();
  std::vector<ThemeFile>* getRequiredFiles();

  BikerTheme* getPlayerTheme();
  BikerTheme* getNetPlayerTheme();
  BikerTheme* getGhostTheme();

  TextureManager* getTextureManager() {
    return &m_texMan;
  }

  private:
  TextureManager m_texMan;
  std::string m_name;
  std::vector<Sprite*> m_sprites;
  std::vector<ThemeMusic*> m_musics;
  std::vector<ThemeSound*> m_sounds;
  std::vector<ThemeFile> m_requiredFiles;

  bool isAFileOutOfDate(const std::string& i_file); // to not download old files for compatibilities

  BikerTheme *m_player;
  BikerTheme *m_netplayer;
  BikerTheme *m_ghost;

  void cleanSprites();
  void cleanMusics();
  void cleanSounds();

  void loadSpritesFromXML(xmlNodePtr pElem);

  // use template instead of duplicate code
  template <typename SpriteType> void newSpriteFromXML(xmlNodePtr pElem,
						       const char*   fileDir,
						       const char*   spriteTypeName);

  void newAnimationSpriteFromXML(xmlNodePtr pElem, bool p_isTexture, const char* fileDir );
  void newDecorationSpriteFromXML(xmlNodePtr pElem);

  void newEdgeEffectSpriteFromXML(xmlNodePtr pElem);

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
	     Color p_UglyRiderColor,
	     Color p_UglyWheelColor,
	     bool p_ghostEffect,
	     Color p_gfxLowRiderColor,
	     Color p_gfxLowFillColor,
	     Color p_gfxLowWheelColor
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

  Color getUglyRiderColor();
  Color getUglyWheelColor();
  Color getGfxLowRiderColor();
  Color getGfxLowFillColor();
  Color getGfxLowWheelColor();

  bool getGhostEffect() const;

 private:
  Theme* m_associated_theme;

  Color m_UglyRiderColor;
  Color m_UglyWheelColor;
  Color m_gfxLowRiderColor;
  Color m_gfxLowFillColor;
  Color m_gfxLowWheelColor;
  
  bool m_ghostEffect;
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
  static void initThemesFromDir(xmDatabase *i_db);

 private:
  static std::string getThemeNameFromFile(std::string p_themeFile);
};

#endif /* __THEME_H__ */
