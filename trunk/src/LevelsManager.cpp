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
#include <time.h>

// increase the version to force reloading of levels
#define CURRENT_LEVEL_INDEX_FILE_VERSION 3

#define VPACKAGENAME_CRAPIEST_MAX 1.5
#define VPACKAGENAME_NICEST_MIN   4.5
#define VPACKAGENAME_EASIEST_MAX  1.5
#define VPACKAGENAME_HARDEST_MIN  4.5

#define STATS_PACK_NUMBER_OF_LEVELS 50

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

std::string LevelsPack::Group() const {
  return m_group;
}

void LevelsPack::setGroup(std::string i_group) {
  m_group = i_group;
}

void LevelsPack::addLevel(Level *i_level, bool i_checkUnique) {
	if(i_checkUnique) {
		/* check if the level is already into the pack */
		for(int i=0; i<m_levels.size(); i++) {
			if(m_levels[i] == i_level) {
				return;
			}
		}
	}
  m_levels.push_back(i_level);
}

void LevelsPack::removeLevel(Level *i_level) {
  for(int i=0; i<m_levels.size(); i++) {
    if(m_levels[i] == i_level) {
      m_levels.erase(m_levels.begin() + i);
      return;
    }
  }
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
  m_favoriteLevels.clear();

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
  m_externalLevels.clear();
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

void LevelsManager::rebuildPacks(
#if defined(SUPPORT_WEBACCESS)
				 WebRoom *i_webHighscores,
				 WebLevels *i_webLevels,
#endif
				 bool i_bDebugMode,
				 std::string i_playerName,
				 vapp::PlayerData *i_profiles,
				 xmDatabase *m_db) {
  /* delete the packs */
  for(int i=0;i<m_levelsPacks.size();i++) {
    delete m_levelsPacks[i];
  }
  m_levelsPacks.clear();

  createVirtualPacks(
#if defined(SUPPORT_WEBACCESS)
		     i_webHighscores,
		     i_webLevels,
#endif
		     i_bDebugMode,
		     i_playerName, i_profiles, m_db);
}

void LevelsManager::createVirtualPacks(
#if defined(SUPPORT_WEBACCESS)
				       WebRoom *i_webHighscores,
				       WebLevels *i_webLevels,
#endif
				       bool i_bDebugMode,
				       std::string i_playerName, vapp::PlayerData *i_profiles,
				       xmDatabase *m_db) {
  LevelsPack *v_pack;
  
  /* levels i've not finished */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_INCOMPLETED_LEVELS));
  m_levelsPacks.push_back(v_pack);
  v_pack->setGroup(GAMETEXT_PACK_SPECIAL);
  for(unsigned int i=0; i<m_levels.size(); i++) {
    if(i_profiles->isLevelCompleted(i_playerName, m_levels[i]->Id()) == false) {
      v_pack->addLevel(m_levels[i]);
    }
  }

  /* all levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_ALL_LEVELS));
  m_levelsPacks.push_back(v_pack);
  v_pack->setGroup(GAMETEXT_PACK_SPECIAL);
  for(unsigned int i=0; i<m_levels.size(); i++) {
    v_pack->addLevel(m_levels[i]);
  }

  /* scripted levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_SCRIPTED));
  m_levelsPacks.push_back(v_pack);
  v_pack->setGroup(GAMETEXT_PACK_SPECIAL);
  for(unsigned int i=0; i<m_levels.size(); i++) {
    if(m_levels[i]->isScripted()) {
      v_pack->addLevel(m_levels[i]);
    }
  }

  /* musical */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_MUSICAL));
  m_levelsPacks.push_back(v_pack);
  v_pack->setGroup(GAMETEXT_PACK_SPECIAL);
  for(unsigned int i=0; i<m_levels.size(); i++) {
    if(m_levels[i]->Music() != "") {
      v_pack->addLevel(m_levels[i]);
    }
  }

  /* favorite levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_FAVORITE_LEVELS));
  m_levelsPacks.push_back(v_pack);
  v_pack->setGroup(GAMETEXT_PACK_SPECIAL);
  for(unsigned int i=0; i<m_favoriteLevels.size(); i++) {
    v_pack->addLevel(m_favoriteLevels[i]);
  }

#if defined(SUPPORT_WEBACCESS)
  /* levels with no highscore */
  if(i_webHighscores != NULL) {
    v_pack = new LevelsPack(std::string(VPACKAGENAME_LEVELS_WITH_NO_HIGHSCORE));
    v_pack->setGroup(GAMETEXT_PACK_ROOM);
    m_levelsPacks.push_back(v_pack);
    for(unsigned int i=0; i<m_levels.size(); i++) {
      WebHighscore* wh = i_webHighscores->getHighscoreFromLevel(m_levels[i]->Id());
      if(wh == NULL) {
	v_pack->addLevel(m_levels[i]);
      }
    }
  }
