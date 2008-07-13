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

#include "xmDatabase.h"
#include "helpers/VExcept.h"
#include "GameText.h"
#include "WWW.h"
#include "Game.h"
#include "helpers/Log.h"
#include "XMSession.h"
#include "VFileIO.h"

#define XMDB_VERSION         27
#define DB_MAX_SQL_RUNTIME 0.25

bool xmDatabase::Trace = false;

xmDatabase::xmDatabase()
{
}

void xmDatabase::init(const std::string& i_dbFile,
		      const std::string& i_profile,
		      const std::string& i_gameDir,
		      const std::string& i_userDir,
		      const std::string& i_binPackCheckSum,
		      bool i_dbDirsCheck,
		      XmDatabaseUpdateInterface *i_interface) {
  int v_version;

  m_requiredLevelsUpdateAfterInit  = false;
  m_requiredReplaysUpdateAfterInit = false;
  m_requiredThemesUpdateAfterInit  = false;

  if(sqlite3_open(i_dbFile.c_str(), &m_db) != 0) {
    throw Exception("Unable to open the database (" + i_dbFile
		    + ") : " + sqlite3_errmsg(m_db));
  }

  sqlite3_trace(m_db, sqlTrace, NULL);
  createUserFunctions();

//  if(sqlite3_threadsafe() == 0) {
//    Logger::Log("** Warning ** Sqlite is not threadSafe !!!");
//  } else {
//    Logger::Log("Sqlite is threadSafe");
//  }

  v_version = getXmDbVersion();
  Logger::Log("XmDb version is %i", v_version);

  if(v_version > XMDB_VERSION) {
    throw Exception("Your XM database required a newer version of xmoto");
  }

  if(v_version < XMDB_VERSION) {
    Logger::Log("Update XmDb version from %i to %i", v_version, XMDB_VERSION);

    if(i_interface != NULL) {
      i_interface->updatingDatabase(GAMETEXT_DB_UPGRADING);
    }
    upgradeXmDbToVersion(v_version, i_profile, i_interface); 
  }

  /* check if gameDir and userDir are the same - otherwise, the computer probably changed */  
  std::string v_oldGameDir = getXmDbGameDir();
  std::string v_oldUserDir = getXmDbUserDir();
  bool v_areDirectoryOK = FS::areSamePath(i_gameDir, v_oldGameDir) && FS::areSamePath(i_userDir, v_oldUserDir);

  if((v_areDirectoryOK == false && i_dbDirsCheck) || i_binPackCheckSum != getXmDbBinPackCheckSum()) {
    m_requiredLevelsUpdateAfterInit  = true;
    m_requiredReplaysUpdateAfterInit = true;
    m_requiredThemesUpdateAfterInit  = true;
    setXmDbGameDir(i_gameDir);
    setXmDbUserDir(i_userDir);
    setXmDbBinPackCheckSum(i_binPackCheckSum);

    /* -- first initialisation or xmoto.bin/userdir update -- */
    webLoadDataFirstTime();
  } else {
    // directory are not ok, but check directory is disabled => update the directories anyway
    if(v_areDirectoryOK == false) {
      setXmDbGameDir(i_gameDir);
      setXmDbUserDir(i_userDir);

      try {
	updateXMDirectories(v_oldGameDir, i_gameDir, v_oldUserDir, i_userDir);
      } catch(Exception &e) {
	/* forcing update */
	m_requiredLevelsUpdateAfterInit  = true;
	m_requiredReplaysUpdateAfterInit = true;
	m_requiredThemesUpdateAfterInit  = true;

	/* -- first initialisation or xmoto.bin/userdir update -- */
	webLoadDataFirstTime();
      }
    }
  }
}

