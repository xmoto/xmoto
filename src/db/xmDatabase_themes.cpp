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

#include "helpers/VExcept.h"
#include "md5sum/md5file.h"
#include "xmDatabase.h"

bool xmDatabase::themes_isIndexUptodate() const {
  return m_requiredThemesUpdateAfterInit == false;
}

void xmDatabase::themes_add_begin() {
  simpleSql("BEGIN TRANSACTION;");
  simpleSql("DELETE FROM themes;");
}

void xmDatabase::themes_add(const std::string &i_id_theme,
                            const std::string &i_filepath) {
  simpleSql("INSERT INTO themes(id_theme, filepath, checkSum) "
            "VALUES (\"" +
            protectString(i_id_theme) + "\", \"" + protectString(i_filepath) +
            "\", \"" + protectString(md5file(i_filepath)) + "\");");
}

void xmDatabase::themes_update(const std::string &i_id_theme,
                               const std::string &i_filepath) {
  simpleSql("UPDATE themes "
            "SET filepath=\"" +
            protectString(i_filepath) + "\", checkSum=\"" +
            protectString(md5file(i_filepath)) +
            "\" "
            "WHERE id_theme=\"" +
            protectString(i_id_theme) + "\";");
}

void xmDatabase::themes_add_end() {
  simpleSql("COMMIT;");
}

void xmDatabase::themes_delete(const std::string &i_id_theme) {
  simpleSql("DELETE FROM themes WHERE id_theme=\"" + protectString(i_id_theme) +
            "\";");
}

bool xmDatabase::themes_exists(const std::string &i_id_theme) {
  char **v_result;
  unsigned int nrow;

  v_result = readDB("SELECT id_theme FROM themes WHERE id_theme=\"" +
                      protectString(i_id_theme) + "\";",
                    nrow);
  read_DB_free(v_result);
  return nrow == 1;
}

std::string xmDatabase::themes_getFileName(const std::string &i_id_theme) {
  char **v_result;
  unsigned int nrow;
  std::string v_res;

  v_result = readDB("SELECT filepath FROM themes WHERE id_theme=\"" +
                      protectString(i_id_theme) + "\";",
                    nrow);
  if (nrow != 1) {
    read_DB_free(v_result);
    throw Exception("Theme not found");
  }

  v_res = getResult(v_result, 1, 0, 0);
  read_DB_free(v_result);

  return v_res;
}
