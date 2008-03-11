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

#include "LevelsManager.h"
#include "VXml.h"
#include "GameText.h"
#include <algorithm> 

#define CURRENT_LEVEL_INDEX_FILE_VERSION 1

LevelsPack::LevelsPack(std::string i_name) {
  m_name         = i_name;
  m_showTimes    = true;
  m_showWebTimes = true;
  m_sorted       = true;
  setHintsFromFile();
}

LevelsPack::~LevelsPack() {
}

void LevelsPack::setHintsFromFile() {
  std::vector<std::string> LpkFiles = vapp::FS::findPhysFiles("Levels/*.lpk", true);

  for(int i=0; i<LpkFiles.size(); i++) {
    vapp::XMLDocument XML; 
    XML.readFromFile(LpkFiles[i]);
    TiXmlDocument *pDoc = XML.getLowLevelAccess();
          
    if(pDoc != NULL) {
      TiXmlElement *pLpkHintsElem = pDoc->FirstChildElement("lpkhints");
      if(pLpkHintsElem != NULL) {
	/* For this level pack? */
	const char *pcFor = pLpkHintsElem->Attribute("for");
	if(pcFor != NULL && m_name == pcFor) {
	  /* Yup. Extract hints */
	  for(TiXmlElement *pHintElem = pLpkHintsElem->FirstChildElement("hint");
	      pHintElem != NULL; pHintElem=pHintElem->NextSiblingElement("hint")) {
	    /* Check for known hints... */
	    const char *pc;
	    
	    pc = pHintElem->Attribute("show_times");
	    if(pc != NULL) {
	      m_showTimes = atoi(pc)==1;
	    }
	    
	    pc = pHintElem->Attribute("show_wtimes");
	    if(pc != NULL) {
	      m_showWebTimes = atoi(pc)==1;
	    }
	  }
	}              
      }
    }
  }
}

std::string LevelsPack::Name() const {
  return m_name;
}

void LevelsPack::addLevel(Level *i_level) {
  m_levels.push_back(i_level);
}

const std::vector<Level *>& LevelsPack::Levels() {
  return m_levels;
}

bool LevelsPack::ShowTimes() const {
  return m_showTimes;
}

bool LevelsPack::ShowWebTimes() const {
  return m_showWebTimes;
}

void LevelsPack::setShowTimes(bool i_showTimes) {
  m_showTimes = i_showTimes;
}

void LevelsPack::setShowWebTimes(bool i_showWebTimes) {
  m_showWebTimes = i_showWebTimes;
}

bool LevelsPack::Sorted() const {
  return m_sorted;
}

void LevelsPack::setSorted(bool i_sorted) {
  m_sorted = i_sorted;
}

LevelsManager::LevelsManager() {
}

LevelsManager::~LevelsManager() {
  clean();
}
 
void LevelsManager::clean() {
  m_newLevels.clear();
  m_updatedLevels.clear();
  m_favoritesLevels.clear();

  /* delete packs */
  for(int i=0;i<m_levelsPacks.size();i++) {
    delete m_levelsPacks[i];
  }
  m_levelsPacks.clear();

  /* delete levels */
  for(unsigned int i=0; i<m_levels.size(); i++) {
    delete m_levels[i];
  }
  m_levels.clear();
}

LevelsPack& LevelsManager::LevelsPackByName(const std::string &i_name) {
  for(int i=0; i<m_levelsPacks.size(); i++) {
    if(m_levelsPacks[i]->Name() == i_name) {
      return *m_levelsPacks[i];
    }
  }
  throw Exception(std::string("Level pack " + i_name + " not found").c_str());
}

bool LevelsManager::doesLevelsPackExist(const std::string &i_name) const {
  for(int i=0; i<m_levelsPacks.size(); i++) {
    if(m_levelsPacks[i]->Name() == i_name) {
      return true;
    }
  }
  return false;
}

