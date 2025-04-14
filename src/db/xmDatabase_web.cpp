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

#include "common/VXml.h"
#include "common/WWW.h"
#include "helpers/Log.h"
#include "helpers/VExcept.h"
#include "xmDatabase.h"
#include "xmoto/GameText.h"
#include <sstream>

#define XM_NB_THIEFS_MAX 3

bool xmDatabase::webrooms_checkKeyExists_id_room(const std::string &i_id_room) {
  return checkKey("SELECT count(1) FROM webrooms "
                  "WHERE id_room=" +
                  i_id_room + ";");
}

void xmDatabase::webrooms_addRoom(const std::string &i_id_room,
                                  const std::string &i_name,
                                  const std::string &i_highscoreUrl) {
  simpleSql("INSERT INTO webrooms("
            "id_room,  name, highscoresUrl) "
            "VALUES(" +
            i_id_room + ", " + "\"" + protectString(i_name) + "\", " + "\"" +
            protectString(i_highscoreUrl) + "\");");
}

std::string xmDatabase::webhighscores_updateDB(
  FileDataType i_fdt,
  const std::string &i_webhighscoresFile,
  const std::string &i_websource) {
  XMLDocument v_xml;
  xmlNodePtr v_xmlElt;
  std::string v_roomName;
  std::string v_roomId;
  std::string v_levelId;
  std::string v_player;
  std::string v_rplUrl;
  std::string v_strtime;
  std::string v_date;
  int v_time;
  size_t pos_1, pos_2;

  try {
    simpleSql("BEGIN TRANSACTION;");

    v_xml.readFromFile(i_fdt, i_webhighscoresFile);

    v_xmlElt = v_xml.getRootNode("xmoto_worldrecords");
    if (v_xmlElt == NULL) {
      throw Exception("unable to analyze xml highscore file");
    }

    /* get Room information */
    v_roomName = XMLDocument::getOption(v_xmlElt, "roomname");
    v_roomId = XMLDocument::getOption(v_xmlElt, "roomid");

    if (v_roomId == "") {
      throw Exception("error : unable to analyze xml highscore file");
    }

    /* be sure the room is into the webrooms table */
    if (webrooms_checkKeyExists_id_room(v_roomId) == false) {
      webrooms_addRoom(v_roomId, v_roomName, i_websource);
    }

    simpleSql("DELETE FROM webhighscores WHERE id_room=" + v_roomId + ";");

    for (xmlNodePtr pSubElem = XMLDocument::subElement(v_xmlElt, "worldrecord");
         pSubElem != NULL;
         pSubElem = XMLDocument::nextElement(pSubElem)) {
      v_levelId = XMLDocument::getOption(pSubElem, "level_id");
      if (v_levelId == "") {
        continue;
      }

      v_player = XMLDocument::getOption(pSubElem, "player");
      if (v_player == "") {
        continue;
      }

      /* time */
      v_strtime = XMLDocument::getOption(pSubElem, "time");
      if (v_strtime == "") {
        continue;
      }

      pos_1 = v_strtime.find(":", 0);
      if (pos_1 == std::string::npos || pos_1 == v_strtime.length() - 1)
        continue;
      pos_2 = v_strtime.find(":", pos_1 + 1);
      if (pos_2 == std::string::npos)
        continue;

      v_time =
        atoi(v_strtime.substr(0, pos_1).c_str()) * 6000 +
        atoi(v_strtime.substr(pos_1 + 1, pos_2 - pos_1 - 1).c_str()) * 100 +
        atoi(
          v_strtime.substr(pos_2 + 1, v_strtime.length() - pos_2 - 1).c_str());

      /* replay */
      v_rplUrl = XMLDocument::getOption(pSubElem, "replay");
      if (v_rplUrl == "") {
        continue;
      }

      /* date */
      v_date = XMLDocument::getOption(pSubElem, "date");
      if (v_date == "") {
        continue;
      }

      std::ostringstream v_secondTime;
      v_secondTime << v_time;

      simpleSql("INSERT INTO webhighscores(id_room, id_level, id_profile, "
                "finishTime, date, fileUrl)"
                "VALUES(" +
                v_roomId + ", \"" + protectString(v_levelId) + "\", \"" +
                protectString(v_player) + "\", " + v_secondTime.str() + ", \"" +
                v_date + "\", \"" + protectString(v_rplUrl) + "\");");
    }
    simpleSql("COMMIT;");
  } catch (Exception &e) {
    simpleSql("ROLLBACK;");
    throw e;
  }
  return v_roomId;
}

