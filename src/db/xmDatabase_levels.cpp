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

#include "common/VFileIO.h"
#include "common/VXml.h"
#include "helpers/Log.h"
#include "xmDatabase.h"
#include "xmscene/Level.h"
#include <sstream>

void xmDatabase::levels_add_begin(bool i_isToReload) {
  std::ostringstream v_cacheFV;
  v_cacheFV << CACHE_LEVEL_FORMAT_VERSION;

  simpleSql("BEGIN TRANSACTION;");

  if (i_isToReload) {
    simpleSql(
      "DELETE FROM levels "
      "WHERE isToReload=1 "
      "AND (loadingCacheFormatVersion IS NULL OR loadingCacheFormatVersion < " +
      v_cacheFV.str() + ");");
    simpleSql("UPDATE levels SET loaded=0 WHERE isToReload=1;");
  } else {
    simpleSql("DELETE FROM levels "
              "WHERE (loadingCacheFormatVersion IS NULL OR "
              "loadingCacheFormatVersion < " +
              v_cacheFV.str() + ");");
    simpleSql("UPDATE levels SET loaded=0;");
  }
}

void xmDatabase::levels_add_end() {
  simpleSql("DELETE FROM levels WHERE loaded=0;");
  simpleSql("COMMIT;");
}

void xmDatabase::levels_updateDB(const std::vector<Level *> &i_levels,
                                 bool i_isToReload,
                                 XmDatabaseUpdateInterface *i_interface) {
  simpleSql("BEGIN TRANSACTION;");
  simpleSql("DELETE FROM levels;");

  for (unsigned int i = 0; i < i_levels.size(); i++) {
    levels_add(i_levels[i]->Id(),
               i_levels[i]->FileName(),
               i_levels[i]->Name(),
               i_levels[i]->Checksum(),
               i_levels[i]->Author(),
               i_levels[i]->Description(),
               i_levels[i]->Date(),
               i_levels[i]->Music(),
               i_levels[i]->isScripted(),
               i_levels[i]->isPhysics(),
               i_isToReload);
  }

  simpleSql("COMMIT;");
}

int xmDatabase::levels_nbLevelsToDownload() {
  char **v_result;
  unsigned int nrow;
  v_result = readDB("SELECT a.name FROM weblevels AS a "
                    "LEFT OUTER JOIN levels AS b ON a.id_level=b.id_level "
                    "WHERE b.id_level IS NULL OR a.checkSum <> b.checkSum;",
                    nrow);
  read_DB_free(v_result);
  return nrow;
}

bool xmDatabase::levels_isIndexUptodate() const {
  return m_requiredLevelsUpdateAfterInit == false;
}

void xmDatabase::levels_addToFavorite(const std::string &i_profile,
                                      const std::string &i_id_level) {
  simpleSql("INSERT INTO levels_favorite(id_profile, id_level) "
            "VALUES(\"" +
            protectString(i_profile) + "\", \"" + protectString(i_id_level) +
            "\");");
}

void xmDatabase::levels_delToFavorite(const std::string &i_profile,
                                      const std::string &i_id_level) {
  simpleSql("DELETE FROM levels_favorite "
            "WHERE id_profile=\"" +
            protectString(i_profile) + "\" AND  id_level=\"" +
            protectString(i_id_level) + "\";");
}

void xmDatabase::levels_addToBlacklist(const std::string &i_profile,
                                       const std::string &i_id_level) {
  simpleSql("INSERT INTO levels_blacklist(id_profile, id_level) "
            "VALUES(\"" +
            protectString(i_profile) + "\", \"" + protectString(i_id_level) +
            "\");");
}

void xmDatabase::levels_delToBlacklist(const std::string &i_profile,
                                       const std::string &i_id_level) {
  simpleSql("DELETE FROM levels_blacklist "
            "WHERE id_profile=\"" +
            protectString(i_profile) + "\" AND  id_level=\"" +
            protectString(i_id_level) + "\";");
}

