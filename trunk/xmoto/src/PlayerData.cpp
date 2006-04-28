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

/* 
 *  Player data (profiles, topscores, etc) management.
 */
#include "VFileIO.h"
#include "PlayerData.h"

namespace vapp {

  /*===========================================================================
  Utility function to translate pre-0.1.7 level IDs to 0.1.9 level IDs
  ===========================================================================*/
  std::string PlayerData::_Fix016LevelID(const std::string &s) {
    /* NOTE! Remember to update this if changing level numbering again */
    if(s == "_iL00_") return "tut1";
    else if(s == "_iL03_") return "_iL08_";
    else if(s == "_iL04_") return "_iL09_";
    else if(s == "_iL05_") return "_iL10_";
    else if(s == "_iL06_") return "_iL11_";
    else if(s == "_iL07_") return "_iL12_";
    else if(s == "_iL08_") return "_iL13_";
    else if(s == "_iL09_") return "_iL14_";
    else if(s == "_iL10_") return "_iL15_";
    else if(s == "_iL11_") return "_iL16_";
    else if(s == "_iL12_") return "_iL17_";
    else if(s == "_iL13_") return "_iL18_";
    else if(s == "_iL14_") return "_iL19_";
    else if(s == "_iL15_") return "_iL20_";
    else if(s == "_iL16_") return "_iL21_";    
    else if(s == "_iL17_") return "_iL22_";
    return s;
  }

  /*===========================================================================
  Utility function to translate pre-0.1.9 level IDs to 0.1.9 level IDs
  ===========================================================================*/
  std::string PlayerData::_Fix018LevelID(const std::string &s) {
    /* NOTE! Remember to update this if changing level numbering again */
    if(s == "_iL00_") return "tut1";
    //else if(s == "_iL01_") return "_iL00_";
    //else if(s == "_iL02_") return "_iL01_";
    //else if(s == "_iL03_") return "_iL02_";
    //else if(s == "_iL04_") return "_iL03_";
    //else if(s == "_iL05_") return "_iL04_";
    //else if(s == "_iL06_") return "_iL05_";
    //else if(s == "_iL07_") return "_iL06_";
    //else if(s == "_iL08_") return "_iL07_";
    //else if(s == "_iL09_") return "_iL08_";
    //else if(s == "_iL10_") return "_iL09_";
    //else if(s == "_iL11_") return "_iL10_";
    //else if(s == "_iL12_") return "_iL11_";
    //else if(s == "_iL13_") return "_iL12_";
    //else if(s == "_iL14_") return "_iL13_";
    //else if(s == "_iL15_") return "_iL14_";
    //else if(s == "_iL16_") return "_iL15_";    
    //else if(s == "_iL17_") return "_iL16_";
    //else if(s == "_iL18_") return "_iL17_";
    //else if(s == "_iL19_") return "_iL18_";
    //else if(s == "_iL20_") return "_iL19_";
    //else if(s == "_iL21_") return "_iL20_";
    //else if(s == "_iL22_") return "_iL21_";
    //else if(s == "_iL23_") return "_iL22_";
    //else if(s == "_iL24_") return "_iL23_";
    //else if(s == "_iL25_") return "_iL24_";
    //else if(s == "_iL26_") return "_iL25_";
    //else if(s == "_iL27_") return "_iL26_";
    //else if(s == "_iL28_") return "_iL27_";
    return s;
  }

