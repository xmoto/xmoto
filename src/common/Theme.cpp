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
#include "VFileIO.h"
#include "VXml.h"
#include "db/xmDatabase.h"
#include "helpers/Log.h"
#include "helpers/VExcept.h"
#include "md5sum/md5file.h"
#include "xmoto/Game.h"

std::vector<Sprite *> &Theme::getSpritesList() {
  return m_sprites;
}

std::vector<ThemeSound *> &Theme::getSoundsList() {
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
                            THEME_PLAYER_UGLYWHEELCOLOR,
                            false,
                            THEME_PLAYER_GRAPHICS_LOW_BIKER,
                            THEME_PLAYER_GRAPHICS_LOW_FILL,
                            THEME_PLAYER_GRAPHICS_LOW_WHEEL);

  m_netplayer = new BikerTheme(this,
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
                               THEME_PLAYER_UGLYWHEELCOLOR,
                               false,
                               THEME_PLAYER_GRAPHICS_LOW_BIKER,
                               THEME_PLAYER_GRAPHICS_LOW_FILL,
                               THEME_PLAYER_GRAPHICS_LOW_WHEEL);

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
                           THEME_GHOST_UGLYWHEELCOLOR,
                           true,
                           THEME_GHOST_GRAPHICS_LOW_BIKER,
                           THEME_GHOST_GRAPHICS_LOW_FILL,
                           THEME_GHOST_GRAPHICS_LOW_WHEEL);
}

