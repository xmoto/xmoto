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

#ifndef __REPLAY_H__
#define __REPLAY_H__

#include "VCommon.h"
#include "VApp.h"
#include "VFileIO.h"
#include "DBuffer.h"
#include "MotoGame.h"

#define REPLAY_SPEED_INCREMENT 0.25
#define STATES_PER_CHUNK 512

namespace vapp {

  /*===========================================================================
  Structs
  ===========================================================================*/  
  /* Replay information - an array of these suckers are returned when you
     probe for replays on a computer */
  struct ReplayInfo {
    std::string Name;
    std::string Player;
    std::string Level;
    float fFrameRate;
    float fFinishTime;
    
    int nTimeStamp;   /* replay file time stamp */
  };
  
  /* Replays states (frames) are grouped together in chunks for easy
     processing */
  struct ReplayStateChunk {
    char *pcChunkData;
    int nNumStates;
  };

  /*===========================================================================
  ReplayList class
  ===========================================================================*/  
  class ReplayList {
    public:
      ~ReplayList() {clear();} /* destructor */
    
      /* Methods */
      void initFromDir();
      void addReplay(const std::string &Replay);
      void delReplay(const std::string &Replay);
      void clear(void);

      std::vector<ReplayInfo *>* findReplays(const std::string &PlayerName = "",
					     const std::string &LevelID = "");
      
    private:
      /* Data */
      std::vector<ReplayInfo *> m_Replays;
  };
   
  /*===========================================================================
  Replay class
  ===========================================================================*/  
  class Replay : public DBuffer {
    public:
      Replay();
      virtual ~Replay();
      
      /* Methods */
      void storeState(const SerializedBikeState& state);
      /* go and get the next state */
      void loadState(SerializedBikeState& state);  
      void peekState(SerializedBikeState& state); /* get current state */
      
      void createReplay(const std::string &FileName,const std::string &LevelID,const std::string &Player,float fFrameRate,int nStateSize);
      void saveReplay(void);
      std::string openReplay(const std::string &FileName,float *pfFrameRate,std::string &Player);
      static void deleteReplay(std::string ReplayName);
      void reinitialize();
      std::string getLevelId();

      void finishReplay(bool bFinished,float fFinishTime);
      void fastforward(float fSeconds);
      void fastrewind(float fSeconds);
      void pause();
      void faster();
      void slower();
      float getSpeed() const; /* get multiple factor of the replay */
      
      void setSpeed(float f) {m_speed_factor = f;}
      bool isPaused(void) {return m_is_paused;}

      /* Data interface */
      bool didFinish(void) {return m_bFinished;}
      float getFinishTime(void) {return m_fFinishTime;}     
      float getFrameRate(void) {return m_fFrameRate;} 
      const std::string &getPlayerName(void) {return m_PlayerName;}
      bool endOfFile(void) {return m_bEndOfFile;}
      
      /* Static data interface */
      static void enableCompression(bool b) {m_bEnableCompression = b;}
    
      static std::string giveAutomaticName();

      std::vector<RecordedGameEvent *> *getEvents() {return &m_ReplayEvents;}

      /* return NULL if the replay is not valid */
      static ReplayInfo* getReplayInfos(const std::string p_ReplayName);

    private: 
      /* Data */ 
      std::vector<ReplayStateChunk> m_Chunks;
      int m_nCurChunk;
      float m_nCurState; /* is a float so that manage slow */   
      std::string m_FileName,m_LevelID,m_PlayerName;
      float m_fFrameRate;
      int m_nStateSize;
      bool m_bFinished;
      bool m_bEndOfFile;
      float m_fFinishTime;
      char *m_pcInputEventsData;
      int m_nInputEventsDataSize;

      float m_speed_factor; /* nb frame to increment each time ;
			       is a float so that manage slow */
      bool  m_is_paused;
 
      /* Helpers */
      void _FreeReplay(void);
      bool nextState(float p_frames); /* go to the next state */
      bool nextNormalState(); /* go to the next state */
      
      /* Static data */
      static bool m_bEnableCompression;

      /* Events reconstructed from replay */
      std::vector<RecordedGameEvent *> m_ReplayEvents;
  };

}

#endif

