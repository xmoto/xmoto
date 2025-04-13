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

#include "LevelsManager.h"
#include "GameText.h"
#include "SysMessage.h"
#include "common/VFileIO.h"
#include "common/VXml.h"
#include "common/WWWAppInterface.h"
#include "db/xmDatabase.h"
#include "helpers/Log.h"
#include "sqlqueries.h"
#include <algorithm>
#include <sstream>
#include <time.h>

LevelsPack::LevelsPack(std::string i_name,
                       const std::string &i_sql,
                       bool i_ascSort) {
  m_name = i_name;
  m_showTimes = true;
  m_showWebTimes = true;
  m_sql_levels = i_sql;
  m_ascSort = i_ascSort;
  m_nbLevels = -1;
  m_nbFinishedLevels = -1;
}

LevelsPack::~LevelsPack() {}

void LevelsPack::updateCount(xmDatabase *i_db, const std::string &i_profile) {
  char **v_result;
  unsigned int nrow;

  /* number of levels*/
  v_result =
    i_db->readDB("SELECT count(id_level) FROM (" + m_sql_levels + ");", nrow);

  if (i_db->getResult(v_result, 1, 0, 0) == NULL) {
    i_db->read_DB_free(v_result);
    throw Exception("Unable to update level pack count");
  }

  m_nbLevels = atoi(i_db->getResult(v_result, 1, 0, 0));
  i_db->read_DB_free(v_result);

  /* finished levels */
  v_result = i_db->readDB(
    "SELECT count(1) FROM (SELECT a.id_level FROM (" + m_sql_levels +
      ") AS a INNER JOIN stats_profiles_levels AS b ON a.id_level=b.id_level "
      "WHERE b.id_profile=\"" +
      xmDatabase::protectString(i_profile) +
      "\" AND b.nbCompleted+0 > 0 "
      "GROUP BY a.id_level);",
    nrow);

  if (nrow != 1) {
    i_db->read_DB_free(v_result);
    throw Exception("Unable to update level pack count");
  }
  m_nbFinishedLevels = atoi(i_db->getResult(v_result, 1, 0, 0));
  i_db->read_DB_free(v_result);
}

int LevelsPack::getNumberOfLevels() {
  return m_nbLevels;
}

std::string LevelsPack::getLevelsQuery() const {
  return "SELECT id_level, name FROM (" + m_sql_levels +
         ") ORDER BY sort_field " + std::string(m_ascSort ? "ASC" : "DESC") +
         ";";
}

std::string LevelsPack::getLevelsWithHighscoresQuery(
  const std::string &i_profile,
  const std::string &i_id_room) const {
  return "SELECT a.id_level AS id_level, MIN(a.name) AS name, "
         "MIN(c.finishTime+0), MIN(b.finishTime+0) "
         "FROM (" +
         m_sql_levels +
         ") AS a "
         "LEFT OUTER JOIN webhighscores AS b "
         "ON (a.id_level = b.id_level AND b.id_room=" +
         i_id_room +
         ") "
         "LEFT OUTER JOIN profile_completedLevels AS c "
         "ON (a.id_level=c.id_level AND c.id_profile=\"" +
         xmDatabase::protectString(i_profile) +
         "\") "
         "GROUP BY a.id_level ORDER BY MIN(a.sort_field) " +
         std::string(m_ascSort ? "ASC" : "DESC") + ";";
}

int LevelsPack::getNumberOfFinishedLevels() {
  return m_nbFinishedLevels;
}

std::string LevelsPack::Name() const {
  return m_name;
}

void LevelsPack::setName(const std::string &i_name) {
  m_name = i_name;
}

void LevelsPack::setDescription(const std::string &i_description) {
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
  m_levelsPackMutex = SDL_CreateMutex();
}

LevelsManager::~LevelsManager() {
  clean();
  SDL_DestroyMutex(m_levelsPackMutex);
}

void LevelsManager::clean() {
  cleanPacks();
}

void LevelsManager::cleanPacks() {
  /* delete existing packs */
  for (unsigned int i = 0; i < m_levelsPacks.size(); i++) {
    delete m_levelsPacks[i];
  }
  m_levelsPacks.clear();
}

LevelsPack &LevelsManager::LevelsPackByName(const std::string &i_name) {
  for (unsigned int i = 0; i < m_levelsPacks.size(); i++) {
    if (m_levelsPacks[i]->Name() == i_name) {
      return *m_levelsPacks[i];
    }
  }
  throw Exception(std::string("Level pack " + i_name + " not found").c_str());
}

bool LevelsManager::doesLevelsPackExist(const std::string &i_name) const {
  for (unsigned int i = 0; i < m_levelsPacks.size(); i++) {
    if (m_levelsPacks[i]->Name() == i_name) {
      return true;
    }
  }
  return false;
}

void LevelsManager::makePacks_add(const std::string &i_pack_name,
                                  const std::string &i_sql,
                                  const std::string &i_group_name,
                                  const std::string &i_pack_description,
                                  bool i_ascSort) {
  LevelsPack *v_pack;

  v_pack = new LevelsPack(i_pack_name, i_sql, i_ascSort);
  v_pack->setGroup(i_group_name);
  v_pack->setDescription(i_pack_description);
  m_levelsPacks.push_back(v_pack);
}

