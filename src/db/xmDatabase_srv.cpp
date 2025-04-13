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

#include "md5sum/md5file.h"
#include "xmDatabase.h"
#include <sstream>

bool xmDatabase::srv_isAdmin(const std::string &i_profile,
                             const std::string &i_password) {
  char **v_result;
  unsigned int nrow;
  std::string v_md5password = md5sum(i_password);

  v_result = readDB("SELECT id FROM srv_admins WHERE id_profile=\"" +
                      protectString(i_profile) +
                      "\" "
                      "AND password=\"" +
                      protectString(v_md5password) + "\";",
                    nrow);
  if (nrow != 1) {
    read_DB_free(v_result);
    return false;
  }
  read_DB_free(v_result);

  return true;
}

void xmDatabase::srv_addAdmin(const std::string &i_profile,
                              const std::string &i_password) {
  std::string v_md5password = md5sum(i_password);
  simpleSql("INSERT INTO srv_admins(id_profile, password) VALUES (\"" +
            protectString(i_profile) + "\", \"" + protectString(v_md5password) +
            "\");");
}

void xmDatabase::srv_removeAdmin(int id) {
  std::ostringstream v_n;
  v_n << id;
  simpleSql("DELETE FROM srv_admins WHERE id=" + v_n.str() + ";");
}

void xmDatabase::srv_addBan(const std::string &i_profile,
                            const std::string &i_ip,
                            unsigned int i_nbDays,
                            const std::string &i_admin_banner) {
  std::ostringstream v_n;
  v_n << i_nbDays;

  if (checkKey("SELECT count(1) FROM srv_bans WHERE id_profile=\"" +
               protectString(i_profile) + "\" AND ip=\"" + protectString(i_ip) +
               "\";")) {
    simpleSql(
      "UPDATE srv_bans SET from_date=datetime('now'), nb_days=" + v_n.str() +
      ", id_admin_banner=\"" + protectString(i_admin_banner) +
      "\" "
      "WHERE id_profile=\"" +
      protectString(i_profile) + "\" AND ip=\"" + protectString(i_ip) + "\";");
  } else {
    simpleSql("INSERT INTO srv_bans(id_profile, ip, from_date, nb_days, "
              "id_admin_banner) VALUES (\"" +
              protectString(i_profile) + "\", \"" + protectString(i_ip) +
              "\", datetime('now'), " + v_n.str() + ", \"" +
              protectString(i_admin_banner) + "\");");
  }
}

void xmDatabase::srv_removeBan(int id) {
  std::ostringstream v_n;
  v_n << id;
  simpleSql("DELETE FROM srv_bans WHERE id=" + v_n.str() + ";");
}

bool xmDatabase::srv_isBanned(const std::string &i_profile,
                              const std::string &i_ip) {
  char **v_result;
  unsigned int nrow;

  v_result =
    readDB("SELECT id FROM srv_bans "
           "WHERE (id_profile = '*' OR id_profile=\"" +
             protectString(i_profile) +
             "\") "
             "AND   (ip         = '*' OR ip        =\"" +
             protectString(i_ip) +
             "\") "
             "AND   nb_days - (julianday('now')-julianday(from_date)) > 0",
           nrow);
  read_DB_free(v_result);
  return nrow != 0;
}

void xmDatabase::srv_cleanBans() {
  simpleSql("DELETE FROM srv_bans WHERE nb_days - "
            "(julianday('now')-julianday(from_date)) <= 0;");
}

void xmDatabase::srv_changePassword(const std::string &i_profile,
                                    const std::string &i_password) {
  std::string v_md5password = md5sum(i_password);
  simpleSql("UPDATE srv_admins SET password=\"" + protectString(v_md5password) +
            "\" "
            "WHERE id_profile=\"" +
            protectString(i_profile) + "\";");
}
