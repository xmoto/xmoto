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
#include "db/xmDatabase.h"

LevelsPack::LevelsPack(std::string i_name, const std::string& i_sql, bool i_ascSort) {
  m_name         = i_name;
  m_showTimes    = true;
  m_showWebTimes = true;
  //setHintsFromFile();
  m_sql_levels   = i_sql;
  m_ascSort      = i_ascSort;
}

LevelsPack::~LevelsPack() {
}

int LevelsPack::getNumberOfLevels(xmDatabase *i_db) {
  char **v_result;
  int nrow;
  int n;

  v_result = i_db->readDB("SELECT count(id_level) FROM (" + m_sql_levels + ");",
			  nrow);
  if(i_db->getResult(v_result, 1, 0, 0) == NULL) {
    i_db->read_DB_free(v_result);
    return 0;
  }

  n = atoi(i_db->getResult(v_result, 1, 0, 0));
  i_db->read_DB_free(v_result);
  return n;
}

std::string LevelsPack::getLevelsQuery() const {
  return "SELECT id_level, name FROM (" + m_sql_levels + ") ORDER BY sort_field "
    + std::string(m_ascSort ? "ASC":"DESC") + ";";
}

std::string LevelsPack::getLevelsWithHighscoresQuery(const std::string& i_profile,
						     const std::string& i_id_room) const {
  return "SELECT a.id_level AS id_level, MIN(a.name) AS name, MIN(c.finishTime), MIN(b.finishTime) "
    "FROM ("+ m_sql_levels + ") AS a "
    "LEFT OUTER JOIN webhighscores AS b "
    "ON (a.id_level = b.id_level AND b.id_room=" + i_id_room + ") "
    "LEFT OUTER JOIN profile_completedLevels AS c "
    "ON (a.id_level=c.id_level AND c.id_profile=\"" + xmDatabase::protectString(i_profile) + "\") "
    "GROUP BY a.id_level ORDER BY min(a.sort_field) " + std::string(m_ascSort ? "ASC":"DESC") + ";";
}