void LevelsManager::makePacks(const std::string &i_playerName,
                              const std::string &i_id_room,
                              bool i_bDebugMode,
                              bool i_bAdminMode,
                              xmDatabase *i_db) {
  LevelsPack *v_pack;
  char **v_result;
  unsigned int nrow;

  lockLevelsPacks();
  cleanPacks();

  /* standard packs */
  v_result = i_db->readDB("SELECT DISTINCT packName FROM weblevels WHERE "
                          "packName<>'' ORDER BY UPPER(packName);",
                          nrow);
  for (unsigned int i = 0; i < nrow; i++) {
    char v_levelPackStr[256];

    v_pack = new LevelsPack(
      i_db->getResult(v_result, 1, i, 0),
      "SELECT a.id_level AS id_level, a.name AS name, b.packNum || "
      "UPPER(a.name) AS sort_field "
      "FROM levels AS a "
      "INNER JOIN weblevels AS b ON a.id_level=b.id_level "
      "LEFT OUTER JOIN levels_blacklist AS c ON (a.id_level = c.id_level AND "
      "c.id_profile=xm_profile()) "
      "WHERE b.packName=\"" +
        xmDatabase::protectString(i_db->getResult(v_result, 1, i, 0)) +
        "\" "
        "AND (b.crappy IS NULL OR xm_userCrappy(b.crappy)=0) "
        "AND (b.children_compliant IS NULL OR "
        "xm_userChildrenCompliant(b.children_compliant)=1) "
        "AND c.id_level IS NULL");
    v_pack->setGroup(GAMETEXT_PACK_STANDARD);

    snprintf(v_levelPackStr,
             256,
             VPACKAGENAME_DESC_STANDARD,
             i_db->getResult(v_result, 1, i, 0));
    v_pack->setDescription(v_levelPackStr);
    m_levelsPacks.push_back(v_pack);
  }
  i_db->read_DB_free(v_result);

  /* add all packages */
  makePacks_add(VPACKAGENAME_ALL_LEVELS,
                QUERY_LVL_ALL,
                GAMETEXT_PACK_SPECIAL,
                VPACKAGENAME_DESC_ALL_LEVELS);
  makePacks_add(VPACKAGENAME_MY_LEVELS,
                QUERY_LVL_MY,
                GAMETEXT_PACK_SPECIAL,
                VPACKAGENAME_DESC_MY_LEVELS);
  makePacks_add(VPACKAGENAME_FAVORITE_LEVELS,
                QUERY_LVL_FAVORITES,
                GAMETEXT_PACK_SPECIAL,
                VPACKAGENAME_DESC_FAVORITE_LEVELS);
  makePacks_add(VPACKAGENAME_BLACKLIST_LEVELS,
                QUERY_LVL_BLACKLIST,
                GAMETEXT_PACK_SPECIAL,
                VPACKAGENAME_DESC_BLACKLIST_LEVELS);
  makePacks_add(VPACKAGENAME_SCRIPTED,
                QUERY_LVL_SCRIPTED,
                GAMETEXT_PACK_SPECIAL,
                VPACKAGENAME_DESC_SCRIPTED);
  makePacks_add(VPACKAGENAME_PHYSICS,
                QUERY_LVL_PHYSICS,
                GAMETEXT_PACK_SPECIAL,
                VPACKAGENAME_DESC_PHYSICS);
  makePacks_add(VPACKAGENAME_INCOMPLETED_LEVELS,
                QUERY_LVL_INCOMPLETED,
                GAMETEXT_PACK_SPECIAL,
                VPACKAGENAME_DESC_INCOMPLETED_LEVELS);
  makePacks_add(VPACKAGENAME_NEW_LEVELS,
                QUERY_LVL_NEW,
                GAMETEXT_PACK_SPECIAL,
                VPACKAGENAME_DESC_NEW_LEVELS);
  makePacks_add(VPACKAGENAME_CRAPPY_LEVELS,
                QUERY_LVL_CRAPPY,
                GAMETEXT_PACK_SPECIAL,
                VPACKAGENAME_DESC_CRAPPY_LEVELS);
  makePacks_add(VPACKAGENAME_LAST_LEVELS,
                QUERY_LVL_LAST,
                GAMETEXT_PACK_SPECIAL,
                VPACKAGENAME_DESC_LAST_LEVELS,
                false /* not ok */);
  makePacks_add(VPACKAGENAME_LEVELS_STOLEN,
                QUERY_LVL_STOLEN,
                GAMETEXT_PACK_ROOM,
                VPACKAGENAME_DESC_LEVELS_STOLEN,
                false /* not ok */);
  makePacks_add(VPACKAGENAME_LEVELS_WITH_NO_HIGHSCORE,
                QUERY_LVL_NO_HIGHSCORE,
                GAMETEXT_PACK_ROOM,
                VPACKAGENAME_DESC_LEVELS_WITH_NO_HIGHSCORE);
  makePacks_add(VPACKAGENAME_YOU_HAVE_NOT_THE_HIGHSCORE,
                QUERY_LVL_NOT_THE_HIGHSCORE,
                GAMETEXT_PACK_ROOM,
                VPACKAGENAME_DESC_YOU_HAVE_NOT_THE_HIGHSCORE);
  makePacks_add(VPACKAGENAME_LAST_HIGHSCORES,
                QUERY_LVL_LAST_HIGHSCORES,
                GAMETEXT_PACK_ROOM,
                VPACKAGENAME_DESC_LAST_HIGHSCORES,
                false /* not ok */);
  makePacks_add(VPACKAGENAME_OLDEST_HIGHSCORES,
                QUERY_LVL_OLDEST_HIGHSCORES,
                GAMETEXT_PACK_ROOM,
                VPACKAGENAME_DESC_OLDEST_HIGHSCORES);

  // STD PACKNAME
  // VPACKAGENAME_MEDAL_PLATINIUM
  // VPACKAGENAME_MEDAL_GOLD
  // VPACKAGENAME_MEDAL_SILVER
  // VPACKAGENAME_MEDAL_BRONZE
  // VPACKAGENAME_MEDAL_NONE
  // VPACKAGENAME_LAST_PLAYED
  // VPACKAGENAME_NEVER_PLAYED
  // VPACKAGENAME_MOST_PLAYED
  // VPACKAGENAME_LESS_PLAYED
  // VPACKAGENAME_NICEST_LEVELS
  // VPACKAGENAME_CRAPPIEST_LEVELS
  // VPACKAGENAME_EASIEST_LEVELS
  // VPACKAGENAME_HARDEST_LEVELS
  // VPACKAGENAME_VOTE_NOT_LOCKED_LEVELS
  // VPACKAGENAME_SHORT_LEVELS
  // VPACKAGENAME_MEDIUM_LEVELS
  // VPACKAGENAME_LONG_LEVELS
  // VPACKAGENAME_VERY_LONG_LEVELS
  // BY PROFILE

  /* rooms */

  /* medals */
  v_pack = new LevelsPack(
    std::string(VPACKAGENAME_MEDAL_PLATINIUM),
    "SELECT b.id_level AS id_level, b.name AS name, UPPER(b.name) AS "
    "sort_field "
    "FROM webhighscores AS a INNER JOIN levels AS b "
    "ON (a.id_level = b.id_level "
    "AND a.id_room=" +
      i_id_room +
      ") "
      "LEFT OUTER JOIN weblevels AS c ON a.id_level=c.id_level "
      "LEFT OUTER JOIN levels_blacklist AS d ON (a.id_level = "
      "d.id_level AND d.id_profile=\"" +
      xmDatabase::protectString(i_playerName) +
      "\") "
      "WHERE a.id_profile = \"" +
      xmDatabase::protectString(i_playerName) +
      "\" "
      "AND d.id_level IS NULL "
      "AND (c.crappy IS NULL OR xm_userCrappy(c.crappy)=0) "
      "AND (c.children_compliant IS NULL OR "
      "xm_userChildrenCompliant(c.children_compliant)=1)");
  v_pack->setGroup(GAMETEXT_PACK_MEDALS);
  v_pack->setDescription(VPACKAGENAME_DESC_MEDAL_PLATINIUM);
  m_levelsPacks.push_back(v_pack);

  v_pack =
    new LevelsPack(std::string(VPACKAGENAME_MEDAL_GOLD),
                   "SELECT id_level, name, sort_field FROM ("
                   "SELECT a.id_level AS id_level, MAX(b.name) AS name, "
                   "UPPER(MAX(b.name)) AS sort_field, "
                   "MIN(c.finishTime+0) AS webFinishTime, MIN(a.finishTime+0) "
                   "AS userFinishTime "
                   "FROM profile_completedLevels AS a INNER JOIN levels AS b "
                   "ON (a.id_level = b.id_level "
                   "AND a.id_profile=\"" +
                     xmDatabase::protectString(i_playerName) +
                     "\") "
                     "LEFT OUTER JOIN webhighscores AS c "
                     "ON (c.id_level = a.id_level AND c.id_room=" +
                     i_id_room +
                     ") "
                     "LEFT OUTER JOIN weblevels AS d ON a.id_level=d.id_level "
                     "LEFT OUTER JOIN levels_blacklist AS e ON (a.id_level = "
                     "e.id_level AND e.id_profile=\"" +
                     xmDatabase::protectString(i_playerName) +
                     "\") "
                     "WHERE (d.crappy IS NULL OR xm_userCrappy(d.crappy)=0) "
                     "AND (d.children_compliant IS NULL OR "
                     "xm_userChildrenCompliant(d.children_compliant)=1) "
                     "AND e.id_level IS NULL "
                     "GROUP BY a.id_level " /* get the best highscores */
                     ") "
                     "WHERE (webFinishTime IS NULL OR webFinishTime+0 >= "
                     "userFinishTime*0.95) "
                     "EXCEPT " /* remove the webhighscores where the player's
                                  name is the same */
                     "SELECT b.id_level AS id_level, b.name AS name, "
                     "UPPER(b.name) AS sort_field "
                     "FROM webhighscores AS a INNER JOIN levels AS b "
                     "ON (a.id_level = b.id_level "
                     "AND a.id_room=" +
                     i_id_room +
                     ") "
                     "WHERE a.id_profile = \"" +
                     xmDatabase::protectString(i_playerName) + "\"");
  v_pack->setGroup(GAMETEXT_PACK_MEDALS);
  v_pack->setDescription(VPACKAGENAME_DESC_MEDAL_GOLD);
  m_levelsPacks.push_back(v_pack);

  v_pack =
    new LevelsPack(std::string(VPACKAGENAME_MEDAL_SILVER),
                   "SELECT id_level, name, sort_field FROM ("
                   "SELECT a.id_level AS id_level, MAX(b.name) AS name, "
                   "UPPER(MAX(b.name)) AS sort_field, "
                   "MIN(c.finishTime+0) AS webFinishTime, MIN(a.finishTime+0) "
                   "AS userFinishTime "
                   "FROM profile_completedLevels AS a INNER JOIN levels AS b "
                   "ON (a.id_level = b.id_level "
                   "AND a.id_profile=\"" +
                     xmDatabase::protectString(i_playerName) +
                     "\") "
                     "LEFT OUTER JOIN webhighscores AS c "
                     "ON (c.id_level = a.id_level AND c.id_room=" +
                     i_id_room +
                     ") "
                     "LEFT OUTER JOIN weblevels AS d ON a.id_level=d.id_level "
                     "LEFT OUTER JOIN levels_blacklist AS e ON (a.id_level = "
                     "e.id_level AND e.id_profile=\"" +
                     xmDatabase::protectString(i_playerName) +
                     "\") "
                     "WHERE (d.crappy IS NULL OR xm_userCrappy(d.crappy)=0) "
                     "AND (d.children_compliant IS NULL OR "
                     "xm_userChildrenCompliant(d.children_compliant)=1) "
                     "AND e.id_level IS NULL "
                     "GROUP BY a.id_level " /* get the best highscores */
                     ") "
                     "WHERE (webFinishTime IS NOT NULL "
                     "AND webFinishTime+0 >= userFinishTime*0.90 "
                     "AND webFinishTime+0 < userFinishTime*0.95) "
                     "EXCEPT " /* remove the webhighscores where the player's
                                  name is the same */
                     "SELECT b.id_level AS id_level, b.name AS name, "
                     "UPPER(b.name) AS sort_field "
                     "FROM webhighscores AS a INNER JOIN levels AS b "
                     "ON (a.id_level = b.id_level "
                     "AND a.id_room=" +
                     i_id_room +
                     ") "
                     "WHERE a.id_profile = \"" +
                     xmDatabase::protectString(i_playerName) + "\"");
  v_pack->setGroup(GAMETEXT_PACK_MEDALS);
  v_pack->setDescription(VPACKAGENAME_DESC_MEDAL_SILVER);
  m_levelsPacks.push_back(v_pack);

  v_pack =
    new LevelsPack(std::string(VPACKAGENAME_MEDAL_BRONZE),
                   "SELECT id_level, name, sort_field FROM ("
                   "SELECT a.id_level AS id_level, MAX(b.name) AS name, "
                   "UPPER(MAX(b.name)) AS sort_field, "
                   "MIN(c.finishTime+0) AS webFinishTime, MIN(a.finishTime+0) "
                   "AS userFinishTime "
                   "FROM profile_completedLevels AS a INNER JOIN levels AS b "
                   "ON (a.id_level = b.id_level "
                   "AND a.id_profile=\"" +
                     xmDatabase::protectString(i_playerName) +
                     "\") "
                     "LEFT OUTER JOIN webhighscores AS c "
                     "ON (c.id_level = a.id_level AND c.id_room=" +
                     i_id_room +
                     ") "
                     "LEFT OUTER JOIN weblevels AS d ON a.id_level=d.id_level "
                     "LEFT OUTER JOIN levels_blacklist AS e ON (a.id_level = "
                     "e.id_level AND e.id_profile=\"" +
                     xmDatabase::protectString(i_playerName) +
                     "\") "
                     "WHERE (d.crappy IS NULL OR xm_userCrappy(d.crappy)=0) "
                     "AND (d.children_compliant IS NULL OR "
                     "xm_userChildrenCompliant(d.children_compliant)=1) "
                     "AND e.id_level IS NULL "
                     "GROUP BY a.id_level " /* get the best highscores */
                     ") "
                     "WHERE (webFinishTime IS NOT NULL "
                     "AND webFinishTime+0 >= userFinishTime*0.80 "
                     "AND webFinishTime+0 < userFinishTime*0.90) "
                     "EXCEPT " /* remove the webhighscores where the player's
                                  name is the same */
                     "SELECT b.id_level AS id_level, b.name AS name, "
                     "UPPER(b.name) AS sort_field "
                     "FROM webhighscores AS a INNER JOIN levels AS b "
                     "ON (a.id_level = b.id_level "
                     "AND a.id_room=" +
                     i_id_room +
                     ") "
                     "WHERE a.id_profile = \"" +
                     xmDatabase::protectString(i_playerName) + "\"");
  v_pack->setGroup(GAMETEXT_PACK_MEDALS);
  v_pack->setDescription(VPACKAGENAME_DESC_MEDAL_BRONZE);
  m_levelsPacks.push_back(v_pack);

  v_pack = new LevelsPack(
    std::string(VPACKAGENAME_MEDAL_NONE),
    "SELECT a.id_level AS id_level, a.name AS name, "
    "UPPER(a.name) AS sort_field "
    "FROM levels AS a "
    "LEFT OUTER JOIN (SELECT id_level, MIN(finishTime+0) AS userFinishTime "
    "FROM profile_completedLevels WHERE id_profile=\"" +
      xmDatabase::protectString(i_playerName) +
      "\" GROUP BY id_level) b ON a.id_level = b.id_level "
      "LEFT OUTER JOIN (SELECT id_level, MIN(finishTime+0) AS webFinishTime "
      "FROM webhighscores WHERE id_room=" +
      i_id_room +
      " GROUP BY id_level) c ON a.id_level = c.id_level "
      "WHERE "
      "(a.id_level IN (SELECT id_level FROM weblevels WHERE "
      "(crappy IS NULL OR xm_userCrappy(crappy)=0) "
      "AND (children_compliant IS NULL OR "
      "xm_userChildrenCompliant(children_compliant)=1)) OR "
      "a.id_level NOT IN (SELECT id_level FROM weblevels)) " // is local level
                                                             // only
      "AND a.id_level NOT IN (SELECT id_level FROM levels_blacklist WHERE "
      "id_profile=\"" +
      xmDatabase::protectString(i_playerName) +
      "\") "
      "AND ((webFinishTime IS NOT NULL "
      "AND webFinishTime+0 < userFinishTime*0.80) "
      "OR userFinishTime IS NULL) "
      "AND a.id_level NOT IN ( " /* remove the webhighscores where the player's
                   name is the same */
      "SELECT b.id_level "
      "FROM webhighscores AS a INNER JOIN levels AS b "
      "ON (a.id_level = b.id_level "
      "AND a.id_room=" +
      i_id_room +
      ") "
      "WHERE a.id_profile = \"" +
      xmDatabase::protectString(i_playerName) + "\")");
  v_pack->setGroup(GAMETEXT_PACK_MEDALS);
  v_pack->setDescription(VPACKAGENAME_DESC_MEDAL_NONE);
  m_levelsPacks.push_back(v_pack);

  /* stats */
  /* last played levels */
  v_pack =
    new LevelsPack(std::string(VPACKAGENAME_LAST_PLAYED),
                   "SELECT a.id_level AS id_level, MIN(a.name) AS name, "
                   "MAX(b.last_play_date) AS sort_field "
                   "FROM levels AS a INNER JOIN stats_profiles_levels AS b "
                   "ON (a.id_level=b.id_level AND "
                   "b.id_profile=\"" +
                     xmDatabase::protectString(i_playerName) +
                     "\") "
                     "LEFT OUTER JOIN weblevels AS c ON a.id_level=c.id_level "
                     "LEFT OUTER JOIN levels_blacklist AS d ON (a.id_level = "
                     "d.id_level AND d.id_profile=\"" +
                     xmDatabase::protectString(i_playerName) +
                     "\") "
                     "WHERE b.last_play_date IS NOT NULL "
                     "AND (c.crappy IS NULL OR xm_userCrappy(c.crappy)=0) "
                     "AND (c.children_compliant IS NULL OR "
                     "xm_userChildrenCompliant(c.children_compliant)=1) "
                     "AND d.id_level IS NULL "
                     "GROUP BY a.id_level, b.id_profile "
                     "ORDER BY MAX(b.last_play_date) DESC "
                     "LIMIT 100",
                   false);
  v_pack->setGroup(GAMETEXT_PACK_STATS);
  v_pack->setDescription(VPACKAGENAME_DESC_LAST_PLAYED);
  m_levelsPacks.push_back(v_pack);

  /* never played levels */
  v_pack = new LevelsPack(
    std::string(VPACKAGENAME_NEVER_PLAYED),
    "SELECT a.id_level AS id_level, a.name AS name, UPPER(a.name) AS "
    "sort_field "
    "FROM levels AS a LEFT OUTER JOIN stats_profiles_levels AS b "
    "ON (a.id_level=b.id_level AND "
    "b.id_profile=\"" +
      xmDatabase::protectString(i_playerName) +
      "\") "
      "LEFT OUTER JOIN weblevels AS c ON a.id_level=c.id_level "
      "LEFT OUTER JOIN levels_blacklist AS d ON (a.id_level = d.id_level AND "
      "d.id_profile=\"" +
      xmDatabase::protectString(i_playerName) +
      "\") "
      "WHERE b.id_profile IS NULL "
      "AND d.id_level IS NULL "
      "AND (c.crappy IS NULL OR xm_userCrappy(c.crappy)=0)"
      "AND (c.children_compliant IS NULL OR "
      "xm_userChildrenCompliant(c.children_compliant)=1)");
  v_pack->setGroup(GAMETEXT_PACK_STATS);
  v_pack->setDescription(VPACKAGENAME_DESC_NEVER_PLAYED);
  m_levelsPacks.push_back(v_pack);

  /* most played levels */
  v_pack =
    new LevelsPack(std::string(VPACKAGENAME_MOST_PLAYED),
                   "SELECT a.id_level AS id_level, MIN(a.name) AS name, "
                   "SUM(b.nbPlayed+0) AS sort_field "
                   "FROM levels AS a INNER JOIN stats_profiles_levels AS b "
                   "ON (a.id_level=b.id_level AND "
                   "b.id_profile=\"" +
                     xmDatabase::protectString(i_playerName) +
                     "\") "
                     "LEFT OUTER JOIN weblevels AS c ON a.id_level=c.id_level "
                     "LEFT OUTER JOIN levels_blacklist AS d ON (a.id_level = "
                     "d.id_level AND d.id_profile=\"" +
                     xmDatabase::protectString(i_playerName) +
                     "\") "
                     "WHERE (c.crappy IS NULL OR xm_userCrappy(c.crappy)=0) "
                     "AND (c.children_compliant IS NULL OR "
                     "xm_userChildrenCompliant(c.children_compliant)=1) "
                     "AND d.id_level IS NULL "
                     "GROUP BY a.id_level, b.id_profile "
                     "ORDER BY MAX(b.nbPlayed+0) DESC LIMIT 100",
                   false);
  v_pack->setGroup(GAMETEXT_PACK_STATS);
  v_pack->setDescription(VPACKAGENAME_DESC_MOST_PLAYED);
  m_levelsPacks.push_back(v_pack);

  /* less played levels */
  v_pack =
    new LevelsPack(std::string(VPACKAGENAME_LESS_PLAYED),
                   "SELECT a.id_level AS id_level, MIN(a.name) AS name, "
                   "SUM(b.nbPlayed+0) AS sort_field "
                   "FROM levels AS a INNER JOIN stats_profiles_levels AS b "
                   "ON (a.id_level=b.id_level AND "
                   "b.id_profile=\"" +
                     xmDatabase::protectString(i_playerName) +
                     "\") "
                     "LEFT OUTER JOIN weblevels AS c ON a.id_level=c.id_level "
                     "LEFT OUTER JOIN levels_blacklist AS d ON (a.id_level = "
                     "d.id_level AND d.id_profile=\"" +
                     xmDatabase::protectString(i_playerName) +
                     "\") "
                     "WHERE b.nbPlayed+0 > 0 "
                     "AND (c.crappy IS NULL OR xm_userCrappy(c.crappy)=0) "
                     "AND (c.children_compliant IS NULL OR "
                     "xm_userChildrenCompliant(c.children_compliant)=1) "
                     "AND d.id_level IS NULL "
                     "GROUP BY a.id_level, b.id_profile "
                     "ORDER BY MIN(b.nbPlayed+0) ASC LIMIT 100");
  v_pack->setGroup(GAMETEXT_PACK_STATS);
  v_pack->setDescription(VPACKAGENAME_DESC_LESS_PLAYED);
  m_levelsPacks.push_back(v_pack);

  /* nicest levels */
  v_pack = new LevelsPack(
    std::string(VPACKAGENAME_NICEST_LEVELS),
    "SELECT a.id_level AS id_level, a.name AS name, b.quality+0 AS sort_field "
    "FROM levels AS a INNER JOIN weblevels AS b ON a.id_level=b.id_level "
    "LEFT OUTER JOIN levels_blacklist AS c ON (a.id_level = c.id_level AND "
    "c.id_profile=\"" +
      xmDatabase::protectString(i_playerName) +
      "\") "
      "WHERE b.quality+0 >= 4.0 "
      "AND (b.crappy IS NULL OR xm_userCrappy(b.crappy)=0) "
      "AND (b.children_compliant IS NULL OR "
      "xm_userChildrenCompliant(b.children_compliant)=1) "
      "AND c.id_level IS NULL "
      "ORDER BY b.quality+0 DESC LIMIT 100",
    false);
  v_pack->setGroup(GAMETEXT_PACK_WEBVOTES);
  v_pack->setDescription(VPACKAGENAME_DESC_NICEST_LEVELS);
  m_levelsPacks.push_back(v_pack);

  /* crappiest levels */
  if (i_bDebugMode) {
    v_pack = new LevelsPack(
      std::string(VPACKAGENAME_CRAPPIEST_LEVELS),
      "SELECT a.id_level AS id_level, a.name AS name, b.quality+0 AS "
      "sort_field "
      "FROM levels AS a INNER JOIN weblevels AS b ON a.id_level=b.id_level "
      "LEFT OUTER JOIN levels_blacklist AS c ON (a.id_level = c.id_level AND "
      "c.id_profile=\"" +
        xmDatabase::protectString(i_playerName) +
        "\") "
        "WHERE b.quality+0 <= 2.0 "
        "AND (b.crappy IS NULL OR xm_userCrappy(b.crappy)=0) "
        "AND (b.children_compliant IS NULL OR "
        "xm_userChildrenCompliant(b.children_compliant)=1) "
        "AND c.id_level IS NULL "
        "ORDER BY b.quality+0 ASC LIMIT 100");
    v_pack->setGroup(GAMETEXT_PACK_WEBVOTES);
    v_pack->setDescription(VPACKAGENAME_DESC_CRAPPIEST_LEVELS);
    m_levelsPacks.push_back(v_pack);
  }

  /* easiest levels */
  v_pack = new LevelsPack(
    std::string(VPACKAGENAME_EASIEST_LEVELS),
    "SELECT a.id_level AS id_level, a.name AS name, b.difficulty+0 AS "
    "sort_field "
    "FROM levels AS a INNER JOIN weblevels AS b ON a.id_level=b.id_level "
    "LEFT OUTER JOIN levels_blacklist AS c ON (a.id_level = c.id_level AND "
    "c.id_profile=\"" +
      xmDatabase::protectString(i_playerName) +
      "\") "
      "WHERE b.difficulty+0 <= 3.0 "
      "AND (b.crappy IS NULL OR xm_userCrappy(b.crappy)=0) "
      "AND (b.children_compliant IS NULL OR "
      "xm_userChildrenCompliant(b.children_compliant)=1) "
      "AND c.id_level IS NULL "
      "ORDER BY b.difficulty+0 ASC LIMIT 100");
  v_pack->setGroup(GAMETEXT_PACK_WEBVOTES);
  v_pack->setDescription(VPACKAGENAME_DESC_EASIEST_LEVELS);
  m_levelsPacks.push_back(v_pack);

  /* hardest levels */
  v_pack = new LevelsPack(
    std::string(VPACKAGENAME_HARDEST_LEVELS),
    "SELECT a.id_level AS id_level, a.name AS name, b.difficulty+0 AS "
    "sort_field "
    "FROM levels AS a INNER JOIN weblevels AS b ON a.id_level=b.id_level "
    "LEFT OUTER JOIN levels_blacklist AS c ON (a.id_level = c.id_level AND "
    "c.id_profile=\"" +
      xmDatabase::protectString(i_playerName) +
      "\") "
      "WHERE b.difficulty+0 >= 3.0 "
      "AND (b.crappy IS NULL OR xm_userCrappy(b.crappy)=0) "
      "AND (b.children_compliant IS NULL OR "
      "xm_userChildrenCompliant(b.children_compliant)=1) "
      "AND c.id_level IS NULL "
      "ORDER BY b.difficulty+0 DESC LIMIT 100",
    false);
  v_pack->setGroup(GAMETEXT_PACK_WEBVOTES);
  v_pack->setDescription(VPACKAGENAME_DESC_HARDEST_LEVELS);
  m_levelsPacks.push_back(v_pack);

  /* vote not locked levels */
  if (i_bAdminMode) {
    v_pack = new LevelsPack(
      std::string(VPACKAGENAME_VOTE_NOT_LOCKED_LEVELS),
      "SELECT a.id_level AS id_level, a.name AS name, UPPER(a.name) AS "
      "sort_field "
      "FROM levels AS a INNER JOIN weblevels AS b "
      "ON a.id_level=b.id_level "
      "WHERE b.vote_locked=0 "
      "AND xm_userChildrenCompliant(b.children_compliant)=1"); // do not remove
    // blacklisted
    // levels, only
    // children not
    // compliant one
    v_pack->setGroup(GAMETEXT_PACK_WEBVOTES);
    v_pack->setDescription(VPACKAGENAME_DESC_VOTE_NOT_LOCKED_LEVELS);
    m_levelsPacks.push_back(v_pack);
  }

  /* short levels */
  v_pack = new LevelsPack(
    std::string(VPACKAGENAME_SHORT_LEVELS),
    "SELECT a.id_level AS id_level, a.name AS name, b.finishTime+0 AS "
    "sort_field "
    "FROM levels AS a INNER JOIN webhighscores AS b ON (a.id_level=b.id_level "
    "AND b.id_room=" +
      i_id_room +
      ") "
      "LEFT OUTER JOIN levels_blacklist AS c ON (a.id_level = "
      "c.id_level AND c.id_profile=\"" +
      xmDatabase::protectString(i_playerName) +
      "\") "
      "LEFT OUTER JOIN weblevels AS d ON a.id_level=d.id_level "
      "WHERE b.finishTime+0 < 2500 AND c.id_level IS NULL "
      "AND (d.crappy IS NULL OR xm_userCrappy(d.crappy)=0) "
      "AND (d.children_compliant IS NULL OR "
      "xm_userChildrenCompliant(d.children_compliant)=1)");
  v_pack->setGroup(GAMETEXT_PACK_BY_LENGTH);
  v_pack->setDescription(VPACKAGENAME_DESC_SHORT_LEVELS);
  m_levelsPacks.push_back(v_pack);

  /* Medium levels */
  v_pack = new LevelsPack(
    std::string(VPACKAGENAME_MEDIUM_LEVELS),
    "SELECT a.id_level AS id_level, a.name AS name, b.finishTime+0 AS "
    "sort_field "
    "FROM levels AS a INNER JOIN webhighscores AS b ON (a.id_level=b.id_level "
    "AND b.id_room=" +
      i_id_room +
      ") "
      "LEFT OUTER JOIN levels_blacklist AS c ON (a.id_level = "
      "c.id_level AND c.id_profile=\"" +
      xmDatabase::protectString(i_playerName) +
      "\") "
      "LEFT OUTER JOIN weblevels AS d ON a.id_level=d.id_level "
      "WHERE b.finishTime+0 >= 2500 AND b.finishTime+0 < 6000 AND c.id_level "
      "IS NULL "
      "AND (d.crappy IS NULL OR xm_userCrappy(d.crappy)=0) "
      "AND (d.children_compliant IS NULL OR "
      "xm_userChildrenCompliant(d.children_compliant)=1)");
  v_pack->setGroup(GAMETEXT_PACK_BY_LENGTH);
  v_pack->setDescription(VPACKAGENAME_DESC_MEDIUM_LEVELS);
  m_levelsPacks.push_back(v_pack);

  /* Long levels*/
  v_pack = new LevelsPack(
    std::string(VPACKAGENAME_LONG_LEVELS),
    "SELECT a.id_level AS id_level, a.name AS name, b.finishTime+0 AS "
    "sort_field "
    "FROM levels AS a INNER JOIN webhighscores AS b ON (a.id_level=b.id_level "
    "AND b.id_room=" +
      i_id_room +
      ") "
      "LEFT OUTER JOIN levels_blacklist AS c ON (a.id_level = "
      "c.id_level AND c.id_profile=\"" +
      xmDatabase::protectString(i_playerName) +
      "\") "
      "LEFT OUTER JOIN weblevels AS d ON a.id_level=d.id_level "
      "WHERE b.finishTime+0 >= 6000 AND b.finishTime+0 < 12000 AND c.id_level "
      "IS NULL "
      "AND (d.crappy IS NULL OR xm_userCrappy(d.crappy)=0) "
      "AND (d.children_compliant IS NULL OR "
      "xm_userChildrenCompliant(d.children_compliant)=1)");
  v_pack->setGroup(GAMETEXT_PACK_BY_LENGTH);
  v_pack->setDescription(VPACKAGENAME_DESC_LONG_LEVELS);
  m_levelsPacks.push_back(v_pack);

  /* Very long levels*/
  v_pack = new LevelsPack(
    std::string(VPACKAGENAME_VERY_LONG_LEVELS),
    "SELECT a.id_level AS id_level, a.name AS name, b.finishTime+0 AS "
    "sort_field "
    "FROM levels AS a INNER JOIN webhighscores AS b ON (a.id_level=b.id_level "
    "AND b.id_room=" +
      i_id_room +
      ") "
      "LEFT OUTER JOIN levels_blacklist AS c ON (a.id_level = "
      "c.id_level AND c.id_profile=\"" +
      xmDatabase::protectString(i_playerName) +
      "\") "
      "LEFT OUTER JOIN weblevels AS d ON a.id_level=d.id_level "
      "WHERE b.finishTime+0 >= 12000 AND c.id_level IS NULL "
      "AND (d.crappy IS NULL OR xm_userCrappy(d.crappy)=0) "
      "AND (d.children_compliant IS NULL OR "
      "xm_userChildrenCompliant(d.children_compliant)=1)");
  v_pack->setGroup(GAMETEXT_PACK_BY_LENGTH);
  v_pack->setDescription(VPACKAGENAME_DESC_VERY_LONG_LEVELS);
  m_levelsPacks.push_back(v_pack);

  /* by winner */
  v_result =
    i_db->readDB("SELECT a.id_profile "
                 "FROM webhighscores AS a "
                 "LEFT OUTER JOIN weblevels AS b ON a.id_level=b.id_level "
                 "WHERE (b.crappy IS NULL OR xm_userCrappy(b.crappy)=0) "
                 "AND (b.children_compliant IS NULL OR "
                 "xm_userChildrenCompliant(b.children_compliant)=1) "
                 "AND a.id_room=" +
                   i_id_room +
                   " "
                   "GROUP BY a.id_profile "
                   "ORDER BY COUNT(1) DESC LIMIT 10;",
                 nrow);
  for (unsigned int i = 0; i < nrow; i++) {
    char v_levelPackStr[256];

    std::ostringstream v_n;
    if (i + 1 < 10)
      v_n << "0";
    v_n << i + 1;
    v_n << ". ";

    std::string v_id_profile = i_db->getResult(v_result, 1, i, 0);
    v_pack = new LevelsPack(
      v_n.str() + v_id_profile,
      "SELECT a.id_level AS id_level, a.name AS name, UPPER(a.name) AS "
      "sort_field "
      "FROM levels AS a INNER JOIN "
      "webhighscores AS b ON (a.id_level = b.id_level "
      "AND b.id_room=" +
        i_id_room +
        ") "
        "LEFT OUTER JOIN weblevels AS c ON a.id_level=c.id_level "
        "LEFT OUTER JOIN levels_blacklist AS d ON (a.id_level = "
        "d.id_level AND d.id_profile=\"" +
        xmDatabase::protectString(i_playerName) +
        "\") "
        "WHERE b.id_profile=\"" +
        xmDatabase::protectString(v_id_profile) +
        "\" "
        "AND (c.crappy IS NULL OR xm_userCrappy(c.crappy)=0) "
        "AND (c.children_compliant IS NULL OR "
        "xm_userChildrenCompliant(c.children_compliant)=1) "
        "AND d.id_level IS NULL");
    v_pack->setGroup(GAMETEXT_PACK_BEST_DRIVERS);
    snprintf(v_levelPackStr,
             256,
             VPACKAGENAME_DESC_BEST_DRIVERS,
             v_id_profile.c_str());
    v_pack->setDescription(v_levelPackStr);
    m_levelsPacks.push_back(v_pack);
  }
  i_db->read_DB_free(v_result);

  unlockLevelsPacks();
}

