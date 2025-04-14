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

#include "StatePlayingLocal.h"
#include "StateDeadJust.h"
#include "StateDeadMenu.h"
#include "StateFinished.h"
#include "StateLevelInfoViewer.h"
#include "StateMessageBox.h"
#include "StatePause.h"
#include "common/CameraAnimation.h"
#include "common/XMSession.h"
#include "helpers/Log.h"
#include "net/NetClient.h"
#include "thread/XMThreadStats.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"
#include "xmoto/LuaLibGame.h"
#include "xmoto/Renderer.h"
#include "xmoto/Sound.h"
#include "xmoto/SysMessage.h"
#include "xmoto/Trainer.h"
#include "xmoto/Universe.h"
#include "xmscene/Bike.h"
#include "xmscene/BikeController.h"
#include "xmscene/Camera.h"

StatePlayingLocal::StatePlayingLocal(Universe *i_universe,
                                     GameRenderer *i_renderer)
  : StatePlaying(i_universe, i_renderer) {
  m_name = "StatePlayingLocal";
  m_gameIsFinished = false;

  /* prepare stats */
  makeStatsStr();

  StateManager::instance()->registerAsObserver("OPTIONS_UPDATED", this);

  if (XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("STATS_UPDATED");
    StateManager::instance()->registerAsEmitter("LEVELS_UPDATED");
  }

  if (m_universe != NULL) {
    if (m_universe->getScenes().size() != 0) {
      if (NetClient::instance()->isConnected()) {
        NA_playingLevel na(m_universe->getScenes()[0]->getLevelSrc()->Id());
        NetClient::instance()->send(&na, 0);
      }
    }
  }
}

StatePlayingLocal::~StatePlayingLocal() {
  StateManager::instance()->unregisterAsObserver("OPTIONS_UPDATED", this);
}

void StatePlayingLocal::enter() {
  StatePlaying::enter();

  /*
    warning, this function is called when a new game start, or when the player
    resussite after a checkpoint restoration
   */

  m_gameIsFinished = false;

  // reset trainer mode use
  bool v_onlyNewGame = true;
  if (m_universe != NULL) {
    for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
      // if only new game are set, then, reset the trainer use
      if (m_universe->getScenes()[i]->playInitLevelDone()) {
        v_onlyNewGame = false;
      }
    }
  }
  if (v_onlyNewGame) { // if any game is continuing, don't reset the trainer use
    Trainer::instance()->resetTrainerUse();
  }

  // initialiaze the level
  std::string v_level_name;
  try {
    if (m_universe != NULL) {
      for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
        v_level_name = m_universe->getScenes()[i]->getLevelSrc()->Name();
        if (m_universe->getScenes()[i]->playInitLevelDone() == false) {
          m_universe->getScenes()[i]->playInitLevel();
        }
      }
    }
  } catch (Exception &e) {
    LogWarning("level '%s' cannot be loaded", v_level_name.c_str());

    std::string v_msg;
    char cBuf[256];
    snprintf(cBuf, 256, GAMETEXT_LEVELCANNOTBELOADED, v_level_name.c_str());
    v_msg = std::string(cBuf) + "\n" + e.getMsg();

    StateMessageBox *v_msgboxState =
      new StateMessageBox(this, v_msg, UI_MSGBOX_OK);
    v_msgboxState->setMsgBxId("ERROR");
    StateManager::instance()->pushState(v_msgboxState);
  }

  // read keys for more reactivity
  dealWithActivedKeys();
}

void StatePlayingLocal::leave() {
  if (m_universe != NULL) {
    for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->setInfos("");
    }
  }

  if (GameApp::instance()->isRequestingEnd()) {
    // when end if forced, update stats as aborted
    if (m_universe != NULL) {
      if (m_universe->getScenes().size() == 1) {
        if (m_universe->getScenes()[0]->Players().size() == 1) {
          if (m_universe->getScenes()[0]->Players()[0]->isDead() == false &&
              m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
            StateManager::instance()->getDbStatsThread()->delay_abortedLevel(
              XMSession::instance()->profile(),
              m_universe->getScenes()[0]->getLevelSrc()->Id(),
              m_universe->getScenes()[0]->getTime() -
                m_universe->getScenes()[0]->getCheckpointStartTime());
            StateManager::instance()->getDbStatsThread()->doJob();
          }
        }
      }
    }
  }
}