int LevelsPack::getNumberOfFinishedLevels(xmDatabase *i_db, const std::string& i_profile) {
  char **v_result;
  int nrow;
  int n;

  v_result = i_db->readDB("SELECT count(a.id_level) FROM (" +
			  m_sql_levels +
			  ") AS a INNER JOIN stats_profiles_levels AS b "
			  "ON a.id_level=b.id_level WHERE b.id_profile=\"" +
			  xmDatabase::protectString(i_profile) + "\" AND b.nbCompleted > 0;",
			  nrow);
  if(i_db->getResult(v_result, 1, 0, 0) == NULL) {
    i_db->read_DB_free(v_result);
    return 0;
  }
  n = atoi(i_db->getResult(v_result, 1, 0, 0));
  i_db->read_DB_free(v_result);
  return n;
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

void LevelsPack::setDescription(const std::string& i_description) {
  m_description = i_description;
}

std::string LevelsPack::Description() const {
  return m_description;
}

std::string LevelsPack::Group() const {
  return m_group;
}

void LevelsPack::setGroup(std::string i_group) {
  m_group = i_group;
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

LevelsManager::LevelsManager() {
}

LevelsManager::~LevelsManager() {
  clean();
}
 
void LevelsManager::clean() {
  cleanPacks();
}

void LevelsManager::cleanPacks() {
  /* delete existing packs */
  for(int i=0;i<m_levelsPacks.size();i++) {
    delete m_levelsPacks[i];
  }
  m_levelsPacks.clear();
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

void LevelsManager::makePacks(xmDatabase *i_db,
			      const std::string& i_playerName,
			      const std::string& i_id_room,
			      bool i_bDebugMode) {
  LevelsPack *v_pack;
  char **v_result;
  int nrow;

  cleanPacks();

  /* all levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_ALL_LEVELS),
			  "SELECT id_level, name, UPPER(name) AS sort_field FROM levels");
  v_pack->setGroup(GAMETEXT_PACK_SPECIAL);
  v_pack->setDescription(VPACKAGENAME_DESC_ALL_LEVELS);
  m_levelsPacks.push_back(v_pack);

  /* my levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_MY_LEVELS),
			  "SELECT id_level, name, UPPER(name) AS sort_field FROM levels WHERE isToReload=1");
  v_pack->setGroup(GAMETEXT_PACK_SPECIAL);
  v_pack->setDescription(VPACKAGENAME_DESC_MY_LEVELS);
  m_levelsPacks.push_back(v_pack);
  
  /* favorite levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_FAVORITE_LEVELS),
			  "SELECT a.id_level AS id_level, a.name AS name, UPPER(a.name) AS sort_field "
			  "FROM levels AS a INNER JOIN levels_favorite AS b "
			  "ON a.id_level=b.id_level "
			  "WHERE b.id_profile=\"" + xmDatabase::protectString(i_playerName) + "\"");
  v_pack->setGroup(GAMETEXT_PACK_SPECIAL);
  v_pack->setDescription(VPACKAGENAME_DESC_FAVORITE_LEVELS);
  m_levelsPacks.push_back(v_pack);

  /* scripted levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_SCRIPTED),
			  "SELECT id_level, name, UPPER(name) AS sort_field FROM levels WHERE isScripted=1");
  v_pack->setGroup(GAMETEXT_PACK_SPECIAL);
  v_pack->setDescription(VPACKAGENAME_DESC_SCRIPTED);
  m_levelsPacks.push_back(v_pack);

  /* musical */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_MUSICAL),
			  "SELECT id_level, name, UPPER(name) AS sort_field "
			  "FROM levels WHERE music<>\"\"");
  v_pack->setGroup(GAMETEXT_PACK_SPECIAL);
  v_pack->setDescription(VPACKAGENAME_DESC_MUSICAL);
  m_levelsPacks.push_back(v_pack);

  /* levels i've not finished */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_INCOMPLETED_LEVELS),
			  "SELECT a.id_level AS id_level, a.name AS name, UPPER(a.name) AS sort_field "
			  "FROM levels AS a LEFT OUTER JOIN stats_profiles_levels AS b "
			  "ON (a.id_level=b.id_level AND "
			  "b.id_profile=\"" + xmDatabase::protectString(i_playerName) + "\") "
			  "WHERE b.nbCompleted = 0 OR b.id_profile IS NULL");
  v_pack->setGroup(GAMETEXT_PACK_SPECIAL);
  v_pack->setDescription(VPACKAGENAME_DESC_INCOMPLETED_LEVELS);
  m_levelsPacks.push_back(v_pack);

  /* new and updated levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_NEW_LEVELS),
			  "SELECT b.id_level AS id_level, "
			  "xm_lvlUpdatedToTxt(a.isAnUpdate) || \": \" || b.name AS name, "
			  "UPPER(b.name) AS sort_field "
			  "FROM levels_new AS a INNER JOIN levels AS b ON a.id_level=b.id_level");
  v_pack->setGroup(GAMETEXT_PACK_SPECIAL);
  v_pack->setDescription(VPACKAGENAME_DESC_NEW_LEVELS);
  m_levelsPacks.push_back(v_pack);

  /* rooms */
  /* levels with no highscore */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_LEVELS_WITH_NO_HIGHSCORE),
			  "SELECT a.id_level AS id_level, a.name AS name, UPPER(a.name) AS sort_field "
			  "FROM levels AS a LEFT OUTER JOIN "
			  "webhighscores AS b ON (a.id_level = b.id_level "
			  "AND b.id_room=" + i_id_room + ") "
			  "WHERE b.id_level IS NULL");
  v_pack->setGroup(GAMETEXT_PACK_ROOM);
  v_pack->setDescription(VPACKAGENAME_DESC_LEVELS_WITH_NO_HIGHSCORE);
  m_levelsPacks.push_back(v_pack);

  /* levels i've not the highscore */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_YOU_HAVE_NOT_THE_HIGHSCORE),
			  "SELECT a.id_level AS id_level, a.name AS name, UPPER(a.name) AS sort_field "
			  "FROM levels AS a INNER JOIN "
			  "webhighscores AS b ON a.id_level = b.id_level "
			  "WHERE b.id_room=" + i_id_room + " "
			  "AND b.id_profile<>\"" + xmDatabase::protectString(i_playerName) + "\" ");
  v_pack->setGroup(GAMETEXT_PACK_ROOM);
  v_pack->setDescription(VPACKAGENAME_DESC_YOU_HAVE_NOT_THE_HIGHSCORE);
  m_levelsPacks.push_back(v_pack);

  /* last highscores */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_LAST_HIGHSCORES),
			  "SELECT a.id_level AS id_level, a.name AS name, b.date AS sort_field "
			  "FROM levels AS a INNER JOIN "
			  "webhighscores AS b ON a.id_level = b.id_level "
			  "WHERE b.id_room=" + i_id_room + " "
			  "ORDER by b.date DESC LIMIT 50", false);
  v_pack->setDescription(VPACKAGENAME_DESC_LAST_HIGHSCORES);
  m_levelsPacks.push_back(v_pack);
  v_pack->setGroup(GAMETEXT_PACK_ROOM);

  /* last levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_LAST_LEVELS),
			  "SELECT a.id_level AS id_level, a.name AS name, b.creationDate AS sort_field "
			  "FROM levels AS a INNER JOIN "
			  "weblevels AS b ON a.id_level = b.id_level "
			  "ORDER by b.creationDate DESC LIMIT 50", false);
  v_pack->setGroup(GAMETEXT_PACK_ROOM);
  v_pack->setDescription(VPACKAGENAME_DESC_LAST_LEVELS);
  m_levelsPacks.push_back(v_pack);

  /* oldest highscores */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_OLDEST_HIGHSCORES),
			  "SELECT a.id_level AS id_level, a.name AS name, b.date AS sort_field "
			  "FROM levels AS a INNER JOIN "
			  "webhighscores AS b ON a.id_level = b.id_level "
			  "WHERE b.id_room=" + i_id_room + " "
			  "ORDER by b.date ASC LIMIT 50");
  v_pack->setGroup(GAMETEXT_PACK_ROOM);
  v_pack->setDescription(VPACKAGENAME_DESC_OLDEST_HIGHSCORES);
  m_levelsPacks.push_back(v_pack);

  /* medals */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_MEDAL_PLATINIUM),
			  "SELECT b.id_level AS id_level, b.name AS name, UPPER(b.name) AS sort_field "
			  "FROM webhighscores AS a INNER JOIN levels AS b "
			  "ON a.id_level = b.id_level "
			  "WHERE a.id_profile = \"" + xmDatabase::protectString(i_playerName) + "\""
			  );
  v_pack->setGroup(GAMETEXT_PACK_MEDALS);
  v_pack->setDescription(VPACKAGENAME_DESC_MEDAL_PLATINIUM);
  m_levelsPacks.push_back(v_pack);

  v_pack = new LevelsPack(std::string(VPACKAGENAME_MEDAL_GOLD),
			  "SELECT id_level, name, sort_field FROM ("
			  "SELECT a.id_level AS id_level, MAX(b.name) AS name, "
			  "UPPER(MAX(b.name)) AS sort_field, "
			  "MIN(c.finishTime) AS webFinishTime, MIN(a.finishTime) AS userFinishTime "
			  "FROM profile_completedLevels AS a INNER JOIN levels AS b "
			  "ON (a.id_level = b.id_level "
			  "AND a.id_profile=\"" + xmDatabase::protectString(i_playerName) + "\") "
			  "LEFT OUTER JOIN webhighscores AS c "
			  "ON (c.id_level = a.id_level AND c.id_room=" + i_id_room + ") "
			  "GROUP BY a.id_level " /* get the best highscores */
			  ") "
			  "WHERE (webFinishTime IS NULL OR webFinishTime >= userFinishTime*0.95) "
			  "EXCEPT " /* remove the webhighscores where the player's name is the same */
			  "SELECT b.id_level AS id_level, b.name AS name, UPPER(b.name) AS sort_field "
			  "FROM webhighscores AS a INNER JOIN levels AS b "
			  "ON a.id_level = b.id_level "
			  "WHERE a.id_profile = \"" + xmDatabase::protectString(i_playerName) + "\""
			  );
  v_pack->setGroup(GAMETEXT_PACK_MEDALS);
  v_pack->setDescription(VPACKAGENAME_DESC_MEDAL_GOLD);
  m_levelsPacks.push_back(v_pack);

  v_pack = new LevelsPack(std::string(VPACKAGENAME_MEDAL_SILVER),
			  "SELECT id_level, name, sort_field FROM ("
			  "SELECT a.id_level AS id_level, MAX(b.name) AS name, "
			  "UPPER(MAX(b.name)) AS sort_field, "
			  "MIN(c.finishTime) AS webFinishTime, MIN(a.finishTime) AS userFinishTime "
			  "FROM profile_completedLevels AS a INNER JOIN levels AS b "
			  "ON (a.id_level = b.id_level "
			  "AND a.id_profile=\"" + xmDatabase::protectString(i_playerName) + "\") "
			  "LEFT OUTER JOIN webhighscores AS c "
			  "ON (c.id_level = a.id_level AND c.id_room=" + i_id_room + ") "
			  "GROUP BY a.id_level " /* get the best highscores */
			  ") "
			  "WHERE (webFinishTime IS NOT NULL "
			  "AND webFinishTime >= userFinishTime*0.90 "
			  "AND webFinishTime < userFinishTime*0.95) "
			  "EXCEPT " /* remove the webhighscores where the player's name is the same */
			  "SELECT b.id_level AS id_level, b.name AS name, UPPER(b.name) AS sort_field "
			  "FROM webhighscores AS a INNER JOIN levels AS b "
			  "ON a.id_level = b.id_level "
			  "WHERE a.id_profile = \"" + xmDatabase::protectString(i_playerName) + "\""
			  );
  v_pack->setGroup(GAMETEXT_PACK_MEDALS);
  v_pack->setDescription(VPACKAGENAME_DESC_MEDAL_SILVER);
  m_levelsPacks.push_back(v_pack);

  v_pack = new LevelsPack(std::string(VPACKAGENAME_MEDAL_BRONZE),
			  "SELECT id_level, name, sort_field FROM ("
			  "SELECT a.id_level AS id_level, MAX(b.name) AS name, "
			  "UPPER(MAX(b.name)) AS sort_field, "
			  "MIN(c.finishTime) AS webFinishTime, MIN(a.finishTime) AS userFinishTime "
			  "FROM profile_completedLevels AS a INNER JOIN levels AS b "
			  "ON (a.id_level = b.id_level "
			  "AND a.id_profile=\"" + xmDatabase::protectString(i_playerName) + "\") "
			  "LEFT OUTER JOIN webhighscores AS c "
			  "ON (c.id_level = a.id_level AND c.id_room=" + i_id_room + ") "
			  "GROUP BY a.id_level " /* get the best highscores */
			  ") "
			  "WHERE (webFinishTime IS NOT NULL "
			  "AND webFinishTime >= userFinishTime*0.80 "
			  "AND webFinishTime < userFinishTime*0.90) "
			  "EXCEPT " /* remove the webhighscores where the player's name is the same */
			  "SELECT b.id_level AS id_level, b.name AS name, UPPER(b.name) AS sort_field "
			  "FROM webhighscores AS a INNER JOIN levels AS b "
			  "ON a.id_level = b.id_level "
			  "WHERE a.id_profile = \"" + xmDatabase::protectString(i_playerName) + "\""
			  );
  v_pack->setGroup(GAMETEXT_PACK_MEDALS);
  v_pack->setDescription(VPACKAGENAME_DESC_MEDAL_BRONZE);
  m_levelsPacks.push_back(v_pack);

  v_pack = new LevelsPack(std::string(VPACKAGENAME_MEDAL_NONE),
			  "SELECT id_level, name, sort_field FROM ("
			  "SELECT a.id_level AS id_level, MAX(a.name) AS name, "
			  "UPPER(MAX(a.name)) AS sort_field, "
			  "MIN(c.finishTime) AS webFinishTime, MIN(b.finishTime) AS userFinishTime "
			  "FROM levels AS a LEFT OUTER JOIN profile_completedLevels AS b "
			  "ON (a.id_level = b.id_level "
			  "AND b.id_profile=\"" + xmDatabase::protectString(i_playerName) + "\") "
			  "LEFT OUTER JOIN webhighscores AS c "
			  "ON (c.id_level = a.id_level AND c.id_room=" + i_id_room + ") "
			  "GROUP BY a.id_level "
			  ") "
			  "WHERE (webFinishTime IS NOT NULL "
			  "AND webFinishTime < userFinishTime*0.80) "
			  "OR userFinishTime IS NULL "
			  "EXCEPT " /* remove the webhighscores where the player's name is the same */
			  "SELECT b.id_level AS id_level, b.name AS name, UPPER(b.name) AS sort_field "
			  "FROM webhighscores AS a INNER JOIN levels AS b "
			  "ON a.id_level = b.id_level "
			  "WHERE a.id_profile = \"" + xmDatabase::protectString(i_playerName) + "\""
			  );
  v_pack->setGroup(GAMETEXT_PACK_MEDALS);
  v_pack->setDescription(VPACKAGENAME_DESC_MEDAL_NONE);
  m_levelsPacks.push_back(v_pack);

  /* stats */
  /* last played levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_LAST_PLAYED),
			  "SELECT a.id_level AS id_level, a.name AS name, b.last_play_date AS sort_field "
			  "FROM levels AS a INNER JOIN stats_profiles_levels AS b "
			  "ON (a.id_level=b.id_level AND "
			  "b.id_profile=\"" + xmDatabase::protectString(i_playerName) + "\") "
			  "WHERE b.last_play_date IS NOT NULL "
			  "ORDER BY b.last_play_date "
			  "LIMIT 50", false);
  v_pack->setGroup(GAMETEXT_PACK_STATS);
  v_pack->setDescription(VPACKAGENAME_DESC_LAST_PLAYED);
  m_levelsPacks.push_back(v_pack);

  /* never played levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_NEVER_PLAYED),
			  "SELECT a.id_level AS id_level, a.name AS name, UPPER(a.name) AS sort_field "
			  "FROM levels AS a LEFT OUTER JOIN stats_profiles_levels AS b "
			  "ON (a.id_level=b.id_level AND "
			  "b.id_profile=\"" + xmDatabase::protectString(i_playerName) + "\") "
			  "WHERE b.id_profile IS NULL");
  v_pack->setGroup(GAMETEXT_PACK_STATS);
  v_pack->setDescription(VPACKAGENAME_DESC_NEVER_PLAYED);
  m_levelsPacks.push_back(v_pack);
  
  /* most played levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_MOST_PLAYED),
			  "SELECT a.id_level AS id_level, a.name AS name, b.nbPlayed AS sort_field "
			  "FROM levels AS a INNER JOIN stats_profiles_levels AS b "
			  "ON (a.id_level=b.id_level AND "
			  "b.id_profile=\"" + xmDatabase::protectString(i_playerName) + "\") "
			  "ORDER BY b.nbPlayed DESC LIMIT 50", false);
  v_pack->setGroup(GAMETEXT_PACK_STATS);
  v_pack->setDescription(VPACKAGENAME_DESC_MOST_PLAYED);
  m_levelsPacks.push_back(v_pack);

  /* less played levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_LESS_PLAYED),
			  "SELECT a.id_level AS id_level, a.name AS name, b.nbPlayed AS sort_field "
			  "FROM levels AS a INNER JOIN stats_profiles_levels AS b "
			  "ON (a.id_level=b.id_level AND "
			  "b.id_profile=\"" + xmDatabase::protectString(i_playerName) + "\") "
			  "WHERE b.nbPlayed > 0 "
			  "ORDER BY b.nbPlayed ASC LIMIT 50");
  v_pack->setGroup(GAMETEXT_PACK_STATS);
  v_pack->setDescription(VPACKAGENAME_DESC_LESS_PLAYED);
  m_levelsPacks.push_back(v_pack);

  /* nicest levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_NICEST_LEVELS),
			  "SELECT a.id_level AS id_level, a.name AS name, b.quality AS sort_field "
			  "FROM levels AS a INNER JOIN weblevels AS b ON a.id_level=b.id_level "
			  "WHERE b.quality >= 4.5 ORDER BY b.quality DESC LIMIT 50", false);
  v_pack->setGroup(GAMETEXT_PACK_WEBVOTES);
  v_pack->setDescription(VPACKAGENAME_DESC_NICEST_LEVELS);
  m_levelsPacks.push_back(v_pack);

  /* crapiest levels */
  if(i_bDebugMode) {
    v_pack = new LevelsPack(std::string(VPACKAGENAME_CRAPIEST_LEVELS),
			    "SELECT a.id_level AS id_level, a.name AS name, b.quality AS sort_field "
			    "FROM levels AS a INNER JOIN weblevels AS b ON a.id_level=b.id_level "
			    "WHERE b.quality <= 1.5 ORDER BY b.quality ASC LIMIT 50");
    v_pack->setGroup(GAMETEXT_PACK_WEBVOTES);
    v_pack->setDescription(VPACKAGENAME_DESC_CRAPIEST_LEVELS);
    m_levelsPacks.push_back(v_pack);
  }

  /* easiest levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_EASIEST_LEVELS),
			  "SELECT a.id_level AS id_level, a.name AS name, b.difficulty AS sort_field "
			  "FROM levels AS a INNER JOIN weblevels AS b ON a.id_level=b.id_level "
			  "WHERE b.difficulty <= 1.5 ORDER BY b.difficulty ASC LIMIT 50");
  v_pack->setGroup(GAMETEXT_PACK_WEBVOTES);
  v_pack->setDescription(VPACKAGENAME_DESC_EASIEST_LEVELS);
  m_levelsPacks.push_back(v_pack);

  /* hardest levels */
  v_pack = new LevelsPack(std::string(VPACKAGENAME_HARDEST_LEVELS),
			  "SELECT a.id_level AS id_level, a.name AS name, b.difficulty AS sort_field "
			  "FROM levels AS a INNER JOIN weblevels AS b ON a.id_level=b.id_level "
			  "WHERE b.difficulty >= 4.5 ORDER BY b.difficulty DESC LIMIT 50", false);
  v_pack->setGroup(GAMETEXT_PACK_WEBVOTES);
  v_pack->setDescription(VPACKAGENAME_DESC_HARDEST_LEVELS);
  m_levelsPacks.push_back(v_pack);

  /* standard packs */
  v_result = i_db->readDB("SELECT DISTINCT packName FROM levels ORDER BY UPPER(packName);",
			  nrow);
  for(unsigned int i=0; i<nrow; i++) {
    v_pack = new LevelsPack(i_db->getResult(v_result, 1, i, 0),
			    "SELECT id_level, name, packNum || UPPER(name) AS sort_field "
			    "FROM levels WHERE packName=\"" +
			    xmDatabase::protectString(i_db->getResult(v_result, 1, i, 0)) +
			    "\"");
    v_pack->setGroup(GAMETEXT_PACK_STANDARD);
    v_pack->setDescription(std::string(VPACKAGENAME_DESC_STANDARD) +
			   " \"" + std::string(i_db->getResult(v_result, 1, i, 0)) + "\"");
    m_levelsPacks.push_back(v_pack);
  }
  i_db->read_DB_free(v_result);

  /* by winner */
  v_result = i_db->readDB("SELECT id_profile "
			  "FROM webhighscores "
			  "GROUP BY id_profile "
			  "ORDER BY COUNT(1) DESC LIMIT 10;",
			  nrow);
  for(unsigned int i=0; i<nrow; i++) {
    std::ostringstream v_n;
    if(i+1<10) v_n << "0";
    v_n << i+1;
    v_n << ". ";

    std::string v_id_profile = i_db->getResult(v_result, 1, i, 0);
    v_pack = new LevelsPack(v_n.str() + v_id_profile,
			    "SELECT a.id_level AS id_level, a.name AS name, UPPER(a.name) AS sort_field "
			    "FROM levels AS a INNER JOIN "
			    "webhighscores AS b ON a.id_level = b.id_level "
			    "WHERE b.id_room=" + i_id_room + " "
			    "AND b.id_profile=\"" + xmDatabase::protectString(v_id_profile) + "\"");
    v_pack->setGroup(GAMETEXT_PACK_BEST_DRIVERS);
    v_pack->setDescription(std::string(VPACKAGENAME_DESC_BEST_DRIVERS) + " " + v_id_profile);
    m_levelsPacks.push_back(v_pack);
  }
  i_db->read_DB_free(v_result);  
 
}

