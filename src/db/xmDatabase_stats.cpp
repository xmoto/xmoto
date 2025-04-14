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
#include "helpers/VExcept.h"
#include "xmDatabase.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"
#include <sstream>

/*
  IMPORTANT NOTE: this update is used ONLY when sitekey didn't exits !!
  you must change the query to take case of it if you want to reuse it
*/
void xmDatabase::updateDB_stats(XmDatabaseUpdateInterface *i_interface) {
  XMLDocument v_xml;
  xmlNodePtr v_xmlElt;
  std::string v_tag, v_tagv;
  std::string v_playerName, v_since, v_starts;
  std::string v_levelId, v_playedTime, v_played, v_died, v_restarted,
    v_completed;

  v_xml.readFromFile(FDT_DATA, "stats.xml");
  v_xmlElt = v_xml.getRootNode("stats");
  if (v_xmlElt == NULL) {
    throw Exception("Unable to read xml file");
  }

  if (v_xmlElt != NULL) {
    /* Start eating the XML */
    try {
      simpleSql("BEGIN TRANSACTION;");

      /* Get players */
      for (xmlNodePtr pSubElem = XMLDocument::subElement(v_xmlElt, "player");
           pSubElem != NULL;
           pSubElem = XMLDocument::nextElement(pSubElem)) {
        v_playerName = XMLDocument::getOption(pSubElem, "name");

        if (v_playerName != "") {
          v_since = XMLDocument::getOption(pSubElem, "since");
          if (v_since == "") {
            v_since = "2000-01-01 00:00:00";
          } else {
            v_since =
              v_since.substr(6, 10) + " " + v_since.substr(0, 5) + ":00";
          }

          v_starts = "";
          for (xmlNodePtr pSubSubElem =
                 XMLDocument::subElement(pSubElem, "stat");
               pSubSubElem != NULL;
               pSubSubElem = XMLDocument::nextElement(pSubSubElem)) {
            v_tag = XMLDocument::getOption(pSubSubElem, "tag");
            if (v_tag == "xmotostarts") {
              v_starts = XMLDocument::getOption(pSubSubElem, "v");
            }
          }

          /* insert row */
          simpleSql("INSERT INTO stats_profiles(id_profile, nbStarts, since) "
                    "VALUES(\"" +
                    protectString(v_playerName) + "\", " + v_starts + ", \"" +
                    v_since + "\");");

          if (i_interface != NULL) {
            i_interface->updatingDatabase(
              std::string(GAMETEXT_DB_UPGRADING_STATS_PROFILE) + " " +
                v_playerName,
              -1);
          }

          /* Read per-level stats */
          for (xmlNodePtr pSubSubElem =
                 XMLDocument::subElement(pSubElem, "level");
               pSubSubElem != NULL;
               pSubSubElem = XMLDocument::nextElement(pSubSubElem)) {
            v_levelId = XMLDocument::getOption(pSubSubElem, "id");
            if (v_levelId != "") {
              v_playedTime = "0";
              v_played = "0";
              v_died = "0";
              v_restarted = "0";
              v_completed = "0";

              /* Read stats */
              for (xmlNodePtr pSubSubSubElem =
                     XMLDocument::subElement(pSubSubElem, "stat");
                   pSubSubSubElem != NULL;
                   pSubSubSubElem = XMLDocument::nextElement(pSubSubSubElem)) {
                v_tag = XMLDocument::getOption(pSubSubSubElem, "tag");
                if (v_tag != "") {
                  v_tagv = XMLDocument::getOption(pSubSubSubElem, "v");
                  if (v_tagv != "") {
                    if (v_tag == "playtime")
                      v_playedTime = v_tagv;
                    else if (v_tag == "played")
                      v_played = v_tagv;
                    else if (v_tag == "died")
                      v_died = v_tagv;
                    else if (v_tag == "restarts")
                      v_restarted = v_tagv;
                    else if (v_tag == "completed")
                      v_completed = v_tagv;
                  }
                }
              }

              /* insert row */
              simpleSql(std::string(
                          "INSERT INTO stats_profiles_levels("
                          "id_profile, id_level, nbPlayed, nbDied, nbCompleted,"
                          "nbRestarted, playedTime) VALUES(") +
                        "\"" + protectString(v_playerName) + "\", " + "\"" +
                        protectString(v_levelId) + "\", " + v_played + ", " +
                        v_died + ", " + v_completed + ", " + v_restarted +
                        ", " + v_playedTime + "*100);");
            }
          }
        }
      }

      simpleSql("COMMIT;");
    } catch (Exception &e) {
      simpleSql("ROLLBACK;");
      throw e;
    }
  }
}

