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
#include "../VFileIO.h"
#include "../VXml.h"
#include "../XMSession.h"
#include "../helpers/VExcept.h"
#include <sstream>

void xmDatabase::sync_buildServerFile(const std::string& i_outFile, const std::string& i_sitekey, const std::string& i_profile) {
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
    XMSession::instance()->setDbSync(this, i_profile, XMSession::instance()->dbSync(this, i_profile) +1);
    v_lastDbSync << XMSession::instance()->dbSync(this, i_profile);

    simpleSql("UPDATE profile_completedLevels SET dbSync=\"" + v_lastDbSync.str() + "\" WHERE synchronized=0;");
    simpleSql("UPDATE stats_profiles_levels   SET dbSync=\"" + v_lastDbSync.str() + "\" WHERE synchronized=0;");
    /* ***** */

    if(pfh == NULL) {
      throw Exception("Unable to open " + i_outFile);
    }

    XMFS::writeLine(pfh,"<?xml version=\"1.0\" encoding=\"utf-8\"?>");

    /* header */
    v_result = readDB("SELECT nbStarts, since "
		      "FROM stats_profiles "
		      "WHERE sitekey=\"" + protectString(i_sitekey) + "\" AND id_profile=\"" + protectString(i_profile) + "\";", nrow);
    if(nrow != 1) {
      read_DB_free(v_result);
      throw Exception("Unable to retrieve informations");
    }

    snprintf(v_line, 2048,
	     "<xmoto_sync fileformat=\"1\" sitekey=\"%s\" profile=\"%s\" nbStarts=\"%i\" since=\"%s\">",
	     XML::str2xmlstr(i_sitekey).c_str(), XML::str2xmlstr(i_profile).c_str(),
	     atoi(getResult(v_result, 2, 0, 0)), getResult(v_result, 2, 0, 1));
    XMFS::writeLine(pfh, v_line);
    read_DB_free(v_result);
 
    /* stats_levels */
    XMFS::writeLine(pfh, "<stats_levels>");
    v_result = readDB("SELECT dbSync, id_level, nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime, last_play_date "
		      "FROM stats_profiles_levels "
		      "WHERE sitekey=\"" + protectString(i_sitekey) + "\" AND id_profile=\"" + protectString(i_profile) + "\" "
		      "AND synchronized=0;", nrow);
    for(unsigned int i=0; i<nrow; i++) {
      snprintf(v_line, 2048, "<stats_level dbSync=\"%i\" id_level=\"%s\" nbPlayed=\"%i\" nbDied=\"%i\" nbCompleted=\"%i\" nbRestarted=\"%i\" playedTime=\"%i\" last_play_date=\"%s\" />",
	       atoi(getResult(v_result, 8, i, 0)),
	       XML::str2xmlstr(getResult(v_result, 8, i, 1)).c_str(),
	       atoi(getResult(v_result, 8, i, 2)), atoi(getResult(v_result, 8, i, 3)),
	       atoi(getResult(v_result, 8, i, 4)), atoi(getResult(v_result, 8, i, 5)),
	       getResult(v_result, 8, i, 6) == NULL ? 0 : atoi(getResult(v_result, 8, i, 6)),
	       getResult(v_result, 8, i, 7) == NULL ? "" : getResult(v_result, 8, i, 7));
      XMFS::writeLine(pfh, v_line);
    }
    XMFS::writeLine(pfh, "</stats_levels>");

    /* stats_completedLevels */
    XMFS::writeLine(pfh, "<stats_completedLevels>");
    v_result = readDB("SELECT dbSync, id_level, timeStamp, finishTime "
		      "FROM profile_completedLevels "
		      "WHERE sitekey=\"" + protectString(i_sitekey) + "\" AND id_profile=\"" + protectString(i_profile) + "\" "
		      "AND synchronized=0;", nrow);
    for(unsigned int i=0; i<nrow; i++) {
      snprintf(v_line, 2048, "<stats_completedLevel dbSync=\"%i\" id_level=\"%s\" timeStamp=\"%s\" finishTime=\"%i\" />",
	       atoi(getResult(v_result, 4, i, 0)),
	       XML::str2xmlstr(getResult(v_result, 4, i, 1)).c_str(),
	       getResult(v_result, 4, i, 2), atoi(getResult(v_result, 4, i, 3)));
      XMFS::writeLine(pfh, v_line);
    }
    XMFS::writeLine(pfh, "</stats_completedLevels>");

    XMFS::writeLine(pfh, "</xmoto_sync>");

    XMFS::closeFile(pfh);
  }