#endif

#if defined(SUPPORT_WEBACCESS)
  /* levels i've not the highscore */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_YOU_HAVE_NOT_THE_HIGHSCORE));
  m_levelsPacks.push_back(v_pack);
  v_pack->setGroup(GAMETEXT_PACK_ROOM);
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
#endif

#if defined(SUPPORT_WEBACCESS)
//  /* levels i've not the highscore */
//  v_pack = new LevelsPack(std::string("Last highscores"));
//  m_levelsPacks.push_back(v_pack);
//  v_pack->setGroup(GAMETEXT_PACK_ROOM);
//  for(unsigned int i=0; i<m_levels.size(); i++) {
//    if(i_webHighscores != NULL) {
//      WebHighscore* wh = i_webHighscores->getHighscoreFromLevel(m_levels[i]->Id());
//      if(wh != NULL) {
//	if(wh->getPlayerName() != i_playerName) {
//	  v_pack->addLevel(m_levels[i]);
//	}
//      }
//    }
//  }
#endif

  /* stats */
/////  vapp::PlayerStats *v_playerStats = i_stats->_FindPlayerStats(i_playerName);
/////
/////  /* never played levels */
/////  if(v_playerStats != NULL) {
/////    v_pack = new LevelsPack(std::string(VPACKAGENAME_NEVER_PLAYED));
/////    v_pack->setGroup(GAMETEXT_PACK_STATS);
/////    m_levelsPacks.push_back(v_pack);
/////    std::vector<vapp::LevelStats *> v_levelsStats = v_playerStats->Levels;
/////
/////    for(unsigned int i=0; i<m_levels.size(); i++) {
/////      bool found = false;
/////      for(unsigned int j=0; j<v_levelsStats.size(); j++) {
/////	if(m_levels[i]->Id() == v_levelsStats[j]->LevelID) {
/////	  found = true;
/////	}
/////      }
/////      if(found == false) {
/////	v_pack->addLevel(m_levels[i]);
/////      }
/////    }
/////  }
/////
/////  /* most played levels */
/////  if(v_playerStats != NULL) {
/////    v_pack = new LevelsPack(std::string(VPACKAGENAME_MOST_PLAYED));
/////    v_pack->setGroup(GAMETEXT_PACK_STATS);
/////    m_levelsPacks.push_back(v_pack);
/////    std::vector<vapp::LevelStats *> v_levelsStats = v_playerStats->Levels;
/////
/////    vapp::LevelStats* v_most_played_levels[STATS_PACK_NUMBER_OF_LEVELS];
/////    for(int i=0; i<STATS_PACK_NUMBER_OF_LEVELS; i++) {
/////      v_most_played_levels[i] = NULL;
/////    }
/////    for(unsigned int i=0; i<v_levelsStats.size(); i++) {
/////      int best_position = -1;
/////      for(int j=STATS_PACK_NUMBER_OF_LEVELS-1; j>=0; j--) {
/////    	if(v_most_played_levels[j] == NULL) {
/////    	  best_position = j; /* take a free position */
/////    	} else {
/////    	  /* if there is no free position, take the place if it's better */
/////    	  if(v_levelsStats[i]->nPlayed > v_most_played_levels[j]->nPlayed) {
/////    	    /* ok, take the position if the current best position is not better */
/////    	    if(best_position == -1) { /* no position already set */
/////    	      best_position = j;
/////    	    } else {
/////    	      if(v_most_played_levels[best_position] != NULL) { /* the current best must not be free */
/////    		if(v_most_played_levels[best_position]->nPlayed > v_most_played_levels[j]->nPlayed) {
/////    		  best_position = j;
/////    		}
/////    	      }
/////    	    }
/////    	  }
/////    	}
/////      }
/////      if(best_position != -1) {
/////	v_most_played_levels[best_position] = v_levelsStats[i];
/////      }
/////    }
/////    for(unsigned int i=0; i<STATS_PACK_NUMBER_OF_LEVELS; i++) {
/////      if(v_most_played_levels[i] != NULL) {
/////    	try {
/////    	  v_pack->addLevel(&(LevelById(v_most_played_levels[i]->LevelID)));
/////    	} catch(Exception &e) {
/////    	  /* the level no longer exists, ok, ignore it */
/////	  vapp::Log("** Warning : Level of id '%s' no longer exists (from stats file)",
/////		    v_most_played_levels[i]->LevelID.c_str());
/////    	}
/////      }
/////    }
/////  }
/////
/////  /* less played levels */
/////  if(v_playerStats != NULL) {
/////    v_pack = new LevelsPack(std::string(VPACKAGENAME_LESS_PLAYED));
/////    v_pack->setGroup(GAMETEXT_PACK_STATS);
/////    m_levelsPacks.push_back(v_pack);
/////    std::vector<vapp::LevelStats *> v_levelsStats = v_playerStats->Levels;
/////
/////    vapp::LevelStats* v_less_played_levels[STATS_PACK_NUMBER_OF_LEVELS];
/////    for(int i=0; i<STATS_PACK_NUMBER_OF_LEVELS; i++) {
/////      v_less_played_levels[i] = NULL;
/////    }
/////    for(unsigned int i=0; i<v_levelsStats.size(); i++) {
/////      int best_position = -1;
/////      for(int j=STATS_PACK_NUMBER_OF_LEVELS-1; j>=0; j--) {
/////    	if(v_less_played_levels[j] == NULL) {
/////    	  best_position = j; /* take a free position */
/////    	} else {
/////    	  /* if there is no free position, take the place if it's better */
/////    	  if(v_levelsStats[i]->nPlayed < v_less_played_levels[j]->nPlayed) {
/////    	    /* ok, take the position if the current best position is not better */
/////    	    if(best_position == -1) { /* no position already set */
/////    	      best_position = j;
/////    	    } else {
/////    	      if(v_less_played_levels[best_position] != NULL) { /* the current best must not be free */
/////    		if(v_less_played_levels[best_position]->nPlayed < v_less_played_levels[j]->nPlayed) {
/////    		  best_position = j;
/////    		}
/////    	      }
/////    	    }
/////    	  }
/////    	}
/////      }
/////      if(best_position != -1) {
/////	v_less_played_levels[best_position] = v_levelsStats[i];
/////      }
/////    }
/////    for(unsigned int i=0; i<STATS_PACK_NUMBER_OF_LEVELS; i++) {
/////      if(v_less_played_levels[i] != NULL) {
/////    	try {
/////    	  v_pack->addLevel(&(LevelById(v_less_played_levels[i]->LevelID)));
/////    	} catch(Exception &e) {
/////    	  /* the level no longer exists, ok, ignore it */
/////	  vapp::Log("** Warning : Level of id '%s' no longer exists (from stats file)",
/////		    v_less_played_levels[i]->LevelID.c_str());
/////    	}
/////      }
/////    }
/////  }

  /* new levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_NEW_LEVELS));
  m_levelsPacks.push_back(v_pack);
  v_pack->setGroup(GAMETEXT_PACK_LAST);
  for(unsigned int i=0; i<m_newLevels.size(); i++) {
    v_pack->addLevel(m_newLevels[i]);
  }

  /* updated levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_UPDATED_LEVELS));
  m_levelsPacks.push_back(v_pack);
  v_pack->setGroup(GAMETEXT_PACK_LAST);
  for(unsigned int i=0; i<m_updatedLevels.size(); i++) {
    v_pack->addLevel(m_updatedLevels[i]);
  }

#if defined(SUPPORT_WEBACCESS)
  /* nicest levels */
  if(i_webLevels != NULL) {
    v_pack = new LevelsPack(std::string(VPACKAGENAME_NICEST_LEVELS));
    m_levelsPacks.push_back(v_pack);
    v_pack->setGroup(GAMETEXT_PACK_WEBVOTES);
    for(unsigned int i=0; i<i_webLevels->getLevels().size(); i++) {
      WebLevel *v_wlevel = i_webLevels->getLevels()[i];
      if(v_wlevel->getQuality() >= VPACKAGENAME_NICEST_MIN) {
	try {
	  v_pack->addLevel(&LevelById(v_wlevel->getId()));
	} catch(Exception &e) {
	  /* ok, will not be in the pack */
	}
      }
    }
  }
