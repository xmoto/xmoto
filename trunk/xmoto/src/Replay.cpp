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
 *  Replay recording and management.
 */
#include "Replay.h"
#if defined(WIN32) 
  #include "zlib.h"
#else
  #include <zlib.h>
#endif

#include <time.h> 
#include <ctime>
#include <iostream>
#include <locale>

#include <arch/SwapEndian.h>

namespace vapp {

  bool Replay::m_bEnableCompression = true;

  Replay::Replay() {
    m_bFinished = false;
    m_bEndOfFile = false;
    m_fFinishTime = 0.0f;
    m_pcInputEventsData = NULL;
    m_nInputEventsDataSize = 0;
    m_speed_factor = 1.0;
    m_is_paused = false;
  }
      
  Replay::~Replay() {
    _FreeReplay();
  }        
  
  void Replay::_FreeReplay(void) {
    /* Get rid of replay events */
    for(int i=0;i<m_ReplayEvents.size();i++)
      delete m_ReplayEvents[i];
    m_ReplayEvents.clear();

    /* Dealloc chunks */
    for(int i=0;i<m_Chunks.size();i++) {
      if(m_Chunks[i].pcChunkData != NULL) {
        delete [] m_Chunks[i].pcChunkData;
      }
    }
    m_Chunks.clear();
    
    if(m_pcInputEventsData != NULL) delete [] m_pcInputEventsData;
  }
  
  void Replay::finishReplay(bool bFinished,float fFinishTime) {
    m_fFinishTime = fFinishTime;
    m_bFinished = bFinished;
  
    saveReplay();
  }
  
  void Replay::createReplay(const std::string &FileName,const std::string &LevelID,const std::string &Player,float fFrameRate,int nStateSize) {
    m_FileName = FileName;
    m_LevelID = LevelID;
    m_PlayerName = Player;
    m_fFrameRate = fFrameRate;
    m_nStateSize = nStateSize;    
    
    initOutput(1024);
  }
  
  void Replay::saveReplay(void) {
    /* Save it */
    FileHandle *pfh = FS::openOFile(std::string("Replays/") + m_FileName);
    if(pfh == NULL) {
      Log("** Warning ** : Failed to open replay file for output: %s",(std::string("Replays/") + m_FileName).c_str());
      return;
    }        
    
    /* Write header */
    FS::writeByte(pfh,1); /* Version: 1 */
    FS::writeInt_LE(pfh,0x12345678); /* Endianness guard */
    FS::writeString(pfh,m_LevelID);
    FS::writeString(pfh,m_PlayerName);
    FS::writeFloat_LE(pfh,m_fFrameRate);
    FS::writeInt_LE(pfh,m_nStateSize);
    FS::writeBool(pfh,m_bFinished);
    FS::writeFloat_LE(pfh,m_fFinishTime);
    
    /* Events */
    const char *pcUncompressedEvents = convertOutputToInput();
    int nUncompressedEventsSize = numRemainingBytes();
    FS::writeInt_LE(pfh,nUncompressedEventsSize);
    
    /* Compression? */
    if(m_bEnableCompression) {
      /* Compress events with zlib */
      char *pcCompressedEvents = new char [nUncompressedEventsSize * 2 + 12];
      uLongf nDestLen = nUncompressedEventsSize * 2 + 12;
      uLongf nSrcLen = nUncompressedEventsSize;
      int nZRet = compress2((Bytef *)pcCompressedEvents,&nDestLen,(Bytef *)pcUncompressedEvents,nSrcLen,9);
      if(nZRet != Z_OK) {
        /* Failed to compress, save raw events */
        FS::writeBool(pfh,false); /* compression: false */
        FS::writeBuf(pfh,(char *)pcUncompressedEvents,nUncompressedEventsSize);
      }
      else {
        /* OK */        
        FS::writeBool(pfh,true); /* compression: true */
        FS::writeInt_LE(pfh,nDestLen);
        FS::writeBuf(pfh,(char *)pcCompressedEvents,nDestLen);
      }
      delete [] pcCompressedEvents;
    }
    else {    
      /* No compression */
      FS::writeBool(pfh,false); /* compression: false */
      FS::writeBuf(pfh,(char *)pcUncompressedEvents,nUncompressedEventsSize);
    }
    
    /* Chunks */
    if(m_Chunks.empty()) FS::writeInt_LE(pfh,0);
    else {        
      FS::writeInt_LE(pfh,m_Chunks.size());
    
      /* Write chunks */    
      for(int i=0;i<m_Chunks.size();i++) {
        FS::writeInt_LE(pfh,m_Chunks[i].nNumStates);

        /* Compression enabled? */
        if(m_bEnableCompression) {
          /* Try compressing the chunk with zlib */        
          unsigned char *pcCompressed = new unsigned char[m_nStateSize * m_Chunks[i].nNumStates * 2 + 12];
          uLongf nDestLen = m_nStateSize * m_Chunks[i].nNumStates * 2 + 12;
          uLongf nSrcLen = m_nStateSize * m_Chunks[i].nNumStates;
          int nZRet = compress2((Bytef *)pcCompressed,&nDestLen,(Bytef *)m_Chunks[i].pcChunkData,nSrcLen,9);
          if(nZRet != Z_OK) {
            /* Failed to compress... Save uncompressed chunk then */
            FS::writeBool(pfh,false); /* compression: false */
            FS::writeBuf(pfh,m_Chunks[i].pcChunkData,m_nStateSize * m_Chunks[i].nNumStates);
          }
          else {
            /* Compressed ok */
            FS::writeBool(pfh,true); /* compression: true */
            FS::writeInt_LE(pfh,nDestLen);
            FS::writeBuf(pfh,(char *)pcCompressed,nDestLen);
          }
          delete [] pcCompressed;        
        }
        else {
          FS::writeBool(pfh,false); /* compression: false */
          FS::writeBuf(pfh,m_Chunks[i].pcChunkData,m_nStateSize * m_Chunks[i].nNumStates);
        }
      }
    }
    
    /* Clean up */
    FS::closeFile(pfh);
  }
  