void LevelsManager::rebuildPacks(WebRoom *i_webHighscores,
				 std::string i_playerName,
				 vapp::PlayerData *i_profiles,
				 vapp::Stats *i_stats) {
  /* delete the packs */
  for(int i=0;i<m_levelsPacks.size();i++) {
    delete m_levelsPacks[i];
  }
  m_levelsPacks.clear();

  /* reparse levels to build them */
  for(unsigned int i=0; i<m_levels.size(); i++) {
    if(doesLevelsPackExist(m_levels[i]->Pack())) {
      LevelsPackByName(m_levels[i]->Pack()).addLevel(m_levels[i]);
    } else {
      LevelsPack *v_levelsPack = new LevelsPack(m_levels[i]->Pack());
      v_levelsPack->addLevel(m_levels[i]);
      m_levelsPacks.push_back(v_levelsPack);
    }
  }

  createVirtualPacks(i_webHighscores, i_playerName, i_profiles, i_stats);
}

void LevelsManager::createVirtualPacks(WebRoom *i_webHighscores, std::string i_playerName, vapp::PlayerData *i_profiles, vapp::Stats *i_stats) {
  LevelsPack *v_pack;
  
  /* levels with no highscore */
  if(i_webHighscores != NULL) {
    v_pack = new LevelsPack("~ " + std::string(VPACKAGENAME_LEVELS_WITH_NO_HIGHSCORE));
    m_levelsPacks.push_back(v_pack);
    for(unsigned int i=0; i<m_levels.size(); i++) {
      WebHighscore* wh = i_webHighscores->getHighscoreFromLevel(m_levels[i]->Id());
      if(wh == NULL) {
	v_pack->addLevel(m_levels[i]);
      }
    }
  }

  /* levels i've not the highscore */
  v_pack = new LevelsPack("~ " + std::string(VPACKAGENAME_YOU_HAVE_NOT_THE_HIGHSCORE));
  m_levelsPacks.push_back(v_pack);
  for(unsigned int i=0; i<m_levels.size(); i++) {
    if(i_webHighscores != NULL) {
      WebHighscore* wh = i_webHighscores->getHighscoreFromLevel(m_levels[i]->Id());
      if(wh != NULL) {
	if(wh->getPlayerName() != i_playerName) {
	  v_pack->addLevel(m_levels[i]);
	}
      }
    }
  }

  /* levels i've not the highscore */
  //v_pack = new LevelsPack("~ " + std::string(VPACKAGENAME_RANDOM_LEVELS));
  //v_pack->setSorted(false);
  //m_levelsPacks.push_back(v_pack);
  //for(unsigned int i=0; i<m_levels.size(); i++) {
  //  v_pack->addLevel(m_levels[i]);
  //}

  /* levels i've not finished */
  v_pack = new LevelsPack("~ " + std::string(VPACKAGENAME_INCOMPLETED_LEVELS));
  m_levelsPacks.push_back(v_pack);
  for(unsigned int i=0; i<m_levels.size(); i++) {
    if(i_profiles->isLevelCompleted(i_playerName, m_levels[i]->Id()) == false) {
      v_pack->addLevel(m_levels[i]);
    }
  }

  /* all levels */
  v_pack = new LevelsPack("~ " + std::string(VPACKAGENAME_ALL_LEVELS));
  m_levelsPacks.push_back(v_pack);
  for(unsigned int i=0; i<m_levels.size(); i++) {
    v_pack->addLevel(m_levels[i]);
  }

  /* favorites levels */
  v_pack = new LevelsPack("~ " + std::string(VPACKAGENAME_FAVORITES_LEVELS));
  m_levelsPacks.push_back(v_pack);
  for(unsigned int i=0; i<m_favoritesLevels.size(); i++) {
    v_pack->addLevel(m_favoritesLevels[i]);
  }

  /* new levels */
  v_pack = new LevelsPack("~ " + std::string(VPACKAGENAME_NEW_LEVELS));
  m_levelsPacks.push_back(v_pack);
  for(unsigned int i=0; i<m_newLevels.size(); i++) {
    v_pack->addLevel(m_newLevels[i]);
  }

  /* updated levels */
  v_pack = new LevelsPack("~ " + std::string(VPACKAGENAME_UPDATED_LEVELS));
  m_levelsPacks.push_back(v_pack);
  for(unsigned int i=0; i<m_updatedLevels.size(); i++) {
    v_pack->addLevel(m_updatedLevels[i]);
  }

  /* most played levels */
  // don't work
  //vapp::PlayerStats *v_playerStats = i_stats->_FindPlayerStats(i_playerName);
  //if(v_playerStats != NULL) {
  //  v_pack = new LevelsPack("~ " + std::string(VPACKAGENAME_MOST_PLAYED_LEVELS));
  //  v_pack->setSorted(false);
  //  m_levelsPacks.push_back(v_pack);
  //
  //  // bad sort !!! n2
  //  for(unsigned int i=0; i<m_levels.size(); i++) {
  //    for(unsigned int j=0; j<m_levels.size()-1; j++) {
  //	if(vapp::Stats::compareLevelMostPlayed(*(m_levels[i]), *(m_levels[j]), v_playerStats) != 1) {
  //	  Level *tmp = m_levels[i];
  //	  m_levels[i] = m_levels[j];
  //	  m_levels[j] = tmp;
  //	}
  //    }
  //  }
  //
  //  for(unsigned int i=0; (i<m_levels.size() && i<=20); i++) {
  //    v_pack->addLevel(m_levels[i]);
  //  }
  //}

}

