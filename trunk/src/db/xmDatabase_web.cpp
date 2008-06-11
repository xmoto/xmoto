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
#include "helpers/VExcept.h"
#include "helpers/Log.h"
#include "WWW.h"

bool xmDatabase::webrooms_checkKeyExists_id_room(const std::string& i_id_room) {
  return checkKey("SELECT count(1) FROM webrooms "
		  "WHERE id_room=" + i_id_room + ";");
}

void xmDatabase::webrooms_addRoom(const std::string& i_id_room, const std::string& i_name,
  const std::string& i_highscoreUrl) {
  simpleSql("INSERT INTO webrooms("
	    "id_room,  name, highscoresUrl) "
	    "VALUES(" + i_id_room + ", " +
	    "\"" + protectString(i_name)         + "\", " +
	    "\"" + protectString(i_highscoreUrl) + "\");");
}

std::string xmDatabase::webhighscores_updateDB(const std::string& i_webhighscoresFile,
					       const std::string& i_websource) {
  XMLDocument v_webHSXml;
  TiXmlDocument *v_webHSXmlData;
  TiXmlElement *v_webHSXmlDataElement;
  const char *pc;
  std::string v_roomName;
  std::string v_roomId;
  std::string v_levelId;
  std::string v_player;
  std::string v_rplUrl;
  std::string v_strtime;
  std::string v_date;
  int v_time;
  unsigned int pos_1, pos_2;

  try {
    simpleSql("BEGIN TRANSACTION;");

    v_webHSXml.readFromFile(i_webhighscoresFile);
    v_webHSXmlData = v_webHSXml.getLowLevelAccess();

    if(v_webHSXmlData == NULL) {
      throw Exception("error : unable to analyze xml highscore file");
    }

    v_webHSXmlDataElement = v_webHSXmlData->FirstChildElement("xmoto_worldrecords");
    if(v_webHSXmlDataElement == NULL) {
      throw Exception("error : unable to analyze xml highscore file");
    }

    /* get Room informations */
    pc = v_webHSXmlDataElement->Attribute("roomname");
    if(pc != NULL) {
      v_roomName = pc;
    }
    pc = v_webHSXmlDataElement->Attribute("roomid");
    if(pc != NULL) {
      v_roomId = pc;
    }
    if(v_roomId == "") {
      throw Exception("error : unable to analyze xml highscore file");
    }

    /* be sure the room is into the webrooms table */
    if(webrooms_checkKeyExists_id_room(v_roomId) == false) {
      webrooms_addRoom(v_roomId, v_roomName, i_websource);
    }

    simpleSql("DELETE FROM webhighscores WHERE id_room=" + v_roomId + ";");
    for(TiXmlElement *pVarElem = v_webHSXmlDataElement->FirstChildElement("worldrecord");
	pVarElem!=NULL;
	pVarElem = pVarElem->NextSiblingElement("worldrecord")
	) {
      
      pc = pVarElem->Attribute("level_id");
      if(pc == NULL) {
	continue;
      }
      v_levelId = pc;

      pc = pVarElem->Attribute("player");
      if(pc == NULL) {
	continue;
      }
      v_player = pc;

      /* time */
      pc = pVarElem->Attribute("time");
      if(pc == NULL) {
	continue;
      }
      v_strtime = pc;

      pos_1 = v_strtime.find(":", 0);
      if(pos_1 == std::string::npos || pos_1 == v_strtime.length()-1)
	continue;
      pos_2 = v_strtime.find(":", pos_1+1);
      if(pos_2 == std::string::npos)
	continue;

      v_time =
	atoi(v_strtime.substr(0, pos_1).c_str())               * 6000 +
	atoi(v_strtime.substr(pos_1+1, pos_2-pos_1-1).c_str()) * 100  +
        atoi(v_strtime.substr(pos_2+1, v_strtime.length()-pos_2-1).c_str());

      /* replay */
      pc = pVarElem->Attribute("replay");
      if(pc == NULL) {
	continue;
      }
      v_rplUrl = pc;

      /* date */
      pc = pVarElem->Attribute("date");
      if(pc == NULL) {
	continue;
      }
      v_date = pc;

      std::ostringstream v_secondTime;
      v_secondTime << v_time;

      simpleSql("INSERT INTO webhighscores(id_room, id_level, id_profile, finishTime, date, fileUrl)"
		"VALUES("   + v_roomId   + ", \"" +
		protectString(v_levelId) + "\", \"" +
		protectString(v_player)  + "\", "   +
		v_secondTime.str()       + ", \""   +
		v_date                   + "\", \"" +
		protectString(v_rplUrl)  + "\");");
    }
    simpleSql("COMMIT;");
  } catch(Exception &e) {
    simpleSql("ROLLBACK;");
    throw e;
  }
  return v_roomId;
}

