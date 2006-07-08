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

#include "Theme.h"
#include "VExcept.h"
#include "VXml.h"
#include "VApp.h"
#include "BuiltInFont.h"

class vapp::App;

std::vector<Sprite*> Theme::getSpritesList() {
  return m_sprites;
}

void Theme::initDefaultFont() {
    CBuiltInFont Fnt;
      
    /* Create texture */
    int nImgWidth = 256, nImgHeight = 256;
    vapp::Color *pImgData = new vapp::Color[nImgWidth * nImgHeight];
    memset(pImgData,0,nImgWidth * nImgHeight * sizeof(vapp::Color));
  
    /* Fill texture with glyphs */
    int cx=0,cy=0,w=Fnt.getCharWidth(),h=Fnt.getCharHeight();
    for(int i=0;i<256;i++) {
      unsigned char *pc = Fnt.fetchChar(i);
      for(int y=0;y<h;y++) {
        for(int x=0;x<w;x++) {
          unsigned char *pcT = (unsigned char *)&pImgData[(cx+x) + (cy+y)*nImgWidth];
          pcT[0] = 255;
          pcT[1] = 255;
          pcT[2] = 255;
          pcT[3] = pc[x+y*w];
        }
      }
      
      cx += w;
      if(cx+w > nImgWidth) {
        cx = 0;
        cy += h;
        if(cy+h > nImgHeight) {
          delete [] pImgData;
          throw vapp::TextureError("default font does not fit in texture");
        }
      }        
    }
    
    /* Load it */
    m_pDefaultFontTexture = m_texMan.createTexture("default-font",(unsigned char *)pImgData,
						   256,256,true);
      
    delete [] pImgData;
}

vapp::Texture* Theme::getDefaultFont() {
  if(m_pDefaultFontTexture == NULL) {
    initDefaultFont();
  }
  return m_pDefaultFontTexture;
}

Theme::Theme() {
  m_pDefaultFontTexture = NULL;

  m_player = new BikerTheme(this,
			    THEME_PLAYER_BODY,
			    THEME_PLAYER_FRONT,
			    THEME_PLAYER_REAR,
			    THEME_PLAYER_WHEEL,
			    THEME_PLAYER_LOWERARM,
			    THEME_PLAYER_LOWERLEG,
			    THEME_PLAYER_TORSO,
			    THEME_PLAYER_UPPERARM,
			    THEME_PLAYER_UPPERLEG,
			    THEME_PLAYER_UGLYRIDERCOLOR,
			    THEME_PLAYER_UGLYWHEELCOLOR
			    );

#if defined(ALLOW_GHOST)
  m_ghost = new BikerTheme(this,
			   THEME_GHOST_BODY,
			   THEME_GHOST_FRONT,
			   THEME_GHOST_REAR,
			   THEME_GHOST_WHEEL,
			   THEME_GHOST_LOWERARM,
			   THEME_GHOST_LOWERLEG,
			   THEME_GHOST_TORSO,
			   THEME_GHOST_UPPERARM,
			   THEME_GHOST_UPPERLEG,
			   THEME_GHOST_UGLYRIDERCOLOR,
			   THEME_GHOST_UGLYWHEELCOLOR
			   );
#endif
}

Theme::~Theme() {
  cleanSprites();
  
  /* Kill textures */
  m_texMan.unloadTextures();
}
 
vapp::Texture* Theme::loadTexture(std::string p_fileName, bool bSmall, bool bClamp, bool bFilter) {
  return m_texMan.loadTexture(p_fileName.c_str(), bSmall, bClamp, bFilter);
}

void Theme::load(std::string p_themeFile) {
  vapp::XMLDocument v_ThemeXml;
  TiXmlDocument *v_ThemeXmlData;
  TiXmlElement *v_ThemeXmlDataElement;
  const char *pc;

  try {

    /* open the file */
    v_ThemeXml.readFromFile(p_themeFile);
    
    v_ThemeXmlData = v_ThemeXml.getLowLevelAccess();
    
    if(v_ThemeXmlData == NULL) {
      throw vapp::Exception("error : unable analyse xml theme file");
    }
    
    /* read the theme name */
    v_ThemeXmlDataElement = v_ThemeXmlData->FirstChildElement("xmoto_theme");
    if(v_ThemeXmlDataElement != NULL) {
      pc = v_ThemeXmlDataElement->Attribute("name");
      m_name = pc;
    }
    
    if(m_name == "") {
      throw vapp::Exception("error : the theme has no name !");
    }
    
    /* get sprites */
    loadSpritesFromXML(v_ThemeXmlDataElement);

  } catch(vapp::Exception &e) {
    throw vapp::Exception("error : unable analyse xml theme file");
  }
}