const std::vector<Level *>& LevelsManager::Levels() {
  return m_levels;
}

const std::vector<LevelsPack *>& LevelsManager::LevelsPacks() {
  return m_levelsPacks;
}

void LevelsManager::reloadLevelsFromFiles(bool i_enableCache) {
  clean();

  std::vector<std::string> LvlFiles = vapp::FS::findPhysFiles("Levels/*.lvl", true);
  loadLevelsFromLvl(LvlFiles, i_enableCache);
  try {
    /* then, recreate the index */
    createLevelsIndex();
  } catch(Exception &e2) {
    vapp::Log((std::string("Unable to create the level index:\n") + e2.getMsg()).c_str());
  }

  try {
    loadNewLevelsXml();
  } catch(Exception &e) {
    vapp::Log("** Warning : Unable to load NewLevels xml file");
  }
  try {
    loadFavoritesLevelsXml();
  } catch(Exception &e) {
    vapp::Log("** Warning : Unable to load FavoritesLevels xml file");
  }
}

std::string LevelsManager::LevelIndexFileName() {
  return vapp::FS::getUserDir() + "/" + "LCache/levels.index";
}

void LevelsManager::loadLevelsFromIndex() {
  int v_nbLevels;

  clean();

  vapp::FileHandle *pfh = vapp::FS::openIFile(LevelIndexFileName());
  if(pfh == NULL) {
    throw Exception((std::string("Unable to open file ") + LevelIndexFileName()).c_str());
  }

  try {
    int v_version = vapp::FS::readInt_LE(pfh); /* version */
    if(v_version != CURRENT_LEVEL_INDEX_FILE_VERSION) {
      throw Exception("Invalid level index file version");
    }
    v_nbLevels = vapp::FS::readInt_LE(pfh);
    for(int i=0; i<v_nbLevels; i++) {
      Level *v_level = new Level();
      try {
	v_level->setFileName(vapp::FS::readString(pfh));
	  v_level->importBinaryHeader(pfh);
	  m_levels.push_back(v_level);
      } catch(Exception &e) {
	delete v_level;
      }
    }

  } catch(Exception &e) {
    clean();
    vapp::FS::closeFile(pfh);
    throw e;
  }
  vapp::FS::closeFile(pfh);

  try {
    loadNewLevelsXml();
  } catch(Exception &e) {
    vapp::Log("** Warning : Unable to load NewLevels xml file");
  }

  try {
    loadFavoritesLevelsXml();
  } catch(Exception &e) {
    vapp::Log("** Warning : Unable to load FavoritesLevels xml file");
  }

}
   
