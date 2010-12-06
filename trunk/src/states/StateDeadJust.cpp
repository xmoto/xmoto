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

#include "StateDeadJust.h"
#include "../Game.h"
#include "StateDeadMenu.h"
#include "../GameText.h"
#include "../Universe.h"
#include "StateVote.h"
#include "StatePlayingLocal.h"
#include "../thread/SendVoteThread.h"
#include "../drawlib/DrawLib.h"
#include "../helpers/Log.h"

#define STATE_DEAD_MAX_TIME 140
#define VELOCITY_UNTIL_TORSO_RIP 0.005

StateDeadJust::StateDeadJust(Universe* i_universe, GameRenderer* i_renderer)
: StateScene(i_universe, i_renderer, true, true)
{
  m_name    = "StateDeadJust";
  StateManager::instance()->registerAsObserver("TOCHECKPOINT", this);
}

StateDeadJust::~StateDeadJust()
{
  StateManager::instance()->unregisterAsObserver("TOCHECKPOINT", this);
}

void StateDeadJust::enter()
{
  StateScene::enter();
  m_enterTime = 0;
  
  if(m_universe != NULL) {
    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      
      m_universe->getScenes()[i]->clearGameMessages();
      m_universe->getScenes()[i]->setInfos(m_universe->getScenes()[i]->getLevelSrc()->Name());
    }
    m_universe->getScenes()[0]->gameMessage((m_universe->getScenes()[0]->getCheckpoint() != NULL ? GAMETEXT_JUSTDEAD_CHECKPOINT : std::string(""))
					    + std::string("\n") + GAMETEXT_JUSTDEAD_RESTART + std::string("\n") +
					    GAMETEXT_JUSTDEAD_DISPLAYMENU, true, 260, gameMsg); // in multiplayer its good, to have it displayed only once
  }

  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(SendVoteThread::isToPropose(xmDatabase::instance("main"), m_universe->getScenes()[0]->getLevelSrc()->Id())) {
	StateManager::instance()->pushState(new StateVote(m_universe->getScenes()[0]->getLevelSrc()->Id()));
      }
    }
  }
}

void StateDeadJust::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE)) {
    StateManager::instance()->pushState(new StateDeadMenu(m_universe));
  }

  else if(i_type == INPUT_DOWN && i_xmkey == (*InputHandler::instance()->getGlobalKey(INPUT_RESTARTLEVEL))) {
    /* retart immediatly the level */
    restartLevel();
  }

  else if(i_type == INPUT_DOWN && i_xmkey == (*InputHandler::instance()->getGlobalKey(INPUT_RESTARTCHECKPOINT))) {
    toCheckpointBeeingDead();
  }

  else {
    StateScene::xmKey(i_type, i_xmkey);
  }
}

void StateDeadJust::restartLevel(bool i_reloadLevel) {
  restartLevelToPlay(i_reloadLevel);
}

void StateDeadJust::nextLevel(bool i_positifOrder) {
  nextLevelToPlay(i_positifOrder);
}

bool StateDeadJust::update() {
  float v_torsoVelocity = 0;

  if(XMSession::instance()->enableDeadAnimation() == false) {
    StateManager::instance()->pushState(new StateDeadMenu(m_universe));
    return false;
  }

  if(m_universe != NULL) {
    for(unsigned int i=0; i < m_universe->getScenes().size(); i++) {
      for(unsigned int j=0; j < m_universe->getScenes()[i]->Players().size(); j++) {
	if(m_universe->getScenes()[i]->Players()[j]->getTorsoVelocity() > v_torsoVelocity) {
	  v_torsoVelocity = m_universe->getScenes()[i]->Players()[j]->getTorsoVelocity();
	}
      }
    }
    if( v_torsoVelocity <= VELOCITY_UNTIL_TORSO_RIP && m_enterTime == 0) {
      m_enterTime = GameApp::getXMTimeInt();
    }    
    if( m_enterTime != 0 && GameApp::getXMTimeInt() - m_enterTime > STATE_DEAD_MAX_TIME*23 ) {
      if(StateManager::instance()->isTopOfTheStates(this)) { // only if not already recovered
	StateManager::instance()->pushState(new StateDeadMenu(m_universe));
	return false;
      } else {
	return StateScene::update();
      }
    } else {
      return StateScene::update();
    } 
  }

  return false;
}

void StateDeadJust::executeOneCommand(std::string cmd, std::string args) {
  LogDebug("cmd [%s [%s]] executed by state [%s].",
	   cmd.c_str(), args.c_str(), getName().c_str());

  if(cmd == "TOCHECKPOINT") {
    toCheckpointBeeingDead();
  }

  else {
    StateScene::executeOneCommand(cmd, args);
  }
}

void StateDeadJust::toCheckpointBeeingDead() {
  bool v_isCheckpoint = false;
  
  // resussite players    
  for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
    if(m_universe->getScenes()[j]->getCheckpoint() != NULL) {
      v_isCheckpoint = true;
      for(unsigned int i=0; i<m_universe->getScenes()[j]->Players().size(); i++) {
	if(m_universe->getScenes()[j]->Players()[i]->isDead()) {
	  m_universe->getScenes()[j]->resussitePlayer(i);
	}
      }
    }
  }
  
  if(v_isCheckpoint) {
    StateScene::playToCheckpoint();
    StateManager::instance()->replaceState(new StatePlayingLocal(m_universe, m_renderer), getStateId());
  }
}
