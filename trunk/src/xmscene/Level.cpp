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
 *  Level files are stored as XML. The LevelSrc class represents one of these
 *  files and allows on-the-fly changes to everything - useful both in game 
 *  code and in editor code.
 */

#include "Level.h"
#include "../VFileIO.h"
#include "../XMBuild.h"
#include "../VXml.h"
#include "../helpers/Color.h"
#include "../helpers/Log.h"
#include "../db/xmDatabase.h"
#include "Block.h"
#include "Entity.h"
#include "Zone.h"
#include "SkyApparence.h"
#include "../Collision.h"
#include "Scene.h"
#include "../chipmunk/chipmunk.h"
#include "ChipmunkWorld.h"
#include "PhysicsSettings.h"
#include "../Theme.h"

Level::Level() {
  m_xmotoTooOld = false;
  m_leftLimit   = 0.0;
  m_rightLimit  = 0.0;
  m_topLimit    = 0.0;
  m_bottomLimit = 0.0;
  m_xmlSource   = NULL;
  m_nbEntitiesToTake = 0;
  m_borderTexture = "";
  m_numberLayer   = 0;
  m_isScripted    = false;
  m_isPhysics     = false;
  m_sky           = new SkyApparence();

  m_rSpriteForStrawberry      = "Strawberry";
  m_rSpriteForWecker 	      = "Wrecker";
  m_rSpriteForFlower 	      = "Flower";
  m_rSpriteForStar            = "Star";
  m_rSpriteForCheckpointDown  = "Checkpoint_1";
  m_rSpriteForCheckpointUp    = "Wrecker";
  m_rSoundForPickUpStrawberry = "PickUpStrawberry";

  m_strawberrySprite = NULL;
  m_wreckerSprite    = NULL;
  m_flowerSprite     = NULL;
  m_starSprite       = NULL;
  m_checkpointSpriteDown = NULL;
  m_checkpointSpriteUp = NULL;
}

Level::~Level() {
  unloadToPlay();
  unloadLevelBody();
  delete m_sky;
}

std::string Level::Id() const {
  return m_id;
}

std::string Level::Name() const {
  return m_name;
}

std::string Level::Author() const {
  return m_author;
}

std::string Level::Date() const {
  return m_date;
}

std::string Level::Description() const {
  return m_description;
}

std::string Level::Music() const {
  return m_music;
}

bool Level::isXMotoTooOld() const {
  return m_xmotoTooOld;
}

std::string Level::getRequiredVersion() const {
  return m_requiredVersion;
}

std::string Level::Pack() const {
  return m_pack;
}

std::string Level::PackNum() const {
  return m_packNum;
}

float Level::LeftLimit() const {
  return m_leftLimit;
}

float Level::RightLimit() const {
  return m_rightLimit;
}

float Level::TopLimit() const {
  return m_topLimit;
}

void Level::setLimits(float v_leftLimit, float v_rightLimit, float v_topLimit, float v_bottomLimit) {
  m_leftLimit   = v_leftLimit;
  m_rightLimit  = v_rightLimit;
  m_topLimit    = v_topLimit;
  m_bottomLimit = v_bottomLimit;
}

float Level::BottomLimit() const {
  return m_bottomLimit;
}

Vector2f Level::PlayerStart() const {
  return m_playerStart;
}

std::string Level::FileName() const {
  return m_fileName;
}

void Level::setFileName(const std::string& i_filename) {
  m_fileName = i_filename;
}

std::string Level::Checksum() const {
  return m_checkSum;
}

bool Level::isScripted() const {
  return m_isScripted;
}

bool Level::isPhysics() const {
  return m_isPhysics;
}

void Level::updatePhysics(int i_time, int timeStep, CollisionSystem* p_CollisionSystem, ChipmunkWorld* i_chipmunkWorld, DBuffer* i_recorder) {

  if(i_chipmunkWorld == NULL) {
    return;
  }

  cpSpaceStep(i_chipmunkWorld->getSpace(), ((double)timeStep)/100.0);

  // loop through all blocks, looking for chipmunky ones
  for(unsigned int i=0; i<m_blocks.size(); i++) {
    m_blocks[i]->updatePhysics(i_time, timeStep, p_CollisionSystem, i_recorder);
  }
}

Block* Level::getBlockById(const std::string& i_id) {
  for(unsigned int i=0; i<m_blocks.size(); i++) {
    if(m_blocks[i]->Id() == i_id) {
      return m_blocks[i];
    }
  }
  throw Exception("Block '" + i_id + "'" + " doesn't exist");
}

Entity* Level::getEntityById(const std::string& i_id) {
  for(unsigned int i=0; i<m_entities.size(); i++) {
    if(m_entities[i]->Id() == i_id) {
      return m_entities[i];
    }
  }
  for(unsigned int i=0; i<m_entitiesDestroyed.size(); i++) {
    if(m_entitiesDestroyed[i]->Id() == i_id) {
      return m_entitiesDestroyed[i];
    }
  }
  for(unsigned int i=0; i<m_entitiesExterns.size(); i++) {
    if(m_entitiesExterns[i]->Id() == i_id) {
      return m_entitiesExterns[i];
    }
  }
  throw Exception("Entity '" + i_id + "'" + " doesn't exist");
}
 
Zone* Level::getZoneById(const std::string& i_id) {
  for(unsigned int i=0; i<m_zones.size(); i++) {
    if(m_zones[i]->Id() == i_id) {
      return m_zones[i];
    }
  }
  throw Exception("Zone '" + i_id + "'" + " doesn't exist");
}

Entity* Level::getStartEntity() {
  for(unsigned int i=0; i<m_entities.size(); i++) {
    if(m_entities[i]->SpriteName() == "PlayerStart") {
      return m_entities[i];
    }
  }
  throw Exception("No start found");
}

void Level::setId(const std::string& i_id) {
  m_id = i_id;
}

void Level::setName(const std::string& i_name) {
  m_name = i_name;
}

void Level::setDescription(const std::string& i_description) {
  m_description = i_description;
}

void Level::setDate(const std::string& i_date) {
  m_date = i_date;
}

void Level::setAuthor(const std::string& i_author) {
  m_author = i_author;
}

void Level::setCollisionSystem(CollisionSystem* p_CollisionSystem) {
  m_pCollisionSystem = p_CollisionSystem;
}

std::string Level::scriptFileName() const {
  return m_scriptFileName;
}

std::string Level::scriptSource() const {
  return m_scriptSource;
}

void Level::setScriptFileName(const std::string& i_scriptFileName) {
  m_scriptFileName = i_scriptFileName;
}

std::vector<Block *>& Level::Blocks() {
  return m_blocks;
}

std::vector<Zone *> &Level::Zones() {
  return m_zones;
}

std::vector<Zone *> &Level::DeathZones() {
  return m_zonesDeath;
}