void xmDatabase::updateXMDirectories(const std::string& i_oldGameDir, const std::string& i_newGameDir,
				     const std::string& i_oldUserDir, const std::string& i_newUserDir) {
  Logger::Log("Updating XM directories from %s to %s", i_oldGameDir.c_str(), i_newGameDir.c_str());
  Logger::Log("Updating XM directories from %s to %s", i_oldUserDir.c_str(), i_newUserDir.c_str());

  try {
    simpleSql("BEGIN TRANSACTION;");

    simpleSql("UPDATE levels SET filepath ="
	      " xm_replaceStart(filepath, \"" + protectString(i_oldGameDir) + "\", \"" + protectString(i_newGameDir) + "\")"
	      " WHERE filepath LIKE \""  + protectString(i_oldGameDir) + "%\";");

    simpleSql("UPDATE themes SET filepath ="
	      " xm_replaceStart(filepath, \"" + protectString(i_oldGameDir) + "\", \"" + protectString(i_newGameDir) + "\")"
	      " WHERE filepath LIKE \""  + protectString(i_oldGameDir) + "%\";");

    simpleSql("UPDATE levels SET filepath ="
	      " xm_replaceStart(filepath, \"" + protectString(i_oldUserDir) + "\", \"" + protectString(i_newUserDir) + "\")"
	      " WHERE filepath LIKE \""  + protectString(i_oldUserDir) + "%\";");

    simpleSql("UPDATE themes SET filepath ="
	      " xm_replaceStart(filepath, \"" + protectString(i_oldUserDir) + "\", \"" + protectString(i_newUserDir) + "\")"
	      " WHERE filepath LIKE \""  + protectString(i_oldUserDir) + "%\";");

    simpleSql("COMMIT;");
  } catch(Exception &e) {
    simpleSql("ROLLBACK;");
    throw e;
  }
}


void xmDatabase::init(const std::string& i_dbFile)
{
  if(sqlite3_open(i_dbFile.c_str(), &m_db) != 0){
    throw Exception("Unable to open the database ("
		    + i_dbFile
		    + ") : "
		    + sqlite3_errmsg(m_db));
  }

  sqlite3_trace(m_db, sqlTrace, NULL);
  createUserFunctions();
}
 
xmDatabase::~xmDatabase() {
  sqlite3_close(m_db);
}

void xmDatabase::setTrace(bool i_value) {
  xmDatabase::Trace = i_value;
}

void xmDatabase::sqlTrace(void* arg1, const char* sql) {
  if(Trace) {
    printf("%s\n", sql);
  }
}

/* return false if the parameter doesn't exist */
bool xmDatabase::getXmParameterKey(const std::string& i_key, std::string& o_value) {
  char **v_result;
  unsigned int nrow;

  v_result = readDB("SELECT value FROM xm_parameters WHERE param=\"" + protectString(i_key) + "\";", nrow);
  if(nrow != 1) {
    read_DB_free(v_result);
    return false;
  }

  o_value = getResult(v_result, 1, 0, 0);
  read_DB_free(v_result);

  return true;
}

std::string xmDatabase::getXmDbGameDir() {
  std::string v_dir;

  if(getXmParameterKey("gameDir", v_dir)) {
    return v_dir;
  }
  return "";
}

std::string xmDatabase::getXmDbUserDir() {
  std::string v_dir;

  if(getXmParameterKey("userDir", v_dir)) {
    return v_dir;
  }
  return "";
}

void xmDatabase::setXmParameterKey(const std::string& i_key, const std::string& i_value) {
  char **v_result;
  unsigned int nrow;
  
  v_result = readDB("SELECT value FROM xm_parameters WHERE param=\"" + protectString(i_key) + "\";", nrow);
  read_DB_free(v_result);

  if(nrow == 0) {
    simpleSql("INSERT INTO xm_parameters(param, value) VALUES(\"" + protectString(i_key) + "\", \""
	      + protectString(i_value) + "\");");
  } else {
    simpleSql("UPDATE xm_parameters SET value=\""
	      + protectString(i_value) + "\" WHERE param=\"" + protectString(i_key) + "\"");
  }
}

