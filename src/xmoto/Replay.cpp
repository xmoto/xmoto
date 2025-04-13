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
#include "helpers/Log.h"
#if defined(WIN32)
#include "zlib.h"
#else
#include <zlib.h>
#endif

#include <ctime>
#include <iostream>
#include <time.h>

#include "Game.h"
#include "GameEvents.h"
#include "db/xmDatabase.h"
#include "helpers/FileCompression.h"
#include "helpers/Log.h"
#include "helpers/SwapEndian.h"
#include "xmscene/Bike.h"
#include "xmscene/Block.h"

// minimum value to consider a block has moved
#define RMOVINGBLOCK_MIN_DIFFMOVE 0.1
#define RMOVINGBLOCK_MIN_DIFFROTATION 0.1

// if the block has moved, save it but at a frequency depending on the distance
// to the player
#define RMOVINGBLOCK_MIN_FARDIFFTIME1 15
#define RMOVINGBLOCK_MIN_FARPLAYERLENGTH1 5.0
#define RMOVINGBLOCK_MIN_FARDIFFTIME2 80
#define RMOVINGBLOCK_MIN_FARPLAYERLENGTH2 10.0
#define RMOVINGBLOCK_MIN_FARDIFFTIME3 150
// #define RMOVINGBLOCK_MIN_FARPLAYERLENGTH3 no values while it's all what is
//  greater

// if the block has not moved a lot, but for a long time, save it
#define MOVINGBLOCK_MIN_LONGDIFFTIME 500
#define RMOVINGBLOCK_MIN_LONGDIFFMOVE 0.05
#define RMOVINGBLOCK_MIN_LONGDIFFROTATION 0.05

Replay::Replay() {
  m_bFinished = false;
  m_bEndOfFile = false;
  m_finishTime = 0;
  m_pcInputEventsData = NULL;
  m_nInputEventsDataSize = 0;
  m_saved = false;
}

Replay::~Replay() {
  _FreeReplay();
}

void Replay::_FreeReplay(void) {
  /* Get rid of replay events */
  for (unsigned int i = 0; i < m_ReplayEvents.size(); i++) {
    delete m_ReplayEvents[i]->Event;
    delete m_ReplayEvents[i];
  }
  m_ReplayEvents.clear();

  /* Dealloc chunks */
  for (unsigned int i = 0; i < m_Chunks.size(); i++) {
    if (m_Chunks[i]->pcChunkData != NULL) {
      delete[] m_Chunks[i]->pcChunkData;
    }
    delete m_Chunks[i];
  }
  m_Chunks.clear();

  if (m_pcInputEventsData != NULL) {
    delete[] m_pcInputEventsData;
    m_pcInputEventsData = NULL;
  }
}

void Replay::finishReplay(bool bFinished, int finishTime) {
  m_finishTime = finishTime;
  m_bFinished = bFinished;
}

void Replay::createReplay(const std::string &FileName,
                          const std::string &LevelID,
                          const std::string &Player,
                          float fFrameRate,
                          unsigned int nStateSize) {
  m_FileName = FileName;
  m_LevelID = LevelID;
  m_PlayerName = Player;
  m_fFrameRate = fFrameRate;
  m_nStateSize = nStateSize;

  initOutput(1024);
}

void Replay::saveReplayIfNot(int i_format) {
  if (m_saved) {
    return;
  }

  FileHandle *pfh =
    XMFS::openOFile(FDT_DATA, std::string("Replays/") + m_FileName);
  if (pfh == NULL) {
    LogWarning("Failed to open replay file for output: %s",
               (std::string("Replays/") + m_FileName).c_str());
    return;
  }

  switch (i_format) {
    // case 0: do not use format 0 anymore, it's the same as 1 + events
    case 1:
      saveReplay_1(pfh);
      break;

    case 3:
      saveReplay_3(pfh);
      break;

    default:
      XMFS::closeFile(pfh);
      throw Exception("Invalid replay format");
  }

  XMFS::closeFile(pfh);
  m_saved = true;
}

