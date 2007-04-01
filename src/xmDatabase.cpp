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
#include "VApp.h"
#include "GameText.h"

#define XMDB_VERSION 2

xmDatabase::xmDatabase(std::string i_dbFile, XmDatabaseUpdateInterface *i_interface) {
  int v_version;

  if(sqlite3_open(i_dbFile.c_str(), &m_db) != 0) {
    throw Exception("Unable to open the database (" + i_dbFile
		    + ") : " + sqlite3_errmsg(m_db));
  }

  sqlite3_trace(m_db, sqlTrace, NULL);

  v_version = getXmDbVersion();
  vapp::Log("XmDb version is %i", v_version);

  if(v_version > XMDB_VERSION) {
    throw Exception("Your XM database required a newer version of xmoto");
  }

  if(v_version < XMDB_VERSION) {
    vapp::Log("Update XmDb version from %i to %i", v_version, XMDB_VERSION);

    if(i_interface != NULL) {
      i_interface->updatingDatabase(GAMETEXT_DB_UPGRADING);
    }
    upgrateXmDbToVersion(v_version, i_interface); 
  }
}
 
xmDatabase::~xmDatabase() {
  sqlite3_close(m_db);
}

void xmDatabase::sqlTrace(void* arg1, const char* sql) {
  //printf("%s\n", sql);
}

int xmDatabase::getXmDbVersion() {
  char **v_result;
  int nrow, ncolumn;
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

void xmDatabase::upgrateXmDbToVersion(int i_fromVersion, XmDatabaseUpdateInterface *i_interface) {
  char *errMsg;
  std::string v_errMsg;

  /* no break in this swicth ! */
  switch(i_fromVersion) {

  case 0:
    if(sqlite3_exec(m_db,
		    "CREATE TABLE xm_parameters(param PRIMARY KEY, value);"
		    "INSERT INTO xm_parameters(param, value) VALUES(\"xmdb_version\", 1);"
		    , NULL,
		    NULL, &errMsg) != SQLITE_OK) {
      v_errMsg = errMsg;
      sqlite3_free(errMsg);
      throw Exception("Unable to update xmDb from 0: " + v_errMsg);
    }

  case 1:
    if(sqlite3_exec(m_db,
		    "CREATE TABLE stats_profiles(id_profile PRIMARY KEY, nbStarts, since);"
		    "CREATE TABLE stats_profiles_levels("
		    "id_profile, id_level, nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime,"
		    "PRIMARY KEY(id_profile, id_level));"
		    , NULL,
		    NULL, &errMsg) != SQLITE_OK) {
      v_errMsg = errMsg;
      sqlite3_free(errMsg);
      throw Exception("Unable to update xmDb from 1: " + v_errMsg);
    }
    try {
      updateDB_stats(i_interface);
    } catch(Exception &e) {
      vapp::Log(std::string("Oups, updateDB_stats() failed: " + e.getMsg()).c_str());
    }
    updateXmDbVersion(2);

  }
}

bool xmDatabase::checkKey(std::string i_sql) {
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

void xmDatabase::simpleSql(std::string i_sql) {
  char *errMsg;
  std::string v_errMsg;

  if(sqlite3_exec(m_db,
		  i_sql.c_str(),
		  NULL,
		  NULL, &errMsg) != SQLITE_OK) {
    v_errMsg = errMsg;
    sqlite3_free(errMsg);
    throw Exception(v_errMsg);
  }
}

void xmDatabase::debugResult(char **i_result, int ncolumn, int nrow) {
  for(int i=0; i<ncolumn*(nrow+1); i++) {
    printf("result[%i] = %s\n", i, i_result[i]);
  }
}