std::vector<Zone *> &Level::TeleportZones() {
  return m_zonesTeleport;
}

const SkyApparence* Level::Sky() const {
  return m_sky;
}

void Level::killEntity(const std::string& i_entityId) {
  for(unsigned int i=0; i<m_entities.size(); i++) {
    if(m_entities[i]->Id() == i_entityId) {
      if(m_entities[i]->IsToTake()){
	m_nbEntitiesToTake--;
      }
      m_entities[i]->setAlive(false);
      m_entitiesDestroyed.push_back(m_entities[i]);
      m_entities.erase(m_entities.begin() + i);
      return;
    }
  }
  throw Exception("Entity '" + i_entityId + "' can't be killed");
}

std::vector<Joint*>& Level::Joints()
{
  return m_joints;
}

std::vector<Entity *>& Level::Entities() {
  return m_entities;
}

unsigned int Level::countToTakeEntities() {
  return m_nbEntitiesToTake;
}

void Level::revertEntityDestroyed(const std::string& i_entityId) {
  for(unsigned int i=0; i<m_entitiesDestroyed.size(); i++) {
    if(m_entitiesDestroyed[i]->Id() == i_entityId) {
      m_entitiesDestroyed[i]->setAlive(true);

      if(m_entitiesDestroyed[i]->IsToTake()){
	m_nbEntitiesToTake++;
      }

      m_entities.push_back(m_entitiesDestroyed[i]);

      /* add it back to the collision system */
      m_pCollisionSystem->addEntity(m_entitiesDestroyed[i]);

      m_entitiesDestroyed.erase(m_entitiesDestroyed.begin() + i);

      return;
    }
  }
  throw Exception("Entity '" + i_entityId + "' can't be reverted");
}

void Level::updateToTime(Scene& i_scene, PhysicsSettings* i_physicsSettings) {
  int      v_time    = i_scene.getTime();
  Vector2f v_gravity = i_scene.getGravity();

  std::vector<Entity*> v_entities = Entities();
  unsigned int         size       = v_entities.size();

  for(unsigned int i=0; i<size; i++) {
    v_entities[i]->updateToTime(v_time, v_gravity, i_physicsSettings);
  }

  std::vector<Entity*> v_externalEntities = EntitiesExterns();
  size                                    = v_externalEntities.size();

  for(unsigned int i=0; i<size; i++) {
    v_externalEntities[i]->updateToTime(v_time, v_gravity, i_physicsSettings);
  }
}