const std::vector<LevelsPack *>& LevelsManager::LevelsPacks() {
  return m_levelsPacks;
}

void LevelsManager::reloadExternalLevels(xmDatabase *i_db,
					 XMotoLoadLevelsInterface *i_loadLevelsInterface) {
  std::vector<std::string> LvlFiles = vapp::FS::findPhysFiles("Levels/MyLevels/*.lvl", true);

  i_db->levels_add_begin(true);

  for(int i=0; i<LvlFiles.size(); i++) {
    bool bCached = false;

    Level *v_level = new Level();

    try {
      if(i_loadLevelsInterface != NULL) {
	i_loadLevelsInterface->loadLevelHook(LvlFiles[i], (i*100) / LvlFiles.size());
      }

      v_level->setFileName(LvlFiles[i]);
      bCached = v_level->loadReducedFromFile();
      
      // Check for ID conflict
      if(doesLevelExist(i_db, v_level->Id())) {
	throw Exception("Duplicate level ID");
      }
      i_db->levels_add(v_level->Id(),
		       v_level->FileName(),
		       v_level->Name(),
		       v_level->Checksum(),
		       v_level->Author(),
		       v_level->Description(),
		       v_level->Date(),
		       v_level->Pack(),
		       v_level->PackNum(),
		       v_level->Music(),
		       v_level->isScripted(),
		       true);
    } catch(Exception &e) {
    }
    delete v_level;
  }

  i_db->levels_add_end();
}