void LevelsManager::createLevelsIndex() {
  /* for windows : your must remove the file before create it */
  remove(LevelIndexFileName().c_str());

  vapp::FileHandle *pfh = vapp::FS::openOFile(LevelIndexFileName());
  if(pfh == NULL) {
    throw Exception((std::string("Unable to open file ") + LevelIndexFileName()).c_str());
    return;
  }

  try {
    vapp::FS::writeInt_LE(pfh, CURRENT_LEVEL_INDEX_FILE_VERSION); /* version */
    vapp::FS::writeInt_LE(pfh, m_levels.size());
    for(int i=0; i<m_levels.size(); i++) {
      vapp::FS::writeString(pfh, m_levels[i]->FileName());
      m_levels[i]->exportBinaryHeader(pfh);
    }
  } catch(Exception &e) {
    vapp::FS::closeFile(pfh);
    throw e;
  }
  
  vapp::FS::closeFile(pfh);
}

void LevelsManager::deleteLevelsIndex() {
  remove(LevelIndexFileName().c_str());
}

void LevelsManager::loadLevelsFromLvl(const std::vector<std::string> &LvlFiles, bool i_enableCache) {
  loadLevelsFromLvl(LvlFiles, i_enableCache, false);
}

void LevelsManager::loadLevelsFromLvl(const std::vector<std::string> &LvlFiles, bool i_enableCache, bool i_newLevels) {
  for(int i=0;i<LvlFiles.size();i++) {
    bool bCached = false;
    Level *v_level = new Level();
    
    try {
      v_level->setFileName(LvlFiles[i]);
      bCached = v_level->loadReducedFromFile(i_enableCache);
      
      // Check for ID conflict
      for(int k=0; k<m_levels.size(); k++) {
	if(m_levels[k]->Id() == v_level->Id()) {
	  /* Conflict! */
	  vapp::Log("** Warning ** : More than one level with ID '%s'!",m_levels[k]->Id().c_str());
	  vapp::Log("                (%s)", v_level->FileName().c_str());
	  vapp::Log("                (%s)", m_levels[k]->FileName().c_str());
	  if(bCached) vapp::Log("                (cached)");
	  throw Exception("Duplicate level ID");
	}
      }
      m_levels.push_back(v_level);
      if(i_newLevels) {
	m_newLevels.push_back(v_level);
      }
    } catch(Exception &e) {
      delete v_level;
      vapp::Log("** Warning ** : Problem loading '%s' (%s)",
	  LvlFiles[i].c_str(),e.getMsg().c_str());            
    }
  }
}

void LevelsManager::checkPrerequires(bool &v_enableCache) {
  std::string LCachePath = vapp::FS::getUserDir() + std::string("/LCache");

  if(v_enableCache && vapp::FS::isDir(LCachePath) == false) {
    try {
      vapp::FS::mkArborescence(LCachePath);
    } catch(Exception &e) {
      v_enableCache = false;
      vapp::Log("** Warning ** : Level cache directory can't be created, forcing caching off!");
    }
  }
}

void LevelsManager::cleanCache() {
  /* Find all .blv-files in the directory */
  std::vector<std::string> BlvFiles = vapp::FS::findPhysFiles("LCache/*.blv");
  for(int i=0; i<BlvFiles.size(); i++) {
    vapp::FS::deleteFile(BlvFiles[i]);
  }
}

Level& LevelsManager::LevelById(const std::string& i_id) {
  for(unsigned int i=0; i<m_levels.size(); i++) {
    if(m_levels[i]->Id() == i_id) {
      return *(m_levels[i]);
    }
  }
  throw Exception(std::string("Level " + i_id + " doesn't exist").c_str());
}

Level& LevelsManager::LevelByFileName(const std::string& i_fileName) {
  for(unsigned int i=0; i<m_levels.size(); i++) {
    if(m_levels[i]->FileName() == i_fileName) {
      return *(m_levels[i]);
    }
  }
  throw Exception(std::string("Level " + i_fileName + " doesn't exist").c_str());
}

