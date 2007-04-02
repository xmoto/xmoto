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
#include "VXml.h"
#include "GameText.h"
#include "VApp.h"
#include "GUI.h"

void xmDatabase::updateDB_stats(XmDatabaseUpdateInterface *i_interface) {
  vapp::XMLDocument XML;
  const char *tag, *tagv;
  std::string v_playerName, v_since, v_starts;
  std::string v_levelId, v_playedTime, v_played, v_died, v_restarted, v_completed;

  XML.readFromFile("stats.xml", NULL);
  TiXmlDocument *pXML = XML.getLowLevelAccess();
        
  if(pXML != NULL) {    
    /* Start eating the XML */
    TiXmlElement *pStatsElem = pXML->FirstChildElement("stats");
    if(pStatsElem != NULL) {
      
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
	    std::string v_tmp = tag;
	    v_since = v_tmp.substr(6, 10) + " " + v_tmp.substr(0, 5) + ":00";
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
		    "VALUES(\"" + v_playerName + "\", "
		    + v_starts + ", \"" + v_since + "\");");
	  
	  /* Read per-level stats */
	  int nb = 0;
	  for(TiXmlElement *pLevelElem = pPlayerElem->FirstChildElement("level");
	      pLevelElem!=NULL;pLevelElem=pLevelElem->NextSiblingElement("level")) {

	    tag = pLevelElem->Attribute("id");
	    if(tag != NULL) {
	      v_levelId = tag;

	      v_playedTime = "0.0";
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
			"\"" + v_playerName + "\", " +
			"\"" + v_levelId    + "\", " +
			v_played     + ", " +
			v_died       + ", " +
			v_completed  + ", " +
			v_restarted  + ", " +
			v_playedTime + ");");

	      if(i_interface != NULL) {
		std::ostringstream v_nb;
		v_nb << nb;
		i_interface->updatingDatabase(std::string(GAMETEXT_DB_UPGRADING_STATS_PROFILE)
					      + " " + v_playerName + " " + v_nb.str() + " data");
	      }
	      nb++;
	    }
	  }
	}
      }
    }
  }
}

bool xmDatabase::stats_checkKeyExists_stats_profiles(std::string i_profile) {
  return checkKey("SELECT count(1) FROM stats_profiles "
		  "WHERE id_profile=\"" + i_profile + "\";");
}

bool xmDatabase::stats_checkKeyExists_stats_profiles_levels(std::string i_profile, std::string i_level) {
  return checkKey("SELECT count(1) FROM stats_profiles_levels "
		  "WHERE id_profile=\"" + i_profile + "\" " +
		  "AND id_level=\""     + i_level   + "\";");
}

void xmDatabase::stats_levelCompleted(const std::string &PlayerName,
				      const std::string &LevelID,
				      float fPlayTime) {
  std::ostringstream v_playTime;
  v_playTime << fPlayTime;

  if(stats_checkKeyExists_stats_profiles_levels(PlayerName, LevelID)) {
    simpleSql("UPDATE stats_profiles_levels SET "
	      "nbCompleted=nbCompleted+1,"
	      "nbPlayed=nbPlayed+1,"
	      "playedTime=playedTime+" + v_playTime.str() + " "   +
	      "WHERE id_profile=\""    + PlayerName       + "\" " +
	      "AND id_level=\""        + LevelID          + "\";");
  } else {
    simpleSql("INSERT INTO stats_profiles_levels("
	      "id_profile, id_level,"
	      "nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime) "
	      "VALUES (\"" + PlayerName + "\", \"" + LevelID + "\", 1, 0, 1, 0, \"" +
	      v_playTime.str() + "\");");
  }
}

void xmDatabase::stats_died(const std::string &PlayerName,
			    const std::string &LevelID,
			    float fPlayTime) {
  std::ostringstream v_playTime;
  v_playTime << fPlayTime;

  if(stats_checkKeyExists_stats_profiles_levels(PlayerName, LevelID)) {
    simpleSql("UPDATE stats_profiles_levels SET "
	      "nbDied=nbDied+1,"
	      "nbPlayed=nbPlayed+1,"
	      "playedTime=playedTime+" + v_playTime.str() + " "   +
	      "WHERE id_profile=\""    + PlayerName       + "\" " +
	      "AND id_level=\""        + LevelID          + "\";");
  } else {
    simpleSql("INSERT INTO stats_profiles_levels("
	      "id_profile, id_level,"
	      "nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime) "
	      "VALUES (\"" + PlayerName + "\", \"" + LevelID + "\", 1, 1, 0, 0, \"" +
	      v_playTime.str() + "\");");
  }
}

