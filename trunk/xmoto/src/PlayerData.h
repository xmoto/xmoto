/*=============================================================================
  XMOTO
  Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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

#ifndef __PLAYERDATA_H__
#define __PLAYERDATA_H__

#include "VCommon.h"
#include "VApp.h"

#define INTERNAL_LEVELS "_iL00_", "_iL01_", "_iL02_", "_iL03_", "_iL04_", \
                        "_iL05_", "_iL06_", "_iL07_", "_iL08_", "_iL09_", \
                        "_iL10_", "_iL11_", "_iL12_", "_iL13_", "_iL14_", \
                        "_iL15_", "_iL16_", "_iL17_", "_iL18_", "_iL19_", \
                        "_iL20_", "_iL21_", "_iL22_", "_iL23_", "_iL24_", \
                        "_iL25_", "_iL26_", "_iL27_", "_iL28_", "_iL29_", \
                        "_iL30_", "_iL31_", "_iL32_", "_iL33_", "_iL34_", \
                        "_iL35_", "_iL36_", "_iL37_", "_iL38_", "_iL39_", \
                        "_iL40_", "_iL41_", "_iL42_", "_iL43_", "_iL44_", \
                        "_iL45_","_iL46_","_iL47_","_iL48_","_iL49_"

namespace vapp {

  /*===========================================================================
    Best times entry
    ===========================================================================*/
  struct PlayerTimeEntry {
    PlayerTimeEntry() {
      fFinishTime = 0.0f;
    }
	
    std::string PlayerName;                       /* Player name */
    std::string Replay;                           /* Optional replay */
    std::string TimeStamp;                        /* When? */
    float fFinishTime;                            /* Finishing time */
  };

  /*===========================================================================
    Level stats
    ===========================================================================*/
  struct PlayerLevelStats {
    std::string LevelID;                          /* ID of level */
    PlayerTimeEntry BestTimes[10];                /* Keep 10 best times */
    int nNumBestTimes;                            /* Number of times in list */
  };

  /*===========================================================================
    Player profile
    ===========================================================================*/
  struct PlayerProfile {
    std::string PlayerName;                       /* Name of player */
    std::vector<std::string> Completed;           /* Completed levels */
    std::vector<std::string> Skipped;             /* Skipped levels */
    std::vector<PlayerLevelStats *> LevelStats;   /* Stats for all levels */
  };
	
  /*===========================================================================
    Player "database" - highscores, profiles, madness.
    ===========================================================================*/
  class PlayerData {
  public:
    ~PlayerData() {_FreePlayerData();}
	  
    /* Methods */
    void loadFile(void);
    void saveFile(void);
	    
    PlayerProfile *createProfile(std::string PlayerName);
    void destroyProfile(std::string PlayerName);
    bool isLevelCompleted(std::string PlayerName,std::string LevelID);
    bool isLevelSkipped(std::string PlayerName,std::string LevelID);
    void completeLevel(std::string PlayerName,std::string LevelID);
    void skipLevel(std::string PlayerName,std::string LevelID);
    void addFinishTime(std::string PlayerName,std::string Replay,std::string LevelID,float fTime,std::string TimeStamp);
    PlayerTimeEntry *getBestTime(std::string LevelID);
    PlayerTimeEntry *getBestPlayerTime(std::string PlayerName,std::string LevelID);
    std::vector<PlayerTimeEntry *> createLevelTop10(std::string LevelID);
    std::vector<PlayerTimeEntry *> createPlayerOnlyLevelTop10(std::string PlayerName,std::string LevelID);
    PlayerProfile *getProfile(std::string PlayerName);
    bool isLevelAvailable(std::string PlayerName,std::string LevelID);
	    	    
    bool isInternal(std::string LevelID);
	    	    	  
    /* Data interface */
    std::vector<PlayerProfile *> &getProfiles(void) {return m_Profiles;}
	    
  private:
    /* Data */
    std::vector<PlayerProfile *> m_Profiles;
	    
    /* Helpers */
    void _FreePlayerData(void);
    std::string _Fix016LevelID(const std::string &s);
    std::string _Fix018LevelID(const std::string &s);
  };

}

#endif