bool LevelsManager::doesLevelExist(const std::string& i_id) {
  for(unsigned int i=0; i<m_levels.size(); i++) {
    if(m_levels[i]->Id() == i_id) {
      return true;
    }
  }
  return false;
}

void LevelsManager::printLevelsList() const {
  printf("%-40s %-40s\n", "Id", "Name");
  for(int i=0; i<m_levels.size(); i++) {          
    printf("%-40s %-40s\n",
	   m_levels[i]->Id().c_str(),
	   m_levels[i]->Name().c_str()
	   );
  }
}

const std::vector<Level *>& LevelsManager::NewLevels() {
  return m_newLevels;
}

const std::vector<Level *>& LevelsManager::UpdatedLevels() {
  return m_updatedLevels;
}

void LevelsManager::updateLevelsFromLvl(const std::vector<std::string> &NewLvl,
					const std::vector<std::string> &UpdatedLvlFileNames,
					bool i_enableCache) {
  m_newLevels.clear();
  m_updatedLevels.clear();

  loadLevelsFromLvl(NewLvl, i_enableCache, true);  

  Level *v_level;

  for(int i=0;i<UpdatedLvlFileNames.size();i++) {
    try {
      v_level = &(LevelByFileName(UpdatedLvlFileNames[i]));
      v_level->loadReducedFromFile(i_enableCache);
      m_updatedLevels.push_back(v_level);
    } catch(Exception &e) {
      vapp::Log("** Warning ** : Problem updating '%s' (%s)", UpdatedLvlFileNames[i].c_str(), e.getMsg().c_str());
    }
  }
}

std::string LevelsManager::NewLevelsXmlFilePath() {
  return vapp::FS::getUserDir() + "/" + "newLevels.xml";
}

void LevelsManager::saveNewLevelsXml() const {
  remove(NewLevelsXmlFilePath().c_str());

  vapp::FileHandle *pfh = vapp::FS::openOFile(NewLevelsXmlFilePath());
  if(pfh == NULL) {
    vapp::Log("** Warning ** : failed to open '%s'", NewLevelsXmlFilePath().c_str());
    return;
  }

  vapp::FS::writeLineF(pfh,"<?xml version=\"1.0\" encoding=\"utf-8\"?>");
  vapp::FS::writeLineF(pfh,"<newLevels>");  

  for(int i=0; i<m_newLevels.size(); i++) {
    vapp::FS::writeLineF(pfh, "<level id=\"%s\" />", vapp::XML::str2xmlstr(m_newLevels[i]->Id()).c_str());
  }
  for(int i=0; i<m_updatedLevels.size(); i++) {
    vapp::FS::writeLineF(pfh, "<level id=\"%s\" updated=\"true\" />", vapp::XML::str2xmlstr(m_updatedLevels[i]->Id()).c_str());
  }

  vapp::FS::writeLineF(pfh,"</newLevels>");  
  vapp::FS::closeFile(pfh);
}

void LevelsManager::loadNewLevelsXml() {
  m_newLevels.clear();
  m_updatedLevels.clear();

  vapp::XMLDocument v_newLevelsXml;
  TiXmlDocument *v_newLevelsXmlData;
  TiXmlElement *v_newLevelsXmlDataElement;
  const char *pc;
  std::string v_levelId, v_updated;
  
  v_newLevelsXml.readFromFile(NewLevelsXmlFilePath());
  v_newLevelsXmlData = v_newLevelsXml.getLowLevelAccess();

  if(v_newLevelsXmlData == NULL) {
    throw Exception("error : unable to analyze xml newLevels file");
  }

  v_newLevelsXmlDataElement = v_newLevelsXmlData->FirstChildElement("newLevels");
    
  if(v_newLevelsXmlDataElement == NULL) {
    throw Exception("error : unable to analyze xml newLevels file");
  }
    
  TiXmlElement *pVarElem = v_newLevelsXmlDataElement->FirstChildElement("level");
  while(pVarElem != NULL) {
    v_levelId = "";
    v_updated = "";

    pc = pVarElem->Attribute("id");
    if(pc != NULL) {
      v_levelId = pc;
  
      pc = pVarElem->Attribute("updated");
      if(pc != NULL) {
	v_updated = pc;   
      }

      /* add the level into the list */
      if(v_updated == "true") {
	m_updatedLevels.push_back(&(LevelById(v_levelId)));
      } else {
	m_newLevels.push_back(&(LevelById(v_levelId)));
      }
    }
    pVarElem = pVarElem->NextSiblingElement("level");
  }
}

