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
#include <sstream>

bool xmDatabase::srv_isAdmin(const std::string& i_profile, const std::string& i_password) {
  char **v_result;
  unsigned int nrow;
  std::string v_md5password = md5sum(i_password);

  v_result = readDB("SELECT id FROM srv_admins WHERE id_profile=\"" + protectString(i_profile) + "\" "
		    "AND password=\"" + protectString(v_md5password) + "\";",
		    nrow);
  if(nrow != 1) {
    read_DB_free(v_result);
    return false;
  }
  read_DB_free(v_result);

  return true;
}

void xmDatabase::srv_addAdmin(const std::string& i_login, const std::string& i_password) {
}

void xmDatabase::srv_removeAdmin(int id) {
  std::ostringstream v_n;
  v_n << id;
  simpleSql("DELETE FROM srv_admins WHERE id=" + v_n.str() + ";");
}
