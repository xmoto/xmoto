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
#include "states/StateVote.h"
#include "thread/SendVoteThread.h"
#include "../drawlib/DrawLib.h"

#define STATE_DEAD_MAX_TIME 140
#define VELOCITY_UNTIL_TORSO_RIP 0.005

StateDeadJust::StateDeadJust(Universe* i_universe, const std::string& i_id)
: StateScene(i_universe, i_id, true, true)
{
  m_name    = "StateDeadJust";
}

StateDeadJust::~StateDeadJust()
{
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
    m_universe->getScenes()[0]->gameMessage(GAMETEXT_JUSTDEAD_RESTART + std::string("\n") +
					    GAMETEXT_JUSTDEAD_DISPLAYMENU, true, 260, gameMsg); // in multiplayer its good, to have it displayed only once
  }

  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(SendVoteThread::isToPropose(xmDatabase::instance("main"), m_universe->getScenes()[0]->getLevelSrc()->Id())) {
	StateManager::instance()->pushState(new StateVote(StateManager::instance()->getUniqueId(),
							  m_universe->getScenes()[0]->getLevelSrc()->Id()));
      }
    }
  }
}

void StateDeadJust::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE)) {
    StateManager::instance()->pushState(new StateDeadMenu(m_universe, false, true));
  }

  else if(i_type == INPUT_DOWN && i_xmkey == InputHandler::instance()->getRestartLevel()) {
    /* retart immediatly the level */
    restartLevel();
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
    StateManager::instance()->pushState(new StateDeadMenu(m_universe, false, true));
    return false;
  } else {
    return StateScene::update();
  } 
}