void Level::loadXML() {
  /* Load XML document and fetch tinyxml handle */
  unloadLevelBody();
  if(m_xmlSource == NULL) {
    m_xmlSource = new XMLDocument();
  }

  m_xmlSource->readFromFile(FDT_DATA, m_fileName, NULL, true);

  TiXmlDocument *pDoc = m_xmlSource->getLowLevelAccess();
  if(pDoc == NULL) throw Exception("failed to load level XML " + m_fileName);
  
  /* Start the fantastic parsing by fetching the <level> element */
  TiXmlElement *pLevelElem = XML::findElement(*m_xmlSource, NULL,std::string("level"));    
  if(pLevelElem == NULL) throw Exception("<level> tag not found in XML");
  
  /* Get level ID */
  m_id = XML::getOption(pLevelElem,"id");
  if(m_id == "") throw Exception("no ID specified in level XML");

  /* Get required xmoto version */
  m_requiredVersion = XML::getOption(pLevelElem,"rversion");
  m_xmotoTooOld = false;
  if(m_id != "") {
    /* Check version */
    if(compareVersionNumbers(XMBuild::getVersionString(),m_requiredVersion) < 0) {
      /* Our version is too low to load this */
      m_xmotoTooOld = true;
      LogWarning("Level '%s' requires a newer version (%s) to load!",m_id.c_str(),m_requiredVersion.c_str());
    }
  }
  
  /* Set default info */
  m_name = m_id;
  m_date = "";
  m_description = "";
  m_author = "";
  m_music = "";
  
  m_scriptFileName = "";
  m_scriptSource = "";
  m_borderTexture = "";

  m_bottomLimit = m_leftLimit = -50.0f;
  m_topLimit    = m_rightLimit = 50.0f;
  
  m_playerStart = Vector2f(0.0, 0.0);
  m_numberLayer = 0;
  
  m_isScripted = false;
  m_isPhysics  = false;

  m_sky->reInit();

  if(!m_xmotoTooOld) {    
    /* Get level pack */
    m_pack = XML::getOption(pLevelElem,"levelpack");
    m_packNum = XML::getOption(pLevelElem,"levelpackNum");      
    
    /* Get level <info> element */
    TiXmlElement *pInfoElem = XML::findElement(*m_xmlSource, pLevelElem,std::string("info"));
    if(pInfoElem != NULL) {
      /* Name */
      std::string Tmp = XML::getElementText(*m_xmlSource, pInfoElem,"name");
      if(Tmp != "") m_name = Tmp;

      /* Author */
      Tmp = XML::getElementText(*m_xmlSource, pInfoElem,"author");
      if(Tmp != "") m_author = Tmp;
      
      /* Description */
      Tmp = XML::getElementText(*m_xmlSource, pInfoElem,"description");
      if(Tmp != "") m_description = Tmp;
      
      /* Date */
      Tmp = XML::getElementText(*m_xmlSource, pInfoElem,"date");
      if(Tmp != "") m_date = Tmp;
      
      /* Sky */
      Tmp = XML::getElementText(*m_xmlSource, pInfoElem,"sky");
      if(Tmp != "") {
        m_sky->setTexture(Tmp);
        m_sky->setBlendTexture(Tmp);
      }
      

      /* advanced sky parameters ? */
      bool v_useAdvancedOptions = false;
      TiXmlElement *pSkyElem = XML::findElement(*m_xmlSource, NULL, std::string("sky"));
      std::string v_skyValue;
      if(pSkyElem != NULL) {
	v_skyValue = XML::getOption(pSkyElem, "zoom");
	if(v_skyValue != "") {
	  m_sky->setZoom(atof(v_skyValue.c_str()));
	  v_useAdvancedOptions = true;
	}
	v_skyValue = XML::getOption(pSkyElem, "offset");
	if(v_skyValue != "") {
	  m_sky->setOffset(atof(v_skyValue.c_str()));
	  v_useAdvancedOptions = true;
	}

	int v_r = -1, v_g = -1, v_b = -1, v_a = -1;
	v_skyValue = XML::getOption(pSkyElem, "color_r");
	if(v_skyValue != "") v_r = atoi(v_skyValue.c_str());
	v_skyValue = XML::getOption(pSkyElem, "color_g");
	if(v_skyValue != "") v_g = atoi(v_skyValue.c_str());
	v_skyValue = XML::getOption(pSkyElem, "color_b");
	if(v_skyValue != "") v_b = atoi(v_skyValue.c_str());
	v_skyValue = XML::getOption(pSkyElem, "color_a");
	if(v_skyValue != "") v_a = atoi(v_skyValue.c_str());
	if(v_r != -1 || v_g != -1 || v_b != -1 || v_a != -1) {
	  if(v_r == -1) v_r = 0;
	  if(v_g == -1) v_g = 0;
	  if(v_b == -1) v_b = 0;
	  if(v_a == -1) v_a = 0;
	  m_sky->setTextureColor(TColor(v_r, v_g, v_b, v_a));
	  v_useAdvancedOptions = true;
	}

	v_skyValue = XML::getOption(pSkyElem, "drifted");
	if(v_skyValue == "true") {
	  m_sky->setDrifted(true);
	  v_useAdvancedOptions = true;

	  v_skyValue = XML::getOption(pSkyElem, "driftZoom");
	  if(v_skyValue != "") m_sky->setDriftZoom(atof(v_skyValue.c_str()));

	  int v_r = -1, v_g = -1, v_b = -1, v_a = -1;
	  v_skyValue = XML::getOption(pSkyElem, "driftColor_r");
	  if(v_skyValue != "") v_r = atoi(v_skyValue.c_str());
	  v_skyValue = XML::getOption(pSkyElem, "driftColor_g");
	  if(v_skyValue != "") v_g = atoi(v_skyValue.c_str());
	  v_skyValue = XML::getOption(pSkyElem, "driftColor_b");
	  if(v_skyValue != "") v_b = atoi(v_skyValue.c_str());
	  v_skyValue = XML::getOption(pSkyElem, "driftColor_a");
	  if(v_skyValue != "") v_a = atoi(v_skyValue.c_str());
	  if(v_r != -1 || v_g != -1 || v_b != -1 || v_a != -1) {
	    if(v_r == -1) v_r = 0;
	    if(v_g == -1) v_g = 0;
	    if(v_b == -1) v_b = 0;
	    if(v_a == -1) v_a = 0;
	    m_sky->setDriftTextureColor(TColor(v_r, v_g, v_b, v_a)); 
	  }
	}
	
	/* Sky Blend texture */
        v_skyValue = XML::getOption(pSkyElem, "BlendTexture");
	if(v_skyValue != "") {
	  m_sky->setBlendTexture(v_skyValue.c_str());
	  v_useAdvancedOptions = true;
	}
	
      }
      if(v_useAdvancedOptions == false) {
	/* set old values in case no option is used */
	m_sky->setOldXmotoValuesFromTextureName();
      }

      /* Border */
      TiXmlElement *pBorderElem = XML::findElement(*m_xmlSource, NULL, std::string("border"));
      if(pBorderElem != NULL) {
	m_borderTexture = XML::getOption(pBorderElem, "texture");  
      }

      /* Music */
      TiXmlElement *pMusicElem = XML::findElement(*m_xmlSource, NULL, std::string("music"));
      if(pMusicElem != NULL) {
	m_music = XML::getOption(pMusicElem, "name");
	if(m_music == "None") {
	  m_music = "";
	}
      }
    }

    /* background level offsets */
    TiXmlElement* pLayerOffsets = XML::findElement(*m_xmlSource, NULL, std::string("layeroffsets"));
    if(pLayerOffsets == NULL){
      m_numberLayer = 0;
    }
    else {
      for(TiXmlElement* pElem = pLayerOffsets->FirstChildElement("layeroffset");
	  pElem!=NULL;
	  pElem=pElem->NextSiblingElement("layeroffset")) {
	Vector2f offset;
	offset.x = atof(XML::getOption(pElem, "x").c_str());
	offset.y = atof(XML::getOption(pElem, "y").c_str());
	m_numberLayer++;
	m_layerOffsets.push_back(offset);
	m_isLayerFront.push_back(XML::getOption(pElem, "frontlayer", "false") == "true");
      }
    }

    TiXmlElement *pThemeReplaceElem = XML::findElement(*m_xmlSource, pLevelElem,std::string("theme_replacements"));
    if(pThemeReplaceElem != NULL) {
      /* Get replacements  for sprites */
      for(TiXmlElement *pElem = pThemeReplaceElem->FirstChildElement("sprite_replacement");
	  pElem!=NULL;
	  pElem=pElem->NextSiblingElement("sprite_replacement")) {
	std::string v_old_name = XML::getOption(pElem, "old_name");
	std::string v_new_name = XML::getOption(pElem, "new_name");
	/* for efficacity and before other are not required, only change main ones */
	if        (v_old_name == "Strawberry" && v_new_name != "") {
	  m_rSpriteForStrawberry = v_new_name;
	} else	if(v_old_name == "Wrecker"    && v_new_name != "") {
	  m_rSpriteForWecker     = v_new_name;
	} else 	if(v_old_name == "Flower"     && v_new_name != "") {
	  m_rSpriteForFlower     = v_new_name;
	} else 	if(v_old_name == "Star"       && v_new_name != "") {
	  m_rSpriteForStar       = v_new_name;
	}
      }
      /* Get replacements  for sounds */
      for(TiXmlElement *pElem = pThemeReplaceElem->FirstChildElement("sound_replacement"); pElem!=NULL;
	  pElem=pElem->NextSiblingElement("sound_replacement")) {
	std::string v_old_name = XML::getOption(pElem, "old_name");
	std::string v_new_name = XML::getOption(pElem, "new_name");
	/* for efficacity and before other are not required, only change main ones */
	if        (v_old_name == "PickUpStrawberry") {
	  m_rSoundForPickUpStrawberry = v_new_name;
	}
      }
    }

    /* Get script */
    TiXmlElement *pScriptElem = XML::findElement(*m_xmlSource, pLevelElem,std::string("script"));
    if(pScriptElem != NULL) {
      m_isScripted = true;
        
      /* External script file specified? */
      m_scriptFileName = XML::getOption(pScriptElem,"source");      
      
      /* Encapsulated script? */
      for(TiXmlNode *pScript=pScriptElem->FirstChild();pScript!=NULL;
          pScript=pScript->NextSibling()) {
        if(pScript->Type() == TiXmlNode::TEXT) {
          m_scriptSource.append(pScript->Value());
        }
      }
    }    
    
    /* Get level limits */
    TiXmlElement *pLimitsElem = XML::findElement(*m_xmlSource, pLevelElem,std::string("limits"));
    if(pLimitsElem != NULL) {
      m_bottomLimit = atof( XML::getOption(pLimitsElem,"bottom","-50").c_str() );
      m_leftLimit = atof( XML::getOption(pLimitsElem,"left","-50").c_str() );
      m_topLimit = atof( XML::getOption(pLimitsElem,"top","50").c_str() );
      m_rightLimit = atof( XML::getOption(pLimitsElem,"right","50").c_str() );
    }

    /* Get zones */
    for(TiXmlElement *pElem = pLevelElem->FirstChildElement("zone"); pElem!=NULL;
        pElem=pElem->NextSiblingElement("zone")) {
      m_zones.push_back(Zone::readFromXml(pElem));
      m_zones.back()->updateZoneSpeciality(m_scriptSource);
      if(m_zones.back()->isDeathZone()) {
        m_zonesDeath.push_back(Zone::readFromXml(pElem));
      }
      else if(m_zones.back()->isTeleportZone()) {
        m_zonesTeleport.push_back(Zone::readFromXml(pElem)); 
      }
    }
    
    /* determine whether the levels is physics or not */
    //for(TiXmlElement *pElem = pLevelElem->FirstChildElement("block"); pElem!=NULL;
    //    pElem=pElem->NextSiblingElement("block")) {
    //  if(Block::isPhysics_readFromXml(m_xmlSource, pElem)) {
    //	m_isPhysics = true;
    //  }
    //}

    /* Get blocks */
    for(TiXmlElement *pElem = pLevelElem->FirstChildElement("block"); pElem!=NULL;
        pElem=pElem->NextSiblingElement("block")) {
      Block* v_block = Block::readFromXml(m_xmlSource, pElem);
      if(v_block->isPhysics()) {
	m_isPhysics = true;
      }
      m_blocks.push_back(v_block);
    }

    /* Get entities */
    for(TiXmlElement *pElem = pLevelElem->FirstChildElement("entity"); pElem!=NULL; 
        pElem=pElem->NextSiblingElement("entity")) {
      Entity* v_entity = Entity::readFromXml(pElem);
      if(v_entity->Speciality() == ET_JOINT) {
	Joint* v_joint = (Joint*)v_entity;
	v_joint->readFromXml(pElem);
	m_joints.push_back(v_joint);
      }
      else m_entities.push_back(v_entity);
    }    

    try {
      m_playerStart = getStartEntity()->InitialPosition();
    } catch(Exception &e) {
      LogWarning("no player start entity for level %s", Id().c_str());
      m_playerStart = Vector2f(0.0, 0.0);
    }
  }

  /* if blocks with layerid but level with no layer, throw an exception */
  for(unsigned int i=0; i<m_blocks.size(); i++){
    int blockLayer = m_blocks[i]->getLayer();
    if(blockLayer != -1){
      if((getNumberLayer() == 0) || (blockLayer >= getNumberLayer())){
	LogWarning("Level '%s' Block '%s' as a layer, but the level has no layer", m_id.c_str(), m_blocks[i]->Id().c_str());
	throw Exception("the block has a layer but the level is without layers");
      }
    }
  }

  addLimits();

  delete m_xmlSource;
  m_xmlSource = NULL;

  m_isBodyLoaded = true;
}

