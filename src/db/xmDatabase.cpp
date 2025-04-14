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
#include "common/VFileIO.h"
#include "common/WWW.h"
#include "common/XMSession.h"
#include "helpers/Log.h"
#include "helpers/Time.h"
#include "helpers/VExcept.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"
#include "xmoto/UserConfig.h"
#include "xmoto/input/Input.h"
#include "xmoto/input/InputLegacy.h"
#include <sstream>
#include <utility>

#define XMDB_VERSION 38
#define DB_MAX_SQL_RUNTIME 0.25
#define DB_BUSY_TIMEOUT 60000 // 60 seconds

bool xmDatabase::Trace = false;

xmDatabase::xmDatabase() {
  m_db = NULL;
  m_openingVersion = -1;
}

void xmDatabase::setUpdateAfterInitDone() {
  setXmParameterKey("requireUpdateAfterInit", "0");
}

void xmDatabase::openIfNot(const std::string &i_dbFileUTF8) {
  if (m_db != NULL)
    return;

  std::string dbFile = XMFS::getUserDirUTF8(FDT_DATA) + "/" + i_dbFileUTF8;

  // LogDebug("openDB(%X)", this);
  if (sqlite3_open(dbFile.c_str(), &m_db) != 0) {
    throw Exception("Unable to open the database (" + i_dbFileUTF8 +
                    ") : " + sqlite3_errmsg(m_db));
  }

#ifdef SQLITE_DBCONFIG_DQS_DML
  sqlite3_db_config(m_db, SQLITE_DBCONFIG_DQS_DML, 1, NULL);
#endif

  sqlite3_busy_timeout(m_db, DB_BUSY_TIMEOUT);
  sqlite3_trace(m_db, sqlTrace, NULL);
  createUserFunctions();

  //  if(sqlite3_threadsafe() == 0) {
  //    LogWarning("Sqlite is not threadSafe !!!");
  //  } else {
  //    LogInfo("Sqlite is threadSafe");
  //  }

  m_openingVersion = getXmDbVersion();
}

void xmDatabase::preInitForProfileLoading(const std::string &i_dbFileUTF8) {
  openIfNot(i_dbFileUTF8);

  // in case your db structure is not exactly the same, add some additionnal
  // checks
  if (m_openingVersion == XMDB_VERSION) {
    return;
  }

  // be sure upgrade of xm_parameters table is done -- xm_parameters is required
  // cause to the siteKey required for the profile
  if (doesTableExists("xm_parameters") == false) {
    simpleSql("CREATE TABLE xm_parameters(param PRIMARY KEY, value);");
  }

  // be sure upgrade of profiles_configs_configs is done -- profiles_configs is
  // required to query values of profile config
  if (doesTableExists("profiles_configs") == false) {
    simpleSql("CREATE TABLE profiles_configs(id_profile, key, value, PRIMARY "
              "KEY(id_profile, key));");
  }
}

void xmDatabase::backupXmDb(const std::string &dbFile) {
  std::ostringstream backupName;
  backupName << "xm.v" << m_openingVersion << "." << currentDateTime() << ".db";

  std::string outputPath;
  if (!XMFS::copyFile(
        FDT_DATA, dbFile, "Backups/" + backupName.str(), outputPath, true)) {
    throw Exception("Failed to copy database file");
  }
  LogInfo("Database backed up to: %s", outputPath.c_str());
}

