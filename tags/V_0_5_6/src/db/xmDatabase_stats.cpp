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
#include "../helpers/VExcept.h"
#include "../VXml.h"
#include "../GameText.h"
#include "../Game.h"
#include <sstream>

/*
  IMPORTANT NOTE: this update is used ONLY when sitekey didn't exits !! 
  you must change the query to take case of it if you want to reuse it
*/
void xmDatabase::updateDB_stats(XmDatabaseUpdateInterface *i_interface) {
  XMLDocument XML;
  const char *tag, *tagv;
  std::string v_playerName, v_since, v_starts;
  std::string v_levelId, v_playedTime, v_played, v_died, v_restarted, v_completed;

  XML.readFromFile(FDT_DATA, "stats.xml", NULL);
  TiXmlDocument *pXML = XML.getLowLevelAccess();
        
  if(pXML != NULL) {    
    /* Start eating the XML */
    TiXmlElement *pStatsElem = pXML->FirstChildElement("stats");
    if(pStatsElem != NULL) {

      try {
     
	simpleSql("BEGIN TRANSACTION;");
 
	/* Get players */
	for(TiXmlElement *pPlayerElem = pStatsElem->FirstChildElement("player");
	    pPlayerElem!=NULL;pPlayerElem=pPlayerElem->NextSiblingElement("player")) {

	  tag = pPlayerElem->Attribute("name");

	  if(tag != NULL) {
	    v_playerName = tag;

	    tag = pPlayerElem->Attribute("since");
	    if(tag == NULL) {
	      v_since = "2000-01-01 00:00:00";
	    } else {
	      if(std::string(tag) == "") {
		v_since = "2000-01-01 00:00:00";
	      } else {
		std::string v_tmp = tag;
		v_since = v_tmp.substr(6, 10) + " " + v_tmp.substr(0, 5) + ":00";
	      }
	    }

	    v_starts = "";
	    for(TiXmlElement *pStatElem = pPlayerElem->FirstChildElement("stat");
		pStatElem!=NULL;pStatElem=pStatElem->NextSiblingElement("stat")) {
	      tag = pStatElem->Attribute("tag");
	      if(tag != NULL) {
		if(std::string(tag) == "xmotostarts") {
		  tag = pStatElem->Attribute("v");
		  if(tag != NULL) {
		    v_starts = tag;
		  }
		}
	      }
	    }

	    /* insert row */
	    simpleSql("INSERT INTO stats_profiles(id_profile, nbStarts, since) "
		      "VALUES(\"" + protectString(v_playerName) + "\", "
		      + v_starts + ", \"" + v_since + "\");");
	  
	    if(i_interface != NULL) {
	      i_interface->updatingDatabase(std::string(GAMETEXT_DB_UPGRADING_STATS_PROFILE)
					    + " " + v_playerName);
	    }

	    /* Read per-level stats */
	    for(TiXmlElement *pLevelElem = pPlayerElem->FirstChildElement("level");
		pLevelElem!=NULL;pLevelElem=pLevelElem->NextSiblingElement("level")) {

	      tag = pLevelElem->Attribute("id");
	      if(tag != NULL) {
		v_levelId = tag;

		v_playedTime = "0";
		v_played     = "0";
		v_died       = "0";
		v_restarted  = "0";
		v_completed  = "0";

		/* Read stats */
		for(TiXmlElement *pStatElem = pLevelElem->FirstChildElement("stat");
		    pStatElem!=NULL;pStatElem=pStatElem->NextSiblingElement("stat")) {
		  tag = pStatElem->Attribute("tag");
		  if(tag != NULL) {
		    tagv = pStatElem->Attribute("v");
		    if(tagv != NULL) {
		      if(std::string(tag)      == "playtime")  v_playedTime = tagv;
		      else if(std::string(tag) == "played")    v_played     = tagv;
		      else if(std::string(tag) == "died")      v_died       = tagv;
		      else if(std::string(tag) == "restarts")  v_restarted  = tagv;
		      else if(std::string(tag) == "completed") v_completed  = tagv;
		    }
		  }
		}

		/* insert row */
		simpleSql(std::string("INSERT INTO stats_profiles_levels("
				      "id_profile, id_level, nbPlayed, nbDied, nbCompleted,"
				      "nbRestarted, playedTime) VALUES(") + 
			  "\"" + protectString(v_playerName) + "\", " +
			  "\"" + protectString(v_levelId)    + "\", " +
			  v_played     + ", " +
			  v_died       + ", " +
			  v_completed  + ", " +
			  v_restarted  + ", " +
			  v_playedTime + "*100);");
	      }
	    }
	  }
	}

	simpleSql("COMMIT;");
      } catch(Exception &e) {
	simpleSql("ROLLBACK;");
	throw e;
      }
    }
  }
}