void LevelsManager::addExternalLevel(xmDatabase *i_db, std::string i_levelFile) {

  Level *v_level = new Level();
  try {
    bool bCached = false;

    v_level->setFileName(i_levelFile);
    bCached = v_level->loadReducedFromFile();
      
    // Check for ID conflict
    if(doesLevelExist(i_db, v_level->Id())) {
      throw Exception("Duplicate level ID");
    }
    i_db->levels_add(v_level->Id(),
		     v_level->FileName(),
		     v_level->Name(),
		     v_level->Checksum(),
		     v_level->Author(),
		     v_level->Description(),
		     v_level->Date(),
		     v_level->Pack(),
		     v_level->PackNum(),
		     v_level->Music(),
		     v_level->isScripted(),
		     true);
  } catch(Exception &e) {
  }
  delete v_level;
}

void LevelsManager::reloadLevelsFromLvl(xmDatabase *i_db,
					XMotoLoadLevelsInterface *i_loadLevelsInterface) {
  std::vector<std::string> LvlFiles = vapp::FS::findPhysFiles("Levels/*.lvl", true);

  i_db->levels_add_begin(false);

  for(int i=0; i<LvlFiles.size(); i++) {
    bool bCached = false;
    int v_isExternal;

    v_isExternal = LvlFiles[i].find("Levels/MyLevels/") != std::string::npos;
    if(v_isExternal) {
      continue; // don't load external levels now
    }

    Level *v_level = new Level();

    try {
      if(i_loadLevelsInterface != NULL) {
	i_loadLevelsInterface->loadLevelHook(v_level->Name(), (i*100) / LvlFiles.size());
      }

      v_level->setFileName(LvlFiles[i]);
      bCached = v_level->loadReducedFromFile();
      
      // Check for ID conflict
      if(doesLevelExist(i_db, v_level->Id())) {
	throw Exception("Duplicate level ID");
      }
      i_db->levels_add(v_level->Id(),
		       v_level->FileName(),
		       v_level->Name(),
		       v_level->Checksum(),
		       v_level->Author(),
		       v_level->Description(),
		       v_level->Date(),
		       v_level->Pack(),
		       v_level->PackNum(),
		       v_level->Music(),
		       v_level->isScripted(),
		       false);
    } catch(Exception &e) {
      vapp::Log("** Warning (just mean that the level has been updated if the level is in xmoto.bin) ** : %s (%s - %s)",
		e.getMsg().c_str(),
		v_level->Name().c_str(),
		v_level->FileName().c_str());
    }
    delete v_level;
  }

  i_db->levels_add_end();

  reloadExternalLevels(i_db, i_loadLevelsInterface);
}

