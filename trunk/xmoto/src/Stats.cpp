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
 *  Game statistics
 */
#include <time.h>
#include "Stats.h"
#include "GameText.h"

namespace vapp {

	/*===========================================================================
	Load/save XML stats
  ===========================================================================*/
  void Stats::loadXML(const std::string &FileName) {
    XMLDocument XML;
    XML.readFromFile(FileName,NULL);
    TiXmlDocument *pXML = XML.getLowLevelAccess();
        
    if(pXML != NULL) {    
      /* Start eating the XML */
      TiXmlElement *pStatsElem = pXML->FirstChildElement("stats");
      if(pStatsElem != NULL) {
        _FreeStats();      
      
        /* Get players */
        for(TiXmlElement *pPlayerElem = pStatsElem->FirstChildElement("player");
            pPlayerElem!=NULL;pPlayerElem=pPlayerElem->NextSiblingElement("player")) {
          const char *pcPlayerName = pPlayerElem->Attribute("name");
          if(pcPlayerName != NULL) {
            /* Create player */
            PlayerStats *pPlayer = _FindPlayerStats(pcPlayerName);
            if(pPlayer == NULL) {
              pPlayer = new PlayerStats;
              pPlayer->PlayerName = pcPlayerName;
              m_Players.push_back(pPlayer);
            }
            
            /* Stats since? */
            const char *pcStatsSince = pPlayerElem->Attribute("since");
            if(pcStatsSince != NULL) {             
              pPlayer->StatsSince = pcStatsSince;
            }
            
            /* Read stats */
            for(TiXmlElement *pStatElem = pPlayerElem->FirstChildElement("stat");
                pStatElem!=NULL;pStatElem=pStatElem->NextSiblingElement("stat")) {
              std::string Tag = _ReadString(pStatElem,"tag");
              if(Tag == "playtime") pPlayer->fTotalPlayTime = _ReadFloat(pStatElem,"v");
              else if(Tag == "xmotostarts") pPlayer->nXMotoStarts = _ReadInt(pStatElem,"v");
              else if(Tag == "played") pPlayer->nLevelsPlayed = _ReadInt(pStatElem,"v");
              else if(Tag == "diffplayed") pPlayer->nDiffLevelsPlayed = _ReadInt(pStatElem,"v");
              else if(Tag == "died") pPlayer->nDeaths = _ReadInt(pStatElem,"v");
              else if(Tag == "restarts") pPlayer->nRestarts = _ReadInt(pStatElem,"v");
              else if(Tag == "completed") pPlayer->nNumCompleted = _ReadInt(pStatElem,"v");
            }
            
            /* Read per-level stats */
            for(TiXmlElement *pLevelElem = pPlayerElem->FirstChildElement("level");
                pLevelElem!=NULL;pLevelElem=pLevelElem->NextSiblingElement("level")) {
              std::string LevelID = _ReadString(pLevelElem,"id");
              std::string LevelName = _ReadString(pLevelElem,"name");
              
              if(LevelID != "" && LevelName != "") {
                /* Create level */
                LevelStats *pLevel = _FindLevelStats(pPlayer,LevelID,LevelName);
                if(pLevel == NULL) {
                  pLevel = new LevelStats;
                  pLevel->LevelID = LevelID;
                  pLevel->LevelName = LevelName;
                  pPlayer->Levels.push_back(pLevel);
                }
              
                /* Read stats */
                for(TiXmlElement *pStatElem = pLevelElem->FirstChildElement("stat");
                    pStatElem!=NULL;pStatElem=pStatElem->NextSiblingElement("stat")) {
                  std::string Tag = _ReadString(pStatElem,"tag");
                  if(Tag == "playtime") pLevel->fPlayTime = _ReadFloat(pStatElem,"v");
                  else if(Tag == "played") pLevel->nPlayed = _ReadInt(pStatElem,"v");
                  else if(Tag == "died") pLevel->nDied = _ReadInt(pStatElem,"v");
                  else if(Tag == "restarts") pLevel->nRestarts = _ReadInt(pStatElem,"v");
                  else if(Tag == "completed") pLevel->nCompleted = _ReadInt(pStatElem,"v");
                }                
              }
            }
          }
        }
      }
    }
  }
  