/* Load using the best way possible. File name must already be set!
  *  Return whether or not it was loaded from the cache. */
bool Level::loadReducedFromFile() {
  std::string cacheFileName;

  m_checkSum = XMFS::md5sum(FDT_DATA, FileName());

  // First try to load it from the cache
  bool cached = false;
  /* Determine name in cache */
  std::string LevelFileBaseName = XMFS::getFileBaseName(FileName());
  cacheFileName = getNameInCache();
    
  try {
    cached = importBinaryHeaderFromFile(FDT_CACHE, cacheFileName, m_checkSum);
  } catch (Exception &e) {
    LogWarning("Exception while loading binary level, will load "
	       "XML instead for '%s' (%s)", FileName().c_str(),
	       e.getMsg().c_str());
  }
  
  // If we couldn't get it from the cache, then load from (slow) XML
  if (!cached) {
    loadXML();
    exportBinary(FDT_CACHE, cacheFileName, m_checkSum); /* Cache it now */
  }
  
  unloadLevelBody(); /* remove body datas */

  return cached;
}

std::string Level::getNameInCache() const {
  return "LCache/" + Checksum() + XMFS::getFileBaseName(FileName()) + ".blv";
}

void Level::removeFromCache(xmDatabase *i_db, const std::string& i_id_level) {
  char **v_result;
  unsigned int nrow;
  v_result = i_db->readDB("SELECT checkSum, filepath FROM levels WHERE id_level=\"" +
			  xmDatabase::protectString(i_id_level) + "\";",
			  nrow);
  if(nrow == 1) {
    try {
      std::string v_checkSum = i_db->getResult(v_result, 2, 0, 0);
      std::string v_filePath = i_db->getResult(v_result, 2, 0, 1);
      XMFS::deleteFile(FDT_CACHE, "LCache/" + v_checkSum + XMFS::getFileBaseName(v_filePath) + ".blv");
    } catch(Exception &e) {
      /* ok, it was perhaps not in cache */
    }
  }
  i_db->read_DB_free(v_result);
}

/*===========================================================================
  Export binary level file
  ===========================================================================*/