void Theme::loadSpritesFromXML(TiXmlElement *p_ThemeXmlDataElement) {
  std::string v_spriteType;
  const char *pc;
  
  for(TiXmlElement *pVarElem = p_ThemeXmlDataElement->FirstChildElement("sprite");
      pVarElem!=NULL;
      pVarElem = pVarElem->NextSiblingElement("sprite")
      ) {
    pc = pVarElem->Attribute("type");

    if(pc == NULL) { continue; }
    v_spriteType = pc;

    if(v_spriteType == "Animation") {
      newAnimationSpriteFromXML(pVarElem);
    } else if(v_spriteType == "BikerPart") {
      newBikerPartSpriteFromXML(pVarElem);
    } else if(v_spriteType == "Decoration") {
      newDecorationSpriteFromXML(pVarElem);
    } else if(v_spriteType == "Effect") {
      newEffectSpriteFromXML(pVarElem);
    } else if(v_spriteType == "Font") {
      newFontSpriteFromXML(pVarElem);
    } else if(v_spriteType == "Misc") {
      newMiscSpriteFromXML(pVarElem);
    } else if(v_spriteType == "Texture") {
      newTextureSpriteFromXML(pVarElem);
    } else if(v_spriteType == "UI") {
      newUISpriteFromXML(pVarElem);
    } else {
      vapp::Log("Warning: unknown type '%s' in theme file !", v_spriteType.c_str());
    }
  }
}

Sprite* Theme::getSprite(enum SpriteType pSpriteType, std::string pName) {
  for(int i=0; i<m_sprites.size(); i++) {
    if(m_sprites[i]->getType() == pSpriteType) {
      if(m_sprites[i]->getName() == pName) {
	return m_sprites[i];
      }
    }
  }

  return NULL;
}

void Theme::cleanSprites() {
  for(int i=0; i<m_sprites.size(); i++) {
    delete m_sprites[i];
  }
  m_sprites.clear();
}

void Theme::newAnimationSpriteFromXML(TiXmlElement *pVarElem) {
  std::string v_name;
  std::string v_fileBase;
  std::string v_fileExtension;
  const char *pc;
  AnimationSprite *v_anim;

  float global_centerX = 0.5;
  float global_centerY = 0.5;
  float global_width   = 1.0;
  float global_height  = 1.0;
  float global_delay   = 0.1;

  pc = pVarElem->Attribute("name");
  if(pc == NULL) {return;}
  v_name = pc;

  pc = pVarElem->Attribute("fileBase");
  if(pc == NULL) {return;}
  v_fileBase = pc;

  pc = pVarElem->Attribute("fileExtension");
  if(pc == NULL) {return;}
  v_fileExtension = pc;

  pc = pVarElem->Attribute("centerX");
  if(pc != NULL) {global_centerX = atof(pc);}

  pc = pVarElem->Attribute("centerY");
  if(pc != NULL) {global_centerY = atof(pc);}

  pc = pVarElem->Attribute("width");
  if(pc != NULL) {global_width = atof(pc);}

  pc = pVarElem->Attribute("height");
  if(pc != NULL) {global_height = atof(pc);}

  pc = pVarElem->Attribute("delay");
  if(pc != NULL) {global_delay = atof(pc);}

  v_anim = new AnimationSprite(this, v_name, v_fileBase, v_fileExtension);
  m_sprites.push_back(v_anim);

  for(TiXmlElement *pVarSubElem = pVarElem->FirstChildElement("frame");
      pVarSubElem!=NULL;
      pVarSubElem = pVarSubElem->NextSiblingElement("frame")
      ) {
    float v_centerX;
    float v_centerY;
    float v_width;
    float v_height;
    float v_delay;

    pc = pVarSubElem->Attribute("centerX");
    if(pc != NULL) {v_centerX = atof(pc);} else {v_centerX = global_centerX;}
    
    pc = pVarSubElem->Attribute("centerY");
    if(pc != NULL) {v_centerY = atof(pc);} else {v_centerY = global_centerY;}
    
    pc = pVarSubElem->Attribute("width");
    if(pc != NULL) {v_width = atof(pc);} else {v_width = global_width;}
    
    pc = pVarSubElem->Attribute("height");
    if(pc != NULL) {v_height = atof(pc);} else {v_height = global_height;}

    pc = pVarSubElem->Attribute("delay");
    if(pc != NULL) {v_delay = atof(pc);} else {v_delay = global_delay;}

    v_anim->addFrame(v_centerX, v_centerY, v_width, v_height, v_delay);
  }
}

