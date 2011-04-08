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

#include "StatePreplayingReplay.h"
#include "StateReplaying.h"
#include "StateManager.h"
#include "../Universe.h"
#include "../XMSession.h"
#include "../helpers/Log.h"
#include "../Replay.h"
#include "../xmscene/Camera.h"
#include "../xmscene/BikePlayer.h"

StatePreplayingReplay::StatePreplayingReplay(const std::string i_replay, bool i_sameLevel)
: StatePreplaying("", i_sameLevel) {
    ReplayInfo* v_info;

    m_name   = "StatePreplayingReplay";
    m_replay = i_replay;
    m_replayBiker = NULL;

    try {
      v_info = Replay::getReplayInfos(i_replay);

      if(v_info == NULL) {
	throw Exception("Unable to retrieve replay infos");
      }
    } catch(Exception &e) {
      LogError("Unable to retrieve infos about replays '%s'", i_replay.c_str());
      m_idlevel = "";
      return;
    }

    m_idlevel = v_info->Level;
    delete v_info;
}

StatePreplayingReplay::~StatePreplayingReplay() {
  // don't clean the replay biker while the scene clean its players
  // delete m_replayBiker;
}

void StatePreplayingReplay::initUniverse() {
  m_universe->initPlay(&m_screen, 1, false);
  m_universe->addGhostToExclude(m_replay);
}

void StatePreplayingReplay::preloadLevels() {
  for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
    m_universe->getScenes()[i]->prePlayLevel(NULL, false);
  }
}

void StatePreplayingReplay::initPlayers() {
  if(m_universe->getScenes().size() > 0) {
    m_replayBiker = m_universe->getScenes()[0]->addReplayFromFile(m_replay,
								  Theme::instance(),
								  Theme::instance()->getPlayerTheme(),
								  XMSession::instance()->enableEngineSound());
    m_universe->getScenes()[0]->getCamera()->setPlayerToFollow(m_replayBiker);
    m_universe->getScenes()[0]->getCamera()->setScroll(false, m_universe->getScenes()[0]->getGravity());
  }
}

void StatePreplayingReplay::runPlaying() {
  StateManager::instance()->replaceState(new StateReplaying(m_universe, m_renderer, m_replay, m_replayBiker), getStateId());
}