  std::string Replay::openReplay(const std::string &FileName,float *pfFrameRate,std::string &Player) {

    /* Open file for input */
    FileHandle *pfh = FS::openIFile(std::string("Replays/") + FileName);
    if(pfh == NULL) {
      /* Try adding a .rpl extension */
      pfh = FS::openIFile(std::string("Replays/") + FileName + std::string(".rpl"));
      if(pfh == NULL) {    
        /* Finally try opening as if it is a full path */
        pfh = FS::openIFile(FileName);
        if(pfh == NULL) {
          Log("** Warning ** : Failed to open replay file for input: %s",(std::string("Replays/") + FileName).c_str());
          return "";
        }
      }
    }
    
    /* Read header */
    int nVersion = FS::readByte(pfh); 
        
    /* Supported version? */
    if(nVersion != 0 && nVersion != 1) {
      FS::closeFile(pfh);
      Log("** Warning ** : Unsupported replay file version (%d): %s",nVersion,(std::string("Replays/") + FileName).c_str());
      return "";
    }
    else {
      /* Little/big endian safety check */
      if(FS::readInt_LE(pfh) != 0x12345678) {
        FS::closeFile(pfh);
        Log("** Warning ** : Sorry, the replay you're trying to open are not endian-compatible with your computer!");
        return "";        
      }
    
      /* Read level ID */
      m_LevelID = FS::readString(pfh);

      /* Read player name */
      Player = m_PlayerName = FS::readString(pfh);
      
      /* Read replay frame rate */
      m_fFrameRate = FS::readFloat_LE(pfh);
      if(pfFrameRate != NULL) *pfFrameRate = m_fFrameRate;            

      /* Read state size */
      m_nStateSize = FS::readInt_LE(pfh);
      
      /* Read finish time if any */
      m_bFinished = FS::readBool(pfh);
      m_fFinishTime = FS::readFloat_LE(pfh);

      /* Version 1 includes event data */
      if(nVersion == 1) {
        /* Read uncompressed size */
        m_nInputEventsDataSize = FS::readInt_LE(pfh);
        m_pcInputEventsData = new char [m_nInputEventsDataSize];
        
        /* Compressed? */
        if(FS::readBool(pfh)) {
          /* Compressed */          
          int nCompressedEventsSize = FS::readInt_LE(pfh);
          
          char *pcCompressedEvents = new char [nCompressedEventsSize];
          FS::readBuf(pfh,pcCompressedEvents,nCompressedEventsSize);
          
          /* Unpack */
          uLongf nDestLen = m_nInputEventsDataSize;
          uLongf nSrcLen = nCompressedEventsSize;
          int nZRet = uncompress((Bytef *)m_pcInputEventsData,&nDestLen,(Bytef *)pcCompressedEvents,nSrcLen);
          if(nZRet != Z_OK || nDestLen != m_nInputEventsDataSize) {
            delete [] pcCompressedEvents;
            delete [] m_pcInputEventsData; m_pcInputEventsData = NULL;
            FS::closeFile(pfh);
            _FreeReplay();
            Log("** Warning ** : Failed to uncompress events in replay: %s",FileName.c_str());
            return "";
          }
          
          /* Clean up */
          delete [] pcCompressedEvents;
        }
        else {
          /* Not compressed */
          FS::readBuf(pfh,m_pcInputEventsData,m_nInputEventsDataSize);
        }

        /* Set up input stream */
        initInput(m_pcInputEventsData,m_nInputEventsDataSize);        
      }
      
      /* Read chunks */
      int nNumChunks = FS::readInt_LE(pfh);
      for(int i=0;i<nNumChunks;i++) {        
        ReplayStateChunk Chunk;        
        Chunk.nNumStates = FS::readInt_LE(pfh);
        Chunk.pcChunkData = new char [Chunk.nNumStates * m_nStateSize];
        
        /* Compressed or not compressed? */
        if(FS::readBool(pfh)) {
          /* Compressed! - read compressed size */
          int nCompressedSize = FS::readInt_LE(pfh);
          
          /* Read compressed data */
          unsigned char *pcCompressed = new unsigned char [nCompressedSize];
          FS::readBuf(pfh,(char *)pcCompressed,nCompressedSize);
         
          /* Uncompress it */           
          uLongf nDestLen = Chunk.nNumStates * m_nStateSize;
          uLongf nSrcLen = nCompressedSize;
          int nZRet = uncompress((Bytef *)Chunk.pcChunkData,&nDestLen,(Bytef *)pcCompressed,nSrcLen);
          if(nZRet != Z_OK || nDestLen != Chunk.nNumStates * m_nStateSize) {
            delete [] pcCompressed;
            delete [] Chunk.pcChunkData;
            FS::closeFile(pfh);
            _FreeReplay();
            Log("** Warning ** : Failed to uncompress chunk %d in replay: %s",i,FileName.c_str());
            return "";
          }
          
          /* Clean up */
          delete [] pcCompressed;
        }
        else {
          /* Not compressed! */
          FS::readBuf(pfh,Chunk.pcChunkData,m_nStateSize*Chunk.nNumStates);
        }
        
        m_Chunks.push_back(Chunk);
      }
    }
    
    /* Clean up */
    FS::closeFile(pfh);
    
    m_nCurChunk = 0;
    m_nCurState = 0.0;
    
            
    /* Reconstruct game events that are going to happen during the replay */
    MotoGame::unserializeGameEvents(this, &m_ReplayEvents);

    return m_LevelID;
  }
      