std::string xmDatabase::webrooms_getName(const std::string &i_id_room) {
  char **v_result;
  unsigned int nrow;
  std::string v_res;

  v_result =
    readDB("SELECT name FROM webrooms WHERE id_room=" + i_id_room + ";", nrow);
  if (nrow != 1) {
    read_DB_free(v_result);
    return ""; /* not found */
  }
  v_res = getResult(v_result, 1, 0, 0);

  read_DB_free(v_result);
  return v_res;
}

int xmDatabase::webrooms_getHighscoreTime(const std::string &i_id_room,
                                          const std::string &i_id_level) {
  char **v_result;
  unsigned int nrow;
  std::string v_res;

  v_result =
    readDB("SELECT finishTime FROM webhighscores "
           "WHERE id_room=" +
             i_id_room + " AND id_level=\"" + protectString(i_id_level) + "\";",
           nrow);
  if (nrow != 1) {
    read_DB_free(v_result);
    return -1; /* not found */
  }
  v_res = getResult(v_result, 1, 0, 0);

  read_DB_free(v_result);
  return atoi(v_res.c_str());
}

bool xmDatabase::isOnTheWeb(const std::string &i_id_level) {
  return checkKey("SELECT count(1) FROM weblevels WHERE id_level=\"" +
                  protectString(i_id_level) + "\";");
}

bool xmDatabase::isWebVoteLocked(const std::string &i_id_level) {
  return checkKey("SELECT count(1) FROM weblevels WHERE id_level=\"" +
                  protectString(i_id_level) + "\" AND vote_locked=1;");
}

void xmDatabase::weblevels_updateDB(FileDataType i_fdt,
                                    const std::string &i_weblevelsFile) {
  XMLDocument v_xml;
  xmlNodePtr v_xmlElt;

  try {
    simpleSql("BEGIN TRANSACTION;");
    simpleSql("DELETE FROM weblevels;");

    v_xml.readFromFile(i_fdt, i_weblevelsFile);

    v_xmlElt = v_xml.getRootNode("xmoto_levels");
    if (v_xmlElt == NULL) {
      throw Exception("unable to analyze xml file");
    }

    for (xmlNodePtr pSubElem = XMLDocument::subElement(v_xmlElt, "level");
         pSubElem != NULL;
         pSubElem = XMLDocument::nextElement(pSubElem)) {
      std::string v_levelId, v_levelName, v_url, v_MD5sum_web;
      std::string v_difficulty, v_quality, v_creationDate;
      std::string v_crappy, v_children_compliant, v_vote_locked;
      std::string v_packname, v_packnum;

      v_levelId = XMLDocument::getOption(pSubElem, "level_id");
      if (v_levelId == "")
        continue;

      v_levelName = XMLDocument::getOption(pSubElem, "name");
      if (v_levelName == "")
        continue;

      v_packname = XMLDocument::getOption(pSubElem, "packname");
      if (v_packname != "") {
        v_packnum = XMLDocument::getOption(pSubElem, "packnum");
      }

      v_url = XMLDocument::getOption(pSubElem, "url");
      if (v_url == "")
        continue;

      v_MD5sum_web = XMLDocument::getOption(pSubElem, "sum");
      if (v_MD5sum_web == "")
        continue;

      /* web information */
      v_difficulty = XMLDocument::getOption(pSubElem, "web_difficulty");
      if (v_difficulty == "")
        continue;
      for (unsigned int i = 0; i < v_difficulty.length(); i++) {
        if (v_difficulty[i] == ',')
          v_difficulty[i] = '.';
      }

      v_quality = XMLDocument::getOption(pSubElem, "web_quality");
      if (v_quality == "")
        continue;
      for (unsigned int i = 0; i < v_quality.length(); i++) {
        if (v_quality[i] == ',')
          v_quality[i] = '.';
      }

      v_creationDate = XMLDocument::getOption(pSubElem, "creation_date");
      if (v_creationDate == "")
        continue;

      v_crappy = XMLDocument::getOption(pSubElem, "crappy");
      if (v_crappy == "") {
        v_crappy = "0";
      } else {
        v_crappy = v_crappy == "true" ? "1" : "0";
      }

      v_children_compliant =
        XMLDocument::getOption(pSubElem, "children_compliant");
      if (v_children_compliant == "") {
        v_children_compliant = "1";
      } else {
        v_children_compliant = v_children_compliant == "true" ? "1" : "0";
      }

      v_vote_locked = XMLDocument::getOption(pSubElem, "vote_locked");
      if (v_vote_locked == "") {
        v_vote_locked = "0";
      } else {
        v_vote_locked = v_vote_locked == "true" ? "1" : "0";
      }

      // add the level
      simpleSql(
        "INSERT INTO weblevels(id_level, name, packname, packnum, fileUrl, "
        "checkSum, difficulty, quality, creationDate, crappy, "
        "children_compliant, vote_locked) VALUES (\"" +
        protectString(v_levelId) + "\", \"" + protectString(v_levelName) +
        "\", \"" + protectString(v_packname) + "\", \"" +
        protectString(v_packnum) + "\", \"" + protectString(v_url) + "\", \"" +
        protectString(v_MD5sum_web) + "\", " + v_difficulty + ", " + v_quality +
        ", \"" + v_creationDate + "\", " + v_crappy + ", " +
        v_children_compliant + ", " + v_vote_locked + ");");
    }
    simpleSql("COMMIT;");
  } catch (Exception &e) {
    simpleSql("ROLLBACK;");
    throw e;
  }
}

