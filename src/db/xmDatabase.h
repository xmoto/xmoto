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

#ifndef __XMDATABASE_H__
#define __XMDATABASE_H__

#include <sqlite3.h>
#include <string>
#include <vector>
#include "xmDatabaseUpdateInterface.h"
#include "helpers/MultiSingleton.h"
#include "common/VFileIO_types.h"

class Level;

class xmDatabase : public MultiSingleton<xmDatabase> {
  friend class MultiSingleton<xmDatabase>;

private:
  xmDatabase();
  virtual ~xmDatabase();

public:
  void preInitForProfileLoading(const std::string& i_dbFileUTF8);
  // global init (upgrade data model)
  void init(const std::string& i_dbFileUTF8,
	    const std::string& i_profile,
	    const std::string& i_gameDataDir,
	    const std::string& i_userDataDir,
	    const std::string& i_binPackCheckSum,
	    bool i_dbDirsCheck,
	    XmDatabaseUpdateInterface *i_interface = NULL);
  // simple init (for subthreads)
  void init(const std::string& i_dbFileUTF8, bool i_readOnly = false);
  void setUpdateAfterInitDone(); // call once, update after init are done
  int  getXmDbVersion();
  static int getMemoryUsed();

  /* RULE:
     all write access must be done from class xmDatabase
     read can be done from anywhere using readDB;
  */
  char **readDB(const std::string& i_sql, unsigned int &i_nrow);
  void read_DB_free(char **i_result);
  char* getResult(char **i_result, int ncolumn, unsigned int i_row, int i_column);
  void debugResult(char **i_result, int ncolumn, unsigned int nrow);
  static std::string protectString(const std::string& i_str);
  static void setTrace(bool i_value);

  /* stats */
  void stats_createProfile(const std::string& i_sitekey, const std::string& i_profile);
  void stats_destroyProfile(const std::string& i_profile);
  bool stats_checkKeyExists_stats_profiles(const std::string& i_sitekey, const std::string& i_profile);
  void stats_levelCompleted(const std::string& i_sitekey,
			    const std::string &PlayerName,
			    const std::string &LevelID,
			    int i_playTime);
  void stats_died(const std::string& i_sitekey,
		  const std::string &PlayerName,
		  const std::string &LevelID,
		  int i_playTime);
  void stats_abortedLevel(const std::string& i_sitekey,
			  const std::string &PlayerName,
			  const std::string &LevelID,
			  int i_playTime);
  void stats_levelRestarted(const std::string& i_sitekey,
			    const std::string &PlayerName,
			    const std::string &LevelID,
			    int i_playTime);
  void stats_xmotoStarted(const std::string& i_sitekey, const std::string &PlayerName);

  /* levels */
  void levels_updateDB(const std::vector<Level *>& i_levels,
		       bool i_isToReload,
		       XmDatabaseUpdateInterface *i_interface = NULL); /* levels from files */
  int  levels_nbLevelsToDownload();
  bool levels_isIndexUptodate() const;
  void levels_addToFavorite(const std::string& i_profile, const std::string& i_id_level);
  void levels_delToFavorite(const std::string& i_profile, const std::string& i_id_level);
  void levels_addToBlacklist(const std::string& i_profile, const std::string& i_id_level);
  void levels_delToBlacklist(const std::string& i_profile, const std::string& i_id_level);
  void levels_cleanNew();
  void levels_addToNew(const std::string& i_id_level, bool i_isAnUpdate);
  void levels_addToNew_begin();
  void levels_addToNew_end();

  void levels_add_begin(bool i_isToReload);
  bool levels_add_fast(const std::string& i_filepath, std::string& o_levelName, bool i_isToReload);
  void levels_add(const std::string& i_id_level,
		  const std::string& i_filepath, const std::string& i_name,
		  const std::string& i_checkSum,
		  const std::string& i_author, const std::string& i_description,
		  const std::string& i_date,
		  const std::string& i_music, bool i_isScripted, bool i_isPhysics,
		  bool i_isToReload);
  void levels_update(const std::string& i_id_level,
		     const std::string& i_filepath, const std::string& i_name,
		     const std::string& i_checkSum,
		     const std::string& i_author, const std::string& i_description,
		     const std::string& i_date,
		     const std::string& i_music, bool i_isScripted, bool i_isPhysics,
		     bool i_isToReload);
  void levels_add_end();
  void levels_cleanNoWWWLevels();