#endif

#if defined(SUPPORT_WEBACCESS)
  /* crapiest levels */
  if(i_bDebugMode) {
    if(i_webLevels != NULL) {
      v_pack = new LevelsPack(std::string(VPACKAGENAME_CRAPIEST_LEVELS));
      m_levelsPacks.push_back(v_pack);
      v_pack->setGroup(GAMETEXT_PACK_WEBVOTES);
      for(unsigned int i=0; i<i_webLevels->getLevels().size(); i++) {
	WebLevel *v_wlevel = i_webLevels->getLevels()[i];
	if(v_wlevel->getQuality() <= VPACKAGENAME_CRAPIEST_MAX) {
	  try {
	    v_pack->addLevel(&LevelById(v_wlevel->getId()));
	  } catch(Exception &e) {
	    /* ok, will not be in the pack */
	  }
	}
      }
    }
  }
#endif

  /* easiest levels */
#if defined(SUPPORT_WEBACCESS)
  /* easiest levels */
  if(i_webLevels != NULL) {
    v_pack = new LevelsPack(std::string(VPACKAGENAME_EASIEST_LEVELS));
    m_levelsPacks.push_back(v_pack);
    v_pack->setGroup(GAMETEXT_PACK_WEBVOTES);
    for(unsigned int i=0; i<i_webLevels->getLevels().size(); i++) {
      WebLevel *v_wlevel = i_webLevels->getLevels()[i];
      if(v_wlevel->getDifficulty() <= VPACKAGENAME_EASIEST_MAX) {
	try {
	  v_pack->addLevel(&LevelById(v_wlevel->getId()));
	} catch(Exception &e) {
	  /* ok, will not be in the pack */
	}
      }
    }
  }