void LevelsManager::checkPrerequires() {
  std::string LCachePath = vapp::FS::getUserDir() + std::string("/LCache");

  if(vapp::FS::isDir(LCachePath) == false) {
    vapp::FS::mkArborescenceDir(LCachePath);
  }

  if(vapp::FS::isDir(vapp::FS::getUserDir() + "/Levels/MyLevels") == false) {
    vapp::FS::mkArborescenceDir(vapp::FS::getUserDir() + "/Levels/MyLevels");
  }
}

void LevelsManager::cleanCache() {
  /* Find all .blv-files in the directory */
  std::vector<std::string> BlvFiles = vapp::FS::findPhysFiles("LCache/*.blv");
  for(int i=0; i<BlvFiles.size(); i++) {
    vapp::FS::deleteFile(BlvFiles[i]);
  }
}

std::string LevelsManager::LevelByFileName(xmDatabase *i_db, const std::string& i_fileName) {
    char **v_result;
    int nrow;
    std::string v_id_level;

    v_result = i_db->readDB("SELECT id_level FROM levels WHERE filepath=\"" +
			    xmDatabase::protectString(i_fileName) + "\";",
			    nrow);
    if(nrow == 0) {
      i_db->read_DB_free(v_result);
      throw Exception(std::string("Level " + i_fileName + " doesn't exist").c_str());
    }

    v_id_level = i_db->getResult(v_result, 1, 0, 0);

    i_db->read_DB_free(v_result);
    return v_id_level;
}