  /*===========================================================================
  Load from binary (would rather use XML, but this makes it a bit more 
  cumbersome to cheat)
  ===========================================================================*/
 	void PlayerData::loadFile(void) {
	  _FreePlayerData();
	
	  /* Open binary file for input */
	  FileHandle *pfh = FS::openIFile("players.bin");
	  if(pfh == NULL) {
      /* Nuthin */
      return;	    
	  }
	  
	  /* Parse it */
	  int nVersion = FS::readShort(pfh);
	  
	  if(nVersion == 0x10) {
	    /* This is the old (<= 0.1.6) players.bin file. At 0.1.7 there was a bit of moving
	       around in the levels numbers, so if we try to load this now,  it will for sure explode.
	       Instead, simply load it and translate the level numbers on-the-fly */
	    Log("** Warning ** : Found players.bin from before version 0.1.7, trying to upgrade...\n");
	  
	    /* Read number of player profiles */
	    int nNumPlayerProfiles = FS::readInt(pfh);

	    if(nNumPlayerProfiles >= 0) {
	      /* For each player profile */
        for(int i=0;i<nNumPlayerProfiles;i++) {
          std::string PlayerName = FS::readString(pfh);
          PlayerProfile *pPlayer = createProfile(PlayerName);
          if(pPlayer != NULL) {
            int nNumCompletedInternals = FS::readInt(pfh);
            int nNumSkippedInternals = FS::readInt(pfh);
            
            for(int j=0;j<nNumCompletedInternals;j++)
              completeLevel(PlayerName,_Fix016LevelID(FS::readString(pfh)));

            for(int j=0;j<nNumSkippedInternals;j++)
              skipLevel(PlayerName,_Fix016LevelID(FS::readString(pfh)));
              
            while(1) {
              std::string LevelID = _Fix016LevelID(FS::readString(pfh));
              if(LevelID.length() == 0) break;
              std::string Replay = FS::readString(pfh);
              std::string TimeStamp = FS::readString(pfh);
              float fTime = FS::readFloat(pfh);
              addFinishTime(PlayerName,Replay,LevelID,fTime,TimeStamp);             
            }
          }
        }
      }
	  }
	  else if(nVersion == 0x11) {
	    /* This is the old (<= 0.1.8) players.bin file. At 0.1.9 there was a bit of moving
	       around in the levels numbers, so if we try to load this now,  it will for sure explode.
	       Instead, simply load it and translate the level numbers on-the-fly */
	    Log("** Warning ** : Found players.bin from before version 0.1.9, trying to upgrade...\n");
	  
	    /* Read number of player profiles */
	    int nNumPlayerProfiles = FS::readInt(pfh);

	    if(nNumPlayerProfiles >= 0) {
	      /* For each player profile */
        for(int i=0;i<nNumPlayerProfiles;i++) {
          std::string PlayerName = FS::readString(pfh);
          PlayerProfile *pPlayer = createProfile(PlayerName);
          if(pPlayer != NULL) {
            int nNumCompletedInternals = FS::readInt(pfh);
            int nNumSkippedInternals = FS::readInt(pfh);
            
            for(int j=0;j<nNumCompletedInternals;j++)
              completeLevel(PlayerName,_Fix018LevelID(FS::readString(pfh)));

            for(int j=0;j<nNumSkippedInternals;j++)
              skipLevel(PlayerName,_Fix018LevelID(FS::readString(pfh)));
              
            while(1) {
              std::string LevelID = _Fix018LevelID(FS::readString(pfh));
              if(LevelID.length() == 0) break;
              std::string Replay = FS::readString(pfh);
              std::string TimeStamp = FS::readString(pfh);
              float fTime = FS::readFloat(pfh);
              addFinishTime(PlayerName,Replay,LevelID,fTime,TimeStamp);             
            }
          }
        }
      }
    }
	  else if(nVersion == 0x12) {
	    /* Read number of player profiles */
	    int nNumPlayerProfiles = FS::readInt(pfh);

	    if(nNumPlayerProfiles >= 0) {
	      /* For each player profile */
        for(int i=0;i<nNumPlayerProfiles;i++) {
          std::string PlayerName = FS::readString(pfh);
          PlayerProfile *pPlayer = createProfile(PlayerName);
          if(pPlayer != NULL) {
            int nNumCompletedInternals = FS::readInt(pfh);
            int nNumSkippedInternals = FS::readInt(pfh);
            
            for(int j=0;j<nNumCompletedInternals;j++)
              completeLevel(PlayerName,FS::readString(pfh));

            for(int j=0;j<nNumSkippedInternals;j++)
              skipLevel(PlayerName,FS::readString(pfh));
              
            while(1) {
              std::string LevelID =FS::readString(pfh);
              if(LevelID.length() == 0) break;
              std::string Replay = FS::readString(pfh);
              std::string TimeStamp = FS::readString(pfh);
              float fTime = FS::readFloat(pfh);
              addFinishTime(PlayerName,Replay,LevelID,fTime,TimeStamp);             
            }
          }
        }
      }
	  }
	  else Log("** Warning ** : invalid/unsupported 'players.bin' file, all old stats is lost");
	  
	  /* Clean */
	  FS::closeFile(pfh);
	}