bool xmDatabase::init(const std::string &i_dbFileUTF8,
                      const std::string &i_profile,
                      const std::string &i_gameDataDir,
                      const std::string &i_userDataDir,
                      const std::string &i_binPackCheckSum,
                      bool i_dbDirsCheck,
                      XmDatabaseUpdateInterface *i_interface) {
  m_requiredLevelsUpdateAfterInit = false;
  m_requiredReplaysUpdateAfterInit = false;
  m_requiredThemesUpdateAfterInit = false;

  openIfNot(i_dbFileUTF8);
  LogInfo("XmDb version is %i", m_openingVersion);

  // mark all objects as requiring an update
  // will be set off - aims to allow people killing xmoto while updating

  // if the previous value was not 0, force update
  std::string v_previousRequireUpdateAfterInit;

  // the version 0 is special (in that xm_parameters doesn't exist yet)
  if (m_openingVersion > 0) {
    if (getXmParameterKey("requireUpdateAfterInit",
                          v_previousRequireUpdateAfterInit) == false) {
      m_requiredLevelsUpdateAfterInit = true;
      m_requiredReplaysUpdateAfterInit = true;
      m_requiredThemesUpdateAfterInit = true;
    } else {
      if (v_previousRequireUpdateAfterInit == "1") {
        m_requiredLevelsUpdateAfterInit = true;
        m_requiredReplaysUpdateAfterInit = true;
        m_requiredThemesUpdateAfterInit = true;
      }
    }
  }

  if (m_openingVersion > XMDB_VERSION) {
    throw Exception("Your XM database required a newer version of xmoto");
  }

  if (m_openingVersion < XMDB_VERSION) {
    if (m_openingVersion != 0) {
      LogInfo("Backing up XmDb");

      try {
        backupXmDb(i_dbFileUTF8);
      } catch (Exception &e) {
        LogError("Failed to backup database:");
        LogError(e.getMsg().c_str());
        LogError("Bailing out..");

        if (m_db != NULL) {
          sqlite3_close(m_db);
          m_db = NULL;
        }

        return false;
      }
    }

    LogInfo(
      "Updating XmDb version from %i to %i", m_openingVersion, XMDB_VERSION);

    if (i_interface != NULL) {
      i_interface->updatingDatabase(GAMETEXT_DB_UPGRADING, 0);
    }
    // now, mark it as required in any case if case the player kills xmoto
    if (m_openingVersion >
        0) { // the version 0 is special => xm_parameters still doesn't exist
      setXmParameterKey("requireUpdateAfterInit", "1");
    }

    upgradeXmDbToVersion(m_openingVersion, i_profile, i_interface);
  }

  std::string v_oldGameDataDir = getXmDbGameDataDir();
  std::string v_oldUserDataDir = getXmDbUserDataDir();
  bool v_areDirectoryOK = XMFS::areSamePath(i_userDataDir, v_oldUserDataDir);

  if ((!v_areDirectoryOK && i_dbDirsCheck) ||
      i_binPackCheckSum != getXmDbBinPackCheckSum()) {
    m_requiredLevelsUpdateAfterInit = true;
    m_requiredReplaysUpdateAfterInit = true;
    m_requiredThemesUpdateAfterInit = true;

    // now, mark it as required in any case if case the player kills xmoto
    setXmParameterKey("requireUpdateAfterInit", "1");

    setXmDbGameDataDir(i_gameDataDir);
    setXmDbUserDataDir(i_userDataDir);
    setXmDbBinPackCheckSum(i_binPackCheckSum);

    /* -- first initialisation or xmoto.bin/userdatadir update -- */
    webLoadDataFirstTime();
  } else {
    // directory are not ok, but check directory is disabled => update the
    // directories anyway
    if (!v_areDirectoryOK) {
      // now, mark it as required in any case if case the player kills xmoto
      setXmParameterKey("requireUpdateAfterInit", "1");

      setXmDbGameDataDir(i_gameDataDir);
      setXmDbUserDataDir(i_userDataDir);

      try {
        updateXMDirectories(
          v_oldGameDataDir, i_gameDataDir, v_oldUserDataDir, i_userDataDir);
      } catch (Exception &e) {
        /* forcing update */
        m_requiredLevelsUpdateAfterInit = true;
        m_requiredReplaysUpdateAfterInit = true;
        m_requiredThemesUpdateAfterInit = true;

        /* -- first initialisation or xmoto.bin/userdatadir update -- */
        webLoadDataFirstTime();
      }
    }
  }

  return true;
}

void xmDatabase::updateXMDirectories(const std::string &i_oldGameDataDir,
                                     const std::string &i_newGameDataDir,
                                     const std::string &i_oldUserDataDir,
                                     const std::string &i_newUserDataDir) {
  LogInfo("Updating XM directories from %s to %s",
          i_oldGameDataDir.c_str(),
          i_newGameDataDir.c_str());
  LogInfo("Updating XM directories from %s to %s",
          i_oldUserDataDir.c_str(),
          i_newUserDataDir.c_str());

  try {
    simpleSql("BEGIN TRANSACTION;");

    simpleSql("UPDATE levels SET filepath ="
              " xm_replaceStart(filepath, \"" +
              protectString(i_oldGameDataDir) + "\", \"" +
              protectString(i_newGameDataDir) +
              "\")"
              " WHERE filepath LIKE \"" +
              protectString(i_oldGameDataDir) + "%\";");

    simpleSql("UPDATE themes SET filepath ="
              " xm_replaceStart(filepath, \"" +
              protectString(i_oldGameDataDir) + "\", \"" +
              protectString(i_newGameDataDir) +
              "\")"
              " WHERE filepath LIKE \"" +
              protectString(i_oldGameDataDir) + "%\";");

    simpleSql("UPDATE levels SET filepath ="
              " xm_replaceStart(filepath, \"" +
              protectString(i_oldUserDataDir) + "\", \"" +
              protectString(i_newUserDataDir) +
              "\")"
              " WHERE filepath LIKE \"" +
              protectString(i_oldUserDataDir) + "%\";");

    simpleSql("UPDATE themes SET filepath ="
              " xm_replaceStart(filepath, \"" +
              protectString(i_oldUserDataDir) + "\", \"" +
              protectString(i_newUserDataDir) +
              "\")"
              " WHERE filepath LIKE \"" +
              protectString(i_oldUserDataDir) + "%\";");

    simpleSql("COMMIT;");
  } catch (Exception &e) {
    simpleSql("ROLLBACK;");
    throw e;
  }
}

void xmDatabase::init(const std::string &i_dbFile, bool i_readOnly) {
  /* my sqlite version doesn't allow readonly ; wait to allow this */
  //  if(i_readOnly) {
  //    LogDebug("Database opened in read-only mode");
  //    if(sqlite3_open_v2(i_dbFile.c_str(), &m_db, SQLITE_OPEN_READONLY, NULL)
  //    != 0){
  //      if(m_db != NULL) {
  //	sqlite3_close(m_db); // close even if it fails as requested in the
  // documentation
  //	m_db = NULL;
  //      }
  //      throw Exception("Unable to open the database (" + i_dbFile + ") : " +
  //      sqlite3_errmsg(m_db));
  //    }
  //  } else {
  //

  openIfNot(i_dbFile);
}

xmDatabase::~xmDatabase() {
  if (m_db != NULL) {
    sqlite3_close(m_db);
    m_db = NULL;
  }
}

void xmDatabase::setTrace(bool i_value) {
  xmDatabase::Trace = i_value;
}

void xmDatabase::sqlTrace(void *arg1, const char *sql) {
  if (Trace) {
    printf("%s\n", sql);
  }
}