  void Stats::saveXML(const std::string &FileName) {
    FileHandle *pfh = FS::openOFile(FileName);
    if(pfh != NULL) {
      FS::writeLineF(pfh,"<?xml version=\"1.0\" encoding=\"utf-8\"?>");
      FS::writeLineF(pfh,"<stats>");
      
      for(int i=0;i<m_Players.size();i++) {
        FS::writeLineF(pfh,"  <player name=\"%s\" since=\"%s\">",m_Players[i]->PlayerName.c_str(),m_Players[i]->StatsSince.c_str());
        
        FS::writeLineF(pfh,"    <stat tag=\"playtime\" v=\"%f\"/>",m_Players[i]->fTotalPlayTime);
        FS::writeLineF(pfh,"    <stat tag=\"xmotostarts\" v=\"%d\"/>",m_Players[i]->nXMotoStarts);
        FS::writeLineF(pfh,"    <stat tag=\"played\" v=\"%d\"/>",m_Players[i]->nLevelsPlayed);
        FS::writeLineF(pfh,"    <stat tag=\"diffplayed\" v=\"%d\"/>",m_Players[i]->nDiffLevelsPlayed);
        FS::writeLineF(pfh,"    <stat tag=\"died\" v=\"%d\"/>",m_Players[i]->nDeaths);
        FS::writeLineF(pfh,"    <stat tag=\"restarts\" v=\"%d\"/>",m_Players[i]->nRestarts);
        FS::writeLineF(pfh,"    <stat tag=\"completed\" v=\"%d\"/>",m_Players[i]->nNumCompleted);
        
        for(int j=0;j<m_Players[i]->Levels.size();j++) {
          LevelStats *pLevel = m_Players[i]->Levels[j];
        
          FS::writeLineF(pfh,"    <level id=\"%s\" name=\"%s\">",pLevel->LevelID.c_str(),pLevel->LevelName.c_str());
          
          FS::writeLineF(pfh,"      <stat tag=\"played\" v=\"%d\"/>",pLevel->nPlayed);
          FS::writeLineF(pfh,"      <stat tag=\"died\" v=\"%d\"/>",pLevel->nDied);
          FS::writeLineF(pfh,"      <stat tag=\"completed\" v=\"%d\"/>",pLevel->nCompleted);
          FS::writeLineF(pfh,"      <stat tag=\"playtime\" v=\"%f\"/>",pLevel->fPlayTime);
          FS::writeLineF(pfh,"      <stat tag=\"restarts\" v=\"%d\"/>",pLevel->nRestarts);
          
          FS::writeLineF(pfh,"    </level>");
        }
        
        FS::writeLineF(pfh,"  </player>");
      }
      
      FS::writeLineF(pfh,"</stats>");
    
      FS::closeFile(pfh);
    }
  }
      
  std::string Stats::_ReadString(TiXmlElement *pElem,const std::string &s) {
    const char *pc = pElem->Attribute(s.c_str());
    if(pc == NULL) return "";
    return pc;
  }
  
  float Stats::_ReadFloat(TiXmlElement *pElem,const std::string &s) {
    const char *pc = pElem->Attribute(s.c_str());
    if(pc == NULL) return 0.0f;
    return atof(pc);
  }
  
  int Stats::_ReadInt(TiXmlElement *pElem,const std::string &s) {
    const char *pc = pElem->Attribute(s.c_str());
    if(pc == NULL) return 0;
    return atoi(pc);
  }  
      
