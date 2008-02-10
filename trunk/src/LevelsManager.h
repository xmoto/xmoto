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

#ifndef __LEVELMANAGER_H__
#define __LEVELMANAGER_H__

#include <string>
#include "xmscene/Level.h"
#include "db/xmDatabase.h"
#include "XMotoLoadLevelsInterface.h"
#include "helpers/Singleton.h"

class WWWAppInterface;

class LevelsPack {
  public:
  LevelsPack(std::string i_name, const std::string& i_sql, bool i_ascSort = true);
  ~LevelsPack();

  std::string Name() const;
  void setName(const std::string& i_name);
  bool ShowTimes() const;
  bool ShowWebTimes() const;
  void setShowTimes(bool i_showTimes);
  void setShowWebTimes(bool i_showWebTimes);
  void setDescription(const std::string& i_description);
  std::string Description() const;

  std::string Group() const;
  void setGroup(std::string i_group);
  std::string getLevelsQuery() const;
  std::string getLevelsWithHighscoresQuery(const std::string& i_profile,
					   const std::string& i_id_room) const;
  int getNumberOfLevels(xmDatabase *i_db);
  int getNumberOfFinishedLevels(xmDatabase *i_db, const std::string& i_profile);

  private:
  void setHintsFromFile();

  std::string m_name;
  std::string m_group;
  std::string m_sql_levels;
  bool m_ascSort;
  bool m_showTimes;
  bool m_showWebTimes;
  std::string m_description;
};

class LevelsManager : public Singleton<LevelsManager> {
  friend class Singleton<LevelsManager>;

private:
  LevelsManager();
  ~LevelsManager();

public:
  LevelsPack& LevelsPackByName(const std::string &i_name);
  std::string LevelByFileName(const std::string& i_fileName, xmDatabase *i_db);
  bool doesLevelExist(const std::string& i_id, xmDatabase* i_db);

  bool doesLevelsPackExist(const std::string &i_name) const;

  /* to load or reload levels from files */
  void makePacks(const std::string& i_playerName,
		 const std::string& i_id_room,
		 bool i_bDebugMode, xmDatabase *i_db);
  void addExternalLevel(std::string i_levelFile, xmDatabase *i_db);
  void reloadLevelsFromLvl(xmDatabase* i_db, XMotoLoadLevelsInterface *i_loadLevelsInterface = NULL);

  /* to load news levels */
  /* to reload levels already loaded (it will put them into the updateLevels list) */
  /* create the newLevels.xml file */
  void updateLevelsFromLvl(const std::vector<std::string> &NewLvl,
			   const std::vector<std::string> &UpdatedLvl,
			   WWWAppInterface* pCaller,
			   xmDatabase* i_db);

  const std::vector<LevelsPack *> &LevelsPacks();

  static void checkPrerequires();
  static void cleanCache();
  
  void printLevelsList(xmDatabase *i_db) const;

  bool isInFavorite(std::string i_profile, const std::string& i_id_level, xmDatabase *i_db);
  void addToFavorite(std::string i_profile, const std::string& i_id_level, xmDatabase *i_db);
  void delFromFavorite(std::string i_profile, const std::string& i_id_level, xmDatabase *i_db);

  bool isInBlacklist(std::string i_profile, const std::string& i_id_level, xmDatabase *i_db);
  void addToBlacklist(std::string i_profile, const std::string& i_id_level, xmDatabase *i_db);
  void delFromBlacklist(std::string i_profile, const std::string& i_id_level, xmDatabase *i_db);


  static std::string getQuickStartPackQuery(unsigned int i_qualityMIN, unsigned int i_difficultyMIN,
					    unsigned int i_qualityMAX, unsigned int i_difficultyMAX,
					    const std::string& i_profile, const std::string& i_id_room, xmDatabase *i_db);
  void reloadExternalLevels(xmDatabase* i_db, XMotoLoadLevelsInterface *i_loadLevelsInterface = NULL);

  private:
  void clean();
  void cleanPacks();
  void reloadInternalLevels(xmDatabase* i_db, XMotoLoadLevelsInterface *i_loadLevelsInterface = NULL);

  std::vector<LevelsPack *> m_levelsPacks;
};

#endif /* __LEVELSMANAGER__ */