void Level::exportBinary(FileDataType i_fdt, const std::string &FileName, const std::string& pSum) {
  /* Don't do this if we failed to load level from XML */
  if(isXMotoTooOld()) return;
  
  /* Export binary... */
  FileHandle *pfh = XMFS::openOFile(i_fdt, FileName);
  if(pfh == NULL) {
    LogWarning("Failed to export binary: %s",FileName.c_str());
  }
  else {
    exportBinaryHeader(pfh);

    XMFS::writeString(pfh,   m_sky->Texture());
    XMFS::writeFloat_LE(pfh, m_sky->Zoom());
    XMFS::writeFloat_LE(pfh, m_sky->Offset());
    XMFS::writeInt_LE(pfh,   m_sky->TextureColor().Red());
    XMFS::writeInt_LE(pfh,   m_sky->TextureColor().Green());
    XMFS::writeInt_LE(pfh,   m_sky->TextureColor().Blue());
    XMFS::writeInt_LE(pfh,   m_sky->TextureColor().Alpha());
    XMFS::writeBool(pfh,     m_sky->Drifted());
    XMFS::writeFloat_LE(pfh, m_sky->DriftZoom());
    XMFS::writeInt_LE(pfh,   m_sky->DriftTextureColor().Red());
    XMFS::writeInt_LE(pfh,   m_sky->DriftTextureColor().Green());
    XMFS::writeInt_LE(pfh,   m_sky->DriftTextureColor().Blue());
    XMFS::writeInt_LE(pfh,   m_sky->DriftTextureColor().Alpha());
    
    XMFS::writeString(pfh,   m_sky->BlendTexture());
    
    XMFS::writeString(pfh,m_borderTexture);
    XMFS::writeString(pfh,m_scriptFileName);

    XMFS::writeFloat_LE(pfh,m_leftLimit);
    XMFS::writeFloat_LE(pfh,m_rightLimit);
    XMFS::writeFloat_LE(pfh,m_topLimit);
    XMFS::writeFloat_LE(pfh,m_bottomLimit);

    XMFS::writeString(pfh,m_rSpriteForStrawberry);
    XMFS::writeString(pfh,m_rSpriteForFlower);
    XMFS::writeString(pfh,m_rSpriteForWecker);
    XMFS::writeString(pfh,m_rSpriteForStar);
    XMFS::writeString(pfh,m_rSpriteForCheckpointDown);
    XMFS::writeString(pfh,m_rSpriteForCheckpointUp);
    XMFS::writeString(pfh,m_rSoundForPickUpStrawberry);

    XMFS::writeInt_LE(pfh, m_numberLayer);
    for(int i=0; i<m_numberLayer; i++){
      XMFS::writeFloat_LE(pfh, m_layerOffsets[i].x);
      XMFS::writeFloat_LE(pfh, m_layerOffsets[i].y);
      XMFS::writeBool(pfh,     m_isLayerFront[i]);
    }    

    /* Write script (if any) */
    XMFS::writeInt_LE(pfh,m_scriptSource.length());
    XMFS::writeBuf(pfh,(char *)m_scriptSource.c_str(),m_scriptSource.length());
      
    /* Write blocks */
    XMFS::writeInt_LE(pfh,m_blocks.size());
    for(unsigned int i=0;i<m_blocks.size();i++) {
      m_blocks[i]->saveBinary(pfh);
    }
      
    /* Write entities */
    XMFS::writeInt_LE(pfh,m_entities.size());
    for(unsigned int i=0;i<m_entities.size();i++) {
      m_entities[i]->saveBinary(pfh);
    }

    // write joints
    XMFS::writeInt_LE(pfh, m_joints.size());
    for(unsigned int i=0; i<m_joints.size(); i++) {
      m_joints[i]->saveBinary(pfh);
    }
      
    /* Write zones */
    XMFS::writeInt_LE(pfh,m_zones.size());
    for(unsigned int i=0;i<m_zones.size();i++) {
      m_zones[i]->saveBinary(pfh);
    }
                
    /* clean up */
    XMFS::closeFile(pfh);
  }
}
  
bool Level::isFullyLoaded() const {
  return m_isBodyLoaded;
}

void Level::loadFullyFromFile() {
  if(importBinary(FDT_CACHE, getNameInCache(), Checksum()) == false) {
    loadXML();
    exportBinary(FDT_CACHE, getNameInCache(), m_checkSum);
  }
  loadRemplacementSprites();
}

void Level::importBinaryHeader(FileHandle *pfh) {
  unloadLevelBody();

  m_isBodyLoaded = false;  
  m_playerStart  = Vector2f(0.0, 0.0);
  m_xmotoTooOld  = false;

  int nFormat = XMFS::readInt_LE(pfh);
  
  if(nFormat != CACHE_LEVEL_FORMAT_VERSION) {
    throw Exception("Old file format");
  }
  
  m_checkSum    = XMFS::readString(pfh);
  m_id      	= XMFS::readString(pfh);
  m_pack    	= XMFS::readString(pfh);
  m_packNum 	= XMFS::readString(pfh);
  m_name    	= XMFS::readString(pfh);
  m_description = XMFS::readString(pfh);
  m_author 	= XMFS::readString(pfh);
  m_date   	= XMFS::readString(pfh);
  m_music       = XMFS::readString(pfh);
  m_isScripted  = XMFS::readBool(pfh);
  m_isPhysics   = XMFS::readBool(pfh);
}

void Level::importHeader(const std::string& i_id,
			 const std::string& i_checkSum,
			 const std::string& i_pack,
			 const std::string& i_packNum,
			 const std::string& i_name,
			 const std::string& i_description,
			 const std::string& i_author,
			 const std::string& i_date,
			 const std::string& i_music,
			 bool i_isScripted,
			 bool i_isPhysics
			 ) {
  unloadLevelBody();

  m_isBodyLoaded = false;  
  m_playerStart  = Vector2f(0.0, 0.0);
  m_xmotoTooOld  = false;

  m_id      	= i_id;
  m_checkSum    = i_checkSum;
  m_pack    	= i_pack;
  m_packNum 	= i_packNum;
  m_name    	= i_name;
  m_description = i_description;
  m_author 	= i_author;
  m_date   	= i_date;
  m_music       = i_music;
  m_isScripted  = i_isScripted;
  m_isPhysics   = i_isPhysics;
}

  /*===========================================================================
  Import binary level file
  ===========================================================================*/
bool Level::importBinaryHeaderFromFile(FileDataType i_fdt, const std::string &FileName, const std::string& pSum) {
  /* Import binary */
  FileHandle *pfh = XMFS::openIFile(i_fdt, FileName, true);
  if(pfh == NULL) {
    return false;
  }

  try {
    importBinaryHeader(pfh);
    if(m_checkSum != pSum) {
      LogWarning("CRC check failed, can't import: %s",FileName.c_str());
      XMFS::closeFile(pfh);
      return false;
    }
  } catch(Exception &e) {
    XMFS::closeFile(pfh);
    return false;
  }
             
  /* clean up */
  XMFS::closeFile(pfh);

  return true;
}

void Level::exportBinaryHeader(FileHandle *pfh) {
  /* version two includes dynamic information about blocks */
  /* 3 -> includes now the grip of the block, width and height of the sprites */      
  /* 4 -> includes level pack num */
  /* 5 -> clean code */
  /* Write CRC32 of XML */
  XMFS::writeInt_LE(pfh, CACHE_LEVEL_FORMAT_VERSION);
  XMFS::writeString(pfh	    , m_checkSum);
  XMFS::writeString(pfh	    , m_id);
  XMFS::writeString(pfh	    , m_pack);
  XMFS::writeString(pfh	    , m_packNum);
  XMFS::writeString(pfh	    , m_name);
  XMFS::writeString(pfh	    , m_description);
  XMFS::writeString(pfh	    , m_author);
  XMFS::writeString(pfh	    , m_date);
  XMFS::writeString(pfh	    , m_music);
  XMFS::writeBool(  pfh	    , m_isScripted);
  XMFS::writeBool(  pfh	    , m_isPhysics);
}

