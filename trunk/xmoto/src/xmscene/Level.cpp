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

/* 
 *  Level files are stored as XML. The LevelSrc class represents one of these
 *  files and allows on-the-fly changes to everything - useful both in game 
 *  code and in editor code.
 */

#include "Level.h"
#include "../VFileIO.h"
#include "../VXml.h"
#include "../md5sum/md5file.h"
#include "../helpers/Color.h"

#define CACHE_LEVEL_FORMAT_VERSION 6

Level::Level() {
  m_xmotoTooOld = false;
  m_leftLimit   = 0.0;
  m_rightLimit  = 0.0;
  m_topLimit    = 0.0;
  m_bottomLimit = 0.0;
  m_xmlSource = NULL;
}

Level::~Level() {
  unloadToPlay();
  unloadLevelBody();
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
  return (m_scriptFileName != "" || m_scriptSource != "");
}

Block& Level::getBlockById(const std::string& i_id) {
  for(unsigned int i=0; i<m_blocks.size(); i++) {
    if(m_blocks[i]->Id() == i_id) {
      return *(m_blocks[i]);
    }
  }
  throw Exception("Block '" + i_id + "'" + " doesn't exist");
}

Entity& Level::getEntityById(const std::string& i_id) {
  for(unsigned int i=0; i<m_entities.size(); i++) {
    if(m_entities[i]->Id() == i_id) {
      return *(m_entities[i]);
    }
  }
  for(unsigned int i=0; i<m_entitiesDestroyed.size(); i++) {
    if(m_entitiesDestroyed[i]->Id() == i_id) {
      return *(m_entitiesDestroyed[i]);
    }
  }
  for(unsigned int i=0; i<m_entitiesExterns.size(); i++) {
    if(m_entitiesExterns[i]->Id() == i_id) {
      return *(m_entitiesExterns[i]);
    }
  }
  throw Exception("Entity '" + i_id + "'" + " doesn't exist");
}
 
Zone& Level::getZoneById(const std::string& i_id) {
  for(unsigned int i=0; i<m_zones.size(); i++) {
    if(m_zones[i]->Id() == i_id) {
      return *(m_zones[i]);
    }
  }
  throw Exception("Zone '" + i_id + "'" + " doesn't exist");
}

