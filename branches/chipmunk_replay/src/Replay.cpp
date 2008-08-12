/*=============================================================================
XMOTO

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

#include "helpers/Log.h"
#include "helpers/SwapEndian.h"
#include "GameEvents.h"
#include "Game.h"
#include "xmscene/Bike.h"
#include "xmscene/Block.h"
#include "xmscene/Level.h"
#include "db/xmDatabase.h"

bool Replay::m_bEnableCompression = true;

Replay::Replay() {
  m_bFinished = false;
  m_bEndOfFile = false;
  m_finishTime = 0;
  m_pcInputEventsData = NULL;
  m_nInputEventsDataSize = 0;
  m_pcInputChipmunkData = NULL;
  m_nInputChipmunkDataSize = 0;
  m_version = 0;
  m_displayInformation = false;
  m_curPos = 0;
}

Replay::~Replay() {
  _FreeReplay();
}        

void Replay::_FreeReplay(void) 
{
  /* Get rid of replay events */
  for(unsigned int i=0;i<m_replayEvents.size();i++) {
    delete m_replayEvents[i]->Event;
    delete m_replayEvents[i];
  }
  m_replayEvents.clear();

  /* Dealloc chunks */
  for(unsigned int i=0;i<m_Chunks.size();i++) {
    if(m_Chunks[i]->pcChunkData != NULL) {
      delete [] m_Chunks[i]->pcChunkData;
    }
    delete m_Chunks[i];
  }
  m_Chunks.clear();

#define DELETE_NULLIFY(data) \
  if((data) != NULL) { \
    delete [] (data);  \
    (data) = NULL;     \
  }

  DELETE_NULLIFY(m_pcInputEventsData);
  DELETE_NULLIFY(m_pcInputChipmunkData);
}

void Replay::finishReplay(bool bFinished, int finishTime) {
  m_finishTime = finishTime;
  m_bFinished = bFinished;
  saveReplay();
}

void Replay::createReplay(const std::string &FileName,
			  const std::string &LevelID,
			  const std::string &Player, float fFrameRate,
			  unsigned int nStateSize, bool physicalReplay) {
  m_FileName   = FileName;
  m_LevelID    = LevelID;
  m_PlayerName = Player;
  m_fFrameRate = fFrameRate;
  m_nStateSize = nStateSize;    
  if(physicalReplay == false)
    m_version = 1;
  else
    m_version = 2;

  m_replayEventsBuffer.initOutput(1024);
}

void Replay::saveReplay(void) {
  FileHandle* pfh = FS::openOFile(std::string("Replays/") + m_FileName);
  if(pfh == NULL) {
    LogWarning("Failed to open replay file for output: %s",(std::string("Replays/") + m_FileName).c_str());
    return;
  }        

  // Write header
  saveHeader(pfh);

  /* Events */
  saveDBuffer(pfh, m_replayEventsBuffer);

  // chipmunk blocks
  if(m_version >= 2) {
    saveDBuffer(pfh, m_chipmunkFramesBuffer);
  }

  /* Chunks */
  saveChunks(pfh);

  FS::closeFile(pfh);
}

