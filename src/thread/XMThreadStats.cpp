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

#include "XMThreadStats.h"
#include "db/xmDatabase.h"
#include "helpers/VExcept.h"
#include "states/StateManager.h"
#include "helpers/Log.h"

XMThreadStats::XMThreadStats(const std::string& i_sitekey, StateManager* i_manager) : XMThread("XMTHREADSTATS") {
    m_sitekey = i_sitekey;
    m_manager = i_manager;
    m_eventsMutex = SDL_CreateMutex();
}

XMThreadStats::~XMThreadStats() {
  SDL_DestroyMutex(m_eventsMutex);
}

void XMThreadStats::delay_levelCompleted(const std::string &PlayerName, const std::string &LevelID, int i_playTime) {
  xmstats_event xe;

  xe.playerName = PlayerName;
  xe.levelId    = LevelID;
  xe.playTime   = i_playTime;

  SDL_LockMutex(m_eventsMutex);
  m_events_levelCompleted.push_back(xe);
  SDL_UnlockMutex(m_eventsMutex);
}

void XMThreadStats::delay_died(const std::string &PlayerName, const std::string &LevelID, int i_playTime) {
  xmstats_event xe;

  xe.playerName = PlayerName;
  xe.levelId    = LevelID;
  xe.playTime   = i_playTime;

  SDL_LockMutex(m_eventsMutex);
  m_events_died.push_back(xe);
  SDL_UnlockMutex(m_eventsMutex);
}

void XMThreadStats::delay_abortedLevel(const std::string &PlayerName, const std::string &LevelID, int i_playTime) {
  xmstats_event xe;

  xe.playerName = PlayerName;
  xe.levelId    = LevelID;
  xe.playTime   = i_playTime;

  SDL_LockMutex(m_eventsMutex);
  m_events_abortedLevel.push_back(xe);
  SDL_UnlockMutex(m_eventsMutex);
}

void XMThreadStats::delay_levelRestarted(const std::string &PlayerName, const std::string &LevelID, int i_playTime) {
  xmstats_event xe;

  xe.playerName = PlayerName;
  xe.levelId    = LevelID;
  xe.playTime   = i_playTime;

  SDL_LockMutex(m_eventsMutex);
  m_events_levelRestarted.push_back(xe);
  SDL_UnlockMutex(m_eventsMutex);
}

int XMThreadStats::realThreadFunction() {
  try {
    play();
  } catch(Exception &e) {
    return 1;
  }

  return 0;
}

void XMThreadStats::play() {
  xmstats_event xe;

  SDL_LockMutex(m_eventsMutex);

  try {
    while(m_events_levelCompleted.size() > 0) {
      xe = m_events_levelCompleted[0];
      m_pDb->stats_levelCompleted(m_sitekey, xe.playerName, xe.levelId, xe.playTime);
      m_events_levelCompleted.erase(m_events_levelCompleted.begin());
    }
    
    while(m_events_died.size() > 0) {
      xe = m_events_died[0];
      m_pDb->stats_died(m_sitekey, xe.playerName, xe.levelId, xe.playTime);
      m_events_died.erase(m_events_died.begin());
    }
    
    while(m_events_abortedLevel.size() > 0) {
      xe = m_events_abortedLevel[0];
      m_pDb->stats_abortedLevel(m_sitekey, xe.playerName, xe.levelId, xe.playTime);
      m_events_abortedLevel.erase(m_events_abortedLevel.begin());
    }
    
    while(m_events_levelRestarted.size() > 0) {
      xe = m_events_levelRestarted[0];
      m_pDb->stats_levelRestarted(m_sitekey, xe.playerName, xe.levelId, xe.playTime);
      m_events_levelRestarted.erase(m_events_levelRestarted.begin());
    }
  } catch(Exception &e) {
    LogError("Unable to update statistics");
  }    


  SDL_UnlockMutex(m_eventsMutex);

  m_manager->sendAsynchronousMessage(std::string("STATS_UPDATED"));
}

void XMThreadStats::doJob() {
  if(isThreadRunning()) {
    // it could happend that the previous thread is not finished, but some job will not be done. Anyway, it will be not next time
    return;
  }

  startThread();
}
