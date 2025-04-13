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

void xmDatabase::markAsVoted(const std::string &i_profile,
                             const std::string &i_id_level) {
  if (checkKey("SELECT count(1) FROM profiles_votes "
               "WHERE id_profile=\"" +
               protectString(i_profile) +
               "\" AND "
               "id_level=\"" +
               protectString(i_id_level) + "\";")) {
    return; // already marked as voted
  }

  simpleSql(
    std::string("INSERT INTO profiles_votes(id_profile, id_level) VALUES(") +
    "\"" + protectString(i_profile) + "\", \"" + protectString(i_id_level) +
    "\");");
}

bool xmDatabase::isVoted(const std::string &i_profile,
                         const std::string &i_id_level) {
  return checkKey("SELECT count(1) FROM profiles_votes WHERE "
                  "id_profile=\"" +
                  protectString(i_profile) +
                  "\" AND "
                  "id_level=\"" +
                  protectString(i_id_level) + "\";");
}