void xmDatabase::setXmDbGameDir(const std::string& i_gameDir) {
  setXmParameterKey("gameDir", i_gameDir);
}

void xmDatabase::setXmDbUserDir(const std::string& i_userDir) {
  setXmParameterKey("userDir", i_userDir);
}

std::string xmDatabase::getXmDbBinPackCheckSum() {
  std::string v_sum;

  if(getXmParameterKey("binPackCkSum", v_sum)) {
    return v_sum;
  }
  return "";
}

void xmDatabase::setXmDbBinPackCheckSum(const std::string& i_binPackChecksum) {
  setXmParameterKey("binPackCkSum", i_binPackChecksum);
}

int xmDatabase::getXmDbVersion() {
  char **v_result;
  int nrow;
  int ncolumn;
  char *errMsg;
  std::string v_errMsg;
  int v_version;

  /* check if table xm_parameters exists */
  if(checkKey("SELECT count(1) FROM sqlite_master WHERE type='table' AND name='xm_parameters';")
     == false) {
    return 0;
  }

  /* dabatase created, get the version number */
  if(sqlite3_get_table(m_db,
		       "SELECT value FROM xm_parameters WHERE param=\"xmdb_version\";",
		       &v_result, &nrow, &ncolumn, &errMsg)
     != SQLITE_OK) {
    v_errMsg = errMsg;
    sqlite3_free(errMsg);
    throw Exception("Unable to get xmDb version: " + v_errMsg);
  }

  if(nrow != 1) {
    sqlite3_free_table(v_result);
    throw Exception("Unable to get xmDb version: " + v_errMsg);
  }
  v_version = atoi(v_result[1]);
  sqlite3_free_table(v_result);

  return v_version;
}

void xmDatabase::updateXmDbVersion(int i_newVersion) {
  std::ostringstream v_newVersion;
  std::string v_errMsg;

  v_newVersion << i_newVersion;

  simpleSql("UPDATE xm_parameters SET value="+ v_newVersion.str() +
	    " WHERE param=\"xmdb_version\";");
}

std::string xmDatabase::getXmDbSiteKey() {
  std::string v_sitekey;

  if(getXmParameterKey("siteKey", v_sitekey)) {
    return v_sitekey;
  }

  // site key is empty, generate it
  return setXmDbSiteKey();
}

std::string xmDatabase::setXmDbSiteKey() {
  std::string v_sitekey = generateSiteKey();
  setXmParameterKey("siteKey", v_sitekey);
  return v_sitekey;
}

std::string xmDatabase::generateSiteKey() {
  std::string v_sitekey = "";
  struct tm *pTime;
  time_t T;
  char cBuf[256] = "";
  int n;
  std::ostringstream v_rd;

  time(&T);
  pTime = localtime(&T);
  if(pTime != NULL) {
    snprintf(cBuf, 256, "%d%02d%02d%02d%02d%02d",
	     pTime->tm_year+1900, pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec);                    
    v_sitekey += cBuf;
  }
  // add random key
  n = (int) (10000000.0 * (rand() / (RAND_MAX + 1.0)));
  v_rd << n;
  v_sitekey += v_rd.str();

  //printf("siteKey: %s\n", v_sitekey.c_str());
  return v_sitekey;
}