void xmDatabase::updateDB_favorite(const std::string &i_profile,
                                   XmDatabaseUpdateInterface *i_interface) {
  XMLDocument v_xml;
  xmlNodePtr v_xmlElt;
  std::string v_levelId;

  v_xml.readFromFile(FDT_DATA, "favoriteLevels.xml");
  v_xmlElt = v_xml.getRootNode("favoriteLevels");
  if (v_xmlElt == NULL) {
    throw Exception("Unable to read xml file");
  }

  try {
    simpleSql("BEGIN TRANSACTION;");

    for (xmlNodePtr pSubElem = XMLDocument::subElement(v_xmlElt, "level");
         pSubElem != NULL;
         pSubElem = XMLDocument::nextElement(pSubElem)) {
      v_levelId = XMLDocument::getOption(pSubElem, "id");
      if (v_levelId != "") {
        /* add the level into the list */
        simpleSql("INSERT INTO levels_favorite(id_profile, id_level) "
                  "VALUES(\"" +
                  protectString(i_profile) + "\", \"" +
                  protectString(v_levelId) + "\");");
      }
    }
    simpleSql("COMMIT;");
  } catch (Exception &e) {
    simpleSql("ROLLBACK;");
  }
}

void xmDatabase::levels_addToNew_begin() {
  simpleSql("BEGIN TRANSACTION;");
}

void xmDatabase::levels_addToNew_end() {
  simpleSql("COMMIT;");
}

void xmDatabase::levels_cleanNew() {
  simpleSql("DELETE FROM levels_new;");
}

void xmDatabase::levels_addToNew(const std::string &i_id_level,
                                 bool i_isAnUpdate) {
  simpleSql("INSERT INTO levels_new(id_level, isAnUpdate) "
            "VALUES (\"" +
            protectString(i_id_level) + "\", " +
            std::string(i_isAnUpdate ? "1" : "0") + ");");
}

void xmDatabase::levels_add(const std::string &i_id_level,
                            const std::string &i_filepath,
                            const std::string &i_name,
                            const std::string &i_checkSum,
                            const std::string &i_author,
                            const std::string &i_description,
                            const std::string &i_date,
                            const std::string &i_music,
                            bool i_isScripted,
                            bool i_isPhysics,
                            bool i_isToReload) {
  std::ostringstream v_cacheFV;
  v_cacheFV << CACHE_LEVEL_FORMAT_VERSION;

  simpleSql("INSERT INTO levels(id_level,"
            "filepath, name, checkSum, author, description, "
            "date_str, music, isScripted, isPhysics, isToReload, loaded, "
            "loadingCacheFormatVersion) "
            "VALUES(\"" +
            protectString(i_id_level) + "\", " + "\"" +
            protectString(i_filepath) + "\", " + "\"" + protectString(i_name) +
            "\", " + "\"" + protectString(i_checkSum) + "\", " + "\"" +
            protectString(i_author) + "\", " + "\"" +
            protectString(i_description) + "\", " + "\"" +
            protectString(i_date) + "\", " + "\"" + protectString(i_music) +
            "\", " + std::string(i_isScripted ? "1" : "0") + ", " +
            std::string(i_isPhysics ? "1" : "0") + ", " +
            std::string(i_isToReload ? "1" : "0") +
            std::string(", 1, " + v_cacheFV.str() + ");"));
}

void xmDatabase::levels_update(const std::string &i_id_level,
                               const std::string &i_filepath,
                               const std::string &i_name,
                               const std::string &i_checkSum,
                               const std::string &i_author,
                               const std::string &i_description,
                               const std::string &i_date,
                               const std::string &i_music,
                               bool i_isScripted,
                               bool i_isPhysics,
                               bool i_isToReload) {
  std::ostringstream v_cacheFV;
  v_cacheFV << CACHE_LEVEL_FORMAT_VERSION;

  simpleSql("UPDATE levels SET name=\"" + protectString(i_name) +
            "\", filepath=\"" + protectString(i_filepath) + "\", checkSum=\"" +
            protectString(i_checkSum) + "\", author=\"" +
            protectString(i_author) + "\", description=\"" +
            protectString(i_description) + "\", date_str=\"" +
            protectString(i_date) + "\", music=\"" + protectString(i_music) +
            "\", isScripted=" + std::string(i_isScripted ? "1" : "0") +
            ", isPhysics=" + std::string(i_isPhysics ? "1" : "0") +
            ", isToReload=" + std::string(i_isToReload ? "1" : "0") +
            ", loaded=1, loadingCacheFormatVersion = " + v_cacheFV.str() +
            " WHERE id_level=\"" + protectString(i_id_level) + "\";");
}