Theme::~Theme() {
  delete m_player;
  delete m_netplayer;
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

Texture *Theme::loadTexture(std::string p_fileName,
                            bool bSmall,
                            WrapMode wrapMode,
                            FilterMode eFilterMode,
                            bool persistent,
                            Sprite *associateSprite) {
  return m_texMan.loadTexture(p_fileName.c_str(),
                              bSmall,
                              wrapMode,
                              eFilterMode,
                              persistent,
                              associateSprite);
}

int Theme::getTextureSize(std::string p_fileName) {
  return m_texMan.getTextureSize(p_fileName);
}

std::vector<ThemeFile> *Theme::getRequiredFiles() {
  return &m_requiredFiles;
}

void Theme::load(FileDataType i_fdt, std::string p_themeFile) {
  LogInfo(std::string("Loading theme from file " + p_themeFile).c_str());

  m_requiredFiles.clear();

  m_texMan.removeAssociatedSpritesFromTextures();
  cleanSprites();
  cleanMusics();
  cleanSounds();

  XMLDocument v_xml;
  xmlNodePtr v_xmlElt;

  try {
    /* open the file */
    v_xml.readFromFile(i_fdt, p_themeFile);

    v_xmlElt = v_xml.getRootNode("xmoto_theme");
    if (v_xmlElt == NULL) {
      throw Exception("unable to analyze xml file");
    }

    m_name = XMLDocument::getOption(v_xmlElt, "name");

    if (m_name == "") {
      throw Exception("unnamed theme");
    }

    /* get sprites */
    loadSpritesFromXML(v_xmlElt);
  } catch (Exception &e) {
    throw Exception("unable to analyze xml theme file");
  }
}

bool Theme::isAFileOutOfDate(const std::string &i_file) {
  /* files that you can found in themes files for compatibility reason but no
   * need to download */

  if (i_file == "Textures/UI/NewLevelsAvail.png")
    return true;
  if (i_file == "Textures/Effects/Sky1.jpg")
    return true;
  if (i_file == "Textures/Effects/Sky2.jpg")
    return true;
  if (i_file == "Textures/Effects/Sky2Drift.jpg")
    return true;
  if (i_file == "Textures/Fonts/MFont.png")
    return true;
  if (i_file == "Textures/Fonts/SFont.png")
    return true;
  if (i_file == "Textures/UI/Loading.png")
    return true;

  return false;
}

void Theme::loadSpritesFromXML(xmlNodePtr pElem) {
  std::string v_spriteType;
  bool v_isAnimation;
  bool v_enableAnimation;
  std::string v_sum;

  v_enableAnimation = XMSession::instance()->disableAnimations();

  for (xmlNodePtr pSubElem = XMLDocument::subElement(pElem, "sprite");
       pSubElem != NULL;
       pSubElem = XMLDocument::nextElement(pSubElem)) {
    v_spriteType = XMLDocument::getOption(pSubElem, "type");
    if (v_spriteType == "") {
      continue;
    }

    /* this is not the nice method to allow animation,
       but initially, animation was an entire type,
       in the future i want to make it only a method of display
       so that all sprite type can be animated
    */
    v_isAnimation = XMLDocument::getOption(pSubElem, "fileBase") != "";

    if (v_spriteType == "BikerPart") {
      newSpriteFromXML<BikerPartSprite>(
        pSubElem, THEME_BIKERPART_SPRITE_FILE_DIR, "BikerPart");
    } else if (v_spriteType == "Effect") {
      newSpriteFromXML<EffectSprite>(
        pSubElem, THEME_EFFECT_SPRITE_FILE_DIR, "Effect");
    } else if (v_spriteType == "Font") {
      newSpriteFromXML<FontSprite>(
        pSubElem, THEME_FONT_SPRITE_FILE_DIR, "Font");
    } else if (v_spriteType == "Misc") {
      newSpriteFromXML<MiscSprite>(
        pSubElem, THEME_MISC_SPRITE_FILE_DIR, "Misc");
    } else if (v_spriteType == "Texture" && v_isAnimation == false) {
      newSpriteFromXML<TextureSprite>(
        pSubElem, THEME_TEXTURE_SPRITE_FILE_DIR, "Texture");
    } else if (v_spriteType == "Texture" && v_isAnimation == true &&
               v_enableAnimation == true) {
      newSpriteFromXML<TextureSprite>(
        pSubElem, THEME_TEXTURE_SPRITE_FILE_DIR, "Texture");
    } else if (v_spriteType == "Texture" && v_isAnimation == true &&
               v_enableAnimation == false) {
      newAnimationSpriteFromXML(pSubElem, true, THEME_TEXTURE_SPRITE_FILE_DIR);
    } else if (v_spriteType == "UI") {
      newSpriteFromXML<UISprite>(pSubElem, THEME_UI_SPRITE_FILE_DIR, "UI");
    }

    else if (v_spriteType == "Entity" && v_isAnimation == true) {
      newAnimationSpriteFromXML(
        pSubElem, false, THEME_ANIMATION_SPRITE_FILE_DIR);
    } else if (v_spriteType == "Entity" && v_isAnimation == false) {
      newDecorationSpriteFromXML(pSubElem);
    }

    else if (v_spriteType == "EdgeEffect") {
      newEdgeEffectSpriteFromXML(pSubElem);
    }

    else {
      LogWarning("unknown type '%s' in theme file !", v_spriteType.c_str());
    }
  }

  /* get musics */
  std::string v_musicName;
  std::string v_musicFile;
  v_sum = "";

  for (xmlNodePtr pSubElem = XMLDocument::subElement(pElem, "music");
       pSubElem != NULL;
       pSubElem = XMLDocument::nextElement(pSubElem)) {
    v_musicName = XMLDocument::getOption(pSubElem, "name");
    if (v_musicName == "") {
      continue;
    }

    v_musicFile = XMLDocument::getOption(pSubElem, "file");
    if (v_musicFile == "") {
      continue;
    }

    v_sum = XMLDocument::getOption(pSubElem, "sum");

    if (isAFileOutOfDate(THEME_MUSICS_FILE_DIR + std::string("/") +
                         v_musicFile) == false) {
      m_musics.push_back(new ThemeMusic(this, v_musicName, v_musicFile));

      ThemeFile v_file = {
        THEME_MUSICS_FILE_DIR + std::string("/") + v_musicFile, v_sum
      };
      m_requiredFiles.push_back(v_file);
    }
  }

  /* get sounds */
  std::string v_soundName;
  std::string v_soundFile;
  v_sum = "";

  for (xmlNodePtr pSubElem = XMLDocument::subElement(pElem, "sound");
       pSubElem != NULL;
       pSubElem = XMLDocument::nextElement(pSubElem)) {
    v_soundName = XMLDocument::getOption(pSubElem, "name");
    if (v_soundName == "") {
      continue;
    }

    v_soundFile = XMLDocument::getOption(pSubElem, "file");
    if (v_soundFile == "") {
      continue;
    }

    v_sum = XMLDocument::getOption(pSubElem, "sum");

    if (isAFileOutOfDate(THEME_SOUNDS_FILE_DIR + std::string("/") +
                         v_soundFile) == false) {
      m_sounds.push_back(new ThemeSound(this, v_soundName, v_soundFile));
      ThemeFile v_file = {
        THEME_SOUNDS_FILE_DIR + std::string("/") + v_soundFile, v_sum
      };
      m_requiredFiles.push_back(v_file);
    }
  }
}

Sprite *Theme::getSprite(enum SpriteType pSpriteType, std::string pName) {
  for (unsigned int i = 0; i < m_sprites.size(); i++) {
    if (m_sprites[i]->getType() == pSpriteType) {
      if (m_sprites[i]->getName() == pName) {
        return m_sprites[i];
      }
    }
  }

  return NULL;
}

std::string Theme::getHashMusic(const std::string &i_key) {
  unsigned int hash = 0;
  for (unsigned int i = 0; i < i_key.size(); i++) {
    hash += (unsigned int)(i_key[i]);
  }
  return m_musics[hash % m_musics.size()]->Name();
}

ThemeMusic *Theme::getMusic(std::string i_name) {
  for (unsigned int i = 0; i < m_musics.size(); i++) {
    if (m_musics[i]->Name() == i_name) {
      return m_musics[i];
    }
  }
  throw Exception("Music " + i_name + " not found");
}

ThemeSound *Theme::getSound(std::string i_name) {
  for (unsigned int i = 0; i < m_sounds.size(); i++) {
    if (m_sounds[i]->Name() == i_name) {
      return m_sounds[i];
    }
  }
  throw Exception("Sound " + i_name + " not found");
}

void Theme::cleanSprites() {
  for (unsigned int i = 0; i < m_sprites.size(); i++) {
    delete m_sprites[i];
  }
  m_sprites.clear();
}

void Theme::cleanMusics() {
  for (unsigned int i = 0; i < m_musics.size(); i++) {
    delete m_musics[i];
  }
  m_musics.clear();
}

void Theme::cleanSounds() {
  for (unsigned int i = 0; i < m_sounds.size(); i++) {
    delete m_sounds[i];
  }
  m_sounds.clear();
}

template<typename SpriteType>
void Theme::newSpriteFromXML(xmlNodePtr pElem,
                             const char *fileDir,
                             const char *spriteTypeName) {
  std::string v_name;
  std::string v_fileName;
  std::string v_sum;

  v_name = XMLDocument::getOption(pElem, "name");
  if (v_name == "") {
    LogWarning("%s with no name", spriteTypeName);
    return;
  }

  v_fileName = XMLDocument::getOption(pElem, "file");
  if (v_fileName == "") {
    LogWarning("%s with no file", spriteTypeName);
    return;
  }

  v_sum = XMLDocument::getOption(pElem, "sum");

  if (isAFileOutOfDate(fileDir + std::string("/") + v_fileName) == false) {
    Sprite *pSprite = new SpriteType(this, v_name, v_fileName);
    pSprite->setOrder(m_sprites.size());

    m_sprites.push_back(pSprite);

    ThemeFile v_file = { fileDir + std::string("/") + v_fileName, v_sum };
    m_requiredFiles.push_back(v_file);
  }
}

void Theme::newAnimationSpriteFromXML(xmlNodePtr pElem,
                                      bool v_isTexture,
                                      const char *fileDir) {
  std::string v_name;
  std::string v_fileBase;
  std::string v_fileExtension;
  std::string v_tmp;
  AnimationSprite *v_anim;

  float global_centerX = 0.5;
  float global_centerY = 0.5;
  float global_width = 1.0;
  float global_height = 1.0;
  float global_delay = 0.1;

  v_name = XMLDocument::getOption(pElem, "name");
  if (v_name == "") {
    LogWarning("Animation with no name");
    return;
  }

  v_fileBase = XMLDocument::getOption(pElem, "fileBase");
  if (v_fileBase == "") {
    LogWarning("Animation with no fileBase");
    return;
  }

  v_fileExtension = XMLDocument::getOption(pElem, "fileExtension");
  if (v_fileExtension == "") {
    LogWarning("Animation with no fileExtension");
    return;
  }

  v_tmp = XMLDocument::getOption(pElem, "centerX");
  if (v_tmp != "") {
    global_centerX = atof(v_tmp.c_str());
  }

  v_tmp = XMLDocument::getOption(pElem, "centerY");
  if (v_tmp != "") {
    global_centerY = atof(v_tmp.c_str());
  }

  v_tmp = XMLDocument::getOption(pElem, "width");
  if (v_tmp != "") {
    global_width = atof(v_tmp.c_str());
  }

  v_tmp = XMLDocument::getOption(pElem, "height");
  if (v_tmp != "") {
    global_height = atof(v_tmp.c_str());
  }

  v_tmp = XMLDocument::getOption(pElem, "delay");
  if (v_tmp != "") {
    global_delay = atof(v_tmp.c_str());
  }

  v_anim =
    new AnimationSprite(this, v_name, v_fileBase, v_fileExtension, v_isTexture);
  v_anim->setOrder(m_sprites.size());
  m_sprites.push_back(v_anim);

  unsigned int n = 0;
  char buf[3];

  for (xmlNodePtr pSubElem = XMLDocument::subElement(pElem, "frame");
       pSubElem != NULL;
       pSubElem = XMLDocument::nextElement(pSubElem)) {
    float v_centerX;
    float v_centerY;
    float v_width;
    float v_height;
    float v_delay;
    std::string v_sum;

    v_tmp = XMLDocument::getOption(pSubElem, "centerX");
    if (v_tmp != "") {
      v_centerX = atof(v_tmp.c_str());
    } else {
      v_centerX = global_centerX;
    }

    v_tmp = XMLDocument::getOption(pSubElem, "centerY");
    if (v_tmp != "") {
      v_centerY = atof(v_tmp.c_str());
    } else {
      v_centerY = global_centerY;
    }

    v_tmp = XMLDocument::getOption(pSubElem, "width");
    if (v_tmp != "") {
      v_width = atof(v_tmp.c_str());
    } else {
      v_width = global_width;
    }

    v_tmp = XMLDocument::getOption(pSubElem, "height");
    if (v_tmp != "") {
      v_height = atof(v_tmp.c_str());
    } else {
      v_height = global_height;
    }

    v_tmp = XMLDocument::getOption(pSubElem, "delay");
    if (v_tmp != "") {
      v_delay = atof(v_tmp.c_str());
    } else {
      v_delay = global_delay;
    }

    v_sum = XMLDocument::getOption(pSubElem, "sum");

    if (n < 100) {
      snprintf(buf, 3, "%02u", n);

      if (isAFileOutOfDate(fileDir + std::string("/") + v_fileBase +
                           std::string(buf) + std::string(".") +
                           v_fileExtension) == false) {
        v_anim->addFrame(v_centerX, v_centerY, v_width, v_height, v_delay);

        std::string fileName = fileDir + std::string("/") + v_fileBase +
                               std::string(buf) + std::string(".") +
                               v_fileExtension;

        ThemeFile v_file = { fileDir + std::string("/") + v_fileBase +
                               std::string(buf) + std::string(".") +
                               v_fileExtension,
                             v_sum };
        m_requiredFiles.push_back(v_file);
      }
      n++;
    }
  }
}

void Theme::newDecorationSpriteFromXML(xmlNodePtr pElem) {
  // create decoration sprites as an animation sprite with one frame
  std::string v_name;
  std::string v_fileName;
  std::string v_fileBase;
  std::string v_fileExtension;
  std::string v_blendmode;
  std::string v_sum = "";
  std::string v_tmp;
  AnimationSprite *v_anim;
  float v_width;
  float v_height;
  float v_centerX;
  float v_centerY;
  float v_delay;

  float global_centerX = 0.5;
  float global_centerY = 0.5;
  float global_width = 1.0;
  float global_height = 1.0;
  float global_delay = 0.1;

  v_name = XMLDocument::getOption(pElem, "name");
  if (v_name == "") {
    LogWarning("Sprite with no name");
    return;
  }

  v_fileName = XMLDocument::getOption(pElem, "file");
  if (v_fileName == "") {
    LogWarning("Sprite with no name");
    return;
  }

  // split fileName in fileBase and fileExtension to mimic an animation sprite
  unsigned int dotPos = v_fileName.find_last_of('.');
  v_fileBase = v_fileName.substr(0, dotPos);
  v_fileExtension = v_fileName.substr(dotPos + 1);

  v_tmp = XMLDocument::getOption(pElem, "width");
  if (v_tmp != "")
    v_width = atof(v_tmp.c_str());
  else
    v_width = global_width;

  v_tmp = XMLDocument::getOption(pElem, "height");
  if (v_tmp != "")
    v_height = atof(v_tmp.c_str());
  else
    v_height = global_height;

  v_tmp = XMLDocument::getOption(pElem, "centerX");
  if (v_tmp != "")
    v_centerX = atof(v_tmp.c_str());
  else
    v_centerX = global_centerX;

  v_tmp = XMLDocument::getOption(pElem, "centerY");
  if (v_tmp != "")
    v_centerY = atof(v_tmp.c_str());
  else
    v_centerY = global_centerY;

  v_blendmode = XMLDocument::getOption(pElem, "blendmode");
  if (v_blendmode == "")
    v_blendmode = "default";

  v_sum = XMLDocument::getOption(pElem, "sum");

  v_delay = global_delay;

  v_anim =
    new AnimationSprite(this, v_name, v_fileBase, v_fileExtension, false);
  v_anim->setBlendMode(strToBlendMode(v_blendmode));
  v_anim->setOrder(m_sprites.size());

  m_sprites.push_back(v_anim);

  if (isAFileOutOfDate(THEME_ANIMATION_SPRITE_FILE_DIR + std::string("/") +
                       v_fileBase + std::string(".") + v_fileExtension) ==
      false) {
    v_anim->addFrame(v_centerX, v_centerY, v_width, v_height, v_delay);

    ThemeFile v_file = { THEME_ANIMATION_SPRITE_FILE_DIR + std::string("/") +
                           v_fileBase + std::string(".") + v_fileExtension,
                         v_sum };
    m_requiredFiles.push_back(v_file);
  }
}

void Theme::newEdgeEffectSpriteFromXML(xmlNodePtr pElem) {
  std::string v_name;
  std::string v_fileName;
  std::string v_scale;
  std::string v_depth;
  std::string v_sum;

  v_name = XMLDocument::getOption(pElem, "name");
  if (v_name == "") {
    LogWarning("Edge with no name");
    return;
  }

  v_fileName = XMLDocument::getOption(pElem, "file");
  if (v_fileName == "") {
    LogWarning("Edge with no file");
    return;
  }

  v_scale = XMLDocument::getOption(pElem, "scale");
  if (v_scale == "") {
    LogWarning("Edge with no scale");
    return;
  }

  v_depth = XMLDocument::getOption(pElem, "depth");
  if (v_depth == "") {
    LogWarning("Edge with no depth");
    return;
  }

  v_sum = XMLDocument::getOption(pElem, "sum");

  if (isAFileOutOfDate(THEME_EDGEEFFECT_SPRITE_FILE_DIR + std::string("/") +
                       v_fileName) == false) {
    Sprite *pSprite = new EdgeEffectSprite(
      this, v_name, v_fileName, atof(v_scale.c_str()), atof(v_depth.c_str()));
    pSprite->setOrder(m_sprites.size());

    m_sprites.push_back(pSprite);

    ThemeFile v_file = {
      THEME_EDGEEFFECT_SPRITE_FILE_DIR + std::string("/") + v_fileName, v_sum
    };
    m_requiredFiles.push_back(v_file);
  }
}

BikerTheme *Theme::getPlayerTheme() {
  return m_player;
}

BikerTheme *Theme::getNetPlayerTheme() {
  return m_netplayer;
}

BikerTheme *Theme::getGhostTheme() {
  return m_ghost;
}

Sprite::Sprite(Theme *p_associated_theme, std::string v_name) {
  m_associated_theme = p_associated_theme;
  m_name = v_name;
  m_blendmode = SPRITE_BLENDMODE_DEFAULT;
  m_persistent = false;
}

Sprite::~Sprite() {}

Texture *Sprite::getTexture(bool bSmall,
                            WrapMode wrapMode,
                            FilterMode eFilterMode) {
  Texture *v_currentTexture;

  v_currentTexture = getCurrentTexture();
  if (v_currentTexture == NULL) {
    v_currentTexture =
      m_associated_theme->loadTexture(getCurrentTextureFileName(),
                                      bSmall,
                                      wrapMode,
                                      eFilterMode,
                                      m_persistent,
                                      this);
    setCurrentTexture(v_currentTexture);
  }

  // in register mode, register into the texture
  if (m_persistent == false) {
    if (m_associated_theme->getTextureManager()->registering()) {
      if (m_associated_theme->getTextureManager()->isRegisteredTexture(
            v_currentTexture) == false) {
        m_associated_theme->getTextureManager()->registerTexture(
          v_currentTexture);
      }
    }
  }

  return v_currentTexture;
}

int Sprite::getTextureSize() {
  return m_associated_theme->getTextureSize(getCurrentTextureFileName());
}

void Sprite::setOrder(unsigned int order) {
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

AnimationSprite::AnimationSprite(Theme *p_associated_theme,
                                 std::string p_name,
                                 std::string p_fileBase,
                                 std::string p_fileExtention,
                                 bool p_isTexture)
  : Sprite(p_associated_theme, p_name) {
  m_current_frame = 0;
  m_fileBase = p_fileBase;
  m_fileExtension = p_fileExtention;
  m_fFrameTime = 0.0;
  m_animation = false;
  m_isTexture = p_isTexture;

  if (p_isTexture) {
    m_type = SPRITE_TYPE_ANIMATION_TEXTURE;
  } else {
    m_type = SPRITE_TYPE_ANIMATION;
  }
}

AnimationSprite::~AnimationSprite() {
  cleanFrames();
}

std::string AnimationSprite::getFileDir() {
  if (m_isTexture) {
    return THEME_TEXTURE_SPRITE_FILE_DIR;
  } else {
    return THEME_ANIMATION_SPRITE_FILE_DIR;
  }
}

Texture *AnimationSprite::getCurrentTexture() {
  if (m_frames.size() == 0) {
    return NULL;
  }

  return m_frames[getCurrentFrame()]->getTexture();
}

std::string AnimationSprite::getCurrentTextureFileName() {
  if (m_animation == false) {
    return getFileDir() + "/" + m_fileBase + std::string(".") + m_fileExtension;
  } else {
    char v_num[3];
    snprintf(
      v_num, 3, "%02u", getCurrentFrame() % 100); // max 100 frames by animation

    return getFileDir() + "/" + m_fileBase + std::string(v_num) +
           std::string(".") + m_fileExtension;
  }
}

void AnimationSprite::setCurrentTexture(Texture *p_texture) {
  m_frames[getCurrentFrame()]->setTexture(p_texture);
}

int AnimationSprite::getCurrentFrame() {
  if (m_animation == false) {
    return 0;
  } else {
    /* Next frame? */
    float v_realTime = GameApp::getXMTime();

    while (v_realTime > m_fFrameTime + m_frames[m_current_frame]->getDelay()) {
      m_fFrameTime = v_realTime;
      m_current_frame++;
      if (m_current_frame == m_frames.size())
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

void AnimationSprite::addFrame(float p_centerX,
                               float p_centerY,
                               float p_width,
                               float p_height,
                               float p_delay) {
  m_frames.push_back(new AnimationSpriteFrame(
    this, p_centerX, p_centerY, p_width, p_height, p_delay));
  if (m_frames.size() > 1)
    m_animation = true;
}

void AnimationSprite::cleanFrames() {
  for (unsigned int i = 0; i < m_frames.size(); i++) {
    delete m_frames[i];
  }
  m_frames.clear();
}

void AnimationSprite::loadTextures() {
  unsigned int saveCurFrame = m_current_frame;

  // reset frameTime so that getCurrentFrame does not increment it
  m_fFrameTime = GameApp::getXMTime();

  for (unsigned int i = 0; i < m_frames.size(); i++) {
    m_current_frame = i;
    getTexture();
  }

  m_current_frame = saveCurFrame;
}

void AnimationSprite::invalidateTextures() {
  unsigned int saveCurFrame = m_current_frame;

  // reset frameTime so that getCurrentFrame does not increment it
  m_fFrameTime = GameApp::getXMTime();

  for (unsigned int i = 0; i < m_frames.size(); i++) {
    m_current_frame = i;
    setCurrentTexture(NULL);
  }

  m_current_frame = saveCurFrame;
}

AnimationSpriteFrame::AnimationSpriteFrame(
  AnimationSprite *p_associatedAnimationSprite,
  float p_centerX,
  float p_centerY,
  float p_width,
  float p_height,
  float p_delay) {
  m_associatedAnimationSprite = p_associatedAnimationSprite;
  m_texture = NULL;
  m_centerX = p_centerX;
  m_centerY = p_centerY;
  m_width = p_width;
  m_height = p_height;
  m_delay = p_delay;
}

AnimationSpriteFrame::~AnimationSpriteFrame() {}

Texture *AnimationSpriteFrame::getTexture() {
  return m_texture;
}

void AnimationSpriteFrame::setTexture(Texture *p_texture) {
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

BikerPartSprite::BikerPartSprite(Theme *p_associated_theme,
                                 std::string p_name,
                                 std::string p_fileName)
  : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
  m_type = SPRITE_TYPE_BIKERPART;
  m_persistent = true;
}

BikerPartSprite::~BikerPartSprite() {}

std::string BikerPartSprite::getFileDir() {
  return THEME_BIKERPART_SPRITE_FILE_DIR;
}

EffectSprite::EffectSprite(Theme *p_associated_theme,
                           std::string p_name,
                           std::string p_fileName)
  : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
  m_type = SPRITE_TYPE_EFFECT;
}

EffectSprite::~EffectSprite() {}

std::string EffectSprite::getFileDir() {
  return THEME_EFFECT_SPRITE_FILE_DIR;
}

EdgeEffectSprite::EdgeEffectSprite(Theme *p_associated_theme,
                                   std::string p_name,
                                   std::string p_filename,
                                   float p_fScale,
                                   float p_fDepth)
  : SimpleFrameSprite(p_associated_theme, p_name, p_filename) {
  m_fScale = p_fScale;
  m_fDepth = p_fDepth;
  m_type = SPRITE_TYPE_EDGEEFFECT;
}

EdgeEffectSprite::~EdgeEffectSprite() {}

std::string EdgeEffectSprite::getFileDir() {
  return THEME_EDGEEFFECT_SPRITE_FILE_DIR;
}

float EdgeEffectSprite::getScale() const {
  return m_fScale;
}

float EdgeEffectSprite::getDepth() const {
  return m_fDepth;
}

Texture *EdgeEffectSprite::getTexture(bool bSmall,
                                      WrapMode wrapMode,
                                      FilterMode eFilterMode) {
  return Sprite::getTexture(bSmall, wrapMode, eFilterMode);
}

FontSprite::FontSprite(Theme *p_associated_theme,
                       std::string p_name,
                       std::string p_fileName)
  : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
  m_type = SPRITE_TYPE_FONT;
  m_persistent = true;
}

FontSprite::~FontSprite() {}

std::string FontSprite::getFileDir() {
  return THEME_FONT_SPRITE_FILE_DIR;
}

MiscSprite::MiscSprite(Theme *p_associated_theme,
                       std::string p_name,
                       std::string p_fileName)
  : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
  m_type = SPRITE_TYPE_MISC;
  m_persistent = true;
}

MiscSprite::~MiscSprite() {}

std::string MiscSprite::getFileDir() {
  return THEME_MISC_SPRITE_FILE_DIR;
}

UISprite::UISprite(Theme *p_associated_theme,
                   std::string p_name,
                   std::string p_fileName)
  : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
  m_type = SPRITE_TYPE_UI;
  m_persistent = true;
}

UISprite::~UISprite() {}

std::string UISprite::getFileDir() {
  return THEME_UI_SPRITE_FILE_DIR;
}

TextureSprite::TextureSprite(Theme *p_associated_theme,
                             std::string p_name,
                             std::string p_fileName)
  : SimpleFrameSprite(p_associated_theme, p_name, p_fileName) {
  m_type = SPRITE_TYPE_TEXTURE;
}

TextureSprite::~TextureSprite() {}

std::string TextureSprite::getFileDir() {
  return THEME_TEXTURE_SPRITE_FILE_DIR;
}

SimpleFrameSprite::SimpleFrameSprite(Theme *p_associated_theme,
                                     std::string p_name,
                                     std::string p_fileName)
  : Sprite(p_associated_theme, p_name) {
  m_fileName = p_fileName;
  m_texture = NULL;
}

SimpleFrameSprite::~SimpleFrameSprite() {}

void SimpleFrameSprite::loadTextures() {
  getTexture();
}

void SimpleFrameSprite::invalidateTextures() {
  setCurrentTexture(NULL);
}

Texture *SimpleFrameSprite::getCurrentTexture() {
  return m_texture;
}

std::string SimpleFrameSprite::getCurrentTextureFileName() {
  return getFileDir() + "/" + m_fileName;
}

void SimpleFrameSprite::setCurrentTexture(Texture *p_texture) {
  m_texture = p_texture;
}

BikerTheme::BikerTheme(Theme *p_associated_theme,
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
                       Color p_gfxLowWheelColor) {
  m_associated_theme = p_associated_theme;
  m_Body = p_Body;
  m_Front = p_Front;
  m_Rear = p_Rear;
  m_Wheel = p_Wheel;
  m_LowerArm = p_LowerArm;
  m_LowerLeg = p_LowerLeg;
  m_Torso = p_Torso;
  m_UpperArm = p_UpperArm;
  m_UpperLeg = p_UpperLeg;

  m_UglyRiderColor = p_UglyRiderColor;
  m_UglyWheelColor = p_UglyWheelColor;
  m_gfxLowRiderColor = p_gfxLowRiderColor;
  m_gfxLowFillColor = p_gfxLowFillColor;
  m_gfxLowWheelColor = p_gfxLowWheelColor;

  m_ghostEffect = p_ghostEffect;
}

BikerTheme::~BikerTheme() {}

Sprite *BikerTheme::getBody() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_Body);
}