	/*===========================================================================
	Report generator
  ===========================================================================*/
  UIWindow *Stats::generateReport(const std::string &PlayerName,UIWindow *pParent,int x,int y,int nWidth,int nHeight,UIFont *pFont) {
    /* Create stats window */
    UIWindow *p = new UIWindow(pParent,x,y,"",nWidth,nHeight);
    
    PlayerStats *pPlayer = _FindPlayerStats(PlayerName);
    if(pPlayer == NULL) {
      UIStatic *pText = new UIStatic(p,0,0,GAMETEXT_NOSTATS,nWidth,20);
      pText->setFont(pFont);      
    }    
    else {
      /* Per-player info */
      char cBuf[512];
      char cTime[512];
      int nHours = ((int)pPlayer->fTotalPlayTime) / (60 * 60);
      int nMinutes = (((int)pPlayer->fTotalPlayTime) / (60)) - nHours*60;
      int nSeconds = (((int)pPlayer->fTotalPlayTime)) - nMinutes*60 - nHours*3600;
      if(nHours > 0) sprintf(cTime,(std::string(GAMETEXT_XHOURS) + ", " + std::string(GAMETEXT_XMINUTES) + ", " + std::string(GAMETEXT_AND) + " " + std::string(GAMETEXT_XSECONDS)).c_str(),nHours,nMinutes,nSeconds);
      else if(nMinutes > 0) sprintf(cTime,(std::string(GAMETEXT_XMINUTES) +  " " + std::string(GAMETEXT_AND) +  " " + std::string(GAMETEXT_XSECONDS)).c_str(),nMinutes,nSeconds);
      else sprintf(cTime,GAMETEXT_XSECONDS,nSeconds);
      
      sprintf(cBuf,GAMETEXT_XMOTOGLOBALSTATS,      
              pPlayer->StatsSince.c_str(),pPlayer->nXMotoStarts,pPlayer->nLevelsPlayed,pPlayer->nDiffLevelsPlayed,pPlayer->nDeaths,pPlayer->nNumCompleted,pPlayer->nRestarts,cTime);                           
      
      UIStatic *pText = new UIStatic(p,0,0,cBuf,nWidth,80);
      pText->setHAlign(UI_ALIGN_LEFT);
      pText->setTextSolidColor(MAKE_COLOR(255,255,0,255));
      pText->setFont(pFont);
      
      /* Per-level stats */      
      pText = new UIStatic(p,0,90, std::string(GAMETEXT_MOSTPLAYEDLEVELSFOLLOW) + ":",nWidth,20);
      pText->setHAlign(UI_ALIGN_LEFT);
      pText->setTextSolidColor(MAKE_COLOR(255,255,0,255));
      pText->setFont(pFont);      
      
      std::vector<LevelStats *> Levels = _GetOrderedLevels(ORDER_BY_PLAYTIME,PlayerName);
      int cy = 110;
      for(int i=0;i<Levels.size();i++) {
        if(cy + 45 > nHeight) break; /* out of window */
        
        sprintf(cBuf,("[%s] %s:\n   " + std::string(GAMETEXT_XMOTOLEVELSTATS)).c_str(),
                App::formatTime(Levels[i]->fPlayTime).c_str(),Levels[i]->LevelName.c_str(),
                Levels[i]->nPlayed,Levels[i]->nDied,Levels[i]->nCompleted,Levels[i]->nRestarts);
        
        pText = new UIStatic(p,0,cy,cBuf,nWidth,45);
        pText->setHAlign(UI_ALIGN_LEFT);        
        pText->setTextSolidColor(MAKE_COLOR(255,255,0,255));
        pText->setFont(pFont);
        
        cy += 45;
      }
    }
    
    UIButton *pUpdateButton = new UIButton(p,nWidth-115,nHeight-57,GAMETEXT_UPDATE,115,57);
    pUpdateButton->setContextHelp(CONTEXTHELP_UPDATESTATS);
    pUpdateButton->setFont(pFont);
    pUpdateButton->setType(UI_BUTTON_TYPE_SMALL);
    pUpdateButton->setID("UPDATE_BUTTON");
    return p;
  }
      
	/*===========================================================================
	Statistics collectors
  ===========================================================================*/
  void Stats::levelCompleted(const std::string &PlayerName,const std::string &LevelID,const std::string &LevelName,float fPlayTime) {
    PlayerStats *pPlayer;
    LevelStats *pLevel;
    bool bNewLevel = _DefinePlayerAndLevel(&pPlayer,&pLevel,PlayerName,LevelID,LevelName);

    pPlayer->fTotalPlayTime += fPlayTime;
    pPlayer->nLevelsPlayed++;
    pPlayer->nNumCompleted++;
    
    pLevel->fPlayTime += fPlayTime;
    pLevel->nCompleted++;
    pLevel->nPlayed++;
    
    if(bNewLevel)
      pPlayer->nDiffLevelsPlayed++;
  }
  