  /* replays */
  bool replays_isIndexUptodate() const;
  void replays_add_begin();
  void replays_add(const std::string& i_id_level,
		   const std::string& i_name,
		   const std::string& i_id_profile,
		   bool               i_isFinished,
		   int                i_finishTime);
  void replays_add_end();
  void replays_delete(const std::string& i_replay);
  bool replays_exists(const std::string& i_name);
  void replays_print();

  /* themes */
  bool themes_isIndexUptodate() const;
  void themes_add_begin();
  void themes_add(const std::string& i_id_theme,
		  const std::string& i_filepath);
  void themes_update(const std::string& i_id_theme,
		     const std::string& i_filepath);
  void themes_add_end();
  void themes_delete(const std::string& i_theme);
  bool themes_exists(const std::string& i_id_theme);
  std::string themes_getFileName(const std::string& i_id_theme);

  /* profiles */
  void profiles_addFinishTime(const std::string& i_sitekey, const std::string& i_profile, const std::string& i_id_level,
			      const std::string& i_timeStamp, int i_finishTime);
  /* for old migration, when sitekey didn't exist */
  void profiles_addFinishTime_nositekey(const std::string& i_profile, const std::string& i_id_level,
			      const std::string& i_timeStamp, int i_finishTime);

  /* web */
  std::string webhighscores_updateDB(FileDataType i_fdt, const std::string& i_webhighscoresFile,
				     const std::string& i_websource);
  void weblevels_updateDB(FileDataType i_fdt, const std::string& i_weblevelsFile);
  void webrooms_updateDB(FileDataType i_fdt, const std::string& i_webroomsFile);
  void webthemes_updateDB(FileDataType i_fdt, const std::string& i_webThemesFile);
  std::string webrooms_getName(const std::string& i_id_room);
  int webrooms_getHighscoreTime(const std::string& i_id_room,
				const std::string& i_id_level); /* or a negativ value if not */
  void webLoadDataFirstTime();
  bool isOnTheWeb(const std::string& i_id_level);
  bool isWebVoteLocked(const std::string& i_id_level);
  void updateMyHighscoresFromHighscores(const std::string& i_id_profile);
  void updateMyHighscoresKnownStolenBack(const std::string& i_id_profile);
  bool markMyHighscoresKnownStolen(const std::string& i_id_profile, std::string& o_stolen_msg);

  /* configuration */
	/* if you save several values, use begin and end to make it faster */
	void config_setValue_begin();
	void config_setValue_end();
  std::string config_getString(const std::string& i_id_profile, const std::string& i_key,  const std::string& i_default);
  bool        config_getBool(const std::string& i_id_profile, const std::string& i_key,    bool i_default);
  int         config_getInteger(const std::string& i_id_profile, const std::string& i_key, int i_default);
  float       config_getFloat(const std::string& i_id_profile, const std::string& i_key,   float i_default);
  void        config_setString(const std::string& i_id_profile, const std::string& i_key,  const std::string& i_value);
  void        config_setBool(const std::string& i_id_profile, const std::string& i_key,    bool i_value);
  void        config_setInteger(const std::string& i_id_profile, const std::string& i_key, int i_value);
  void        config_setFloat(const std::string& i_id_profile, const std::string& i_key,   float i_value);

  // sync
  std::string getXmDbSiteKey();
  void sync_buildServerFile(const std::string& i_outFile, const std::string& i_sitekey, const std::string& i_profile);
  void sync_updateDB(const std::string& i_profile, const std::string& i_sitekey, const std::string& i_file, int i_newDbSyncServer);

  /**
     @brief Mark all lines that should be synchronized as synchronized
   */
  void setSynchronized();

  /* votes */
  void markAsVoted(const std::string& i_profile, const std::string& i_id_level);
  bool isVoted(const std::string& i_profile, const std::string& i_id_level);

  /* server cmd */
  bool srv_isAdmin(const std::string& i_profile, const std::string& i_password);
  void srv_addAdmin(const std::string& i_profile, const std::string& i_password);
  void srv_removeAdmin(int id);
  void srv_addBan(const std::string& i_profile, const std::string& i_ip, unsigned int i_nbDays, const std::string& i_admin_banner);
  void srv_removeBan(int id);
  bool srv_isBanned(const std::string& i_profile, const std::string& i_ip);
  void srv_cleanBans();
  void srv_changePassword(const std::string& i_profile, const std::string& i_password);

