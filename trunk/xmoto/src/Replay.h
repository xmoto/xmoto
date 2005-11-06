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
#include "VFileIO.h"

namespace vapp {

  struct ReplayInfo {
    std::string Name;
    std::string Player;
    std::string Level;
    float fFrameRate;
  };
  
  struct ReplayStateChunk {
    char *pcChunkData;
    int nNumStates;
  };

  class Replay {
    public:
      Replay();
      ~Replay();
      
      /* Methods */
      void storeState(const char *pcState);
      bool loadState(char *pcState);      
      void createReplay(const std::string &FileName,const std::string &LevelID,const std::string &Player,float fFrameRate,int nStateSize);
      void saveReplay(void);
      std::string openReplay(const std::string &FileName,float *pfFrameRate,std::string &Player);
      void finishReplay(bool bFinished,float fFinishTime);
      void fastforward(float fSeconds);
      void fastrewind(float fSeconds);
      
      /* Static methods */
      static std::vector<ReplayInfo *> createReplayList(const std::string &PlayerName);
      static void freeReplayList(std::vector<ReplayInfo *> &List);
      
      /* Data interface */
      bool didFinish(void) {return m_bFinished;}
      float getFinishTime(void) {return m_fFinishTime;}      
    
    private: 
      /* Data */ 
      std::vector<ReplayStateChunk> m_Chunks;
      int m_nCurChunk,m_nCurState;      
      std::string m_FileName,m_LevelID,m_PlayerName;
      float m_fFrameRate;
      int m_nStateSize;
      bool m_bFinished;
      float m_fFinishTime;
  };

  //class Replay {
  //  public:
  //    Replay();
  //    ~Replay();
  //    
  //    /* Methods */
  //    void storeState(const char *pcState,int nStateSize);
  //    bool loadState(char *pcState,int nStateSize);
  //    void createReplay(const std::string &FileName,const std::string &LevelID,const std::string &Player,float fFrameRate);
  //    std::string openReplay(const std::string &FileName,float *pfFrameRate,std::string &Player);
  //    void finishReplay(void);
  //    void fastforward(int nStateSize,float fSeconds,float fFrameRate);
  //    void fastrewind(int nStateSize,float fSeconds,float fFrameRate);
  //    
  //    /* Static methods */
  //    static std::vector<ReplayInfo *> createReplayList(const std::string &PlayerName);
  //    static void freeReplayList(std::vector<ReplayInfo *> &List);
  //  
  //  private: 
  //    /* Data */ 
  //    FileHandle *m_pfh;
  //    int m_nVersion;      
  //    int m_nHeaderEnd;
  //};

};

#endif