bool LevelsManager::doesLevelExist(xmDatabase *i_db,
				   const std::string& i_id) {
  char **v_result;
  int nrow;

  v_result = i_db->readDB("SELECT id_level "
			  "FROM levels "
			  "WHERE id_level=\"" + xmDatabase::protectString(i_id) + "\";",
			  nrow);
  i_db->read_DB_free(v_result);

  return nrow != 0;
}

void LevelsManager::printLevelsList(xmDatabase *i_db) const {
  char **v_result;
  int nrow;

  printf("%-40s %-40s\n", "Id", "Name");

  v_result = i_db->readDB("SELECT id_level, name FROM levels;", nrow);
  for(unsigned int i=0; i<nrow; i++) {
    printf("%-40s %-40s\n",
	   i_db->getResult(v_result, 2, i, 0),
	   i_db->getResult(v_result, 2, i, 1)
  	   );
  }
  i_db->read_DB_free(v_result);
}

void LevelsManager::updateLevelsFromLvl(xmDatabase *i_db,
					const std::vector<std::string> &NewLvl,
					const std::vector<std::string> &UpdatedLvl) {
  Level *v_level;

  i_db->levels_cleanNew();

  /* new */
  for(int i=0; i<NewLvl.size(); i++) {
    bool bCached = false;
    v_level = new Level();

    try {
      v_level->setFileName(NewLvl[i]);
      bCached = v_level->loadReducedFromFile();
      
      // Check for ID conflict
      if(doesLevelExist(i_db, v_level->Id())) {
	throw Exception("Duplicate level ID");
      }
      i_db->levels_add(v_level->Id(),
		       v_level->FileName(),
		       v_level->Name(),
		       v_level->Checksum(),
		       v_level->Author(),
		       v_level->Description(),
		       v_level->Date(),
		       v_level->Pack(),
		       v_level->PackNum(),
		       v_level->Music(),
		       v_level->isScripted(),
		       false);
      i_db->levels_addToNew(v_level->Id(), false);
    } catch(Exception &e) {
      vapp::Log("** Warning ** : %s", e.getMsg().c_str() );
    }
    delete v_level;
  }

  /* updated */
  for(int i=0; i<UpdatedLvl.size(); i++) {
    bool bCached = false;
    v_level = new Level();

    try {
      v_level->setFileName(UpdatedLvl[i]);
      bCached = v_level->loadReducedFromFile();

      i_db->levels_update(v_level->Id(),
			  v_level->FileName(),
			  v_level->Name(),
			  v_level->Checksum(),
			  v_level->Author(),
			  v_level->Description(),
			  v_level->Date(),
			  v_level->Pack(),
			  v_level->PackNum(),
			  v_level->Music(),
			  v_level->isScripted(),
			  false);
      i_db->levels_addToNew(v_level->Id(), true);

    } catch(Exception &e) {
      vapp::Log("** Warning ** : %s", e.getMsg().c_str() );
    }
    delete v_level;
  }
}

