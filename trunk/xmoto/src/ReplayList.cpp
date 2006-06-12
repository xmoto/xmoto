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
 *  Replay list class
 */
#include "Replay.h"

namespace vapp {

  /*===========================================================================
  Scan for new replays in /Replays
  ===========================================================================*/
  void ReplayList::update(const std::string &Replay) {
    int nNumNew = 0;
  
    /* Set all current replays to NOT updated */
    for(int i=0;i<m_Replays.size();i++) {
      m_Replays[i]->bUpdated = false;
    }
  
    std::vector<std::string> ReplayFiles;
    
    if(Replay != "") {
      /* We just want to update a single .rpl file */
      char cBuf[1024];
      sprintf(cBuf,"Replays/%s.rpl",Replay.c_str());
      ReplayFiles = FS::findPhysFiles(cBuf);
    }
    else {
      /* Scan /Replays dir for .rpl files */
      ReplayFiles = FS::findPhysFiles("Replays/*.rpl");
    }
    
    for(int i=0;i<ReplayFiles.size();i++) {
      /* Latest.rpl? We don't want that here... (TODO: or do we?) */
      if(FS::getFileBaseName(ReplayFiles[i]) != "Latest") {    
        /* Get time stamp */
        int nTimeStamp = FS::getFileTimeStamp(ReplayFiles[i]);
        //printf("[%s %d]\n",ReplayFiles[i].c_str(),nTimeStamp);
        
        std::string BaseName = FS::getFileBaseName(ReplayFiles[i]);
        
        /* Look up replay */
        ReplayInfo *pRpl = _FindReplayInfo(BaseName);
        if(pRpl != NULL) pRpl->bUpdated = true; /* we still got this */
        
        /* Found anything, and is the time stamp the same? If so, we don't 
           want to check this again */
        if(pRpl == NULL || pRpl->nTimeStamp != nTimeStamp) {
          /* Try opening it */
          FileHandle *pfh = FS::openIFile(ReplayFiles[i]);
          if(pfh != NULL) {
            int nVersion = FS::readByte(pfh);
            if(nVersion == 0 || nVersion == 1) {
              if(FS::readInt(pfh) == 0x12345678) {                  
                std::string LevelID = FS::readString(pfh);
                std::string Player = FS::readString(pfh);
                float fFrameRate = FS::readFloat(pfh);
                int nStateSize = FS::readInt(pfh);
                bool bFinished = FS::readBool(pfh);
                float fFinishTime = FS::readFloat(pfh);
                
                /* Already got replay with this name? If so, just update it */
                if(pRpl == NULL) {
                  pRpl = new ReplayInfo;
                  pRpl->bUpdated = true;
                  m_Replays.push_back(pRpl);
                }              
                
                nNumNew++;
                
                /* Set members */
                pRpl->Level = LevelID;
                pRpl->Name = BaseName;
                pRpl->Player = Player;
                pRpl->nTimeStamp = nTimeStamp;
                pRpl->fFrameRate = fFrameRate;

                if(bFinished)
                  pRpl->fFinishTime = fFinishTime;
                else
                  pRpl->fFinishTime = -1;                
              }
            }
            else {
              /* Not supported */
            }
          
            FS::closeFile(pfh);
          }
        }
      }
    }
    
    /* Some replays not updated? If so, remove them, as they're probably no
       longer available */
    bool bDel;
    do {
      bDel = false;
      for(int i=0;i<m_Replays.size();i++) {
        if(!m_Replays[i]->bUpdated) {
          /* Delete this */
          bDel = true;
          
          delete m_Replays[i];
          m_Replays.erase( m_Replays.begin() + i );
          
          break;
        }
      }
    } while(bDel);
    
    //printf("NEW REPLAYS = %d\n",nNumNew);
  }
  
  /*===========================================================================
  Clean up stuff
  ===========================================================================*/
  void ReplayList::clear(void) {
    for(int i=0;i<m_Replays.size();i++)
      delete m_Replays[i];
    m_Replays.clear();     
  }
  
  /*===========================================================================
  Look up replays in list
  ===========================================================================*/
  std::vector<ReplayInfo *> ReplayList::findReplays(const std::string &PlayerName,const std::string &LevelID) {
    /* Find replays */
    if(PlayerName == "" && LevelID == "")
      return m_Replays; /* we want everything */
      
    std::vector<ReplayInfo *> Ret;
    
    for(int i=0;i<m_Replays.size();i++) {
      if((PlayerName=="" || PlayerName==m_Replays[i]->Player) &&
         (LevelID=="" || LevelID==m_Replays[i]->Level)) {
        /* Got one */
        Ret.push_back(m_Replays[i]);
      }
    }
    
    /* Return list */
    return Ret;
  }

  /*===========================================================================
  Look up specific replay
  ===========================================================================*/
  ReplayInfo *ReplayList::_FindReplayInfo(const std::string &ReplayName) {
    for(int i=0;i<m_Replays.size();i++) { 
      if(m_Replays[i]->Name == ReplayName) return m_Replays[i]; /* found */
    }
    return NULL; /* nothing */
  }

};