void xmDatabase::setSynchronized() {
  simpleSql("UPDATE profile_completedLevels SET synchronized=1 WHERE synchronized=0;");    
  simpleSql("UPDATE stats_profiles_levels   SET synchronized=1 WHERE synchronized=0;");    
}

void xmDatabase::sync_updateDB(const std::string& i_profile, const std::string& i_sitekey, const std::string& i_file, int i_newDbSyncServer) {

  XMLDocument v_syncLXml;
  TiXmlDocument *v_syncLXmlData;
  TiXmlElement *v_syncLXmlDataElement, *v_syncLXmlDataElement2, *v_syncLXmlDataElement3;
  const char *pc;
  TiXmlElement *pVarElem;

  std::string v_siteKey, v_id_level, v_timeStamp, v_finishTime;
  std::string v_nbPlayed, v_nbDied, v_nbCompleted, v_nbRestarted, v_playedTime, v_lastPlayDate, v_since, v_nbStarts;

  int v_newDbSyncServer;

  try {
    simpleSql("BEGIN TRANSACTION;");

    v_syncLXml.readFromFile(FDT_CACHE, i_file);
    v_syncLXmlData = v_syncLXml.getLowLevelAccess();

    if(v_syncLXmlData == NULL) {
      throw Exception("error : unable to analyze sync file");
    }

    v_syncLXmlDataElement = v_syncLXmlData->FirstChildElement("xmoto_uploadDbSyncResult");
  
    if(v_syncLXmlDataElement == NULL) {
      throw Exception("error : unable to analyze sync file");
    }

    v_syncLXmlDataElement2 = v_syncLXmlDataElement->FirstChildElement("xmoto_sync");
  
    if(v_syncLXmlDataElement2 == NULL) {
      throw Exception("error : unable to analyze sync file");
    }

    pc = v_syncLXmlDataElement2->Attribute("dbSyncServer");
    if(pc == NULL) {
      throw Exception("Invalid xml");
    }
    // get the last dbsyncserver
    v_newDbSyncServer = atoi(pc);

    /* sites */
    v_syncLXmlDataElement3 = v_syncLXmlDataElement2->FirstChildElement("sites");
  
    if(v_syncLXmlDataElement3 == NULL) {
      throw Exception("error : unable to analyze sync file");
    }

    pVarElem = v_syncLXmlDataElement3->FirstChildElement("site");
    while(pVarElem != NULL) {

      pc = pVarElem->Attribute("sitekey");
      if(pc == NULL) {
	throw Exception("Invalid xml");
      }
      v_siteKey = pc;

      pc = pVarElem->Attribute("nbStarts");
      if(pc == NULL) {
	throw Exception("Invalid xml");
      }
      v_nbStarts = pc;

      pc = pVarElem->Attribute("since");
      if(pc == NULL) {
	throw Exception("Invalid xml");
      }
      v_since = pc;

      // add or update the level stats
      if(checkKey("SELECT COUNT(1) FROM stats_profiles WHERE sitekey=\"" + protectString(v_siteKey) + "\""
		  " AND id_profile=\"" + protectString(i_profile) + "\";")) {
	// update
	simpleSql("UPDATE stats_profiles "
		  "SET nbStarts=\"" + protectString(v_nbStarts) +
		  "\", since=\"" + protectString(v_since) +
		  "\" WHERE sitekey=\"" + protectString(v_siteKey) + "\""
		  " AND id_profile=\"" + protectString(i_profile) + "\";");

      } else {
	// insert
	simpleSql("INSERT INTO stats_profiles (sitekey, id_profile, nbStarts, since)"
		  " VALUES (\"" + protectString(v_siteKey) + "\", \"" + protectString(i_profile) + "\", \"" + protectString(v_nbStarts) + "\", \"" + protectString(v_since) + "\");");
      }

      pVarElem = pVarElem->NextSiblingElement("site");
    }

    // stats_completedLevels
    v_syncLXmlDataElement3 = v_syncLXmlDataElement2->FirstChildElement("stats_completedLevels");
  
    if(v_syncLXmlDataElement3 == NULL) {
      throw Exception("error : unable to analyze sync file");
    }

    pVarElem = v_syncLXmlDataElement3->FirstChildElement("stats_completedLevel");
    while(pVarElem != NULL) {

      pc = pVarElem->Attribute("sitekey");
      if(pc == NULL) {
	throw Exception("Invalid xml");
      }
      v_siteKey = pc;

      pc = pVarElem->Attribute("id_level");
      if(pc == NULL) {
	throw Exception("Invalid xml");
      }
      v_id_level = pc;

      pc = pVarElem->Attribute("timeStamp");
      if(pc == NULL) {
	throw Exception("Invalid xml");
      }
      v_timeStamp = pc;

      pc = pVarElem->Attribute("finishTime");
      if(pc == NULL) {
	throw Exception("Invalid xml");
      }
      v_finishTime = pc;

      // add the completed level
      simpleSql("INSERT INTO profile_completedLevels(id_profile, id_level, timeStamp, finishTime, sitekey, synchronized, dbSync)"
		" VALUES (\"" + protectString(i_profile) + "\", \"" + protectString(v_id_level) + "\", "
		"\"" + protectString(v_timeStamp) + "\", \"" + protectString(v_finishTime) + "\", "
		"\"" + protectString(v_siteKey) + "\", 1, NULL);");

      pVarElem = pVarElem->NextSiblingElement("stats_completedLevel");
    }


    /* stats levels */
    v_syncLXmlDataElement3 = v_syncLXmlDataElement2->FirstChildElement("stats_levels");
  
    if(v_syncLXmlDataElement3 == NULL) {
      throw Exception("error : unable to analyze sync file");
    }

    pVarElem = v_syncLXmlDataElement3->FirstChildElement("stats_level");
    while(pVarElem != NULL) {

      pc = pVarElem->Attribute("sitekey");
      if(pc == NULL) {
	throw Exception("Invalid xml");
      }
      v_siteKey = pc;

      pc = pVarElem->Attribute("id_level");
      if(pc == NULL) {
	throw Exception("Invalid xml");
      }
      v_id_level = pc;

      pc = pVarElem->Attribute("nbPlayed");
      if(pc == NULL) {
	throw Exception("Invalid xml");
      }
      v_nbPlayed = pc;

      pc = pVarElem->Attribute("nbDied");
      if(pc == NULL) {
	throw Exception("Invalid xml");
      }
      v_nbDied = pc;

      pc = pVarElem->Attribute("nbCompleted");
      if(pc == NULL) {
	throw Exception("Invalid xml");
      }
      v_nbCompleted = pc;

      pc = pVarElem->Attribute("nbRestarted");
      if(pc == NULL) {
	throw Exception("Invalid xml");
      }
      v_nbRestarted = pc;

      pc = pVarElem->Attribute("playedTime");
      if(pc == NULL) {
	throw Exception("Invalid xml");
      }
      v_playedTime = pc;

      pc = pVarElem->Attribute("last_play_date");
      if(pc == NULL) {
	throw Exception("Invalid xml");
      }
      v_lastPlayDate = pc;

      // add or update the level stats
      if(checkKey("SELECT COUNT(1) FROM stats_profiles_levels WHERE sitekey=\"" + protectString(v_siteKey) + "\""
		  " AND id_profile=\"" + protectString(i_profile) + "\" AND id_level=\"" + protectString(v_id_level) + "\";")) {
	// update
	simpleSql("UPDATE stats_profiles_levels "
		  "SET nbPlayed=\"" + protectString(v_nbPlayed) + "\", nbDied=\"" + protectString(v_nbDied) +
		  "\", nbCompleted=\"" + protectString(v_nbCompleted) + "\", nbRestarted=\"" + protectString(v_nbRestarted) +
		  "\", playedTime=\"" + protectString(v_playedTime) + "\", last_play_date=\"" + protectString(v_lastPlayDate) + "\""
		  "WHERE sitekey=\"" + protectString(v_siteKey) + "\""
		  " AND id_profile=\"" + protectString(i_profile) + "\" AND id_level=\"" + protectString(v_id_level) + "\";");

      } else {
	// insert
	simpleSql("INSERT INTO stats_profiles_levels (sitekey, id_profile, id_level, nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime, last_play_date, synchronized, dbSync)"
		  " VALUES (\"" + protectString(v_siteKey) + "\", \"" + protectString(i_profile) + "\", \"" + protectString(v_id_level) + "\", \"" + protectString(v_nbPlayed) + "\", \"" + protectString(v_nbDied) + "\", \"" + protectString(v_nbCompleted) + "\", \"" + protectString(v_nbRestarted) + "\", \"" + protectString(v_playedTime) + "\", \"" + protectString(v_lastPlayDate) + "\", 1, NULL);");
      }

      pVarElem = pVarElem->NextSiblingElement("stats_level");
    }

    XMSession::instance()->setDbSyncServer(this, i_profile, v_newDbSyncServer);

    simpleSql("COMMIT;");
  } catch(Exception &e) {
    simpleSql("ROLLBACK;");
    throw e;
  }

}
