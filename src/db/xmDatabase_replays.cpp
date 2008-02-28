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
#include <sstream>
#include <math.h>

bool xmDatabase::replays_isIndexUptodate() const {
  return m_requiredReplaysUpdateAfterInit == false;
}

void xmDatabase::replays_add_begin() {
  simpleSql("BEGIN TRANSACTION;");
  simpleSql("DELETE FROM replays;");
}

void xmDatabase::replays_add(const std::string& i_id_level,
			     const std::string& i_name,
			     const std::string& i_id_profile,
			     bool               i_isFinished,
			     int                i_finishTime) {
  std::ostringstream v_finishTime;

  v_finishTime << i_finishTime;

  simpleSql("INSERT INTO replays(id_level, name, id_profile, isFinished, finishTime) " 
	    "VALUES (\"" +
	    protectString(i_id_level)   + "\", \"" +
	    protectString(i_name)       + "\", \"" +
	    protectString(i_id_profile) + "\", "   +
	    std::string(i_isFinished ? "1":"0") + ", " +
	    v_finishTime.str()          + ");");
}

void xmDatabase::replays_add_end() {
  simpleSql("COMMIT;");
}

void xmDatabase::replays_delete(const std::string& i_replay) {
  simpleSql("DELETE FROM replays WHERE name=\""+ protectString(i_replay) + "\";");
}

bool xmDatabase::replays_exists(const std::string& i_name) {
  char **v_result;
  unsigned int nrow;

  v_result = readDB("SELECT name FROM replays WHERE name=\"" + protectString(i_name) + "\";",
		    nrow);
  read_DB_free(v_result);
  return nrow == 1;
}