Sprite *BikerTheme::getFront() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_Front);
}

Sprite *BikerTheme::getRear() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_Rear);
}

Sprite *BikerTheme::getWheel() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_Wheel);
}

Sprite *BikerTheme::getLowerArm() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_LowerArm);
}

Sprite *BikerTheme::getLowerLeg() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_LowerLeg);
}

Sprite *BikerTheme::getTorso() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_Torso);
}

Sprite *BikerTheme::getUpperArm() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_UpperArm);
}

Sprite *BikerTheme::getUpperLeg() {
  return m_associated_theme->getSprite(SPRITE_TYPE_BIKERPART, m_UpperLeg);
}

Color BikerTheme::getUglyRiderColor() {
  return m_UglyRiderColor;
}

Color BikerTheme::getUglyWheelColor() {
  return m_UglyWheelColor;
}

Color BikerTheme::getGfxLowRiderColor() {
  return m_gfxLowRiderColor;
}

Color BikerTheme::getGfxLowFillColor() {
  return m_gfxLowFillColor;
}

Color BikerTheme::getGfxLowWheelColor() {
  return m_gfxLowWheelColor;
}

bool BikerTheme::getGhostEffect() const {
  return m_ghostEffect;
}

void ThemeChoicer::initThemesFromDir(xmDatabase *i_db) {
  std::vector<std::string> v_themesFiles = XMFS::findPhysFiles(
    FDT_DATA, std::string(THEMES_DIRECTORY) + std::string("/*.xml"), true);
  std::string v_name;

  i_db->themes_add_begin();
  for (unsigned int i = 0; i < v_themesFiles.size(); i++) {
    try {
      v_name = getThemeNameFromFile(v_themesFiles[i]);
      if (i_db->themes_exists(v_name) == false) {
        i_db->themes_add(v_name, v_themesFiles[i]);
      } else {
        LogWarning(
          std::string("Theme " + v_name + " is present several times").c_str());
      }
    } catch (Exception &e) {
      /* anyway, give up this theme */
    }
  }
  i_db->themes_add_end();
}