void Replay::saveReplay_3(FileHandle *pfh) {
  const char *pcData;
  int nDataSize;
  char *pcCompressedData;
  int nCompressedDataSize;

  DBuffer v_replay;

  /* keep header uncompressed to be faster to read just it */

  /* Header */
  XMFS::writeByte(pfh, 3); /* Version: 3 */
  XMFS::writeInt_LE(pfh, 0x12345678); /* Endianness guard */
  XMFS::writeString(pfh, m_LevelID);
  XMFS::writeString(pfh, m_PlayerName);
  XMFS::writeFloat_LE(pfh, m_fFrameRate);
  XMFS::writeInt_LE(pfh, m_nStateSize);
  XMFS::writeBool(pfh, m_bFinished);
  XMFS::writeFloat_LE(pfh, GameApp::timeToFloat(m_finishTime));

  /* ***** ***** ***** ***** ***** **/
  /* compress all except the header */
  /* ***** ***** ***** ***** ***** **/

  v_replay.initOutput(32);

  /* Events */
  pcData = convertOutputToInput();
  nDataSize = numRemainingBytes();

  v_replay << nDataSize;
  v_replay.writeBuf(pcData, nDataSize);

  /* Chunks */
  v_replay << (unsigned int)m_Chunks.size();
  for (unsigned int i = 0; i < m_Chunks.size(); i++) {
    v_replay << m_Chunks[i]->nNumStates;
    v_replay.writeBuf(m_Chunks[i]->pcChunkData,
                      m_nStateSize * m_Chunks[i]->nNumStates);
  }

  /* Moving blocks */
  int nstates = 0;
  unsigned int nmovingBlocks = 0;

  // save only moving blocks
  for (unsigned int i = 0; i < m_movingBlocksForSaving.size(); i++) {
    if (m_movingBlocksForSaving[i].states.size() >
        1) { // > 1 because if there is only one state, the block has not moved
      nmovingBlocks++;
    }
  }
  v_replay << nmovingBlocks;
  for (unsigned int i = 0; i < m_movingBlocksForSaving.size(); i++) {
    if (m_movingBlocksForSaving[i].states.size() >
        1) { // > 1 because if there is only one state, the block has not moved
      v_replay << m_movingBlocksForSaving[i].name;
      v_replay << m_movingBlocksForSaving[i].states.size();
      for (unsigned int j = 0; j < m_movingBlocksForSaving[i].states.size();
           j++) {
        v_replay << m_movingBlocksForSaving[i].states[j].time;
        v_replay << m_movingBlocksForSaving[i].states[j].position.x;
        v_replay << m_movingBlocksForSaving[i].states[j].position.y;
        v_replay << m_movingBlocksForSaving[i].states[j].rotation;
        nstates++;
      }
    }
  }

  LogInfo("Replay moving block states size = %iKB (%i blocks, %i states, %i "
          "bytes/state)",
          nstates * sizeof(rmblockState) / 1024,
          m_movingBlocksForSaving.size(),
          nstates,
          sizeof(rmblockState));

  /* zip and write into the file */
  pcData = v_replay.convertOutputToInput();
  nDataSize = v_replay.numRemainingBytes();
  pcCompressedData =
    FileCompression::zcompress(pcData, nDataSize, nCompressedDataSize);
  XMFS::writeInt_LE(pfh, nDataSize);
  XMFS::writeInt_LE(pfh, nCompressedDataSize);
  XMFS::writeBuf(pfh, (char *)pcCompressedData, nCompressedDataSize);
  free(pcCompressedData);
  LogInfo("Replay - uncompressed = %iKB ; compressed = %iKB (ratio = %.2f%%)",
          nDataSize / 1024,
          nCompressedDataSize / 1024,
          (((float)nCompressedDataSize * 100.0)) / ((float)nDataSize));
}

void Replay::saveReplay_1(FileHandle *pfh) {
  /* Write header */
  XMFS::writeByte(pfh, 1); /* Version: 1 */
  XMFS::writeInt_LE(pfh, 0x12345678); /* Endianness guard */
  XMFS::writeString(pfh, m_LevelID);
  XMFS::writeString(pfh, m_PlayerName);
  XMFS::writeFloat_LE(pfh, m_fFrameRate);
  XMFS::writeInt_LE(pfh, m_nStateSize);
  XMFS::writeBool(pfh, m_bFinished);
  XMFS::writeFloat_LE(pfh, GameApp::timeToFloat(m_finishTime));

  /* Events */
  const char *pcUncompressedEvents = convertOutputToInput();
  int nUncompressedEventsSize = numRemainingBytes();
  XMFS::writeInt_LE(pfh, nUncompressedEventsSize);

  /* Compress events with zlib */
  char *pcCompressedEvents = new char[nUncompressedEventsSize * 2 + 12];
  uLongf nDestLen = nUncompressedEventsSize * 2 + 12;
  uLongf nSrcLen = nUncompressedEventsSize;
  int nZRet = compress2((Bytef *)pcCompressedEvents,
                        &nDestLen,
                        (Bytef *)pcUncompressedEvents,
                        nSrcLen,
                        9);
  if (nZRet != Z_OK) {
    /* Failed to compress, save raw events */
    XMFS::writeBool(pfh, false); /* compression: false */
    XMFS::writeBuf(pfh, (char *)pcUncompressedEvents, nUncompressedEventsSize);
  } else {
    /* OK */
    XMFS::writeBool(pfh, true); /* compression: true */
    XMFS::writeInt_LE(pfh, nDestLen);
    XMFS::writeBuf(pfh, (char *)pcCompressedEvents, nDestLen);
  }
  delete[] pcCompressedEvents;

  /* Chunks */
  XMFS::writeInt_LE(pfh, m_Chunks.size());

  /* Write chunks */
  for (unsigned int i = 0; i < m_Chunks.size(); i++) {
    XMFS::writeInt_LE(pfh, m_Chunks[i]->nNumStates);

    /* Try compressing the chunk with zlib */
    unsigned char *pcCompressed =
      new unsigned char[m_nStateSize * m_Chunks[i]->nNumStates * 2 + 12];
    uLongf nDestLen = m_nStateSize * m_Chunks[i]->nNumStates * 2 + 12;
    uLongf nSrcLen = m_nStateSize * m_Chunks[i]->nNumStates;
    int nZRet = compress2((Bytef *)pcCompressed,
                          &nDestLen,
                          (Bytef *)m_Chunks[i]->pcChunkData,
                          nSrcLen,
                          9);
    if (nZRet != Z_OK) {
      /* Failed to compress... Save uncompressed chunk then */
      XMFS::writeBool(pfh, false); /* compression: false */
      XMFS::writeBuf(
        pfh, m_Chunks[i]->pcChunkData, m_nStateSize * m_Chunks[i]->nNumStates);
    } else {
      /* Compressed ok */
      XMFS::writeBool(pfh, true); /* compression: true */
      XMFS::writeInt_LE(pfh, nDestLen);
      XMFS::writeBuf(pfh, (char *)pcCompressed, nDestLen);
    }
    delete[] pcCompressed;
  }
}

