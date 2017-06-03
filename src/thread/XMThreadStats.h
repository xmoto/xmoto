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

// delocalize the database update of the stats in other threads to not make
// freeze the interface

#ifndef __XMTHREADSTATS_H__
#define __XMTHREADSTATS_H__

#include "XMThread.h"
#include <vector>

class StateManager;

struct SDL_mutex;

struct xmstats_event {
  std::string playerName;
  std::string levelId;
  int playTime;
};

class XMThreadStats : public XMThread {
public:
  XMThreadStats(const std::string &i_sitekey, StateManager *i_manager);
  ~XMThreadStats();

  void delay_levelCompleted(const std::string &PlayerName,
                            const std::string &LevelID,
                            int i_playTime);
  void delay_died(const std::string &PlayerName,
                  const std::string &LevelID,
                  int i_playTime);
  void delay_abortedLevel(const std::string &PlayerName,
                          const std::string &LevelID,
                          int i_playTime);
  void delay_levelRestarted(const std::string &PlayerName,
                            const std::string &LevelID,
                            int i_playTime);

  // execute the db events
  virtual int realThreadFunction();

  void doJob();

private:
  void play();

  SDL_mutex *m_eventsMutex;

  std::string m_sitekey;
  StateManager *m_manager; // for the communication

  std::vector<xmstats_event> m_events_levelCompleted;
  std::vector<xmstats_event> m_events_died;
  std::vector<xmstats_event> m_events_abortedLevel;
  std::vector<xmstats_event> m_events_levelRestarted;
};

#endif
