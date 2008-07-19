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

#include "Theme.h"
#include "helpers/VExcept.h"
#include "VXml.h"
#include "Game.h"
#include "md5sum/md5file.h"
#include "db/xmDatabase.h"
#include "helpers/Log.h"
#include "VFileIO.h"

std::vector<Sprite*> Theme::getSpritesList() {
  return m_sprites;
}

std::vector<ThemeSound*> Theme::getSoundsList() {
  return m_sounds;
}

Theme::Theme() {
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
}

Theme::~Theme() {
  delete m_player;
  delete m_ghost;

  cleanSprites();
  cleanMusics();
  cleanSounds();

  /* Kill textures */
  m_texMan.unloadTextures();
}
 
std::string Theme::Name() const {
  return m_name;
}

Texture* Theme::loadTexture(std::string p_fileName, bool bSmall, bool bClamp, FilterMode eFilterMode) {
  return m_texMan.loadTexture(p_fileName.c_str(), bSmall, bClamp, eFilterMode);
}

std::vector<ThemeFile>* Theme::getRequiredFiles() {
  return &m_requiredFiles;
}

void Theme::load(std::string p_themeFile) {
  LogInfo(std::string("Loading theme from file " + p_themeFile).c_str());

  m_requiredFiles.clear();

  cleanSprites(); /* removing existing sprites */
  cleanMusics();
  cleanSounds();

  XMLDocument v_ThemeXml;
  TiXmlDocument *v_ThemeXmlData;
  TiXmlElement *v_ThemeXmlDataElement;
  const char *pc;

  try {
    /* open the file */
    v_ThemeXml.readFromFile(p_themeFile);
    
    v_ThemeXmlData = v_ThemeXml.getLowLevelAccess();
    
    if(v_ThemeXmlData == NULL) {
      throw Exception("unable to analyze xml theme file");
    }
    
    /* read the theme name */
    v_ThemeXmlDataElement = v_ThemeXmlData->FirstChildElement("xmoto_theme");
    if(v_ThemeXmlDataElement != NULL) {
      pc = v_ThemeXmlDataElement->Attribute("name");
      m_name = pc;
    }
    
    if(m_name == "") {
      throw Exception("unnamed theme");
    }
    
    /* get sprites */
    loadSpritesFromXML(v_ThemeXmlDataElement);

  } catch(Exception &e) {
    throw Exception("unable to analyze xml theme file");
  }
}

bool Theme::isAFileOutOfDate(const std::string& i_file) {

  /* files that you can found in themes files for compatibility reason but no need to download */

  if(i_file == "Textures/UI/NewLevelsAvail.png") return true;
  if(i_file == "Textures/Effects/Sky1.jpg")      return true;
  if(i_file == "Textures/Effects/Sky2.jpg")      return true;
  if(i_file == "Textures/Effects/Sky2Drift.jpg") return true;
  if(i_file == "Textures/Fonts/MFont.png")       return true;
  if(i_file == "Textures/Fonts/SFont.png")       return true;
  if(i_file == "Textures/UI/Loading.png")        return true;

  return false;
}