  void Replay::storeState(const SerializedBikeState& state) {
    char *addr;
    
    if(m_Chunks.empty()) {
      ReplayStateChunk Chunk;
      Chunk.pcChunkData = new char [STATES_PER_CHUNK * m_nStateSize];
      Chunk.nNumStates = 1;
      addr = Chunk.pcChunkData;
      m_Chunks.push_back(Chunk);
    }
    else {
      int i = m_Chunks.size() - 1;
      if(m_Chunks[i].nNumStates < STATES_PER_CHUNK) {
        addr = &m_Chunks[i].pcChunkData[m_Chunks[i].nNumStates * m_nStateSize];
        m_Chunks[i].nNumStates++;
      }
      else {
        ReplayStateChunk Chunk;
        Chunk.pcChunkData = new char [STATES_PER_CHUNK * m_nStateSize];
        Chunk.nNumStates = 1;
        addr = Chunk.pcChunkData;
        m_Chunks.push_back(Chunk);        
      }
    }
    
    memcpy(addr,(const char*)&state,m_nStateSize);
	SwapEndian::LittleSerializedBikeState(*(SerializedBikeState*)addr);
  }
  
  bool Replay::nextNormalState() {
    return nextState(m_speed_factor);
  }

  bool Replay::nextState(float p_frames) {
    m_bEndOfFile = false;
    if(m_is_paused) {
      return ! m_bEndOfFile;
    }

    m_nCurState += p_frames;

    /* end or start of a chunk */
    if(m_nCurState >= STATES_PER_CHUNK || m_nCurState < 0.0) {

      /* go on the following chunk */
      if(m_nCurState >= STATES_PER_CHUNK) { /* end of a chunk */
	if(m_nCurChunk < m_Chunks.size() -1) {	
	  m_nCurChunk++;
	  m_nCurState = m_nCurState - m_Chunks[m_nCurChunk-1].nNumStates -1;
	}
      } else {
	if(m_nCurState < 0.0) { /* start of a chunk */
	  if(m_nCurChunk > 0) {	
	    m_nCurChunk--;
	    m_nCurState = m_Chunks[m_nCurChunk-1].nNumStates -1 + m_nCurState;
	  }
	}
      }

      /* if that's the beginning */
      if(m_nCurState < 0.0) {
	m_nCurState = 0.0;
      }

      /* if that's end */
      if(m_nCurState >= m_Chunks[m_nCurChunk].nNumStates) {
	m_nCurState = m_Chunks[m_nCurChunk].nNumStates -1;
	return false;
      }
      
    } else { /* that's not the end or the start of the chunk */

      /* if that's the beginning, do nothing */

      /* if that's end */
      if(m_nCurState >= m_Chunks[m_nCurChunk].nNumStates) {
	m_nCurState = m_Chunks[m_nCurChunk].nNumStates -1;
	return false;
      }
    }
    
    return true;
  }