  /*===========================================================================
  Save to binary
  ===========================================================================*/
	void PlayerData::saveFile(void) {
	  /* Open binary file for output */
	  FileHandle *pfh = FS::openOFile("players.bin");
	  if(pfh == NULL) { 
	    Log("** Warning ** : failed to save 'players.bin', all new stats is lost");
      return;	    
	  }
	  
	  /* Write */
	  FS::writeShort(pfh,0x12);
	  FS::writeInt(pfh,m_Profiles.size());
	  for(int i=0;i<m_Profiles.size();i++) {
	    FS::writeString(pfh,m_Profiles[i]->PlayerName);
	    FS::writeInt(pfh,m_Profiles[i]->CompletedInternals.size());
	    FS::writeInt(pfh,m_Profiles[i]->SkippedInternals.size());
	    
	    for(int j=0;j<m_Profiles[i]->CompletedInternals.size();j++)
        FS::writeString(pfh,m_Profiles[i]->CompletedInternals[j]);	      

	    for(int j=0;j<m_Profiles[i]->SkippedInternals.size();j++)
        FS::writeString(pfh,m_Profiles[i]->SkippedInternals[j]);	      
        
      for(int j=0;j<m_Profiles[i]->LevelStats.size();j++) {
        for(int k=0;k<m_Profiles[i]->LevelStats[j]->nNumBestTimes;k++) {
          FS::writeString(pfh,m_Profiles[i]->LevelStats[j]->LevelID);
          FS::writeString(pfh,m_Profiles[i]->LevelStats[j]->BestTimes[k].Replay);
          FS::writeString(pfh,m_Profiles[i]->LevelStats[j]->BestTimes[k].TimeStamp);
          FS::writeFloat(pfh,m_Profiles[i]->LevelStats[j]->BestTimes[k].fFinishTime);
        }
      }
      
      FS::writeByte(pfh,0);
	  }
	  
	  /* Clean */
	  FS::closeFile(pfh);	  
	}
	
  /*===========================================================================
  Creation and destruction of profiles
  ===========================================================================*/
	PlayerProfile *PlayerData::createProfile(std::string PlayerName) {
	  if(getProfile(PlayerName) != NULL) {
	    Log("** Warning ** : player profile '%s' already in use",PlayerName.c_str());
	    return NULL;
	  }
	
	  PlayerProfile *pProfile = new PlayerProfile;
	  pProfile->PlayerName = PlayerName;	  
	  m_Profiles.push_back(pProfile);
	  return pProfile;
	}
	
	void PlayerData::destroyProfile(std::string PlayerName) {
	  for(int i=0;i<m_Profiles.size();i++) {
	    if(m_Profiles[i]->PlayerName == PlayerName) {
	      for(int j=0;j<m_Profiles[i]->LevelStats.size();j++)
	        delete m_Profiles[i]->LevelStats[j];
	    
	      delete m_Profiles[i];
	      m_Profiles.erase(m_Profiles.begin() + i);
	      return;
	    }
	  }
	  Log("** Warning ** : attempt to destroy unknown player profile '%s'",PlayerName.c_str());
	}
	
  /*===========================================================================
  Information about internal levels
  ===========================================================================*/
	bool PlayerData::isLevelCompleted(std::string PlayerName,std::string LevelID) {
	  PlayerProfile *pPlayer = getProfile(PlayerName);
	  if(pPlayer == NULL) return false;
	  
	  for(int i=0;i<pPlayer->CompletedInternals.size();i++)
	    if(pPlayer->CompletedInternals[i] == LevelID) return true;

	  for(int i=0;i<pPlayer->CompletedInternals.size();i++)
	  if(pPlayer->CompletedInternals[i] == LevelID) return true;
	  
	  return false;
	}
	
	bool PlayerData::isLevelSkipped(std::string PlayerName,std::string LevelID) {
	  PlayerProfile *pPlayer = getProfile(PlayerName);
	  if(pPlayer == NULL) return false;
	  
	  for(int i=0;i<pPlayer->SkippedInternals.size();i++)
	    if(pPlayer->SkippedInternals[i] == LevelID) return true;
	  
	  return false;
	}
	
	void PlayerData::completeLevel(std::string PlayerName,std::string LevelID) {
	  PlayerProfile *pPlayer = getProfile(PlayerName);
	  if(pPlayer == NULL) return;

	  /* If skipped, unskip it */
	  for(int i=0;i<pPlayer->SkippedInternals.size();i++) {
	    if(pPlayer->SkippedInternals[i] == LevelID) {
	      pPlayer->SkippedInternals.erase(pPlayer->SkippedInternals.begin() + i);
	      break;
	    }
	  }
	  
	  if(!isLevelCompleted(PlayerName,LevelID)) {
	    pPlayer->CompletedInternals.push_back(LevelID);
	  }
	}
	
	void PlayerData::skipLevel(std::string PlayerName,std::string LevelID) {
	  if(!isLevelCompleted(PlayerName,LevelID) && !isLevelSkipped(PlayerName,LevelID)) {
	    PlayerProfile *pPlayer = getProfile(PlayerName);
	    if(pPlayer != NULL) pPlayer->SkippedInternals.push_back(LevelID);	    
	  }	  
	}
	
