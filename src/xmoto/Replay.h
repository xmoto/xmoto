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

#include "common/DBuffer.h"
#include "common/VCommon.h"
#include "common/VFileIO.h"
#include "xmscene/Scene.h"

#define STATES_PER_CHUNK 512

class BikeState;
class PhysicsSettings;

/*===========================================================================
Structs
===========================================================================*/
/* Replay information - an array of these suckers are returned when you
   probe for replays on a computer */
struct ReplayInfo {
  std::string Name;
  std::string Player;
  std::string Level;
  bool IsFinished;
  int finishTime;
};

/* Replays states (frames) are grouped together in chunks for easy
   processing */
struct ReplayStateChunk {
  char *pcChunkData;
  int nNumStates;
};

/* to be able to rewind exactly at the same position */
struct ReplayPosition {
  bool bEndOfFile;
  unsigned int nCurChunk;
  float nCurState;
};

/* moving blocks (physics) */
struct rmblockState {
  int time;
  Vector2f position;
  float rotation;
};

struct rmblock {
  std::string name;
  std::vector<rmblockState> states;
  int lastMovingTime;
};

struct rmtimeState {
  int time;
  Vector2f position;
  float rotation;
};

struct rmtime {
  std::string name;
  Block *block;
  unsigned int readPos;
  std::vector<rmtimeState> states;
};

/*===========================================================================
Replay class
===========================================================================*/
class Replay : public DBuffer {
public:
  Replay();
  virtual ~Replay();

  /* Methods */
  void storeState(const SerializedBikeState &state);
  void storeBlocks(int i_time,
                   const std::vector<Block *> &i_blocks,
                   const std::vector<Biker *> &i_players,
                   bool i_forceSaveAll = false);
  /* go and get the next state */
  void loadState(BikeState *state, PhysicsSettings *i_physicsSettings);
  void peekState(BikeState *state,
                 PhysicsSettings *i_physicsSettings); /* get current state */

  void createReplay(const std::string &FileName,
                    const std::string &LevelID,
                    const std::string &Player,
                    float fFrameRate,
                    unsigned int nStateSize);
  std::string openReplay(const std::string &FileName,
                         std::string &Player,
                         bool bDisplayInformation = false);

  static void deleteReplay(std::string ReplayName);
  static void cleanReplays(xmDatabase *i_db);
  static int cleanReplaysNb(
    xmDatabase *i_db); // nb replays that cleanReplays() would clean

  void reinitialize();
  std::string getLevelId();

  void finishReplay(bool bFinished, int finishTime);
  int CurrentFrame() const;

  void fastforward(int i_time);
  void fastrewind(int i_time, int i_minimumNbFrame = 0); // i_minimumNbFrame,
  // because sometimes,
  // rewind do nothing if
  // i_time it too small

  /* Data interface */
  bool didFinish(void) { return m_bFinished; }
  int getFinishTime(void) { return m_finishTime; }
  float getFrameRate(void) { return m_fFrameRate; }
  const std::string &getPlayerName(void) { return m_PlayerName; }
  bool endOfFile(void) { return m_bEndOfFile; }

  static std::string giveAutomaticName();

  std::vector<RecordedGameEvent *> *getEvents() { return &m_ReplayEvents; }

  // moving blocks
  std::vector<rmtime> *getMovingBlocks();

  /* return NULL if the replay is not valid */
  static ReplayInfo *getReplayInfos(const std::string p_ReplayName);

  void saveReplayIfNot(int i_format);

  // store and restore replay position
  void rewindAtPosition(ReplayPosition i_rs);
  void rewindAtBeginning();
  ReplayPosition getReplayPosition() const;

private:
  /* Data */
  std::vector<ReplayStateChunk *> m_Chunks;
  unsigned int m_nCurChunk;
  float m_nCurState; /* is a float so that manage slow */
  std::string m_FileName, m_LevelID, m_PlayerName;
  float m_fFrameRate;
  unsigned int m_nStateSize;
  bool m_bFinished;
  bool m_bEndOfFile;
  int m_finishTime;
  char *m_pcInputEventsData;
  unsigned int m_nInputEventsDataSize;

  /* Helpers */
  void _FreeReplay(void);
  bool nextState(int p_frames); /* go to the next state */
  bool nextNormalState(); /* go to the next state */

  bool m_saved;

  /* Events reconstructed from replay */
  std::vector<RecordedGameEvent *> m_ReplayEvents;

  void saveReplay_1(FileHandle *pfh);
  void saveReplay_3(FileHandle *pfh);

  void openReplay_1(FileHandle *pfh, bool bDisplayInformation, int nVersion);
  void openReplay_3(FileHandle *pfh, bool bDisplayInformation);

  /* moving blocks (physics) */
  std::vector<rmblock> m_movingBlocksForSaving;
  std::vector<rmtime> m_movingBlocksForLoading;
  bool isPhysicBlockToSave(Block *i_block,
                           int i_time,
                           const std::vector<rmblockState> &i_states,
                           const std::vector<Biker *> &i_players,
                           bool i_forceSaveAll);
};

#endif
