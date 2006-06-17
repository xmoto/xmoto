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

#ifndef __STATS_H__
#define __STATS_H__

#include "VCommon.h"
#include "VApp.h"
#include "VFileIO.h"
#include "VXml.h"
#include "GUI.h"

namespace vapp {

	/*===========================================================================
	Structs
  ===========================================================================*/
  struct LevelStats {
    LevelStats() {
      nPlayed = nDied = nCompleted = nRestarts = 0;
      fPlayTime = 0.0f;
    }
  
    std::string LevelID,LevelName;
    
    /* Misc counters */
    int nPlayed;            /* Number of times this level has been played */
    int nDied;              /* Number of deaths in level */
    int nCompleted;         /* Times completed */
    float fPlayTime;        /* Total playtime for level */
    int nRestarts;          /* Number of times level has been restarted */
  };
  
  struct PlayerStats {
    PlayerStats() {
      fTotalPlayTime = 0.0f;
      nXMotoStarts = nLevelsPlayed = nDiffLevelsPlayed = nDeaths = nRestarts = 
        nNumCompleted = 0;
    }
    
    std::string PlayerName;
    
    /* Per level stats */
    std::vector<LevelStats *> Levels;
    
    /* Misc counters */
    float fTotalPlayTime;   /* Total playtime, all levels */
    int nXMotoStarts;       /* Number of times the player has started X-Moto */
    int nLevelsPlayed;      /* Number of times the player has played a level */
    int nDiffLevelsPlayed;  /* Number of different levels */
    int nDeaths;            /* Total deaths */
    int nRestarts;          /* Total level restarts */
    int nNumCompleted;      /* Levels completed */
  };

	/*===========================================================================
	Enums
  ===========================================================================*/
  enum StatOrder {
    ORDER_BY_PLAYED,
    ORDER_BY_PLAYTIME,
    ORDER_BY_DEATHS,
    ORDER_BY_COMPLETED,
    ORDER_BY_RESTARTS
  };

	/*===========================================================================
	Statistics class
  ===========================================================================*/
  class Stats {
    public:
      ~Stats() {_FreeStats();}
    
      /* Methods */
      void loadXML(const std::string &FileName);
      void saveXML(const std::string &FileName);
      
      UIWindow *generateReport(const std::string &PlayerName,UIWindow *pParent,int x,int y,int nWidth,int nHeight,UIFont *pFont);
      
      /* Stats collectors */
      void levelCompleted(const std::string &PlayerName,const std::string &LevelID,const std::string &LevelName,float fPlayTime);
      void died(const std::string &PlayerName,const std::string &LevelID,const std::string &LevelName,float fPlayTime);
      void abortedLevel(const std::string &PlayerName,const std::string &LevelID,const std::string &LevelName,float fPlayTime);
      void levelRestarted(const std::string &PlayerName,const std::string &LevelID,const std::string &LevelName,float fPlayTime);
      void xmotoStarted(const std::string &PlayerName);
      
    private:
      /* Data */
      std::vector<PlayerStats *> m_Players;
      
      /* Helpers */
      void _FreeStats(void);
      PlayerStats *_FindPlayerStats(const std::string &PlayerName);
      LevelStats *_FindLevelStats(PlayerStats *pPlayer,const std::string &LevelID,const std::string &LevelName);
      bool _DefinePlayerAndLevel(PlayerStats **ppPlayer,LevelStats **ppLevel,
                                 const std::string &PlayerName,const std::string &LevelID,const std::string &LevelName);
      std::string _ReadString(TiXmlElement *pElem,const std::string &s);
      float _ReadFloat(TiXmlElement *pElem,const std::string &s);
      int _ReadInt(TiXmlElement *pElem,const std::string &s);
      
      std::vector<LevelStats *> _GetOrderedLevels(StatOrder Order,const std::string &PlayerName);
  };

};

#endif