void Theme::newBikerPartSpriteFromXML(TiXmlElement *pVarElem) {
  std::string v_name;
  std::string v_fileName;
  const char *pc;

  pc = pVarElem->Attribute("name");
  if(pc == NULL) {return;}
  v_name = pc;

  pc = pVarElem->Attribute("file");
  if(pc == NULL) {return;}
  v_fileName = pc;

  m_sprites.push_back(new BikerPartSprite(this, v_name, v_fileName));
}

void Theme::newDecorationSpriteFromXML(TiXmlElement *pVarElem) {
  std::string v_name;
  std::string v_fileName;
  std::string v_width;
  std::string v_height;
  std::string v_centerX;
  std::string v_centerY;
  const char *pc;

  pc = pVarElem->Attribute("name");
  if(pc == NULL) {return;}
  v_name = pc;

  pc = pVarElem->Attribute("file");
  if(pc == NULL) {return;}
  v_fileName = pc;

  pc = pVarElem->Attribute("width");
  if(pc == NULL) {return;}
  v_width = pc;

  pc = pVarElem->Attribute("height");
  if(pc == NULL) {return;}
  v_height = pc;

  pc = pVarElem->Attribute("centerX");
  if(pc == NULL) {return;}
  v_centerX = pc;

  pc = pVarElem->Attribute("centerY");
  if(pc == NULL) {return;}
  v_centerY = pc;

  m_sprites.push_back(new DecorationSprite(this, v_name, v_fileName,
					   atof(v_width.c_str()),
					   atof(v_height.c_str()),
					   atof(v_centerX.c_str()),
					   atof(v_centerY.c_str())
					   ));
}

void Theme::newEffectSpriteFromXML(TiXmlElement *pVarElem) {
  std::string v_name;
  std::string v_fileName;
  const char *pc;

  pc = pVarElem->Attribute("name");
  if(pc == NULL) {return;}
  v_name = pc;

  pc = pVarElem->Attribute("file");
  if(pc == NULL) {return;}
  v_fileName = pc;

  m_sprites.push_back(new EffectSprite(this, v_name, v_fileName));
}

void Theme::newFontSpriteFromXML(TiXmlElement *pVarElem) {
  std::string v_name;
  std::string v_fileName;
  const char *pc;

  pc = pVarElem->Attribute("name");
  if(pc == NULL) {return;}
  v_name = pc;

  pc = pVarElem->Attribute("file");
  if(pc == NULL) {return;}
  v_fileName = pc;

  m_sprites.push_back(new FontSprite(this, v_name, v_fileName));
}

void Theme::newMiscSpriteFromXML(TiXmlElement *pVarElem) {
  std::string v_name;
  std::string v_fileName;
  const char *pc;

  pc = pVarElem->Attribute("name");
  if(pc == NULL) {return;}
  v_name = pc;

  pc = pVarElem->Attribute("file");
  if(pc == NULL) {return;}
  v_fileName = pc;

  m_sprites.push_back(new MiscSprite(this, v_name, v_fileName));
}

void Theme::newUISpriteFromXML(TiXmlElement *pVarElem) {
  std::string v_name;
  std::string v_fileName;
  const char *pc;

  pc = pVarElem->Attribute("name");
  if(pc == NULL) {return;}
  v_name = pc;

  pc = pVarElem->Attribute("file");
  if(pc == NULL) {return;}
  v_fileName = pc;

  m_sprites.push_back(new UISprite(this, v_name, v_fileName));
}

void Theme::newTextureSpriteFromXML(TiXmlElement *pVarElem) {
  std::string v_name;
  std::string v_fileName;
  const char *pc;

  pc = pVarElem->Attribute("name");
  if(pc == NULL) {return;}
  v_name = pc;

  pc = pVarElem->Attribute("file");
  if(pc == NULL) {return;}
  v_fileName = pc;

  m_sprites.push_back(new TextureSprite(this, v_name, v_fileName));
}

