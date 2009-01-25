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

#ifndef __REPLAY_H__
#define __REPLAY_H__

#include "VCommon.h"
#include "VFileIO.h"
#include "DBuffer.h"
#include "xmscene/Scene.h"

#define STATES_PER_CHUNK 512

class BikeState;
class Level;

/* Replay information - an array of these suckers are returned when you
   probe for replays on a computer */
struct ReplayInfo {
  std::string Name;
  std::string Player;
  std::string Level;
  bool  IsFinished;
  int finishTime;
};

/* Replays states (frames) are grouped together in chunks for easy
   processing */
struct ReplayStateChunk {
  char* pcChunkData;
  int nNumStates;
};


struct ChipmunkBlockState {
  std::string id;
  float posX;
  float posY;
  float rot;
};

struct ChipmunkFrame {
  int frameId;
  int timeStamp;
  std::vector<ChipmunkBlockState> blockStates;
};


class Replay {
public:
  Replay();
  virtual ~Replay();

  void storeState(const SerializedBikeState& state);
  void storeBlocks(const std::vector<Block*>& i_blocks, int curTime);
  /* go and get the next state */
  void loadState(BikeState* state);  
  /* get current state */
  void peekState(BikeState* state);
  void updatePhysBlocksToFrame(int frame, Level* curLvl);

  void createReplay(const std::string &FileName,
		    const std::string &LevelID,
		    const std::string &Player,
		    float fFrameRate,
		    unsigned int nStateSize,
		    bool physicalReplay);
  void saveReplay(void);
  std::string openReplay(const std::string &FileName, std::string &Player, bool bDisplayInformation = false);

  static void deleteReplay(std::string ReplayName);
  static void cleanReplays(xmDatabase *i_db);
  static int  cleanReplaysNb(xmDatabase *i_db); // nb replays that cleanReplays() would clean

  void reinitialize();
  // call only from BikeGhost to know the associated level
  std::string getLevelId();

  void finishReplay(bool bFinished,int finishTime);
  int CurrentFrame() const;

  void fastforward(int i_time);
  void fastrewind(int i_time, int i_minimumNbFrame = 0); // i_minimumNbFrame, because sometimes, rewind do nothing if i_time it too small

  /* Data interface */
  bool didFinish(void) {return m_bFinished;}
  int getFinishTime(void) {return m_finishTime;}     
  float getFrameRate(void) {return m_fFrameRate;} 
  const std::string &getPlayerName(void) {return m_PlayerName;}
  bool endOfFile(void) {return m_bEndOfFile;}

  /* Static data interface */
  static void enableCompression(bool b) {m_bEnableCompression = b;}
  static std::string giveAutomaticName();

  std::vector<RecordedGameEvent*>* getEvents() {return &m_replayEvents;}

  /* return NULL if the replay is not valid */
  static ReplayInfo* getReplayInfos(const std::string p_ReplayName);

  DBuffer* getEventBuffer() {
    return &m_replayEventsBuffer;
  }

  unsigned int getVersion() {
    return m_version;
  }

private: 
  std::vector<ReplayStateChunk*> m_Chunks;
  unsigned int m_nCurChunk;
  /* is a float so that manage slow */   
  float        m_nCurState;
  std::string  m_FileName,m_LevelID,m_PlayerName;
  float        m_fFrameRate;
  unsigned int m_nStateSize;
  bool         m_bFinished;
  bool         m_bEndOfFile;
  int          m_finishTime;
  char*        m_pcInputEventsData;
  unsigned int m_nInputEventsDataSize;
  unsigned int m_version;
  char*        m_pcInputChipmunkData;
  unsigned int m_nInputChipmunkDataSize;

  void _FreeReplay(void);
  bool nextState(int p_frames);
  bool nextNormalState();

  void saveHeader(FileHandle* pfh);
  void loadHeader(FileHandle* pfh);
  void saveDBuffer(FileHandle* pfh, DBuffer& buffer);
  void loadDBuffer(FileHandle* pfh, DBuffer& o_buffer, char** o_data, unsigned int* o_size);
  void saveChunks(FileHandle* pfh);
  void loadChunks(FileHandle* pfh);

  static bool m_bEnableCompression;

  /* Events reconstructed from replay */
  std::vector<RecordedGameEvent*> m_replayEvents;

  // chipmunk frames reconstructed from replay
  std::vector<ChipmunkFrame*> m_chipmunkFrames;

  DBuffer m_chipmunkFramesBuffer;
  DBuffer m_replayEventsBuffer;

  bool m_displayInformation;

  // physic blocks replaying
  unsigned int m_curPos;
};

#endif