#endif

  /* hardest levels */
#if defined(SUPPORT_WEBACCESS)
  /* hardest levels */
  if(i_webLevels != NULL) {
    v_pack = new LevelsPack(std::string(VPACKAGENAME_HARDEST_LEVELS));
    m_levelsPacks.push_back(v_pack);
    v_pack->setGroup(GAMETEXT_PACK_WEBVOTES);
    for(unsigned int i=0; i<i_webLevels->getLevels().size(); i++) {
      WebLevel *v_wlevel = i_webLevels->getLevels()[i];
      if(v_wlevel->getDifficulty() >= VPACKAGENAME_HARDEST_MIN) {
	try {
	  v_pack->addLevel(&LevelById(v_wlevel->getId()));
	} catch(Exception &e) {
	  /* ok, will not be in the pack */
	}
      }
    }
  }
#endif

  /* standard packs */
  for(unsigned int i=0; i<m_levels.size(); i++) {
    if(doesLevelsPackExist(m_levels[i]->Pack())) {
      LevelsPackByName(m_levels[i]->Pack()).addLevel(m_levels[i]);
    } else {
      LevelsPack *v_levelsPack = new LevelsPack(m_levels[i]->Pack());
      v_levelsPack->setGroup(GAMETEXT_PACK_STANDARD);
      v_levelsPack->addLevel(m_levels[i]);
      m_levelsPacks.push_back(v_levelsPack);
    }
  }

