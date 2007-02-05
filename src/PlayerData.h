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
	    	    
    /* Data interface */
    std::vector<PlayerProfile *> &getProfiles(void) {return m_Profiles;}
	    
  private:
    /* Data */
    std::vector<PlayerProfile *> m_Profiles;
	    
    
    /* Helpers */
    typedef std::string (*LevelFixer)(const std::string &);
    static std::string _NoFixLevelID (const std::string &);
    static std::string _Fix016LevelID(const std::string &);
    static std::string _Fix018LevelID(const std::string &);
    
    void _LoadFileHandle(FileHandle *pfh, LevelFixer fixer, bool swapped);
    void _FreePlayerData(void);
  };

}

#endif