void xmDatabase::levels_cleanNoWWWLevels() {
  char **v_result;
  unsigned int nrow;
  std::string v_name, v_filepath, v_id_level;
  std::string v_savePath =
    XMFS::getUserDir(FDT_DATA) + std::string("/Trash/Levels");
  std::string v_basename;

  // make directory for levels
  XMFS::mkArborescenceDir(v_savePath);

  v_result = readDB(
    "SELECT a.id_level, a.name, a.filepath "
    "FROM levels AS a LEFT OUTER JOIN weblevels AS b ON a.id_level=b.id_level "
    "WHERE b.id_level IS NULL AND isToReload=0;",
    nrow);
  for (unsigned int i = 0; i < nrow; i++) {
    v_id_level = getResult(v_result, 3, i, 0);
    v_name = getResult(v_result, 3, i, 1);
    v_filepath = getResult(v_result, 3, i, 2);

    if (XMFS::isInUserDir(FDT_DATA,
                          v_filepath)) { // remove only files of the user dir
      LogInfo("Removing level %s (%s)", v_name.c_str(), v_filepath.c_str());
      try {
        simpleSql("DELETE FROM levels WHERE id_level=\"" +
                  protectString(v_id_level) + "\";");

        v_basename = XMFS::getFileBaseName(v_filepath);
        if (XMFS::moveFile(v_filepath,
                           v_savePath + "/" + v_basename + ".lvl") == false) {
          LogWarning("Unable to move the file into the trash");
        }
      } catch (Exception &e) {
        LogWarning("Removing the level failed !");
      }

    } else {
      LogWarning(
        "NOT Removing level %s (%s) : the level is not is the user space",
        v_name.c_str(),
        v_filepath.c_str());
    }
  }
  read_DB_free(v_result);
}

// if a level has the same checksum and has been loaded in the save cache format
// version, don't reanalyse the level,
// just resussite it from the trash (loaded=0)
bool xmDatabase::levels_add_fast(const std::string &i_filepath,
                                 std::string &o_levelName,
                                 bool i_isToReload) {
  char **v_result;
  unsigned int nrow;
  unsigned int i;
  bool v_found;
  std::string v_checksum, v_dbchecksum;
  std::string v_cond;
  if (i_isToReload) {
    v_cond = "isToReload=1 AND ";
  }

  v_result =
    readDB("SELECT name, checkSum FROM levels "
           "WHERE " +
             v_cond + "filepath=\"" + protectString(i_filepath) + "\";",
           nrow);

  // no result, no need to compute checksum
  if (nrow == 0) {
    read_DB_free(v_result);
    return false;
  }

  // checksum
  v_checksum = XMFS::md5sum(FDT_DATA, i_filepath);
  i = 0;
  v_found = false;
  while (i < nrow && v_found == false) {
    v_dbchecksum = getResult(v_result, 2, i, 1);
    if (v_dbchecksum == v_checksum) {
      v_found = true;
      o_levelName = getResult(v_result, 2, i, 0);
    }
    i++;
  }
  read_DB_free(v_result);

  // no level with the same checksum found
  if (v_found == false) {
    return false;
  }

  // found it in the database
  simpleSql("UPDATE levels SET loaded=1 WHERE " + v_cond + "filepath=\"" +
            protectString(i_filepath) +
            "\" "
            "AND checkSum=\"" +
            protectString(v_checksum) + "\";");
  return true;
}