void xmDatabase::stats_createProfile(const std::string &i_sitekey,
                                     const std::string &i_profile) {
  simpleSql("INSERT INTO stats_profiles(sitekey, id_profile, nbStarts, since) "
            "VALUES(\"" +
            protectString(i_sitekey) + "\", \"" + protectString(i_profile) +
            "\", 0, \"" + GameApp::getTimeStamp() + "\");");
}

void xmDatabase::stats_destroyProfile(const std::string &i_profile) {
  /* delete with all sitekeys */
  try {
    simpleSql("BEGIN TRANSACTION;");
    simpleSql("DELETE FROM stats_profiles          WHERE id_profile=\"" +
              protectString(i_profile) + "\";");
    simpleSql("DELETE FROM levels_favorite         WHERE id_profile=\"" +
              protectString(i_profile) + "\";");
    simpleSql("DELETE FROM profile_completedLevels WHERE id_profile=\"" +
              protectString(i_profile) + "\";");
    simpleSql("DELETE FROM stats_profiles_levels   WHERE id_profile=\"" +
              protectString(i_profile) + "\";");
    simpleSql("COMMIT;");
  } catch (Exception &e) {
    simpleSql("ROLLBACK;");
    throw e;
  }
}

bool xmDatabase::stats_checkKeyExists_stats_profiles(
  const std::string &i_sitekey,
  const std::string &i_profile) {
  return checkKey("SELECT count(1) FROM stats_profiles "
                  "WHERE sitekey=\"" +
                  protectString(i_sitekey) +
                  "\" "
                  "AND   id_profile=\"" +
                  protectString(i_profile) + "\";");
}

bool xmDatabase::stats_checkKeyExists_stats_profiles_levels(
  const std::string &i_sitekey,
  const std::string &i_profile,
  const std::string &i_level) {
  return checkKey("SELECT count(1) FROM stats_profiles_levels "
                  "WHERE sitekey=\"" +
                  protectString(i_sitekey) +
                  "\" "
                  "AND   id_profile=\"" +
                  protectString(i_profile) +
                  "\" "
                  "AND   id_level=\"" +
                  protectString(i_level) + "\";");
}

void xmDatabase::stats_levelCompleted(const std::string &i_sitekey,
                                      const std::string &PlayerName,
                                      const std::string &LevelID,
                                      int i_playTime) {
  // printf("stats: level completed\n");
  std::ostringstream v_playTime;
  v_playTime << i_playTime;

  if (stats_checkKeyExists_stats_profiles_levels(
        i_sitekey, PlayerName, LevelID)) {
    simpleSql("UPDATE stats_profiles_levels SET "
              "nbCompleted=nbCompleted+1,"
              "nbPlayed=nbPlayed+1,"
              "playedTime=playedTime+" +
              v_playTime.str() + "," +
              "last_play_date=datetime('now', 'localtime'), "
              "synchronized = 0 "
              "WHERE sitekey=\"" +
              protectString(i_sitekey) +
              "\" "
              "AND id_profile=\"" +
              protectString(PlayerName) +
              "\" "
              "AND id_level=\"" +
              protectString(LevelID) + "\";");
  } else {
    simpleSql("INSERT INTO stats_profiles_levels("
              "sitekey, id_profile, id_level,"
              "nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime, "
              "last_play_date, synchronized) "
              "VALUES (\"" +
              protectString(i_sitekey) + "\",\"" + protectString(PlayerName) +
              "\", \"" + protectString(LevelID) + "\", 1, 0, 1, 0, \"" +
              v_playTime.str() + "\", datetime('now', 'localtime'), 0);");
  }
}