  /*===========================================================================
  Check if a level is internal
  ===========================================================================*/
  bool PlayerData::isInternal(std::string LevelID) {
      return true;

    char *pcInternals[] = {
      "_iL00_","_iL01_","_iL02_","_iL03_","_iL04_","_iL05_","_iL06_","_iL07_","_iL08_","_iL09_",
      "_iL10_","_iL11_","_iL12_","_iL13_","_iL14_","_iL15_","_iL16_","_iL17_","_iL18_","_iL19_",
      "_iL20_","_iL21_","_iL22_","_iL23_","_iL24_","_iL25_","_iL26_","_iL27_","_iL28_","_iL29_",
      "_iL30_","_iL31_","_iL32_","_iL33_","_iL34_","_iL35_","_iL36_","_iL37_","_iL38_","_iL39_",
      "_iL40_","_iL41_","_iL42_","_iL43_","_iL44_","_iL45_","_iL46_","_iL47_","_iL48_","_iL49_",
      NULL      
    };

    int j = 0;
    while(pcInternals[j] != NULL) {
      if(!strcmp(pcInternals[j],LevelID.c_str())) {
        return true;
      }
      j++;
    }
    return false;
  }
	
  /*===========================================================================
  Merge finish time
  ===========================================================================*/
	void PlayerData::addFinishTime(std::string PlayerName,std::string Replay,std::string LevelID,float fTime,std::string TimeStamp) {
	  PlayerProfile *pPlayer = getProfile(PlayerName);
	  if(pPlayer == NULL) return;
	  
	  /* Look for this level */
	  PlayerLevelStats *pStats = NULL; 
	  
	  for(int i=0;i<pPlayer->LevelStats.size();i++) {
	    if(pPlayer->LevelStats[i]->LevelID == LevelID) 
	      pStats = pPlayer->LevelStats[i];
	  }	  
	  
	  /* Is this internal? */
	  if(isInternal(LevelID)) {
	    completeLevel(PlayerName,LevelID);
	  }
	  
	  /* Not there? Then add it */
	  if(pStats == NULL) {
	    pStats = new PlayerLevelStats;
	    pStats->LevelID = LevelID;
	    pStats->nNumBestTimes = 0;
	    pPlayer->LevelStats.push_back(pStats);
	  }
	  
	  /* If less than 10 entries, simply add this and we're done */
	  if(pStats->nNumBestTimes < 10) {
	    pStats->BestTimes[pStats->nNumBestTimes].fFinishTime = fTime;	    
	    pStats->BestTimes[pStats->nNumBestTimes].PlayerName = PlayerName;
	    pStats->BestTimes[pStats->nNumBestTimes].Replay = Replay;
	    pStats->BestTimes[pStats->nNumBestTimes].TimeStamp = TimeStamp;
	    pStats->nNumBestTimes++;
	  }
	  else {
	    /* Find poorest time in list */
	    int nBadTime=-1;
	    
	    for(int i=0;i<pStats->nNumBestTimes;i++) {
	      if(nBadTime<0 || pStats->BestTimes[i].fFinishTime > pStats->BestTimes[nBadTime].fFinishTime)
	        nBadTime = i;
	    }
	    
	    /* Should we replace it? */
	    if(pStats->BestTimes[nBadTime].fFinishTime > fTime) {
	      /* Yup */
	      pStats->BestTimes[nBadTime].fFinishTime = fTime;
	      pStats->BestTimes[nBadTime].PlayerName = PlayerName;
	      pStats->BestTimes[nBadTime].Replay = Replay;
	      pStats->BestTimes[nBadTime].TimeStamp = TimeStamp;
	    }
	  }
	}
	
  /*===========================================================================
  Get global best time of level
  ===========================================================================*/
	PlayerTimeEntry *PlayerData::getBestTime(std::string LevelID) {
    /* Get best time of level */
    PlayerTimeEntry *pBest = NULL;
    for(int i=0;i<m_Profiles.size();i++) {
      for(int j=0;j<m_Profiles[i]->LevelStats.size();j++) {
        if(m_Profiles[i]->LevelStats[j]->LevelID == LevelID) {
          for(int k=0;k<m_Profiles[i]->LevelStats[j]->nNumBestTimes;k++) {
            if(pBest == NULL ||
               m_Profiles[i]->LevelStats[j]->BestTimes[k].fFinishTime < pBest->fFinishTime) {
              pBest = &m_Profiles[i]->LevelStats[j]->BestTimes[k];
            }
          }
        }
      }
    }
    
    return pBest;
	}