void xmDatabase::upgradeXmDbToVersion(int i_fromVersion,
				      const std::string& i_profile,
				      XmDatabaseUpdateInterface *i_interface) {
  std::string v_errMsg;
  std::string v_sitekey;

  if(i_fromVersion != 0) { /* cannot create site key if xm_parameters doesn't exist */
    v_sitekey = getXmDbSiteKey();
  }

  /* no break in this swicth ! */
  switch(i_fromVersion) {

  case 0:
    try {
      simpleSql("CREATE TABLE xm_parameters(param PRIMARY KEY, value);"
		"INSERT INTO xm_parameters(param, value) VALUES(\"xmdb_version\", 1);");
      v_sitekey = getXmDbSiteKey();
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 0: " + e.getMsg());
    }

  case 1:
    try {
      simpleSql("CREATE TABLE stats_profiles(id_profile PRIMARY KEY, nbStarts, since);"
		"CREATE TABLE stats_profiles_levels("
		"id_profile, id_level, nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime,"
		"PRIMARY KEY(id_profile, id_level));");
      updateXmDbVersion(2);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 1: " + e.getMsg());
    }

    try {
      updateDB_stats(i_interface);
    } catch(Exception &e) {
      Logger::Log(std::string("Oups, updateDB_stats() failed: " + e.getMsg()).c_str());
    }

  case 2:
    try {
      simpleSql("CREATE TABLE levels(id_level PRIMARY KEY,"
		"filepath, name, checkSum, author, description, "
		"date_str, packName, packNum, music, isScripted, isToReload);");
      updateXmDbVersion(3);
      m_requiredLevelsUpdateAfterInit = true;
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 2: " + e.getMsg());
    }

  case 3:
    try {
      simpleSql("CREATE TABLE levels_new(id_level PRIMARY KEY, isAnUpdate);");
      simpleSql("CREATE TABLE levels_favorite(id_profile, id_level, PRIMARY KEY(id_profile, id_level));");
      try {
	updateDB_favorite(i_profile, i_interface);
      } catch(Exception &e) {
	Logger::Log(std::string("Oups, updateDB_favorite() failed: " + e.getMsg()).c_str());
      }
      updateXmDbVersion(4);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 3: " + e.getMsg());
    }

  case 4:
    try {
      simpleSql("CREATE TABLE profile_completedLevels("
		"id_profile, id_level, timeStamp, finishTime);");
      simpleSql("CREATE INDEX profile_completedLevels_idx1 "
		"ON profile_completedLevels(id_profile, id_level);");
      try {
	updateDB_profiles(i_interface);
      } catch(Exception &e) {
	Logger::Log(std::string("Oups, updateDB_profiles() failed: " + e.getMsg()).c_str());
      }
      updateXmDbVersion(5);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 4: " + e.getMsg());
    }

  case 5:
    try {
      simpleSql("CREATE INDEX levels_packName_idx1 ON levels(packName);");
      updateXmDbVersion(6);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 5: " + e.getMsg());
    }

  case 6:
    try {
      simpleSql("CREATE table replays(id_level, name PRIMARY KEY, id_profile, isFinished, finishTime);");
      simpleSql("CREATE INDEX replays_id_level_idx1   ON replays(id_level);");
      simpleSql("CREATE INDEX replays_id_profile_idx1 ON replays(id_profile);");
      m_requiredReplaysUpdateAfterInit = true;
      updateXmDbVersion(7);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 6: " + e.getMsg());
    }

  case 7:
    try {
      simpleSql("CREATE table webhighscores(id_room, id_level, id_profile, finishTime, date, fileUrl, PRIMARY KEY(id_room, id_level));");
      simpleSql("CREATE INDEX webhighscores_id_profile_idx1 ON webhighscores(id_profile);");
      simpleSql("CREATE INDEX webhighscores_date_idx1       ON webhighscores(date);");
      updateXmDbVersion(8);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 7: " + e.getMsg());
    }

  case 8:
    try {
      simpleSql("CREATE table weblevels(id_level, name, fileUrl, checkSum, difficulty, quality, creationDate, PRIMARY KEY(id_level));");
      simpleSql("CREATE INDEX weblevels_difficulty_idx1 ON weblevels(difficulty);");
      simpleSql("CREATE INDEX weblevels_quality_idx1 ON weblevels(quality);");
      simpleSql("CREATE INDEX weblevels_creationDate_idx1 ON weblevels(creationDate);");
      updateXmDbVersion(9);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 8: " + e.getMsg());
    }

  case 9:
    try {
      simpleSql("CREATE table webrooms(id_room PRIMARY KEY, name, highscoresUrl);");
      simpleSql("CREATE INDEX weblevels_name_idx1 ON webrooms(name);");
      simpleSql("INSERT INTO webrooms(id_room, name, highscoresUrl) VALUES ("
		DEFAULT_WEBROOM_ID ", \"" +
		protectString(DEFAULT_WEBROOM_NAME) + "\", \"" +
		protectString(DEFAULT_WEBHIGHSCORES_URL) + "\");");
      updateXmDbVersion(10);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 9: " + e.getMsg());
    }

  case 10:
    try {
      simpleSql("CREATE table themes(id_theme PRIMARY KEY, filepath, checkSum);");
      m_requiredThemesUpdateAfterInit = true;
      updateXmDbVersion(11);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 10: " + e.getMsg());
    }

  case 11:
    try {
      simpleSql("CREATE table webthemes(id_theme PRIMARY KEY, fileUrl, checkSum);");
      updateXmDbVersion(12);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 11: " + e.getMsg());
    }

  case 12:
    try {
      simpleSql("ALTER TABLE stats_profiles_levels ADD COLUMN last_play_date DEFAULT NULL;");
      simpleSql("CREATE INDEX stats_profiles_levels_last_play_date_idx1 ON stats_profiles_levels(last_play_date);");
      updateXmDbVersion(13);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 12: " + e.getMsg());
    }

  case 13:
    try {
      simpleSql("ALTER TABLE weblevels ADD COLUMN crappy DEFAULT 0;");
      simpleSql("CREATE INDEX weblevels_crappy_idx1 ON weblevels(crappy);");
      updateXmDbVersion(14);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 13: " + e.getMsg());
    }

  case 14:
    try {
      simpleSql("CREATE INDEX webhighscores_finishTime_idx1 ON webhighscores(finishTime);");
      updateXmDbVersion(15);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 14: " + e.getMsg());
    }

  case 15:
    try {
      simpleSql("CREATE TABLE levels_blacklist(id_profile, id_level, PRIMARY KEY(id_profile, id_level));");
      updateXmDbVersion(16);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 15: " + e.getMsg());
    }

  case 16:
    try {
      simpleSql("CREATE INDEX webhighscores_id_level_idx1 ON webhighscores(id_level);");
      updateXmDbVersion(17);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 16: " + e.getMsg());
    }

  case 17:
    try {
      // fix stats_profiles_levels due to old old old xmoto versions
      fixStatsProfilesLevelsNbCompleted();
      updateXmDbVersion(18);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 17: " + e.getMsg());
    }

  case 18: // from float to int storage for time
    try {
      m_requiredReplaysUpdateAfterInit = true;
      simpleSql("DELETE from webhighscores;");
      simpleSql("UPDATE profile_completedLevels SET finishTime=finishTime*100;");
      simpleSql("UPDATE stats_profiles_levels   SET playedTime=playedTime*100;");
      updateXmDbVersion(19);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 18: " + e.getMsg());
    }

  case 19:
    try {
      simpleSql("ALTER TABLE weblevels ADD COLUMN children_compliant DEFAULT 1;");
      simpleSql("CREATE INDEX weblevels_children_compliant_idx1 ON weblevels(children_compliant);");
      updateXmDbVersion(20);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 19: " + e.getMsg());
    }

  case 20:
    try {
      simpleSql("CREATE TABLE profiles_configs(id_profile, key, value, PRIMARY KEY(id_profile, key));");
      updateXmDbVersion(21);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 20: " + e.getMsg());
    }

  case 21:
    try {
      updateDB_config("" /* site key still does exist */);
      updateXmDbVersion(22);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 21: " + e.getMsg());
    }

  case 22:
    try {
      // add column sitekey as primary key - sqlite doesn't support that without recreating the table
      simpleSql("ALTER  TABLE stats_profiles RENAME TO stats_profiles_old;");
      simpleSql("CREATE TABLE stats_profiles(sitekey, id_profile, nbStarts, since, PRIMARY KEY(sitekey, id_profile));");
      simpleSql("INSERT INTO  stats_profiles SELECT \"" + protectString(v_sitekey) + "\", id_profile, nbStarts, since FROM stats_profiles_old;");
      simpleSql("DROP   TABLE stats_profiles_old;");

      updateXmDbVersion(23);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 22: " + e.getMsg());
    }

  case 23:
    try {
      // add column sitekey as primary key - sqlite doesn't support that without recreating the table
      simpleSql("ALTER  TABLE stats_profiles_levels RENAME TO stats_profiles_levels_old;");
      simpleSql("CREATE TABLE stats_profiles_levels(sitekey, id_profile, id_level, nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime, last_play_date DEFAULT NULL, synchronized, PRIMARY KEY(sitekey, id_profile, id_level));");
      simpleSql("INSERT INTO  stats_profiles_levels SELECT \"" + protectString(v_sitekey) + "\", id_profile, id_level, nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime, last_play_date, 0 FROM stats_profiles_levels_old;");
      simpleSql("DROP   TABLE stats_profiles_levels_old;");
      /* recreate the index */
      simpleSql("CREATE INDEX stats_profiles_levels_last_play_date_idx1 ON stats_profiles_levels(last_play_date);");
      // index not redondante with the key while it is choiced when sitekey is not set ; make xmoto really faster
      simpleSql("CREATE INDEX stats_profiles_levels_id_profile_id_level_idx1 ON stats_profiles_levels(id_profile, id_level);");

      updateXmDbVersion(24);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 23: " + e.getMsg());
    }

  case 24:
    try {
      simpleSql("ALTER TABLE profile_completedLevels ADD COLUMN sitekey;");
      simpleSql("ALTER TABLE profile_completedLevels ADD COLUMN synchronized;");
      simpleSql("UPDATE profile_completedLevels SET sitekey = \"" + protectString(v_sitekey) + "\";");
      simpleSql("UPDATE profile_completedLevels SET synchronized = 0;");
      simpleSql("DROP INDEX profile_completedLevels_idx1;");
      /* recreate the index */
      simpleSql("CREATE INDEX profile_completedLevels_idx1 "
		"ON profile_completedLevels(sitekey, id_profile, id_level);");
      // index not redondante with the idx1 while it is choiced when sitekey is not set ; make xmoto really faster
      simpleSql("CREATE INDEX profile_completedLevels_idx2 "
		"ON profile_completedLevels(id_profile, id_level);");

      updateXmDbVersion(25);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 24: " + e.getMsg());
    }

  case 25:
    try {
      simpleSql("ALTER TABLE levels ADD COLUMN isPhysics DEFAULT 0;");
      simpleSql("CREATE INDEX levels_isPhysics_idx1 ON levels(isPhysics);");
      updateXmDbVersion(26);
      m_requiredLevelsUpdateAfterInit = true;
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 25: " + e.getMsg());
    }

  case 26:
    try {
      /**
	 dbSync field is the revision in which you upload the line.
	 -> tagging lines where upd=0 and dbsync is null to the last dbsync
	 -> sending via xml all the lines where synchronized=0

	 if server answer ko : do nothing
	 if server answer ok : update synchronized to 1
	 if xmoto crashes, when you restart, xmoto doesn't know the answer of the server :
	 - if the answer was ok, lines with synchronized=0 are resend, but will not be used on the server side on the next update
	 - if the answer was ko, lines with synchronized=0 are resend, and will be used on the server side.

       */
      simpleSql("ALTER TABLE profile_completedLevels ADD COLUMN dbSync DEFAULT NULL;");
      simpleSql("CREATE INDEX profile_completedLevels_dbSync_idx1 ON profile_completedLevels(dbSync);");
      simpleSql("ALTER TABLE stats_profiles_levels ADD COLUMN dbSync DEFAULT NULL;");
      simpleSql("CREATE INDEX stats_profiles_levels_dbSync_idx1 ON stats_profiles_levels(dbSync);");
      simpleSql("INSERT INTO xm_parameters(param, value) VALUES(\"dbSync\", 0);");
      updateXmDbVersion(27);
    } catch(Exception &e) {
      throw Exception("Unable to update xmDb from 26: " + e.getMsg());
    }
    // next
  }
}