const std::vector<Level *>& LevelsManager::FavoritesLevels() {
  return m_favoritesLevels;
}

std::string LevelsManager::FavoritesLevelsXmlFilePath() {
  return vapp::FS::getUserDir() + "/" + "favoritesLevels.xml";
}

void LevelsManager::addToFavorites(Level *i_level) {
  /* check if the level is already into the favorites list */
  for(int i=0; i<m_favoritesLevels.size(); i++) {
    if(m_favoritesLevels[i] == i_level) {
      return;
    }
  }
  m_favoritesLevels.push_back(i_level);
}

void LevelsManager::delFromFavorites(Level *i_level) {
  for(int i=0; i<m_favoritesLevels.size(); i++) {
    if(m_favoritesLevels[i] == i_level) {
      m_favoritesLevels.erase(m_favoritesLevels.begin() + i);
      return;
    }
  }
}

void LevelsManager::saveFavoritesLevelsXml() const {
  remove(FavoritesLevelsXmlFilePath().c_str());

  vapp::FileHandle *pfh = vapp::FS::openOFile(FavoritesLevelsXmlFilePath());
  if(pfh == NULL) {
    vapp::Log("** Warning ** : failed to open '%s'", FavoritesLevelsXmlFilePath().c_str());
    return;
  }

  vapp::FS::writeLineF(pfh,"<?xml version=\"1.0\" encoding=\"utf-8\"?>");
  vapp::FS::writeLineF(pfh,"<favoritesLevels>");  

  for(int i=0; i<m_favoritesLevels.size(); i++) {
    vapp::FS::writeLineF(pfh, "<level id=\"%s\" />", vapp::XML::str2xmlstr(m_favoritesLevels[i]->Id()).c_str());
  }

  vapp::FS::writeLineF(pfh,"</favoritesLevels>");  
  vapp::FS::closeFile(pfh);
}

void LevelsManager::loadFavoritesLevelsXml() {
  m_favoritesLevels.clear();

  vapp::XMLDocument v_favoritesLevelsXml;
  TiXmlDocument *v_favoritesLevelsXmlData;
  TiXmlElement *v_favoritesLevelsXmlDataElement;
  const char *pc;
  std::string v_levelId;
  
  v_favoritesLevelsXml.readFromFile(FavoritesLevelsXmlFilePath());
  v_favoritesLevelsXmlData = v_favoritesLevelsXml.getLowLevelAccess();

  if(v_favoritesLevelsXmlData == NULL) {
    throw Exception("error : unable to analyze xml favoritesLevels file");
  }

  v_favoritesLevelsXmlDataElement = v_favoritesLevelsXmlData->FirstChildElement("favoritesLevels");
  
  if(v_favoritesLevelsXmlDataElement == NULL) {
    throw Exception("error : unable to analyze xml favoritesLevels file");
  }
    
  TiXmlElement *pVarElem = v_favoritesLevelsXmlDataElement->FirstChildElement("level");
  while(pVarElem != NULL) {
    v_levelId = "";

    pc = pVarElem->Attribute("id");
    if(pc != NULL) {
      v_levelId = pc;
  
      /* add the level into the list */
      m_favoritesLevels.push_back(&(LevelById(v_levelId)));
    }
    pVarElem = pVarElem->NextSiblingElement("level");
  }
}

void LevelsManager::saveXml() const {
  saveFavoritesLevelsXml();
  saveNewLevelsXml();
}