bool Level::importBinary(FileDataType i_fdt, const std::string &FileName, const std::string& pSum) {
  unloadLevelBody();
  bool bRet = true;
  
  m_playerStart = Vector2f(0.0, 0.0);
    
  m_xmotoTooOld = false;
  
  /* Import binary */
  FileHandle *pfh = XMFS::openIFile(i_fdt, FileName, true);
  if(pfh == NULL) {
    return false;
  }
  else {
    /* Read tag - it tells something about the format */
    int nFormat = XMFS::readInt_LE(pfh);
    
    if(nFormat == CACHE_LEVEL_FORMAT_VERSION) { /* reject other formats */
      /* Read "format 1" / "format 2" binary level */
      /* Right */
      std::string md5sum = XMFS::readString(pfh);
      if(md5sum != pSum) {
        LogWarning("CRC check failed, can't import: %s",FileName.c_str());
        bRet = false;
      }
      else {
        /* Read header */
        m_id = XMFS::readString(pfh);
        m_pack = XMFS::readString(pfh);
        m_packNum = XMFS::readString(pfh);
        m_name = XMFS::readString(pfh);
        m_description = XMFS::readString(pfh);
        m_author = XMFS::readString(pfh);
        m_date = XMFS::readString(pfh);
        m_music = XMFS::readString(pfh);
	m_isScripted = XMFS::readBool(pfh);
	m_isPhysics  = XMFS::readBool(pfh);

	/* sky */
        m_sky->setTexture(XMFS::readString(pfh));
	m_sky->setZoom(XMFS::readFloat_LE(pfh));
	m_sky->setOffset(XMFS::readFloat_LE(pfh));
        
	int v_r, v_g, v_b, v_a;
	v_r = XMFS::readInt_LE(pfh);
	v_g = XMFS::readInt_LE(pfh);
	v_b = XMFS::readInt_LE(pfh);
	v_a = XMFS::readInt_LE(pfh);
	m_sky->setTextureColor(TColor(v_r, v_g, v_b, v_a));

	m_sky->setDrifted(XMFS::readBool(pfh));
	m_sky->setDriftZoom(XMFS::readFloat_LE(pfh));

	v_r = XMFS::readInt_LE(pfh);
	v_g = XMFS::readInt_LE(pfh);
	v_b = XMFS::readInt_LE(pfh);
	v_a = XMFS::readInt_LE(pfh);
	m_sky->setDriftTextureColor(TColor(v_r, v_g, v_b, v_a));
	/* *** */
        m_sky->setBlendTexture(XMFS::readString(pfh));
        m_borderTexture = XMFS::readString(pfh);

        m_scriptFileName = XMFS::readString(pfh);

        m_leftLimit = XMFS::readFloat_LE(pfh);
        m_rightLimit = XMFS::readFloat_LE(pfh);
        m_topLimit = XMFS::readFloat_LE(pfh);
        m_bottomLimit = XMFS::readFloat_LE(pfh);

	/* theme replacements */
	m_rSpriteForStrawberry 	    = XMFS::readString(pfh);
	m_rSpriteForFlower     	    = XMFS::readString(pfh);
	m_rSpriteForWecker     	    = XMFS::readString(pfh);
	m_rSpriteForStar       	    = XMFS::readString(pfh);
	m_rSpriteForCheckpointDown  = XMFS::readString(pfh);
	m_rSpriteForCheckpointUp    = XMFS::readString(pfh);
	m_rSoundForPickUpStrawberry = XMFS::readString(pfh);

	if(m_rSpriteForStrawberry == "")
	  m_rSpriteForStrawberry = "Strawberry";
	if(m_rSpriteForFlower == "")
	  m_rSpriteForFlower = "Flower";
	if(m_rSpriteForWecker == "")
	  m_rSpriteForWecker = "Wrecker";
	if(m_rSpriteForStar == "")
	  m_rSpriteForStar = "Star";
	if(m_rSpriteForCheckpointDown == "")
	  m_rSpriteForCheckpointDown = "Checkpoint_down";
	if(m_rSpriteForCheckpointUp == "")
	  m_rSpriteForCheckpointUp = "Checkpoint_up";

	/* layers */
	m_numberLayer = XMFS::readInt_LE(pfh);
	for(int i=0; i<m_numberLayer; i++){
	  Vector2f offset;
	  offset.x = XMFS::readFloat_LE(pfh);
	  offset.y = XMFS::readFloat_LE(pfh);
	  m_layerOffsets.push_back(offset);
	  m_isLayerFront.push_back(XMFS::readBool(pfh));
	}

        /* Read embedded script */
        int nScriptSourceLen = XMFS::readInt_LE(pfh);
        if(nScriptSourceLen > 0) {
          char *pcTemp = new char[nScriptSourceLen+1];
          XMFS::readBuf(pfh,(char *)pcTemp,nScriptSourceLen);
          pcTemp[nScriptSourceLen]='\0';
            
          m_scriptSource = pcTemp;
            
          delete [] pcTemp;           
        }
        else
          m_scriptSource = "";

        /* Read blocks */
        int nNumBlocks = XMFS::readInt_LE(pfh);
        m_blocks.reserve(nNumBlocks);
        for(int i=0;i<nNumBlocks;i++) {
          m_blocks.push_back(Block::readFromBinary(pfh));
        }

        /* Read entities */
        int nNumEntities = XMFS::readInt_LE(pfh);
        m_entities.reserve(nNumEntities); // we dont need that much reserved space, since some strawberries were eaten
        for(int i=0;i<nNumEntities;i++) {
	  Entity* v_entity = Entity::readFromBinary(pfh);
	  m_entities.push_back(v_entity);
        }

	try {
	  m_playerStart = getStartEntity()->InitialPosition();
	} catch(Exception &e) {
	  LogWarning("no player start entity for level %s", Id().c_str());
	  m_playerStart = Vector2f(0.0, 0.0);
	}
	
        int nNumJoints = XMFS::readInt_LE(pfh);
        m_joints.reserve(nNumJoints);
        for(int i=0; i<nNumJoints; i++) {
	  Entity* v_entity = Entity::readFromBinary(pfh);
	  Joint* v_joint = (Joint*)v_entity;
	  v_joint->readFromBinary(pfh);
	  m_joints.push_back(v_joint);
        }

        /* Read zones */
        int nNumZones = XMFS::readInt_LE(pfh);
        m_zones.reserve(nNumZones);
        for(int i=0;i<nNumZones;i++) {
          m_zones.push_back(Zone::readFromBinary(pfh));
          m_zones.back()->updateZoneSpeciality(m_scriptSource);
          if(m_zones.back()->isDeathZone()) {
            m_zonesDeath.push_back(m_zones.back());
          }
          else if(m_zones.back()->isTeleportZone()) {
            m_zonesTeleport.push_back(m_zones.back());
          }
        }                                                                       
      }
    }
    else {
      LogWarning("Invalid binary format (%d), can't import: %s",nFormat,FileName.c_str());
      bRet = false;
    }
             
    /* clean up */
    XMFS::closeFile(pfh);
  }
    
  m_isBodyLoaded = bRet;
  return bRet;
}

/*===========================================================================
  Some static helpers
  ===========================================================================*/
