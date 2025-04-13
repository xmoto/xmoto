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
#include "StateDeadMenu.h"
#include "StatePlayingLocal.h"
#include "StateVote.h"
#include "drawlib/DrawLib.h"
#include "helpers/Log.h"
#include "thread/SendVoteThread.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"
#include "xmoto/Universe.h"
#include "xmscene/Camera.h"

#define STATE_DEAD_MAX_TIME 2000
#define VELOCITY_UNTIL_TORSO_RIP 0.05

StateDeadJust::StateDeadJust(Universe *i_universe, GameRenderer *i_renderer)
  : StateScene(i_universe, i_renderer, true, true) {
  m_name = "StateDeadJust";
  StateManager::instance()->registerAsObserver("TOCHECKPOINT", this);
}

StateDeadJust::~StateDeadJust() {
  StateManager::instance()->unregisterAsObserver("TOCHECKPOINT", this);
}

void StateDeadJust::enter() {
  StateScene::enter();
  m_enterTime = 0;

  if (!m_universe) {
    return;
  }

  bool safemode = XMSession::instance()->isSafemodeActive();

  for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
    m_universe->getScenes()[i]->clearGameMessages();
    m_universe->getScenes()[i]->setInfos(
      m_universe->getScenes()[i]->getLevelSrc()->Name());
  }

  std::string message;

  if (checkpointReached()) {
    message = GAMETEXT_JUSTDEAD_CHECKPOINT;
  }

  if (!safemode || !checkpointReached()) {
    message += "\n" + std::string(GAMETEXT_JUSTDEAD_RESTART);
  }

  message += "\n" + std::string(GAMETEXT_JUSTDEAD_DISPLAYMENU);

  // in multiplayer its good, to have it displayed only once
  m_universe->getScenes()[0]->gameMessage(message, true, 260, gameMsg);

  if (m_universe->getScenes().size() == 1) {
    if (SendVoteThread::isToPropose(
          xmDatabase::instance("main"),
          m_universe->getScenes()[0]->getLevelSrc()->Id())) {
      StateManager::instance()->pushState(
        new StateVote(m_universe->getScenes()[0]->getLevelSrc()->Id()));
    }
  }
}

void StateDeadJust::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  bool safemode = XMSession::instance()->isSafemodeActive();

  if (i_type == INPUT_DOWN &&
      (i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE) ||
       i_xmkey.getJoyButton() == SDL_CONTROLLER_BUTTON_START)) {
    StateManager::instance()->pushState(
      new StateDeadMenu(m_universe, getStateId()));
  }

  else if (i_type == INPUT_DOWN &&
           i_xmkey == (*Input::instance()->getGlobalKey(INPUT_RESTARTLEVEL))) {
    if (!safemode || !checkpointReached()) {
      /* restart the level immediately */
      restartLevel();
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == (*Input::instance()->getGlobalKey(
                                                INPUT_RESTARTCHECKPOINT))) {
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
  float v_torsoVelocity = 0.0;

  if (!XMSession::instance()->enableDeadAnimation()) {
    StateManager::instance()->pushState(
      new StateDeadMenu(m_universe, getStateId()));
    return false;
  }

  if (!m_universe) {
    return false;
  }

  // compute the max velocity of the biker
  // for(unsigned int i=0; i < m_universe->getScenes().size(); i++) {
  //  for(unsigned int j=0; j < m_universe->getScenes()[i]->Players().size();
  //  j++) {
  //    if(m_universe->getScenes()[i]->Players()[j]->getTorsoVelocity() >
  //    v_torsoVelocity) {
  //      v_torsoVelocity =
  //      m_universe->getScenes()[i]->Players()[j]->getTorsoVelocity();
  //    }
  //  }
  //}
  // instead of taking the max of the players, take the max of the cameras, take
  // the one on the camera
  for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
    for (unsigned int j = 0; j < m_universe->getScenes()[i]->Cameras().size();
         j++) {
      if (m_universe->getScenes()[i]->Cameras()[j]->getPlayerToFollow() !=
          NULL) {
        if (m_universe->getScenes()[i]
              ->Cameras()[j]
              ->getPlayerToFollow()
              ->getTorsoVelocity() > v_torsoVelocity) {
          v_torsoVelocity = m_universe->getScenes()[i]
                              ->Cameras()[j]
                              ->getPlayerToFollow()
                              ->getTorsoVelocity();
        }
      }
    }
  }

  if (m_enterTime == 0) {
    // continue if the velocity is still too much
    if (v_torsoVelocity >= VELOCITY_UNTIL_TORSO_RIP) {
      return StateScene::update();
    }
    // initialize the no movement time
    m_enterTime = GameApp::getXMTimeInt();
  } else {
    // reset if the biker followed takes some speed or change
    // if(v_torsoVelocity >= VELOCITY_UNTIL_TORSO_RIP) {
    //  m_enterTime = 0;
    //  return StateScene::update();
    //}
  }

  // still some time to update
  if (GameApp::getXMTimeInt() - m_enterTime <= STATE_DEAD_MAX_TIME) {
    return StateScene::update();
  }

  // no more update allowed
  if (StateManager::instance()->isTopOfTheStates(
        this)) { // only if not already recovered
    StateManager::instance()->pushState(
      new StateDeadMenu(m_universe, getStateId()));
    return false;
  }

  return false;
}

void StateDeadJust::executeOneCommand(std::string cmd, std::string args) {
  LogDebug("cmd [%s [%s]] executed by state [%s].",
           cmd.c_str(),
           args.c_str(),
           getName().c_str());

  if (cmd == "TOCHECKPOINT") {
    toCheckpointBeeingDead();
  }

  else {
    StateScene::executeOneCommand(cmd, args);
  }
}

void StateDeadJust::toCheckpointBeeingDead() {
  bool v_isCheckpoint = false;

  // resuscitate players
  for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
    if (m_universe->getScenes()[j]->getCheckpoint() != NULL) {
      v_isCheckpoint = true;
      for (unsigned int i = 0; i < m_universe->getScenes()[j]->Players().size();
           i++) {
        if (m_universe->getScenes()[j]->Players()[i]->isDead()) {
          m_universe->getScenes()[j]->resussitePlayer(i);
        }
      }
    }
  }

  if (v_isCheckpoint) {
    StateScene::playToCheckpoint();
    StateManager::instance()->replaceState(
      new StatePlayingLocal(m_universe, m_renderer), getStateId());
  } else {
    if (XMSession::instance()->beatingMode()) {
      restartLevel();
    }
  }
}

bool StateDeadJust::checkpointReached() const {
  return m_universe->getScenes()[0]->getCheckpoint() != nullptr;
}