void StatePlayingLocal::enterAfterPop() {
  StatePlaying::enterAfterPop();

  // recheck keys
  dealWithActivedKeys();
  m_fLastPhysTime = GameApp::getXMTime();
}

bool StatePlayingLocal::update() {
  if (StatePlaying::update() == false)
    return false;

  if (isLockedScene() == false) {
    bool v_all_dead = true;
    bool v_one_still_play = false;
    bool v_one_finished = false;

    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        for (unsigned int i = 0;
             i < m_universe->getScenes()[j]->Players().size();
             i++) {
          if (m_universe->getScenes()[j]->Players()[i]->isDead() == false) {
            v_all_dead = false;
          }
          if (m_universe->getScenes()[j]->Players()[i]->isFinished()) {
            v_one_finished = true;
          }

          if (m_universe->getScenes()[j]->Players()[i]->isFinished() == false &&
              m_universe->getScenes()[j]->Players()[i]->isDead() == false) {
            v_one_still_play = true;
          }
        }
      }
    }

    // MultiPlayer modes must be set here!
    if (m_gameIsFinished == false) {
      if (v_one_still_play == false ||
          XMSession::instance()->MultiStopWhenOneFinishes()) { // let people
        // continuing
        // when one
        // finished or
        // not
        if (v_one_finished) {
          /* You're done maaaan! :D */
          onOneFinish();
          m_gameIsFinished = true;
        } else if (v_all_dead) {
          /* You're dead maan! */
          onAllDead();
          m_gameIsFinished = true;
        }
      }
    }
  }

  return true;
}

void StatePlayingLocal::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (i_type == INPUT_DOWN &&
      (i_xmkey == (*Input::instance()->getGlobalKey(INPUT_PLAYINGPAUSE)) ||
       i_xmkey.getJoyButton() == SDL_CONTROLLER_BUTTON_START)) {
    if (isLockedScene() == false) {
      /* Escape pauses */
      m_displayStats = true;
      StateManager::instance()->pushState(
        new StatePause(m_universe, getStateId()));
    }
  }