std::string Replay::openReplay(const std::string &FileName, std::string &Player, bool bDisplayInformation) {
  m_displayInformation = bDisplayInformation;
  /* Try opening as if it is a full path */
  FileHandle* pfh = FS::openIFile(FileName, true);
  if(pfh == NULL) {
    /* Open file for input */
    pfh = FS::openIFile(std::string("Replays/") + FileName);
    if(pfh == NULL) {
      /* Try adding a .rpl extension */
      pfh = FS::openIFile(std::string("Replays/") + FileName + std::string(".rpl"));
      if(pfh == NULL) {    
	LogWarning("Failed to open replay file for input: %s",(std::string("Replays/") + FileName).c_str());
	throw Exception("Unable to open the replay");
      }
    }
  }

  loadHeader(pfh);

  /* Version 1 includes event data */
  if(m_version >= 1) {
    loadDBuffer(pfh, m_replayEventsBuffer,
		&m_pcInputEventsData,
		&m_nInputEventsDataSize);
  }

  // version 2 include chipmunk blocks
  if(m_version == 2) {
    loadDBuffer(pfh, m_chipmunkFramesBuffer,
		&m_pcInputChipmunkData,
		&m_nInputChipmunkDataSize);
  }

  /* Read chunks */
  loadChunks(pfh);

  /* Clean up */
  FS::closeFile(pfh);
    
  m_nCurChunk = 0;
  m_nCurState = 0.0;
    
  /* Reconstruct game events that are going to happen during the replay */
  if(m_displayInformation) {
    LogInfo("%-30s:\n", "Game Events");
  }

  /* unserialize events */
  MotoGame::unserializeGameEvents(&m_replayEventsBuffer, &m_replayEvents, m_displayInformation);
  m_replayEventsBuffer.initOutput(1024);
  for(unsigned int i=0; i<m_replayEvents.size(); i++) {
    m_replayEvents[i]->Event->serialize(m_replayEventsBuffer);
  }

  if(m_version == 2) {
    // unserialize chipmunk frames
    MotoGame::unserializeChipmunkFrames(&m_chipmunkFramesBuffer, m_chipmunkFrames, m_displayInformation);
  }

  return m_LevelID;
}

void Replay::storeBlocks(const std::vector<Block*>& i_blocks, int curTime) {
  std::vector<Block*> movedPhysicalBlocks;

  for(unsigned int i=0; i<i_blocks.size(); i++) {
    if(i_blocks[i]->isPhysics() == true
       && i_blocks[i]->hasMoved() == true) {
      movedPhysicalBlocks.push_back(i_blocks[i]);
      i_blocks[i]->hasMoved(false);
    }
  }

  if(movedPhysicalBlocks.size() == 0)
    return;

  m_chipmunkFramesBuffer << CurrentFrame();
  m_chipmunkFramesBuffer << curTime;
  m_chipmunkFramesBuffer << movedPhysicalBlocks.size();

  std::vector<Block*>::iterator it = movedPhysicalBlocks.begin();
  for(; it != movedPhysicalBlocks.end(); ++it) {
    (*it)->serialize(m_chipmunkFramesBuffer);
  }

  movedPhysicalBlocks.clear();
}

void Replay::storeState(const SerializedBikeState& state) {
  char *addr;

  if(m_Chunks.size() == 0) {
    ReplayStateChunk* Chunk = new ReplayStateChunk();
    Chunk->pcChunkData = new char [STATES_PER_CHUNK * m_nStateSize];
    Chunk->nNumStates = 1;
    addr = Chunk->pcChunkData;
    m_Chunks.push_back(Chunk);
  } else {
    int i = m_Chunks.size() - 1;
    if(m_Chunks[i]->nNumStates < STATES_PER_CHUNK) {
      addr = &m_Chunks[i]->pcChunkData[m_Chunks[i]->nNumStates * m_nStateSize];
      m_Chunks[i]->nNumStates++;
    }
    else {
      ReplayStateChunk* Chunk = new ReplayStateChunk();
      Chunk->pcChunkData = new char [STATES_PER_CHUNK * m_nStateSize];
      Chunk->nNumStates = 1;
      addr = Chunk->pcChunkData;
      m_Chunks.push_back(Chunk);        
    }
  }
    
  memcpy(addr,(const char*)&state,m_nStateSize);
  SwapEndian::LittleSerializedBikeState(*(SerializedBikeState*)addr);
}
  
int Replay::CurrentFrame() const {
  return (int)(m_nCurChunk * STATES_PER_CHUNK + m_nCurState + 1);
}

bool Replay::nextNormalState() {
  return nextState(1);
}