void Theme::loadSpritesFromXML(TiXmlElement *p_ThemeXmlDataElement) {
  std::string v_spriteType;
  bool v_isAnimation;
  const char *pc;
  std::string v_sum;
  
  for(TiXmlElement *pVarElem = p_ThemeXmlDataElement->FirstChildElement("sprite");
      pVarElem!=NULL;
      pVarElem = pVarElem->NextSiblingElement("sprite")
      ) {
    pc = pVarElem->Attribute("type");
    if(pc == NULL) { continue; }
    v_spriteType = pc;

    /* this is not the nice method to allow animation,
       but initially, animation was an entire type,
       in the future i want to make it only a method of display
       so that all sprite type can be animated
    */
    pc = pVarElem->Attribute("fileBase");
    v_isAnimation = (pc != NULL);

    if(v_spriteType == "BikerPart") {
      newSpriteFromXML<BikerPartSprite>(pVarElem,
					THEME_BIKERPART_SPRITE_FILE_DIR,
					"BikerPart");
    }
    else if(v_spriteType == "Effect") {
      newSpriteFromXML<EffectSprite>(pVarElem,
				     THEME_EFFECT_SPRITE_FILE_DIR,
				     "Effect");
    }
    else if(v_spriteType == "Font") {
      newSpriteFromXML<FontSprite>(pVarElem,
				   THEME_FONT_SPRITE_FILE_DIR,
				   "Font");
    }
    else if(v_spriteType == "Misc") {
      newSpriteFromXML<MiscSprite>(pVarElem,
				   THEME_MISC_SPRITE_FILE_DIR,
				   "Misc");
    }
    else if(v_spriteType == "Texture") {
      newSpriteFromXML<TextureSprite>(pVarElem,
				      THEME_TEXTURE_SPRITE_FILE_DIR,
				      "Texture");
    }
    else if(v_spriteType == "UI") {
      newSpriteFromXML<UISprite>(pVarElem,
				 THEME_UI_SPRITE_FILE_DIR,
				 "UI");
    }

    else if(v_spriteType == "Entity" && v_isAnimation == true) {
      newAnimationSpriteFromXML(pVarElem);
    }
    else if(v_spriteType == "Entity" && v_isAnimation == false) {
      newDecorationSpriteFromXML(pVarElem);
    }

    else if(v_spriteType == "EdgeEffect") {
      newEdgeEffectSpriteFromXML(pVarElem);
    }

    else {
      LogInfo("Warning: unknown type '%s' in theme file !", v_spriteType.c_str());
    }
  }

  /* get musics */
  std::string v_musicName;
  std::string v_musicFile;
  v_sum = "";

  for(TiXmlElement *pVarElem = p_ThemeXmlDataElement->FirstChildElement("music");
      pVarElem!=NULL;
      pVarElem = pVarElem->NextSiblingElement("music")
      ) {

    pc = pVarElem->Attribute("name");
    if(pc == NULL) { continue; }
    v_musicName = pc;

    pc = pVarElem->Attribute("file");
    if(pc == NULL) { continue; }
    v_musicFile = pc;

    pc = pVarElem->Attribute("sum");
    if(pc != NULL) { v_sum = pc; };

    if(isAFileOutOfDate(THEME_MUSICS_FILE_DIR + std::string("/") + v_musicFile) == false) {
      m_musics.push_back(new ThemeMusic(this, v_musicName, v_musicFile));

      ThemeFile v_file = {THEME_MUSICS_FILE_DIR + std::string("/") + v_musicFile, v_sum};
      m_requiredFiles.push_back(v_file);
    }
  }

  /* get sounds */
  std::string v_soundName;
  std::string v_soundFile;
  v_sum = "";

  for(TiXmlElement *pVarElem = p_ThemeXmlDataElement->FirstChildElement("sound");
      pVarElem!=NULL;
      pVarElem = pVarElem->NextSiblingElement("sound")
      ) {

    pc = pVarElem->Attribute("name");
    if(pc == NULL) { continue; }
    v_soundName = pc;

    pc = pVarElem->Attribute("file");
    if(pc == NULL) { continue; }
    v_soundFile = pc;

    pc = pVarElem->Attribute("sum");
    if(pc != NULL) { v_sum = pc; };

    if(isAFileOutOfDate(THEME_SOUNDS_FILE_DIR + std::string("/") + v_soundFile) == false) {
      m_sounds.push_back(new ThemeSound(this, v_soundName, v_soundFile));
      ThemeFile v_file = {THEME_SOUNDS_FILE_DIR + std::string("/") + v_soundFile, v_sum};
      m_requiredFiles.push_back(v_file);
    }
  }

}

Sprite* Theme::getSprite(enum SpriteType pSpriteType, std::string pName) {
  for(unsigned int i=0; i<m_sprites.size(); i++) {
    if(m_sprites[i]->getType() == pSpriteType) {
      if(m_sprites[i]->getName() == pName) {
	return m_sprites[i];
      }
    }
  }

  return NULL;
}

ThemeMusic* Theme::getMusic(std::string i_name) {
  for(unsigned int i=0; i<m_musics.size(); i++) {
    if(m_musics[i]->Name() == i_name) {
      return m_musics[i];
    }
  }
  throw Exception("Music " + i_name + " not found");
}

ThemeSound* Theme::getSound(std::string i_name) {
  for(unsigned int i=0; i<m_sounds.size(); i++) {
    if(m_sounds[i]->Name() == i_name) {
      return m_sounds[i];
    }
  }
  throw Exception("Sound " + i_name + " not found");
}

void Theme::cleanSprites() {
  for(unsigned int i=0; i<m_sprites.size(); i++) {
    delete m_sprites[i];
  }
  m_sprites.clear();
}

void Theme::cleanMusics() {
   for(unsigned int i=0; i<m_musics.size(); i++) {
    delete m_musics[i];
  }
  m_musics.clear(); 
}

