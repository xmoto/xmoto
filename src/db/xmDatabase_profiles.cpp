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
#include "GameText.h"
#include "VFileIO.h"
#include "helpers/VExcept.h"

void xmDatabase::updateDB_profiles(XmDatabaseUpdateInterface *i_interface) {
  std::string v_playerName;
  int         v_nNumPlayerProfiles; 
  int         v_nNumCompleted;
  int         v_nNumSkipped;
  std::string v_id_level;
  std::string v_replay;
  std::string v_timeStamp;
  float       v_fTime;
  short       v_nVersion;
  /* Open binary file for input */
  vapp::FileHandle *pfh = vapp::FS::openIFile("players.bin");
  if(pfh == NULL) {
    return;       
  }

  try {
    v_nVersion = vapp::FS::readShort_LE(pfh);
    if(v_nVersion != 0x12) {
      vapp::FS::closeFile(pfh);
      throw Exception("players.bin not in 0x12 version");
    }
  } catch(Exception &e) {
    vapp::FS::closeFile(pfh);
    throw e;
  }

  try {
    simpleSql("BEGIN TRANSACTION;");

    /* Read number of player profiles */
    v_nNumPlayerProfiles = vapp::FS::readInt_LE(pfh);
    
    if(v_nNumPlayerProfiles >= 0) {
      /* For each player profile */
      for(int i=0; i<v_nNumPlayerProfiles; i++) {
	v_playerName = vapp::FS::readString(pfh);
	
	if(i_interface != NULL) {
	  i_interface->updatingDatabase(std::string(GAMETEXT_DB_UPGRADING_PROFILE)
					+ " " + v_playerName);
	}

	v_nNumCompleted = vapp:: FS::readInt_LE(pfh);
	v_nNumSkipped   = vapp::FS::readInt_LE(pfh);
	
	for(int j=0; j<v_nNumCompleted; j++) {
	  v_id_level = vapp::FS::readString(pfh);
	}      

	for(int j=0; j<v_nNumSkipped; j++) {
	  v_id_level = vapp::FS::readString(pfh);
	}      
	
	v_id_level = vapp::FS::readString(pfh);
	while(v_id_level.length() != 0) {
	  v_replay    = vapp::FS::readString(pfh);
	  v_timeStamp = vapp::FS::readString(pfh);
	  v_timeStamp = v_timeStamp.substr(9, 10) + " " + v_timeStamp.substr(0, 8);
	  v_fTime     = vapp::FS::readFloat_LE(pfh);

	  /* insert row */
	  profiles_addFinishTime(v_playerName, v_id_level, v_timeStamp, v_fTime);

	  v_id_level = vapp::FS::readString(pfh);
	}
      }
    }
    simpleSql("COMMIT;");
  } catch(Exception &e) {
    simpleSql("ROLLBACK;");
    throw e;
  }

  vapp::FS::closeFile(pfh);
}

void xmDatabase::profiles_addFinishTime(const std::string& i_profile, const std::string& i_id_level,
					const std::string& i_timeStamp, float i_finishTime) {
  std::ostringstream v_fTime_str;
  v_fTime_str << i_finishTime;
  std::string v_timeToKeep;
  char **v_result;
  int nrow;

  simpleSql(std::string("INSERT INTO profile_completedLevels("
			"id_profile, id_level, timeStamp, finishTime) "
			"VALUES(") + 
	    "\"" + protectString(i_profile) + "\", " +
	    "\"" + protectString(i_id_level)   + "\", " +
	    "\"" + i_timeStamp       	       + "\", " +
	    v_fTime_str.str()        	       + ");");
  
  /* keep only the top 10 */
  v_result = readDB("SELECT finishTime FROM profile_completedLevels "
		    "WHERE id_level=\""   + protectString(i_id_level)   + "\" "
		    "AND   id_profile=\"" + protectString(i_profile) + "\" "
		    "ORDER BY finishTime ASC LIMIT 10;",
		    nrow);
  if(nrow < 10) {
    read_DB_free(v_result);
    return;
  }
  v_timeToKeep = getResult(v_result, 1, 9, 0);
  read_DB_free(v_result);

  simpleSql("DELETE FROM profile_completedLevels "
	    "WHERE id_level=\""   + protectString(i_id_level) + "\" "
	    "AND   id_profile=\"" + protectString(i_profile)  + "\" "
	    "AND finishTime > "   + v_timeToKeep + ";");
}