  void Replay::loadState(SerializedBikeState& state) {
    /* (11th july, 2006) rasmus: i've swapped the two following lines, it apparently fixes
       interpolation in replays, but i don't know if it break something else */  
    peekState(state);

    m_bEndOfFile = (m_nCurChunk       == m_Chunks.size()-1 && 
	 	    (int)m_nCurState  == m_Chunks[m_nCurChunk].nNumStates-1);

    if(nextNormalState()) { /* do nothing */ }
  }
  
  void Replay::peekState(SerializedBikeState& state) {
    /* Like loadState() but this one does not advance the cursor... it just takes a peek */
    memcpy((char *)&state,
	   &m_Chunks[m_nCurChunk].pcChunkData[((int)m_nCurState)*m_nStateSize],
	   m_nStateSize);
	SwapEndian::LittleSerializedBikeState(state);
  }

  //std::vector<ReplayInfo *> Replay::createReplayList(const std::string &PlayerName,const std::string &LevelIDCheck) {
  //  return ReplayList::findReplays(PlayerName,LevelIDCheck);
  
    //std::vector<ReplayInfo *> Ret;
    //
    ///* Find all replays done by the given player name */
    //std::vector<std::string> ReplayFiles = FS::findPhysFiles("Replays/*.rpl");
    //for(int i=0;i<ReplayFiles.size();i++) {
    //  /* Try opening it */
    //  FileHandle *pfh = FS::openIFile(ReplayFiles[i]);
    //  if(pfh != NULL) {
    //    int nVersion = FS::readByte(pfh);
    //    if(nVersion == 0 || nVersion == 1) {
    //      if(FS::readInt_LE(pfh) == 0x12345678) {                  
    //        std::string LevelID = FS::readString(pfh);
    //        std::string Player = FS::readString(pfh);
    //        float fFrameRate = FS::readFloat_LE(pfh);
    //        int nStateSize = FS::readInt_LE(pfh);
    //        bool bFinished = FS::readBool(pfh);
    //        float fFinishTime = FS::readFloat_LE(pfh);
    //        
    //        if((PlayerName=="" || PlayerName==Player) && FS::getFileBaseName(ReplayFiles[i]) != "Latest") {
    //          if(LevelIDCheck=="" || LevelID==LevelIDCheck) {
    //            /* Fine. */
    //            ReplayInfo *pInfo = new ReplayInfo;
    //            pInfo->Level = LevelID;
    //            pInfo->Name = FS::getFileBaseName(ReplayFiles[i]);
    //            pInfo->Player = Player;
    //            pInfo->fFrameRate = fFrameRate;
    //            
    //            if(bFinished)
    //              pInfo->fFinishTime = fFinishTime;
    //            else
    //              pInfo->fFinishTime = -1;
    //            
    //            Ret.push_back(pInfo);
    //          }
    //        }
    //      }
    //    }
    //    else {
    //      /* Not supported */
    //    }
    //  
    //    FS::closeFile(pfh);
    //  }
    //}
    //
    ///* Super. */
    //return Ret;
  //}
  