#if defined(SUPPORT_WEBACCESS)
  /* by winner */
  if(i_webHighscores != NULL) {
    for(unsigned int i=0; i<m_levels.size(); i++) {
      WebHighscore* wh = i_webHighscores->getHighscoreFromLevel(m_levels[i]->Id());
      if(wh != NULL) {
	if(doesLevelsPackExist(wh->getPlayerName())) {
	  v_pack = &LevelsPackByName(wh->getPlayerName());
	} else {
	  v_pack = new LevelsPack(wh->getPlayerName());
	  v_pack->setGroup(VPACKAGENAME_BEST_DRIVER);
	  m_levelsPacks.push_back(v_pack);
	}
	v_pack->addLevel(m_levels[i]);
      }
    }
  }
#endif

  /* by authors */
  /*
  for(unsigned int i=0; i<m_levels.size(); i++) {
    if(doesLevelsPackExist(m_levels[i]->Author()) == false) {
      v_pack = new LevelsPack(m_levels[i]->Author());
      m_levelsPacks.push_back(v_pack);
      v_pack->setGroup(GAMETEXT_PACK_AUTHORS);
    } else {
      v_pack = &LevelsPackByName(m_levels[i]->Author());
    }
    v_pack->addLevel(m_levels[i]);
  }
  */

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

void LevelsManager::loadExternalLevels(bool i_enableCache) {
  std::vector<std::string> LvlFiles = vapp::FS::findPhysFiles("Levels/MyLevels/*.lvl", true);

  loadLevelsFromLvl(LvlFiles,
		    i_enableCache,
		    false,
		    true
		    ); 
}

void LevelsManager::addExternalLevel(std::string i_levelFile) {
  std::vector<std::string> LvlFiles;
  LvlFiles.push_back(i_levelFile);

  loadLevelsFromLvl(LvlFiles,
		    false,
		    false,
		    true
		    );  
}

void LevelsManager::reloadLevelsFromFiles(bool i_enableCache, XMotoLoadLevelsInterface *i_loadLevelsInterface) {
  clean();

  std::vector<std::string> LvlFiles = vapp::FS::findPhysFiles("Levels/*.lvl", true);
  loadLevelsFromLvl(LvlFiles, i_enableCache, i_loadLevelsInterface);
  loadExternalLevels(i_enableCache);
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
    loadFavoriteLevelsXml();
  } catch(Exception &e) {
    vapp::Log("** Warning : Unable to load FavoriteLevels xml file");
  }
}

std::string LevelsManager::LevelIndexFileName() {
  return vapp::FS::getUserDir() + "/" + "LCache/levels.index";
}

