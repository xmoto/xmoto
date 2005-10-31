/*=============================================================================
XMOTO
Copyright (C) 2005 Rasmus Neckelmann (neckelmann@gmail.com)

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

#ifndef __REPLAY_H__
#define __REPLAY_H__

#include "VCommon.h"
#include "VApp.h"

namespace vapp {

	/*===========================================================================
	Replay info
  ===========================================================================*/  
  struct ReplayInfo {
    ReplayInfo() {
      fFinishTime=0.0f;
    }  

    std::string PlayerName,LevelID,Name;
    float fFinishTime;
  };

	/*===========================================================================
	Replay class
  ===========================================================================*/
  class Replay {
    public:
      Replay() {
        m_fFinishTime = 0.0f;
      }
      ~Replay() {_Free();}
      
      /* Static replay prober */
      static std::vector<ReplayInfo *> probeReplays(void);
      static void freeReplayList(std::vector<ReplayInfo *> &List);
//      static std::string eventName(ReplayEventType Type);
      
      /* Methods */
      void loadFromFile(std::string Name);
      void saveToFile(std::string Name);
  //    void event(int nFrame,ReplayEventType Type);      
      
      /* Data interface */
      //std::vector<ReplayEventChunk *> &getEventChunks(void) {return m_EventChunks;}            
      float getFinishTime() {return m_fFinishTime;}
      void setFinishTime(float fTime) {m_fFinishTime=fTime;}
      std::string getPlayerName(void) {return m_PlayerName;}
      void setPlayerName(std::string Name) {m_PlayerName=Name;}
      std::string getLevelID(void) {return m_LevelID;}
      void setLevelID(std::string ID) {m_LevelID=ID;}      
      
    private:
      /* Data */
      //std::vector<ReplayEventChunk *> m_EventChunks;
      
      /* Information */
      float m_fFinishTime;      /* Finishing time - not necesarily true */
      std::string m_PlayerName; /* Name of accomplishing player */
      std::string m_LevelID;    /* Level ID */
      
      /* Helpers */
      void _Free(void);
  };

};

#endif