  /*===========================================================================
  Get local best time of level
  ===========================================================================*/
	PlayerTimeEntry *PlayerData::getBestPlayerTime(std::string PlayerName,std::string LevelID) {
    /* Get best time of level */
    PlayerTimeEntry *pBest = NULL;
	  PlayerProfile *pPlayer = getProfile(PlayerName);
	  if(pPlayer == NULL) return NULL;	  
    for(int j=0;j<pPlayer->LevelStats.size();j++) {
      if(pPlayer->LevelStats[j]->LevelID == LevelID) {
        for(int k=0;k<pPlayer->LevelStats[j]->nNumBestTimes;k++) {
          if(pBest == NULL ||
             pPlayer->LevelStats[j]->BestTimes[k].fFinishTime < pBest->fFinishTime) {
            pBest = &pPlayer->LevelStats[j]->BestTimes[k];
          }
        }
      }
    }
    
    return pBest;
	}
	
  /*===========================================================================
  Create global top10 for level
  ===========================================================================*/
	std::vector<PlayerTimeEntry *> PlayerData::createLevelTop10(std::string LevelID) {
	  std::vector<PlayerTimeEntry *> Top10;
	  
    for(int i=0;i<m_Profiles.size();i++) {
      for(int j=0;j<m_Profiles[i]->LevelStats.size();j++) {
        if(m_Profiles[i]->LevelStats[j]->LevelID == LevelID) {
          for(int k=0;k<m_Profiles[i]->LevelStats[j]->nNumBestTimes;k++) {
            /* Merge this time into the top10 */
            bool bMerged = false;
            for(int z=0;z<Top10.size();z++) { 
              if(m_Profiles[i]->LevelStats[j]->BestTimes[k].fFinishTime < Top10[z]->fFinishTime) {
                Top10.insert(Top10.begin() + z,&m_Profiles[i]->LevelStats[j]->BestTimes[k]);
                bMerged = true;
                break;
              }
            }
            
            if(!bMerged && Top10.size() < 10) {
              Top10.push_back(&m_Profiles[i]->LevelStats[j]->BestTimes[k]);
            }
          }
        }
      }
    }	  
    
    return Top10;
	}

  /*===========================================================================
  Create top10 for level
  ===========================================================================*/
	std::vector<PlayerTimeEntry *> PlayerData::createPlayerOnlyLevelTop10(std::string PlayerName,std::string LevelID) {
	  std::vector<PlayerTimeEntry *> Top10;
	  PlayerProfile *pPlayer = getProfile(PlayerName);
	  if(pPlayer == NULL) return Top10;	  
	  
    for(int j=0;j<pPlayer->LevelStats.size();j++) {
      if(pPlayer->LevelStats[j]->LevelID == LevelID) {
        for(int k=0;k<pPlayer->LevelStats[j]->nNumBestTimes;k++) {
          /* Merge this time into the top10 */
          bool bMerged = false;
          for(int z=0;z<Top10.size();z++) { 
            if(pPlayer->LevelStats[j]->BestTimes[k].fFinishTime < Top10[z]->fFinishTime) {
              Top10.insert(Top10.begin() + z,&pPlayer->LevelStats[j]->BestTimes[k]);
              bMerged = true;
              break;
            }
          }
          
          if(!bMerged && Top10.size() < 10) {
            Top10.push_back(&pPlayer->LevelStats[j]->BestTimes[k]);
          }
        }
      }
    }
    
    return Top10;
	}

  /*===========================================================================
  Find named profile
  ===========================================================================*/
  PlayerProfile *PlayerData::getProfile(std::string PlayerName) {
	  for(int i=0;i<m_Profiles.size();i++) {
	    if(m_Profiles[i]->PlayerName == PlayerName) return m_Profiles[i];
	  }    
	  return NULL;
  }

  /*===========================================================================
  Free stuff
  ===========================================================================*/
  void PlayerData::_FreePlayerData(void) {
	  for(int i=0;i<m_Profiles.size();i++) {
      for(int j=0;j<m_Profiles[i]->LevelStats.size();j++)
        delete m_Profiles[i]->LevelStats[j];
      delete m_Profiles[i];
	  }
  }	    
	
  /*===========================================================================
  Determine whether this level can be played by this player
  ===========================================================================*/
  bool PlayerData::isLevelAvailable(std::string PlayerName,std::string LevelID) {
    /* TODO: !! */
    return true;
  }

};