void xmDatabase::webrooms_updateDB(FileDataType i_fdt,
                                   const std::string &i_webroomsFile) {
  XMLDocument v_xml;
  xmlNodePtr v_xmlElt;

  std::string v_RoomName, v_RoomHighscoreUrl, v_RoomId;

  try {
    simpleSql("BEGIN TRANSACTION;");
    simpleSql("DELETE FROM webrooms;");

    v_xml.readFromFile(i_fdt, i_webroomsFile);

    v_xmlElt = v_xml.getRootNode("xmoto_rooms");
    if (v_xmlElt == NULL) {
      throw Exception("unable to analyze xml file");
    }

    for (xmlNodePtr pSubElem = XMLDocument::subElement(v_xmlElt, "room");
         pSubElem != NULL;
         pSubElem = XMLDocument::nextElement(pSubElem)) {
      v_RoomName = XMLDocument::getOption(pSubElem, "name");
      if (v_RoomName == "")
        continue;

      v_RoomHighscoreUrl = XMLDocument::getOption(pSubElem, "highscores_url");
      if (v_RoomName == "")
        continue;

      v_RoomId = XMLDocument::getOption(pSubElem, "id");
      if (v_RoomId == "")
        continue;

      webrooms_addRoom(v_RoomId, v_RoomName, v_RoomHighscoreUrl);
    }
    simpleSql("COMMIT;");
  } catch (Exception &e) {
    simpleSql("ROLLBACK;");
    throw e;
  }
}

void xmDatabase::webthemes_updateDB(FileDataType i_fdt,
                                    const std::string &i_webThemesFile) {
  XMLDocument v_xml;
  xmlNodePtr v_xmlElt;
  std::string v_themeName, v_url, v_MD5sum_web;

  try {
    simpleSql("BEGIN TRANSACTION;");
    simpleSql("DELETE FROM webthemes;");

    v_xml.readFromFile(i_fdt, i_webThemesFile);

    v_xmlElt = v_xml.getRootNode("xmoto_themes");
    if (v_xmlElt == NULL) {
      throw Exception("unable to analyze xml file");
    }

    for (xmlNodePtr pSubElem = XMLDocument::subElement(v_xmlElt, "theme");
         pSubElem != NULL;
         pSubElem = XMLDocument::nextElement(pSubElem)) {
      v_themeName = XMLDocument::getOption(pSubElem, "name");
      if (v_themeName == "")
        continue;

      v_url = XMLDocument::getOption(pSubElem, "url");
      if (v_url == "")
        continue;

      v_MD5sum_web = XMLDocument::getOption(pSubElem, "sum");
      if (v_MD5sum_web == "")
        continue;

      webthemes_addTheme(v_themeName, v_url, v_MD5sum_web);
    }
    simpleSql("COMMIT;");
  } catch (Exception &e) {
    simpleSql("ROLLBACK;");
    throw e;
  }
}