void xmDatabase::stats_createProfile(const std::string& i_sitekey, const std::string& i_profile) {
  simpleSql("INSERT INTO stats_profiles(sitekey, id_profile, nbStarts, since) "
	    "VALUES(\"" + protectString(i_sitekey) + "\", \"" + protectString(i_profile) + "\", 0, \"" + 
	    GameApp::getTimeStamp() + "\");");
}

void xmDatabase::stats_destroyProfile(const std::string& i_profile) {
  /* delete with all sitekeys */
  try {
    simpleSql("BEGIN TRANSACTION;");
    simpleSql("DELETE FROM stats_profiles          WHERE id_profile=\""+ protectString(i_profile) + "\";");
    simpleSql("DELETE FROM levels_favorite         WHERE id_profile=\""+ protectString(i_profile) + "\";");
    simpleSql("DELETE FROM profile_completedLevels WHERE id_profile=\""+ protectString(i_profile) + "\";");
    simpleSql("DELETE FROM stats_profiles_levels   WHERE id_profile=\""+ protectString(i_profile) + "\";");
    simpleSql("COMMIT;");
  } catch(Exception &e) {
    simpleSql("ROLLBACK;");
    throw e;
  }
}

bool xmDatabase::stats_checkKeyExists_stats_profiles(const std::string& i_sitekey, const std::string& i_profile) {
  return checkKey("SELECT count(1) FROM stats_profiles "
		  "WHERE sitekey=\""    + protectString(i_sitekey) + "\" "
		  "AND   id_profile=\"" + protectString(i_profile) + "\";");
}

bool xmDatabase::stats_checkKeyExists_stats_profiles_levels(const std::string& i_sitekey,
							    const std::string& i_profile,
							    const std::string& i_level) {
  return checkKey("SELECT count(1) FROM stats_profiles_levels "
		  "WHERE sitekey=\""      + protectString(i_sitekey) + "\" "
		  "AND   id_profile=\""   + protectString(i_profile) + "\" "
		  "AND   id_level=\""     + protectString(i_level)   + "\";");
}

void xmDatabase::stats_levelCompleted(const std::string& i_sitekey,
				      const std::string &PlayerName,
				      const std::string &LevelID,
				      int i_playTime) {
  //printf("stats: level completed\n");
  std::ostringstream v_playTime;
  v_playTime << i_playTime;

  if(stats_checkKeyExists_stats_profiles_levels(i_sitekey, PlayerName, LevelID)) {
    simpleSql("UPDATE stats_profiles_levels SET "
	      "nbCompleted=nbCompleted+1,"
	      "nbPlayed=nbPlayed+1,"
	      "playedTime=playedTime+" + v_playTime.str() + ","   +
	      "last_play_date=datetime('now', 'localtime'), "
	      "synchronized = 0 "
	      "WHERE sitekey=\""    + protectString(i_sitekey)        + "\" "
	      "AND id_profile=\""   + protectString(PlayerName)       + "\" "
	      "AND id_level=\""     + protectString(LevelID)          + "\";");
  } else {
    simpleSql("INSERT INTO stats_profiles_levels("
	      "sitekey, id_profile, id_level,"
	      "nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime, last_play_date, synchronized) "
	      "VALUES (\"" + protectString(i_sitekey) + "\",\"" + protectString(PlayerName) + "\", \"" + protectString(LevelID) +
	      "\", 1, 0, 1, 0, \"" + v_playTime.str() + "\", datetime('now', 'localtime'), 0);");
  }
}