/* return false if the parameter doesn't exist */
bool xmDatabase::getXmParameterKey(const std::string &i_key,
                                   std::string &o_value) {
  char **v_result;
  unsigned int nrow;

  v_result = readDB("SELECT value FROM xm_parameters WHERE param=\"" +
                      protectString(i_key) + "\";",
                    nrow);
  if (nrow != 1) {
    read_DB_free(v_result);
    return false;
  }

  o_value = getResult(v_result, 1, 0, 0);
  read_DB_free(v_result);

  return true;
}

std::string xmDatabase::getXmDbGameDataDir() {
  std::string v_dir;

  if (getXmParameterKey("gameDir", v_dir)) {
    return v_dir;
  }
  return "";
}

std::string xmDatabase::getXmDbUserDataDir() {
  std::string v_dir;

  if (getXmParameterKey("userDir", v_dir)) {
    return v_dir;
  }
  return "";
}

void xmDatabase::setXmParameterKey(const std::string &i_key,
                                   const std::string &i_value) {
  char **v_result;
  unsigned int nrow;

  v_result = readDB("SELECT value FROM xm_parameters WHERE param=\"" +
                      protectString(i_key) + "\";",
                    nrow);
  read_DB_free(v_result);

  if (nrow == 0) {
    simpleSql("INSERT INTO xm_parameters(param, value) VALUES(\"" +
              protectString(i_key) + "\", \"" + protectString(i_value) +
              "\");");
  } else {
    simpleSql("UPDATE xm_parameters SET value=\"" + protectString(i_value) +
              "\" WHERE param=\"" + protectString(i_key) + "\"");
  }
}

void xmDatabase::setXmDbGameDataDir(const std::string &i_gameDataDir) {
  setXmParameterKey("gameDir", i_gameDataDir);
}

void xmDatabase::setXmDbUserDataDir(const std::string &i_userDataDir) {
  setXmParameterKey("userDir", i_userDataDir);
}

std::string xmDatabase::getXmDbBinPackCheckSum() {
  std::string v_sum;

  if (getXmParameterKey("binPackCkSum", v_sum)) {
    return v_sum;
  }
  return "";
}

void xmDatabase::setXmDbBinPackCheckSum(const std::string &i_binPackChecksum) {
  setXmParameterKey("binPackCkSum", i_binPackChecksum);
}

int xmDatabase::getMemoryUsed() {
  return sqlite3_memory_used();
}

bool xmDatabase::doesTableExists(const std::string &i_table) {
  return checkKey(
    "SELECT count(1) FROM sqlite_master WHERE type='table' AND name='" +
    i_table + "';");
}

int xmDatabase::getXmDbVersion() {
  char **v_result;
  int nrow;
  int ncolumn;
  char *errMsg;
  std::string v_errMsg;
  int v_version;

  /* check if table xm_parameters exists */
  if (doesTableExists("xm_parameters") == false) {
    return 0;
  }

  /* dabatase created, get the version number */
  if (sqlite3_get_table(
        m_db,
        "SELECT value FROM xm_parameters WHERE param=\"xmdb_version\";",
        &v_result,
        &nrow,
        &ncolumn,
        &errMsg) != SQLITE_OK) {
    v_errMsg = errMsg;
    sqlite3_free(errMsg);
    throw Exception("Unable to get xmDb version: " + v_errMsg);
  }

  if (nrow != 1) {
    sqlite3_free_table(v_result);
    throw Exception("Unable to get xmDb version: " + v_errMsg);
  }
  v_version = atoi(v_result[1]);
  sqlite3_free_table(v_result);

  return v_version;
}

void xmDatabase::updateXmDbVersion(int i_newVersion,
                                   XmDatabaseUpdateInterface *i_interface) {
  std::ostringstream v_newVersion;
  std::string v_errMsg;

  v_newVersion << i_newVersion;

  simpleSql("UPDATE xm_parameters SET value=" + v_newVersion.str() +
            " WHERE param=\"xmdb_version\";");

  i_interface->updatingDatabase(GAMETEXT_DB_UPGRADING,
                                (int)((i_newVersion * 100) / XMDB_VERSION));
}