std::string ThemeChoicer::getThemeNameFromFile(std::string p_themeFile) {
  XMLDocument v_xml;
  xmlNodePtr v_xmlElt;
  std::string m_name;

  /* open the file */
  v_xml.readFromFile(FDT_DATA, p_themeFile);

  v_xmlElt = v_xml.getRootNode("xmoto_theme");
  if (v_xmlElt == NULL) {
    throw Exception("unable to analyze xml file");
  }

  m_name = XMLDocument::getOption(v_xmlElt, "name");
  if (m_name == "") {
    throw Exception("error : the theme has no name !");
  }

  return m_name;
}

SpriteBlendMode Theme::strToBlendMode(const std::string &s) {
  if (s == "add")
    return SPRITE_BLENDMODE_ADDITIVE;
  return SPRITE_BLENDMODE_DEFAULT;
}

std::string Theme::blendModeToStr(SpriteBlendMode Mode) {
  if (Mode == SPRITE_BLENDMODE_ADDITIVE)
    return "add";
  return "default";
}

ThemeMusic::ThemeMusic(Theme *p_associated_theme,
                       std::string i_name,
                       std::string i_fileName) {
  m_associated_theme = p_associated_theme;
  m_name = i_name;
  m_fileName = i_fileName;
}

ThemeMusic::~ThemeMusic() {}

std::string ThemeMusic::Name() const {
  return m_name;
}

std::string ThemeMusic::FileName() const {
  return m_fileName;
}

std::string ThemeMusic::FilePath() const {
  return XMFS::FullPath(FDT_DATA,
                        THEME_MUSICS_FILE_DIR + std::string("/") + m_fileName);
}

ThemeSound::ThemeSound(Theme *p_associated_theme,
                       std::string i_name,
                       std::string i_fileName) {
  m_associated_theme = p_associated_theme;
  m_name = i_name;
  m_fileName = i_fileName;
}

ThemeSound::~ThemeSound() {}

std::string ThemeSound::Name() const {
  return m_name;
}

std::string ThemeSound::FileName() const {
  return m_fileName;
}

std::string ThemeSound::FilePath() const {
  return THEME_SOUNDS_FILE_DIR + std::string("/") + m_fileName;
}