#if defined(ENABLE_DEV)
  else if (i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP_0, KMOD_LCTRL)) {
    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        for (unsigned int i = 0;
             i < m_universe->getScenes()[j]->Players().size();
             i++) {
          if (m_universe->getScenes()[j]->Cameras().size() > 0) {
            m_universe->TeleportationCheatTo(
              i,
              Vector2f(
                m_universe->getScenes()[j]->Cameras()[0]->getCameraPositionX(),
                m_universe->getScenes()[j]
                  ->Cameras()[0]
                  ->getCameraPositionY()));
          }
        }
      }
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP_0, KMOD_NONE)) {
    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        Trainer::instance()->storePosition(
          m_universe->getScenes()[j]->getLevelSrc()->Id(),
          m_universe->getScenes()[j]->getPlayerPosition(0));
        // TODO: bool getPlayerFaceDir (int i_player)
        char sysmsg[256];
        snprintf(sysmsg,
                 256,
                 SYS_MSG_TRAIN_STORED,
                 Trainer::instance()->getMaxRestoreIndex() + 1);
        SysMessage::instance()->displayText(sysmsg);
      }
    }
  }

  // TRAINER
  else if (i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_BACKSPACE, KMOD_ALT)) {
    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        if (Trainer::instance()->isRestorePositionAvailable(
              m_universe->getScenes()[j]->getLevelSrc()->Id())) {
          Vector2f pos = Trainer::instance()->getCurrentRestorePosition(
            m_universe->getScenes()[j]->getLevelSrc()->Id());
          m_universe->TeleportationCheatTo(0, pos);
          char sysmsg[256];
          snprintf(sysmsg,
                   256,
                   SYS_MSG_TRAIN_RESTORING,
                   Trainer::instance()->getCurrentRestoreIndex() + 1,
                   Trainer::instance()->getMaxRestoreIndex() + 1);
          SysMessage::instance()->displayText(sysmsg);
        } else {
          SysMessage::instance()->displayText(SYS_MSG_TRAIN_NO_RESTORE_AVAIL);
        }
      }
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_MINUS, KMOD_NONE)) {
    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        if (Trainer::instance()->isRestorePositionAvailable(
              m_universe->getScenes()[j]->getLevelSrc()->Id())) {
          Vector2f pos = Trainer::instance()->getPreviousRestorePosition(
            m_universe->getScenes()[j]->getLevelSrc()->Id());
          m_universe->TeleportationCheatTo(0, pos);
          char sysmsg[256];
          snprintf(sysmsg,
                   256,
                   SYS_MSG_TRAIN_RESTORING,
                   Trainer::instance()->getCurrentRestoreIndex() + 1,
                   Trainer::instance()->getMaxRestoreIndex() + 1);
          SysMessage::instance()->displayText(sysmsg);
        } else {
          SysMessage::instance()->displayText(SYS_MSG_TRAIN_NO_RESTORE_AVAIL);
        }
      }
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_PLUS, KMOD_NONE)) {
    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        if (Trainer::instance()->isRestorePositionAvailable(
              m_universe->getScenes()[j]->getLevelSrc()->Id())) {
          Vector2f pos = Trainer::instance()->getNextRestorePosition(
            m_universe->getScenes()[j]->getLevelSrc()->Id());
          m_universe->TeleportationCheatTo(0, pos);
          char sysmsg[256];
          snprintf(sysmsg,
                   256,
                   SYS_MSG_TRAIN_RESTORING,
                   Trainer::instance()->getCurrentRestoreIndex() + 1,
                   Trainer::instance()->getMaxRestoreIndex() + 1);
          SysMessage::instance()->displayText(sysmsg);
        } else {
          SysMessage::instance()->displayText(SYS_MSG_TRAIN_NO_RESTORE_AVAIL);
        }
      }
    }
  }
#endif

  else if (i_type == INPUT_DOWN && i_xmkey == (*Input::instance()->getGlobalKey(
                                                INPUT_RESTARTCHECKPOINT))) {

    bool v_isCheckpoint = false;
    bool safemode = XMSession::instance()->isSafemodeActive();

    for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
      if (!m_universe->getScenes()[j]->getCheckpoint()) {
        continue;
      }

      v_isCheckpoint = true;
      break;
    }

    if (!safemode) {
      if (v_isCheckpoint) {
        StateScene::playToCheckpoint();
      } else if (XMSession::instance()->beatingMode()) {
        restartLevel();
      }
    }
  }

  else if (i_type == INPUT_DOWN &&
           i_xmkey == (*Input::instance()->getGlobalKey(INPUT_LEVELINFO))) {
    if (!isLockedScene()) {
      m_displayStats = true;
      StateManager::instance()->pushState(new StateLevelInfoViewer(
        m_universe->getScenes()[0]->getLevelSrc()->Id(), true, false));
    }
  }

  else {
    if (i_type == INPUT_DOWN) {
      if (m_autoZoom == false) {
        // to avoid people changing direction during the autozoom
        handleInput(INPUT_DOWN, i_xmkey);
      }
    } else {
      handleInput(INPUT_UP, i_xmkey);
    }
    StatePlaying::xmKey(i_type, i_xmkey);
  }
}