void xmDatabase::webthemes_addTheme(const std::string &i_id_theme,
                                    const std::string &i_url,
                                    const std::string &i_checkSum) {
  simpleSql("INSERT INTO webthemes("
            "id_theme,  fileUrl, checkSum) "
            "VALUES(\"" +
            protectString(i_id_theme) + "\", " + "\"" + protectString(i_url) +
            "\", " + "\"" + protectString(i_checkSum) + "\");");
}

void xmDatabase::webLoadDataFirstTime() {
  char **v_result;
  unsigned int nrow;
  bool v_update = false;

  /* updating weblevels table with defaults */
  try {
    /* if the is any row already in the table, no update is done */
    v_result = readDB("SELECT count(1) FROM weblevels;", nrow);
    if (nrow == 1) {
      v_update = atoi(getResult(v_result, 1, 0, 0)) == 0;
    }
    read_DB_free(v_result);

    if (v_update) {
      LogInfo("Loading weblevels with delivered weblevels.xml");
      weblevels_updateDB(FDT_DATA, "default/weblevels.xml"); // FDT_DATA because
      // first loading
      // file a really a
      // data
    }
  } catch (Exception &e) {
    /* ok, no pb */
    LogWarning("Loading delivered weblevels.xml failed");
  }

  /* updating webhighscores table with defaults */
  try {
    /* if the is any row already in the table, no update is done */
    v_result = readDB("SELECT count(1) FROM webhighscores;", nrow);
    if (nrow == 1) {
      v_update = atoi(getResult(v_result, 1, 0, 0)) == 0;
    }
    read_DB_free(v_result);

    if (v_update) {
      LogInfo("Loading webhighscores with delivered webhighscores.xml");
      webhighscores_updateDB(FDT_DATA,
                             "default/webhighscores.xml",
                             DEFAULT_WEBHIGHSCORES_URL); // FDT_DATA because
      // first loading file a
      // really a data
    }
  } catch (Exception &e) {
    /* ok, no pb */
    LogWarning("Loading delivered webhighscores.xml failed");
  }
}

void xmDatabase::updateMyHighscoresFromHighscores(
  const std::string &i_id_profile) {
  char **v_result;
  unsigned int nrow;

  /* get my highscores which are in webhighscores but not in
   * levels_mywebhighscores */
  try {
    v_result =
      readDB("SELECT a.id_profile, a.id_room, a.id_level "
             "FROM webhighscores AS a "
             "LEFT OUTER JOIN levels_mywebhighscores AS b ON (a.id_room    = "
             "b.id_room     "
             "AND a.id_level   = b.id_level    "
             "AND a.id_profile = b.id_profile) "
             "WHERE a.id_profile = \"" +
               protectString(i_id_profile) +
               "\" "
               "AND   b.id_profile IS NULL;",
             nrow);

    if (nrow > 0) {
      try {
        simpleSql("BEGIN TRANSACTION;");
        for (unsigned int i = 0; i < nrow; i++) {
          simpleSql("INSERT INTO levels_mywebhighscores("
                    "id_profile, id_room, id_level) "
                    "VALUES(\"" +
                    protectString(getResult(v_result, 3, i, 0)) + "\", " +
                    getResult(v_result, 3, i, 1) + ", " + "\"" +
                    protectString(getResult(v_result, 3, i, 2)) + "\");");
        }
        simpleSql("COMMIT;");
      } catch (Exception &e) {
        simpleSql("ROLLBACK;");
      }
    }

    read_DB_free(v_result);
  } catch (Exception &e) {
    /* ok, no pb */
    LogWarning("Unable to update my highscores");
  }

  /* reset levels i get back */
  updateMyHighscoresKnownStolenBack(i_id_profile);
}