  //void Replay::freeReplayList(std::vector<ReplayInfo *> &List) {
    ///* Free list items */
    //for(int i=0;i<List.size();i++)
    //  delete List[i];
    //List.clear();
  //}
  
  void Replay::pause() {
    m_is_paused = ! m_is_paused;
  }

  void Replay::faster() {
    if(m_is_paused == false) {
      m_speed_factor += REPLAY_SPEED_INCREMENT;
    }
  }

  void Replay::slower() {
    if(m_is_paused == false) {
      m_speed_factor -= REPLAY_SPEED_INCREMENT;
    }
  }

  float Replay::getSpeed() const {
    if(m_is_paused) {
      return 0.0;
    }
    return m_speed_factor;
  }

  void Replay::fastforward(float fSeconds) {
    /* How many states should we move forward? */
    float nNumStates = (fSeconds * m_fFrameRate);
    nextState(nNumStates);
  }

  void Replay::fastrewind(float fSeconds) {
    /* How many states should we move forward? */
    float nNumStates = (fSeconds * m_fFrameRate);
    nextState(-nNumStates);
  }  

  std::string Replay::giveAutomaticName() {
    time_t date;
    time(&date);
    struct tm *TL = localtime(&date);

    char date_str[256];
    strftime(date_str, sizeof(date_str)-1, "%d-%m-%y %H_%M", TL);

    return std::string(date_str);
  }
  
  void Replay::reinitialize() {
    m_nCurChunk = 0;
    m_nCurState = 0.0;
    m_bEndOfFile = false;
    m_speed_factor = 1;
    m_is_paused = false;

    /* resetting events */
    for(int i=0;i<m_ReplayEvents.size();i++) {
      m_ReplayEvents[i]->bPassed = false;
    }
  }

  std::string Replay::getLevelId() {
    return m_LevelID;
  }

  void Replay::deleteReplay(std::string ReplayName) {
    FS::deleteFile(std::string("Replays/") + ReplayName + std::string(".rpl"));
  }

  ReplayInfo* Replay::getReplayInfos(const std::string p_ReplayName) {
    /* Found anything, and is the time stamp the same? If so, we don't 
       want to check this again */

      /* Try opening it */
      FileHandle *pfh = FS::openIFile("Replays/" + p_ReplayName + ".rpl");
      if(pfh == NULL) {
	return NULL;
      }

      int nVersion = FS::readByte(pfh);
      if(nVersion != 0 && nVersion != 1) {
	FS::closeFile(pfh);
	return NULL;
      }

      if(FS::readInt_LE(pfh) != 0x12345678) {   
	FS::closeFile(pfh);
	return NULL;
      }
  
      std::string LevelID = FS::readString(pfh);
      std::string Player  = FS::readString(pfh);
      float fFrameRate    = FS::readFloat_LE(pfh);
      int nStateSize      = FS::readInt_LE(pfh);
      bool bFinished      = FS::readBool(pfh);
      float fFinishTime   = FS::readFloat_LE(pfh);
                
      ReplayInfo *pRpl     = new ReplayInfo;
      int nTimeStamp       = FS::getFileTimeStamp("Replays/" + p_ReplayName + ".rpl");

      /* Set members */
      pRpl->Level = LevelID;
      pRpl->Name = p_ReplayName;
      pRpl->Player = Player;
      pRpl->nTimeStamp = nTimeStamp;
      pRpl->fFrameRate = fFrameRate;

      if(bFinished) {
	pRpl->fFinishTime = fFinishTime;
      } else {
	pRpl->fFinishTime = -1;                
      }
          
      FS::closeFile(pfh);
      return pRpl;
  }

}