void Replay::openReplay_3(FileHandle *pfh, bool bDisplayInformation) {
  DBuffer v_replay;
  int v_nDataSize;
  char *v_pcData;
  int v_nCompressedDataSize;
  char *v_pcCompressedData;

  /* Little/big endian safety check */
  if (XMFS::readInt_LE(pfh) != 0x12345678) {
    LogWarning("Sorry, the replay you're trying to open are not "
               "endian-compatible with your computer!");
    throw Exception("Unable to open the replay");
  }

  /* Header */
  m_LevelID = XMFS::readString(pfh);
  if (bDisplayInformation) {
    printf("%-30s: %s\n", "Level Id", m_LevelID.c_str());
  }

  m_PlayerName = XMFS::readString(pfh);
  if (bDisplayInformation) {
    printf("%-30s: %s\n", "Player", m_PlayerName.c_str());
  }

  m_fFrameRate = XMFS::readFloat_LE(pfh);

  m_nStateSize = XMFS::readInt_LE(pfh);
  if (bDisplayInformation) {
    printf("%-30s: %i\n", "State size", m_nStateSize);
  }

  m_bFinished = XMFS::readBool(pfh);
  m_finishTime = GameApp::floatToTime(XMFS::readFloat_LE(pfh));
  if (bDisplayInformation) {
    if (m_bFinished) {
      printf("%-30s: %.2f (%f)\n",
             "Finish time",
             m_finishTime / 100.0,
             m_finishTime / 100.0);
    } else {
      printf("%-30s: %s\n", "Finish time", "unfinished");
    }
  }

  /* zuncompressed */
  v_nDataSize = XMFS::readInt_LE(pfh);
  v_nCompressedDataSize = XMFS::readInt_LE(pfh);

  v_pcData = (char *)malloc(v_nDataSize);
  if (v_pcData == NULL) {
    throw Exception("Unable to malloc for decompression");
  }
  v_pcCompressedData = (char *)malloc(v_nCompressedDataSize);
  if (v_pcCompressedData == NULL) {
    free(v_pcData);
    throw Exception("Unable to malloc for decompression");
  }
  XMFS::readBuf(pfh, v_pcCompressedData, v_nCompressedDataSize);
  try {
    FileCompression::zuncompress(
      v_pcCompressedData, v_nCompressedDataSize, v_pcData, v_nDataSize);
  } catch (Exception &e) {
    free(v_pcData);
    free(v_pcCompressedData);
    throw e;
  }
  free(v_pcCompressedData);
  v_replay.initInput(v_pcData, v_nDataSize);

  /* Events */
  v_replay >> m_nInputEventsDataSize;
  if (bDisplayInformation) {
    printf("%-30s: %i\n", "Events data size", m_nInputEventsDataSize);
  }
  m_pcInputEventsData = new char[m_nInputEventsDataSize];
  v_replay.readBuf(m_pcInputEventsData, m_nInputEventsDataSize);
  initInput(m_pcInputEventsData, m_nInputEventsDataSize);

  /* Chunks */
  unsigned int nNumChunks;
  v_replay >> nNumChunks;
  if (bDisplayInformation) {
    printf("%-30s: %i\n", "Number of chunks", nNumChunks);
  }
  if (nNumChunks == 0) {
    _FreeReplay();
    LogWarning("try to open a replay with no chunk");
    free(v_pcData);
    throw Exception("Replay with no chunk !");
  }

  for (unsigned int i = 0; i < nNumChunks; i++) {
    if (bDisplayInformation) {
      printf("Chunk %02i\n", i);
    }

    ReplayStateChunk *Chunk = new ReplayStateChunk();
    v_replay >> Chunk->nNumStates;

    if (bDisplayInformation) {
      printf("   %-27s: %i\n", "Number of states", Chunk->nNumStates);
    }

    Chunk->pcChunkData = new char[Chunk->nNumStates * m_nStateSize];

    v_replay.readBuf(Chunk->pcChunkData, m_nStateSize * Chunk->nNumStates);
    m_Chunks.push_back(Chunk);
  }

  /* moving blocks */
  unsigned int v_nmovingBlocks;
  unsigned int v_nstates;
  rmtime t;
  rmtimeState s;

  v_replay >> v_nmovingBlocks;

  for (unsigned int i = 0; i < v_nmovingBlocks; i++) {
    v_replay >> t.name;
    t.block = NULL; // don't initialize now, the level is not loaded
    t.readPos = 0;
    m_movingBlocksForLoading.push_back(t);

    v_replay >> v_nstates;
    for (unsigned int j = 0; j < v_nstates; j++) {
      v_replay >> s.time;
      v_replay >> s.position.x;
      v_replay >> s.position.y;
      v_replay >> s.rotation;

      m_movingBlocksForLoading[m_movingBlocksForLoading.size() - 1]
        .states.push_back(s);
    }
  }
  free(v_pcData);
}