  void Stats::died(const std::string &PlayerName,const std::string &LevelID,const std::string &LevelName,float fPlayTime) {
    PlayerStats *pPlayer;
    LevelStats *pLevel;
    bool bNewLevel = _DefinePlayerAndLevel(&pPlayer,&pLevel,PlayerName,LevelID,LevelName);

    pPlayer->fTotalPlayTime += fPlayTime;
    pPlayer->nLevelsPlayed++;
    pPlayer->nDeaths++;
           
    pLevel->fPlayTime += fPlayTime;
    pLevel->nDied++;
    pLevel->nPlayed++;
    
    if(bNewLevel)
      pPlayer->nDiffLevelsPlayed++;
  }
    
  void Stats::abortedLevel(const std::string &PlayerName,const std::string &LevelID,const std::string &LevelName,float fPlayTime) {
    PlayerStats *pPlayer;
    LevelStats *pLevel;
    bool bNewLevel = _DefinePlayerAndLevel(&pPlayer,&pLevel,PlayerName,LevelID,LevelName);

    pPlayer->fTotalPlayTime += fPlayTime;
    pPlayer->nLevelsPlayed++;

    pLevel->fPlayTime += fPlayTime;
    pLevel->nPlayed++;
        
    if(bNewLevel)
      pPlayer->nDiffLevelsPlayed++;
  }
  
  void Stats::levelRestarted(const std::string &PlayerName,const std::string &LevelID,const std::string &LevelName,float fPlayTime) {
    PlayerStats *pPlayer;
    LevelStats *pLevel;
    bool bNewLevel = _DefinePlayerAndLevel(&pPlayer,&pLevel,PlayerName,LevelID,LevelName);

    pPlayer->fTotalPlayTime += fPlayTime;
    pPlayer->nLevelsPlayed++;
    pPlayer->nRestarts++;

    pLevel->fPlayTime += fPlayTime;
    pLevel->nPlayed++;
    pLevel->nRestarts++;
    
    if(bNewLevel)
      pPlayer->nDiffLevelsPlayed++;
  }
  
  void Stats::xmotoStarted(const std::string &PlayerName) {
    PlayerStats *pPlayer = _FindPlayerStats(PlayerName);
    if(pPlayer==NULL) {
      pPlayer = new PlayerStats;
      pPlayer->PlayerName = PlayerName;
      m_Players.push_back(pPlayer);

      /* Note this time */
      time_t ATime;               
      time(&ATime);
      struct tm *ptm = localtime(&ATime);

      char cTemp[256];
      sprintf(cTemp,"%02d:%02d %04d-%02d-%02d",ptm->tm_hour,ptm->tm_min,1900+ptm->tm_year,ptm->tm_mon+1,ptm->tm_mday);              
                  
      pPlayer->StatsSince = cTemp;
    }
    
    pPlayer->nXMotoStarts++;
  }
      
	/*===========================================================================
	Define player/level if missing
  ===========================================================================*/
  bool Stats::_DefinePlayerAndLevel(PlayerStats **ppPlayer,LevelStats **ppLevel,
                                    const std::string &PlayerName,const std::string &LevelID,const std::string &LevelName) {
    bool bNewLevel = false;                                      

    *ppPlayer = _FindPlayerStats(PlayerName);
    if(*ppPlayer == NULL) {
      /* No player with that name, define it */
      *ppPlayer = new PlayerStats;
      (*ppPlayer)->PlayerName = PlayerName;
      m_Players.push_back(*ppPlayer);
      
      /* Note this time */
      time_t ATime;               
      time(&ATime);
      struct tm *ptm = localtime(&ATime);

      char cTemp[256];
      sprintf(cTemp,"%02d:%02d %04d-%02d-%02d",ptm->tm_hour,ptm->tm_min,1900+ptm->tm_year,ptm->tm_mon,ptm->tm_mday);              
                  
      (*ppPlayer)->StatsSince = cTemp;
    }
    
    *ppLevel = _FindLevelStats(*ppPlayer,LevelID,LevelName);
    if(*ppLevel == NULL) {
      /* No such level, define it */
      *ppLevel = new LevelStats;
      (*ppLevel)->LevelID = LevelID;
      (*ppLevel)->LevelName = LevelName;
      (*ppPlayer)->Levels.push_back(*ppLevel);
      bNewLevel = true;
    }
    
    return bNewLevel;
  }
      