const std::vector<LevelsPack *> &LevelsManager::LevelsPacks() {
  return m_levelsPacks;
}

void LevelsManager::reloadExternalLevels(
  xmDatabase *i_db,
  bool i_loadMainLayerOnly,
  XMotoLoadLevelsInterface *i_loadLevelsInterface) {
  std::vector<std::string> LvlFiles =
    XMFS::findPhysFiles(FDT_DATA, "Levels/MyLevels/*.lvl", true);
  std::string v_levelName;

  // main case : no external level
  if (LvlFiles.size() == 0) {
    return;
  }

  i_db->levels_add_begin(true);
  for (unsigned int i = 0; i < LvlFiles.size(); i++) {
    /* add the level from the unloaded levels if possible to make it faster */
    if (i_db->levels_add_fast(LvlFiles[i], v_levelName, true) == false) {
      Level *v_level = new Level();

      try {
        v_level->setFileName(LvlFiles[i]);
        v_level->loadReducedFromFile(i_loadMainLayerOnly);

        v_levelName = v_level->Name();

        // Check for ID conflict
        if (doesLevelExist(v_level->Id(), i_db)) {
          throw Exception("Duplicate level ID");
        }
        i_db->levels_add(v_level->Id(),
                         v_level->FileName(),
                         v_level->Name(),
                         v_level->Checksum(),
                         v_level->Author(),
                         v_level->Description(),
                         v_level->Date(),
                         v_level->Music(),
                         v_level->isScripted(),
                         v_level->isPhysics(),
                         true);
      } catch (Exception &e) {
      }
      delete v_level;
    }

    if (i_loadLevelsInterface != NULL) {
      i_loadLevelsInterface->loadLevelHook(v_levelName,
                                           ((i + 1) * 100) / LvlFiles.size());
    }
  }

  i_db->levels_add_end();
}

