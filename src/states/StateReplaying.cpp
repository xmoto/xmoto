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

#include "StateReplaying.h"
#include "StateLevelInfoViewer.h"
#include "StateMessageBox.h"
#include "StatePreplayingReplay.h"
#include "StateViewHighscore.h"
#include "common/XMSession.h"
#include "drawlib/DrawLib.h"
#include "helpers/Log.h"
#include "helpers/Text.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"
#include "xmoto/Renderer.h"
#include "xmoto/SysMessage.h"
#include "xmoto/Trainer.h"
#include "xmoto/Universe.h"
#include "xmoto/VideoRecorder.h"
#include "xmscene/BikePlayer.h"
#include "xmscene/Camera.h"

#define NEXTLEVEL_MAXTRY 10

StateReplaying::StateReplaying(Universe *i_universe,
                               GameRenderer *i_renderer,
                               const std::string &i_replay,
                               ReplayBiker *i_replayBiker)
  : StateScene(i_universe, i_renderer) {
  m_name = "StateReplaying";
  m_replay = i_replay;
  m_replayBiker = i_replayBiker;
  m_stopToUpdate = false;
}

StateReplaying::~StateReplaying() {
  // don't clean the replay biker while the scene clean its players
  // delete m_replayBiker;
}

void StateReplaying::enter() {
  StateScene::enter();

  m_stopToUpdate = false;

  if (m_renderer != NULL) {
    m_renderer->setShowEngineCounter(false);

    if (XMSession::instance()->hidePlayingInformation() == false) {
      m_renderer->setShowMinimap(XMSession::instance()->showMinimap());
      m_renderer->setShowTimePanel(true);
    } else {
      m_renderer->setShowMinimap(false);
      m_renderer->setShowTimePanel(false);
    }

    m_renderer->setShowGhostsText(true);
  }

  try {
    if (XMSession::instance()->hidePlayingInformation() == false) {
      // display replay information
      char c_tmp[1024];
      snprintf(
        c_tmp, 1024, GAMETEXT_BY_PLAYER, m_replayBiker->playerName().c_str());

      for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
        m_universe->getScenes()[i]->setInfos(
          m_universe->getScenes()[i]->getLevelSrc()->Name() + " " +
          std::string(c_tmp));
      }
    }

    if (m_universe != NULL && m_renderer != NULL) {
      if (m_universe->getScenes().size() > 0 &&
          XMSession::instance()->hidePlayingInformation() == false) {
        m_renderer->showReplayHelp(
          m_universe->getScenes()[0]->getSpeed(),
          m_universe->getScenes()[0]->getLevelSrc()->isScripted() == false &&
            m_universe->getScenes()[0]->getLevelSrc()->isPhysics() == false);
      }
    }

    // music
    playLevelMusic();

    // highscores
    setScoresTimes();
  } catch (Exception &e) {
    abortPlaying();
    StateManager::instance()->replaceState(
      new StateMessageBox(this, splitText(e.getMsg(), 50), UI_MSGBOX_OK),
      getStateId());
    return;
  }
}

void StateReplaying::leave() {
  if (m_universe != NULL) {
    for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->setInfos("");
    }
  }
}

bool StateReplaying::update() {
  if (m_stopToUpdate) {
    return false;
  }

  if (StateScene::update() == false)
    return false;

  if ((m_replayBiker->isDead() || m_replayBiker->isFinished()) &&
      m_stopToUpdate == false) {
    m_stopToUpdate = true;

    if (XMSession::instance()->benchmark()) {
      m_requestForEnd = true;
      closePlaying();
      printf(" * %i frames rendered in %.2f seconds\n",
             m_benchmarkNbFrame,
             GameApp::getXMTime() - m_benchmarkStartTime);
      printf(" * Average framerate: %.2f fps\n",
             ((double)m_benchmarkNbFrame) /
               (GameApp::getXMTime() - m_benchmarkStartTime));
    }
  }

  return true;
}