BikerTheme* Theme::getPlayerTheme() {
  return m_player;
}

#if defined(ALLOW_GHOST)
BikerTheme* Theme::getGhostTheme() {
  return m_ghost;
}
#endif

Sprite::Sprite(Theme* p_associated_theme, std::string v_name) {
  m_associated_theme = p_associated_theme;
  m_name = v_name;
}

Sprite::~Sprite() {
}

vapp::Texture* Sprite::getTexture(bool bSmall, bool bClamp, bool bFilter) {
  vapp::Texture* v_currentTexture;

  v_currentTexture = getCurrentTexture();
  if(v_currentTexture == NULL) {
    v_currentTexture = m_associated_theme->loadTexture(getCurrentTextureFileName(),
						       bSmall,
						       bClamp,
						       bFilter);
    if(v_currentTexture == NULL) { 
      throw vapp::Exception("Unable to load texture '" + getCurrentTextureFileName() + "'");
    }
    setCurrentTexture(v_currentTexture);
  }

  return v_currentTexture;
}

std::string Sprite::getName() {
  return m_name;
}

std::string Sprite::getFileDir() {
  return THEME_SPRITE_FILE_DIR;
}

AnimationSprite::AnimationSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileBase, std::string p_fileExtention) : Sprite(p_associated_theme, p_name) {
  m_current_frame = 0;
  m_fileBase      = p_fileBase;
  m_fileExtension = p_fileExtention;
  m_fFrameTime    = 0.0;
}

AnimationSprite::~AnimationSprite() {
  cleanFrames();
}

enum SpriteType AnimationSprite::getType() {
  return SPRITE_TYPE_ANIMATION;
}

std::string AnimationSprite::getFileDir() {
  return THEME_ANIMATION_SPRITE_FILE_DIR;
}

vapp::Texture* AnimationSprite::getCurrentTexture() {
  if(m_frames.size() == 0) {return NULL;}
  return m_frames[getCurrentFrame()]->getTexture();
}

std::string AnimationSprite::getCurrentTextureFileName() {
  char v_num[3];
  snprintf(v_num, 3, "%02i", getCurrentFrame() % 100); // max 100 frames by animation

  return getFileDir() + "/" + m_fileBase + std::string(v_num) + std::string(".") + m_fileExtension;
}

void AnimationSprite::setCurrentTexture(vapp::Texture *p_texture) {
  m_frames[getCurrentFrame()]->setTexture(p_texture);
}

int AnimationSprite::getCurrentFrame() {
  /* Next frame? */
  float v_realTime = vapp::App::getRealTime();

  while(v_realTime > m_fFrameTime + m_frames[m_current_frame]->getDelay()) {
    m_fFrameTime = v_realTime;
    m_current_frame++;
    if(m_current_frame == m_frames.size()) m_current_frame = 0;      
  }

  return m_current_frame;
}

float AnimationSprite::getCenterX() {
  return m_frames[getCurrentFrame()]->getCenterX();
}

float AnimationSprite::getCenterY() {
  return m_frames[getCurrentFrame()]->getCenterY();
}

float AnimationSprite::getWidth() {
  return m_frames[getCurrentFrame()]->getWidth();
}

float AnimationSprite::getHeight() {
  return m_frames[getCurrentFrame()]->getHeight();
}

void AnimationSprite::addFrame(float p_centerX, float p_centerY, float p_width, float p_height, float p_delay) {
  m_frames.push_back(new AnimationSpriteFrame(this, p_centerX, p_centerY, p_width, p_height, p_delay));
}

void AnimationSprite::cleanFrames() {
  for(int i=0; i<m_frames.size(); i++) {
    delete m_frames[i];
  }
  m_frames.clear();
}

AnimationSpriteFrame::AnimationSpriteFrame(AnimationSprite *p_associatedAnimationSprite,
					   float p_centerX,
					   float p_centerY,
					   float p_width,
					   float p_height,
					   float p_delay
    ) {
  m_associatedAnimationSprite = p_associatedAnimationSprite;
  m_texture = NULL;
  m_centerX = p_centerX;
  m_centerY = p_centerY;
  m_width   = p_width;
  m_height  = p_height;
  m_delay   = p_delay;
}