void Theme::cleanSounds() {
   for(unsigned int i=0; i<m_sounds.size(); i++) {
    delete m_sounds[i];
  }
  m_sounds.clear(); 
}


template <typename SpriteType>
void Theme::newSpriteFromXML(TiXmlElement *pVarElem,
			     const char*   fileDir,
			     const char*   spriteTypeName)
{
  std::string v_name;
  std::string v_fileName;
  const char *pc;
  std::string v_sum;

  pc = pVarElem->Attribute("name");
  if(pc == NULL) {
    LogInfo("** %s with no name", spriteTypeName);
    return;
  }
  v_name = pc;

  pc = pVarElem->Attribute("file");
  if(pc == NULL) {
    LogInfo("** %s with no file", spriteTypeName);
    return;
  }
  v_fileName = pc;

  pc = pVarElem->Attribute("sum");
  if(pc != NULL)
    v_sum = pc;

  if(isAFileOutOfDate(fileDir + std::string("/") + v_fileName) == false) {
    Sprite* pSprite = new SpriteType(this, v_name, v_fileName);
    pSprite->setOrder(m_sprites.size());

    m_sprites.push_back(pSprite);
    
    ThemeFile v_file = {fileDir + std::string("/") + v_fileName, v_sum};
    m_requiredFiles.push_back(v_file);
  }
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
  if(pc == NULL) {LogInfo("** Animation with no name"); return;}
  v_name = pc;
  
  pc = pVarElem->Attribute("fileBase");
  if(pc == NULL) {LogInfo("** Animation with no fileBase"); return;}
  v_fileBase = pc;

  pc = pVarElem->Attribute("fileExtension");
  if(pc == NULL) {LogInfo("** Animation with no fileExtension"); return;}
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
  v_anim->setOrder(m_sprites.size());
  m_sprites.push_back(v_anim);

  int n = 0;
  char buf[3];
  for(TiXmlElement *pVarSubElem = pVarElem->FirstChildElement("frame");
      pVarSubElem!=NULL;
      pVarSubElem = pVarSubElem->NextSiblingElement("frame")
      ) {
    float v_centerX;
    float v_centerY;
    float v_width;
    float v_height;
    float v_delay;
    std::string v_sum;

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

    pc = pVarSubElem->Attribute("sum");
    if(pc != NULL) { v_sum = pc; };

    if(n < 100) {
      sprintf(buf, "%02i", n);

      if(isAFileOutOfDate(THEME_ANIMATION_SPRITE_FILE_DIR + std::string("/") +
			  v_fileBase + std::string(buf) + std::string(".") + v_fileExtension) == false) {
	v_anim->addFrame(v_centerX, v_centerY, v_width, v_height, v_delay);

	ThemeFile v_file = {THEME_ANIMATION_SPRITE_FILE_DIR + std::string("/") +
			    v_fileBase + std::string(buf) + std::string(".") + v_fileExtension,
			    v_sum};
	m_requiredFiles.push_back(v_file);
      }
      n++;
    }
  }
}