void xmDatabase::updateMyHighscoresKnownStolenBack(
  const std::string &i_id_profile) {
  char **v_result;
  unsigned int nrow;

  /* reset to known_stolen=0 levels where known_stolen = 1 but highscore owner =
   * i_id_profile */
  try {
    v_result = readDB(
      "SELECT a.id_profile, a.id_room, a.id_level "
      "FROM webhighscores AS a "
      "INNER JOIN levels_mywebhighscores AS b ON (a.id_room    = b.id_room     "
      "AND a.id_level   = b.id_level    "
      "AND a.id_profile = b.id_profile) "
      "WHERE a.id_profile = \"" +
        protectString(i_id_profile) +
        "\" "
        "AND   b.known_stolen = 1;",
      nrow);

    if (nrow > 0) {
      try {
        simpleSql("BEGIN TRANSACTION;");
        for (unsigned int i = 0; i < nrow; i++) {
          simpleSql("UPDATE levels_mywebhighscores "
                    "SET known_stolen=0 "
                    "WHERE id_profile=\"" +
                    protectString(getResult(v_result, 3, i, 0)) +
                    "\" "
                    "AND   id_room=" +
                    getResult(v_result, 3, i, 1) +
                    " "
                    "AND   id_level=\"" +
                    protectString(getResult(v_result, 3, i, 2)) + "\";");
        }
        simpleSql("COMMIT;");
      } catch (Exception &e) {
        simpleSql("ROLLBACK;");
      }
    }

    read_DB_free(v_result);
  } catch (Exception &e) {
    /* ok, no pb */
    LogWarning("Unable to update known stolen on my highscores");
  }
}

bool xmDatabase::markMyHighscoresKnownStolen(const std::string &i_id_profile,
                                             std::string &o_stolen_msg) {
  char **v_result;
  unsigned int nrow;
  bool v_res;
  char v_line_info[256];

  v_res = false;
  o_stolen_msg = "";

  try {
    v_result = readDB(
      "SELECT a.id_room, a.id_level, c.name, a.id_profile, d.name "
      "FROM webhighscores AS a "
      "INNER JOIN levels_mywebhighscores AS b ON (a.id_room    = b.id_room     "
      "AND a.id_level   = b.id_level    "
      "AND b.id_profile = \"" +
        protectString(i_id_profile) +
        "\") "
        "INNER JOIN levels AS c ON a.id_level = c.id_level "
        "INNER JOIN webrooms AS d ON a.id_room=d.id_room "
        "WHERE a.id_profile <> \"" +
        protectString(i_id_profile) +
        "\" "
        "AND   b.known_stolen = 0 "
        "ORDER BY a.date DESC;",
      nrow);

    if (nrow > 0) {
      v_res = true;

      try {
        simpleSql("BEGIN TRANSACTION;");
        for (unsigned int i = 0; i < nrow; i++) {
          if (i < XM_NB_THIEFS_MAX) {
            if (o_stolen_msg != "") {
              o_stolen_msg += "\n";
            }

            snprintf(v_line_info,
                     256,
                     GAMETEXT_HIGHSCORE_STOLEN,
                     getResult(v_result, 5, i, 4),
                     getResult(v_result, 5, i, 3),
                     getResult(v_result, 5, i, 2));
            o_stolen_msg += v_line_info;

          } else if (i == XM_NB_THIEFS_MAX) {
            o_stolen_msg += "\n...";
          }

          simpleSql("UPDATE levels_mywebhighscores "
                    "SET known_stolen=1, known_stolen_date=datetime('now', "
                    "'localtime') "
                    "WHERE id_profile=\"" +
                    protectString(i_id_profile) +
                    "\" "
                    "AND   id_room=" +
                    getResult(v_result, 5, i, 0) +
                    " "
                    "AND   id_level=\"" +
                    protectString(getResult(v_result, 5, i, 1)) + "\";");
        }
        simpleSql("COMMIT;");
      } catch (Exception &e) {
        simpleSql("ROLLBACK;");
      }
    }

    read_DB_free(v_result);
  } catch (Exception &e) {
    /* ok, no pb */
    LogWarning("Unable to update known stolen on my highscores");
  }

  return v_res;
}