void xmDatabase::stats_abortedLevel(const std::string &PlayerName,
				    const std::string &LevelID,
				    float fPlayTime) {
  std::ostringstream v_playTime;
  v_playTime << fPlayTime;
  
  if(stats_checkKeyExists_stats_profiles_levels(PlayerName, LevelID)) {
    simpleSql("UPDATE stats_profiles_levels SET "
	      "nbPlayed=nbPlayed+1,"
	      "playedTime=playedTime+" + v_playTime.str() + " "   +
	      "WHERE id_profile=\""    + PlayerName       + "\" " +
	      "AND id_level=\""        + LevelID          + "\";");
  } else {
    simpleSql("INSERT INTO stats_profiles_levels("
	      "id_profile, id_level,"
	      "nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime) "
	      "VALUES (\"" + PlayerName + "\", \"" + LevelID + "\", 1, 0, 0, 0, \"" +
	      v_playTime.str() + "\");");
  }
}

void xmDatabase::stats_levelRestarted(const std::string &PlayerName,
				      const std::string &LevelID,
				      float fPlayTime) {
  std::ostringstream v_playTime;
  v_playTime << fPlayTime;

  if(stats_checkKeyExists_stats_profiles_levels(PlayerName, LevelID)) {
    simpleSql("UPDATE stats_profiles_levels SET "
	      "nbRestarted=nbRestarted+1,"
	      "nbPlayed=nbPlayed+1,"
	      "playedTime=playedTime+" + v_playTime.str() + " "   +
	      "WHERE id_profile=\""    + PlayerName       + "\" " +
	      "AND id_level=\""        + LevelID          + "\";");
  } else {
    simpleSql("INSERT INTO stats_profiles_levels("
	      "id_profile, id_level,"
	      "nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime) "
	      "VALUES (\"" + PlayerName + "\", \"" + LevelID + "\", 1, 0, 0, 1, \"" +
	      v_playTime.str() + "\");");
  }
}

void xmDatabase::stats_xmotoStarted(const std::string &PlayerName) {
  if(stats_checkKeyExists_stats_profiles(PlayerName)) {
    simpleSql("UPDATE stats_profiles SET "
	      "nbStarts=nbStarts+1 "
	      "WHERE id_profile=\"" + PlayerName + "\";");
  } else {
    /* Note this time */
    time_t ATime;               
    time(&ATime);
    struct tm *ptm = localtime(&ATime);
    char cTemp[20];
    std::string v_since;

    sprintf(cTemp,"%04d-%02d-%02d %02d:%02d:%02d",
	    1900+ptm->tm_year, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);  
    v_since = cTemp;

    simpleSql("INSERT INTO stats_profiles(id_profile, nbStarts, since) "
	      "VALUES(\"" + PlayerName + "\", 0, \"" + v_since + "\");");
  }
}