void xmDatabase::stats_died(const std::string& i_sitekey,
			    const std::string &PlayerName,
			    const std::string &LevelID,
			    int i_playTime) {
  //printf("stats: level dead\n");
  std::ostringstream v_playTime;
  v_playTime << i_playTime;

  if(stats_checkKeyExists_stats_profiles_levels(i_sitekey, PlayerName, LevelID)) {
    simpleSql("UPDATE stats_profiles_levels SET "
	      "nbDied=nbDied+1,"
	      "nbPlayed=nbPlayed+1,"
	      "playedTime=playedTime+" + v_playTime.str() + ","   +
	      "last_play_date=datetime('now', 'localtime'), "
	      "synchronized = 0 "
	      "WHERE sitekey=\""    + protectString(i_sitekey)        + "\" "
	      "AND id_profile=\""   + protectString(PlayerName)       + "\" "
	      "AND id_level=\""        + protectString(LevelID)          + "\";");
  } else {
    simpleSql("INSERT INTO stats_profiles_levels("
	      "sitekey, id_profile, id_level,"
	      "nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime, last_play_date, synchronized) "
	      "VALUES (\"" + protectString(i_sitekey) + "\",\"" + protectString(PlayerName) + "\", \"" + protectString(LevelID) +
	      "\", 1, 1, 0, 0, \"" +
	      v_playTime.str() + "\", datetime('now', 'localtime'), 0);");
  }
}

void xmDatabase::stats_abortedLevel(const std::string& i_sitekey,
				    const std::string &PlayerName,
				    const std::string &LevelID,
				    int i_playTime) {
  //printf("stats: level aborted\n");
  std::ostringstream v_playTime;
  v_playTime << i_playTime;
  
  if(stats_checkKeyExists_stats_profiles_levels(i_sitekey, PlayerName, LevelID)) {
    simpleSql("UPDATE stats_profiles_levels SET "
	      "nbPlayed=nbPlayed+1,"
	      "playedTime=playedTime+" + v_playTime.str() + ","   +
	      "last_play_date=datetime('now', 'localtime'), "
	      "synchronized = 0 "
	      "WHERE sitekey=\""    + protectString(i_sitekey)        + "\" "
	      "AND id_profile=\""   + protectString(PlayerName)       + "\" "
	      "AND id_level=\""        + protectString(LevelID)          + "\";");
  } else {
    simpleSql("INSERT INTO stats_profiles_levels("
	      "sitekey, id_profile, id_level,"
	      "nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime, last_play_date, synchronized) "
	      "VALUES (\"" + protectString(i_sitekey) + "\",\"" + protectString(PlayerName) + "\", \"" + protectString(LevelID) +
	      "\", 1, 0, 0, 0, \"" +
	      v_playTime.str() + "\", datetime('now', 'localtime'), 0);");
  }
}

void xmDatabase::stats_levelRestarted(const std::string& i_sitekey,
				      const std::string &PlayerName,
				      const std::string &LevelID,
				      int i_playTime) {
  //printf("stats: level restarted\n");
  std::ostringstream v_playTime;
  v_playTime << i_playTime;

  if(stats_checkKeyExists_stats_profiles_levels(i_sitekey, PlayerName, LevelID)) {
    simpleSql("UPDATE stats_profiles_levels SET "
	      "nbRestarted=nbRestarted+1,"
	      "nbPlayed=nbPlayed+1,"
	      "playedTime=playedTime+" + v_playTime.str() + ","   +
	      "last_play_date=datetime('now', 'localtime'), "
	      "synchronized = 0 "
	      "WHERE sitekey=\""    + protectString(i_sitekey)        + "\" "
	      "AND id_profile=\""   + protectString(PlayerName)       + "\" "
	      "AND id_level=\""        + protectString(LevelID)          + "\";");
  } else {
    simpleSql("INSERT INTO stats_profiles_levels("
	      "sitekey, id_profile, id_level,"
	      "nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime, last_play_date, synchronized) "
	      "VALUES (\"" + protectString(i_sitekey) + "\",\"" + protectString(PlayerName) + "\", \"" + protectString(LevelID) +
	      "\", 1, 0, 0, 1, \"" +
	      v_playTime.str() + "\", datetime('now', 'localtime'), 0);");
  }
}

void xmDatabase::stats_xmotoStarted(const std::string& i_sitekey, const std::string &PlayerName) {
  simpleSql("UPDATE stats_profiles SET "
	    "nbStarts=nbStarts+1 "
	    "WHERE sitekey=\"" + i_sitekey + "\" AND id_profile=\"" + protectString(PlayerName) + "\";");
}