	/*===========================================================================
	Locating things 
  ===========================================================================*/
  PlayerStats *Stats::_FindPlayerStats(const std::string &PlayerName) {
    for(int i=0;i<m_Players.size();i++)
      if(m_Players[i]->PlayerName == PlayerName) return m_Players[i];
    return NULL;
  }
  
  LevelStats *Stats::_FindLevelStats(PlayerStats *pPlayer,const std::string &LevelID,const std::string &LevelName) {
    for(int i=0;i<pPlayer->Levels.size();i++) {
      if(pPlayer->Levels[i]->LevelID == LevelID && pPlayer->Levels[i]->LevelName == LevelName)
        return pPlayer->Levels[i];
    }
    return NULL;
  }
      
	/*===========================================================================
	Cleaning up
  ===========================================================================*/
  void Stats::_FreeStats(void) {
    for(int i=0;i<m_Players.size();i++) {
      for(int j=0;j<m_Players[i]->Levels.size();j++) {
        delete m_Players[i]->Levels[j];
      }
      delete m_Players[i];
    }
    m_Players.clear();
  }

	/*===========================================================================
	Create ordered list of levels
  ===========================================================================*/
  std::vector<LevelStats *> Stats::_GetOrderedLevels(StatOrder Order,const std::string &PlayerName) {
    PlayerStats *pPlayer = _FindPlayerStats(PlayerName);
    std::vector<LevelStats *> Ret;
    if(pPlayer != NULL) {
      
      for(int i=0;i<pPlayer->Levels.size();i++) {
        /* Find place to insert this level */
        bool bInserted = false;
        for(int j=0;j<Ret.size();j++) {
          /* Better than this? */
          bool bBetter = false;
          
          switch(Order) {
            case ORDER_BY_COMPLETED:
              if(pPlayer->Levels[i]->nCompleted > Ret[j]->nCompleted) bBetter = true;
              break;
            case ORDER_BY_DEATHS:
              if(pPlayer->Levels[i]->nDied > Ret[j]->nDied) bBetter = true;
              break;
            case ORDER_BY_PLAYED:
              if(pPlayer->Levels[i]->nPlayed > Ret[j]->nPlayed) bBetter = true;
              break;
            case ORDER_BY_PLAYTIME:
              if(pPlayer->Levels[i]->fPlayTime > Ret[j]->fPlayTime) bBetter = true;
              break;
            case ORDER_BY_RESTARTS:
              if(pPlayer->Levels[i]->nRestarts > Ret[j]->nRestarts) bBetter = true;
              break;
          }
          
          if(bBetter) {
            /* Insert here */
            bInserted = true;
            Ret.insert(Ret.begin()+j,pPlayer->Levels[i]);
            break;
          }
        }       
        
        /* Not inserted? Hmm, append to end */        
        if(!bInserted) {                
          Ret.push_back(pPlayer->Levels[i]);
        }
      }
    }    
    
    return Ret;
  }

  int Stats::compareLevelMostPlayed(const Level& i_lvl1, const Level& i_lvl2, PlayerStats *i_playerStats) {
    int n1;

    for(unsigned int i=0; i<i_playerStats->Levels.size(); i++) {
      if(i_playerStats->Levels[i]->LevelID == i_lvl1.Id()) {
    	n1 = i_playerStats->Levels[i]->nPlayed;
      }
    }

    for(unsigned int i=0; i<i_playerStats->Levels.size(); i++) {
      if(i_playerStats->Levels[i]->LevelID == i_lvl2.Id()) {
    	if(i_playerStats->Levels[i]->nPlayed < n1) return  1;
    	if(i_playerStats->Levels[i]->nPlayed > n1) return -1;
    	return 0;
      }
    }
    
    return 0;
  }

}