bool Replay::nextState(int p_frames) {
  m_bEndOfFile = false;

  m_nCurState += p_frames;

  /* end or start of a chunk */
  if(m_nCurState >= STATES_PER_CHUNK || m_nCurState < 0.0) {

    /* go on the following chunk */
    if(m_nCurState >= STATES_PER_CHUNK) { /* end of a chunk */
      if(m_nCurChunk < m_Chunks.size() - 1) {
	m_nCurState = m_nCurState - m_Chunks[m_nCurChunk]->nNumStates -1;
	m_nCurChunk++;
      }
    } else {
      if(m_nCurState < 0.0) { /* start of a chunk */
	if(m_nCurChunk > 0) {
	  m_nCurState = m_nCurState + m_Chunks[m_nCurChunk-1]->nNumStates -1;
	  m_nCurChunk--;
	}
      }
    }

    /* if that's the beginning */
    if(m_nCurState < 0.0) {
      m_nCurState = 0.0;
    }

    /* if that's end */
    if(m_nCurState >= m_Chunks[m_nCurChunk]->nNumStates) {
      m_nCurState = m_Chunks[m_nCurChunk]->nNumStates -1;
      return false;
    }
      
  } else { /* that's not the end or the start of the chunk */

    /* if that's the beginning, do nothing */

    /* if that's end */
    if(m_nCurState >= m_Chunks[m_nCurChunk]->nNumStates) {
      m_nCurState = m_Chunks[m_nCurChunk]->nNumStates -1;
      return false;
    }
  }
    
  return true;
}

void Replay::loadState(BikeState* state) {
  /* (11th july, 2006) rasmus: i've swapped the two following lines, it apparently fixes
     interpolation in replays, but i don't know if it break something else */  
  peekState(state);

  m_bEndOfFile = (m_nCurChunk       == m_Chunks.size()-1 && 
		  (int)m_nCurState  == m_Chunks[m_nCurChunk]->nNumStates-1);

  if(m_bEndOfFile == false) {
    if(nextNormalState()) { /* do nothing */ }
  }
}
  
void Replay::peekState(BikeState* state) {
  SerializedBikeState v_bs;

  /* Like loadState() but this one does not advance the cursor... it just takes a peek */
  memcpy((char *)&v_bs, &m_Chunks[m_nCurChunk]->pcChunkData[((int)m_nCurState)*m_nStateSize], m_nStateSize);
  SwapEndian::LittleSerializedBikeState(v_bs);

  BikeState::convertStateFromReplay(&v_bs, state);
}

void Replay::updatePhysBlocksToFrame(int frame, Level* curLvl)
{
  if(frame <= m_chipmunkFrames[m_curPos]->frameId)
    return;

  while(m_chipmunkFrames[m_curPos]->frameId < frame) {
    ChipmunkFrame* pFrame = m_chipmunkFrames[m_curPos];
    std::vector<ChipmunkBlockState>::iterator it = pFrame->blockStates.begin();
    while(it != pFrame->blockStates.end()) {
      Block* pBlock = curLvl->getBlockById((*it).id);

      pBlock->setDynamicPosition(Vector2f((*it).posX, (*it).posY));
      pBlock->setDynamicRotation((*it).rot);

      ++it;
    }

    if(m_curPos >= m_chipmunkFrames.size())
      return;
    m_curPos++;
  }
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

  /* resetting events */
  for(unsigned int i=0;i<m_replayEvents.size();i++) {
    m_replayEvents[i]->bPassed = false;
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
    pfh = FS::openIFile(p_ReplayName, true);
    if(pfh == NULL) {
      LogError("cant open replay file [%s].",
	       p_ReplayName.c_str());
      return NULL;
    }
  }
      
  int nVersion = FS::readByte(pfh);
  if(nVersion < 0 || nVersion > 2) {
    FS::closeFile(pfh);
    LogError("Replay [%s] has an incompatible version %d.",
	     p_ReplayName.c_str(), nVersion);
    return NULL;
  }
      
  if(FS::readInt_LE(pfh) != 0x12345678) {   
    FS::closeFile(pfh); 
    LogError("Replay [%s] has a bad magic number.",
	     p_ReplayName.c_str(), nVersion);
   return NULL;
  }
  
  std::string LevelID = FS::readString(pfh);
  std::string Player  = FS::readString(pfh);
  /*float fFrameRate    = */ FS::readFloat_LE(pfh);
  /*int nStateSize      = */ FS::readInt_LE(pfh);
  bool bFinished      = FS::readBool(pfh);
  int finishTime   = GameApp::floatToTime(FS::readFloat_LE(pfh));
                
  ReplayInfo *pRpl     = new ReplayInfo;

  /* Set members */
  pRpl->Level      = LevelID;
  pRpl->Name       = p_ReplayName;
  pRpl->Player     = Player;
  pRpl->IsFinished = bFinished;

  if(bFinished) {
    pRpl->finishTime = finishTime;
  } else {
    pRpl->finishTime = -1;                
  }
          
  FS::closeFile(pfh);
  return pRpl;
}

