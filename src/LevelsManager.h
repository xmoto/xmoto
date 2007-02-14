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

#ifndef __LEVELMANAGER_H__
#define __LEVELMANAGER_H__

#include <string>
#include "xmscene/Level.h"
#include "PlayerData.h"
#include "Stats.h"

class LevelsPack {
  public:
  LevelsPack(std::string i_name);
  ~LevelsPack();

  std::string Name() const;
  void addLevel(Level *i_level, bool i_checkUnique = false);
  void removeLevel(Level *i_level);
  const std::vector<Level *> &Levels();
  bool ShowTimes() const;
  bool ShowWebTimes() const;
  bool Sorted() const;
  void setShowTimes(bool i_showTimes);
  void setShowWebTimes(bool i_showWebTimes);
  void setSorted(bool i_sorted);

  private:
  void setHintsFromFile();

  std::string m_name;
  std::vector<Level *> m_levels;
  bool m_showTimes;
  bool m_showWebTimes;
  bool m_sorted;
};

class LevelsManager {

  public:
  LevelsManager();
  ~LevelsManager();

  LevelsPack& LevelsPackByName(const std::string &i_name);
  Level& LevelById(const std::string& i_id);
  Level& LevelByFileName(const std::string& i_fileName);
  bool doesLevelExist(const std::string& i_id);

  bool doesLevelsPackExist(const std::string &i_name) const;
  void rebuildPacks(
#if defined(SUPPORT_WEBACCESS)
		    WebRoom *i_webHighscores,
		    WebLevels *i_webLevels,
#endif
		    std::string i_playerName,
		    vapp::PlayerData *i_profiles,
		    vapp::Stats *i_stats);

  /* to load or reload levels from files */
  void reloadLevelsFromFiles(bool i_enableCache);
  /* to load the levels from the index */
  void loadLevelsFromIndex();
  /* to load new levels */
  void loadLevelsFromLvl(const std::vector<std::string> &LvlFiles, bool i_enableCache);

  /* to load news levels */
  /* to reload levels already loaded (it will put them into the updateLevels list) */
  /* create the newLevels.xml file */
  void updateLevelsFromLvl(const std::vector<std::string> &NewLvl,
													 const std::vector<std::string> &UpdatedLvlFileNames,
													 const std::vector<std::string> &UpdatedLvlIds,
													 bool i_enableCache);

  const std::vector<Level *> &Levels();
  const std::vector<LevelsPack *> &LevelsPacks();

  const std::vector<Level *> &NewLevels();
  const std::vector<Level *> &UpdatedLevels();

  static void checkPrerequires(bool &v_enableCache);
  void createLevelsIndex();
  static void deleteLevelsIndex();
  static void cleanCache();
  
  void printLevelsList() const;

  void addToFavorite(Level *i_level);
  void delFromFavorite(Level *i_level);
  const std::vector<Level *> &FavoriteLevels();

  /* save the files so that when you reinit, it's restored */
  void saveXml() const;

  private:
  void createVirtualPacks(
#if defined(SUPPORT_WEBACCESS)
			  WebRoom *i_webHighscores,
			  WebLevels *i_webLevels,
#endif
			  std::string i_playerName,
			  vapp::PlayerData *i_profiles,
			  vapp::Stats *i_stats);
  void clean();
  static std::string LevelIndexFileName();

  std::vector<Level *> m_levels;
  std::vector<LevelsPack *> m_levelsPacks;

  std::vector<Level *> m_newLevels;
  std::vector<Level *> m_updatedLevels;

  std::vector<Level *> m_favoriteLevels;

  static std::string NewLevelsXmlFilePath();
  void saveNewLevelsXml() const;
  void loadNewLevelsXml();

  static std::string FavoriteLevelsXmlFilePath();
  void saveFavoriteLevelsXml() const;
  void loadFavoriteLevelsXml();

  /* is private to for externals to use updateLevelsFromLvl for new levels */
  void loadLevelsFromLvl(const std::vector<std::string> &LvlFiles, bool i_enableCache, bool i_newLevels);
};

#endif /* __LEVELSMANAGER__ */