std::string xmDatabase::getXmDbSiteKey() {
  std::string v_sitekey;

  if (getXmParameterKey("siteKey", v_sitekey)) {
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
  if (pTime != NULL) {
    snprintf(cBuf,
             256,
             "%d%02d%02d%02d%02d%02d",
             pTime->tm_year + 1900,
             pTime->tm_mon + 1,
             pTime->tm_mday,
             pTime->tm_hour,
             pTime->tm_min,
             pTime->tm_sec);
    v_sitekey += cBuf;
  }
  // add random key
  n = (int)(10000000.0 * (rand() / (RAND_MAX + 1.0)));
  v_rd << n;
  v_sitekey += v_rd.str();

  // printf("siteKey: %s\n", v_sitekey.c_str());
  return v_sitekey;
}

void xmDatabase::upgradeXmDbToVersion(int i_fromVersion,
                                      const std::string &i_profile,
                                      XmDatabaseUpdateInterface *i_interface) {
  std::string v_errMsg;
  std::string v_sitekey;
  char **v_result;
  unsigned int nrow;

  if (i_fromVersion !=
      0) { /* cannot create site key if xm_parameters doesn't exist */
    v_sitekey = getXmDbSiteKey();
  }

  /* no break in this swicth ! */
  switch (i_fromVersion) {
    case 0:
      try {
        if (doesTableExists("xm_parameters") ==
            false) { // be carefull of preinit
          simpleSql("CREATE TABLE xm_parameters(param PRIMARY KEY, value);");
        }
        simpleSql("INSERT INTO xm_parameters(param, value) "
                  "VALUES(\"xmdb_version\", 1);");
        setXmParameterKey("requireUpdateAfterInit", "1");
        v_sitekey = getXmDbSiteKey();
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 0: " + e.getMsg());
      }

    case 1:
      try {
        simpleSql("CREATE TABLE stats_profiles(id_profile PRIMARY KEY, "
                  "nbStarts, since);"
                  "CREATE TABLE stats_profiles_levels("
                  "id_profile, id_level, nbPlayed, nbDied, nbCompleted, "
                  "nbRestarted, playedTime,"
                  "PRIMARY KEY(id_profile, id_level));");
        updateXmDbVersion(2, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 1: " + e.getMsg());
      }

      try {
        updateDB_stats(i_interface);
      } catch (Exception &e) {
        LogError(
          std::string("Oups, updateDB_stats() failed: " + e.getMsg()).c_str());
      }

    case 2:
      try {
        simpleSql(
          "CREATE TABLE levels(id_level PRIMARY KEY,"
          "filepath, name, checkSum, author, description, "
          "date_str, packName, packNum, music, isScripted, isToReload);");
        updateXmDbVersion(3, i_interface);
        m_requiredLevelsUpdateAfterInit = true;
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 2: " + e.getMsg());
      }

    case 3:
      try {
        simpleSql("CREATE TABLE levels_new(id_level PRIMARY KEY, isAnUpdate);");
        simpleSql("CREATE TABLE levels_favorite(id_profile, id_level, PRIMARY "
                  "KEY(id_profile, id_level));");
        try {
          updateDB_favorite(i_profile, i_interface);
        } catch (Exception &e) {
          LogError(
            std::string("Oups, updateDB_favorite() failed: " + e.getMsg())
              .c_str());
        }
        updateXmDbVersion(4, i_interface);
      } catch (Exception &e) {
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
        } catch (Exception &e) {
          LogError(
            std::string("Oups, updateDB_profiles() failed: " + e.getMsg())
              .c_str());
        }
        updateXmDbVersion(5, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 4: " + e.getMsg());
      }

    case 5:
      try {
        simpleSql("CREATE INDEX levels_packName_idx1 ON levels(packName);");
        updateXmDbVersion(6, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 5: " + e.getMsg());
      }

    case 6:
      try {
        simpleSql("CREATE table replays(id_level, name PRIMARY KEY, "
                  "id_profile, isFinished, finishTime);");
        simpleSql("CREATE INDEX replays_id_level_idx1   ON replays(id_level);");
        simpleSql(
          "CREATE INDEX replays_id_profile_idx1 ON replays(id_profile);");
        m_requiredReplaysUpdateAfterInit = true;
        updateXmDbVersion(7, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 6: " + e.getMsg());
      }

    case 7:
      try {
        simpleSql("CREATE table webhighscores(id_room, id_level, id_profile, "
                  "finishTime, date, fileUrl, PRIMARY KEY(id_room, "
                  "id_level));");
        simpleSql("CREATE INDEX webhighscores_id_profile_idx1 ON "
                  "webhighscores(id_profile);");
        simpleSql(
          "CREATE INDEX webhighscores_date_idx1       ON webhighscores(date);");
        updateXmDbVersion(8, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 7: " + e.getMsg());
      }

    case 8:
      try {
        simpleSql("CREATE table weblevels(id_level, name, fileUrl, checkSum, "
                  "difficulty, quality, creationDate, PRIMARY KEY(id_level));");
        simpleSql(
          "CREATE INDEX weblevels_difficulty_idx1 ON weblevels(difficulty);");
        simpleSql("CREATE INDEX weblevels_quality_idx1 ON weblevels(quality);");
        simpleSql("CREATE INDEX weblevels_creationDate_idx1 ON "
                  "weblevels(creationDate);");
        updateXmDbVersion(9, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 8: " + e.getMsg());
      }

    case 9:
      try {
        simpleSql(
          "CREATE table webrooms(id_room PRIMARY KEY, name, highscoresUrl);");
        simpleSql("CREATE INDEX weblevels_name_idx1 ON webrooms(name);");
        simpleSql("INSERT INTO webrooms(id_room, name, highscoresUrl) VALUES "
                  "(" DEFAULT_WEBROOM_ID ", \"" +
                  protectString(DEFAULT_WEBROOM_NAME) + "\", \"" +
                  protectString(DEFAULT_WEBHIGHSCORES_URL) + "\");");
        updateXmDbVersion(10, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 9: " + e.getMsg());
      }

    case 10:
      try {
        simpleSql(
          "CREATE table themes(id_theme PRIMARY KEY, filepath, checkSum);");
        m_requiredThemesUpdateAfterInit = true;
        updateXmDbVersion(11, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 10: " + e.getMsg());
      }

    case 11:
      try {
        simpleSql(
          "CREATE table webthemes(id_theme PRIMARY KEY, fileUrl, checkSum);");
        updateXmDbVersion(12, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 11: " + e.getMsg());
      }

    case 12:
      try {
        simpleSql("ALTER TABLE stats_profiles_levels ADD COLUMN last_play_date "
                  "DEFAULT NULL;");
        simpleSql("CREATE INDEX stats_profiles_levels_last_play_date_idx1 ON "
                  "stats_profiles_levels(last_play_date);");
        updateXmDbVersion(13, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 12: " + e.getMsg());
      }

    case 13:
      try {
        simpleSql("ALTER TABLE weblevels ADD COLUMN crappy DEFAULT 0;");
        simpleSql("CREATE INDEX weblevels_crappy_idx1 ON weblevels(crappy);");
        updateXmDbVersion(14, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 13: " + e.getMsg());
      }

    case 14:
      try {
        simpleSql("CREATE INDEX webhighscores_finishTime_idx1 ON "
                  "webhighscores(finishTime);");
        updateXmDbVersion(15, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 14: " + e.getMsg());
      }

    case 15:
      try {
        simpleSql("CREATE TABLE levels_blacklist(id_profile, id_level, PRIMARY "
                  "KEY(id_profile, id_level));");
        updateXmDbVersion(16, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 15: " + e.getMsg());
      }

    case 16:
      try {
        simpleSql("CREATE INDEX webhighscores_id_level_idx1 ON "
                  "webhighscores(id_level);");
        updateXmDbVersion(17, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 16: " + e.getMsg());
      }

    case 17:
      try {
        // fix stats_profiles_levels due to old old old xmoto versions
        fixStatsProfilesLevelsNbCompleted();
        updateXmDbVersion(18, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 17: " + e.getMsg());
      }

    case 18: // from float to int storage for time
      try {
        m_requiredReplaysUpdateAfterInit = true;
        simpleSql("DELETE from webhighscores;");
        simpleSql(
          "UPDATE profile_completedLevels SET finishTime=finishTime*100;");
        simpleSql(
          "UPDATE stats_profiles_levels   SET playedTime=playedTime*100;");
        updateXmDbVersion(19, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 18: " + e.getMsg());
      }

    case 19:
      try {
        simpleSql(
          "ALTER TABLE weblevels ADD COLUMN children_compliant DEFAULT 1;");
        simpleSql("CREATE INDEX weblevels_children_compliant_idx1 ON "
                  "weblevels(children_compliant);");
        updateXmDbVersion(20, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 19: " + e.getMsg());
      }

    case 20:
      try {
        if (doesTableExists("profiles_configs") ==
            false) { // be carefull of preinit
          simpleSql("CREATE TABLE profiles_configs(id_profile, key, value, "
                    "PRIMARY KEY(id_profile, key));");
        }
        updateXmDbVersion(21, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 20: " + e.getMsg());
      }

    case 21:
      try {
        try {
          updateDB_config("" /* site key still does exist */);
        } catch (Exception &e) {
          // ok, no upgrade
        }
        updateXmDbVersion(22, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 21: " + e.getMsg());
      }

    case 22:
      try {
        // add column sitekey as primary key - sqlite doesn't support that
        // without recreating the table
        simpleSql("ALTER  TABLE stats_profiles RENAME TO stats_profiles_old;");
        simpleSql("CREATE TABLE stats_profiles(sitekey, id_profile, nbStarts, "
                  "since, PRIMARY KEY(sitekey, id_profile));");
        simpleSql("INSERT INTO  stats_profiles SELECT \"" +
                  protectString(v_sitekey) +
                  "\", id_profile, nbStarts, since FROM stats_profiles_old;");
        simpleSql("DROP   TABLE stats_profiles_old;");

        updateXmDbVersion(23, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 22: " + e.getMsg());
      }

    case 23:
      try {
        // add column sitekey as primary key - sqlite doesn't support that
        // without recreating the table
        simpleSql("ALTER  TABLE stats_profiles_levels RENAME TO "
                  "stats_profiles_levels_old;");
        simpleSql("CREATE TABLE stats_profiles_levels(sitekey, id_profile, "
                  "id_level, nbPlayed, nbDied, nbCompleted, nbRestarted, "
                  "playedTime, last_play_date DEFAULT NULL, synchronized, "
                  "PRIMARY KEY(sitekey, id_profile, id_level));");
        simpleSql("INSERT INTO  stats_profiles_levels SELECT \"" +
                  protectString(v_sitekey) +
                  "\", id_profile, id_level, nbPlayed, nbDied, nbCompleted, "
                  "nbRestarted, playedTime, last_play_date, 0 FROM "
                  "stats_profiles_levels_old;");
        simpleSql("DROP   TABLE stats_profiles_levels_old;");
        /* recreate the index */
        simpleSql("CREATE INDEX stats_profiles_levels_last_play_date_idx1 ON "
                  "stats_profiles_levels(last_play_date);");
        // index not redondante with the key while it is choiced when sitekey is
        // not set ; make xmoto really faster
        simpleSql("CREATE INDEX stats_profiles_levels_id_profile_id_level_idx1 "
                  "ON stats_profiles_levels(id_profile, id_level);");

        updateXmDbVersion(24, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 23: " + e.getMsg());
      }

    case 24:
      try {
        simpleSql("ALTER TABLE profile_completedLevels ADD COLUMN sitekey;");
        simpleSql(
          "ALTER TABLE profile_completedLevels ADD COLUMN synchronized;");
        simpleSql("UPDATE profile_completedLevels SET sitekey = \"" +
                  protectString(v_sitekey) + "\";");
        simpleSql("UPDATE profile_completedLevels SET synchronized = 0;");
        simpleSql("DROP INDEX profile_completedLevels_idx1;");
        /* recreate the index */
        simpleSql("CREATE INDEX profile_completedLevels_idx1 "
                  "ON profile_completedLevels(sitekey, id_profile, id_level);");
        // index not redondante with the idx1 while it is choiced when sitekey
        // is not set ; make xmoto really faster
        simpleSql("CREATE INDEX profile_completedLevels_idx2 "
                  "ON profile_completedLevels(id_profile, id_level);");

        updateXmDbVersion(25, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 24: " + e.getMsg());
      }

    case 25:
      try {
        simpleSql("ALTER TABLE levels ADD COLUMN isPhysics DEFAULT 0;");
        simpleSql("CREATE INDEX levels_isPhysics_idx1 ON levels(isPhysics);");
        updateXmDbVersion(26, i_interface);
        m_requiredLevelsUpdateAfterInit = true;
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 25: " + e.getMsg());
      }

    case 26:
      try {
        simpleSql("ALTER TABLE profile_completedLevels ADD COLUMN dbSync "
                  "DEFAULT NULL;");
        simpleSql("CREATE INDEX profile_completedLevels_dbSync_idx1 ON "
                  "profile_completedLevels(dbSync);");
        simpleSql(
          "ALTER TABLE stats_profiles_levels ADD COLUMN dbSync DEFAULT NULL;");
        simpleSql("CREATE INDEX stats_profiles_levels_dbSync_idx1 ON "
                  "stats_profiles_levels(dbSync);");
        updateXmDbVersion(27, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 26: " + e.getMsg());
      }

    case 27:
      try {
        simpleSql("CREATE TABLE profiles_votes(id_profile, id_level, PRIMARY "
                  "KEY(id_profile, id_level));");
        updateXmDbVersion(28, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 27: " + e.getMsg());
      }

    case 28:
      try {
        simpleSql("ALTER TABLE levels ADD COLUMN loadingCacheFormatVersion "
                  "DEFAULT NULL;");
        simpleSql("ALTER TABLE levels ADD COLUMN loaded DEFAULT NULL;");
        simpleSql("CREATE INDEX levels_checkSum_idx1 ON levels(checkSum);");
        updateXmDbVersion(29, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 28: " + e.getMsg());
      }

    case 29:
      try {
        simpleSql("CREATE TABLE levels_mywebhighscores(id_profile, id_room, "
                  "id_level, PRIMARY KEY(id_profile, id_room, id_level));");
        updateXmDbVersion(30, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 29: " + e.getMsg());
      }

    case 30:
      try {
        simpleSql("ALTER TABLE levels_mywebhighscores ADD COLUMN known_stolen "
                  "DEFAULT 0;");
        simpleSql(
          "ALTER TABLE levels_mywebhighscores ADD COLUMN known_stolen_date;");
        simpleSql("CREATE INDEX levels_mywebhighscores_idx1 ON "
                  "levels_mywebhighscores(known_stolen);");
        simpleSql("CREATE INDEX levels_mywebhighscores_idx2 ON "
                  "levels_mywebhighscores(known_stolen_date);");
        updateXmDbVersion(31, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 30: " + e.getMsg());
      }

    case 31:
      try {
        simpleSql("CREATE TABLE srv_admins(id INTEGER PRIMARY KEY "
                  "AUTOINCREMENT, id_profile UNIQUE, password);");
        updateXmDbVersion(32, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 31: " + e.getMsg());
      }

    case 32:
      try {
        simpleSql("CREATE TABLE srv_bans(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "id_profile, ip, from_date, nb_days);");
        updateXmDbVersion(33, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 32: " + e.getMsg());
      }

    case 33:
      try {
        simpleSql("ALTER TABLE weblevels ADD COLUMN vote_locked DEFAULT 0;");
        simpleSql(
          "CREATE INDEX weblevels_vote_locked_idx1 ON weblevels(vote_locked);");
        updateXmDbVersion(34, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 33: " + e.getMsg());
      }

    case 34:
      try {
        simpleSql("ALTER TABLE weblevels ADD COLUMN packname;");
        simpleSql("ALTER TABLE weblevels ADD COLUMN packnum;");
        simpleSql(
          "CREATE INDEX weblevels_packname_idx1 ON weblevels(packname);");
        simpleSql("DROP INDEX levels_packName_idx1;");

        // do not use level.packname anymore
        // update weblevels table from levels for the first time

        try {
          std::string v_id_level, v_packname, v_packnum;

          simpleSql("BEGIN TRANSACTION;");

          v_result = readDB(std::string("SELECT id_level, packname, packnum "
                                        "FROM levels WHERE packname <> '';"),
                            nrow);
          for (unsigned int i = 0; i < nrow; i++) {
            v_id_level = getResult(v_result, 3, i, 0);
            v_packname = getResult(v_result, 3, i, 1);
            v_packnum = getResult(v_result, 3, i, 2);

            simpleSql("UPDATE weblevels SET packname=\"" +
                      xmDatabase::protectString(v_packname) + "\", packnum=\"" +
                      xmDatabase::protectString(v_packnum) +
                      "\" WHERE id_level=\"" +
                      xmDatabase::protectString(v_id_level) + "\";");
          }
          read_DB_free(v_result);

          // remove information for the table levels
          simpleSql("UPDATE levels SET packname='', packnum='';");

          simpleSql("COMMIT;");

        } catch (Exception &e) {
          /* ok, the player will have to update weblevels via internet */
          simpleSql("COMMIT;"); // anyway, commit what can be commited
        }

        updateXmDbVersion(35, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 34: " + e.getMsg());
      }

    case 35:
      try {
        simpleSql("ALTER TABLE srv_bans ADD COLUMN id_admin_banner;");
        updateXmDbVersion(36, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 35: " + e.getMsg());
      }

    case 36:
      try {
        auto config = GameApp::instance()->getUserConfig();

        Input::instance()->loadConfig(config, this, i_profile);
        InputSDL12Compat::remap();
        Input::instance()->saveConfig(config, this, i_profile);

        updateXmDbVersion(37, i_interface);

        Logger::deleteLegacyLog();
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 36: " + e.getMsg());
      }

    case 37:
      try {
        { // Update user config
          UserConfig defaultConfig;
          XMSession::createDefaultConfig(&defaultConfig);
          std::vector<std::string> varsToReset = {
            "WebLevelsURL",     "WebDbSyncUploadURL",    "WebThemesURL",
            "WebThemesURLBase", "WebHighscoreUploadURL",
          };

          auto userConfig = GameApp::instance()->getUserConfig();
          for (auto &var : varsToReset) {
            userConfig->setString(var, defaultConfig.getString(var));
          }

          XMSession::instance("file")->loadConfig(userConfig, false);
          userConfig->saveFile();
        }

        try { // Update db
          simpleSql("BEGIN TRANSACTION;");

          using StringPair = std::pair<std::string, std::string>;

          std::vector<StringPair> fields = {
            { "webrooms", "highscoresUrl" },
            { "webthemes", "fileUrl" },
            { "weblevels", "fileUrl" },
            { "webhighscores", "fileUrl" },
          };

          std::vector<StringPair> domainRenames = {
            { LEGACY_WEBSITE_DOMAIN, WEBSITE_DOMAIN },
            { LEGACY_DOWNLOAD_DOMAIN, DOWNLOAD_DOMAIN },
          };

          for (auto &pair : fields) {
            auto &table = pair.first;
            auto &field = pair.second;

            for (auto &renames : domainRenames) {
              simpleSql("UPDATE " + table + " SET " + field + " = replace(" +
                        field + ", '" + renames.first + "', '" +
                        renames.second + "');");
            }
          }

          simpleSql("UPDATE profiles_configs SET value = replace(value, "
                    "'" LEGACY_GAMES_DOMAIN "', '" GAMES_DOMAIN
                    "') WHERE key = 'ClientServerName'");

          simpleSql("COMMIT;");
        } catch (Exception &e) {
          simpleSql("ROLLBACK;");
          throw e;
        }

        auto profile = XMSession::instance()->profile();
        XMSession::instance()->loadProfile(profile, this);
        XMSession::instance("file")->loadProfile(profile, this);

        updateXmDbVersion(38, i_interface);
      } catch (Exception &e) {
        throw Exception("Unable to update xmDb from 37: " + e.getMsg());
      }

      // next
  }
}

bool xmDatabase::checkKey(const std::string &i_sql) {
  char **v_result;
  int nrow, ncolumn;
  char *errMsg;
  std::string v_errMsg;
  int v_nb;

  if (sqlite3_get_table(
        m_db, i_sql.c_str(), &v_result, &nrow, &ncolumn, &errMsg) !=
      SQLITE_OK) {
    v_errMsg = errMsg;
    sqlite3_free(errMsg);
    throw Exception("xmDb: " + v_errMsg);
  }

  v_nb = atoi(v_result[1]);
  sqlite3_free_table(v_result);

  return v_nb != 0;
}

void xmDatabase::simpleSql(const std::string &i_sql) {
  char *errMsg;
  std::string v_errMsg;

  // LogDebug("simpleSql(%X): %s", this, i_sql.c_str());

  if (sqlite3_exec(m_db, i_sql.c_str(), NULL, NULL, &errMsg) != SQLITE_OK) {
    v_errMsg = errMsg;
    sqlite3_free(errMsg);
    LogInfo(std::string("simpleSql failed on running : " + i_sql).c_str());
    throw Exception(v_errMsg);
  }
}

void xmDatabase::debugResult(char **i_result, int ncolumn, unsigned int nrow) {
  for (unsigned int i = 0; i < ncolumn * (nrow + 1); i++) {
    printf("result[%i] = %s\n", i, i_result[i]);
  }
}

char *xmDatabase::getResult(char **i_result,
                            int ncolumn,
                            unsigned int i_row,
                            int i_column) {
  return i_result[ncolumn * (i_row + 1) + i_column];
}

char **xmDatabase::readDB(const std::string &i_sql, unsigned int &i_nrow) {
  char **v_result;
  int ncolumn;
  char *errMsg;
  std::string v_errMsg;
  int v_nrow;
  double v_startTime, v_endTime;

  // LogDebug("readDB(%X): %s", this, i_sql.c_str());

  v_startTime = GameApp::getXMTime();
  if (sqlite3_get_table(
        m_db, i_sql.c_str(), &v_result, &v_nrow, &ncolumn, &errMsg) !=
      SQLITE_OK) {
    v_errMsg = errMsg;
    sqlite3_free(errMsg);
    LogError("xmDb failed while running :");
    LogInfo("%s", i_sql.c_str());
    LogError("%s", v_errMsg.c_str()) throw Exception("xmDb: " + v_errMsg);
  }
  v_endTime = GameApp::getXMTime();

  if (v_endTime - v_startTime > DB_MAX_SQL_RUNTIME) {
    LogWarning("long query time detected (%.3f'') for query '%s'",
               v_endTime - v_startTime,
               i_sql.c_str());
  }

  i_nrow = (unsigned int)v_nrow;

  return v_result;
}

void xmDatabase::read_DB_free(char **i_result) {
  sqlite3_free_table(i_result);
}

std::string xmDatabase::protectString(const std::string &i_str) {
  std::string v_res;

  for (unsigned int i = 0; i < i_str.length(); i++) {
    switch (i_str[i]) {
      case '"':
        v_res.append("\"\"");
        break;
      default:
        char c[2] = { i_str[i], '\0' };
        v_res.append(c);
    }
  }
  return v_res;
}

void xmDatabase::createUserFunctions() {
  if (sqlite3_create_function(
        m_db, "xm_floord", 1, SQLITE_ANY, NULL, user_xm_floord, NULL, NULL) !=
      SQLITE_OK) {
    throw Exception("xmDatabase::createUserFunctions() failed !");
  }

  if (sqlite3_create_function(m_db,
                              "xm_lvlUpdatedToTxt",
                              1,
                              SQLITE_ANY,
                              NULL,
                              user_xm_lvlUpdatedToTxt,
                              NULL,
                              NULL) != SQLITE_OK) {
    throw Exception("xmDatabase::createUserFunctions() failed !");
  }

  if (sqlite3_create_function(m_db,
                              "xm_userCrappy",
                              1,
                              SQLITE_ANY,
                              NULL,
                              user_xm_userCrappy,
                              NULL,
                              NULL) != SQLITE_OK) {
    throw Exception("xmDatabase::createUserFunctions() failed !");
  }

  if (sqlite3_create_function(m_db,
                              "xm_userChildrenCompliant",
                              1,
                              SQLITE_ANY,
                              NULL,
                              user_xm_userChildrenCompliant,
                              NULL,
                              NULL) != SQLITE_OK) {
    throw Exception("xmDatabase::createUserFunctions() failed !");
  }

  if (sqlite3_create_function(m_db,
                              "xm_replaceStart",
                              3,
                              SQLITE_ANY,
                              NULL,
                              user_xm_replaceStart,
                              NULL,
                              NULL) != SQLITE_OK) {
    throw Exception("xmDatabase::createUserFunctions() failed !");
  }

  if (sqlite3_create_function(
        m_db, "xm_profile", 0, SQLITE_ANY, NULL, user_xm_profile, NULL, NULL) !=
      SQLITE_OK) {
    throw Exception("xmDatabase::createUserFunctions() failed !");
  }

  if (sqlite3_create_function(
        m_db, "xm_idRoom", 1, SQLITE_ANY, NULL, user_xm_idRoom, NULL, NULL) !=
      SQLITE_OK) {
    throw Exception("xmDatabase::createUserFunctions() failed !");
  }
}

void xmDatabase::user_xm_floord(sqlite3_context *i_context,
                                int i_nArgs,
                                sqlite3_value **i_values) {
  double v_value;

  if (i_nArgs != 1) {
    throw Exception("user_xm_floord failed !");
  }

  v_value = sqlite3_value_double(i_values[0]);
  sqlite3_result_double(i_context, (double)((int)(v_value)));
}

void xmDatabase::user_xm_lvlUpdatedToTxt(sqlite3_context *i_context,
                                         int i_nArgs,
                                         sqlite3_value **i_values) {
  int v_value;

  if (i_nArgs != 1) {
    throw Exception("user_xm_lvlUpdatedToTxt failed !");
  }

  v_value = sqlite3_value_int(i_values[0]);
  sqlite3_result_text(i_context,
                      v_value == 0 ? GAMETEXT_NEW : GAMETEXT_UPDATED,
                      -1,
                      SQLITE_TRANSIENT);
}

void xmDatabase::user_xm_userCrappy(sqlite3_context *i_context,
                                    int i_nArgs,
                                    sqlite3_value **i_values) {
  int v_value;

  if (i_nArgs != 1) {
    throw Exception("user_xm_userCrappy failed !");
  }

  v_value = sqlite3_value_int(i_values[0]);
  sqlite3_result_int(i_context,
                     XMSession::instance()->useCrappyPack() ? v_value : 0);
}

void xmDatabase::user_xm_userChildrenCompliant(sqlite3_context *i_context,
                                               int i_nArgs,
                                               sqlite3_value **i_values) {
  int v_value;

  if (i_nArgs != 1) {
    throw Exception("user_xm_userChildrenCompliant failed !");
  }

  v_value = sqlite3_value_int(i_values[0]);
  sqlite3_result_int(
    i_context, XMSession::instance()->useChildrenCompliant() ? v_value : 1);
}

void xmDatabase::user_xm_replaceStart(sqlite3_context *i_context,
                                      int i_nArgs,
                                      sqlite3_value **i_values) {
  std::string v_value, v_old, v_new, v_result, v_begin, v_end;

  if (i_nArgs != 3) {
    throw Exception("user_xm_replaceStart failed !");
  }

  v_value = (char *)sqlite3_value_text(i_values[0]);
  v_old = (char *)sqlite3_value_text(i_values[1]);
  v_new = (char *)sqlite3_value_text(i_values[2]);

  v_begin = v_value.substr(0, v_old.length());

  if (v_begin != v_old) {
    /* change nothing */
    sqlite3_result_text(i_context, v_value.c_str(), -1, SQLITE_TRANSIENT);
    return;
  }

  v_end = v_value.substr(v_old.length(), v_value.length() - v_old.length());
  v_result = v_new + v_end;

  sqlite3_result_text(i_context, v_result.c_str(), -1, SQLITE_TRANSIENT);
}

void xmDatabase::user_xm_profile(sqlite3_context *i_context,
                                 int i_nArgs,
                                 sqlite3_value **i_values) {
  if (i_nArgs != 0) {
    throw Exception("user_xm_profile failed !");
  }

  sqlite3_result_text(i_context,
                      XMSession::instance()->profile().c_str(),
                      XMSession::instance()->profile().size(),
                      SQLITE_TRANSIENT);
}

void xmDatabase::user_xm_idRoom(sqlite3_context *i_context,
                                int i_nArgs,
                                sqlite3_value **i_values) {
  int v_value;

  if (i_nArgs != 1) {
    throw Exception("user_xm_idRoom failed !");
  }

  v_value = sqlite3_value_int(i_values[0]);

  sqlite3_result_int(i_context,
                     atoi(XMSession::instance()->idRoom(v_value).c_str()));
}
