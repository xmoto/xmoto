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
#include "../GameText.h"
#include <sstream>
#include <math.h>
#include "../md5sum/md5file.h"
#include "../helpers/VExcept.h"
#include "../helpers/Log.h"


bool xmDatabase::bikes_isIndexUptodate() const {
  return m_requiredBikesUpdateAfterInit == false;
}

void xmDatabase::bikes_add_begin() {
  simpleSql("BEGIN TRANSACTION;");
  simpleSql("DELETE FROM bikes;");
}

void xmDatabase::bikes_add(const std::string& i_id_theme,
			   const std::string& i_filepath,
			   const std::string& i_theme,
			   const std::string& i_physics) {
  simpleSql("INSERT INTO bikes(id_bike, filepath, checkSum, theme, physics) " 
	    "VALUES (\"" +
	    protectString(i_id_theme)   + "\", \"" +
	    protectString(i_filepath)   + "\", \"" +
	    protectString(md5file(i_filepath)) + "\", \"" +
	    protectString(i_theme)  + "\", \"" +
	    protectString(i_physics) + "\");");
}

void xmDatabase::bikes_update(const std::string& i_id_theme,
			      const std::string& i_filepath,
			      const std::string& i_theme,
			      const std::string& i_physics) {
  simpleSql("UPDATE bikes "
	    "SET filepath=\"" + protectString(i_filepath) +
	    "\", checkSum=\"" + protectString(md5file(i_filepath)) + 
	    "\", theme=\""    + protectString(i_theme) +
	    "\", physics=\""  + protectString(i_physics) + "\" "
	    "WHERE id_bike=\"" + protectString(i_id_theme) + "\";");
}

void xmDatabase::bikes_add_end() {
  simpleSql("COMMIT;");
}

void xmDatabase::bikes_delete(const std::string& i_id_theme) {
  simpleSql("DELETE FROM bikes WHERE id_bike=\""+ protectString(i_id_theme) + "\";");
}

bool xmDatabase::bikes_exists(const std::string& i_id_theme) {
  char **v_result;
  unsigned int nrow;

  v_result = readDB("SELECT id_bike FROM bikes WHERE id_bike=\"" + protectString(i_id_theme) + "\";",
		    nrow);
  read_DB_free(v_result);
  return nrow == 1;
}

std::string xmDatabase::bikes_getTheme(const std::string& i_id_bike) {
  char **v_result;
  unsigned int nrow;
  std::string v_theme, v_customTheme;

  v_result = readDB("SELECT theme, customTheme FROM bikes WHERE id_bike=\"" + protectString(i_id_bike) + "\";",
		    nrow);
  if(nrow != 1) {
    read_DB_free(v_result);
    throw Exception("Couldn't get Theme from Bike, because Bike not found");
  }

  v_theme = getResult(v_result, 2, 0, 0);
  if(getResult(v_result, 2, 0, 1) != NULL) {
    v_customTheme = getResult(v_result, 2, 0, 1);
  };
  
  read_DB_free(v_result);
  
  LogInfo("%s %s", v_theme.c_str(), v_customTheme.c_str());
  if(v_customTheme == "") {
    return v_theme;
  }
  else {
    return v_customTheme;
  }
}

std::string xmDatabase::bikes_getPhysics(const std::string& i_id_bike) {
  char **v_result;
  unsigned int nrow;
  std::string v_res;

  v_result = readDB("SELECT physics FROM bikes WHERE id_bike=\"" + protectString(i_id_bike) + "\";",
		    nrow);
  if(nrow != 1) {
    read_DB_free(v_result);
    throw Exception("Bike not found, couldn't get Physics");
  }

  v_res = getResult(v_result, 1, 0, 0);
  read_DB_free(v_result);

  return v_res;
}

std::string xmDatabase::bikes_getFileName(const std::string& i_id_bike) {
  char **v_result;
  unsigned int nrow;
  std::string v_res;

  v_result = readDB("SELECT filepath FROM bikes WHERE id_bike=\"" + protectString(i_id_bike) + "\";",
		    nrow);
  if(nrow != 1) {
    read_DB_free(v_result);
    throw Exception("Bike not found");
  }

  v_res = getResult(v_result, 1, 0, 0);
  read_DB_free(v_result);

  return v_res;
}

void xmDatabase::bikes_setCustomTheme(const std::string& i_id_bike, const std::string& i_theme) {
  simpleSql("UPDATE bikes "
	    "SET customTheme=\"" + protectString(i_theme) + "\" "
	    "WHERE id_bike=\"" +   protectString(i_id_bike) + "\";");
}