std::string xmDatabase::webrooms_getName(const std::string& i_id_room) {
  char **v_result;
  unsigned int nrow;
  std::string v_res;

  v_result = readDB("SELECT name FROM webrooms WHERE id_room=" + i_id_room + ";",
		    nrow);
  if(nrow != 1) {
    read_DB_free(v_result);
    return ""; /* not found */
  }
  v_res = getResult(v_result, 1, 0, 0);

  read_DB_free(v_result);
  return v_res;
}

int xmDatabase::webrooms_getHighscoreTime(const std::string& i_id_room,
					  const std::string& i_id_level) {
  char **v_result;
  unsigned int nrow;
  std::string v_res;

  v_result = readDB("SELECT finishTime FROM webhighscores "
		    "WHERE id_room="  + i_id_room +
		    " AND id_level=\"" + protectString(i_id_level) + "\";",
		    nrow);
  if(nrow != 1) {
    read_DB_free(v_result);
    return -1; /* not found */
  }
  v_res = getResult(v_result, 1, 0, 0);

  read_DB_free(v_result);
  return atoi(v_res.c_str());
}

void xmDatabase::weblevels_updateDB(const std::string& i_weblevelsFile) {
  XMLDocument v_webLXml;
  TiXmlDocument *v_webLXmlData;
  TiXmlElement *v_webLXmlDataElement;
  const char *pc;
  std::string v_levelId, v_levelName, v_url, v_MD5sum_web;
  std::string v_difficulty, v_quality, v_creationDate;
  std::string v_crappy, v_children_compliant;

  try {
    simpleSql("BEGIN TRANSACTION;");
    simpleSql("DELETE FROM weblevels;");

    v_webLXml.readFromFile(i_weblevelsFile);
    v_webLXmlData = v_webLXml.getLowLevelAccess();

    if(v_webLXmlData == NULL) {
      throw Exception("error : unable to analyze xml level file");
    }

    v_webLXmlDataElement = v_webLXmlData->FirstChildElement("xmoto_levels");
  
    if(v_webLXmlDataElement == NULL) {
      throw Exception("error : unable to analyze xml level file");
    }

    TiXmlElement *pVarElem = v_webLXmlDataElement->FirstChildElement("level");
    while(pVarElem != NULL) {
    
      pc = pVarElem->Attribute("level_id");
      if(pc == NULL)
	continue;
      v_levelId = pc;
	
      pc = pVarElem->Attribute("name");
      if(pc == NULL)
	continue;
      v_levelName = pc;
	  
      pc = pVarElem->Attribute("url");
      if(pc == NULL)
	continue;
      v_url = pc;  
	    
      pc = pVarElem->Attribute("sum");
      if(pc == NULL)
	continue;
      v_MD5sum_web = pc;
	      
      /* web informations */
      pc = pVarElem->Attribute("web_difficulty");
      if(pc == NULL)
	continue;
      v_difficulty = pc;
      for(unsigned int i=0; i<v_difficulty.length(); i++) {
	if(v_difficulty[i] == ',')
	  v_difficulty[i] = '.';
      }

      pc = pVarElem->Attribute("web_quality");
      if(pc == NULL)
	continue;
      v_quality = pc;
      for(unsigned int i=0; i<v_quality.length(); i++) {
	if(v_quality[i] == ',')
	  v_quality[i] = '.';
      }

      pc = pVarElem->Attribute("creation_date");
      if(pc == NULL)
	continue;
      v_creationDate = pc;

      pc = pVarElem->Attribute("crappy");
      if(pc == NULL) {
	v_crappy = "0";
      } else {
	v_crappy = std::string(pc) == "true" ? "1" : "0";
      }

      pc = pVarElem->Attribute("children_compliant");
      if(pc == NULL) {
	v_children_compliant = "1";
      } else {
	v_children_compliant = std::string(pc) == "true" ? "1" : "0";
      }

      // add the level
      simpleSql("INSERT INTO weblevels(id_level, name, fileUrl, "
		"checkSum, difficulty, quality, creationDate, crappy, children_compliant) VALUES (\"" +
		protectString(v_levelId)    + "\", \"" +
		protectString(v_levelName)  + "\", \"" +
		protectString(v_url)        + "\", \"" +
		protectString(v_MD5sum_web) + "\", "   +
		v_difficulty   + ", "   +
		v_quality      + ", \"" +
		v_creationDate + "\", " +
		v_crappy       + ", "   +
		v_children_compliant + ");");

      pVarElem = pVarElem->NextSiblingElement("level");
    }
    simpleSql("COMMIT;");
  } catch(Exception &e) {
    simpleSql("ROLLBACK;");
    throw e;
  }
}