bool xmDatabase::checkKey(const std::string& i_sql) {
  char **v_result;
  int nrow, ncolumn;
  char *errMsg;
  std::string v_errMsg;
  int v_nb;

  if(sqlite3_get_table(m_db,
		       i_sql.c_str(),
		       &v_result, &nrow, &ncolumn, &errMsg)
     != SQLITE_OK) {
    v_errMsg = errMsg;
    sqlite3_free(errMsg);
    throw Exception("xmDb: " + v_errMsg);
  }
  
  v_nb = atoi(v_result[1]);
  sqlite3_free_table(v_result);
  
  return v_nb != 0;
}

void xmDatabase::simpleSql(const std::string& i_sql) {
  char *errMsg;
  std::string v_errMsg;

  if(sqlite3_exec(m_db,
		  i_sql.c_str(),
		  NULL,
		  NULL, &errMsg) != SQLITE_OK) {
    v_errMsg = errMsg;
    sqlite3_free(errMsg);
    Logger::Log(i_sql.c_str());
    throw Exception(v_errMsg);
  }
}

void xmDatabase::debugResult(char **i_result, int ncolumn, unsigned int nrow) {
  for(unsigned int i=0; i<ncolumn*(nrow+1); i++) {
    printf("result[%i] = %s\n", i, i_result[i]);
  }
}