void LevelsManager::addToFavorite(xmDatabase *i_db, std::string i_profile,
				  const std::string& i_id_level) {
  int nrow;
  char **v_result;
  int v_n;

  /* check if the level is already into the favorite list */
  v_result = i_db->readDB("SELECT COUNT(id_level) "
			  "FROM levels_favorite "
			  "WHERE id_level=\""   + i_db->protectString(i_id_level) + "\" "
			  "AND   id_profile=\"" + i_db->protectString(i_profile) + "\";",
			  nrow);
  v_n = atoi(i_db->getResult(v_result, 1, 0, 0));
  i_db->read_DB_free(v_result);

  if(v_n != 0) {
    return;
  }

  i_db->levels_addToFavorite(i_profile, i_id_level);
}

void LevelsManager::delFromFavorite(xmDatabase *i_db, std::string i_profile,
				    const std::string& i_id_level) {
  int nrow;
  char **v_result;
  int v_n;  

  /* check if the level is into the favorite list */
  v_result = i_db->readDB("SELECT COUNT(id_level) "
			  "FROM levels_favorite "
			  "WHERE id_level=\""   + i_db->protectString(i_id_level) + "\""
			  "AND   id_profile=\"" + i_db->protectString(i_profile)  + "\";",
			  nrow);  
  v_n = atoi(i_db->getResult(v_result, 1, 0, 0));
  i_db->read_DB_free(v_result);

  if(v_n == 0) {
    return;
  }

  i_db->levels_delToFavorite(i_profile, i_id_level);
}