void xmDatabase::webrooms_updateDB(const std::string& i_webroomsFile) {
  XMLDocument v_webRXml;
  TiXmlDocument *v_webRXmlData;
  TiXmlElement *v_webRXmlDataElement;
  const char *pc;
  std::string v_RoomName, v_RoomHighscoreUrl, v_RoomId;

  try {
    simpleSql("BEGIN TRANSACTION;");
    simpleSql("DELETE FROM webrooms;");

    v_webRXml.readFromFile(i_webroomsFile);
    v_webRXmlData = v_webRXml.getLowLevelAccess();

    if(v_webRXmlData == NULL) {
      throw Exception("error : unable to analyze xml rooms list file");
    }

    v_webRXmlDataElement = v_webRXmlData->FirstChildElement("xmoto_rooms");
    
    if(v_webRXmlDataElement == NULL) {
      throw Exception("error : unable to analyze xml rooms list file");
    }
    
    TiXmlElement *pVarElem = v_webRXmlDataElement->FirstChildElement("room");
    while(pVarElem != NULL) {
      
      pc = pVarElem->Attribute("name");
      if(pc == NULL) continue; // hehe it feels the unlimited loop, too tired to fix that tonight
      v_RoomName = pc;
  
      pc = pVarElem->Attribute("highscores_url");
      if(pc == NULL) continue;
      v_RoomHighscoreUrl = pc;   
    
      pc = pVarElem->Attribute("id");
      if(pc == NULL) continue;
      v_RoomId = pc;

      webrooms_addRoom(v_RoomId, v_RoomName, v_RoomHighscoreUrl);

      pVarElem = pVarElem->NextSiblingElement("room");
    }
    simpleSql("COMMIT;");
  } catch(Exception &e) {
    simpleSql("ROLLBACK;");
    throw e;
  }
}

void xmDatabase::webthemes_updateDB(const std::string& i_webThemesFile) {
  XMLDocument v_webTXml;
  TiXmlDocument *v_webTXmlData;
  TiXmlElement *v_webTXmlDataElement;
  const char *pc;
  std::string v_themeName, v_url, v_MD5sum_web;

  try {
    simpleSql("BEGIN TRANSACTION;");
    simpleSql("DELETE FROM webthemes;");
    
    v_webTXml.readFromFile(i_webThemesFile);
    v_webTXmlData = v_webTXml.getLowLevelAccess();
    
    if(v_webTXmlData == NULL) {
      throw Exception("error : unable to analyze xml theme file");
    }
    
    v_webTXmlDataElement = v_webTXmlData->FirstChildElement("xmoto_themes");
    
    if(v_webTXmlDataElement == NULL) {
      throw Exception("error : unable to analyze xml theme file");
    }
    
    TiXmlElement *pVarElem = v_webTXmlDataElement->FirstChildElement("theme");
    while(pVarElem != NULL) {
      
      pc = pVarElem->Attribute("name");
      if(pc == NULL) continue;
      v_themeName = pc;
      
      pc = pVarElem->Attribute("url");
      if(pc == NULL) continue;
      v_url = pc;  
      
      pc = pVarElem->Attribute("sum");
      if(pc == NULL) continue;
      v_MD5sum_web = pc;
      
      webthemes_addTheme(v_themeName, v_url, v_MD5sum_web);
      
      pVarElem = pVarElem->NextSiblingElement("theme");
    }
    simpleSql("COMMIT;");
  } catch(Exception &e) {
    simpleSql("ROLLBACK;");
    throw e;
  }
}

void xmDatabase::webthemes_addTheme(const std::string& i_id_theme, const std::string& i_url,
				    const std::string& i_checkSum) {
  simpleSql("INSERT INTO webthemes("
	    "id_theme,  fileUrl, checkSum) "
	    "VALUES(\"" + protectString(i_id_theme) + "\", " +
	    "\""        + protectString(i_url)      + "\", " +
	    "\""        + protectString(i_checkSum) + "\");");
}

void xmDatabase::webLoadDataFirstTime() {
  char **v_result;
  unsigned int nrow;
  std::string v_res;
  bool v_update = false;

  /* updating weblevels table with defaults */
  try {
    /* if the is any row already in the table, no update is done */
    v_result = readDB("SELECT count(1) FROM weblevels;", nrow);
    if(nrow == 1) {
      v_update = atoi(getResult(v_result, 1, 0, 0)) == 0;
    }
    read_DB_free(v_result);

    if(v_update) {
      Logger::Log("Loading weblevels with delivered weblevels.xml");
      weblevels_updateDB("default/weblevels.xml");
    }
  } catch(Exception &e) {
    /* ok, no pb */
    Logger::Log("** Warning ** : Loading delivered weblevels.xml failed");
  }

  /* updating webhighscores table with defaults */
  try {
    /* if the is any row already in the table, no update is done */
    v_result = readDB("SELECT count(1) FROM webhighscores;", nrow);
    if(nrow == 1) {
      v_update = atoi(getResult(v_result, 1, 0, 0)) == 0;
    }
    read_DB_free(v_result);

    if(v_update) {
      Logger::Log("Loading webhighscores with delivered webhighscores.xml");
      webhighscores_updateDB("default/webhighscores.xml", DEFAULT_WEBHIGHSCORES_URL);
    }
  } catch(Exception &e) {
    /* ok, no pb */
    Logger::Log("** Warning ** : Loading delivered webhighscores.xml failed");
  }
}