void Replay::fastforward(int i_time) {
  /* How many states should we move forward? */
  int nNumStates = (int)((i_time * m_fFrameRate) / 100);
  nextState(nNumStates);
}

void Replay::fastrewind(int i_time, int i_minimumNbFrame) {
  /* How many states should we move forward? */
  int nNumStates = (int)( (i_time * m_fFrameRate) / 100);
  nextState(nNumStates < i_minimumNbFrame ? -i_minimumNbFrame : -nNumStates);
} 

void Replay::cleanReplays(xmDatabase *i_db) {
}

int  Replay::cleanReplaysNb(xmDatabase *i_db) {
  //char **v_result;
  //unsigned int nrow;
  int n;

  /* SELECT replay of from replay_% and %-%-% %_% which are not best replays for the (id_level, id_room, id_profile) if id_profile is in profiles or (id_level, id_room) else */

  //  SELECT name
  //  FROM replays AS a
  //  LEFT OUTER JOIN webhighscores AS b ON (a.id_level = b.id_level AND b.fileUrl NOT LIKE '%/' + a.name + '.rpl')
  //  WHERE a.name LIKE 'replay_%'
  //  AND   
  // ;

  //  v_result = i_db->readDB("SELECT count(name) FROM replays;", nrow);
  //  if(nrow != 1) {
  //    i_db->read_DB_free(v_result);
  //    throw Exception("Unable to determine the number of replays to clean");
  //  }
  //  n = atoi(i_db->getResult(v_result, 1, 0, 0));
  //  i_db->read_DB_free(v_result);
  n = 0;

  return n;
}

void Replay::saveDBuffer(FileHandle* pfh, DBuffer& buffer)
{
  const char *pcUncompressed = buffer.convertOutputToInput();
  int nUncompressedSize      = buffer.numRemainingBytes();
  FS::writeInt_LE(pfh, nUncompressedSize);

  if(m_bEnableCompression) {
    /* Compress with zlib */
    char *pcCompressedEvents = new char [nUncompressedSize * 2 + 12];
    uLongf nDestLen = nUncompressedSize * 2 + 12;
    uLongf nSrcLen  = nUncompressedSize;
    const int compressionLevel = 9;
    int nZRet = compress2((Bytef *)pcCompressedEvents, &nDestLen,
			  (Bytef *)pcUncompressed, nSrcLen,
			  compressionLevel);
    if(nZRet != Z_OK) {
      /* Failed to compress, save raw events */
      FS::writeBool(pfh, false);
      FS::writeBuf(pfh,(char *)pcUncompressed, nUncompressedSize);
    }
    else {
      /* OK */        
      FS::writeBool(pfh, true);
      FS::writeInt_LE(pfh, nDestLen);
      FS::writeBuf(pfh, (char *)pcCompressedEvents, nDestLen);
    }
    delete [] pcCompressedEvents;
  }
  else {    
    /* No compression */
    FS::writeBool(pfh, false);
    FS::writeBuf(pfh, (char *)pcUncompressed, nUncompressedSize);
  }  
}


