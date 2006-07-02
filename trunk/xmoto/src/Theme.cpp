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

class vapp::App;

Theme::Theme() {
}

Theme::~Theme() {
  cleanSprites();
}
 
vapp::Texture* Theme::loadTexture(std::string p_fileName) {
  return m_texMan.loadTexture(p_fileName.c_str());
}

void Theme::load() {
  load(getDefaultThemeFile());
}

void Theme::load(std::string p_themeFile) {
  vapp::XMLDocument v_ThemeXml;
  TiXmlDocument *v_ThemeXmlData;
  TiXmlElement *v_ThemeXmlDataElement;
  const char *pc;

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

std::string Theme::getDefaultThemeFile() {
  if(vapp::FS::isFileReadable("custom_theme.xml")) {
    return "custom_theme.xml";
  }

  if(vapp::FS::isFileReadable("original_theme.xml")) {
    return "original_theme.xml";
  }

  throw vapp::Exception("No avaible theme file !");
  return "";
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

Sprite::Sprite(Theme* p_associated_theme, std::string v_name) {
  m_associated_theme = p_associated_theme;
  m_name = v_name;
}

Sprite::~Sprite() {
}

vapp::Texture* Sprite::getTexture() {
  vapp::Texture* v_currentTexture;

  v_currentTexture = getCurrentTexture();
  if(v_currentTexture == NULL) {
    v_currentTexture = m_associated_theme->loadTexture(getCurrentTextureFileName());
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
  return SPRITE_FILE_DIR;
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
  return ANIMATION_SPRITE_FILE_DIR;
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
    m_current_frame;
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
  return DECORATION_SPRITE_FILE_DIR;
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
  return EFFECT_SPRITE_FILE_DIR;
}

FontSprite::FontSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName) : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
}

FontSprite::~FontSprite() {
}

enum SpriteType FontSprite::getType() {
  return SPRITE_TYPE_FONT;
}

MiscSprite::MiscSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName) : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
}

MiscSprite::~MiscSprite() {
}

enum SpriteType MiscSprite::getType() {
  return SPRITE_TYPE_MISC;
}

std::string MiscSprite::getFileDir() {
  return MISC_SPRITE_FILE_DIR;
}

TextureSprite::TextureSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName) : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
}

TextureSprite::~TextureSprite() {
}

enum SpriteType TextureSprite::getType() {
  return SPRITE_TYPE_TEXTURE;
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
