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
#include "../md5sum/md5file.h"
#include "../helpers/VExcept.h"

bool xmDatabase::physics_isIndexUptodate() const {
  return m_requiredBikesUpdateAfterInit == false;
}

void xmDatabase::physics_add_begin() {
  simpleSql("BEGIN TRANSACTION;");
  simpleSql("DELETE FROM physics;");
}

void xmDatabase::physics_add(const std::string& i_id_physics,
			     const std::string& i_filepath) {
  simpleSql("INSERT INTO physics(id_physics, filepath, checkSum) " 
	    "VALUES (\"" +
	    protectString(i_id_physics)   + "\", \"" +
	    protectString(i_filepath)   + "\", \"" +
	    protectString(md5file(i_filepath)) + "\");");
}

void xmDatabase::physics_update(const std::string& i_id_physics,
			       const std::string& i_filepath) {
  simpleSql("UPDATE physics "
	    "SET filepath=\"" + protectString(i_filepath) +
	    "\", checkSum=\"" + protectString(md5file(i_filepath)) + "\" "
	    "WHERE id_physics=\"" + protectString(i_id_physics) + "\";");
}

void xmDatabase::physics_add_end() {
  simpleSql("COMMIT;");
}

void xmDatabase::physics_delete(const std::string& i_id_physics) {
  simpleSql("DELETE FROM physics WHERE id_physics=\""+ protectString(i_id_physics) + "\";");
}

bool xmDatabase::physics_exists(const std::string& i_id_physics) {
  char **v_result;
  unsigned int nrow;

  v_result = readDB("SELECT id_physics FROM physics WHERE id_physics=\"" + protectString(i_id_physics) + "\";",
		    nrow);
  read_DB_free(v_result);
  return nrow == 1;
}

std::string xmDatabase::physics_getFileName(const std::string& i_id_physics) {
  char **v_result;
  unsigned int nrow;
  std::string v_res;

  v_result = readDB("SELECT filepath FROM physics WHERE id_physics=\"" + protectString(i_id_physics) + "\";",
		    nrow);
  if(nrow != 1) {
    read_DB_free(v_result);
    throw Exception("Physics not found");
  }

  v_res = getResult(v_result, 1, 0, 0);
  read_DB_free(v_result);

  return v_res;
}