void StateReplaying::nextLevel(bool i_positifOrder) {
  GameApp *pGame = GameApp::instance();
  std::string v_nextLevel;
  std::string v_currentLevel;

  // take the level id of the first world
  if (m_universe != NULL) {
    if (m_universe->getScenes().size() > 0) {
      v_currentLevel = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  std::string v_id_profile;
  std::string v_url;
  bool v_isAccessible;
  bool v_noPlayList = false;

  unsigned int maxTry = NEXTLEVEL_MAXTRY;
  unsigned int numTry = 0;
  while (numTry < maxTry && v_noPlayList != true) {
    if (i_positifOrder) {
      v_nextLevel = pGame->determineNextLevel(v_currentLevel);
    } else {
      v_nextLevel = pGame->determinePreviousLevel(v_currentLevel);
    }

    if (v_nextLevel != "") {
      LogDebug(
        "cur lvl [%s] next [%s]", v_currentLevel.c_str(), v_nextLevel.c_str());

      // if there's a highscore for the nextlevel in the main room of the
      // profile ?
      if (pGame->getHighscoreInfos(
            0, v_nextLevel, &v_id_profile, &v_url, &v_isAccessible)) {
        StateManager::instance()->replaceState(
          new StateViewHighscore(v_nextLevel, true), getStateId());
        break;
      } else {
        v_currentLevel = v_nextLevel;
      }
    } else {
      // abort because there is no playlist
      v_noPlayList = true;
    }

    numTry++;
  }

  if (numTry == maxTry && v_noPlayList != true) {
    char buf[512];
    snprintf(
      buf, 512, SYS_MSG_NO_NEXT_HIGHSCORE(NEXTLEVEL_MAXTRY), NEXTLEVEL_MAXTRY);
    SysMessage::instance()->displayText(buf);
  }
}

void StateReplaying::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (i_type == INPUT_DOWN &&
      i_xmkey == (*Input::instance()->getGlobalKey(INPUT_REPLAYINGSTOP))) {
    m_requestForEnd = true;
    closePlaying();
  }

  else if (i_type == INPUT_DOWN && i_xmkey == (*Input::instance()->getGlobalKey(
                                                INPUT_REPLAYINGFORWARD))) {
    /* Right arrow key: fast forward */
    if (m_stopToUpdate == false) {
      if (m_universe != NULL) {
        for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
          m_universe->getScenes()[i]->fastforward(100);
        }
      }
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == (*Input::instance()->getGlobalKey(
                                                INPUT_REPLAYINGREWIND))) {
    if (m_universe != NULL) {
      if (m_universe->getScenes().size() > 0) {
        if (m_universe->getScenes()[0]->getLevelSrc()->isScripted() == false &&
            m_universe->getScenes()[0]->getLevelSrc()->isPhysics() == false) {
          for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
            m_universe->getScenes()[i]->fastrewind(100);
          }
          m_stopToUpdate = false;
        } else {
          // rerun the replay
          restartLevel();
        }
      }
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == (*Input::instance()->getGlobalKey(
                                                INPUT_REPLAYINGPAUSE))) {
    /* pause */
    if (m_universe != NULL && m_renderer != NULL) {
      for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
        m_universe->getScenes()[i]->pause();
      }

      if (m_universe->getScenes().size() > 0 &&
          XMSession::instance()->hidePlayingInformation() == false) {
        m_renderer->showReplayHelp(
          m_universe->getScenes()[0]->getSpeed(),
          m_universe->getScenes()[0]->getLevelSrc()->isScripted() == false);
      }
    }
  }

  else if (i_type == INPUT_DOWN &&
           (i_xmkey ==
              (*Input::instance()->getGlobalKey(INPUT_REPLAYINGFASTER)) ||
            i_xmkey ==
              (*Input::instance()->getGlobalKey(INPUT_REPLAYINGABITFASTER)))) {
    /* faster */
    if (m_universe != NULL && m_renderer != NULL) {
      for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
        if (i_xmkey ==
            (*Input::instance()->getGlobalKey(INPUT_REPLAYINGFASTER))) {
          m_universe->getScenes()[i]->faster();
        } else {
          m_universe->getScenes()[i]->faster(0.01);
        }
      }
      if (m_universe->getScenes().size() > 0 &&
          XMSession::instance()->hidePlayingInformation() == false) {
        m_renderer->showReplayHelp(
          m_universe->getScenes()[0]->getSpeed(),
          m_universe->getScenes()[0]->getLevelSrc()->isScripted() == false);
      }
    }
  }

  else if (i_type == INPUT_DOWN &&
           (i_xmkey ==
              (*Input::instance()->getGlobalKey(INPUT_REPLAYINGSLOWER)) ||
            i_xmkey ==
              (*Input::instance()->getGlobalKey(INPUT_REPLAYINGABITSLOWER)))) {
    /* slower */
    if (m_universe != NULL) {
      for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
        if (i_xmkey ==
            (*Input::instance()->getGlobalKey(INPUT_REPLAYINGSLOWER))) {
          m_universe->getScenes()[i]->slower();
        } else {
          m_universe->getScenes()[i]->slower(0.01);
        }

        if (m_universe->getScenes()[i]->getSpeed() < 0.0) {
          m_stopToUpdate = false;
        }
      }
      if (m_renderer != NULL) {
        if (m_universe->getScenes().size() > 0 &&
            XMSession::instance()->hidePlayingInformation() == false) {
          m_renderer->showReplayHelp(
            m_universe->getScenes()[0]->getSpeed(),
            m_universe->getScenes()[0]->getLevelSrc()->isScripted() == false);
        }
      }
    }
  }
#if defined(ENABLE_DEV)
  else if (i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP_0, KMOD_NONE)) {
    // TRAINER
    /* store current bike position (for trainer) */
    if (m_universe != NULL) {
      for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
        Trainer::instance()->storePosition(
          m_universe->getScenes()[i]->getLevelSrc()->Id(),
          m_universe->getScenes()[i]->getPlayerPosition(0));
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
#endif

  else if (i_type == INPUT_DOWN &&
           i_xmkey == (*Input::instance()->getGlobalKey(INPUT_LEVELINFO))) {
    StateManager::instance()->pushState(new StateLevelInfoViewer(
      m_universe->getScenes()[0]->getLevelSrc()->Id(), true, false));
  }

  else {
    StateScene::xmKey(i_type, i_xmkey);
  }
}

void StateReplaying::restartLevel(bool i_reloadLevel) {
  closePlaying();
  StateManager::instance()->replaceState(
    new StatePreplayingReplay(m_replay, true), getStateId());
}