void Replay::loadDBuffer(FileHandle* pfh, DBuffer& o_buffer, char** o_data, unsigned int* o_size)
{
  /* Read uncompressed size */
  (*o_size) = FS::readInt_LE(pfh);
  if(m_displayInformation) {
    LogInfo("%-30s: %i\n", "Data size", *o_size);
  }

  (*o_data) = new char [m_nInputEventsDataSize];
        
  if(FS::readBool(pfh)) {
    /* Compressed */          
    int nCompressedSize = FS::readInt_LE(pfh);
    if(m_displayInformation) {
      LogInfo("%-30s: %i\n", "Compressed data size", nCompressedSize);
    } 
      
    char *pcCompressed = new char [nCompressedSize];
    FS::readBuf(pfh, pcCompressed, nCompressedSize);
      
    /* Unpack */
    uLongf nDestLen = (*o_size);
    uLongf nSrcLen = nCompressedSize;
    int nZRet = uncompress((Bytef *)(*o_data), &nDestLen,
			   (Bytef *)pcCompressed, nSrcLen);
    if(nZRet != Z_OK || nDestLen != (*o_size)) {
      delete [] pcCompressed;
      delete [] (*o_data);
      (*o_data) = NULL;
      FS::closeFile(pfh);
      _FreeReplay();
      LogWarning("Failed to uncompress data in replay");
      throw Exception("Unable to open the replay");
    }

    delete [] pcCompressed;
  }
  else {
    /* Not compressed */
    FS::readBuf(pfh, *o_data, *o_size);
  }

  /* Set up input stream */
  o_buffer.initInput(*o_data, *o_size);
}

void Replay::saveChunks(FileHandle* pfh)
{
  FS::writeInt_LE(pfh, m_Chunks.size());

  /* Write chunks */    
  for(unsigned int i=0;i<m_Chunks.size();i++) {
    FS::writeInt_LE(pfh,m_Chunks[i]->nNumStates);

    /* Compression enabled? */
    if(m_bEnableCompression) {
      /* Try compressing the chunk with zlib */        
      unsigned char *pcCompressed = new unsigned char[m_nStateSize * m_Chunks[i]->nNumStates * 2 + 12];
      uLongf nDestLen = m_nStateSize * m_Chunks[i]->nNumStates * 2 + 12;
      uLongf nSrcLen = m_nStateSize * m_Chunks[i]->nNumStates;
      int nZRet = compress2((Bytef *)pcCompressed,&nDestLen,(Bytef *)m_Chunks[i]->pcChunkData,nSrcLen,9);
      if(nZRet != Z_OK) {
	/* Failed to compress... Save uncompressed chunk then */
	FS::writeBool(pfh,false); /* compression: false */
	FS::writeBuf(pfh,m_Chunks[i]->pcChunkData,m_nStateSize * m_Chunks[i]->nNumStates);
      } else {
	/* Compressed ok */
	FS::writeBool(pfh,true); /* compression: true */
	FS::writeInt_LE(pfh,nDestLen);
	FS::writeBuf(pfh,(char *)pcCompressed,nDestLen);
      }
      delete [] pcCompressed;        
    } else {
      FS::writeBool(pfh,false); /* compression: false */
      FS::writeBuf(pfh,m_Chunks[i]->pcChunkData,m_nStateSize * m_Chunks[i]->nNumStates);
    }
  }
}