void Replay::openReplay_1(FileHandle *pfh,
                          bool bDisplayInformation,
                          int nVersion) {
  /* Little/big endian safety check */
  if (XMFS::readInt_LE(pfh) != 0x12345678) {
    LogWarning("Sorry, the replay you're trying to open are not "
               "endian-compatible with your computer!");
    throw Exception("Unable to open the replay");
  }

  /* Read level ID */
  m_LevelID = XMFS::readString(pfh);
  if (bDisplayInformation) {
    printf("%-30s: %s\n", "Level Id", m_LevelID.c_str());
  }

  /* Read player name */
  m_PlayerName = XMFS::readString(pfh);
  if (bDisplayInformation) {
    printf("%-30s: %s\n", "Player", m_PlayerName.c_str());
  }

  /* Read replay frame rate */
  m_fFrameRate = XMFS::readFloat_LE(pfh);

  /* Read state size */
  m_nStateSize = XMFS::readInt_LE(pfh);
  if (bDisplayInformation) {
    printf("%-30s: %i\n", "State size", m_nStateSize);
  }

  /* Read finish time if any */
  m_bFinished = XMFS::readBool(pfh);
  m_finishTime = GameApp::floatToTime(XMFS::readFloat_LE(pfh));
  if (bDisplayInformation) {
    if (m_bFinished) {
      printf("%-30s: %.2f (%f)\n",
             "Finish time",
             m_finishTime / 100.0,
             m_finishTime / 100.0);
    } else {
      printf("%-30s: %s\n", "Finish time", "unfinished");
    }
  }

  /* Version 1 includes event data */
  if (nVersion == 1) {
    /* Read uncompressed size */
    m_nInputEventsDataSize = XMFS::readInt_LE(pfh);
    if (bDisplayInformation) {
      printf("%-30s: %i\n", "Events data size", m_nInputEventsDataSize);
    }
    m_pcInputEventsData = new char[m_nInputEventsDataSize];

    /* Compressed? */
    if (XMFS::readBool(pfh)) {
      /* Compressed */
      int nCompressedEventsSize = XMFS::readInt_LE(pfh);
      if (bDisplayInformation) {
        printf(
          "%-30s: %i\n", "Compressed events data size", nCompressedEventsSize);
      }

      char *pcCompressedEvents = new char[nCompressedEventsSize];
      XMFS::readBuf(pfh, pcCompressedEvents, nCompressedEventsSize);

      /* Unpack */
      uLongf nDestLen = m_nInputEventsDataSize;
      uLongf nSrcLen = nCompressedEventsSize;
      int nZRet = uncompress((Bytef *)m_pcInputEventsData,
                             &nDestLen,
                             (Bytef *)pcCompressedEvents,
                             nSrcLen);
      if (nZRet != Z_OK || nDestLen != m_nInputEventsDataSize) {
        delete[] pcCompressedEvents;
        delete[] m_pcInputEventsData;
        m_pcInputEventsData = NULL;
        _FreeReplay();
        LogWarning("Failed to uncompress events in replay");
        throw Exception("Unable to open the replay");
      }

      /* Clean up */
      delete[] pcCompressedEvents;
    } else {
      /* Not compressed */
      XMFS::readBuf(pfh, m_pcInputEventsData, m_nInputEventsDataSize);
    }

    /* Set up input stream */
    initInput(m_pcInputEventsData, m_nInputEventsDataSize);
  }

  /* Read chunks */
  unsigned int nNumChunks = XMFS::readInt_LE(pfh);
  if (bDisplayInformation) {
    printf("%-30s: %i\n", "Number of chunks", nNumChunks);
  }
  if (nNumChunks == 0) {
    _FreeReplay();
    LogWarning("try to open a replay with no chunk");
    throw Exception("Replay with no chunk !");
  }

  for (unsigned int i = 0; i < nNumChunks; i++) {
    if (bDisplayInformation) {
      printf("Chunk %02i\n", i);
    }

    ReplayStateChunk *Chunk = new ReplayStateChunk();
    Chunk->nNumStates = XMFS::readInt_LE(pfh);

    if (bDisplayInformation) {
      printf("   %-27s: %i\n", "Number of states", Chunk->nNumStates);
    }

    Chunk->pcChunkData = new char[Chunk->nNumStates * m_nStateSize];

    /* Compressed or not compressed? */
    if (XMFS::readBool(pfh)) {
      if (bDisplayInformation) {
        printf("   %-27s: %s\n", "Compressed data", "true");
      }

      /* Compressed! - read compressed size */
      int nCompressedSize = XMFS::readInt_LE(pfh);
      if (bDisplayInformation) {
        printf("   %-27s: %i\n", "Compressed states size", nCompressedSize);
      }

      /* Read compressed data */
      unsigned char *pcCompressed = new unsigned char[nCompressedSize];
      XMFS::readBuf(pfh, (char *)pcCompressed, nCompressedSize);

      /* Uncompress it */
      uLongf nDestLen = Chunk->nNumStates * m_nStateSize;
      uLongf nSrcLen = nCompressedSize;
      int nZRet = uncompress(
        (Bytef *)Chunk->pcChunkData, &nDestLen, (Bytef *)pcCompressed, nSrcLen);
      if (nZRet != Z_OK || nDestLen != Chunk->nNumStates * m_nStateSize) {
        LogWarning("Failed to uncompress chunk %d in replay");
        delete[] pcCompressed;
        Chunk->pcChunkData = NULL;
        _FreeReplay();
        throw Exception("Unable to open the replay");
      }

      /* Clean up */
      delete[] pcCompressed;
    } else {
      if (bDisplayInformation) {
        printf("   %-27s: %s\n", "Compressed data", "false");
      }

      /* Not compressed! */
      XMFS::readBuf(pfh, Chunk->pcChunkData, m_nStateSize * Chunk->nNumStates);
    }

    m_Chunks.push_back(Chunk);
  }
}