AnimationSpriteFrame::~AnimationSpriteFrame() {
}

vapp::Texture* AnimationSpriteFrame::getTexture() {
  return m_texture;
}

void  AnimationSpriteFrame::setTexture(vapp::Texture *p_texture) {
  m_texture = p_texture;
}

float AnimationSpriteFrame::getCenterX() const {
  return m_centerX;
}

float AnimationSpriteFrame::getCenterY() const {
  return m_centerY;
}

float AnimationSpriteFrame::getWidth() const {
  return m_width;
}

float AnimationSpriteFrame::getHeight() const {
  return m_height;
}

float AnimationSpriteFrame::getDelay() const {
  return m_delay;
}

BikerPartSprite::BikerPartSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName) : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
}

BikerPartSprite::~BikerPartSprite() {
}

std::string BikerPartSprite::getFileDir() {
  return THEME_BIKERPART_SPRITE_FILE_DIR;
}

enum SpriteType BikerPartSprite::getType() {
  return SPRITE_TYPE_BIKERPART;
}

DecorationSprite::DecorationSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName, float p_width, float p_height, float p_centerX, float p_centerY) : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
  m_width   = p_width;
  m_height  = p_height;
  m_centerX = p_centerX;
  m_centerY = p_centerY;
}

DecorationSprite::~DecorationSprite() {
}

enum SpriteType DecorationSprite::getType() {
  return SPRITE_TYPE_DECORATION;
}

std::string DecorationSprite::getFileDir() {
  return THEME_DECORATION_SPRITE_FILE_DIR;
}

float DecorationSprite::getWidth() {
  return m_width;
}

float DecorationSprite::getHeight() {
  return m_height;
}

float DecorationSprite::getCenterX() {
  return m_centerX;
}

float DecorationSprite::getCenterY() {
  return m_centerY;
}

EffectSprite::EffectSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName) : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
}

EffectSprite::~EffectSprite() {
}

enum SpriteType EffectSprite::getType() {
  return SPRITE_TYPE_EFFECT;
}

std::string EffectSprite::getFileDir() {
  return THEME_EFFECT_SPRITE_FILE_DIR;
}

FontSprite::FontSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName) : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
}

FontSprite::~FontSprite() {
}

enum SpriteType FontSprite::getType() {
  return SPRITE_TYPE_FONT;
}

std::string FontSprite::getFileDir() {
  return THEME_FONT_SPRITE_FILE_DIR;
}

MiscSprite::MiscSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName) : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
}

MiscSprite::~MiscSprite() {
}

enum SpriteType MiscSprite::getType() {
  return SPRITE_TYPE_MISC;
}

std::string MiscSprite::getFileDir() {
  return THEME_MISC_SPRITE_FILE_DIR;
}

UISprite::UISprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName) : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
}

UISprite::~UISprite() {
}

enum SpriteType UISprite::getType() {
  return SPRITE_TYPE_UI;
}

std::string UISprite::getFileDir() {
  return THEME_UI_SPRITE_FILE_DIR;
}

TextureSprite::TextureSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName) : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
}

TextureSprite::~TextureSprite() {
}

enum SpriteType TextureSprite::getType() {
  return SPRITE_TYPE_TEXTURE;
}

std::string TextureSprite::getFileDir() {
  return THEME_TEXTURE_SPRITE_FILE_DIR;
}

SimpleFrameSprite::SimpleFrameSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName) : Sprite(p_associated_theme, p_name) {
  m_fileName = p_fileName;
  m_texture  = NULL;
}

SimpleFrameSprite::~SimpleFrameSprite() {
}

vapp::Texture* SimpleFrameSprite::getCurrentTexture() {
  return m_texture;
}

std::string SimpleFrameSprite::getCurrentTextureFileName() {
  return getFileDir() + "/" + m_fileName;
}

void SimpleFrameSprite::setCurrentTexture(vapp::Texture *p_texture) {
  m_texture = p_texture;
}