char* xmDatabase::getResult(char **i_result, int ncolumn, unsigned int i_row, int i_column) {
  return i_result[ncolumn*(i_row+1) + i_column];
}

char** xmDatabase::readDB(const std::string& i_sql, unsigned int &i_nrow) {
  char **v_result;
  int ncolumn;
  char *errMsg;
  std::string v_errMsg;
  int v_nrow;
	double v_startTime, v_endTime;

  //printf("%s\n", i_sql.c_str());

	v_startTime = GameApp::getXMTime();
  if(sqlite3_get_table(m_db,
		       i_sql.c_str(),
		       &v_result, &v_nrow, &ncolumn, &errMsg)
     != SQLITE_OK) {
    v_errMsg = errMsg;
    sqlite3_free(errMsg);
    Logger::Log("xmDb failed while running");
    Logger::Log("%s", i_sql.c_str());
    throw Exception("xmDb: " + v_errMsg);

  }
	v_endTime = GameApp::getXMTime();

	if(v_endTime - v_startTime > DB_MAX_SQL_RUNTIME) {
		Logger::Log("** Warning ** : long query time detected (%.3f'') for query '%s'", v_endTime - v_startTime, i_sql.c_str());
	}

  i_nrow = (unsigned int) v_nrow;

  return v_result;
}