void LevelsManager::loadLevelsFromIndex(bool i_enableCache) {
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
	/* if one level fails, all must fail because otherwise, some part of pfh is read badly */
	throw e;
      }
    }

  } catch(Exception &e) {
    clean();
    vapp::FS::closeFile(pfh);
    throw e;
  }
  vapp::FS::closeFile(pfh);

  try {
    loadExternalLevels(i_enableCache);
  } catch(Exception &e) {
    vapp::Log("** Warning : Unable to load externals levels");
  }

  try {
    loadNewLevelsXml();
  } catch(Exception &e) {
    vapp::Log("** Warning : Unable to load NewLevels xml file");
  }

  try {
    loadFavoriteLevelsXml();
  } catch(Exception &e) {
    vapp::Log("** Warning : Unable to load FavoriteLevels xml file");
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
    vapp::FS::writeInt_LE(pfh, m_levels.size() - m_externalLevels.size());
    for(int i=0; i<m_levels.size(); i++) {

      bool v_found = false;
      for(int j=0; j<m_externalLevels.size(); j++) {
	if(m_externalLevels[j] == m_levels[i]->Id()) {
	  v_found = true;
	  break;
	}
      }

      /* only index levels which are not externals */
      if(v_found == false) {
	vapp::FS::writeString(pfh, m_levels[i]->FileName());
	m_levels[i]->exportBinaryHeader(pfh);
      }
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

void LevelsManager::loadLevelsFromLvl(const std::vector<std::string> &LvlFiles,
				      bool i_enableCache,
				      XMotoLoadLevelsInterface *i_loadLevelsInterface) {
  loadLevelsFromLvl(LvlFiles, i_enableCache, false, false, i_loadLevelsInterface);
} 

void LevelsManager::loadLevelsFromLvl(const std::vector<std::string> &LvlFiles,
				      bool i_enableCache,
				      bool i_newLevels,
				      bool i_external,
				      XMotoLoadLevelsInterface *i_loadLevelsInterface
				      ) {

  /* find external levels */
  std::vector<std::string> v_externalLevelsFiles;

  if(i_external == false) {
    v_externalLevelsFiles = vapp::FS::findPhysFiles("Levels/MyLevels/*.lvl", true);
  }

  for(int i=0;i<LvlFiles.size();i++) {
    bool bCached = false;

    Level *v_level = new Level();
    
    try {
      v_level->setFileName(LvlFiles[i]);
      bCached = v_level->loadReducedFromFile(i_enableCache);
      
      // Check for ID conflict
      if(i_external == false) { /* don't check for external levels because if i click on a .lvl which exist in the game, i want it works */
	if(doesLevelExist(v_level->Id())) { /* LvlFiles order assure that levels of userDir are load before system levels (which allow level updates) */
	  throw Exception("Duplicate level ID");
	}
      }

      /* is it an external level ? */
      bool v_addInList = false;

      if(i_external) {
	v_addInList = true;
      } else {
	bool v_found = false;
	for(unsigned int j=0; j<v_externalLevelsFiles.size(); j++) {
	  if(v_externalLevelsFiles[j] == LvlFiles[i]) {
	    v_found = true;
	    break;
	  }
	}
	v_addInList = v_found == false;
      }

      if(v_addInList == false) {
	delete v_level;
      } else {
	m_levels.push_back(v_level);
	if(i_external) {
	  m_externalLevels.push_back(v_level->Id());
	}

	if(i_loadLevelsInterface != NULL) {
	  i_loadLevelsInterface->loadLevelHook(v_level->Name(), (i*100) / LvlFiles.size());
	}

	if(i_newLevels) {
	  m_newLevels.push_back(v_level);
	}
      }

    } catch(Exception &e) {
      delete v_level;
    }
  }
}

void LevelsManager::checkPrerequires(bool &v_enableCache) {
  std::string LCachePath = vapp::FS::getUserDir() + std::string("/LCache");

  if(v_enableCache && vapp::FS::isDir(LCachePath) == false) {
    try {
      vapp::FS::mkArborescence(LCachePath + "/file.tmp");
    } catch(Exception &e) {
      v_enableCache = false;
      vapp::Log("** Warning ** : Level cache directory can't be created, forcing caching off!");
    }
  }

  try {
    vapp::FS::mkArborescence(vapp::FS::getUserDir() + "/Levels/MyLevels/file.tmp");
  } catch(Exception &e) {
    vapp::Log("** Warning ** : MyLevels directory can't be created");
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
																				const std::vector<std::string> &UpdatedLvlIds,
																				bool i_enableCache) {
  m_newLevels.clear();
  m_updatedLevels.clear();

  loadLevelsFromLvl(NewLvl, i_enableCache, true);  

  Level *v_level;
  for(int i=0;i<UpdatedLvlIds.size();i++) {
    try {
      v_level = &(LevelById(UpdatedLvlIds[i]));
			v_level->setFileName(UpdatedLvlFileNames[i]);
      v_level->loadReducedFromFile(i_enableCache);
      m_updatedLevels.push_back(v_level);
    } catch(Exception &e) {
      vapp::Log("** Warning ** : Problem updating '%s' (%s)", UpdatedLvlFileNames[i].c_str(), e.getMsg().c_str());
    }
  }

	/* recreate the level index */
	createLevelsIndex();
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

const std::vector<Level *>& LevelsManager::FavoriteLevels() {
  return m_favoriteLevels;
}

std::string LevelsManager::FavoriteLevelsXmlFilePath() {
  return vapp::FS::getUserDir() + "/" + "favoriteLevels.xml";
}

void LevelsManager::addToFavorite(Level *i_level) {
  /* check if the level is already into the favorite list */
  for(int i=0; i<m_favoriteLevels.size(); i++) {
    if(m_favoriteLevels[i] == i_level) {
      return;
    }
  }
  m_favoriteLevels.push_back(i_level);
  LevelsPackByName(std::string(VPACKAGENAME_FAVORITE_LEVELS)).addLevel(i_level);
}

void LevelsManager::delFromFavorite(Level *i_level) {
  for(int i=0; i<m_favoriteLevels.size(); i++) {
    if(m_favoriteLevels[i] == i_level) {
      m_favoriteLevels.erase(m_favoriteLevels.begin() + i);
      LevelsPackByName(std::string(VPACKAGENAME_FAVORITE_LEVELS)).removeLevel(i_level);
      return;
    }
  }
}

void LevelsManager::saveFavoriteLevelsXml() const {
  remove(FavoriteLevelsXmlFilePath().c_str());

  vapp::FileHandle *pfh = vapp::FS::openOFile(FavoriteLevelsXmlFilePath());
  if(pfh == NULL) {
    vapp::Log("** Warning ** : failed to open '%s'", FavoriteLevelsXmlFilePath().c_str());
    return;
  }

  vapp::FS::writeLineF(pfh,"<?xml version=\"1.0\" encoding=\"utf-8\"?>");
  vapp::FS::writeLineF(pfh,"<favoriteLevels>");  

  for(int i=0; i<m_favoriteLevels.size(); i++) {
    vapp::FS::writeLineF(pfh, "<level id=\"%s\" />", vapp::XML::str2xmlstr(m_favoriteLevels[i]->Id()).c_str());
  }

  vapp::FS::writeLineF(pfh,"</favoriteLevels>");  
  vapp::FS::closeFile(pfh);
}

void LevelsManager::loadFavoriteLevelsXml() {
  m_favoriteLevels.clear();

  vapp::XMLDocument v_favoriteLevelsXml;
  TiXmlDocument *v_favoriteLevelsXmlData;
  TiXmlElement *v_favoriteLevelsXmlDataElement;
  const char *pc;
  std::string v_levelId;
  
  v_favoriteLevelsXml.readFromFile(FavoriteLevelsXmlFilePath());
  v_favoriteLevelsXmlData = v_favoriteLevelsXml.getLowLevelAccess();

  if(v_favoriteLevelsXmlData == NULL) {
    throw Exception("error : unable to analyze xml favoriteLevels file");
  }

  v_favoriteLevelsXmlDataElement = v_favoriteLevelsXmlData->FirstChildElement("favoriteLevels");
  
  if(v_favoriteLevelsXmlDataElement == NULL) {
    throw Exception("error : unable to analyze xml favoriteLevels file");
  }
    
  TiXmlElement *pVarElem = v_favoriteLevelsXmlDataElement->FirstChildElement("level");
  while(pVarElem != NULL) {
    v_levelId = "";

    pc = pVarElem->Attribute("id");
    if(pc != NULL) {
      v_levelId = pc;
  
      /* add the level into the list */
      m_favoriteLevels.push_back(&(LevelById(v_levelId)));
    }
    pVarElem = pVarElem->NextSiblingElement("level");
  }
}

void LevelsManager::saveXml() const {
  saveFavoriteLevelsXml();
  saveNewLevelsXml();
}