void xmDatabase::stats_died(const std::string &i_sitekey,
                            const std::string &PlayerName,
                            const std::string &LevelID,
                            int i_playTime) {
  // printf("stats: level dead\n");
  std::ostringstream v_playTime;
  v_playTime << i_playTime;

  if (stats_checkKeyExists_stats_profiles_levels(
        i_sitekey, PlayerName, LevelID)) {
    simpleSql("UPDATE stats_profiles_levels SET "
              "nbDied=nbDied+1,"
              "nbPlayed=nbPlayed+1,"
              "playedTime=playedTime+" +
              v_playTime.str() + "," +
              "last_play_date=datetime('now', 'localtime'), "
              "synchronized = 0 "
              "WHERE sitekey=\"" +
              protectString(i_sitekey) +
              "\" "
              "AND id_profile=\"" +
              protectString(PlayerName) +
              "\" "
              "AND id_level=\"" +
              protectString(LevelID) + "\";");
  } else {
    simpleSql("INSERT INTO stats_profiles_levels("
              "sitekey, id_profile, id_level,"
              "nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime, "
              "last_play_date, synchronized) "
              "VALUES (\"" +
              protectString(i_sitekey) + "\",\"" + protectString(PlayerName) +
              "\", \"" + protectString(LevelID) + "\", 1, 1, 0, 0, \"" +
              v_playTime.str() + "\", datetime('now', 'localtime'), 0);");
  }
}

void xmDatabase::stats_abortedLevel(const std::string &i_sitekey,
                                    const std::string &PlayerName,
                                    const std::string &LevelID,
                                    int i_playTime) {
  // printf("stats: level aborted\n");
  std::ostringstream v_playTime;
  v_playTime << i_playTime;

  if (stats_checkKeyExists_stats_profiles_levels(
        i_sitekey, PlayerName, LevelID)) {
    simpleSql("UPDATE stats_profiles_levels SET "
              "nbPlayed=nbPlayed+1,"
              "playedTime=playedTime+" +
              v_playTime.str() + "," +
              "last_play_date=datetime('now', 'localtime'), "
              "synchronized = 0 "
              "WHERE sitekey=\"" +
              protectString(i_sitekey) +
              "\" "
              "AND id_profile=\"" +
              protectString(PlayerName) +
              "\" "
              "AND id_level=\"" +
              protectString(LevelID) + "\";");
  } else {
    simpleSql("INSERT INTO stats_profiles_levels("
              "sitekey, id_profile, id_level,"
              "nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime, "
              "last_play_date, synchronized) "
              "VALUES (\"" +
              protectString(i_sitekey) + "\",\"" + protectString(PlayerName) +
              "\", \"" + protectString(LevelID) + "\", 1, 0, 0, 0, \"" +
              v_playTime.str() + "\", datetime('now', 'localtime'), 0);");
  }
}

void xmDatabase::stats_levelRestarted(const std::string &i_sitekey,
                                      const std::string &PlayerName,
                                      const std::string &LevelID,
                                      int i_playTime) {
  // printf("stats: level restarted\n");
  std::ostringstream v_playTime;
  v_playTime << i_playTime;

  if (stats_checkKeyExists_stats_profiles_levels(
        i_sitekey, PlayerName, LevelID)) {
    simpleSql("UPDATE stats_profiles_levels SET "
              "nbRestarted=nbRestarted+1,"
              "nbPlayed=nbPlayed+1,"
              "playedTime=playedTime+" +
              v_playTime.str() + "," +
              "last_play_date=datetime('now', 'localtime'), "
              "synchronized = 0 "
              "WHERE sitekey=\"" +
              protectString(i_sitekey) +
              "\" "
              "AND id_profile=\"" +
              protectString(PlayerName) +
              "\" "
              "AND id_level=\"" +
              protectString(LevelID) + "\";");
  } else {
    simpleSql("INSERT INTO stats_profiles_levels("
              "sitekey, id_profile, id_level,"
              "nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime, "
              "last_play_date, synchronized) "
              "VALUES (\"" +
              protectString(i_sitekey) + "\",\"" + protectString(PlayerName) +
              "\", \"" + protectString(LevelID) + "\", 1, 0, 0, 1, \"" +
              v_playTime.str() + "\", datetime('now', 'localtime'), 0);");
  }
}

void xmDatabase::stats_xmotoStarted(const std::string &i_sitekey,
                                    const std::string &PlayerName) {
  simpleSql("UPDATE stats_profiles SET "
            "nbStarts=nbStarts+1 "
            "WHERE sitekey=\"" +
            i_sitekey + "\" AND id_profile=\"" + protectString(PlayerName) +
            "\";");
}
