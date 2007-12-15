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

class WWWAppInterface;

class LevelsPack {
  public:
  LevelsPack(std::string i_name, const std::string& i_sql, bool i_ascSort = true);
  ~LevelsPack();

  std::string Name() const;
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

class LevelsManager {

  public:
  LevelsManager();
  ~LevelsManager();

  LevelsPack& LevelsPackByName(const std::string &i_name);
  std::string LevelByFileName(xmDatabase *i_db, const std::string& i_fileName);
  bool doesLevelExist(xmDatabase *i_db, const std::string& i_id);

  bool doesLevelsPackExist(const std::string &i_name) const;

  /* to load or reload levels from files */
  void makePacks(xmDatabase *i_db,
		 const std::string& i_playerName,
		 const std::string& i_id_room,
		 bool i_bDebugMode,
		 bool i_useCrappyPack);
  void addExternalLevel(xmDatabase *i_db, std::string i_levelFile);
  void reloadExternalLevels(xmDatabase *i_db,
			    XMotoLoadLevelsInterface *i_loadLevelsInterface = NULL);
  void reloadLevelsFromLvl(xmDatabase *i_db,
			   XMotoLoadLevelsInterface *i_loadLevelsInterface = NULL);

  /* to load news levels */
  /* to reload levels already loaded (it will put them into the updateLevels list) */
  /* create the newLevels.xml file */
  void updateLevelsFromLvl(xmDatabase *i_db, const std::vector<std::string> &NewLvl,
			   const std::vector<std::string> &UpdatedLvl,
			   WWWAppInterface* pCaller);

  const std::vector<LevelsPack *> &LevelsPacks();

  static void checkPrerequires();
  static void cleanCache();
  
  void printLevelsList(xmDatabase *i_db) const;

  bool isInFavorite(xmDatabase *i_db, std::string i_profile, const std::string& i_id_level);
  void addToFavorite(xmDatabase *i_db, std::string i_profile, const std::string& i_id_level);
  void delFromFavorite(xmDatabase *i_db, std::string i_profile, const std::string& i_id_level);

  bool isInBlacklist(xmDatabase *i_db, std::string i_profile, const std::string& i_id_level);
  void addToBlacklist(xmDatabase *i_db, std::string i_profile, const std::string& i_id_level);
  void delFromBlacklist(xmDatabase *i_db, std::string i_profile, const std::string& i_id_level);


  static std::string getQuickStartPackQuery(xmDatabase *i_db,
					    unsigned int i_qualityMIN, unsigned int i_difficultyMIN,
					    unsigned int i_qualityMAX, unsigned int i_difficultyMAX,
					    const std::string& i_profile, const std::string& i_id_room);

  private:
  void clean();
  void cleanPacks();

  std::vector<LevelsPack *> m_levelsPacks;
};

#endif /* __LEVELSMANAGER__ */