std::string Replay::openReplay(const std::string &FileName,
                               std::string &Player,
                               bool bDisplayInformation) {
  /* Try opening as if it is a full path */
  FileHandle *pfh = XMFS::openIFile(FDT_DATA, FileName, true);
  if (pfh == NULL) {
    /* Open file for input */
    pfh = XMFS::openIFile(FDT_DATA, std::string("Replays/") + FileName);
    if (pfh == NULL) {
      /* Try adding a .rpl extension */
      pfh = XMFS::openIFile(
        FDT_DATA, std::string("Replays/") + FileName + std::string(".rpl"));
      if (pfh == NULL) {
        LogWarning("Failed to open replay file for input: %s",
                   (std::string("Replays/") + FileName).c_str());
        throw Exception("Unable to open the replay");
      }
    }
  }

  /* Read header */
  int nVersion = XMFS::readByte(pfh);
  if (bDisplayInformation) {
    printf("%-30s: %i\n", "Replay file version", nVersion);
  }

  /* Supported version? */
  switch (nVersion) {
    case 0:
    case 1:
      try {
        openReplay_1(pfh, bDisplayInformation, nVersion);
      } catch (Exception &e) {
        XMFS::closeFile(pfh);
        throw e;
      }
      break;

    case 3:
      try {
        openReplay_3(pfh, bDisplayInformation);
      } catch (Exception &e) {
        XMFS::closeFile(pfh);
        throw e;
      }
      break;

    default:
      XMFS::closeFile(pfh);
      LogWarning("Unsupported replay file version (%d): %s",
                 nVersion,
                 (std::string("Replays/") + FileName).c_str());
      throw Exception("Unable to open the replay (unsupported version)");

      break;
  }

  /* Clean up */
  XMFS::closeFile(pfh);

  Player = m_PlayerName;

  m_nCurChunk = 0;
  m_nCurState = 0.0;

  /* Reconstruct game events that are going to happen during the replay */
  if (bDisplayInformation) {
    printf("%-30s:\n", "Game Events");
  }

  /* unserialize events */
  Scene::unserializeGameEvents(this, &m_ReplayEvents, bDisplayInformation);
  initOutput(1024);
  for (unsigned int i = 0; i < m_ReplayEvents.size(); i++) {
    m_ReplayEvents[i]->Event->serialize(*this);
  }

  return m_LevelID;
}

