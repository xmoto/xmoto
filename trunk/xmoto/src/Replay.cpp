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

#include "Replay.h"
#if defined(WIN32) 
  #include "zlib.h"
#else
  #include <zlib.h>
#endif

namespace vapp {

  bool Replay::m_bEnableCompression = true;

  Replay::Replay() {
    m_bFinished = false;
    m_fFinishTime = 0.0f;
    m_pcInputEventsData = NULL;
    m_nInputEventsDataSize = 0;
  }
      
  Replay::~Replay() {
    _FreeReplay();
  }        
  
  void Replay::_FreeReplay(void) {
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
    FS::writeInt(pfh,0x12345678); /* Endianness guard */
    FS::writeString(pfh,m_LevelID);
    FS::writeString(pfh,m_PlayerName);
    FS::writeFloat(pfh,m_fFrameRate);
    FS::writeInt(pfh,m_nStateSize);
    FS::writeBool(pfh,m_bFinished);
    FS::writeFloat(pfh,m_fFinishTime);
    
    /* Events */
    const char *pcUncompressedEvents = convertOutputToInput();
    int nUncompressedEventsSize = numRemainingBytes();
    FS::writeInt(pfh,nUncompressedEventsSize);
    
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
        FS::writeInt(pfh,nDestLen);
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
    if(m_Chunks.empty()) FS::writeInt(pfh,0);
    else {        
      FS::writeInt(pfh,m_Chunks.size());
    
      /* Write chunks */    
      for(int i=0;i<m_Chunks.size();i++) {
        FS::writeInt(pfh,m_Chunks[i].nNumStates);

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
            FS::writeInt(pfh,nDestLen);
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
        Log("** Warning ** : Failed to open replay file for input: %s",(std::string("Replays/") + FileName).c_str());
        return "";
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
      if(FS::readInt(pfh) != 0x12345678) {
        FS::closeFile(pfh);
        Log("** Warning ** : Sorry, the replay you're trying to open are not endian-compatible with your computer!");
        return "";        
      }
    
      /* Read level ID */
      m_LevelID = FS::readString(pfh);
      
      /* Read player name */
      Player = m_PlayerName = FS::readString(pfh);
      
      /* Read replay frame rate */
      m_fFrameRate = FS::readFloat(pfh);
      if(pfFrameRate != NULL) *pfFrameRate = m_fFrameRate;            

      /* Read state size */
      m_nStateSize = FS::readInt(pfh);
      
      /* Read finish time if any */
      m_bFinished = FS::readBool(pfh);
      m_fFinishTime = FS::readFloat(pfh);
      
      /* Version 1 includes event data */
      if(nVersion == 1) {
        /* Read uncompressed size */
        m_nInputEventsDataSize = FS::readInt(pfh);
        m_pcInputEventsData = new char [m_nInputEventsDataSize];
        
        /* Compressed? */
        if(FS::readBool(pfh)) {
          /* Compressed */          
          int nCompressedEventsSize = FS::readInt(pfh);
          
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
      int nNumChunks = FS::readInt(pfh);
      for(int i=0;i<nNumChunks;i++) {        
        ReplayStateChunk Chunk;        
        Chunk.nNumStates = FS::readInt(pfh);
        Chunk.pcChunkData = new char [Chunk.nNumStates * m_nStateSize];
        
        /* Compressed or not compressed? */
        if(FS::readBool(pfh)) {
          /* Compressed! - read compressed size */
          int nCompressedSize = FS::readInt(pfh);
          
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
    m_nCurState = 0;
    
    return m_LevelID;
  }
      
  #define STATES_PER_CHUNK 512
  void Replay::storeState(const char *pcState) {
    if(m_Chunks.empty()) {
      ReplayStateChunk Chunk;
      Chunk.pcChunkData = new char [STATES_PER_CHUNK * m_nStateSize];
      Chunk.nNumStates = 1;
      memcpy(Chunk.pcChunkData,pcState,m_nStateSize);
      m_Chunks.push_back(Chunk);
    }
    else {
      int i = m_Chunks.size() - 1;
      if(m_Chunks[i].nNumStates < STATES_PER_CHUNK) {
        memcpy(&m_Chunks[i].pcChunkData[m_Chunks[i].nNumStates * m_nStateSize],pcState,m_nStateSize);
        m_Chunks[i].nNumStates++;
      }
      else {
        ReplayStateChunk Chunk;
        Chunk.pcChunkData = new char [STATES_PER_CHUNK * m_nStateSize];
        Chunk.nNumStates = 1;
        memcpy(Chunk.pcChunkData,pcState,m_nStateSize);
        m_Chunks.push_back(Chunk);        
      }
    }
  }
  
  bool Replay::loadState(char *pcState) {
    /* Read next state */
    if(m_nCurState >= STATES_PER_CHUNK) {
      m_nCurChunk++;
      m_nCurState = 0;
    }
    if(m_nCurChunk >= m_Chunks.size()) return false;    
    if(m_nCurState >= m_Chunks[m_nCurChunk].nNumStates) return false;
    
    memcpy(pcState,&m_Chunks[m_nCurChunk].pcChunkData[m_nCurState*m_nStateSize],m_nStateSize);
    m_nCurState++;
    return true;
  }
  
  bool Replay::peekState(char *pcState) {
    /* Like loadState() but this one does not advance the cursor... it just takes a peek */
    int nPeekChunk=m_nCurChunk,nPeekState=m_nCurState;
    if(nPeekState >= STATES_PER_CHUNK) {
      nPeekChunk++;
      nPeekState = 0;
    }
    if(nPeekChunk >= m_Chunks.size()) return false;
    if(nPeekState >= m_Chunks[nPeekChunk].nNumStates) return false;
    
    memcpy(pcState,&m_Chunks[nPeekChunk].pcChunkData[nPeekState*m_nStateSize],m_nStateSize);
    return true;
  }
  
  std::vector<ReplayInfo *> Replay::createReplayList(const std::string &PlayerName,const std::string &LevelIDCheck) {
    std::vector<ReplayInfo *> Ret;
    
    /* Find all replays done by the given player name */
    std::vector<std::string> ReplayFiles = FS::findPhysFiles("Replays/*.rpl");
    for(int i=0;i<ReplayFiles.size();i++) {
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
            
            if((PlayerName=="" || PlayerName==Player) && FS::getFileBaseName(ReplayFiles[i]) != "Latest") {
              if(LevelIDCheck=="" || LevelID==LevelIDCheck) {
                /* Fine. */
                ReplayInfo *pInfo = new ReplayInfo;
                pInfo->Level = LevelID;
                pInfo->Name = FS::getFileBaseName(ReplayFiles[i]);
                pInfo->Player = Player;
                pInfo->fFrameRate = fFrameRate;
                
                if(bFinished)
                  pInfo->fFinishTime = fFinishTime;
                else
                  pInfo->fFinishTime = -1;
                
                Ret.push_back(pInfo);
              }
            }
          }
        }
        else {
          /* Not supported */
        }
      
        FS::closeFile(pfh);
      }
    }
    
    /* Super. */
    return Ret;
  }
  
  void Replay::freeReplayList(std::vector<ReplayInfo *> &List) {
    /* Free list items */
    for(int i=0;i<List.size();i++)
      delete List[i];
    List.clear();
  }
  
  void Replay::fastforward(float fSeconds) {
    /* How many states should we move forward? */
    int nNumStates = (int)(fSeconds * m_fFrameRate);
    
    for(int i=0;i<nNumStates;i++) {
      /* Move one state forward */
      m_nCurState++;
      if(m_nCurState >= m_Chunks[m_nCurChunk].nNumStates) {
        m_nCurChunk++;
        if(m_nCurChunk >= m_Chunks.size()) {
          m_nCurChunk = m_Chunks.size() - 1;
          m_nCurState = m_Chunks[m_nCurChunk].nNumStates - 1;
          break;
        }
        else {
          m_nCurState = 0;
        }
      }
    }
  }  

  void Replay::fastrewind(float fSeconds) {
    /* How many states should we move backward? */
    int nNumStates = (int)(fSeconds * m_fFrameRate);

    for(int i=0;i<nNumStates;i++) {
      /* Move one state back */
      m_nCurState--;
      if(m_nCurState < 0) {
        m_nCurChunk--;
        if(m_nCurChunk < 0) {
          m_nCurChunk = 0;
          m_nCurState = 0;
          break;
        }
        else {
          m_nCurState = m_Chunks[m_nCurChunk].nNumStates - 1;
        }
      }
    }
  }  
  
};