std::string LevelsManager::addExternalLevel(std::string i_levelFile,
                                            xmDatabase *i_db,
                                            bool i_loadMainLayerOnly) {
  std::string id = i_levelFile;
  Level *v_level = new Level();
  try {
    v_level->setFileName(i_levelFile);
    v_level->loadReducedFromFile(i_loadMainLayerOnly);
    id = v_level->Id();

    // Check for ID conflict
    if (doesLevelExist(v_level->Id(), i_db)) {
      throw Exception("Duplicate level ID");
    }
    i_db->levels_add(v_level->Id(),
                     v_level->FileName(),
                     v_level->Name(),
                     v_level->Checksum(),
                     v_level->Author(),
                     v_level->Description(),
                     v_level->Date(),
                     v_level->Music(),
                     v_level->isScripted(),
                     v_level->isPhysics(),
                     true);
  } catch (Exception &e) {
    LogWarning("Unable to add external level (%s)", e.getMsg().c_str());
  }
  delete v_level;
  return id;
}

void LevelsManager::reloadLevelsFromLvl(
  xmDatabase *i_db,
  bool i_loadMainLayerOnly,
  XMotoLoadLevelsInterface *i_loadLevelsInterface) {
  reloadInternalLevels(i_db, i_loadMainLayerOnly, i_loadLevelsInterface);
  reloadExternalLevels(i_db, i_loadMainLayerOnly, i_loadLevelsInterface);
}