void Theme::newDecorationSpriteFromXML(TiXmlElement *pVarElem)
{
  // create decoration sprites as an animation sprite with one frame
  std::string v_name;
  std::string v_fileName;
  std::string v_fileBase;
  std::string v_fileExtension;
  std::string v_blendmode;
  const char *pc;
  std::string v_sum = "";
  AnimationSprite *v_anim;
  float v_width;
  float v_height;
  float v_centerX;
  float v_centerY;
  float v_delay;

  float global_centerX = 0.5;
  float global_centerY = 0.5;
  float global_width   = 1.0;
  float global_height  = 1.0;
  float global_delay   = 0.1;

  pc = pVarElem->Attribute("name");
  if(pc == NULL) {
    LogInfo("** Sprite with no name");
    return;
  }
  v_name = pc;
  
  pc = pVarElem->Attribute("file");
  if(pc == NULL) {
    LogInfo("** Sprite with no name");
    return;
  }
  v_fileName = pc;

  // split fileName in fileBase and fileExtension to mimic an animation sprite
  unsigned int dotPos = v_fileName.find_last_of('.');
  v_fileBase = v_fileName.substr(0, dotPos);
  v_fileExtension = v_fileName.substr(dotPos+1);

  pc = pVarElem->Attribute("width");
  if(pc != NULL)
    v_width = atof(pc);
  else
    v_width = global_width;

  pc = pVarElem->Attribute("height");
  if(pc != NULL)
    v_height = atof(pc);
  else
    v_height = global_height;

  pc = pVarElem->Attribute("centerX");
  if(pc != NULL)
    v_centerX = atof(pc);
  else
    v_centerX = global_centerX;

  pc = pVarElem->Attribute("centerY");
  if(pc != NULL)
    v_centerY = atof(pc);
  else
    v_centerY = global_centerY;

  pc = pVarElem->Attribute("blendmode");
  if(pc != NULL)
    v_blendmode = pc;
  else
    v_blendmode = "default";

  pc = pVarElem->Attribute("sum");
  if(pc != NULL)
    v_sum = pc;

  v_delay = global_delay;

  v_anim = new AnimationSprite(this, v_name, v_fileBase, v_fileExtension);
  v_anim->setBlendMode(strToBlendMode(v_blendmode));
  v_anim->setOrder(m_sprites.size());

  m_sprites.push_back(v_anim);

  if(isAFileOutOfDate(THEME_ANIMATION_SPRITE_FILE_DIR + std::string("/") +
		      v_fileBase + std::string(".") + v_fileExtension) == false) {
    v_anim->addFrame(v_centerX, v_centerY, v_width, v_height, v_delay);

    ThemeFile v_file = {THEME_ANIMATION_SPRITE_FILE_DIR + std::string("/") +
			v_fileBase + std::string(".") + v_fileExtension, v_sum};
    m_requiredFiles.push_back(v_file);
  }
}

void Theme::newEdgeEffectSpriteFromXML(TiXmlElement *pVarElem) {
  std::string v_name;
  std::string v_fileName;
  std::string v_scale;
  std::string v_depth;
  const char *pc;
  std::string v_sum;

  pc = pVarElem->Attribute("name");
  if(pc == NULL) {LogInfo("** Edge with no name"); return;}
  v_name = pc;

  pc = pVarElem->Attribute("file");
  if(pc == NULL) {LogInfo("** Edge with no file"); return;}
  v_fileName = pc;

  pc = pVarElem->Attribute("scale");
  if(pc == NULL) {LogInfo("** Edge with no scale"); return;}
  v_scale = pc;

  pc = pVarElem->Attribute("depth");
  if(pc == NULL) {LogInfo("Edge with no depth"); return;}
  v_depth = pc;

  pc = pVarElem->Attribute("sum");
  if(pc != NULL) { v_sum = pc; };

  if(isAFileOutOfDate(THEME_EDGEEFFECT_SPRITE_FILE_DIR + std::string("/") + v_fileName) == false) {
    Sprite* pSprite = new EdgeEffectSprite(this, v_name, v_fileName,
					   atof(v_scale.c_str()),
					   atof(v_depth.c_str()));
    pSprite->setOrder(m_sprites.size());

    m_sprites.push_back(pSprite);

    ThemeFile v_file = {THEME_EDGEEFFECT_SPRITE_FILE_DIR + std::string("/") + v_fileName, v_sum};
    m_requiredFiles.push_back(v_file);
  }
}



BikerTheme* Theme::getPlayerTheme() {
  return m_player;
}

BikerTheme* Theme::getGhostTheme() {
  return m_ghost;
}

Sprite::Sprite(Theme* p_associated_theme, std::string v_name) {
  m_associated_theme = p_associated_theme;
  m_name = v_name;
  m_blendmode = SPRITE_BLENDMODE_DEFAULT;
}

Sprite::~Sprite() {
}

Texture* Sprite::getTexture(bool bSmall, bool bClamp, FilterMode eFilterMode) {
  Texture* v_currentTexture;

  v_currentTexture = getCurrentTexture();
  if(v_currentTexture == NULL) {
    v_currentTexture = m_associated_theme->loadTexture(getCurrentTextureFileName(),
						       bSmall,
						       bClamp,
						       eFilterMode);
    setCurrentTexture(v_currentTexture);
  }
  
  return v_currentTexture;
}

void Sprite::setOrder(unsigned int order)
{
  m_order = order;
}

SpriteBlendMode Sprite::getBlendMode() {
  return m_blendmode;
}

void Sprite::setBlendMode(SpriteBlendMode Mode) {
  m_blendmode = Mode;
}

std::string Sprite::getFileDir() {
  return THEME_SPRITE_FILE_DIR;
}