  /* data fixes */
  void fixStatsProfilesLevelsNbCompleted();

  private:
  sqlite3 *m_db;
  bool m_requiredLevelsUpdateAfterInit;
  bool m_requiredReplaysUpdateAfterInit;
  bool m_requiredThemesUpdateAfterInit;
  static bool Trace;

  // internal opening
  void openIfNot(const std::string& i_dbFileUTF8);
  int m_openingVersion;

  /* add user function for db */
  void createUserFunctions();
  static void user_xm_floord(sqlite3_context* i_context, int i_nArgs, sqlite3_value** i_values);
  static void user_xm_lvlUpdatedToTxt(sqlite3_context* i_context, int i_nArgs, sqlite3_value** i_values);
  static void user_xm_userCrappy(sqlite3_context* i_context, int i_nArgs, sqlite3_value** i_values);
  static void user_xm_userChildrenCompliant(sqlite3_context* i_context, int i_nArgs, sqlite3_value** i_values);
  static void user_xm_replaceStart(sqlite3_context* i_context, int i_nArgs, sqlite3_value** i_values);
  static void user_xm_profile(sqlite3_context* i_context, int i_nArgs, sqlite3_value** i_values);
  static void user_xm_idRoom(sqlite3_context* i_context, int i_nArgs, sqlite3_value** i_values);

  /* function used to synchronise with the last xmoto version */
  void upgradeXmDbToVersion(int i_fromVersion,
			    const std::string& i_profile,
			    XmDatabaseUpdateInterface *i_interface = NULL);
  void updateXmDbVersion(int i_newVersion, XmDatabaseUpdateInterface *i_interface);
  std::string getXmDbGameDataDir();
  std::string getXmDbUserDataDir();
  std::string getXmDbBinPackCheckSum();
  void setXmDbGameDataDir(const std::string& i_gameDataDir);
  void setXmDbUserDataDir(const std::string& i_userDataDir);
  void setXmDbBinPackCheckSum(const std::string& i_binPackChecksum);

  /* update db directories without reanalysing all the files on the disk */
  void updateXMDirectories(const std::string& i_oldGameDataDir, const std::string& i_newGameDataDir,
			   const std::string& i_oldUserDataDir, const std::string& i_newUserDataDir);

  std::string setXmDbSiteKey();
  
  /* multiple sites */
  static std::string generateSiteKey();

  /* config factorize sql*/
  bool getXmParameterKey(const std::string& i_key, std::string& o_value); // get a parameter value as string
  void setXmParameterKey(const std::string& i_key, const std::string& i_value); // set a parameter value as string
  std::string config_getValue(const std::string& i_id_profile, const std::string& i_key, unsigned int& o_nrow);
  void config_setValue(const std::string& i_id_profile, const std::string& i_key, const std::string& i_value);

  /* if i_sitekey is empty, don't care of it -- require for old db without sitekey */
  void updateDB_config(const std::string& i_sitekey); /* conversion of the old file values */

  /* trace */
  static void sqlTrace(void*, const char* sql);

  /* i_sql must be of form select count() from */
  bool checkKey(const std::string& i_sql);
  void simpleSql(const std::string& i_sql);
  bool doesTableExists(const std::string& i_table);

  /* conversion of old files */
  void updateDB_stats(XmDatabaseUpdateInterface *i_interface = NULL);    /* statistics */
  void updateDB_favorite(const std::string& i_profile,
			 XmDatabaseUpdateInterface *i_interface = NULL); /* favorite */
  bool stats_checkKeyExists_stats_profiles_levels(const std::string& i_sitekey,
						  const std::string& i_profile,
						  const std::string& i_level);
  void updateDB_profiles(XmDatabaseUpdateInterface *i_interface = NULL); /* profiles */

  /* web */
  bool webrooms_checkKeyExists_id_room(const std::string& i_id_room);
  void webrooms_addRoom(const std::string& i_id_room, const std::string& i_name,
			const std::string& i_highscoreUrl);
  void webthemes_addTheme(const std::string& i_id_theme, const std::string& i_url,
			  const std::string& i_checkSum);
};

#endif