int Level::compareLevel(const Level& i_lvl1, const Level& i_lvl2) {

  if(i_lvl1.Pack() == i_lvl2.Pack()) {
    return compareLevelSamePack(i_lvl1, i_lvl2);
  }

  std::string n1, n2;
  n1 = i_lvl1.Name();
  n2 = i_lvl2.Name();
  
  /* lowercase */
  for(unsigned int j=0;j<n1.length();j++)  
    n1[j] = tolower(n1[j]);
  for(unsigned int j=0;j<n2.length();j++)  
    n2[j] = tolower(n2[j]);
  
  if(n1 > n2) {
    return 1;
  } 
  
  if(n1 < n2) {
    return -1;
  } 
  
  return 0;
}

int Level::compareLevelSamePack(const Level& i_lvl1, const Level& i_lvl2) {
  std::string n1, n2;
  
  if(i_lvl1.m_packNum > i_lvl2.m_packNum) {
    return 1;
  }
  if(i_lvl1.m_packNum < i_lvl2.m_packNum) {
    return -1;
  }
  
  n1 = i_lvl1.m_name;
  n2 = i_lvl2.m_name;
  
  /* lowercase */
  for(unsigned int j=0;j<n1.length();j++)  
    n1[j] = tolower(n1[j]);
  for(unsigned int j=0;j<n2.length();j++)  
    n2[j] = tolower(n2[j]);
  
  if(n1 > n2) {
    return 1;
  } 
  
  if(n1 < n2) {
    return -1;
  } 
  
  return 0;
}

int Level::compareVersionNumbers(const std::string &v1,const std::string &v2) {
  int nMajor1=0,nMinor1=0,nRel1=0;
  int nMajor2=0,nMinor2=0,nRel2=0;
  
  sscanf(v1.c_str(),"%d.%d.%d",&nMajor1,&nMinor1,&nRel1);
  sscanf(v2.c_str(),"%d.%d.%d",&nMajor2,&nMinor2,&nRel2);
  
  if(nMajor1 < nMajor2) return -1;
  else if(nMajor1 > nMajor2) return 1;
  
  if(nMinor1 < nMinor2) return -1;
  else if(nMinor1 > nMinor2) return 1;
  
  if(nRel1 < nRel2) return -1;
  else if(nRel1 > nRel2) return 1;
  
  /* Same versions */
  return 0;    
}

int Level::loadToPlay(ChipmunkWorld* i_chipmunkWorld, PhysicsSettings* i_physicsSettings) {
  int v_nbErrors = 0;

  /* preparing blocks */
  for(unsigned int i=0; i<m_blocks.size(); i++) {
    try {
      v_nbErrors += m_blocks[i]->loadToPlay(m_pCollisionSystem, i_chipmunkWorld, i_physicsSettings);
    } catch(Exception &e) {
      throw Exception("Fail to load block '" + m_blocks[i]->Id() + "' :\n" + e.getMsg());
    }
  }
  
  /* Spawn initial entities */
  for(unsigned int i=0; i<m_entities.size(); i++) {
    m_entities[i]->loadToPlay(m_scriptSource);
    Vector2f v = m_entities[i]->DynamicPosition();

    m_pCollisionSystem->addEntity(m_entities[i]);

    if(m_entities[i]->IsToTake()){
      m_nbEntitiesToTake++;
    }
  }
  
  // create joints
  for(unsigned int i=0; i<m_joints.size(); i++) {
    m_joints[i]->loadToPlay(this, i_chipmunkWorld);
  }
  
  return v_nbErrors;
}

void Level::unloadToPlay() {
  /* unload blocks */
  for(unsigned int i=0; i<m_blocks.size(); i++) {
    m_blocks[i]->unloadToPlay();
  }

  /* reput the entities */
  for(unsigned int i=0; i<m_entitiesDestroyed.size(); i++) {
    m_entities.push_back(m_entitiesDestroyed[i]);
  }
  m_entitiesDestroyed.clear();

  for(unsigned int i=0; i<m_entities.size(); i++) {
    m_entities[i]->unloadToPlay();
  }
  
  /* destroy external entities */
  for(unsigned int i=0; i<m_entitiesExterns.size(); i++) {
    delete m_entitiesExterns[i];
  }
  m_entitiesExterns.clear();

  m_nbEntitiesToTake = 0;

  for(unsigned int i=0; i<m_joints.size(); i++) {
    m_joints[i]->unloadToPlay();
  }
  
}

void Level::addLimits() {
  Block *pBlock;
  Vector2f v_P;

  /* Create level surroundings (by limits) */    
  float fVMargin = 20,fHMargin = 20;

  pBlock = new Block("LEVEL_TOP");
  if(m_borderTexture != "") {
    pBlock->setTexture(m_borderTexture);
  }
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit()  - fHMargin, TopLimit() + fVMargin)));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit() + fHMargin, TopLimit() + fVMargin)));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit()           , TopLimit())));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit()            , TopLimit())));
  /* move border block to the second static blocks layer */
  pBlock->setIsLayer(true);
  Blocks().push_back(pBlock);
    
  pBlock = new Block("LEVEL_BOTTOM");
  if(m_borderTexture != "") {
    pBlock->setTexture(m_borderTexture);
  }
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit()           , BottomLimit())));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit() + fHMargin, BottomLimit() - fVMargin)));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit()  - fHMargin, BottomLimit() - fVMargin)));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit()            , BottomLimit())));
  pBlock->setIsLayer(true);
  Blocks().push_back(pBlock);

  pBlock = new Block("LEVEL_LEFT");
  if(m_borderTexture != "") {
    pBlock->setTexture(m_borderTexture);
  }
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit(), TopLimit())));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit(), BottomLimit())));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit() - fHMargin, BottomLimit() - fVMargin)));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit() - fHMargin, TopLimit() + fVMargin))); 
  pBlock->setIsLayer(true);
  Blocks().push_back(pBlock);
    
  pBlock = new Block("LEVEL_RIGHT");
  if(m_borderTexture != "") {
    pBlock->setTexture(m_borderTexture);
  }
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit(), TopLimit())));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit() + fHMargin, TopLimit() + fVMargin)));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit() + fHMargin, BottomLimit() - fVMargin)));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit(), BottomLimit())));
  pBlock->setIsLayer(true);
  Blocks().push_back(pBlock);
}

void Level::spawnEntity(Entity *v_entity) {
  m_entitiesExterns.push_back(v_entity);
  if(v_entity->IsToTake()){
    m_nbEntitiesToTake++;
  }
}

std::vector<Entity *>& Level::EntitiesExterns() {
  return m_entitiesExterns;
}

std::vector<Entity *>& Level::EntitiesDestroyed() {
  return m_entitiesDestroyed;
}

