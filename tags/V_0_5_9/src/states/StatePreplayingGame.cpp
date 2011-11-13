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

#include "StatePreplayingGame.h"
#include "StatePlayingLocal.h"
#include "StateManager.h"
#include "../XMSession.h"
#include "../Universe.h"
#include "../Replay.h"
#include "../helpers/Log.h"
#include "../xmscene/Camera.h"
#include "../xmscene/Level.h"
#include "../Game.h"
#include "../xmscene/BikePlayer.h"

StatePreplayingGame::StatePreplayingGame(const std::string i_idlevel, bool i_sameLevel)
: StatePreplaying(i_idlevel, i_sameLevel) {
    m_name  = "StatePreplayingGame";
}

StatePreplayingGame::~StatePreplayingGame() {
}

void StatePreplayingGame::initUniverse() {
  bool v_multiScenes = XMSession::instance()->multiScenes();
  unsigned int v_nbPlayer = XMSession::instance()->multiNbPlayers();

  m_universe->initPlay(&m_screen, v_nbPlayer, v_multiScenes);
}

void StatePreplayingGame::preloadLevels() {
  // preplays levels
  for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
    m_universe->getScenes()[i]->prePlayLevel(m_universe->getCurrentReplay(), true);
    m_universe->getScenes()[i]->setInfos("");
  }
}

void StatePreplayingGame::initPlayers() {
  bool v_multiScenes = XMSession::instance()->multiScenes();
  unsigned int v_nbPlayer = XMSession::instance()->multiNbPlayers();
  GameApp*  pGame  = GameApp::instance();

  /* add the players */
  LogInfo("Preplay level for %i player(s)", v_nbPlayer);

  if(v_multiScenes == false) { // monoworld
    Scene* v_world = m_universe->getScenes()[0];

    for(unsigned int i=0; i<v_nbPlayer; i++) {
      v_world->setCurrentCamera(i);
      v_world->getCamera()->setPlayerToFollow(v_world->addPlayerLocalBiker(i, v_world->getLevelSrc()->PlayerStart(),
								      DD_RIGHT,
								      Theme::instance(), Theme::instance()->getPlayerTheme(),
								      pGame->getColorFromPlayerNumber(i),
								      pGame->getUglyColorFromPlayerNumber(i),
								      XMSession::instance()->enableEngineSound()));
      v_world->getCamera()->setScroll(false, v_world->getGravity());
    }
        
  } else { // multiworlds
    Scene* v_world;
    
    for(unsigned int i=0; i<v_nbPlayer; i++) {
      v_world = m_universe->getScenes()[i];
      v_world->setCurrentCamera(0);
      
      v_world->getCamera()->setPlayerToFollow(v_world->addPlayerLocalBiker(i, v_world->getLevelSrc()->PlayerStart(),
								      DD_RIGHT,
								      Theme::instance(), Theme::instance()->getPlayerTheme(),
								      pGame->getColorFromPlayerNumber(i),
								      pGame->getUglyColorFromPlayerNumber(i),
								      XMSession::instance()->enableEngineSound()));
      v_world->getCamera()->setScroll(false, v_world->getGravity());
    }
  }
  
  /* initreplay */
  m_universe->initReplay();
  
}

void StatePreplayingGame::runPlaying() {
  StateManager::instance()->replaceState(new StatePlayingLocal(m_universe, m_renderer), getStateId());
}

void StatePreplayingGame::nextLevel(bool i_positifOrder) {
  playingNextLevel(i_positifOrder);
}
