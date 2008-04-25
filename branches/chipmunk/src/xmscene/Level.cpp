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
#include "VFileIO.h"
#include "XMBuild.h"
#include "VXml.h"
#include "helpers/Color.h"
#include "helpers/Log.h"
#include "db/xmDatabase.h"
#include "Block.h"
#include "Entity.h"
#include "Zone.h"
#include "SkyApparence.h"
#include "Collision.h"
#include "Scene.h"
#include "chipmunk/chipmunk.h"
#include "ChipmunkHelper.h"

#define CACHE_LEVEL_FORMAT_VERSION 19

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
  m_sky = new SkyApparence();
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

void Level::updatePhysics(int timeStep) {
  Block* b;

  cpSpaceStep(ChipmunkHelper::Instance()->getSpace(), ((double)timeStep)/100.0);

  // loop through all blocks, looking for chipmunky ones
  //
  for(unsigned int i=0; i<m_blocks.size(); i++) {
    if(m_blocks[i]->isPhysics() == true) {
      Block* b = m_blocks[i];

      // move block according to chipmunk
      m_blocks[i]->setDynamicPosition(Vector2f(((b->mBody->p.x ) / 10.0f) , (b->mBody->p.y ) / 10.0f));
      m_blocks[i]->setDynamicRotation(b->mBody->a);

      // inform collision system that there has been a change
      m_pCollisionSystem->moveDynBlock(m_blocks[i]);

      cpBodyResetForces(m_blocks[i]->mBody);
    } else if (m_blocks[i]->isDynamic() == true) {
      // This is a dynamic block - tell chipmunk about it
      b = m_blocks[i];
      b->mBody->p.x = b->DynamicPosition().x * 10.0f;
      b->mBody->p.y = b->DynamicPosition().y * 10.0f;
      cpBodySetAngle(b->mBody, b->DynamicRotation());

    }
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

void Level::updateToTime(MotoGame& i_scene) {
  bool v_b;

  for(unsigned int i=0;i<Entities().size();i++) {
    v_b = Entities()[i]->updateToTime(i_scene.getTime(), i_scene.getGravity());
  }
  for(unsigned int i=0;i<EntitiesExterns().size();i++) {
    v_b = EntitiesExterns()[i]->updateToTime(i_scene.getTime(), i_scene.getGravity());
  }
}

void Level::saveXML(void) {
  FileHandle *pfh = FS::openOFile(m_fileName);
  if(pfh == NULL) {
    /* Failed! */
    Logger::Log("** Warning ** : failed to save level '%s'",m_fileName.c_str());
    return;
  }
    
  FS::writeLineF(pfh,"<?xml version=\"1.0\" encoding=\"utf-8\"?>");

  std::string v_levelTag;
  v_levelTag = "<level id=\"" + m_id + "\"";
  if(m_pack != "") {
    v_levelTag += " levelpack=\"" + m_pack + "\"";
  }
  if(m_packNum != "") {
    v_levelTag += " levelpackNum=\"" + m_packNum + "\"";
  }
  if(m_requiredVersion != "") {
    v_levelTag += " rversion=\"" + m_requiredVersion + "\"";
  }
  v_levelTag += ">";
  FS::writeLineF(pfh, "%s", v_levelTag.c_str());
  
  /* INFO */
  FS::writeLineF(pfh,"\t<info>");
  FS::writeLineF(pfh,"\t\t<name>%s</name>",m_name.c_str());
  FS::writeLineF(pfh,"\t\t<description>%s</description>",m_description.c_str());
  FS::writeLineF(pfh,"\t\t<author>%s</author>",m_author.c_str());
  FS::writeLineF(pfh,"\t\t<date>%s</date>",m_date.c_str());
  if(m_sky->Drifted()) {
    FS::writeLineF(pfh,
			 "\t\t<sky zoom=\"%f\" offset=\"%f\" color_r=\"%i\" color_g=\"%i\" color_b=\"%i\" color_a=\"%i\" drifted=\"true\" driftZoom=\"%f\" driftColor_r=\"%i\" driftColor_g=\"%i\" driftColor_b=\"%i\" driftColor_a=\"%i\">%s</sky>",
			 m_sky->Zoom(),
			 m_sky->Offset(),
			 m_sky->TextureColor().Red(),
			 m_sky->TextureColor().Green(),
			 m_sky->TextureColor().Blue(),
			 m_sky->TextureColor().Alpha(),
			 m_sky->DriftZoom(),
			 m_sky->DriftTextureColor().Red(),
			 m_sky->DriftTextureColor().Green(),
			 m_sky->DriftTextureColor().Blue(),
			 m_sky->DriftTextureColor().Alpha(),
			 m_sky->Texture().c_str());
  } else {
    FS::writeLineF(pfh,
			 "\t\t<sky zoom=\"%f\" offset=\"%f\" color_r=\"%i\" color_g=\"%i\" color_b=\"%i\" color_a=\"%i\">%s</sky>",
			 m_sky->Zoom(),
			 m_sky->Offset(),
			 m_sky->TextureColor().Red(),
			 m_sky->TextureColor().Green(),
			 m_sky->TextureColor().Blue(),
			 m_sky->TextureColor().Alpha(),
			 m_sky->Texture().c_str());
  }
  FS::writeLineF(pfh,"\t\t<border texture=\"%s\" />",m_borderTexture.c_str());
  FS::writeLineF(pfh,"\t\t<music name=\"%s\" />", m_music.c_str());
  FS::writeLineF(pfh,"\t</info>");

  if(m_numberLayer > 0){
    FS::writeLineF(pfh, "\t\t<layeroffsets>");
    for(int i=0; i<m_numberLayer; i++){
      std::string front = "";
      if(m_isLayerFront[i] == true){
	front = " frontlayer=\"true\"";
      }

      FS::writeLineF(pfh, "\t\t\t<layeroffset x=\"%f\" y=\"%f\"%s/>",
			   m_layerOffsets[i].x, m_layerOffsets[i].y, front.c_str());
    }
    FS::writeLineF(pfh, "\t\t</layeroffsets>");
  }
  
  /* replacement sprites */
  if(m_rSpriteForStrawberry 	 != "" ||
     m_rSpriteForWecker     	 != "" ||
     m_rSpriteForFlower     	 != "" ||
     m_rSpriteForStar       	 != "" ||
     m_rSoundForPickUpStrawberry != ""
     ) {
    FS::writeLineF(pfh,"\t<theme_replacements>");
    if(m_rSpriteForStrawberry != "") {
      FS::writeLineF(pfh,"\t\t<sprite_replacement old_name=\"Strawberry\" new_name=\"%s\" />",
			   m_rSpriteForStrawberry.c_str());
    }
    if(m_rSpriteForWecker != "") {
      FS::writeLineF(pfh,"\t\t<sprite_replacement old_name=\"Wrecker\" new_name=\"%s\" />",
			   m_rSpriteForWecker.c_str());
    }
    if(m_rSpriteForFlower != "") {
      FS::writeLineF(pfh,"\t\t<sprite_replacement old_name=\"Flower\" new_name=\"%s\" />",
			   m_rSpriteForFlower.c_str());
    }
    if(m_rSpriteForStar != "") {
      FS::writeLineF(pfh,"\t\t<sprite_replacement old_name=\"Star\" new_name=\"%s\" />",
			   m_rSpriteForStar.c_str());
    }
    if(m_rSoundForPickUpStrawberry != "") {
      FS::writeLineF(pfh,"\t\t<sound_replacement old_name=\"PickUpStrawberry\" new_name=\"%s\" />",
			   m_rSoundForPickUpStrawberry.c_str());
    }
    FS::writeLineF(pfh,"\t</theme_replacements>");
  }

  /* MISC */
  if(m_scriptFileName != "" && m_scriptSource != "") {
    FS::writeLineF(pfh,"\t<script source=\"%s\">",m_scriptFileName.c_str());
    FS::writeByte(pfh,'\t'); FS::writeByte(pfh,'\t');
    
    /* Write script and transform dangerous characters */
    for(unsigned int i=0;i<m_scriptSource.length();i++) {
      int c = m_scriptSource[i];
      if(c == '<') FS::writeBuf(pfh,"&lt;",4);
      else if(c == '>') FS::writeBuf(pfh,"&gt;",4);
      else FS::writeByte(pfh,c);
    }
    //FS::writeBuf(pfh,(char *)m_ScriptSource.c_str(),m_ScriptSource.length());
    FS::writeLineF(pfh,"</script>");
  }
  else if(m_scriptFileName != "")
    FS::writeLineF(pfh,"\t<script source=\"%s\"/>",m_scriptFileName.c_str());
  else if(m_scriptSource != "") {
    FS::writeLineF(pfh,"\t<script>");
    FS::writeByte(pfh,'\t'); FS::writeByte(pfh,'\t');
    /* Write script and transform dangerous characters */
    for(unsigned int i=0;i<m_scriptSource.length();i++) {
      int c = m_scriptSource[i];
      if(c == '<') FS::writeBuf(pfh,"&lt;",4);
      else if(c == '>') FS::writeBuf(pfh,"&gt;",4);
      else FS::writeByte(pfh,c);
    }
    //      FS::writeBuf(pfh,(char *)m_ScriptSource.c_str(),m_ScriptSource.length());
    FS::writeLineF(pfh,"</script>");
  }
  
  FS::writeLineF(pfh,"\t<limits left=\"%f\" right=\"%f\" top=\"%f\" bottom=\"%f\"/>",m_leftLimit,m_rightLimit,m_topLimit,m_bottomLimit);
  
  /* BLOCKS */
  for(unsigned int i=0;i<m_blocks.size();i++) {
    m_blocks[i]->saveXml(pfh);
  }
  
  /* ENTITIES */
  for(unsigned int i=0;i<m_entities.size();i++) {
    m_entities[i]->saveXml(pfh);
  }
  
  /* ZONE */
  for(unsigned int i=0;i<m_zones.size();i++) {
    m_zones[i]->saveXml(pfh);
  }
  
  FS::writeLineF(pfh,"\n</level>");           
  FS::closeFile(pfh);
}

void Level::loadXML(void) {
  /* Load XML document and fetch tinyxml handle */
  unloadLevelBody();
  if(m_xmlSource == NULL) {
    m_xmlSource = new XMLDocument();
  }

  m_xmlSource->readFromFile(m_fileName, NULL);

  TiXmlDocument *pDoc = m_xmlSource->getLowLevelAccess();
  if(pDoc == NULL) throw Exception("failed to load level XML");
  
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
      Logger::Log("** Warning ** : Level '%s' requires a newer version (%s) to load!",m_id.c_str(),m_requiredVersion.c_str());
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
      if(Tmp != "") m_sky->setTexture(Tmp);

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
//      for(int i=0; i<m_numberLayer; i++){
//	Logger::Log("Level::loadXML offsets layer %d: %f, %f isfront: %s",
//		    i, m_layerOffsets[i].x, m_layerOffsets[i].y, m_isLayerFront[i]?"true":"false");
//      }
    }

    /* replacement theme */
    m_rSpriteForStrawberry 	= "";
    m_rSpriteForWecker 	   	= "";
    m_rSpriteForFlower 	   	= "";
    m_rSpriteForStar   	   	= "";
    m_rSoundForPickUpStrawberry = "";

    TiXmlElement *pThemeReplaceElem = XML::findElement(*m_xmlSource, pLevelElem,std::string("theme_replacements"));
    if(pThemeReplaceElem != NULL) {
      /* Get replacements  for sprites */
      for(TiXmlElement *pElem = pThemeReplaceElem->FirstChildElement("sprite_replacement"); pElem!=NULL;
	  pElem=pElem->NextSiblingElement("sprite_replacement")) {
	std::string v_old_name = XML::getOption(pElem, "old_name");
	std::string v_new_name = XML::getOption(pElem, "new_name");
	/* for efficacity and before other are not required, only change main ones */
	if        (v_old_name == "Strawberry") {
	  m_rSpriteForStrawberry = v_new_name;
	} else	if(v_old_name == "Wrecker") {
	  m_rSpriteForWecker     = v_new_name;
	} else 	if(v_old_name == "Flower") {
	  m_rSpriteForFlower     = v_new_name;
	} else 	if(v_old_name == "Star") {
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

    /* Get entities */
    for(TiXmlElement *pElem = pLevelElem->FirstChildElement("entity"); pElem!=NULL; 
        pElem=pElem->NextSiblingElement("entity")) {
      m_entities.push_back(Entity::readFromXml(pElem));
    }    

    try {
      m_playerStart = getStartEntity()->InitialPosition();
    } catch(Exception &e) {
      Logger::Log("Warning : no player start entity for level %s", Id().c_str());
      m_playerStart = Vector2f(0.0, 0.0);
    }    

    /* Get zones */
    for(TiXmlElement *pElem = pLevelElem->FirstChildElement("zone"); pElem!=NULL;
        pElem=pElem->NextSiblingElement("zone")) {
      m_zones.push_back(Zone::readFromXml(pElem));
    }
    
    /* Get blocks */
    for(TiXmlElement *pElem = pLevelElem->FirstChildElement("block"); pElem!=NULL;
        pElem=pElem->NextSiblingElement("block")) {
      m_blocks.push_back(Block::readFromXml(m_xmlSource, pElem));
    }  
  }

  /* if blocks with layerid but level with no layer, throw an exception */
  for(unsigned int i=0; i<m_blocks.size(); i++){
    int blockLayer = m_blocks[i]->getLayer();
    if(blockLayer != -1){
      if((getNumberLayer() == 0) || (blockLayer >= getNumberLayer())){
	Logger::Log("** Warning ** : Level '%s' Block '%s' as a layer, but the level has no layer", m_id.c_str(), m_blocks[i]->Id().c_str());
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

  m_checkSum = FS::md5sum(FileName());

  // First try to load it from the cache
  bool cached = false;
  /* Determine name in cache */
  std::string LevelFileBaseName = FS::getFileBaseName(FileName());
  cacheFileName = getNameInCache();
    
  try {
    cached = importBinaryHeaderFromFile(cacheFileName, m_checkSum);
  } catch (Exception &e) {
    Logger::Log("** Warning **: Exception while loading binary level, will load "
	      "XML instead for '%s' (%s)", FileName().c_str(),
	      e.getMsg().c_str());
  }
  
  // If we couldn't get it from the cache, then load from (slow) XML
  if (!cached) {
    loadXML();
    exportBinary(cacheFileName, m_checkSum); /* Cache it now */
  }
  
  unloadLevelBody(); /* remove body datas */

  return cached;
}

std::string Level::getNameInCache() const {
  return "LCache/" + Checksum() + FS::getFileBaseName(FileName()) + ".blv";
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
      FS::deleteFile("LCache/" + v_checkSum + FS::getFileBaseName(v_filePath) + ".blv");
    } catch(Exception &e) {
      /* ok, it was perhaps not in cache */
    }
  }
  i_db->read_DB_free(v_result);
}

/*===========================================================================
  Export binary level file
  ===========================================================================*/
void Level::exportBinary(const std::string &FileName, const std::string& pSum) {
  /* Don't do this if we failed to load level from XML */
  if(isXMotoTooOld()) return;
  
  /* Export binary... */
  FileHandle *pfh = FS::openOFile(FileName);
  if(pfh == NULL) {
    Logger::Log("** Warning ** : Failed to export binary: %s",FileName.c_str());
  }
  else {
    exportBinaryHeader(pfh);

    FS::writeString(pfh,   m_sky->Texture());
    FS::writeFloat_LE(pfh, m_sky->Zoom());
    FS::writeFloat_LE(pfh, m_sky->Offset());
    FS::writeInt_LE(pfh,   m_sky->TextureColor().Red());
    FS::writeInt_LE(pfh,   m_sky->TextureColor().Green());
    FS::writeInt_LE(pfh,   m_sky->TextureColor().Blue());
    FS::writeInt_LE(pfh,   m_sky->TextureColor().Alpha());
    FS::writeBool(pfh,     m_sky->Drifted());
    FS::writeFloat_LE(pfh, m_sky->DriftZoom());
    FS::writeInt_LE(pfh,   m_sky->DriftTextureColor().Red());
    FS::writeInt_LE(pfh,   m_sky->DriftTextureColor().Green());
    FS::writeInt_LE(pfh,   m_sky->DriftTextureColor().Blue());
    FS::writeInt_LE(pfh,   m_sky->DriftTextureColor().Alpha());

    FS::writeString(pfh,m_borderTexture);
    FS::writeString(pfh,m_scriptFileName);

    FS::writeFloat_LE(pfh,m_leftLimit);
    FS::writeFloat_LE(pfh,m_rightLimit);
    FS::writeFloat_LE(pfh,m_topLimit);
    FS::writeFloat_LE(pfh,m_bottomLimit);

    FS::writeString(pfh,m_rSpriteForStrawberry);
    FS::writeString(pfh,m_rSpriteForFlower);
    FS::writeString(pfh,m_rSpriteForWecker);
    FS::writeString(pfh,m_rSpriteForStar);
    FS::writeString(pfh,m_rSoundForPickUpStrawberry);

    FS::writeInt_LE(pfh, m_numberLayer);
    for(int i=0; i<m_numberLayer; i++){
      FS::writeFloat_LE(pfh, m_layerOffsets[i].x);
      FS::writeFloat_LE(pfh, m_layerOffsets[i].y);
      FS::writeBool(pfh,     m_isLayerFront[i]);
    }    

    /* Write script (if any) */
    FS::writeInt_LE(pfh,m_scriptSource.length());
    FS::writeBuf(pfh,(char *)m_scriptSource.c_str(),m_scriptSource.length());
      
    /* Write blocks */
    FS::writeInt_LE(pfh,m_blocks.size());
    for(unsigned int i=0;i<m_blocks.size();i++) {
      m_blocks[i]->saveBinary(pfh);
    }
      
    /* Write entities */
    FS::writeInt_LE(pfh,m_entities.size());
    for(unsigned int i=0;i<m_entities.size();i++) {
      m_entities[i]->saveBinary(pfh);
    }  
      
    /* Write zones */
    FS::writeInt_LE(pfh,m_zones.size());
    for(unsigned int i=0;i<m_zones.size();i++) {
      m_zones[i]->saveBinary(pfh);
    }
                
    /* clean up */
    FS::closeFile(pfh);
  }
}
  
bool Level::isFullyLoaded() const {
  return m_isBodyLoaded;
}

void Level::loadFullyFromFile() {
  if(importBinary(getNameInCache(), Checksum()) == false) {
    loadXML();
    exportBinary(getNameInCache(), m_checkSum);
  }
}

void Level::importBinaryHeader(FileHandle *pfh) {
  unloadLevelBody();

  m_isBodyLoaded = false;  
  m_playerStart  = Vector2f(0.0, 0.0);
  m_xmotoTooOld  = false;

  int nFormat = FS::readInt_LE(pfh);
  
  if(nFormat != CACHE_LEVEL_FORMAT_VERSION) {
    throw Exception("Old file format");
  }
  
  m_checkSum    = FS::readString(pfh);
  m_id      	= FS::readString(pfh);
  m_pack    	= FS::readString(pfh);
  m_packNum 	= FS::readString(pfh);
  m_name    	= FS::readString(pfh);
  m_description = FS::readString(pfh);
  m_author 	= FS::readString(pfh);
  m_date   	= FS::readString(pfh);
  m_music       = FS::readString(pfh);
  m_isScripted  = FS::readBool(pfh);
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
			 bool i_isScripted
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
}

  /*===========================================================================
  Import binary level file
  ===========================================================================*/
bool Level::importBinaryHeaderFromFile(const std::string &FileName, const std::string& pSum) {
  /* Import binary */
  FileHandle *pfh = FS::openIFile(FileName);
  if(pfh == NULL) {
    return false;
  }

  try {
    importBinaryHeader(pfh);
    if(m_checkSum != pSum) {
      Logger::Log("** Warning ** : CRC check failed, can't import: %s",FileName.c_str());
      FS::closeFile(pfh);
      return false;
    }
  } catch(Exception &e) {
    FS::closeFile(pfh);
    return false;
  }
             
  /* clean up */
  FS::closeFile(pfh);

  return true;
}

void Level::exportBinaryHeader(FileHandle *pfh) {
  /* version two includes dynamic information about blocks */
  /* 3 -> includes now the grip of the block, width and height of the sprites */      
  /* 4 -> includes level pack num */
  /* 5 -> clean code */
  /* Write CRC32 of XML */
  FS::writeInt_LE(pfh, CACHE_LEVEL_FORMAT_VERSION);
  FS::writeString(pfh	    , m_checkSum);
  FS::writeString(pfh	    , m_id);
  FS::writeString(pfh	    , m_pack);
  FS::writeString(pfh	    , m_packNum);
  FS::writeString(pfh	    , m_name);
  FS::writeString(pfh	    , m_description);
  FS::writeString(pfh	    , m_author);
  FS::writeString(pfh	    , m_date);
  FS::writeString(pfh	    , m_music);
  FS::writeBool(  pfh	    , m_isScripted);
}

bool Level::importBinary(const std::string &FileName, const std::string& pSum) {
  unloadLevelBody();
  bool bRet = true;
  
  m_playerStart = Vector2f(0.0, 0.0);
    
  m_xmotoTooOld = false;
  
  /* Import binary */
  FileHandle *pfh = FS::openIFile(FileName);
  if(pfh == NULL) {
    return false;
  }
  else {
    /* Read tag - it tells something about the format */
    int nFormat = FS::readInt_LE(pfh);
    
    if(nFormat == CACHE_LEVEL_FORMAT_VERSION) { /* reject other formats */
      /* Read "format 1" / "format 2" binary level */
      /* Right */
      std::string md5sum = FS::readString(pfh);
      if(md5sum != pSum) {
        Logger::Log("** Warning ** : CRC check failed, can't import: %s",FileName.c_str());
        bRet = false;
      }
      else {
        /* Read header */
        m_id = FS::readString(pfh);
        m_pack = FS::readString(pfh);
        m_packNum = FS::readString(pfh);
        m_name = FS::readString(pfh);
        m_description = FS::readString(pfh);
        m_author = FS::readString(pfh);
        m_date = FS::readString(pfh);
        m_music = FS::readString(pfh);
	m_isScripted = FS::readBool(pfh);

	/* sky */
        m_sky->setTexture(FS::readString(pfh));
	m_sky->setZoom(FS::readFloat_LE(pfh));
	m_sky->setOffset(FS::readFloat_LE(pfh));

	int v_r, v_g, v_b, v_a;
	v_r = FS::readInt_LE(pfh);
	v_g = FS::readInt_LE(pfh);
	v_b = FS::readInt_LE(pfh);
	v_a = FS::readInt_LE(pfh);
	m_sky->setTextureColor(TColor(v_r, v_g, v_b, v_a));

	m_sky->setDrifted(FS::readBool(pfh));
	m_sky->setDriftZoom(FS::readFloat_LE(pfh));

	v_r = FS::readInt_LE(pfh);
	v_g = FS::readInt_LE(pfh);
	v_b = FS::readInt_LE(pfh);
	v_a = FS::readInt_LE(pfh);
	m_sky->setDriftTextureColor(TColor(v_r, v_g, v_b, v_a));
	/* *** */

        m_borderTexture = FS::readString(pfh);

        m_scriptFileName = FS::readString(pfh);

        m_leftLimit = FS::readFloat_LE(pfh);
        m_rightLimit = FS::readFloat_LE(pfh);
        m_topLimit = FS::readFloat_LE(pfh);
        m_bottomLimit = FS::readFloat_LE(pfh);

	/* theme replacements */
	m_rSpriteForStrawberry 	    = FS::readString(pfh);
	m_rSpriteForFlower     	    = FS::readString(pfh);
	m_rSpriteForWecker     	    = FS::readString(pfh);
	m_rSpriteForStar       	    = FS::readString(pfh);
	m_rSoundForPickUpStrawberry = FS::readString(pfh);

	/* layers */
	m_numberLayer = FS::readInt_LE(pfh);
	for(int i=0; i<m_numberLayer; i++){
	  Vector2f offset;
	  offset.x = FS::readFloat_LE(pfh);
	  offset.y = FS::readFloat_LE(pfh);
	  m_layerOffsets.push_back(offset);
	  m_isLayerFront.push_back(FS::readBool(pfh));
	}

        /* Read embedded script */
        int nScriptSourceLen = FS::readInt_LE(pfh);
        if(nScriptSourceLen > 0) {
          char *pcTemp = new char[nScriptSourceLen+1];
          FS::readBuf(pfh,(char *)pcTemp,nScriptSourceLen);
          pcTemp[nScriptSourceLen]='\0';
            
          m_scriptSource = pcTemp;
            
          delete [] pcTemp;           
        }
        else
          m_scriptSource = "";

        /* Read blocks */
        int nNumBlocks = FS::readInt_LE(pfh);
        m_blocks.reserve(nNumBlocks);
        for(int i=0;i<nNumBlocks;i++) {
          m_blocks.push_back(Block::readFromBinary(pfh));
        }

        /* Read entities */
        int nNumEntities = FS::readInt_LE(pfh);
        m_entities.reserve(nNumEntities);
        for(int i=0;i<nNumEntities;i++) {
          m_entities.push_back(Entity::readFromBinary(pfh));
        }

	try {
	  m_playerStart = getStartEntity()->InitialPosition();
	} catch(Exception &e) {
	  Logger::Log("Warning : no player start entity for level %s", Id().c_str());
	  m_playerStart = Vector2f(0.0, 0.0);
	}

        /* Read zones */
        int nNumZones = FS::readInt_LE(pfh);
        m_zones.reserve(nNumZones);
        for(int i=0;i<nNumZones;i++) {
          m_zones.push_back(Zone::readFromBinary(pfh));
        }                                                                       
      }
    }
    else {
      Logger::Log("** Warning ** : Invalid binary format (%d), can't import: %s",nFormat,FileName.c_str());
      bRet = false;
    }
             
    /* clean up */
    FS::closeFile(pfh);
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

int Level::compareLevelRandom(const Level& i_lvl1, const Level& i_lvl2) {
  return (randomNum(0, 1) < 0.5 ? 1 : -1);
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

int Level::loadToPlay() {
  int v_nbErrors = 0;

  //reset Chipmunk
  //
  ChipmunkHelper::Instance()->reInstance();

  /* preparing blocks */
  for(unsigned int i=0; i<m_blocks.size(); i++) {
    try {
      v_nbErrors += m_blocks[i]->loadToPlay(*m_pCollisionSystem);
    } catch(Exception &e) {
      throw Exception("Fail to load block '" + m_blocks[i]->Id() + "' :\n" + e.getMsg());
    }
  }
  
  /* Spawn initial entities */
  for(unsigned int i=0; i<m_entities.size(); i++) {
    m_entities[i]->loadToPlay();
    Vector2f v = m_entities[i]->DynamicPosition();

    m_pCollisionSystem->addEntity(m_entities[i]);

    if(m_entities[i]->IsToTake()){
      m_nbEntitiesToTake++;
    }
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
  exportBinary(getNameInCache(), m_checkSum);
}

std::string Level::SpriteForStrawberry() const {
  if(m_rSpriteForStrawberry != "") {
    return m_rSpriteForStrawberry;
  }
  return "Strawberry";
}

std::string Level::SpriteForWecker() const {
  if(m_rSpriteForWecker != "") {
    return m_rSpriteForWecker;
  }
  return "Wrecker";
}

std::string Level::SpriteForFlower() const {
  if(m_rSpriteForFlower != "") {
    return m_rSpriteForFlower;
  }
  return "Flower";
}

std::string Level::SpriteForStar() const {
  if(m_rSpriteForStar != "") {
    return m_rSpriteForStar;
  }
  return "Star";
}

std::string Level::SoundForPickUpStrawberry() const {
  if(m_rSoundForPickUpStrawberry != "") {
    return m_rSoundForPickUpStrawberry;
  }
  return "PickUpStrawberry";
}