void xmDatabase::read_DB_free(char **i_result) {
  sqlite3_free_table(i_result);
}

std::string xmDatabase::protectString(const std::string& i_str) {
  std::string v_res;

  for(unsigned int i=0; i<i_str.length(); i++) {
    switch(i_str[i]) {
    case '"':
      v_res.append("\"\"");
      break;
    default:
      char c[2] = {i_str[i], '\0'};
      v_res.append(c);
    }
  }
  return v_res;
}

void xmDatabase::createUserFunctions() {
  if(sqlite3_create_function(m_db, "xm_floord", 1, SQLITE_ANY, NULL,
			     user_xm_floord, NULL, NULL) != SQLITE_OK) {
    throw Exception("xmDatabase::createUserFunctions() failed !");
  }

  if(sqlite3_create_function(m_db, "xm_lvlUpdatedToTxt", 1, SQLITE_ANY, NULL,
			     user_xm_lvlUpdatedToTxt, NULL, NULL) != SQLITE_OK) {
    throw Exception("xmDatabase::createUserFunctions() failed !");
  }

  if(sqlite3_create_function(m_db, "xm_userCrappy", 1, SQLITE_ANY, NULL,
			     user_xm_userCrappy, NULL, NULL) != SQLITE_OK) {
    throw Exception("xmDatabase::createUserFunctions() failed !");
  }

  if(sqlite3_create_function(m_db, "xm_userChildrenCompliant", 1, SQLITE_ANY, NULL,
			     user_xm_userChildrenCompliant, NULL, NULL) != SQLITE_OK) {
    throw Exception("xmDatabase::createUserFunctions() failed !");
  }

  if(sqlite3_create_function(m_db, "xm_replaceStart", 3, SQLITE_ANY, NULL,
			     user_xm_replaceStart, NULL, NULL) != SQLITE_OK) {
    throw Exception("xmDatabase::createUserFunctions() failed !");
  }
}