void Replay::loadChunks(FileHandle* pfh)
{
  unsigned int nNumChunks = FS::readInt_LE(pfh);
  if(m_displayInformation) {
    LogInfo("%-30s: %i\n", "Number of chunks", nNumChunks);
  }
  if(nNumChunks == 0) {
    FS::closeFile(pfh);
    _FreeReplay();
    LogWarning("try to open a replay with no chunk");
    throw Exception("Replay with no chunk !");
  }

  for(unsigned int i=0; i<nNumChunks; i++) {
    if(m_displayInformation) {
      LogInfo("Chunk %02i\n", i);
    }  
  
      ReplayStateChunk* Chunk = new ReplayStateChunk();
      Chunk->nNumStates = FS::readInt_LE(pfh);

      if(m_displayInformation) {
	LogInfo("   %-27s: %i\n", "Number of states", Chunk->nNumStates);
      } 

      Chunk->pcChunkData = new char [Chunk->nNumStates * m_nStateSize];
        
      /* Compressed or not compressed? */
      if(FS::readBool(pfh)) {
	if(m_displayInformation) {
	  LogInfo("   %-27s: %s\n", "Compressed data", "true");
	} 

	/* Compressed! - read compressed size */
	int nCompressedSize = FS::readInt_LE(pfh);
	if(m_displayInformation) {
	  LogInfo("   %-27s: %i\n", "Compressed states size", nCompressedSize);
	}

	/* Read compressed data */
	unsigned char *pcCompressed = new unsigned char [nCompressedSize];
	FS::readBuf(pfh,(char *)pcCompressed,nCompressedSize);
         
	/* Uncompress it */           
	uLongf nDestLen = Chunk->nNumStates * m_nStateSize;
	uLongf nSrcLen = nCompressedSize;
	int nZRet = uncompress((Bytef *)Chunk->pcChunkData,&nDestLen,(Bytef *)pcCompressed,nSrcLen);
	if(nZRet != Z_OK || nDestLen != Chunk->nNumStates * m_nStateSize) {
	  LogWarning("Failed to uncompress chunk %d in replay", i);
	  delete [] pcCompressed;
	  Chunk->pcChunkData = NULL;
	  FS::closeFile(pfh);
	  _FreeReplay();
	  throw Exception("Unable to open the replay");
	}

	/* Clean up */
	delete [] pcCompressed;
      }
      else {
	if(m_displayInformation) {
	  LogInfo("   %-27s: %s\n", "Compressed data", "false");
	} 

	/* Not compressed! */
	FS::readBuf(pfh,Chunk->pcChunkData,m_nStateSize*Chunk->nNumStates);
      }
        
      m_Chunks.push_back(Chunk);
    }
}

void Replay::saveHeader(FileHandle* pfh)
{
  FS::writeByte(pfh, m_version);
  // Endianness guard
  FS::writeInt_LE(pfh, 0x12345678);
  FS::writeString(pfh, m_LevelID);
  FS::writeString(pfh, m_PlayerName);
  FS::writeFloat_LE(pfh, m_fFrameRate);
  FS::writeInt_LE(pfh, m_nStateSize);
  FS::writeBool(pfh, m_bFinished);
  FS::writeFloat_LE(pfh, GameApp::timeToFloat(m_finishTime));
}

void Replay::loadHeader(FileHandle* pfh)
{
  /* Read header */
  m_version = FS::readByte(pfh); 
  if(m_displayInformation) {
    LogInfo("%-30s: %i\n", "Replay file version", m_version);
  }
       
  /* Supported version? */
  if(m_version < 0 || m_version > 2) {
    FS::closeFile(pfh);
    LogWarning("Unsupported replay file version (%d)", m_version);
    throw Exception("Unable to open the replay (unsupported version)");
  } else {
    /* Little/big endian safety check */
    if(FS::readInt_LE(pfh) != 0x12345678) {
      FS::closeFile(pfh);
      LogWarning("Sorry, the replay you're trying to open are not endian-compatible with your computer!");
      throw Exception("Unable to open the replay");        
    }
  
    /* Read level ID */
    m_LevelID = FS::readString(pfh);
    if(m_displayInformation) {
      LogInfo("%-30s: %s\n", "Level Id", m_LevelID.c_str());
    }

    /* Read player name */
    m_PlayerName = FS::readString(pfh);
    if(m_displayInformation) {
      LogInfo("%-30s: %s\n", "Player", m_PlayerName.c_str());
    }      

    /* Read replay frame rate */
    m_fFrameRate = FS::readFloat_LE(pfh);

    /* Read state size */
    m_nStateSize = FS::readInt_LE(pfh);
    if(m_displayInformation) {
      LogInfo("%-30s: %i\n", "State size", m_nStateSize);
    } 
      
    /* Read finish time if any */
    m_bFinished = FS::readBool(pfh);
    m_finishTime = GameApp::floatToTime(FS::readFloat_LE(pfh));
    if(m_displayInformation) {
      if(m_bFinished) {
	LogInfo("%-30s: %.2f (%f)\n", "Finish time", m_finishTime / 100.0, m_finishTime / 100.0);
      } else {
	LogInfo("%-30s: %s\n", "Finish time", "unfinished");
      }
    }
  }
}