void LevelsManager::reloadInternalLevels(
  xmDatabase *i_db,
  bool i_loadMainLayerOnly,
  XMotoLoadLevelsInterface *i_loadLevelsInterface) {
  std::vector<std::string> LvlFiles =
    XMFS::findPhysFiles(FDT_DATA, "Levels/*.lvl", true);
  std::string v_levelName;

  i_db->levels_add_begin(false);

  for (unsigned int i = 0; i < LvlFiles.size(); i++) {
    int v_isExternal;

    v_isExternal = LvlFiles[i].find("Levels/MyLevels/") != std::string::npos;
    if (v_isExternal) {
      continue; // don't load external levels now
    }

    /* add the level from the unloaded levels if possible to make it faster */
    if (i_db->levels_add_fast(LvlFiles[i], v_levelName, false) == false) {
      Level *v_level = new Level();

      try {
        v_level->setFileName(LvlFiles[i]);
        v_level->loadReducedFromFile(i_loadMainLayerOnly);

        // Check for ID conflict
        if (doesLevelExist(v_level->Id(), i_db)) {
          throw Exception("Duplicate level ID");
        }
        i_db->levels_add(v_level->Id(),
                         v_level->FileName(),
                         v_level->Name(),
                         v_level->Checksum(),
                         v_level->Author(),
                         v_level->Description(),
                         v_level->Date(),
                         v_level->Music(),
                         v_level->isScripted(),
                         v_level->isPhysics(),
                         false);
      } catch (Exception &e) {
        LogWarning("(just mean that the level has been updated if the level is "
                   "in xmoto.bin) ** : %s (%s - %s)",
                   e.getMsg().c_str(),
                   v_level->Name().c_str(),
                   v_level->FileName().c_str());
      }
      v_levelName = v_level->Name();
      delete v_level;
    }

    if (i_loadLevelsInterface != NULL) {
      i_loadLevelsInterface->loadLevelHook(v_levelName,
                                           ((i + 1) * 100) / LvlFiles.size());
    }
  }

  i_db->levels_add_end();
}

void LevelsManager::checkPrerequires() {
  std::string LCachePath = XMFS::getUserDir(FDT_CACHE) + std::string("/LCache");

  if (XMFS::isDir(LCachePath) == false) {
    XMFS::mkArborescenceDir(LCachePath);
  }

  if (XMFS::isDir(XMFS::getUserDir(FDT_DATA) + "/Levels/MyLevels") == false) {
    XMFS::mkArborescenceDir(XMFS::getUserLevelsDir() + "/MyLevels");
  }
}

void LevelsManager::cleanCache() {
  /* Find all .blv-files in the directory */
  std::vector<std::string> BlvFiles =
    XMFS::findPhysFiles(FDT_CACHE, "LCache/*.blv");
  for (unsigned int i = 0; i < BlvFiles.size(); i++) {
    XMFS::deleteFile(FDT_CACHE, BlvFiles[i]);
  }
}

std::string LevelsManager::LevelByFileName(const std::string &i_fileName,
                                           xmDatabase *i_db) {
  char **v_result;
  unsigned int nrow;
  std::string v_id_level;

  v_result = i_db->readDB("SELECT id_level FROM levels WHERE filepath=\"" +
                            xmDatabase::protectString(i_fileName) + "\";",
                          nrow);
  if (nrow == 0) {
    i_db->read_DB_free(v_result);
    throw Exception(
      std::string("Level " + i_fileName + " doesn't exist").c_str());
  }

  v_id_level = i_db->getResult(v_result, 1, 0, 0);

  i_db->read_DB_free(v_result);
  return v_id_level;
}

bool LevelsManager::doesLevelExist(const std::string &i_id, xmDatabase *i_db) {
  char **v_result;
  unsigned int nrow;

  v_result = i_db->readDB("SELECT id_level "
                          "FROM levels "
                          "WHERE id_level=\"" +
                            xmDatabase::protectString(i_id) + "\";",
                          nrow);
  i_db->read_DB_free(v_result);

  return nrow != 0;
}

void LevelsManager::printLevelsList(xmDatabase *i_db) const {
  char **v_result;
  unsigned int nrow;

  printf("%-40s %-40s\n", "Id", "Name");

  v_result = i_db->readDB("SELECT id_level, name FROM levels;", nrow);
  for (unsigned int i = 0; i < nrow; i++) {
    printf("%-40s %-40s\n",
           i_db->getResult(v_result, 2, i, 0),
           i_db->getResult(v_result, 2, i, 1));
  }
  i_db->read_DB_free(v_result);
}

void LevelsManager::updateLevelsFromLvl(
  const std::vector<std::string> &NewLvl,
  const std::vector<std::string> &UpdatedLvl,
  bool i_loadMainLayerOnly,
  WWWAppInterface *pCaller,
  xmDatabase *i_db) {
  Level *v_level;
  int current = 0;
  float total = 100.0 / (float)(NewLvl.size() + UpdatedLvl.size());

  try {
    i_db->levels_addToNew_begin();
    i_db->levels_cleanNew();

    /* new */
    for (unsigned int i = 0; i < NewLvl.size(); i++) {
      v_level = new Level();

      try {
        pCaller->setTaskProgress(current * total);
        current++;

        v_level->setFileName(NewLvl[i]);
        v_level->loadReducedFromFile(i_loadMainLayerOnly);

        // Check for ID conflict
        if (doesLevelExist(v_level->Id(), i_db)) {
          throw Exception("Duplicate level ID");
        }
        i_db->levels_add(v_level->Id(),
                         v_level->FileName(),
                         v_level->Name(),
                         v_level->Checksum(),
                         v_level->Author(),
                         v_level->Description(),
                         v_level->Date(),
                         v_level->Music(),
                         v_level->isScripted(),
                         v_level->isPhysics(),
                         false);
        i_db->levels_addToNew(v_level->Id(), false);
      } catch (Exception &e) {
        LogWarning("%s", e.getMsg().c_str());
      }
      delete v_level;
    }

    /* updated */
    for (unsigned int i = 0; i < UpdatedLvl.size(); i++) {
      v_level = new Level();

      try {
        v_level->setFileName(UpdatedLvl[i]);
        v_level->loadReducedFromFile(i_loadMainLayerOnly);

        pCaller->setTaskProgress(current * total);
        pCaller->setBeingDownloadedInformation(v_level->Name());
        current++;

        i_db->levels_update(v_level->Id(),
                            v_level->FileName(),
                            v_level->Name(),
                            v_level->Checksum(),
                            v_level->Author(),
                            v_level->Description(),
                            v_level->Date(),
                            v_level->Music(),
                            v_level->isScripted(),
                            v_level->isPhysics(),
                            false);
        i_db->levels_addToNew(v_level->Id(), true);

      } catch (Exception &e) {
        LogWarning("%s", e.getMsg().c_str());
      }
      delete v_level;
    }
    i_db->levels_addToNew_end();
  } catch (Exception &e) {
    i_db->levels_addToNew_end(); // commit what has been done even if it failed
    throw e;
  }
}

bool LevelsManager::isInFavorite(std::string i_profile,
                                 const std::string &i_id_level,
                                 xmDatabase *i_db) {
  unsigned int nrow;
  char **v_result;
  int v_n;

  /* check if the level is already into the favorite list */
  v_result = i_db->readDB("SELECT COUNT(id_level) "
                          "FROM levels_favorite "
                          "WHERE id_level=\"" +
                            i_db->protectString(i_id_level) +
                            "\" "
                            "AND   id_profile=\"" +
                            i_db->protectString(i_profile) + "\";",
                          nrow);
  v_n = atoi(i_db->getResult(v_result, 1, 0, 0));
  i_db->read_DB_free(v_result);

  return v_n != 0;
}

void LevelsManager::addToFavorite(std::string i_profile,
                                  const std::string &i_id_level,
                                  xmDatabase *i_db) {
  if (isInFavorite(i_profile, i_id_level, i_db) == false) {
    i_db->levels_addToFavorite(i_profile, i_id_level);
  }
}

void LevelsManager::delFromFavorite(std::string i_profile,
                                    const std::string &i_id_level,
                                    xmDatabase *i_db) {
  if (isInFavorite(i_profile, i_id_level, i_db)) {
    i_db->levels_delToFavorite(i_profile, i_id_level);
  }
}