AnimationSprite::AnimationSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileBase, std::string p_fileExtention) : Sprite(p_associated_theme, p_name) {
  m_current_frame = 0;
  m_fileBase      = p_fileBase;
  m_fileExtension = p_fileExtention;
  m_fFrameTime    = 0.0;
  m_type          = SPRITE_TYPE_ANIMATION;
  m_animation     = false;
}

AnimationSprite::~AnimationSprite() {
  cleanFrames();
}

std::string AnimationSprite::getFileDir() {
  return THEME_ANIMATION_SPRITE_FILE_DIR;
}

Texture* AnimationSprite::getCurrentTexture() {
  if(m_frames.size() == 0) {
    return NULL;
  }
  
  return m_frames[getCurrentFrame()]->getTexture();
}

std::string AnimationSprite::getCurrentTextureFileName() {
  if(m_animation == false){
    return getFileDir() + "/" + m_fileBase + std::string(".") + m_fileExtension;
  }else{
    char v_num[3];
    snprintf(v_num, 3, "%02i", getCurrentFrame() % 100); // max 100 frames by animation

    return getFileDir() + "/" + m_fileBase + std::string(v_num) + std::string(".") + m_fileExtension;
  }
}

void AnimationSprite::setCurrentTexture(Texture *p_texture) {
  m_frames[getCurrentFrame()]->setTexture(p_texture);
}

int AnimationSprite::getCurrentFrame() {
  if(m_animation == false){
    return 0;
  }else{
    /* Next frame? */
    float v_realTime = GameApp::getXMTime();

    while(v_realTime > m_fFrameTime + m_frames[m_current_frame]->getDelay()) {
      m_fFrameTime = v_realTime;
      m_current_frame++;
      if(m_current_frame == m_frames.size())
	m_current_frame = 0;      
    }

    return m_current_frame;
  }
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
  if(m_frames.size() > 1)
    m_animation = true;
}

void AnimationSprite::cleanFrames() {
  for(unsigned int i=0; i<m_frames.size(); i++) {
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

Texture* AnimationSpriteFrame::getTexture() {
  return m_texture;
}

void  AnimationSpriteFrame::setTexture(Texture *p_texture) {
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
  m_type = SPRITE_TYPE_BIKERPART;
}

BikerPartSprite::~BikerPartSprite() {
}

std::string BikerPartSprite::getFileDir() {
  return THEME_BIKERPART_SPRITE_FILE_DIR;
}

EffectSprite::EffectSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName) : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
  m_type = SPRITE_TYPE_EFFECT;
}

EffectSprite::~EffectSprite() {
}

std::string EffectSprite::getFileDir() {
  return THEME_EFFECT_SPRITE_FILE_DIR;
}

EdgeEffectSprite::EdgeEffectSprite(Theme* p_associated_theme, std::string p_name, std::string p_filename, float p_fScale, float p_fDepth) : SimpleFrameSprite(p_associated_theme, p_name, p_filename) {
  m_fScale = p_fScale;
  m_fDepth = p_fDepth;
  m_type   = SPRITE_TYPE_EDGEEFFECT;
}

EdgeEffectSprite::~EdgeEffectSprite() {
}

std::string EdgeEffectSprite::getFileDir() {
  return THEME_EDGEEFFECT_SPRITE_FILE_DIR;
}

float EdgeEffectSprite::getScale() const {
  return m_fScale;
}

float EdgeEffectSprite::getDepth() const {
  return m_fDepth;
}

Texture* EdgeEffectSprite::getTexture(bool bSmall, bool bClamp, FilterMode eFilterMode) {
  return Sprite::getTexture(bSmall, bClamp, eFilterMode);
}

FontSprite::FontSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName) : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
  m_type = SPRITE_TYPE_FONT;
}

FontSprite::~FontSprite() {
}

std::string FontSprite::getFileDir() {
  return THEME_FONT_SPRITE_FILE_DIR;
}

MiscSprite::MiscSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName) : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
  m_type = SPRITE_TYPE_MISC;
}

MiscSprite::~MiscSprite() {
}

std::string MiscSprite::getFileDir() {
  return THEME_MISC_SPRITE_FILE_DIR;
}

UISprite::UISprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName) : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
  m_type = SPRITE_TYPE_UI;
}

UISprite::~UISprite() {
}

std::string UISprite::getFileDir() {
  return THEME_UI_SPRITE_FILE_DIR;
}

TextureSprite::TextureSprite(Theme* p_associated_theme, std::string p_name, std::string p_fileName) : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
  m_type = SPRITE_TYPE_TEXTURE;
}

