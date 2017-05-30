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

/*
  IMPORTANT NOTE: this fix is used ONLY when sitekey didn't exits !!
  you must change the query to take case of it if you want to reuse it
*/
void xmDatabase::fixStatsProfilesLevelsNbCompleted() {
  char **v_result;
  unsigned int nrow;
  std::string v_id_level, v_profile, v_nb;

  // levels having stats cannot be marqued as not completed
  v_result = readDB(
    "SELECT a.id_level, a.id_profile, count(*) AS nb "
    "FROM stats_profiles_levels AS a INNER JOIN profile_completedLevels AS b "
    "ON (a.id_level = b.id_level AND a.id_profile = b.id_profile) "
    "WHERE a.nbCompleted = 0 GROUP BY a.id_level, a.id_profile;",
    nrow);

  for (unsigned int i = 0; i < nrow; i++) {
    v_id_level = getResult(v_result, 3, i, 0);
    v_profile = getResult(v_result, 3, i, 1);
    v_nb = getResult(v_result, 3, i, 2);

    simpleSql("UPDATE stats_profiles_levels SET nbCompleted=" + v_nb +
              " WHERE id_profile=\"" + protectString(v_profile) +
              "\" AND id_level=\"" + protectString(v_id_level) + "\"");
  }
  read_DB_free(v_result);
}