bool LevelsManager::isInBlacklist(std::string i_profile,
                                  const std::string &i_id_level,
                                  xmDatabase *i_db) {
  unsigned int nrow;
  char **v_result;
  int v_n;

  /* check if the level is already into the blacklist list */
  v_result = i_db->readDB("SELECT COUNT(id_level) "
                          "FROM levels_blacklist "
                          "WHERE id_level=\"" +
                            i_db->protectString(i_id_level) +
                            "\" "
                            "AND   id_profile=\"" +
                            i_db->protectString(i_profile) + "\";",
                          nrow);
  v_n = atoi(i_db->getResult(v_result, 1, 0, 0));
  i_db->read_DB_free(v_result);

  return v_n != 0;
}

void LevelsManager::addToBlacklist(std::string i_profile,
                                   const std::string &i_id_level,
                                   xmDatabase *i_db) {
  if (isInBlacklist(i_profile, i_id_level, i_db) == false) {
    i_db->levels_addToBlacklist(i_profile, i_id_level);
  }
}

void LevelsManager::delFromBlacklist(std::string i_profile,
                                     const std::string &i_id_level,
                                     xmDatabase *i_db) {
  if (isInBlacklist(i_profile, i_id_level, i_db)) {
    i_db->levels_delToBlacklist(i_profile, i_id_level);
  }
}

std::string LevelsManager::getQuickStartPackQuery(unsigned int i_qualityMIN,
                                                  unsigned int i_difficultyMIN,
                                                  unsigned int i_qualityMAX,
                                                  unsigned int i_difficultyMAX,
                                                  const std::string &i_profile,
                                                  const std::string &i_id_room,
                                                  xmDatabase *i_db) {
  /* SELECT id_level, name, profile_best_finishTime, web_highscore */
  char **v_result;
  unsigned int nrow;
  char *v_res;

  /* if xmoto run only 1 time, run the tutorial pack */
  bool v_tutorials = false;
  v_result = i_db->readDB("SELECT SUM(nbStarts+0) "
                          "FROM stats_profiles "
                          "WHERE id_profile=\"" +
                            xmDatabase::protectString(i_profile) +
                            "\" "
                            "GROUP BY id_profile;",
                          nrow);
  if (nrow == 0) {
    v_tutorials = true;
  } else {
    v_res = i_db->getResult(v_result, 1, 0, 0);
    if (atoi(v_res) < 2) {
      v_tutorials = true;
    }
  }
  i_db->read_DB_free(v_result);

  if (v_tutorials) {
    return "SELECT a.id_level, MIN(a.name), MIN(b.finishTime+0), "
           "MIN(c.finishTime+0) "
           "FROM levels AS a "
           "INNER JOIN webhighscores AS b ON (a.id_level = b.id_level AND "
           "b.id_room=" +
           i_id_room +
           ") "
           "LEFT OUTER JOIN profile_completedLevels AS c "
           "ON (a.id_level=c.id_level AND c.id_profile=\"" +
           xmDatabase::protectString(i_profile) +
           "\") "
           "WHERE b.packName=\"Tutorials\" "
           "GROUP BY a.id_level "
           "ORDER BY b.packNum || UPPER(a.name);";
  }

  /* run the query to check wether there are at least 5 levels, else, all levels
   * randomly */
  /* web quality and difficulty are >=1 and <=5 */
  bool v_haveEnoughLevels;
  std::ostringstream v_qualityMINStr;
  std::ostringstream v_difficultyMINStr;
  std::ostringstream v_qualityMAXStr;
  std::ostringstream v_difficultyMAXStr;

  // make ranges because statistics dont give round numbers anymore
  if (i_qualityMIN == 5) {
    i_qualityMIN = 4.5;
  }
  if (i_qualityMAX == 1) {
    i_qualityMAX = 1.5;
  }
  if (i_qualityMIN == i_qualityMAX) {
    i_qualityMIN -= 0.5;
    i_qualityMAX += 0.5;
  }

  if (i_difficultyMIN == 5) {
    i_difficultyMIN = 4.5;
  }
  if (i_difficultyMAX == 1) {
    i_difficultyMAX = 1.5;
  }
  if (i_difficultyMIN == i_difficultyMAX) {
    i_difficultyMIN -= 0.5;
    i_difficultyMAX += 0.5;
  }

  v_qualityMINStr << i_qualityMIN;
  v_difficultyMINStr << i_difficultyMIN;
  v_qualityMAXStr << i_qualityMAX;
  v_difficultyMAXStr << i_difficultyMAX;

  v_result = i_db->readDB(
    "SELECT count(1) "
    "FROM levels AS a "
    "INNER JOIN weblevels AS b ON a.id_level = b.id_level "
    "LEFT OUTER JOIN levels_blacklist AS c ON (a.id_level = c.id_level AND "
    "c.id_profile = \"" +
      xmDatabase::protectString(i_profile) +
      "\") "
      "WHERE b.quality+0    <= " +
      v_qualityMAXStr.str() +
      " "
      "AND   b.quality+0    >= " +
      v_qualityMINStr.str() +
      " "
      "AND   b.difficulty+0 <= " +
      v_difficultyMAXStr.str() +
      " "
      "AND   b.difficulty+0 >= " +
      v_difficultyMINStr.str() +
      " "
      "AND   xm_userCrappy(b.crappy)=0 "
      "AND   xm_userChildrenCompliant(b.children_compliant)=1 "
      "AND   c.id_level IS NULL "
      "ORDER BY RANDOM();",
    nrow);
  v_res = i_db->getResult(v_result, 1, 0, 0);
  v_haveEnoughLevels = atoi(v_res) >= 5; /* less than 5 levels ? */
  i_db->read_DB_free(v_result);

  if (v_haveEnoughLevels) {
    return "SELECT a.id_level, MIN(a.name), MIN(c.finishTime+0), "
           "MIN(d.finishTime+0) "
           "FROM levels AS a "
           "INNER JOIN weblevels AS b ON a.id_level = b.id_level "
           "LEFT OUTER JOIN webhighscores AS c ON (a.id_level = c.id_level AND "
           "c.id_room=" +
           i_id_room +
           ") "
           "LEFT OUTER JOIN profile_completedLevels AS d "
           "ON (a.id_level=d.id_level AND d.id_profile=\"" +
           xmDatabase::protectString(i_profile) +
           "\") "
           "LEFT OUTER JOIN levels_blacklist AS e ON (a.id_level = e.id_level "
           "AND e.id_profile=\"" +
           xmDatabase::protectString(i_profile) +
           "\") "
           "WHERE b.quality+0    <= " +
           v_qualityMAXStr.str() +
           " "
           "AND   b.quality+0    >= " +
           v_qualityMINStr.str() +
           " "
           "AND   b.difficulty+0 <= " +
           v_difficultyMAXStr.str() +
           " "
           "AND   b.difficulty+0 >= " +
           v_difficultyMINStr.str() +
           " "
           "AND   xm_userCrappy(b.crappy)=0 "
           "AND   xm_userChildrenCompliant(b.children_compliant)=1 "
           "AND   e.id_level IS NULL "
           "GROUP BY a.id_level ORDER BY RANDOM();";
  } else {
    /* all levels randomly */
    SysMessage::instance()->displayInformation(GAMETEXT_QUERY_WITHOUT_RESULT);
    return "SELECT a.id_level, MIN(a.name), MIN(b.finishTime+0), "
           "MIN(c.finishTime+0) "
           "FROM levels AS a "
           "LEFT OUTER JOIN webhighscores AS b ON (a.id_level = b.id_level AND "
           "b.id_room=" +
           i_id_room +
           ") "
           "LEFT OUTER JOIN profile_completedLevels AS c "
           "ON (a.id_level=c.id_level AND c.id_profile=\"" +
           xmDatabase::protectString(i_profile) +
           "\") "
           "LEFT OUTER JOIN weblevels AS d ON (a.id_level=d.id_level) "
           "LEFT OUTER JOIN levels_blacklist AS e ON (a.id_level = e.id_level "
           "AND e.id_profile=\"" +
           xmDatabase::protectString(i_profile) +
           "\") "
           "WHERE xm_userCrappy(d.crappy)=0 "
           "AND xm_userChildrenCompliant(d.children_compliant)=1 "
           "AND   e.id_level IS NULL "
           "GROUP BY a.id_level ORDER BY RANDOM();";
  }
}

void LevelsManager::lockLevelsPacks() {
  SDL_LockMutex(m_levelsPackMutex);
}

void LevelsManager::unlockLevelsPacks() {
  SDL_UnlockMutex(m_levelsPackMutex);
}

void LevelsManager::writeDefaultPackagesSql(FileHandle *pfh,
                                            const std::string &i_sqlName,
                                            const std::string &i_sql) {
  XMFS::writeLineF(
    pfh, (char *)"#define %s \"%s\"", i_sqlName.c_str(), i_sql.c_str());
}