BikerTheme::BikerTheme(Theme* p_associated_theme,
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
		       ) {
  m_associated_theme = p_associated_theme;
  m_Body     	     = p_Body;
  m_Front    	     = p_Front;
  m_Rear     	     = p_Rear;
  m_Wheel    	     = p_Wheel; 
  m_LowerArm 	     = p_LowerArm;
  m_LowerLeg 	     = p_LowerLeg;
  m_Torso    	     = p_Torso;
  m_UpperArm 	     = p_UpperArm;
  m_UpperLeg 	     = p_UpperLeg;

  m_UglyRiderColor = p_UglyRiderColor;
  m_UglyWheelColor = p_UglyWheelColor;
}
 
BikerTheme::~BikerTheme() {
}

Sprite* BikerTheme::getBody() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_Body);
}

Sprite* BikerTheme::getFront() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_Front);
}

Sprite* BikerTheme::getRear() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_Rear);
}

Sprite* BikerTheme::getWheel() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_Wheel);
}

Sprite* BikerTheme::getLowerArm() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_LowerArm);
}

Sprite* BikerTheme::getLowerLeg() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_LowerLeg);
}

Sprite* BikerTheme::getTorso() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_Torso);
}

Sprite* BikerTheme::getUpperArm() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_UpperArm);
}

Sprite* BikerTheme::getUpperLeg() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_UpperLeg);
}

vapp::Color BikerTheme::getUglyRiderColor() {
  return m_UglyRiderColor;
}

vapp::Color BikerTheme::getUglyWheelColor() {
  return m_UglyWheelColor;
}

bool ThemeChoicer::ExistThemeName(std::string p_themeName) {
  printf("ExistThemeName(%s)\n", p_themeName.c_str());
  for(int i=0; i<m_choices.size(); i++) {
    if(m_choices[i]->ThemeName() == p_themeName) {
      printf("Yes !\n");
      return true;
    }
  }

  printf("No !\n");
  return false;
}

ThemeChoicer::ThemeChoicer() {
  initList();
}

ThemeChoicer::~ThemeChoicer() {
  cleanList();
}

std::string ThemeChoicer::getFileName(std::string p_themeName) {
  printf("getFileName(%s)\n", p_themeName.c_str());
  for(int i=0; i<m_choices.size(); i++) {
    if(m_choices[i]->ThemeName() == p_themeName) {
      printf("get %s\n", m_choices[i]->ThemeFile().c_str());
      return m_choices[i]->ThemeFile();
    }
  }
  return "";
}

void ThemeChoicer::cleanList() {
  for(int i=0; i<m_choices.size(); i++) {
    delete m_choices[i];
  }
  m_choices.clear();
}

void ThemeChoicer::initList() {
  cleanList();

  std::vector<std::string> v_themesFiles = vapp::FS::findPhysFiles("Themes/*.xml", true);
  std::string v_name;

  for(int i=0; i<v_themesFiles.size(); i++) {
    try {
      v_name = getThemeNameFromFile(v_themesFiles[i]);
      m_choices.push_back(new ThemeChoice(v_name, v_themesFiles[i]));
      printf("Found %s\n", v_name.c_str());
    } catch(vapp::Exception &e) {
      /* anyway, give up this theme */
    }
  }
}

std::string ThemeChoicer::getThemeNameFromFile(std::string p_themeFile) {
  vapp::XMLDocument v_ThemeXml;
  TiXmlDocument *v_ThemeXmlData;
  TiXmlElement *v_ThemeXmlDataElement;
  const char *pc;
  std::string m_name;

  /* open the file */
  v_ThemeXml.readFromFile(p_themeFile);   
  v_ThemeXmlData = v_ThemeXml.getLowLevelAccess();
  
  if(v_ThemeXmlData == NULL) {
    throw vapp::Exception("error : unable analyse xml theme file");
  }
  
  /* read the theme name */
  v_ThemeXmlDataElement = v_ThemeXmlData->FirstChildElement("xmoto_theme");
  if(v_ThemeXmlDataElement != NULL) {
    pc = v_ThemeXmlDataElement->Attribute("name");
    m_name = pc;
  }
  
  if(m_name == "") {
    throw vapp::Exception("error : the theme has no name !");
  }
  
  return m_name;
}

ThemeChoice::ThemeChoice(std::string p_themeName, std::string p_themeFile) {
  m_themeName = p_themeName;
  m_themeFile = p_themeFile;
}

ThemeChoice::~ThemeChoice() {
}

std::string ThemeChoice::ThemeName() {
  return m_themeName;
}

std::string ThemeChoice::ThemeFile() {
  return m_themeFile;
}