void Level::unloadLevelBody() {
  unloadToPlay();

  m_isBodyLoaded = false;

  /* zones */
  for(unsigned int i=0; i<m_zones.size(); i++) {
    delete m_zones[i];
  }
  m_zones.clear();

  /* blocks */
  for(unsigned int i=0; i<m_blocks.size(); i++) {
    delete m_blocks[i];
  }
  m_blocks.clear();

  /* entities */
  for(unsigned int i=0; i<m_entities.size(); i++) {
    delete m_entities[i];
  }
  m_entities.clear();

  if(m_xmlSource != NULL) {
    delete m_xmlSource;
  }

  m_numberLayer = 0;
  m_layerOffsets.clear();
  m_isLayerFront.clear();
}

void Level::rebuildCache() {
  exportBinary(FDT_CACHE, getNameInCache(), m_checkSum);
}

std::string Level::SpriteForStrawberry() const {
  return m_rSpriteForStrawberry;
}

std::string Level::SpriteForWecker() const {
  return m_rSpriteForWecker;
}

std::string Level::SpriteForFlower() const {
  return m_rSpriteForFlower;
}

std::string Level::SpriteForStar() const {
  return m_rSpriteForStar;
}

std::string Level::SpriteForCheckpointDown() const {
  return m_rSpriteForCheckpointDown;
}

std::string Level::SpriteForCheckpointUp() const {
  return m_rSpriteForCheckpointUp;
}

std::string Level::SoundForPickUpStrawberry() const {
  return m_rSoundForPickUpStrawberry;
}

Sprite* Level::strawberrySprite()
{
  return m_strawberrySprite;
}

Sprite* Level::wreckerSprite()
{
  return m_wreckerSprite;
}

Sprite* Level::flowerSprite()
{
  return m_flowerSprite;
}

Sprite* Level::starSprite()
{
  return m_starSprite;
}

Sprite* Level::checkpointSpriteDown()
{
  return m_checkpointSpriteDown;
}

Sprite* Level::checkpointSpriteUp()
{
  return m_checkpointSpriteUp;
}

void Level::loadRemplacementSprites()
{
  m_strawberrySprite = Theme::instance()->getSprite(SPRITE_TYPE_ANIMATION, SpriteForStrawberry());
  m_wreckerSprite    = Theme::instance()->getSprite(SPRITE_TYPE_ANIMATION, SpriteForWecker());
  m_flowerSprite     = Theme::instance()->getSprite(SPRITE_TYPE_ANIMATION, SpriteForFlower());
  m_starSprite       = Theme::instance()->getSprite(SPRITE_TYPE_ANIMATION, SpriteForStar());
  m_checkpointSpriteDown  = Theme::instance()->getSprite(SPRITE_TYPE_ANIMATION, SpriteForCheckpointDown());
  m_checkpointSpriteUp    = Theme::instance()->getSprite(SPRITE_TYPE_ANIMATION, SpriteForCheckpointUp());
}

float Level::averagePhysicBlocksSize() const {
  int v_nb;
  float v_total;
  float xmin = 0.0, 
        xmax = 0.0, 
        ymin = 0.0, 
        ymax = 0.0;

  if(m_isPhysics == false) {
    return 0;
  }  

  v_nb    = 0;
  v_total = 0.0;

  for(unsigned int i=0; i<m_blocks.size(); i++) {
    if(m_blocks[i]->isPhysics()) {
      v_nb ++;

      for(unsigned int j=0; j<m_blocks[i]->Vertices().size(); j++) {
	if(j == 0) {
	  xmin = m_blocks[i]->Vertices()[j]->Position().x;
	  xmax = m_blocks[i]->Vertices()[j]->Position().x;
	  ymin = m_blocks[i]->Vertices()[j]->Position().y;
	  ymax = m_blocks[i]->Vertices()[j]->Position().y;
	} else {
	  if(xmin > m_blocks[i]->Vertices()[j]->Position().x) {
	    xmin = m_blocks[i]->Vertices()[j]->Position().x;
	  }
	  if(xmax < m_blocks[i]->Vertices()[j]->Position().x) {
	    xmax = m_blocks[i]->Vertices()[j]->Position().x;
	  }
	  if(ymin > m_blocks[i]->Vertices()[j]->Position().y) {
	    ymin = m_blocks[i]->Vertices()[j]->Position().y;
	  }
	  if(ymax < m_blocks[i]->Vertices()[j]->Position().y) {
	    ymax = m_blocks[i]->Vertices()[j]->Position().y;
	  }
	}
      }

      if(xmax - xmin > ymax - ymin) {
	v_total += xmax - xmin;
      } else {
	v_total += ymax - ymin;
      }
    }
  }

  if(v_nb == 0) {
    return 0;
  }

  return v_total/v_nb;
}

float Level::maxPhysicBlocksSize() const {
  float v_max;
  float xmin = 0.0,
        xmax = 0.0,
        ymin = 0.0, 
        ymax = 0.0;

  if(m_isPhysics == false) {
    return 0;
  }  

  v_max = 0.0;

  for(unsigned int i=0; i<m_blocks.size(); i++) {
    if(m_blocks[i]->isPhysics()) {
      for(unsigned int j=0; j<m_blocks[i]->Vertices().size(); j++) {
	if(j == 0) {
	  xmin = m_blocks[i]->Vertices()[j]->Position().x;
	  xmax = m_blocks[i]->Vertices()[j]->Position().x;
	  ymin = m_blocks[i]->Vertices()[j]->Position().y;
	  ymax = m_blocks[i]->Vertices()[j]->Position().y;
	} else {
	  if(xmin > m_blocks[i]->Vertices()[j]->Position().x) {
	    xmin = m_blocks[i]->Vertices()[j]->Position().x;
	  }
	  if(xmax < m_blocks[i]->Vertices()[j]->Position().x) {
	    xmax = m_blocks[i]->Vertices()[j]->Position().x;
	  }
	  if(ymin > m_blocks[i]->Vertices()[j]->Position().y) {
	    ymin = m_blocks[i]->Vertices()[j]->Position().y;
	  }
	  if(ymax < m_blocks[i]->Vertices()[j]->Position().y) {
	    ymax = m_blocks[i]->Vertices()[j]->Position().y;
	  }
	}
      }

      if(xmax - xmin > ymax - ymin) {
	if(v_max < xmax - xmin) {
	  v_max += xmax - xmin;
	}
      } else {
	if(v_max < ymax - ymin) {
	  v_max += ymax - ymin;
	}
      }
    }
  }

  return v_max;
}

int Level::nbPhysicBlocks() const {
  int v_nb;

  if(m_isPhysics == false) {
    return 0;
  }  

  v_nb = 0;
  for(unsigned int i=0; i<m_blocks.size(); i++) {
    if(m_blocks[i]->isPhysics()) {
      v_nb++;
    }
  }

  return v_nb;
}