TextureSprite::~TextureSprite() {
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

Texture* SimpleFrameSprite::getCurrentTexture() {
  return m_texture;
}

std::string SimpleFrameSprite::getCurrentTextureFileName() {
  return getFileDir() + "/" + m_fileName;
}

void SimpleFrameSprite::setCurrentTexture(Texture *p_texture) {
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
           Color p_UglyRiderColor,
           Color p_UglyWheelColor
           ) {
  m_associated_theme = p_associated_theme;
  m_Body           = p_Body;
  m_Front          = p_Front;
  m_Rear           = p_Rear;
  m_Wheel          = p_Wheel; 
  m_LowerArm       = p_LowerArm;
  m_LowerLeg       = p_LowerLeg;
  m_Torso          = p_Torso;
  m_UpperArm       = p_UpperArm;
  m_UpperLeg       = p_UpperLeg;

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

Color BikerTheme::getUglyRiderColor() {
  return m_UglyRiderColor;
}

Color BikerTheme::getUglyWheelColor() {
  return m_UglyWheelColor;
}

void ThemeChoicer::initThemesFromDir(xmDatabase *i_db) {
  std::vector<std::string> v_themesFiles = FS::findPhysFiles(std::string(THEMES_DIRECTORY)
								   + std::string("/*.xml"), true);
  std::string v_name;

  i_db->themes_add_begin();
  for(unsigned int i=0; i<v_themesFiles.size(); i++) {
    try {
      v_name = getThemeNameFromFile(v_themesFiles[i]);
      if(i_db->themes_exists(v_name) == false) {
	i_db->themes_add(v_name, v_themesFiles[i]);
      } else {
	LogInfo(std::string("Theme " + v_name + " is present several times").c_str());
      }
    } catch(Exception &e) {
      /* anyway, give up this theme */
    }
  }
  i_db->themes_add_end();
}

std::string ThemeChoicer::getThemeNameFromFile(std::string p_themeFile) {
  XMLDocument v_ThemeXml;
  TiXmlDocument *v_ThemeXmlData;
  TiXmlElement *v_ThemeXmlDataElement;
  const char *pc;
  std::string m_name;

  /* open the file */
  v_ThemeXml.readFromFile(p_themeFile);   
  v_ThemeXmlData = v_ThemeXml.getLowLevelAccess();
  
  if(v_ThemeXmlData == NULL) {
    throw Exception("error : unable analyse xml theme file");
  }
  
  /* read the theme name */
  v_ThemeXmlDataElement = v_ThemeXmlData->FirstChildElement("xmoto_theme");
  if(v_ThemeXmlDataElement != NULL) {
    pc = v_ThemeXmlDataElement->Attribute("name");
    m_name = pc;
  }
  
  if(m_name == "") {
    throw Exception("error : the theme has no name !");
  }
  
  return m_name;
}

SpriteBlendMode Theme::strToBlendMode(const std::string &s) {
  if(s == "add") return SPRITE_BLENDMODE_ADDITIVE;
  return SPRITE_BLENDMODE_DEFAULT;
}

std::string Theme::blendModeToStr(SpriteBlendMode Mode) {
  if(Mode == SPRITE_BLENDMODE_ADDITIVE) return "add";
  return "default";
}

ThemeMusic::ThemeMusic(Theme* p_associated_theme, std::string i_name, std::string i_fileName) {
  m_associated_theme = p_associated_theme;
  m_name     	      = i_name;
  m_fileName 	      = i_fileName;
}
 
ThemeMusic::~ThemeMusic() {
}

std::string ThemeMusic::Name() const {
  return m_name;
}

std::string ThemeMusic::FileName() const {
  return m_fileName;
}
 
std::string ThemeMusic::FilePath() const {
  return FS::FullPath(THEME_MUSICS_FILE_DIR + std::string("/") + m_fileName);
}

ThemeSound::ThemeSound(Theme* p_associated_theme, std::string i_name, std::string i_fileName) {
  m_associated_theme = p_associated_theme;
  m_name     	      = i_name;
  m_fileName 	      = i_fileName;
}
 
ThemeSound::~ThemeSound() {
}

std::string ThemeSound::Name() const {
  return m_name;
}

std::string ThemeSound::FileName() const {
  return m_fileName;
}
 
std::string ThemeSound::FilePath() const {
  return THEME_SOUNDS_FILE_DIR + std::string("/") + m_fileName;
}
