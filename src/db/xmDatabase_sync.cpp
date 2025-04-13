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
#include "common/XMSession.h"
#include "helpers/VExcept.h"
#include "xmDatabase.h"
#include <sstream>

void xmDatabase::sync_buildServerFile(const std::string &i_outFile,
                                      const std::string &i_sitekey,
                                      const std::string &i_profile) {
  char **v_result;
  unsigned int nrow;
  std::string v_res;
  char v_line[2048];
  std::ostringstream v_lastDbSync;

  FileHandle *pfh = XMFS::openOFile(FDT_CACHE, i_outFile);

  /*
    update dbSync:
    preparing all waiting lines with the new dbSync (incremented of 1)
  */
  XMSession::instance()->setDbSync(
    this, i_profile, XMSession::instance()->dbSync(this, i_profile) + 1);
  v_lastDbSync << XMSession::instance()->dbSync(this, i_profile);

  simpleSql("UPDATE profile_completedLevels SET dbSync=\"" +
            v_lastDbSync.str() + "\" WHERE synchronized=0;");
  simpleSql("UPDATE stats_profiles_levels   SET dbSync=\"" +
            v_lastDbSync.str() + "\" WHERE synchronized=0;");
  /* ***** */

  if (pfh == NULL) {
    throw Exception("Unable to open " + i_outFile);
  }

  XMFS::writeLine(pfh, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");

  /* header */
  v_result = readDB("SELECT nbStarts, since "
                    "FROM stats_profiles "
                    "WHERE sitekey=\"" +
                      protectString(i_sitekey) + "\" AND id_profile=\"" +
                      protectString(i_profile) + "\";",
                    nrow);
  if (nrow != 1) {
    read_DB_free(v_result);
    throw Exception("Unable to retrieve information");
  }

  snprintf(v_line,
           2048,
           "<xmoto_sync fileformat=\"1\" sitekey=\"%s\" profile=\"%s\" "
           "nbStarts=\"%i\" since=\"%s\">",
           XMLDocument::str2xmlstr(i_sitekey).c_str(),
           XMLDocument::str2xmlstr(i_profile).c_str(),
           atoi(getResult(v_result, 2, 0, 0)),
           getResult(v_result, 2, 0, 1));
  XMFS::writeLine(pfh, v_line);
  read_DB_free(v_result);

  /* stats_levels */
  XMFS::writeLine(pfh, "<stats_levels>");
  v_result = readDB("SELECT dbSync, id_level, nbPlayed, nbDied, nbCompleted, "
                    "nbRestarted, playedTime, last_play_date "
                    "FROM stats_profiles_levels "
                    "WHERE sitekey=\"" +
                      protectString(i_sitekey) + "\" AND id_profile=\"" +
                      protectString(i_profile) +
                      "\" "
                      "AND synchronized=0;",
                    nrow);
  for (unsigned int i = 0; i < nrow; i++) {
    snprintf(
      v_line,
      2048,
      "<stats_level dbSync=\"%i\" id_level=\"%s\" nbPlayed=\"%i\" "
      "nbDied=\"%i\" nbCompleted=\"%i\" nbRestarted=\"%i\" playedTime=\"%i\" "
      "last_play_date=\"%s\" />",
      atoi(getResult(v_result, 8, i, 0)),
      XMLDocument::str2xmlstr(getResult(v_result, 8, i, 1)).c_str(),
      atoi(getResult(v_result, 8, i, 2)),
      atoi(getResult(v_result, 8, i, 3)),
      atoi(getResult(v_result, 8, i, 4)),
      atoi(getResult(v_result, 8, i, 5)),
      getResult(v_result, 8, i, 6) == NULL ? 0
                                           : atoi(getResult(v_result, 8, i, 6)),
      getResult(v_result, 8, i, 7) == NULL ? "" : getResult(v_result, 8, i, 7));
    XMFS::writeLine(pfh, v_line);
  }
  XMFS::writeLine(pfh, "</stats_levels>");

  /* stats_completedLevels */
  XMFS::writeLine(pfh, "<stats_completedLevels>");
  read_DB_free(v_result);
  v_result = readDB("SELECT dbSync, id_level, timeStamp, finishTime "
                    "FROM profile_completedLevels "
                    "WHERE sitekey=\"" +
                      protectString(i_sitekey) + "\" AND id_profile=\"" +
                      protectString(i_profile) +
                      "\" "
                      "AND synchronized=0;",
                    nrow);
  for (unsigned int i = 0; i < nrow; i++) {
    snprintf(v_line,
             2048,
             "<stats_completedLevel dbSync=\"%i\" id_level=\"%s\" "
             "timeStamp=\"%s\" finishTime=\"%i\" />",
             atoi(getResult(v_result, 4, i, 0)),
             XMLDocument::str2xmlstr(getResult(v_result, 4, i, 1)).c_str(),
             getResult(v_result, 4, i, 2),
             atoi(getResult(v_result, 4, i, 3)));
    XMFS::writeLine(pfh, v_line);
  }
  read_DB_free(v_result);
  XMFS::writeLine(pfh, "</stats_completedLevels>");

  XMFS::writeLine(pfh, "</xmoto_sync>");

  XMFS::closeFile(pfh);
}

void xmDatabase::setSynchronized() {
  simpleSql(
    "UPDATE profile_completedLevels SET synchronized=1 WHERE synchronized=0;");
  simpleSql(
    "UPDATE stats_profiles_levels   SET synchronized=1 WHERE synchronized=0;");
}

void xmDatabase::sync_updateDB(const std::string &i_profile,
                               const std::string &i_sitekey,
                               const std::string &i_file,
                               int i_newDbSyncServer) {
  XMLDocument v_xml;
  xmlNodePtr v_xmlElt;
  xmlNodePtr v_syncElt;

  std::string v_siteKey, v_id_level, v_timeStamp, v_finishTime;
  std::string v_nbPlayed, v_nbDied, v_nbCompleted, v_nbRestarted, v_playedTime,
    v_lastPlayDate, v_since, v_nbStarts;
  std::string v_tmp;

  int v_newDbSyncServer;

  try {
    simpleSql("BEGIN TRANSACTION;");

    /* open the file */
    v_xml.readFromFile(FDT_CACHE, i_file);

    v_xmlElt = v_xml.getRootNode("xmoto_uploadDbSyncResult");
    if (v_xmlElt == NULL) {
      throw Exception("Unable to analyze xml file");
    }

    v_syncElt = XMLDocument::subElement(v_xmlElt, "xmoto_sync");
    if (v_syncElt == NULL) {
      throw Exception("Unable to analyze xml file");
    }

    v_tmp = XMLDocument::getOption(v_syncElt, "dbSyncServer");
    if (v_tmp == "") {
      throw Exception("Invalid xml");
    }
    // get the last dbsyncserver
    v_newDbSyncServer = atoi(v_tmp.c_str());

    /* sites */
    xmlNodePtr v_xmlSitesElt = XMLDocument::subElement(v_syncElt, "sites");
    if (v_xmlSitesElt == NULL) {
      throw Exception("error : unable to analyze sync file");
    }

    for (xmlNodePtr pSubElem = XMLDocument::subElement(v_xmlSitesElt, "site");
         pSubElem != NULL;
         pSubElem = XMLDocument::nextElement(pSubElem)) {
      v_siteKey = XMLDocument::getOption(pSubElem, "sitekey");
      if (v_siteKey == "") {
        throw Exception("Invalid xml");
      }

      v_nbStarts = XMLDocument::getOption(pSubElem, "nbStarts");
      if (v_nbStarts == "") {
        throw Exception("Invalid xml");
      }

      v_since = XMLDocument::getOption(pSubElem, "since");
      if (v_since == "") {
        throw Exception("Invalid xml");
      }

      // add or update the level stats
      if (checkKey("SELECT COUNT(1) FROM stats_profiles WHERE sitekey=\"" +
                   protectString(v_siteKey) +
                   "\""
                   " AND id_profile=\"" +
                   protectString(i_profile) + "\";")) {
        // update
        simpleSql("UPDATE stats_profiles "
                  "SET nbStarts=\"" +
                  protectString(v_nbStarts) + "\", since=\"" +
                  protectString(v_since) + "\" WHERE sitekey=\"" +
                  protectString(v_siteKey) +
                  "\""
                  " AND id_profile=\"" +
                  protectString(i_profile) + "\";");

      } else {
        // insert
        simpleSql(
          "INSERT INTO stats_profiles (sitekey, id_profile, nbStarts, since)"
          " VALUES (\"" +
          protectString(v_siteKey) + "\", \"" + protectString(i_profile) +
          "\", \"" + protectString(v_nbStarts) + "\", \"" +
          protectString(v_since) + "\");");
      }
    }

    /* stats_completedLevels */
    xmlNodePtr v_xmlCLevelsElt =
      XMLDocument::subElement(v_syncElt, "stats_completedLevels");
    if (v_xmlCLevelsElt == NULL) {
      throw Exception("error : unable to analyze sync file");
    }

    for (xmlNodePtr pSubElem =
           XMLDocument::subElement(v_xmlCLevelsElt, "stats_completedLevel");
         pSubElem != NULL;
         pSubElem = XMLDocument::nextElement(pSubElem)) {
      v_siteKey = XMLDocument::getOption(pSubElem, "sitekey");
      if (v_siteKey == "") {
        throw Exception("Invalid xml");
      }

      v_id_level = XMLDocument::getOption(pSubElem, "id_level");
      if (v_id_level == "") {
        throw Exception("Invalid xml");
      }

      v_timeStamp = XMLDocument::getOption(pSubElem, "timeStamp");
      if (v_timeStamp == "") {
        throw Exception("Invalid xml");
      }

      v_finishTime = XMLDocument::getOption(pSubElem, "finishTime");
      if (v_finishTime == "") {
        throw Exception("Invalid xml");
      }

      // add the completed level
      simpleSql(
        "INSERT INTO profile_completedLevels(id_profile, id_level, "
        "timeStamp, finishTime, sitekey, synchronized, dbSync)"
        " VALUES (\"" +
        protectString(i_profile) + "\", \"" + protectString(v_id_level) +
        "\", "
        "\"" +
        protectString(v_timeStamp) + "\", \"" + protectString(v_finishTime) +
        "\", "
        "\"" +
        protectString(v_siteKey) + "\", 1, NULL);");
    }

    /* stats levels */
    xmlNodePtr v_xmlLevelsElt =
      XMLDocument::subElement(v_syncElt, "stats_levels");
    if (v_xmlLevelsElt == NULL) {
      throw Exception("error : unable to analyze sync file");
    }

    for (xmlNodePtr pSubElem =
           XMLDocument::subElement(v_xmlLevelsElt, "stats_level");
         pSubElem != NULL;
         pSubElem = XMLDocument::nextElement(pSubElem)) {
      v_siteKey = XMLDocument::getOption(pSubElem, "sitekey");
      if (v_siteKey == "") {
        throw Exception("Invalid xml");
      }

      v_id_level = XMLDocument::getOption(pSubElem, "id_level");
      if (v_id_level == "") {
        throw Exception("Invalid xml");
      }

      v_nbPlayed = XMLDocument::getOption(pSubElem, "nbPlayed");
      if (v_nbPlayed == "") {
        throw Exception("Invalid xml");
      }

      v_nbDied = XMLDocument::getOption(pSubElem, "nbDied");
      if (v_nbDied == "") {
        throw Exception("Invalid xml");
      }

      v_nbCompleted = XMLDocument::getOption(pSubElem, "nbCompleted");
      if (v_nbCompleted == "") {
        throw Exception("Invalid xml");
      }

      v_nbRestarted = XMLDocument::getOption(pSubElem, "nbRestarted");
      if (v_nbRestarted == "") {
        throw Exception("Invalid xml");
      }

      v_playedTime = XMLDocument::getOption(pSubElem, "playedTime");
      if (v_playedTime == "") {
        throw Exception("Invalid xml");
      }

      v_lastPlayDate = XMLDocument::getOption(pSubElem, "last_play_date");
      if (v_lastPlayDate == "") {
        throw Exception("Invalid xml");
      }

      // add or update the level stats
      if (checkKey(
            "SELECT COUNT(1) FROM stats_profiles_levels WHERE sitekey=\"" +
            protectString(v_siteKey) +
            "\""
            " AND id_profile=\"" +
            protectString(i_profile) + "\" AND id_level=\"" +
            protectString(v_id_level) + "\";")) {
        // update
        simpleSql("UPDATE stats_profiles_levels "
                  "SET nbPlayed=\"" +
                  protectString(v_nbPlayed) + "\", nbDied=\"" +
                  protectString(v_nbDied) + "\", nbCompleted=\"" +
                  protectString(v_nbCompleted) + "\", nbRestarted=\"" +
                  protectString(v_nbRestarted) + "\", playedTime=\"" +
                  protectString(v_playedTime) + "\", last_play_date=\"" +
                  protectString(v_lastPlayDate) +
                  "\""
                  "WHERE sitekey=\"" +
                  protectString(v_siteKey) +
                  "\""
                  " AND id_profile=\"" +
                  protectString(i_profile) + "\" AND id_level=\"" +
                  protectString(v_id_level) + "\";");

      } else {
        // insert
        simpleSql(
          "INSERT INTO stats_profiles_levels (sitekey, id_profile, id_level, "
          "nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime, "
          "last_play_date, synchronized, dbSync)"
          " VALUES (\"" +
          protectString(v_siteKey) + "\", \"" + protectString(i_profile) +
          "\", \"" + protectString(v_id_level) + "\", \"" +
          protectString(v_nbPlayed) + "\", \"" + protectString(v_nbDied) +
          "\", \"" + protectString(v_nbCompleted) + "\", \"" +
          protectString(v_nbRestarted) + "\", \"" +
          protectString(v_playedTime) + "\", \"" +
          protectString(v_lastPlayDate) + "\", 1, NULL);");
      }
    }

    XMSession::instance()->setDbSyncServer(this, i_profile, v_newDbSyncServer);

    simpleSql("COMMIT;");
  } catch (Exception &e) {
    simpleSql("ROLLBACK;");
    throw e;
  }
}