vapp::UIWindow* xmDatabase::stats_generateReport(const std::string &PlayerName,
						 vapp::UIWindow *pParent,
						 int x, int y, int nWidth, int nHeight,
						 vapp::UIFont *pFont) {
  /* Create stats window */
  vapp::UIWindow *p;
  char **v_result;
  int nrow, ncolumn;
  char *errMsg;
  std::string v_errMsg;

  int   v_nbStarts        = 0;
  std::string v_since     = "";
  float v_totalPlayedTime = 0.0;
  int   v_nbPlayed        = 0;
  int   v_nbDied          = 0;
  int   v_nbCompleted     = 0;
  int   v_nbRestarted     = 0;
  int   v_nbDiffLevels    = 0;
  
  p = new vapp::UIWindow(pParent, x, y, "", nWidth, nHeight);
  
  if(sqlite3_get_table(m_db,
		       std::string("SELECT a.nbStarts, a.since, SUM(b.playedTime), "
				   "SUM(b.nbPlayed), SUM(b.nbDied), SUM(b.nbCompleted), "
				   "SUM(b.nbRestarted), count(b.id_level) "
				   "FROM stats_profiles AS a INNER JOIN stats_profiles_levels AS b "
				   "ON a.id_profile=b.id_profile "
				   "WHERE a.id_profile=\"" + PlayerName + "\" "
				   "GROUP BY a.id_profile;").c_str(),
		       &v_result, &nrow, &ncolumn, &errMsg)
     != SQLITE_OK) {
    v_errMsg = errMsg;
    sqlite3_free(errMsg);
    throw Exception("xmDb: " + v_errMsg);
  }
  v_nbStarts        = atoi(v_result[8]);
  v_since           = v_result[9];
  v_totalPlayedTime = atof(v_result[10]);
  v_nbPlayed        = atoi(v_result[11]);
  v_nbDied          = atoi(v_result[12]);
  v_nbCompleted     = atoi(v_result[13]);
  v_nbRestarted     = atoi(v_result[14]);
  v_nbDiffLevels    = atoi(v_result[15]);

  sqlite3_free_table(v_result);

  /* Per-player info */
  char cBuf[512];
  char cTime[512];
  int nHours = ((int)v_totalPlayedTime) / (60 * 60);
  int nMinutes = (((int)v_totalPlayedTime) / (60)) - nHours*60;
  int nSeconds = (((int)v_totalPlayedTime)) - nMinutes*60 - nHours*3600;
  if(nHours > 0) sprintf(cTime,(std::string(GAMETEXT_XHOURS) + ", " + std::string(GAMETEXT_XMINUTES) + ", " + std::string(GAMETEXT_AND) + " " + std::string(GAMETEXT_XSECONDS)).c_str(),nHours,nMinutes,nSeconds);
  else if(nMinutes > 0) sprintf(cTime,(std::string(GAMETEXT_XMINUTES) +  " " + std::string(GAMETEXT_AND) +  " " + std::string(GAMETEXT_XSECONDS)).c_str(),nMinutes,nSeconds);
  else sprintf(cTime,GAMETEXT_XSECONDS,nSeconds);
  
  sprintf(cBuf,GAMETEXT_XMOTOGLOBALSTATS,      
	  v_since.c_str(), v_nbStarts, v_nbPlayed, v_nbDiffLevels,
	  v_nbDied, v_nbCompleted, v_nbRestarted, cTime);                           

  vapp::UIStatic *pText = new vapp::UIStatic(p, 0, 0, cBuf, nWidth, 80);
  pText->setHAlign(vapp::UI_ALIGN_LEFT);
  pText->setTextSolidColor(MAKE_COLOR(255,255,0,255));
  pText->setFont(pFont);


  /* Per-level stats */      
  pText = new vapp::UIStatic(p,0,90, std::string(GAMETEXT_MOSTPLAYEDLEVELSFOLLOW) + ":",nWidth,20);
  pText->setHAlign(vapp::UI_ALIGN_LEFT);
  pText->setTextSolidColor(MAKE_COLOR(255,255,0,255));
  pText->setFont(pFont);      

  //debugResult(v_result, ncolumn, nrow);
  //std::vector<LevelStats *> Levels = _GetOrderedLevels(ORDER_BY_PLAYTIME,PlayerName);
  //int cy = 110;
  //for(int i=0;i<Levels.size();i++) {
  //  if(cy + 45 > nHeight) break; /* out of window */
  //  
  //  sprintf(cBuf,("[%s] %s:\n   " + std::string(GAMETEXT_XMOTOLEVELSTATS)).c_str(),
  //	      App::formatTime(Levels[i]->fPlayTime).c_str(),Levels[i]->LevelName.c_str(),
  //	      Levels[i]->nPlayed,Levels[i]->nDied,Levels[i]->nCompleted,Levels[i]->nRestarts);
  //  
  //  pText = new UIStatic(p,0,cy,cBuf,nWidth,45);
  //  pText->setHAlign(UI_ALIGN_LEFT);        
  //  pText->setTextSolidColor(MAKE_COLOR(255,255,0,255));
  //  pText->setFont(pFont);
  //  
  //  cy += 45;
  //}
  
  vapp::UIButton *pUpdateButton = new vapp::UIButton(p,nWidth-115,nHeight-57,GAMETEXT_UPDATE,115,57);
  pUpdateButton->setContextHelp(CONTEXTHELP_UPDATESTATS);
  pUpdateButton->setFont(pFont);
  pUpdateButton->setType(vapp::UI_BUTTON_TYPE_SMALL);
  pUpdateButton->setID("UPDATE_BUTTON");
  return p;
}