bool Replay::isPhysicBlockToSave(Block *i_block,
                                 int i_time,
                                 const std::vector<rmblockState> &i_states,
                                 const std::vector<Biker *> &i_players,
                                 bool i_forceSaveAll) {
  // the block must be physic
  if (i_block->isPhysics() == false) {
    return false;
  }

  if (i_forceSaveAll) {
    return true;
  }

  // the block must be saved the first time
  if (i_states.size() == 0) {
    return true;
  }

  // store only on changes and if timediff is enough
  const rmblockState *v_previousState = &(i_states[i_states.size() - 1]);

  // main case : do not save under the RMOVINGBLOCK_MIN_DIFFTIME frequency
  if (i_time < v_previousState->time + RMOVINGBLOCK_MIN_FARDIFFTIME1) {
    return false;
  }

  // at this step the decision is taken according to the distance to the players
  // compute the minimum distance between the block and the players
  float v_playerDist = 0.0;
  Vector2f plen;
  if (i_players.size() > 0) {
    plen = i_players[0]->getState()->CenterP - i_block->DynamicPosition();
    v_playerDist = plen.length();
  }
  for (unsigned int p = 1; p < i_players.size(); p++) {
    plen = i_players[p]->getState()->CenterP - i_block->DynamicPosition();
    if (plen.length() < v_playerDist) {
      v_playerDist = plen.length();
    }
  }

  // at this step, the block is stored only if it has moved
  bool v_hasNormalMove =
    v_previousState->position.x - i_block->DynamicPosition().x >
      RMOVINGBLOCK_MIN_DIFFMOVE ||
    v_previousState->position.x - i_block->DynamicPosition().x <
      -(RMOVINGBLOCK_MIN_DIFFMOVE) ||
    v_previousState->position.y - i_block->DynamicPosition().y >
      RMOVINGBLOCK_MIN_DIFFMOVE ||
    v_previousState->position.y - i_block->DynamicPosition().y <
      -(RMOVINGBLOCK_MIN_DIFFMOVE) ||
    v_previousState->rotation - i_block->DynamicRotation() >
      RMOVINGBLOCK_MIN_DIFFROTATION ||
    v_previousState->rotation - i_block->DynamicRotation() <
      -(RMOVINGBLOCK_MIN_DIFFROTATION);

  if (v_hasNormalMove) {
    // save if the block is near the player
    //
    /* test moved higher whie it's the most current
    if(i_time <= v_previousState->time + RMOVINGBLOCK_MIN_FARDIFFTIME1) {
      return false;
    }
    */

    // save if the block if far from the player, but less frequently
    if (v_playerDist >= RMOVINGBLOCK_MIN_FARPLAYERLENGTH1 &&
        i_time <= v_previousState->time + RMOVINGBLOCK_MIN_FARDIFFTIME2) {
      return false;
    }

    // save if the block if very far from the player, but really less frequently
    if (v_playerDist >= RMOVINGBLOCK_MIN_FARPLAYERLENGTH2 &&
        i_time <= v_previousState->time + RMOVINGBLOCK_MIN_FARDIFFTIME3) {
      return false;
    }

    return true;
  }

  // at this step normal moves are saved, but i suggest to save last block
  // position when they have not moved from a long time
  if (i_time <= v_previousState->time + MOVINGBLOCK_MIN_LONGDIFFTIME) {
    return false;
  }

  bool v_hasMinimalMove =
    v_previousState->position.x - i_block->DynamicPosition().x >
      RMOVINGBLOCK_MIN_LONGDIFFMOVE ||
    v_previousState->position.x - i_block->DynamicPosition().x <
      -(RMOVINGBLOCK_MIN_LONGDIFFMOVE) ||
    v_previousState->position.y - i_block->DynamicPosition().y >
      RMOVINGBLOCK_MIN_LONGDIFFMOVE ||
    v_previousState->position.y - i_block->DynamicPosition().y <
      -(RMOVINGBLOCK_MIN_LONGDIFFMOVE) ||
    v_previousState->rotation - i_block->DynamicRotation() >
      RMOVINGBLOCK_MIN_LONGDIFFROTATION ||
    v_previousState->rotation - i_block->DynamicRotation() <
      -(RMOVINGBLOCK_MIN_LONGDIFFROTATION);
  // save if the block has moved only a little from a long time
  if (v_hasMinimalMove) {
    return true;
  }

  return false;
}