Entity& Level::getStartEntity() {
  for(unsigned int i=0; i<m_entities.size(); i++) {
    if(m_entities[i]->SpriteName() == "PlayerStart") {
      return *(m_entities[i]);
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

void Level::setSky(const std::string& i_sky) {
  m_sky = i_sky;
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

std::string Level::Sky() const {
  return m_sky;
}

void Level::killEntity(const std::string& i_entityId) {
  for(unsigned int i=0; i<m_entities.size(); i++) {
    if(m_entities[i]->Id() == i_entityId) {
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
  unsigned int n = 0;
  
  for(unsigned int i=0; i<m_entities.size(); i++) {
    if(m_entities[i]->IsToTake()) {
      n++;
    }
  }
  return n;
}

void Level::revertEntityDestroyed(const std::string& i_entityId) {
  for(unsigned int i=0; i<m_entitiesDestroyed.size(); i++) {
    if(m_entitiesDestroyed[i]->Id() == i_entityId) {
      m_entities.push_back(m_entitiesDestroyed[i]);
      m_entitiesDestroyed.erase(m_entitiesDestroyed.begin() + i);
      return;
    }
  }
  throw Exception("Entity '" + i_entityId + "' can't be reverted");
}

void Level::updateToTime(vapp::MotoGame& i_scene) {
  bool v_b;

  for(unsigned int i=0;i<Entities().size();i++) {
    v_b = Entities()[i]->updateToTime(i_scene.getTime(), i_scene.getGravity());
  }
  for(unsigned int i=0;i<EntitiesExterns().size();i++) {
    v_b = EntitiesExterns()[i]->updateToTime(i_scene.getTime(), i_scene.getGravity());
  }
}

void Level::saveXML(void) {
  vapp::FileHandle *pfh = vapp::FS::openOFile(m_fileName);
  if(pfh == NULL) {
    /* Failed! */
    vapp::Log("** Warning ** : failed to save level '%s'",m_fileName.c_str());
    return;
  }
    
  vapp::FS::writeLineF(pfh,"<?xml version=\"1.0\" encoding=\"utf-8\"?>");

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
  vapp::FS::writeLineF(pfh, "%s", v_levelTag.c_str());
  
  /* INFO */
  vapp::FS::writeLineF(pfh,"\t<info>");
  vapp::FS::writeLineF(pfh,"\t\t<name>%s</name>",m_name.c_str());
  vapp::FS::writeLineF(pfh,"\t\t<description>%s</description>",m_description.c_str());
  vapp::FS::writeLineF(pfh,"\t\t<author>%s</author>",m_author.c_str());
  vapp::FS::writeLineF(pfh,"\t\t<date>%s</date>",m_date.c_str());
  vapp::FS::writeLineF(pfh,"\t\t<sky>%s</sky>",m_sky.c_str());
  vapp::FS::writeLineF(pfh,"\t</info>");
  
  /* MISC */
  if(m_scriptFileName != "" && m_scriptSource != "") {
    vapp::FS::writeLineF(pfh,"\t<script source=\"%s\">",m_scriptFileName.c_str());
    vapp::FS::writeByte(pfh,'\t'); vapp::FS::writeByte(pfh,'\t');
    
    /* Write script and transform dangerous characters */
    for(unsigned int i=0;i<m_scriptSource.length();i++) {
      int c = m_scriptSource[i];
      if(c == '<') vapp::FS::writeBuf(pfh,"&lt;",4);
      else if(c == '>') vapp::FS::writeBuf(pfh,"&gt;",4);
      else vapp::FS::writeByte(pfh,c);
    }
    //vapp::FS::writeBuf(pfh,(char *)m_ScriptSource.c_str(),m_ScriptSource.length());
    vapp::FS::writeLineF(pfh,"</script>");
  }
  else if(m_scriptFileName != "")
    vapp::FS::writeLineF(pfh,"\t<script source=\"%s\"/>",m_scriptFileName.c_str());
  else if(m_scriptSource != "") {
    vapp::FS::writeLineF(pfh,"\t<script>");
    vapp::FS::writeByte(pfh,'\t'); vapp::FS::writeByte(pfh,'\t');
    /* Write script and transform dangerous characters */
    for(unsigned int i=0;i<m_scriptSource.length();i++) {
      int c = m_scriptSource[i];
      if(c == '<') vapp::FS::writeBuf(pfh,"&lt;",4);
      else if(c == '>') vapp::FS::writeBuf(pfh,"&gt;",4);
      else vapp::FS::writeByte(pfh,c);
    }
    //      vapp::FS::writeBuf(pfh,(char *)m_ScriptSource.c_str(),m_ScriptSource.length());
    vapp::FS::writeLineF(pfh,"</script>");
  }
  
  vapp::FS::writeLineF(pfh,"\t<limits left=\"%f\" right=\"%f\" top=\"%f\" bottom=\"%f\"/>",m_leftLimit,m_rightLimit,m_topLimit,m_bottomLimit);
  
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
  
  vapp::FS::writeLineF(pfh,"\n</level>");           
  vapp::FS::closeFile(pfh);
}

void Level::loadXML(void) {
  /* Load XML document and fetch tinyxml handle */
  unloadLevelBody();
  if(m_xmlSource == NULL) {
    m_xmlSource = new vapp::XMLDocument();
  }

  m_xmlSource->readFromFile( m_fileName, NULL );    
  
  TiXmlDocument *pDoc = m_xmlSource->getLowLevelAccess();
  if(pDoc == NULL) throw Exception("failed to load level XML");
  
  /* Start the fantastic parsing by fetching the <level> element */
  TiXmlElement *pLevelElem = vapp::XML::findElement(*m_xmlSource, NULL,std::string("level"));    
  if(pLevelElem == NULL) throw Exception("<level> tag not found in XML");
  
  /* Get level ID */
  m_id = vapp::XML::getOption(pLevelElem,"id");
  if(m_id == "") throw Exception("no ID specified in level XML");

  /* Get required xmoto version */
  m_requiredVersion = vapp::XML::getOption(pLevelElem,"rversion");
  m_xmotoTooOld = false;
  if(m_id != "") {
    /* Check version */
    if(compareVersionNumbers(vapp::App::getVersionString(),m_requiredVersion) < 0) {
      /* Our version is too low to load this */
      m_xmotoTooOld = true;
      vapp::Log("** Warning ** : Level '%s' requires a newer version (%s) to load!",m_id.c_str(),m_requiredVersion.c_str());
    }
  }
  
  /* Set default info */
  m_name = m_id;
  m_date = "";
  m_description = "";
  m_author = "";
  m_sky = "sky1";
  
  m_scriptFileName = "";
  m_scriptSource = "";
  
  m_bottomLimit = m_leftLimit = -50.0f;
  m_topLimit    = m_rightLimit = 50.0f;
  
  m_playerStart = Vector2f(0.0, 0.0);
  
  if(!m_xmotoTooOld) {    
    /* Get level pack */
    m_pack = vapp::XML::getOption(pLevelElem,"levelpack");
    m_packNum = vapp::XML::getOption(pLevelElem,"levelpackNum");      
    
    /* Get level <info> element */
    TiXmlElement *pInfoElem = vapp::XML::findElement(*m_xmlSource, pLevelElem,std::string("info"));
    if(pInfoElem != NULL) {
      /* Name */
      std::string Tmp = vapp::XML::getElementText(*m_xmlSource, pInfoElem,"name");
      if(Tmp != "") m_name = Tmp;
      
      /* Author */
      Tmp = vapp::XML::getElementText(*m_xmlSource, pInfoElem,"author");
      if(Tmp != "") m_author = Tmp;
      
      /* Description */
      Tmp = vapp::XML::getElementText(*m_xmlSource, pInfoElem,"description");
      if(Tmp != "") m_description = Tmp;
      
      /* Date */
      Tmp = vapp::XML::getElementText(*m_xmlSource, pInfoElem,"date");
      if(Tmp != "") m_date = Tmp;
      
      /* Sky */
      Tmp = vapp::XML::getElementText(*m_xmlSource, pInfoElem,"sky");
      if(Tmp != "") m_sky = Tmp;
    }
    
    /* Get script */
    TiXmlElement *pScriptElem = vapp::XML::findElement(*m_xmlSource, pLevelElem,std::string("script"));
    if(pScriptElem != NULL) {
      /* External script file specified? */
      m_scriptFileName = vapp::XML::getOption(pScriptElem,"source");      
      
      /* Encapsulated script? */
      for(TiXmlNode *pScript=pScriptElem->FirstChild();pScript!=NULL;
          pScript=pScript->NextSibling()) {
        if(pScript->Type() == TiXmlNode::TEXT) {
          m_scriptSource.append(pScript->Value());
        }
      }
    }    
    
    /* Get level limits */
    TiXmlElement *pLimitsElem = vapp::XML::findElement(*m_xmlSource, pLevelElem,std::string("limits"));
    if(pLimitsElem != NULL) {
      m_bottomLimit = atof( vapp::XML::getOption(pLimitsElem,"bottom","-50").c_str() );
      m_leftLimit = atof( vapp::XML::getOption(pLimitsElem,"left","-50").c_str() );
      m_topLimit = atof( vapp::XML::getOption(pLimitsElem,"top","50").c_str() );
      m_rightLimit = atof( vapp::XML::getOption(pLimitsElem,"right","50").c_str() );
    }

    /* Get entities */
    for(TiXmlElement *pElem = pLevelElem->FirstChildElement("entity"); pElem!=NULL; 
        pElem=pElem->NextSiblingElement("entity")) {
      m_entities.push_back(Entity::readFromXml(pElem));
    }    

    try {
      m_playerStart = getStartEntity().InitialPosition();
    } catch(Exception &e) {
      vapp::Log("Warning : no player start entity for level %s", Id().c_str());
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
      m_blocks.push_back(Block::readFromXml(*m_xmlSource, pElem));
    }  
  }
  addLimits();

  delete m_xmlSource;
  m_xmlSource = NULL;

  m_isBodyLoaded = true;
}

/* Load using the best way possible. File name must already be set!
  *  Return whether or not it was loaded from the cache. */
bool Level::loadReducedFromFile(bool cacheEnabled) {
  std::string cacheFileName;

  m_checkSum = md5file(FileName());

  // First try to load it from the cache
  bool cached = false;
  if (cacheEnabled) {
    /* Determine name in cache */
    std::string LevelFileBaseName = vapp::FS::getFileBaseName(FileName());
    cacheFileName = getNameInCache();
    
    try {
      cached = importBinaryHeaderFromFile(cacheFileName, m_checkSum);
    } catch (Exception &e) {
      vapp::Log("** Warning **: Exception while loading binary level, will load "
                "XML instead for '%s' (%s)", FileName().c_str(),
                e.getMsg().c_str());
    }
  }
  
  // If we couldn't get it from the cache, then load from (slow) XML
  if (!cached) {
    loadXML();
    if (cacheEnabled) {
      exportBinary(cacheFileName, m_checkSum); /* Cache it now */
    }
  }
  
  unloadLevelBody(); /* remove body datas */

  return cached;
}

std::string Level::getNameInCache() const {
  return "LCache/" + Checksum() + vapp::FS::getFileBaseName(FileName()) + ".blv";
}

/*===========================================================================
  Export binary level file
  ===========================================================================*/
void Level::exportBinary(const std::string &FileName, const std::string& pSum) {
  /* Don't do this if we failed to load level from XML */
  if(isXMotoTooOld()) return;
  
  /* Export binary... */
  vapp::FileHandle *pfh = vapp::FS::openOFile(FileName);
  if(pfh == NULL) {
    vapp::Log("** Warning ** : Failed to export binary: %s",FileName.c_str());
  }
  else {
    exportBinaryHeader(pfh);

    vapp::FS::writeString(pfh,m_sky);
    vapp::FS::writeString(pfh,m_scriptFileName);
      
    vapp::FS::writeFloat_LE(pfh,m_leftLimit);
    vapp::FS::writeFloat_LE(pfh,m_rightLimit);
    vapp::FS::writeFloat_LE(pfh,m_topLimit);
    vapp::FS::writeFloat_LE(pfh,m_bottomLimit);
      
    /* Write script (if any) */
    vapp::FS::writeInt_LE(pfh,m_scriptSource.length());
    vapp::FS::writeBuf(pfh,(char *)m_scriptSource.c_str(),m_scriptSource.length());
      
    /* Write blocks */
    vapp::FS::writeInt_LE(pfh,m_blocks.size());
    for(unsigned int i=0;i<m_blocks.size();i++) {
      m_blocks[i]->saveBinary(pfh);
    }
      
    /* Write entities */
    vapp::FS::writeInt_LE(pfh,m_entities.size());
    for(unsigned int i=0;i<m_entities.size();i++) {
      m_entities[i]->saveBinary(pfh);
    }  
      
    /* Write zones */
    vapp::FS::writeInt_LE(pfh,m_zones.size());
    for(unsigned int i=0;i<m_zones.size();i++) {
      m_zones[i]->saveBinary(pfh);
    }
                
    /* clean up */
    vapp::FS::closeFile(pfh);
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

void Level::importBinaryHeader(vapp::FileHandle *pfh) {
  unloadLevelBody();

  m_isBodyLoaded = false;  
  m_playerStart  = Vector2f(0.0, 0.0);
  m_xmotoTooOld  = false;

  int nFormat = vapp::FS::readInt_LE(pfh);
  
  if(nFormat != CACHE_LEVEL_FORMAT_VERSION) {
    throw Exception("Old file format");
  }
  
  m_checkSum    = vapp::FS::readString(pfh);
  m_id      	= vapp::FS::readString(pfh);
  m_pack    	= vapp::FS::readString(pfh);
  m_packNum 	= vapp::FS::readString(pfh);
  m_name    	= vapp::FS::readString(pfh);
  m_description = vapp::FS::readString(pfh);
  m_author 	= vapp::FS::readString(pfh);
  m_date   	= vapp::FS::readString(pfh);
}

  /*===========================================================================
  Import binary level file
  ===========================================================================*/
bool Level::importBinaryHeaderFromFile(const std::string &FileName, const std::string& pSum) {
  /* Import binary */
  vapp::FileHandle *pfh = vapp::FS::openIFile(FileName);
  if(pfh == NULL) {
    return false;
  }

  try {
    importBinaryHeader(pfh);
    if(m_checkSum != pSum) {
      vapp::Log("** Warning ** : CRC check failed, can't import: %s",FileName.c_str());
      vapp::FS::closeFile(pfh);
      return false;
    }
  } catch(Exception &e) {
    vapp::FS::closeFile(pfh);
    return false;
  }
             
  /* clean up */
  vapp::FS::closeFile(pfh);

  return true;
}

void Level::exportBinaryHeader(vapp::FileHandle *pfh) {
  /* version two includes dynamic information about blocks */
  /* 3 -> includes now the grip of the block, width and height of the sprites */      
  /* 4 -> includes level pack num */
  /* 5 -> clean code */
  /* Write CRC32 of XML */
  vapp::FS::writeInt_LE(pfh, CACHE_LEVEL_FORMAT_VERSION);
  vapp::FS::writeString(pfh	    , m_checkSum);
  vapp::FS::writeString(pfh	    , m_id);
  vapp::FS::writeString(pfh	    , m_pack);
  vapp::FS::writeString(pfh	    , m_packNum);
  vapp::FS::writeString(pfh	    , m_name);
  vapp::FS::writeString(pfh	    , m_description);
  vapp::FS::writeString(pfh	    , m_author);
  vapp::FS::writeString(pfh	    , m_date);
}

bool Level::importBinary(const std::string &FileName, const std::string& pSum) {
  unloadLevelBody();
  bool bRet = true;
  
  m_playerStart = Vector2f(0.0, 0.0);
    
  m_xmotoTooOld = false;
  
  /* Import binary */
  vapp::FileHandle *pfh = vapp::FS::openIFile(FileName);
  if(pfh == NULL) {
    return false;
  }
  else {
    /* Read tag - it tells something about the format */
    int nFormat = vapp::FS::readInt_LE(pfh);
    
    if(nFormat == CACHE_LEVEL_FORMAT_VERSION) { /* reject other formats */
      /* Read "format 1" / "format 2" binary level */
      /* Right */
      std::string md5sum = vapp::FS::readString(pfh);
      if(md5sum != pSum) {
        vapp::Log("** Warning ** : CRC check failed, can't import: %s",FileName.c_str());
        bRet = false;
      }
      else {
        /* Read header */
        m_id = vapp::FS::readString(pfh);
        m_pack = vapp::FS::readString(pfh);
        m_packNum = vapp::FS::readString(pfh);
        m_name = vapp::FS::readString(pfh);
        m_description = vapp::FS::readString(pfh);
        m_author = vapp::FS::readString(pfh);
        m_date = vapp::FS::readString(pfh);

        m_sky = vapp::FS::readString(pfh);
        m_scriptFileName = vapp::FS::readString(pfh);

        m_leftLimit = vapp::FS::readFloat_LE(pfh);
        m_rightLimit = vapp::FS::readFloat_LE(pfh);
        m_topLimit = vapp::FS::readFloat_LE(pfh);
        m_bottomLimit = vapp::FS::readFloat_LE(pfh);

        /* Read embedded script */
        int nScriptSourceLen = vapp::FS::readInt_LE(pfh);
        if(nScriptSourceLen > 0) {
          char *pcTemp = new char[nScriptSourceLen+1];
          vapp::FS::readBuf(pfh,(char *)pcTemp,nScriptSourceLen);
          pcTemp[nScriptSourceLen]='\0';
            
          m_scriptSource = pcTemp;
            
          delete [] pcTemp;           
        }
        else
          m_scriptSource = "";

        /* Read blocks */
        int nNumBlocks = vapp::FS::readInt_LE(pfh);
        m_blocks.reserve(nNumBlocks);
        for(int i=0;i<nNumBlocks;i++) {
          m_blocks.push_back(Block::readFromBinary(pfh));
        }

        /* Read entities */
        int nNumEntities = vapp::FS::readInt_LE(pfh);
        m_entities.reserve(nNumEntities);
        for(int i=0;i<nNumEntities;i++) {
          m_entities.push_back(Entity::readFromBinary(pfh));
        }

	try {
	  m_playerStart = getStartEntity().InitialPosition();
	} catch(Exception &e) {
	  vapp::Log("Warning : no player start entity for level %s", Id().c_str());
	  m_playerStart = Vector2f(0.0, 0.0);
	}

        /* Read zones */
        int nNumZones = vapp::FS::readInt_LE(pfh);
        m_zones.reserve(nNumZones);
        for(int i=0;i<nNumZones;i++) {
          m_zones.push_back(Zone::readFromBinary(pfh));
        }                                                                       
      }
    }
    else {
      vapp::Log("** Warning ** : Invalid binary format (%d), can't import: %s",nFormat,FileName.c_str());
      bRet = false;
    }
             
    /* clean up */
    vapp::FS::closeFile(pfh);
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

int Level::loadToPlay(vapp::CollisionSystem& p_CollisionSystem) {
  int v_nbErrors = 0;

  /* preparing blocks */
  for(unsigned int i=0; i<m_blocks.size(); i++) {
    try {
      v_nbErrors += m_blocks[i]->loadToPlay(p_CollisionSystem);
    } catch(Exception &e) {
      throw Exception("Fail to load block '" + m_blocks[i]->Id() + "' :\n" + e.getMsg());
    }
  }
  
  /* Spawn initial entities */
  for(unsigned int i=0; i<m_entities.size(); i++) {
    m_entities[i]->loadToPlay();
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
}

void Level::addLimits() {
  Block *pBlock;
  Vector2f v_P;

  /* Create level surroundings (by limits) */    
  float fVMargin = 20,fHMargin = 20;

  pBlock = new Block("LEVEL_TOP");
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit()  - fHMargin, TopLimit() + fVMargin)));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit() + fHMargin, TopLimit() + fVMargin)));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit()           , TopLimit())));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit()            , TopLimit())));
  Blocks().push_back(pBlock);
    
  pBlock = new Block("LEVEL_BOTTOM");
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit()           , BottomLimit())));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit() + fHMargin, BottomLimit() - fVMargin)));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit()  - fHMargin, BottomLimit() - fVMargin)));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit()            , BottomLimit())));
  Blocks().push_back(pBlock);

  pBlock = new Block("LEVEL_LEFT");
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit(), TopLimit())));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit(), BottomLimit())));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit() - fHMargin, BottomLimit() - fVMargin)));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(LeftLimit() - fHMargin, TopLimit() + fVMargin))); 
  Blocks().push_back(pBlock);
    
  pBlock = new Block("LEVEL_RIGHT");
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit(), TopLimit())));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit() + fHMargin, TopLimit() + fVMargin)));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit() + fHMargin, BottomLimit() - fVMargin)));
  pBlock->Vertices().push_back(new BlockVertex(Vector2f(RightLimit(), BottomLimit())));
  Blocks().push_back(pBlock);
}

void Level::clearAfterRewind() {
  for(unsigned int i=0; i<m_entities.size(); i++) {
    m_entities[i]->clearAfterRewind();
  } 
}

void Level::spawnEntity(Entity *v_entity) {
  m_entitiesExterns.push_back(v_entity);
}

std::vector<Entity *>& Level::EntitiesExterns() {
  return m_entitiesExterns;
}

std::string Level::PathForUpdate() const {
  /* If level path is not absolute, this level is not updatable */
  if(vapp::FS::isPathAbsolute(FileName())) {
    return FileName();
  }
  return "";
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
}

void Level::rebuildCache() {
  exportBinary(getNameInCache(), m_checkSum);
}
