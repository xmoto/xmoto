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

/* 
 *  Replay recording and management.
 */
 
/* Broken replays... A big problem for X-Moto, because it is such an 
   essential feature. In short why it doesn't work: Floating-point 
   inconsistency. */
 
#include "VFileIO.h"
#include "Replay.h"

namespace vapp {

  /*===========================================================================
  Replay loading
  ===========================================================================*/
  void Replay::loadFromFile(std::string Name) {
    /* Ok, i'd love to use XML for this (like everything else), but this
       really needs to be optimized for size. :( */
    FileHandle *pfh = FS::openIFile(std::string("Replays/")+Name);
    if(pfh == NULL) {
      Log("** Warning ** : failed to open replay '%s'",Name.c_str());
      return;
    }
    
    /* Read build version (major) -- this should match our */
    if(FS::readShort(pfh) != BUILD_VERSION) {
      Log("** Warning ** : replay '%s' major version mismatch - can't play that back",Name.c_str());      
    }
    else {
      /* Read header */
      std::string LevelID = FS::readString(pfh);
      std::string PlayerName = FS::readString(pfh);
      float fFinishTime = FS::readFloat(pfh);
      int nNumChunks = FS::readInt(pfh);
      
      /* Read chunks */
      for(int i=0;i<nNumChunks;i++) {
        /* Read chunk header */
        int nNumEvents = FS::readInt(pfh);
        
        /* Read events */
        for(int j=0;j<nNumEvents;j++) {           
          int nFrame = FS::readInt(pfh);
          //ReplayEventType Type = (ReplayEventType)FS::readByte(pfh);
          //event(nFrame,Type);
        }        
      }
      
      /* Set stuff */
      setLevelID(LevelID);
      setPlayerName(PlayerName);
      setFinishTime(fFinishTime);
    }
        
    /* Clean up */
    FS::closeFile(pfh);
  }
  
  /*===========================================================================
  Replay saving
  ===========================================================================*/
  void Replay::saveToFile(std::string Name) {
    /* Save replay */
    FileHandle *pfh = FS::openOFile(std::string("Replays/")+Name);
    if(pfh == NULL) {
      Log("** Warning ** : failed to write replay to '%s'",Name.c_str());
      return;
    }

    /* Write build version (major) */   
    FS::writeShort(pfh,BUILD_VERSION);
    
    /* Write extra header info */    
    FS::writeString(pfh,getLevelID());
    FS::writeString(pfh,getPlayerName());
    FS::writeFloat(pfh,getFinishTime());
    //FS::writeInt(pfh,m_EventChunks.size());
    //
    ///* Write chunks */
    //for(int i=0;i<m_EventChunks.size();i++) {
    //  /* Write chunk header */
    //  FS::writeInt(pfh,m_EventChunks[i]->nNumEvents);
    //  
    //  /* Write events */
    //  for(int j=0;j<m_EventChunks[i]->nNumEvents;j++) {
    //    FS::writeInt(pfh,m_EventChunks[i]->Events[j].nFrame);
    //    FS::writeByte(pfh,(unsigned char)m_EventChunks[i]->Events[j].Type);
    //  }
    //}
    
    /* Clean up */
    FS::closeFile(pfh);
  }
  
  /*===========================================================================
  Add event to replay
  ===========================================================================*/  
  //void Replay::event(int nFrame,ReplayEventType Type) {
  //  ReplayEventChunk *pChunk;
  //
  //  /* Find out what chunk */
  //  if(m_EventChunks.empty() || 
  //     m_EventChunks[m_EventChunks.size()-1]->nNumEvents == MAX_CHUNK_EVENTS) {
  //    /* New one */
  //    pChunk = new ReplayEventChunk;
  //    m_EventChunks.push_back(pChunk);
  //    pChunk->nNumEvents = 0;
  //  }
  //  else {
  //    pChunk = m_EventChunks[m_EventChunks.size()-1];
  //  }
  //  
  //  //printf("Frame %d: %s\n",nFrame,eventName(Type).c_str());
  //  
  //  /* Add event to it */
  //  pChunk->Events[pChunk->nNumEvents].nFrame = nFrame;
  //  pChunk->Events[pChunk->nNumEvents].Type = Type;
  //  pChunk->nNumEvents++;
  //}

  /*===========================================================================
  Real cleaning up
  ===========================================================================*/  
  void Replay::_Free(void) {
    //for(int i=0;i<m_EventChunks.size();i++)
    //  delete m_EventChunks[i];
    //m_EventChunks.clear();
  }

  /*===========================================================================
  Static methods for probing replays
  ===========================================================================*/  
  std::vector<ReplayInfo *> Replay::probeReplays(void) {
    /* Scan for all replays */
    std::vector<std::string> ReplayFiles = FS::findPhysFiles("Replays/*.rpl");
    
    /* Analyze them */
    std::vector<ReplayInfo *> Ret;
    
    //for(int i=0;i<ReplayFiles.size();i++) {   
    //  FileHandle *pfh = FS::openIFile(ReplayFiles[i]);
    //  if(pfh != NULL) {
    //    if(FS::readShort(pfh) == BUILD_VERSION) {
    //      std::string LevelID = FS::readString(pfh);
    //      std::string PlayerName = FS::readString(pfh);
    //      float fFinishTime = FS::readFloat(pfh);
    //      int nNumChunks = FS::readInt(pfh);
    //      
    //      ReplayInfo *pi = new ReplayInfo;
    //      pi->fFinishTime = fFinishTime;
    //      pi->PlayerName = PlayerName;
    //      pi->LevelID = LevelID;
    //      pi->Name = FS::getFileBaseName(ReplayFiles[i]);
    //      Ret.push_back(pi);
    //    }    
    //  
    //    FS::closeFile(pfh);        
    //  }
    //}
    
    return Ret;
  }

  void Replay::freeReplayList(std::vector<ReplayInfo *> &List) {
    for(int i=0;i<List.size();i++)
      delete List[i];
  }

  /*===========================================================================
  Static helper methods
  ===========================================================================*/  
  //std::string Replay::eventName(ReplayEventType Type) {
  //  switch(Type) {
  //    case REPLAY_EVENT_START_DRIVING: return "StartDriving";
  //    case REPLAY_EVENT_STOP_DRIVING: return "StopDriving";
  //    case REPLAY_EVENT_START_BRAKING: return "StartBraking";
  //    case REPLAY_EVENT_STOP_BRAKING: return "StopBraking";
  //    case REPLAY_EVENT_CHANGE_DIRECTION: return "ChangeDirection";
  //    case REPLAY_EVENT_START_PUSHING_FORWARD: return "StartPushingForward";
  //    case REPLAY_EVENT_STOP_PUSHING_FORWARD: return "StopPushingForward";
  //    case REPLAY_EVENT_START_PULLING_BACKWARD: return "StartPullingBackward";
  //    case REPLAY_EVENT_STOP_PULLING_BACKWARD: return "StopPullingBackward";
  //  }
  //  return "Undefined";
  //}
  
};