void Replay::storeBlocks(int i_time,
                         const std::vector<Block *> &i_blocks,
                         const std::vector<Biker *> &i_players,
                         bool i_forceSaveAll) {
  // create a structure for each block (initialisation)
  if (m_movingBlocksForSaving.size() < i_blocks.size()) {
    for (unsigned int i = m_movingBlocksForSaving.size(); i < i_blocks.size();
         i++) {
      rmblock rb;
      rb.name = i_blocks[i]->Id();
      rb.lastMovingTime = i_time;
      m_movingBlocksForSaving.push_back(rb);
    }
  }

  // store the delta
  for (unsigned int i = 0; i < i_blocks.size(); i++) {
    /* store this block in the .rpl if the position changed */
    if (isPhysicBlockToSave(i_blocks[i],
                            i_time,
                            m_movingBlocksForSaving[i].states,
                            i_players,
                            i_forceSaveAll)) {
      rmblockState rmbs;
      rmbs.time = i_time;
      rmbs.position = i_blocks[i]->DynamicPosition();
      rmbs.rotation = i_blocks[i]->DynamicRotation();
      m_movingBlocksForSaving[i].states.push_back(rmbs);
    }
  }
}

std::vector<rmtime> *Replay::getMovingBlocks() {
  return &m_movingBlocksForLoading;
}

void Replay::storeState(const SerializedBikeState &state) {
  char *addr;

  if (m_Chunks.size() == 0) {
    ReplayStateChunk *Chunk = new ReplayStateChunk();
    Chunk->pcChunkData = new char[STATES_PER_CHUNK * m_nStateSize];
    Chunk->nNumStates = 1;
    addr = Chunk->pcChunkData;
    m_Chunks.push_back(Chunk);
  } else {
    int i = m_Chunks.size() - 1;
    if (m_Chunks[i]->nNumStates < STATES_PER_CHUNK) {
      addr = &m_Chunks[i]->pcChunkData[m_Chunks[i]->nNumStates * m_nStateSize];
      m_Chunks[i]->nNumStates++;
    } else {
      ReplayStateChunk *Chunk = new ReplayStateChunk();
      Chunk->pcChunkData = new char[STATES_PER_CHUNK * m_nStateSize];
      Chunk->nNumStates = 1;
      addr = Chunk->pcChunkData;
      m_Chunks.push_back(Chunk);
    }
  }

  memcpy(addr, (const char *)&state, m_nStateSize);
  SwapEndian::LittleSerializedBikeState(*(SerializedBikeState *)addr);
}

int Replay::CurrentFrame() const {
  return (int)(m_nCurChunk * STATES_PER_CHUNK + m_nCurState + 1);
}

bool Replay::nextNormalState() {
  return nextState(1);
}

void Replay::rewindAtPosition(ReplayPosition i_rs) {
  m_bEndOfFile = i_rs.bEndOfFile;
  m_nCurChunk = i_rs.nCurChunk;
  m_nCurState = i_rs.nCurState;
}

void Replay::rewindAtBeginning() {
  ReplayPosition v_rs;
  v_rs.bEndOfFile = false;
  v_rs.nCurChunk = 0;
  v_rs.nCurState = 0.0;

  rewindAtPosition(v_rs);
}

ReplayPosition Replay::getReplayPosition() const {
  ReplayPosition v_res;

  v_res.bEndOfFile = m_bEndOfFile;
  v_res.nCurChunk = m_nCurChunk;
  v_res.nCurState = m_nCurState;

  return v_res;
}

bool Replay::nextState(int p_frames) {
  m_bEndOfFile = false;

  m_nCurState += p_frames;

  /* end or start of a chunk */
  if (m_nCurState >= STATES_PER_CHUNK || m_nCurState < 0.0) {
    /* go on the following chunk */
    if (m_nCurState >= STATES_PER_CHUNK) { /* end of a chunk */
      if (m_nCurChunk < m_Chunks.size() - 1) {
        m_nCurState = m_nCurState - m_Chunks[m_nCurChunk]->nNumStates - 1;
        m_nCurChunk++;
      }
    } else {
      if (m_nCurState < 0.0) { /* start of a chunk */
        if (m_nCurChunk > 0) {
          m_nCurState = m_nCurState + m_Chunks[m_nCurChunk - 1]->nNumStates - 1;
          m_nCurChunk--;
        }
      }
    }

    /* if that's the beginning */
    if (m_nCurState < 0.0) {
      m_nCurState = 0.0;
    }

    /* if that's end */
    if (m_nCurState >= m_Chunks[m_nCurChunk]->nNumStates) {
      m_nCurState = m_Chunks[m_nCurChunk]->nNumStates - 1;
      return false;
    }

  } else { /* that's not the end or the start of the chunk */

    /* if that's the beginning, do nothing */

    /* if that's end */
    if (m_nCurState >= m_Chunks[m_nCurChunk]->nNumStates) {
      m_nCurState = m_Chunks[m_nCurChunk]->nNumStates - 1;
      return false;
    }
  }

  return true;
}