void StatePlayingLocal::onOneFinish() {
  GameApp *pGame = GameApp::instance();

  /* finalize the replay */
  if (m_universe != NULL) {
    if (m_universe->isAReplayToSave()) {
      m_universe->finalizeReplay(true);
    }
  }

  /* update profile and stats */
  if (m_universe != NULL) {
    if (m_universe->getScenes().size() == 1) {
      if (m_universe->getScenes()[0]->Players().size() == 1) {
        int v_finish_time = 0;
        std::string TimeStamp = pGame->getTimeStamp();
        if (m_universe->getScenes()[0]->Players()[0]->isFinished()) {
          v_finish_time =
            m_universe->getScenes()[0]->Players()[0]->finishTime();
        }
        // Updating the stats if the Trainer has not been used
        if (Trainer::instance()->trainerHasBeenUsed() == false) {
          xmDatabase::instance("main")->profiles_addFinishTime(
            XMSession::instance()->sitekey(),
            XMSession::instance()->profile(),
            m_universe->getScenes()[0]->getLevelSrc()->Id(),
            TimeStamp,
            v_finish_time);
        }

        StateManager::instance()->getDbStatsThread()->delay_levelCompleted(
          XMSession::instance()->profile(),
          m_universe->getScenes()[0]->getLevelSrc()->Id(),
          m_universe->getScenes()[0]->Players()[0]->finishTime() -
            m_universe->getScenes()[0]->getCheckpointStartTime());
        StateManager::instance()->getDbStatsThread()->doJob();

        StateManager::instance()->sendAsynchronousMessage("LEVELS_UPDATED");
      }
    }
  }

  StateManager::instance()->pushState(
    new StateFinished(m_universe, getStateId()));
}

void StatePlayingLocal::onAllDead() {
  if (m_universe != NULL) {
    if (m_universe->isAReplayToSave()) {
      m_universe->finalizeReplay(false);
    }
  }

  /* Update stats */
  if (m_universe != NULL) {
    if (m_universe->getScenes().size() == 1) {
      if (m_universe->getScenes()[0]->Players().size() == 1) {
        StateManager::instance()->getDbStatsThread()->delay_died(
          XMSession::instance()->profile(),
          m_universe->getScenes()[0]->getLevelSrc()->Id(),
          m_universe->getScenes()[0]->getTime() -
            m_universe->getScenes()[0]->getCheckpointStartTime());
        StateManager::instance()->getDbStatsThread()->doJob();
      }
    }
  }

  StateManager::instance()->replaceState(
    new StateDeadJust(m_universe, m_renderer), getStateId());
}

void StatePlayingLocal::abortPlaying() {
  if (m_universe != NULL) {
    if (m_universe->getScenes().size() == 1) {
      if (m_universe->getScenes()[0]->Players().size() == 1) {
        if (m_universe->getScenes()[0]->Players()[0]->isDead() == false &&
            m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
          StateManager::instance()->getDbStatsThread()->delay_abortedLevel(
            XMSession::instance()->profile(),
            m_universe->getScenes()[0]->getLevelSrc()->Id(),
            m_universe->getScenes()[0]->getTime() -
              m_universe->getScenes()[0]->getCheckpointStartTime());
          StateManager::instance()->getDbStatsThread()->doJob();
        }
      }
    }
  }

  StatePlaying::abortPlaying();
}

void StatePlayingLocal::nextLevel(bool i_positifOrder) {
  playingNextLevel(i_positifOrder);
}

void StatePlayingLocal::restartLevel(bool i_reloadLevel) {
  /* Update stats */
  if (m_universe != NULL) {
    if (m_universe->getScenes().size() == 1) {
      if (m_universe->getScenes()[0]->Players().size() == 1) {
        if (m_universe->getScenes()[0]->Players()[0]->isDead() == false &&
            m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
          StateManager::instance()->getDbStatsThread()->delay_levelRestarted(
            XMSession::instance()->profile(),
            m_universe->getScenes()[0]->getLevelSrc()->Id(),
            m_universe->getScenes()[0]->getTime() -
              m_universe->getScenes()[0]->getCheckpointStartTime());
          StateManager::instance()->getDbStatsThread()->doJob();
        }
      }
    }
  }

  restartLevelToPlay(i_reloadLevel);
}

void StatePlayingLocal::handleInput(InputEventType Type, const XMKey &i_xmkey) {
  handleControllers(Type, i_xmkey);
  handleScriptKeys(Type, i_xmkey);
}