void LevelsManager::writeDefaultPackages(const std::string &i_file) {
  FileHandle *pfh;

  pfh = XMFS::openOFile(FDT_CACHE, i_file);
  if (pfh == NULL) {
    throw Exception("Unable to open file " + i_file);
  }

  XMFS::writeLineF(pfh,
                   (char *)"// this file is generated automatically by "
                           "\"xmoto --buildQueries\", don't edit it");

  writeDefaultPackagesSql(
    pfh,
    "QUERY_LVL_ALL",
    LevelsManager::queryLevelsAsVirtualPack(lprv_dontcare, // scripted
                                            lprv_dontcare, // physics
                                            lprv_dontcare, // crappy
                                            lprv_dontcare, // children compliant
                                            lprv_no, // blacklisted
                                            lprv_dontcare, // to reload
                                            lprv_dontcare, // favorite
                                            lprv_dontcare, // unfinished
                                            lprv_dontcare, // new
                                            lprv_dontcare, // stolen
                                            lprv_dontcare, // highscore driver
                                            "", // highscore driver name
                                            0, // last levels
                                            0 // last highscores
                                            ));

  writeDefaultPackagesSql(
    pfh,
    "QUERY_LVL_MY",
    LevelsManager::queryLevelsAsVirtualPack(lprv_dontcare, // scripted
                                            lprv_dontcare, // physics
                                            lprv_dontcare, // crappy
                                            lprv_dontcare, // children compliant
                                            lprv_no, // blacklisted
                                            lprv_yes, // to reload
                                            lprv_dontcare, // favorite
                                            lprv_dontcare, // unfinished
                                            lprv_dontcare, // new
                                            lprv_dontcare, // stolen
                                            lprv_dontcare, // highscore driver
                                            "", // highscore driver name
                                            0, // last levels
                                            0 // last highscores
                                            ));

  writeDefaultPackagesSql(
    pfh,
    "QUERY_LVL_FAVORITES",
    LevelsManager::queryLevelsAsVirtualPack(lprv_dontcare, // scripted
                                            lprv_dontcare, // physics
                                            lprv_dontcare, // crappy
                                            lprv_dontcare, // children compliant
                                            lprv_no, // blacklisted
                                            lprv_dontcare, // to reload
                                            lprv_yes, // favorite
                                            lprv_dontcare, // unfinished
                                            lprv_dontcare, // new
                                            lprv_dontcare, // stolen
                                            lprv_dontcare, // highscore driver
                                            "", // highscore driver name
                                            0, // last levels
                                            0 // last highscores
                                            ));
  writeDefaultPackagesSql(
    pfh,
    "QUERY_LVL_BLACKLIST",
    LevelsManager::queryLevelsAsVirtualPack(lprv_dontcare, // scripted
                                            lprv_dontcare, // physics
                                            lprv_dontcare, // crappy
                                            lprv_dontcare, // children compliant
                                            lprv_yes, // blacklisted
                                            lprv_dontcare, // to reload
                                            lprv_dontcare, // favorite
                                            lprv_dontcare, // unfinished
                                            lprv_dontcare, // new
                                            lprv_dontcare, // stolen
                                            lprv_dontcare, // highscore driver
                                            "", // highscore driver name
                                            0, // last levels
                                            0 // last highscores
                                            ));

  writeDefaultPackagesSql(
    pfh,
    "QUERY_LVL_SCRIPTED",
    LevelsManager::queryLevelsAsVirtualPack(lprv_yes, // scripted
                                            lprv_dontcare, // physics
                                            lprv_dontcare, // crappy
                                            lprv_dontcare, // children compliant
                                            lprv_no, // blacklisted
                                            lprv_dontcare, // to reload
                                            lprv_dontcare, // favorite
                                            lprv_dontcare, // unfinished
                                            lprv_dontcare, // new
                                            lprv_dontcare, // stolen
                                            lprv_dontcare, // highscore driver
                                            "", // highscore driver name
                                            0, // last levels
                                            0 // last highscores
                                            ));

  writeDefaultPackagesSql(
    pfh,
    "QUERY_LVL_PHYSICS",
    LevelsManager::queryLevelsAsVirtualPack(lprv_dontcare, // scripted
                                            lprv_yes, // physics
                                            lprv_dontcare, // crappy
                                            lprv_dontcare, // children compliant
                                            lprv_no, // blacklisted
                                            lprv_dontcare, // to reload
                                            lprv_dontcare, // favorite
                                            lprv_dontcare, // unfinished
                                            lprv_dontcare, // new
                                            lprv_dontcare, // stolen
                                            lprv_dontcare, // highscore driver
                                            "", // highscore driver name
                                            0, // last levels
                                            0 // last highscores
                                            ));

  writeDefaultPackagesSql(
    pfh,
    "QUERY_LVL_INCOMPLETED",
    LevelsManager::queryLevelsAsVirtualPack(lprv_dontcare, // scripted
                                            lprv_dontcare, // physics
                                            lprv_dontcare, // crappy
                                            lprv_dontcare, // children compliant
                                            lprv_no, // blacklisted
                                            lprv_dontcare, // to reload
                                            lprv_dontcare, // favorite
                                            lprv_no, // finished
                                            lprv_dontcare, // new
                                            lprv_dontcare, // stolen
                                            lprv_dontcare, // highscore driver
                                            "", // highscore driver name
                                            0, // last levels
                                            0 // last highscores
                                            ));

  writeDefaultPackagesSql(
    pfh,
    "QUERY_LVL_NEW",
    LevelsManager::queryLevelsAsVirtualPack(lprv_dontcare, // scripted
                                            lprv_dontcare, // physics
                                            lprv_dontcare, // crappy
                                            lprv_dontcare, // children compliant
                                            lprv_no, // blacklisted
                                            lprv_dontcare, // to reload
                                            lprv_dontcare, // favorite
                                            lprv_dontcare, // finished
                                            lprv_yes, // new
                                            lprv_dontcare, // stolen
                                            lprv_dontcare, // highscore driver
                                            "", // highscore driver name
                                            0, // last levels
                                            0 // last highscores
                                            ));

  writeDefaultPackagesSql(
    pfh,
    "QUERY_LVL_CRAPPY",
    LevelsManager::queryLevelsAsVirtualPack(lprv_dontcare, // scripted
                                            lprv_dontcare, // physics
                                            lprv_yes, // crappy
                                            lprv_dontcare, // children compliant
                                            lprv_no, // blacklisted
                                            lprv_dontcare, // to reload
                                            lprv_dontcare, // favorite
                                            lprv_dontcare, // finished
                                            lprv_dontcare, // new
                                            lprv_dontcare, // stolen
                                            lprv_dontcare, // highscore driver
                                            "", // highscore driver name
                                            0, // last levels
                                            0 // last highscores
                                            ));

  writeDefaultPackagesSql(
    pfh,
    "QUERY_LVL_LAST",
    LevelsManager::queryLevelsAsVirtualPack(lprv_dontcare, // scripted
                                            lprv_dontcare, // physics
                                            lprv_dontcare, // crappy
                                            lprv_dontcare, // children compliant
                                            lprv_no, // blacklisted
                                            lprv_dontcare, // to reload
                                            lprv_dontcare, // favorite
                                            lprv_dontcare, // finished
                                            lprv_dontcare, // new
                                            lprv_dontcare, // stolen
                                            lprv_dontcare, // highscore driver
                                            "", // highscore driver name
                                            100, // last levels
                                            0 // last highscores
                                            ));

  writeDefaultPackagesSql(
    pfh,
    "QUERY_LVL_STOLEN",
    LevelsManager::queryLevelsAsVirtualPack(lprv_dontcare, // scripted
                                            lprv_dontcare, // physics
                                            lprv_dontcare, // crappy
                                            lprv_dontcare, // children compliant
                                            lprv_no, // blacklisted
                                            lprv_dontcare, // to reload
                                            lprv_dontcare, // favorite
                                            lprv_dontcare, // finished
                                            lprv_dontcare, // new
                                            lprv_yes, // stolen
                                            lprv_dontcare, // highscore driver
                                            "", // highscore driver name
                                            0, // last levels
                                            0 // last highscores
                                            ));

  writeDefaultPackagesSql(
    pfh,
    "QUERY_LVL_NO_HIGHSCORE",
    LevelsManager::queryLevelsAsVirtualPack(lprv_dontcare, // scripted
                                            lprv_dontcare, // physics
                                            lprv_dontcare, // crappy
                                            lprv_dontcare, // children compliant
                                            lprv_no, // blacklisted
                                            lprv_dontcare, // to reload
                                            lprv_dontcare, // favorite
                                            lprv_dontcare, // finished
                                            lprv_dontcare, // new
                                            lprv_dontcare, // stolen
                                            lprv_yes, // highscore driver
                                            "", // highscore driver name
                                            0, // last levels
                                            0 // last highscores
                                            ));

  writeDefaultPackagesSql(
    pfh,
    "QUERY_LVL_NOT_THE_HIGHSCORE",
    LevelsManager::queryLevelsAsVirtualPack(lprv_dontcare, // scripted
                                            lprv_dontcare, // physics
                                            lprv_dontcare, // crappy
                                            lprv_dontcare, // children compliant
                                            lprv_no, // blacklisted
                                            lprv_dontcare, // to reload
                                            lprv_dontcare, // favorite
                                            lprv_dontcare, // finished
                                            lprv_dontcare, // new
                                            lprv_dontcare, // stolen
                                            lprv_no, // highscore driver
                                            "<SELF>", // highscore driver name
                                            0, // last levels
                                            0 // last highscores
                                            ));

  writeDefaultPackagesSql(
    pfh,
    "QUERY_LVL_LAST_HIGHSCORES",
    LevelsManager::queryLevelsAsVirtualPack(lprv_dontcare, // scripted
                                            lprv_dontcare, // physics
                                            lprv_dontcare, // crappy
                                            lprv_dontcare, // children compliant
                                            lprv_no, // blacklisted
                                            lprv_dontcare, // to reload
                                            lprv_dontcare, // favorite
                                            lprv_dontcare, // finished
                                            lprv_dontcare, // new
                                            lprv_dontcare, // stolen
                                            lprv_dontcare, // highscore driver
                                            "", // highscore driver name
                                            0, // last levels
                                            100 // last highscores
                                            ));

  writeDefaultPackagesSql(
    pfh,
    "QUERY_LVL_OLDEST_HIGHSCORES",
    LevelsManager::queryLevelsAsVirtualPack(lprv_dontcare, // scripted
                                            lprv_dontcare, // physics
                                            lprv_dontcare, // crappy
                                            lprv_dontcare, // children compliant
                                            lprv_no, // blacklisted
                                            lprv_dontcare, // to reload
                                            lprv_dontcare, // favorite
                                            lprv_dontcare, // finished
                                            lprv_dontcare, // new
                                            lprv_dontcare, // stolen
                                            lprv_dontcare, // highscore driver
                                            "", // highscore driver name
                                            0, // last levels
                                            -100 // last highscores
                                            ));

  XMFS::closeFile(pfh);
}