std::string LevelsManager::getQuickStartPackQuery(xmDatabase *i_db,
						  unsigned int i_qualityMIN, unsigned int i_difficultyMIN,
						  unsigned int i_qualityMAX, unsigned int i_difficultyMAX,
						  const std::string& i_profile, const std::string& i_id_room) {
  /* SELECT id_level, name, profile_best_finishTime, web_highscore */
  char **v_result;
  int nrow;
  char *v_res;

  /* if xmoto run only 1 time, run the tutorial pack */
  bool v_tutorials = false;
  v_result = i_db->readDB("SELECT nbStarts "
			  "FROM stats_profiles "
			  "WHERE id_profile=\"" + xmDatabase::protectString(i_profile) + "\";",
			  nrow);
  if(nrow == 0) {
    v_tutorials = true;
  } else {
    v_res = i_db->getResult(v_result, 1, 0, 0);
    if(atoi(v_res) < 2) {
      v_tutorials = true;
    }
  }
  i_db->read_DB_free(v_result);

  if(v_tutorials) {
    return
      "SELECT a.id_level, MIN(a.name), MIN(b.finishTime), MIN(c.finishTime) "
      "FROM levels AS a "
      "LEFT OUTER JOIN webhighscores AS b ON (a.id_level = b.id_level AND b.id_room=" + i_id_room + ") "
      "LEFT OUTER JOIN profile_completedLevels AS c "
      "ON (a.id_level=c.id_level AND c.id_profile=\"" + xmDatabase::protectString(i_profile) + "\") "
      "WHERE a.packName=\"Tutorials\" "
      "GROUP BY a.id_level "
      "ORDER BY a.packNum || UPPER(a.name);";
  }

  /* run the query to check wether there are at least 5 levels, else, all levels randomly */
  /* web quality and difficulty are >=1 and <=5 */
  bool v_haveEnoughLevels;
  std::ostringstream v_qualityMINStr;
  std::ostringstream v_difficultyMINStr;
  std::ostringstream v_qualityMAXStr;
  std::ostringstream v_difficultyMAXStr;
  v_qualityMINStr    << i_qualityMIN;
  v_difficultyMINStr << i_difficultyMIN;
  v_qualityMAXStr    << i_qualityMAX;
  v_difficultyMAXStr << i_difficultyMAX;

  v_result = i_db->readDB("SELECT count(1) "
			  "FROM levels AS a "
			  "INNER JOIN weblevels AS b ON a.id_level = b.id_level "
			  "WHERE b.quality    <= " + v_qualityMAXStr.str()    + " "
			  "AND   b.quality    >= " + v_qualityMINStr.str()    + " "
			  "AND   b.difficulty <= " + v_difficultyMAXStr.str() + " "
			  "AND   b.difficulty >= " + v_difficultyMINStr.str() + " "
			  "ORDER BY RANDOM();",
			  nrow);
  v_res = i_db->getResult(v_result, 1, 0, 0);
  v_haveEnoughLevels = atoi(v_res) >= 5; /* less than 5 levels ? */
  i_db->read_DB_free(v_result);

  if(v_haveEnoughLevels) {
    return
      "SELECT a.id_level, MIN(a.name), MIN(c.finishTime), MIN(d.finishTime) "
      "FROM levels AS a "
      "INNER JOIN weblevels AS b ON a.id_level = b.id_level "
      "LEFT OUTER JOIN webhighscores AS c ON (a.id_level = c.id_level AND c.id_room=" + i_id_room + ") "
      "LEFT OUTER JOIN profile_completedLevels AS d "
      "ON (a.id_level=d.id_level AND d.id_profile=\"" + xmDatabase::protectString(i_profile) + "\") "
      "WHERE b.quality    <= " + v_qualityMAXStr.str()    + " "
      "AND   b.quality    >= " + v_qualityMINStr.str()    + " "
      "AND   b.difficulty <= " + v_difficultyMAXStr.str() + " "
      "AND   b.difficulty >= " + v_difficultyMINStr.str() + " "
      "GROUP BY a.id_level ORDER BY RANDOM();";
  } else {
    /* all levels randomly */
    return
    "SELECT a.id_level, MIN(a.name), MIN(b.finishTime), MIN(c.finishTime) "
    "FROM levels AS a "
    "LEFT OUTER JOIN webhighscores AS b ON (a.id_level = b.id_level AND b.id_room=" + i_id_room + ") "
    "LEFT OUTER JOIN profile_completedLevels AS c "
    "ON (a.id_level=c.id_level AND c.id_profile=\"" + xmDatabase::protectString(i_profile) + "\") "
    "GROUP BY a.id_level ORDER BY RANDOM();";
  }
}