void xmDatabase::user_xm_floord(sqlite3_context* i_context, int i_nArgs, sqlite3_value** i_values) {
  double v_value;

  if(i_nArgs != 1) {
    throw Exception("user_xm_floord failed !");
  }

  v_value = sqlite3_value_double(i_values[0]);
  sqlite3_result_double(i_context, (double)((int)(v_value)));
}

void xmDatabase::user_xm_lvlUpdatedToTxt(sqlite3_context* i_context, int i_nArgs, sqlite3_value** i_values) {
  int v_value;

  if(i_nArgs != 1) {
    throw Exception("user_xm_lvlUpdatedToTxt failed !");
  }

  v_value = sqlite3_value_int(i_values[0]);
  sqlite3_result_text(i_context, v_value == 0 ? GAMETEXT_NEW : GAMETEXT_UPDATED, -1, SQLITE_TRANSIENT);
}

void xmDatabase::user_xm_userCrappy(sqlite3_context* i_context, int i_nArgs, sqlite3_value** i_values) {
  int v_value;

  if(i_nArgs != 1) {
    throw Exception("user_xm_userCrappy failed !");
  }

  v_value = sqlite3_value_int(i_values[0]);
  sqlite3_result_int(i_context, XMSession::instance()->useCrappyPack() ? v_value : 0);
}

void xmDatabase::user_xm_userChildrenCompliant(sqlite3_context* i_context, int i_nArgs, sqlite3_value** i_values) {
  int v_value;

  if(i_nArgs != 1) {
    throw Exception("user_xm_userChildrenCompliant failed !");
  }

  v_value = sqlite3_value_int(i_values[0]);
  sqlite3_result_int(i_context, XMSession::instance()->useChildrenCompliant() ? v_value : 1);
}

void xmDatabase::user_xm_replaceStart(sqlite3_context* i_context, int i_nArgs, sqlite3_value** i_values) {
  std::string v_value, v_old, v_new, v_result, v_begin, v_end;

  if(i_nArgs != 3) {
    throw Exception("user_xm_replaceStart failed !");
  }

  v_value = (char*) sqlite3_value_text(i_values[0]);
  v_old   = (char*) sqlite3_value_text(i_values[1]);
  v_new   = (char*) sqlite3_value_text(i_values[2]);

  v_begin = v_value.substr(0, v_old.length());

  if(v_begin != v_old) {
    /* change nothing */
    sqlite3_result_text(i_context, v_value.c_str(), -1, SQLITE_TRANSIENT);
    return;
  }

  v_end = v_value.substr(v_old.length(), v_value.length() - v_old.length());
  v_result = v_new + v_end;

  sqlite3_result_text(i_context, v_result.c_str(), -1, SQLITE_TRANSIENT);
}