void Replay::loadState(BikeState *state, PhysicsSettings *i_physicsSettings) {
  /* (11th july, 2006) rasmus: i've swapped the two following lines, it
     apparently fixes
     interpolation in replays, but i don't know if it break something else */
  peekState(state, i_physicsSettings);

  m_bEndOfFile = (m_nCurChunk == m_Chunks.size() - 1 &&
                  (int)m_nCurState == m_Chunks[m_nCurChunk]->nNumStates - 1);

  if (m_bEndOfFile == false) {
    if (nextNormalState()) { /* do nothing */
    }
  }
}

void Replay::peekState(BikeState *state, PhysicsSettings *i_physicsSettings) {
  SerializedBikeState v_bs;

  /* Like loadState() but this one does not advance the cursor... it just takes
   * a peek */
  memcpy((char *)&v_bs,
         &m_Chunks[m_nCurChunk]->pcChunkData[((int)m_nCurState) * m_nStateSize],
         m_nStateSize);
  SwapEndian::LittleSerializedBikeState(v_bs);

  BikeState::convertStateFromReplay(&v_bs, state, i_physicsSettings);
}

std::string Replay::giveAutomaticName() {
  time_t date;
  time(&date);
  struct tm *TL = localtime(&date);

  char date_str[256];
  strftime(date_str, sizeof(date_str) - 1, "%d-%m-%y %H_%M", TL);

  return std::string(date_str);
}

void Replay::reinitialize() {
  m_nCurChunk = 0;
  m_nCurState = 0.0;
  m_bEndOfFile = false;

  /* resetting events */
  for (unsigned int i = 0; i < m_ReplayEvents.size(); i++) {
    m_ReplayEvents[i]->bPassed = false;
  }
}

std::string Replay::getLevelId() {
  return m_LevelID;
}

void Replay::deleteReplay(std::string ReplayName) {
  XMFS::deleteFile(FDT_DATA,
                   std::string("Replays/") + ReplayName + std::string(".rpl"));
}

ReplayInfo *Replay::getReplayInfos(const std::string p_ReplayName) {
  /* Found anything, and is the time stamp the same? If so, we don't
     want to check this again */

  /* Try opening it */
  FileHandle *pfh =
    XMFS::openIFile(FDT_DATA, "Replays/" + p_ReplayName + ".rpl");
  if (pfh == NULL) {
    pfh = XMFS::openIFile(FDT_DATA, p_ReplayName, true);
    if (pfh == NULL) {
      return NULL;
    }
  }

  int nVersion = XMFS::readByte(pfh);
  if (nVersion != 0 && nVersion != 1 && nVersion != 3) {
    XMFS::closeFile(pfh);
    return NULL;
  }

  ReplayInfo *pRpl = new ReplayInfo;
  std::string LevelID;
  std::string Player;
  bool bFinished = true;
  int finishTime = 0;

  if (XMFS::readInt_LE(pfh) != 0x12345678) {
    XMFS::closeFile(pfh);
    delete pRpl;
    return NULL;
  }

  LevelID = XMFS::readString(pfh);
  Player = XMFS::readString(pfh);
  /* fFrameRate = */ XMFS::readFloat_LE(pfh);
  /* nStateSize = */ XMFS::readInt_LE(pfh);
  bFinished = XMFS::readBool(pfh);
  finishTime = GameApp::floatToTime(XMFS::readFloat_LE(pfh));

  /* Set members */
  pRpl->Level = LevelID;
  pRpl->Name = p_ReplayName;
  pRpl->Player = Player;
  pRpl->IsFinished = bFinished;

  if (bFinished) {
    pRpl->finishTime = finishTime;
  } else {
    pRpl->finishTime = -1;
  }

  XMFS::closeFile(pfh);
  return pRpl;
}

void Replay::fastforward(int i_time) {
  /* How many states should we move forward? */
  int nNumStates = (int)((i_time * m_fFrameRate) / 100);
  nextState(nNumStates);
}

void Replay::fastrewind(int i_time, int i_minimumNbFrame) {
  /* How many states should we move forward? */
  int nNumStates = (int)((i_time * m_fFrameRate) / 100);
  nextState(nNumStates < i_minimumNbFrame ? -i_minimumNbFrame : -nNumStates);
}

void Replay::cleanReplays(xmDatabase *i_db) {}

int Replay::cleanReplaysNb(xmDatabase *i_db) {
  // char **v_result;
  // unsigned int nrow;
  int n;

  /* SELECT replay of from replay_% and %-%-% %_% which are not best replays for
   * the (id_level, id_room, id_profile) if id_profile is in profiles or
   * (id_level, id_room) else */

  //  SELECT name
  //  FROM replays AS a
  //  LEFT OUTER JOIN webhighscores AS b ON (a.id_level = b.id_level AND
  //  b.fileUrl NOT LIKE '%/' + a.name + '.rpl')
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
