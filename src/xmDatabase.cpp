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

#define XMDB_VERSION 1

xmDatabase::xmDatabase(std::string i_dbFile) {
  int v_version;

  if(sqlite3_open(i_dbFile.c_str(), &m_db) != 0) {
    throw Exception("Unable to open the database (" + i_dbFile
		    + ") : " + sqlite3_errmsg(m_db));
  }

  v_version = getXmDbVersion();
  vapp::Log("XmDb version is %i", v_version);

  if(v_version > XMDB_VERSION) {
    throw Exception("Your XM database required a newer version of xmoto");
  }

  if(v_version < XMDB_VERSION) {
    vapp::Log("Update XmDb version from %i to %i", v_version, XMDB_VERSION);
    upgrateXmDbToVersion(v_version); 
  }
}
 
xmDatabase::~xmDatabase() {
  sqlite3_close(m_db);
}

int xmDatabase::getXmDbVersion() {
  char **v_result;
  int nrow, ncolumn;
  char *errMsg;
  std::string v_errMsg;
  int v_version;

  /* check if table xm_parameters exists */
  if(sqlite3_get_table(m_db,
		       "SELECT count(1) FROM sqlite_master WHERE type='table' AND name='xm_parameters';",
		       &v_result, &nrow, &ncolumn, &errMsg)
     != SQLITE_OK) {
    v_errMsg = errMsg;
    sqlite3_free(errMsg);
    throw Exception("Unable to list tables in xmDb: " + v_errMsg);
  }

  if(atoi(v_result[1]) == 0) { /* case for no creation */
    sqlite3_free_table(v_result);
    return 0;
  }
  sqlite3_free_table(v_result);

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

void xmDatabase::upgrateXmDbToVersion(int i_fromVersion) {
  char *errMsg;
  std::string v_errMsg;

  /* no break in this swicth ! */
  switch(i_fromVersion) {
  case 0:
    if(sqlite3_exec(m_db,
		    "CREATE TABLE xm_parameters(param PRIMARY KEY, value);"
		    "INSERT INTO xm_parameters VALUES(\"xmdb_version\", 1);"
		    , NULL,
		    NULL, &errMsg) != SQLITE_OK) {
      v_errMsg = errMsg;
      sqlite3_free(errMsg);
      throw Exception("Unable to update xmDb from 0: " + v_errMsg);
    }
   
  }
}