std::string LevelsManager::queryLevelsAsVirtualPack(
  levelPropertyRequiredValue i_isScripted,
  levelPropertyRequiredValue i_isPhysics,
  levelPropertyRequiredValue i_isCrappy,
  levelPropertyRequiredValue i_isChildrenCompliant,
  levelPropertyRequiredValue i_isBlacklisted,
  levelPropertyRequiredValue i_isToReload,
  levelPropertyRequiredValue i_isFavorite,
  levelPropertyRequiredValue i_isFinished,
  levelPropertyRequiredValue i_isNew,
  levelPropertyRequiredValue i_isStolen,
  levelPropertyRequiredValue i_setHighscoreDriver,
  const std::string &i_highscoreDriver,
  int i_limitToLastLevels,
  int i_limitToLastHighscores) {
  std::string v_sql, v_select, v_tables, v_where, v_group, v_order;
  std::string v_levelNameField, v_sortField;
  bool v_mustBeInWebLevels;
  bool v_mustBeInWebHighscores;
  std::ostringstream v_n;

  // args checks
  if (i_limitToLastLevels != 0 && i_limitToLastHighscores != 0) {
    throw Exception("virtual pack: cannot set lastLevels and lastHighscores");
  }
  //

  v_mustBeInWebLevels = false;
  v_mustBeInWebHighscores = false;

  // optimisation for weblevels
  if (i_limitToLastLevels != 0 || i_isCrappy == lprv_yes ||
      i_isChildrenCompliant == lprv_no || i_isStolen != lprv_dontcare) {
    v_mustBeInWebLevels = true; // because the information is in weblevels
  }

  // optimisations for webhighscores
  if (!(i_setHighscoreDriver == lprv_yes && i_highscoreDriver == "") ||
      i_limitToLastHighscores != 0) {
    v_mustBeInWebHighscores = true;
  }

  if (i_limitToLastHighscores != 0) {
    v_sortField = "i.date";
  } else if (i_limitToLastLevels != 0) {
    v_sortField = "b.creationDate";
  } else if (i_isStolen != lprv_dontcare) {
    v_sortField = "g.known_stolen_date";
  } else {
    v_sortField = "UPPER(a.name)";
  }

  if (i_isNew != lprv_dontcare) {
    v_levelNameField =
      "xm_lvlUpdatedToTxt(f.isAnUpdate) || \\\": \\\" || a.name";
  } else {
    v_levelNameField = "a.name";
  }

  if (i_isFinished == lprv_dontcare) {
    v_select = "SELECT a.id_level AS id_level, " + v_levelNameField +
               " AS name, " + v_sortField + " AS sort_field";
  } else {
    v_select = "SELECT a.id_level AS id_level, MIN(" + v_levelNameField +
               ") AS name, MIN(" + v_sortField + ") AS sort_field";
  }

  // FROM
  v_tables = " FROM levels AS a";

  // stolen ? (1/2) -- join here to filter a maximum
  if (i_isStolen != lprv_dontcare) {
    v_tables +=
      " INNER JOIN levels_mywebhighscores AS g ON a.id_level = g.id_level";
  }

  // weblevels ?
  // always required for children compliant
  if (v_mustBeInWebLevels) {
    v_tables += " INNER JOIN weblevels AS b ON a.id_level=b.id_level";
  } else {
    v_tables += " LEFT OUTER JOIN weblevels AS b ON a.id_level=b.id_level";
  }

  // blacklist ?
  if (i_isBlacklisted != lprv_dontcare) {
    if (i_isBlacklisted == lprv_yes) {
      v_tables += " INNER JOIN levels_blacklist AS c ON (a.id_level = "
                  "c.id_level AND c.id_profile=xm_profile())";
    } else {
      v_tables += " LEFT OUTER JOIN levels_blacklist AS c ON (a.id_level = "
                  "c.id_level AND c.id_profile=xm_profile())";
    }
  }

  // favorite ?
  if (i_isFavorite != lprv_dontcare) {
    if (i_isFavorite == lprv_yes) {
      v_tables += " INNER JOIN levels_favorite AS d ON (a.id_level = "
                  "d.id_level AND d.id_profile=xm_profile())";
    } else {
      v_tables += " LEFT OUTER JOIN levels_favorite AS d ON (a.id_level = "
                  "d.id_level AND d.id_profile=xm_profile())";
    }
  }

  // finnished ?
  if (i_isFinished != lprv_dontcare) {
    if (i_isFinished == lprv_yes) {
      v_tables += " INNER JOIN stats_profiles_levels AS e ON (a.id_level = "
                  "e.id_level AND e.id_profile=xm_profile())";
    } else {
      v_tables += " LEFT OUTER JOIN stats_profiles_levels AS e ON (a.id_level "
                  "= e.id_level AND e.id_profile=xm_profile())";
    }
  }

  // new ?
  if (i_isNew != lprv_dontcare) {
    if (i_isNew == lprv_yes) {
      v_tables += " INNER JOIN levels_new AS f ON a.id_level = f.id_level";
    } else {
      v_tables += " LEFT OUTER JOIN levels_new AS f ON a.id_level = f.id_level";
    }
  }

  // stolen ? (2/2) -- h = webhighscores of the stolen levels
  if (i_isStolen != lprv_dontcare) {
    v_tables += " LEFT OUTER JOIN webhighscores AS h";
    v_tables += "     ON (    g.id_room    = h.id_room";
    v_tables += "         AND g.id_level   = h.id_level";
    v_tables += "         AND g.id_profile = h.id_profile)";
  }

  // highscore information ? -- i = webhighscores of levels
  // highscore driver 1/2
  if (i_setHighscoreDriver != lprv_dontcare || i_limitToLastHighscores != 0) {
    if (v_mustBeInWebHighscores) {
      v_tables += " INNER JOIN webhighscores AS i";
    } else {
      v_tables += " LEFT OUTER JOIN webhighscores AS i";
    }

    v_tables += "     ON (    i.id_level   = a.id_level";
    v_tables += "         AND i.id_room    = xm_idRoom(0))"; // 1st room
  }

  //
  // last table is h
  //

  // WHERE

  // keep crappy "where" clause in first while one is always required

  // crappy ?
  switch (i_isCrappy) {
    case lprv_yes:
      v_where += " WHERE b.crappy=1";
      break;
    case lprv_no:
      if (v_mustBeInWebLevels) { // optim
        v_where += " WHERE b.crappy=0";
      } else {
        v_where += " WHERE (b.crappy IS NULL OR b.crappy=0)";
      }
      break;
    case lprv_dontcare:
      // if you don't care, don't force then, use options
      if (v_mustBeInWebLevels) { // optim
        v_where += " WHERE xm_userCrappy(b.crappy)=0";
      } else {
        v_where += " WHERE (b.crappy IS NULL OR xm_userCrappy(b.crappy)=0)";
      }
      break;
  }

  // scripted ?
  if (i_isScripted != lprv_dontcare) {
    v_where +=
      " AND a.isScripted=" + std::string(i_isScripted == lprv_yes ? "1" : "0");
  }

  // physics ?
  if (i_isPhysics != lprv_dontcare) {
    v_where +=
      " AND a.isPhysics=" + std::string(i_isPhysics == lprv_yes ? "1" : "0");
  }

  // children compliant ?
  switch (i_isChildrenCompliant) {
    case lprv_yes:
      if (v_mustBeInWebLevels) { // optim
        v_where += " AND xm_userChildrenCompliant(b.children_compliant)=1 AND "
                   "b.children_compliant=1";
      } else {
        v_where += " AND (b.children_compliant IS NULL OR "
                   "(xm_userChildrenCompliant(b.children_compliant)=1 AND "
                   "b.children_compliant=1)";
      }
      break;
    case lprv_no:
      v_where += " AND b.children_compliant=0 AND "
                 "xm_userChildrenCompliant(b.children_compliant)=1";
      break;
    case lprv_dontcare:
      if (v_mustBeInWebLevels) { // optim
        v_where += " AND xm_userChildrenCompliant(b.children_compliant)=1";
      } else {
        v_where += " AND (b.children_compliant IS NULL OR "
                   "xm_userChildrenCompliant(b.children_compliant)=1)";
      }
      break;
  }

  // blacklisted ?
  if (i_isBlacklisted == lprv_no) {
    v_where += " AND c.id_level IS NULL";
  }

  // favorite ?
  if (i_isFavorite == lprv_no) {
    v_where += " AND d.id_level IS NULL";
  }

  // to reload ?
  if (i_isToReload != lprv_dontcare) {
    v_where += " AND a.isToReload = " +
               std::string(i_isToReload == lprv_yes ? "1" : "0");
  }

  if (i_isNew == lprv_no) {
    v_where += " AND f.id_level IS NULL";
  }

  if (i_isStolen != lprv_dontcare) {
    v_where += " AND g.id_profile=xm_profile()";
    v_where += " AND g.id_room=xm_idRoom(0)"; // main room
    v_where += " AND h.id_profile IS NULL"; // you don't have the highscore
  }

  // highscore driver 2/2
  if (i_setHighscoreDriver != lprv_dontcare) {
    if (i_setHighscoreDriver == lprv_yes) {
      if (i_highscoreDriver == "") {
        v_where += " AND i.id_profile IS NULL";
      } else if (i_highscoreDriver == "<SELF>") { /* special <SELF> value */
        v_where += " AND i.id_profile = xm_profile()";
      } else {
        v_where += " AND i.id_profile = \"" +
                   xmDatabase::protectString(i_highscoreDriver) + "\"";
      }
    } else /* no */ {
      if (i_highscoreDriver == "") {
        // v_where += " AND i.id_profile IS NOT NULL"; // not required while
        // there is an inner join
      } else if (i_highscoreDriver == "<SELF>") { /* special <SELF> value */
        v_where += " AND i.id_profile <> xm_profile()";
      } else {
        v_where += " AND i.id_profile <> \"" +
                   xmDatabase::protectString(i_highscoreDriver) + "\"";
      }
    }
  }

  // GROUP

  // finished ?
  if (i_isFinished != lprv_dontcare) {
    v_group = " GROUP BY a.id_level, e.id_profile";

    if (i_isFinished == lprv_no) {
      v_group += " HAVING MAX(e.nbCompleted+0) = 0 OR MAX(e.nbCompleted+0) IS "
                 "NULL"; // MAX because of site keys
    } else {
      v_group += " HAVING MAX(e.nbCompleted+0) > 0";
    }
  }

  if (i_limitToLastHighscores != 0) {
    v_n << abs(i_limitToLastHighscores);
    v_order = " ORDER BY i.date " +
              std::string((i_limitToLastHighscores > 0 ? "DESC" : "ASC")) +
              " LIMIT " + v_n.str();
  } else if (i_limitToLastLevels != 0) {
    v_n << abs(i_limitToLastLevels);
    v_order = " ORDER BY b.creationDate " +
              std::string((i_limitToLastLevels > 0 ? "DESC" : "ASC")) +
              " LIMIT " + v_n.str();
  }

  // finally...
  v_sql = v_select + v_tables + v_where + v_group + v_order;

  return v_sql;
}

std::string LevelsManager::aRandomLevel(xmDatabase *i_db) const {
  char **v_result;
  unsigned int nrow;
  std::string v_res;

  v_result = i_db->readDB(
    "SELECT id_level FROM levels ORDER BY RANDOM() LIMIT 1;", nrow);

  if (nrow != 1) {
    i_db->read_DB_free(v_result);
    return "";
  }

  if (i_db->getResult(v_result, 1, 0, 0) == NULL) {
    i_db->read_DB_free(v_result);
    return "";
  }

  v_res = i_db->getResult(v_result, 1, 0, 0);
  i_db->read_DB_free(v_result);

  return v_res;
}
