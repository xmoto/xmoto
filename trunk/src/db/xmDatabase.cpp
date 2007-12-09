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
#include "helpers/Log.h"
#include "XMSession.h"

#define XMDB_VERSION 16

bool xmDatabase::Trace = false;

xmDatabase::xmDatabase(const std::string& i_dbFile,
		       const std::string& i_profile,
		       const std::string& i_gameDir,
		       const std::string& i_userDir,
		       const std::string& i_binPackCheckSum,
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
  if(i_gameDir != getXmDbGameDir() || i_userDir != getXmDbUserDir()
     || i_binPackCheckSum != getXmDbBinPackCheckSum()) {
    m_requiredLevelsUpdateAfterInit  = true;
    m_requiredReplaysUpdateAfterInit = true;
    m_requiredThemesUpdateAfterInit  = true;
    setXmDbGameDir(i_gameDir);
    setXmDbUserDir(i_userDir);
    setXmDbBinPackCheckSum(i_binPackCheckSum);
  }
}

xmDatabase::xmDatabase(const std::string& i_dbFile)
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

std::string xmDatabase::getXmDbGameDir() {
  char **v_result;
  unsigned int nrow;
  std::string v_dir;

  v_result = readDB("SELECT value FROM xm_parameters WHERE param=\"gameDir\";", nrow);
  if(nrow != 1) {
    read_DB_free(v_result);
    return "";
  }

  v_dir = getResult(v_result, 1, 0, 0);
  read_DB_free(v_result);

  return v_dir;
}

std::string xmDatabase::getXmDbUserDir() {
  char **v_result;
  unsigned int nrow;
  std::string v_dir;

  v_result = readDB("SELECT value FROM xm_parameters WHERE param=\"userDir\";", nrow);
  if(nrow != 1) {
    read_DB_free(v_result);
    return "";
  }

  v_dir = getResult(v_result, 1, 0, 0);
  read_DB_free(v_result);

  return v_dir;
}

void xmDatabase::setXmDbGameDir(const std::string& i_gameDir) {
  char **v_result;
  unsigned int nrow;

  v_result = readDB("SELECT value FROM xm_parameters WHERE param=\"gameDir\";", nrow);
  read_DB_free(v_result);

  if(nrow == 0) {
    simpleSql("INSERT INTO xm_parameters(param, value) VALUES(\"gameDir\", \""
	      + protectString(i_gameDir) + "\");");
  } else {
    simpleSql("UPDATE xm_parameters SET value=\""
	      + protectString(i_gameDir) + "\" WHERE param=\"gameDir\"");
  }
}

void xmDatabase::setXmDbUserDir(const std::string& i_userDir) {
  char **v_result;
  unsigned int nrow;

  v_result = readDB("SELECT value FROM xm_parameters WHERE param=\"userDir\";", nrow);
  read_DB_free(v_result);

  if(nrow == 0) {
    simpleSql("INSERT INTO xm_parameters(param, value) VALUES(\"userDir\", \""
	      + protectString(i_userDir) + "\");");
  } else {
    simpleSql("UPDATE xm_parameters SET value=\""
	      + protectString(i_userDir) + "\" WHERE param=\"userDir\"");
  }
}

std::string xmDatabase::getXmDbBinPackCheckSum() {
  char **v_result;
  unsigned int nrow;
  std::string v_dir;

  v_result = readDB("SELECT value FROM xm_parameters WHERE param=\"binPackCkSum\";", nrow);
  if(nrow != 1) {
    read_DB_free(v_result);
    return "";
  }

  v_dir = getResult(v_result, 1, 0, 0);
  read_DB_free(v_result);

  return v_dir;
}

void xmDatabase::setXmDbBinPackCheckSum(const std::string& i_binPackChecksum) {
  char **v_result;
  unsigned int nrow;

  v_result = readDB("SELECT value FROM xm_parameters WHERE param=\"binPackCkSum\";", nrow);
  read_DB_free(v_result);

  if(nrow == 0) {
    simpleSql("INSERT INTO xm_parameters(param, value) VALUES(\"binPackCkSum\", \""
	      + protectString(i_binPackChecksum) + "\");");
  } else {
    simpleSql("UPDATE xm_parameters SET value=\""
	      + protectString(i_binPackChecksum) + "\" WHERE param=\"binPackCkSum\"");
  }
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
  char *errMsg;
  std::string v_errMsg;

  v_newVersion << i_newVersion;

  simpleSql("UPDATE xm_parameters SET value="+ v_newVersion.str() +
	    " WHERE param=\"xmdb_version\";");
}

void xmDatabase::upgradeXmDbToVersion(int i_fromVersion,
				      const std::string& i_profile,
				      XmDatabaseUpdateInterface *i_interface) {
  char *errMsg;
  std::string v_errMsg;

  /* no break in this swicth ! */
  switch(i_fromVersion) {

  case 0:
    try {
      simpleSql("CREATE TABLE xm_parameters(param PRIMARY KEY, value);"
		"INSERT INTO xm_parameters(param, value) VALUES(\"xmdb_version\", 1);");
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

  //Logger::Log(i_sql.c_str());
  //printf("%s\n", i_sql.c_str());

  if(sqlite3_exec(m_db,
		  i_sql.c_str(),
		  NULL,
		  NULL, &errMsg) != SQLITE_OK) {
    v_errMsg = errMsg;
    sqlite3_free(errMsg);
    throw Exception(v_errMsg);
  }
}

void xmDatabase::debugResult(char **i_result, int ncolumn, unsigned int nrow) {
  for(int i=0; i<ncolumn*(nrow+1); i++) {
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

  //printf("%s\n", i_sql.c_str());

  if(sqlite3_get_table(m_db,
		       i_sql.c_str(),
		       &v_result, &v_nrow, &ncolumn, &errMsg)
     != SQLITE_OK) {
    v_errMsg = errMsg;
    sqlite3_free(errMsg);
    throw Exception("xmDb: " + v_errMsg);
  }
  i_nrow = (unsigned int) v_nrow;

  return v_result;
}

void xmDatabase::read_DB_free(char **i_result) {
  sqlite3_free_table(i_result);
}

std::string xmDatabase::protectString(const std::string& i_str) {
  std::string v_res;

  for(int i=0; i<i_str.length(); i++) {
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
