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

#include "StateScene.h"
#include "StateMainMenu.h"
#include "StateMessageBox.h"
#include "StatePreplayingGame.h"
#include "StateVote.h"
#include "common/CameraAnimation.h"
#include "common/XMSession.h"
#include "drawlib/DrawLib.h"
#include "helpers/Log.h"
#include "helpers/Text.h"
#include "net/NetActions.h"
#include "net/NetClient.h"
#include "thread/XMThreadStats.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"
#include "xmoto/PhysSettings.h"
#include "xmoto/Renderer.h"
#include "xmoto/Replay.h"
#include "xmoto/SysMessage.h"
#include "xmoto/Universe.h"
#include "xmoto/VideoRecorder.h"
#include "xmscene/Camera.h"
#include "xmscene/Entity.h"
#include "xmscene/Scene.h"

#define INPLAY_ANIMATION_TIME 1.0
#define INPLAY_ANIMATION_SPEED 10
#define PRESTART_ANIMATION_MARGIN_SIZE 5

/* control the particle generation by ask the particle renders to limit themself
 * if there are too much particles on the screen */
#define NB_PARTICLES_TO_RENDER_LIMITATION 512

#define STATS_LEVELS_NOTES_SIZE 15

void StateScene::init(bool i_doShade, bool i_doShadeAnim) {
  Sprite *v_sprite;

  m_type = "SCENE";

  // shade
  m_doShade = i_doShade;
  m_doShadeAnim = i_doShadeAnim;

  m_fLastPhysTime = -1.0;
  // while playing, we want 100 fps for the physic
  m_updateFps = 100;
  m_showCursor = false;
  m_cameraAnim = NULL;
  m_universe = NULL;
  m_renderer = NULL;

  m_benchmarkNbFrame = 0;
  m_benchmarkStartTime = GameApp::getXMTime();

  /* stats */
  m_difficulty = -1.0;
  m_quality = -1.0;

  m_uncheckedTex = m_qualityTex = m_difficultyTex = NULL;
  v_sprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "qsChoiceUnchecked");
  if (v_sprite != NULL) {
    m_uncheckedTex = v_sprite->getTexture();
  }

  v_sprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "qsChoiceQuality");
  if (v_sprite != NULL) {
    m_qualityTex = v_sprite->getTexture();
  }

  v_sprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "qsChoiceDifficulty");
  if (v_sprite != NULL) {
    m_difficultyTex = v_sprite->getTexture();
  }

  m_trackingShotMode = false;

  // message registering
  initMessageRegistering();
}

StateScene::StateScene(bool i_doShade, bool i_doShadeAnim)
  : GameState(false, false) {
  init(i_doShade, i_doShadeAnim);
}

StateScene::StateScene(Universe *i_universe,
                       GameRenderer *i_renderer,
                       bool i_doShade,
                       bool i_doShadeAnim)
  : GameState(false, false) {
  init(i_doShade, i_doShadeAnim);
  m_universe = i_universe;
  m_renderer = i_renderer;
}

StateScene::~StateScene() {
  StateManager::instance()->unregisterAsObserver("ERROR", this);
  StateManager::instance()->unregisterAsObserver("FINISH", this);
  StateManager::instance()->unregisterAsObserver("RESTART", this);
  StateManager::instance()->unregisterAsObserver("NEXTLEVEL", this);
  StateManager::instance()->unregisterAsObserver("PREVIOUSLEVEL", this);
  StateManager::instance()->unregisterAsObserver("ABORT", this);
  StateManager::instance()->unregisterAsObserver("INTERPOLATION_CHANGED", this);
  StateManager::instance()->unregisterAsObserver("MIRRORMODE_CHANGED", this);
  StateManager::instance()->unregisterAsObserver("REPLAY_DOWNLOADED", this);
  StateManager::instance()->unregisterAsObserver("REPLAY_FAILEDTODOWNLOAD",
                                                 this);
  StateManager::instance()->unregisterAsObserver("CHANGE_TRAILCAM", this);

  if (m_cameraAnim != NULL) {
    delete m_cameraAnim;
  }
}

void StateScene::initMessageRegistering() {
  StateManager::instance()->registerAsObserver("ERROR", this);
  StateManager::instance()->registerAsObserver("FINISH", this);
  StateManager::instance()->registerAsObserver("RESTART", this);
  StateManager::instance()->registerAsObserver("NEXTLEVEL", this);
  StateManager::instance()->registerAsObserver("PREVIOUSLEVEL", this);
  StateManager::instance()->registerAsObserver("ABORT", this);
  StateManager::instance()->registerAsObserver("INTERPOLATION_CHANGED", this);
  StateManager::instance()->registerAsObserver("MIRRORMODE_CHANGED", this);
  StateManager::instance()->registerAsObserver("REPLAY_DOWNLOADED", this);
  StateManager::instance()->registerAsObserver("REPLAY_FAILEDTODOWNLOAD", this);
  StateManager::instance()->registerAsObserver("CHANGE_TRAILCAM", this);

  if (XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("FAVORITES_UPDATED");
    StateManager::instance()->registerAsEmitter("BLACKLISTEDLEVELS_UPDATED");
  }
}

void StateScene::leaveAfterPush() {
  // if the shade is set to false, force it when state receives one over
  if (m_doShade == false) {
    if (m_renderer != NULL) {
      m_renderer->setScreenShade(true, true, GameApp::getXMTime());
    }
  }
}

void StateScene::leaveType() {
  // violent closing of the state, closePlaying if not done (ie QUIT event)
  if (m_universe != NULL) {
    closePlaying();
  }
}

void StateScene::enter() {
  GameState::enter();

  if (m_renderer != NULL) {
    m_renderer->setScreenShade(m_doShade, m_doShadeAnim, GameApp::getXMTime());
  }

  ParticlesSource::setAllowParticleGeneration(true);
  m_isLockedScene = false;
  m_autoZoom = false;

  m_benchmarkNbFrame = 0;
  m_benchmarkStartTime = GameApp::getXMTime();

  m_fLastPhysTime = GameApp::getXMTime();
}

void StateScene::enterAfterPop() {
  GameState::enterAfterPop();

  if (m_doShade == false) {
    if (m_renderer != NULL) {
      m_renderer->setScreenShade(false, false, GameApp::getXMTime());
    }
  }
}

bool StateScene::update() {
  if (doUpdate() == false) {
    return false;
  }

  try {
    int nPhysSteps = 0;

    if (isLockedScene() == false) {
      // don't update if that's not required
      // don't do this infinitely, maximum miss 10 frames, then give up
      // in videoRecording mode, don't try to do more to allow to record at a
      // good framerate
      while (
        (m_fLastPhysTime + (PHYS_STEP_SIZE) / 100.0 <= GameApp::getXMTime()) &&
        nPhysSteps < 10 &&
        (XMSession::instance()->enableVideoRecording() == false ||
         nPhysSteps == 0)) {
        if (m_universe != NULL) {
          for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
            m_universe->getScenes()[i]->updateLevel(
              PHYS_STEP_SIZE,
              m_universe->getCurrentReplay(),
              m_universe->getCurrentReplay());
          }
        }
        m_fLastPhysTime += PHYS_STEP_SIZE / 100.0;
        nPhysSteps++;
      }

      // if the delay is too long, reinitialize
      if (m_fLastPhysTime + PHYS_STEP_SIZE / 100.0 < GameApp::getXMTime()) {
        m_fLastPhysTime = GameApp::getXMTime();
      }

      // update camera scrolling
      if (m_universe != NULL) {
        for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
          for (unsigned int i = 0;
               i < m_universe->getScenes()[j]->Cameras().size();
               i++) {
            m_universe->getScenes()[j]->Cameras()[i]->setScroll(
              true, m_universe->getScenes()[j]->getGravity());
          }
        }
      }
    }
  } catch (Exception &e) {
    StateManager::instance()->replaceState(
      new StateMessageBox(NULL, splitText(e.getMsg(), 50), UI_MSGBOX_OK),
      getStateId());
  }

  runAutoZoom();

  return true;
}

bool StateScene::render() {
  GameApp *pGame = GameApp::instance();

  if (XMSession::instance()->ugly()) {
    pGame->getDrawLib()->clearGraphics();
  }

  try {
    if (autoZoom() == false) {
      if (m_universe != NULL && m_renderer != NULL) {
        for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
          for (unsigned int i = 0;
               i < m_universe->getScenes()[j]->getNumberCameras();
               i++) {
            m_universe->getScenes()[j]->setCurrentCamera(i);
            m_renderer->render(m_universe->getScenes()[j]);
          }
        }
        // Render the game messages for OVER the shadow layer, which is needed
        // in multiplayer to supress ugly multiple display
        m_renderer->renderGameMessages(m_universe->getScenes()[0]);
      }
    } else {
      if (m_universe != NULL && m_renderer != NULL) {
        if (m_universe->getScenes().size() > 0) {
          m_universe->getScenes()[0]->setAutoZoomCamera();
          m_renderer->render(m_universe->getScenes()[0]);
          m_renderer->renderGameMessages(m_universe->getScenes()[0]);
        }
      }
    }

    if (m_renderer != NULL) {
      ParticlesSource::setAllowParticleGeneration(
        m_renderer->nbParticlesRendered() < NB_PARTICLES_TO_RENDER_LIMITATION);
    }

    // downloading ghost information
    if (m_universe != NULL) {
      if (m_universe->waitingForGhosts()) {
        FontManager *v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
        FontGlyph *v_fg = v_fm->getGlyph(GAMETEXT_DLGHOSTS);
        int v_border = 5;

        GameApp::instance()->getDrawLib()->drawBox(
          Vector2f(
            m_screen.getDispWidth() / 2 - v_fg->realWidth() / 2 - v_border,
            m_screen.getDispHeight() - v_fg->realHeight() - 2 * v_border),
          Vector2f(m_screen.getDispWidth() / 2 + v_fg->realWidth() / 2 +
                     v_border,
                   m_screen.getDispHeight() + v_border),
          0.0f,
          MAKE_COLOR(255, 255, 255, 50));

        v_fm->printString(GameApp::instance()->getDrawLib(),
                          v_fg,
                          m_screen.getDispWidth() / 2 - v_fg->realWidth() / 2,
                          m_screen.getDispHeight() - v_fg->realHeight() -
                            v_border,
                          MAKE_COLOR(255, 255, 255, 255),
                          0.0,
                          true);
      }
    }

  } catch (Exception &e) {
    StateManager::instance()->replaceState(
      new StateMessageBox(NULL, splitText(e.getMsg(), 50), UI_MSGBOX_OK),
      getStateId());
  }

  GameState::render();
  m_benchmarkNbFrame++;

  return true;
}

void StateScene::onRenderFlush() {
  // take a screenshot
  if (XMSession::instance()->enableVideoRecording()) {
    if (StateManager::instance()->getVideoRecorder() != NULL) {
      if (m_universe != NULL) {
        if (m_universe->getScenes().size() > 0) {
          if ((XMSession::instance()->videoRecordingStartTime() < 0 ||
               XMSession::instance()->videoRecordingStartTime() <=
                 m_universe->getScenes()[0]->getTime()) &&
              (XMSession::instance()->videoRecordingEndTime() < 0 ||
               XMSession::instance()->videoRecordingEndTime() >=
                 m_universe->getScenes()[0]->getTime())) {
            StateManager::instance()->getVideoRecorder()->read(
              m_universe->getScenes()[0]->getTime());
          }
        }
      }
    }
  }
}

void StateScene::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  GameApp *pGame = GameApp::instance();

  if (i_xmkey == (*Input::instance()->getGlobalKey(INPUT_LEVELWATCHING))) {
    if (i_type == INPUT_UP) {
      if (m_cameraAnim != NULL) {
        if (autoZoom() && m_cameraAnim->allowNextStep()) {
          m_cameraAnim->goNextStep();
        }
      }
    } else {
      if (autoZoom() == false) {
        setAutoZoom(true);
      }
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == (*Input::instance()->getGlobalKey(
                                                INPUT_SWITCHSAFEMODE))) {
    bool bSafemodeNotActive = !XMSession::instance()->isSafemodeActive();

    XMSession::instance()->setSafemodeActive(bSafemodeNotActive);
    SysMessage::instance()->displayText(bSafemodeNotActive
                                          ? GAMETEXT_SAFEMODE_ENABLED
                                          : GAMETEXT_SAFEMODE_DISABLED);
  }

  else if (i_type == INPUT_DOWN &&
           i_xmkey == (*Input::instance()->getGlobalKey(INPUT_RESTARTLEVEL))) {
    if (!XMSession::instance()->isSafemodeActive())
      restartLevel();
  }

  else if (i_type == INPUT_DOWN &&
           i_xmkey == (*Input::instance()->getGlobalKey(INPUT_SWITCHPLAYER))) {
    if (m_universe != NULL) {
      m_universe->switchFollowCamera();
    }
  }

  else if (i_type == INPUT_DOWN &&
           i_xmkey ==
             (*Input::instance()->getGlobalKey(INPUT_SWITCHTRACKINGSHOTMODE))) {
    if (m_universe != NULL && m_renderer != NULL) {
      m_trackingShotMode = !m_trackingShotMode;

      for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
        if (m_trackingShotMode) {
          if (m_universe->getScenes()[i]->getGhostTrail() != NULL) {
            if (m_universe->getScenes()[i]->isPaused() == false) {
              // toogle cameras
              for (unsigned int j = 0;
                   j < m_universe->getScenes()[i]->Cameras().size();
                   j++) {
                m_universe->getScenes()[i]->Cameras()[j]->setUseTrackingShot(
                  true);
              }
              // toogle pause mode
              m_universe->getScenes()[i]->pause();
              // toogle in the renderer
              m_renderer->setRenderGhostTrail(true);
            }
          }
        } else {
          // toogle cameras
          for (unsigned int j = 0;
               j < m_universe->getScenes()[i]->Cameras().size();
               j++) {
            m_universe->getScenes()[i]->Cameras()[j]->setUseTrackingShot(false);
          }
          // toogle pause mode
          if (m_universe->getScenes()[i]->isPaused()) {
            m_universe->getScenes()[i]
              ->pause(); // cause the man cause unpause while tracking shot
          }
          // toogle in the renderer
          m_renderer->setRenderGhostTrail(
            XMSession::instance()->renderGhostTrail());
        }
      }
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == (*Input::instance()->getGlobalKey(
                                                INPUT_SWITCHFAVORITE))) {
    if (m_universe != NULL) {
      if (m_universe->getScenes().size() > 0) { // just add the first world
        pGame->switchLevelToFavorite(
          m_universe->getScenes()[0]->getLevelSrc()->Id(), true);
        StateManager::instance()->sendAsynchronousMessage("FAVORITES_UPDATED");
      }
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == (*Input::instance()->getGlobalKey(
                                                INPUT_SWITCHBLACKLIST))) {
    if (m_universe != NULL) {
      if (m_universe->getScenes().size() >
          0) { // just blacklist the first world
        pGame->switchLevelToBlacklist(
          m_universe->getScenes()[0]->getLevelSrc()->Id(), true);
        StateManager::instance()->sendAsynchronousMessage(
          "BLACKLISTEDLEVELS_UPDATED");
      }
    }
  }

  else if (i_type == INPUT_DOWN &&
           i_xmkey == (*Input::instance()->getGlobalKey(INPUT_NEXTLEVEL))) {
    if (!XMSession::instance()->isSafemodeActive())
      nextLevel();
  }

  else if (i_type == INPUT_DOWN &&
           i_xmkey == (*Input::instance()->getGlobalKey(INPUT_PREVIOUSLEVEL))) {
    if (!XMSession::instance()->isSafemodeActive())
      nextLevel(false);
  }

  else if (i_type == INPUT_DOWN &&
           i_xmkey == (*Input::instance()->getGlobalKey(
                        INPUT_SWITCHHIGHSCOREINFORMATION))) {
    if (m_renderer != NULL) {
      if (XMSession::instance()->showHighscoreInGame() == false) {
        XMSession::instance()->setShowHighscoreInGame(true);
      }

      else if (XMSession::instance()->showHighscoreInGame() == true &&
               XMSession::instance()->showNextMedalInGame() == false) {
        XMSession::instance()->setNextMedalInGame(true);
      } else if (XMSession::instance()->showHighscoreInGame() == true &&
                 XMSession::instance()->showNextMedalInGame() == true) {
        XMSession::instance()->setNextMedalInGame(false);
        XMSession::instance()->setShowHighscoreInGame(false);
      }
      setScoresTimes();
    }
  }

  else if (i_type == INPUT_DOWN &&
           i_xmkey ==
             (*Input::instance()->getGlobalKey(INPUT_SWITCHRENDERGHOSTTRAIL))) {
    // toogle
    XMSession::instance()->setRenderGhostTrail(
      !XMSession::instance()->renderGhostTrail());
    if (m_renderer != NULL) {
      m_renderer->setRenderGhostTrail(
        XMSession::instance()->renderGhostTrail());
    }

    bool v_trailAvailable = false;
    if (m_universe != NULL) {
      for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
        if (m_universe->getScenes()[i]->getGhostTrail() != NULL) {
          v_trailAvailable = true;
        }
      }
    }

    if (v_trailAvailable) {
      if (XMSession::instance()->renderGhostTrail()) {
        SysMessage::instance()->displayText(SYS_MSG_TRAIL_VISIBLE);
      } else {
        SysMessage::instance()->displayText(SYS_MSG_TRAIL_INVISIBLE);
      }
    } else {
      SysMessage::instance()->displayText(SYS_MSG_TRAIL_NA);
    }
  }
#if defined(ENABLE_DEV)
  else if (i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP_7, KMOD_NONE)) {
    /* Zoom in */
    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        for (unsigned int i = 0;
             i < m_universe->getScenes()[j]->Cameras().size();
             i++) {
          m_universe->getScenes()[j]->Cameras()[i]->desactiveActionZoom();
          m_universe->getScenes()[j]->Cameras()[i]->setRelativeZoom(0.002);
        }
      }
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP_9, KMOD_NONE)) {
    /* Zoom out */
    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        for (unsigned int i = 0;
             i < m_universe->getScenes()[j]->Cameras().size();
             i++) {
          m_universe->getScenes()[j]->Cameras()[i]->desactiveActionZoom();
          m_universe->getScenes()[j]->Cameras()[i]->setRelativeZoom(-0.002);
        }
      }
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_HOME, KMOD_NONE)) {
    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        for (unsigned int i = 0;
             i < m_universe->getScenes()[j]->Cameras().size();
             i++) {
          m_universe->getScenes()[j]->Cameras()[i]->desactiveActionZoom();
          m_universe->getScenes()[j]->Cameras()[i]->initCamera();
        }
      }
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP_6, KMOD_NONE)) {
    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        for (unsigned int i = 0;
             i < m_universe->getScenes()[j]->Cameras().size();
             i++) {
          m_universe->getScenes()[j]->Cameras()[i]->desactiveActionZoom();
          m_universe->getScenes()[j]->Cameras()[i]->moveCamera(1.0, 0.0);
        }
      }
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP_4, KMOD_NONE)) {
    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        for (unsigned int i = 0;
             i < m_universe->getScenes()[j]->Cameras().size();
             i++) {
          m_universe->getScenes()[j]->Cameras()[i]->desactiveActionZoom();
          m_universe->getScenes()[j]->Cameras()[i]->moveCamera(-1.0, 0.0);
        }
      }
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP_8, KMOD_NONE)) {
    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        for (unsigned int i = 0;
             i < m_universe->getScenes()[j]->Cameras().size();
             i++) {
          m_universe->getScenes()[j]->Cameras()[i]->desactiveActionZoom();
          m_universe->getScenes()[j]->Cameras()[i]->moveCamera(0.0, 1.0);
        }
      }
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP_2, KMOD_NONE)) {
    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        for (unsigned int i = 0;
             i < m_universe->getScenes()[j]->Cameras().size();
             i++) {
          m_universe->getScenes()[j]->Cameras()[i]->desactiveActionZoom();
          m_universe->getScenes()[j]->Cameras()[i]->moveCamera(0.0, -1.0);
        }
      }
    }
  }
#endif

  else if (i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_v, KMOD_LCTRL)) {
    if (XMSession::instance()->adminMode()) {
      if (m_universe != NULL) {
        if (m_universe->getScenes().size() == 1) {
          StateManager::instance()->pushState(
            new StateVote(m_universe->getScenes()[0]->getLevelSrc()->Id()));
        }
      }
    }
  }

  else {
    GameState::xmKey(i_type, i_xmkey);
  }
}

void StateScene::sendFromMessageBox(const std::string &i_id,
                                    UIMsgBoxButton i_button,
                                    const std::string &i_input) {
  if (i_id == "ERROR") {
    addCommand("ERROR");
  }

  else {
    GameState::sendFromMessageBox(i_id, i_button, i_input);
  }
}

void StateScene::setScoresTimes() {
  char **v_result;
  unsigned int nrow;
  char *v_res;
  int v_best_room_time = -1;
  int v_best_player_time = -1;

  std::string T1 = "--:--:--", T2 = "--:--:--", T3 = "--:--:--";
  std::string v_id_level;
  // take the level id of the first world
  if (m_universe != NULL) {
    if (m_universe->getScenes().size() > 0) {
      v_id_level = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  /* get best result */
  v_result = xmDatabase::instance("main")->readDB(
    "SELECT MIN(finishTime+0) FROM profile_completedLevels WHERE "
    "id_level=\"" +
      xmDatabase::protectString(v_id_level) + "\";",
    nrow);
  v_res = xmDatabase::instance("main")->getResult(v_result, 1, 0, 0);
  if (v_res != NULL) {
    T1 = formatTime(atoi(v_res));
  }
  xmDatabase::instance("main")->read_DB_free(v_result);

  /* get best player result */
  v_result = xmDatabase::instance("main")->readDB(
    "SELECT MIN(finishTime+0) FROM profile_completedLevels WHERE "
    "id_level=\"" +
      xmDatabase::protectString(v_id_level) + "\" " + "AND id_profile=\"" +
      xmDatabase::protectString(XMSession::instance()->profile()) + "\";",
    nrow);
  v_res = xmDatabase::instance("main")->getResult(v_result, 1, 0, 0);
  if (v_res != NULL) {
    v_best_player_time = atoi(v_res);
    T2 = formatTime(atoi(v_res));
  }
  xmDatabase::instance("main")->read_DB_free(v_result);

  if (m_renderer != NULL) {
    if (XMSession::instance()->hidePlayingInformation() == false) {
      m_renderer->setBestTime(T1 + std::string(" / ") + T2);
    } else {
      m_renderer->setBestTime("");
    }
  }

  if (m_renderer != NULL) {
    if (XMSession::instance()->showHighscoreInGame() &&
        XMSession::instance()->hidePlayingInformation() == false) {
      std::string v_nextMedal;
      std::string v_author;
      int v_nextMedal_time;
      bool v_isNextMedal;

      if (XMSession::instance()->nbRoomsEnabled() >= 1) {
        std::string v_best_str;
        v_best_str = GameApp::instance()->getWorldRecord(
          0, v_id_level, v_best_room_time, v_author);
      }

      v_isNextMedal =
        GameApp::instance()->getNextMedal(XMSession::instance()->profile(),
                                          v_author,
                                          v_best_room_time,
                                          v_best_player_time,
                                          v_nextMedal,
                                          v_nextMedal_time);

      /* won't next medal, or next medal doesn't exist */
      if (XMSession::instance()->showNextMedalInGame() == false ||
          v_isNextMedal == false) {
        std::string v_strWorldRecord;
        int v_best_time;
        for (unsigned int i = 0; i < XMSession::instance()->nbRoomsEnabled();
             i++) {
          v_strWorldRecord += GameApp::instance()->getWorldRecord(
                                i, v_id_level, v_best_time, v_author) +
                              "\n";
        }
        m_renderer->setWorldRecordTime(v_strWorldRecord);
      } else {
        /* want next medal and next medal exists */
        T3 = formatTime(v_nextMedal_time);
        m_renderer->setWorldRecordTime(v_nextMedal + std::string(": ") + T3);
      }

    } else {
      m_renderer->setWorldRecordTime("");
    }
  }
}

void StateScene::abortPlaying() {
  closePlaying();
}

void StateScene::closePlaying() {
  if (NetClient::instance()->isConnected()) {
    // stop playing
    NA_playingLevel na("");
    NetClient::instance()->send(&na, 0);
  }

  if (NetClient::instance()->isPlayInitialized()) {
    NetClient::instance()->endPlay();
  }

  if (m_renderer != NULL) {
    m_renderer->unprepareForNewLevel(m_universe);
    delete m_renderer;
    m_renderer = NULL;
  }

  if (m_universe != NULL) {
    delete m_universe;
    m_universe = NULL;
  }

  Input::instance()->resetScriptKeyHooks();
}

bool StateScene::isLockedScene() const {
  return m_isLockedScene;
}

void StateScene::lockScene(bool i_value) {
  m_isLockedScene = i_value;
  if (m_isLockedScene == false) {
    m_fLastPhysTime = GameApp::getXMTime();
  }
}

void StateScene::setAutoZoom(bool i_value) {
  if (m_autoZoom == false && i_value == true) {
    lockScene(true);

    if (m_cameraAnim != NULL) {
      delete m_cameraAnim;
    }

    if (m_universe != NULL && m_renderer != NULL) {
      if (m_universe->getScenes().size() >
          0) { // do only for the first world for the moment
        m_universe->getScenes()[0]->setAutoZoomCamera();
        m_cameraAnim =
          new AutoZoomCameraAnimation(m_universe->getScenes()[0]->getCamera(),
                                      &m_screen,
                                      m_renderer,
                                      m_universe->getScenes()[0]);
        m_cameraAnim->init();
      }
    }
  }

  m_autoZoom = i_value;
}

bool StateScene::autoZoom() const {
  return m_autoZoom;
}

void StateScene::runAutoZoom() {
  if (autoZoom()) {
    if (m_cameraAnim != NULL) {
      if (m_cameraAnim->step() == false) {
        lockScene(false);
        m_cameraAnim->uninit();
        setAutoZoom(false);
      }
    }
  }
}

void StateScene::executeOneCommand(std::string cmd, std::string args) {
  LogDebug("cmd [%s [%s]] executed by state [%s].",
           cmd.c_str(),
           args.c_str(),
           getName().c_str());

  if (cmd == "ERROR") {
    closePlaying();

    // there is no other state before
    if (StateManager::instance()->numberOfStates() == 1) {
      // run the mainmenu state
      StateManager::instance()->replaceState(new StateMainMenu(), getStateId());
    } else {
      m_requestForEnd = true;
    }
  }

  else if (cmd == "FINISH") {
    closePlaying();

    // there is no other state before
    if (StateManager::instance()->numberOfStates() == 1) {
      // run the mainmenu state
      StateManager::instance()->replaceState(new StateMainMenu(), getStateId());
    } else {
      m_requestForEnd = true;
    }
  }

  else if (cmd == "RESTART") {
    restartLevel();
  }

  else if (cmd == "NEXTLEVEL") {
    nextLevel();
  }

  else if (cmd == "PREVIOUSLEVEL") {
    nextLevel(false);
  }

  else if (cmd == "ABORT") {
    abortPlaying();

    // there is no other state before
    if (StateManager::instance()->numberOfStates() == 1) {
      // run the mainmenu state
      StateManager::instance()->replaceState(new StateMainMenu(), getStateId());
    } else {
      m_requestForEnd = true;
    }
  }

  else if (cmd == "INTERPOLATION_CHANGED") {
    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        for (unsigned int i = 0;
             i < m_universe->getScenes()[j]->Players().size();
             i++) {
          m_universe->getScenes()[j]->Players()[i]->setInterpolation(
            XMSession::instance()->enableReplayInterpolation());
        }
      }
    }
  }

  else if (cmd == "MIRRORMODE_CHANGED") {
    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        for (unsigned int i = 0;
             i < m_universe->getScenes()[j]->Cameras().size();
             i++) {
          m_universe->getScenes()[j]->Cameras()[i]->setMirrored(
            XMSession::instance()->mirrorMode());
        }
      }
    }
  }

  else if (cmd == "CHANGE_TRAILCAM") {
    if (m_universe != NULL) {
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        for (unsigned int i = 0;
             i < m_universe->getScenes()[j]->Cameras().size();
             i++) {
          m_universe->getScenes()[j]->Cameras()[i]->setUseTrailCam(
            XMSession::instance()->enableTrailCam());
        }
      }
    }
  }

  else if (cmd == "REPLAY_DOWNLOADED") {
    if (m_universe != NULL) {
      m_universe->markDownloadedGhost(args, true);
    }
  }

  else if (cmd == "REPLAY_FAILEDTODOWNLOAD") {
    /* remove it from the waiting replays to be downloaded */
    if (m_universe != NULL) {
      m_universe->markDownloadedGhost(args, false);
    }
  }

  else {
    GameState::executeOneCommand(cmd, args);
  }
}

void StateScene::displayStats() {
  DrawLib *drawLib = GameApp::instance()->getDrawLib();
  FontManager *v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
  FontGlyph *v_fg = v_fm->getGlyph(m_statsStr);
  Vector2f A = Vector2f(m_screen.getDispWidth() - v_fg->realWidth(),
                        m_screen.getDispHeight() - v_fg->realHeight());
  Vector2f B = Vector2f(m_screen.getDispWidth(), m_screen.getDispHeight());
  int vborder = 10;

  GameApp::instance()->getDrawLib()->drawBox(
    A - Vector2f(vborder * 2, vborder * 2), B, 1.0f, 0xFFCCCC77, 0xFFFFFFFF);

  v_fm->printString(drawLib,
                    v_fg,
                    m_screen.getDispWidth() - v_fg->realWidth() - vborder,
                    m_screen.getDispHeight() - v_fg->realHeight() - vborder,
                    MAKE_COLOR(220, 255, 255, 255),
                    -1.0,
                    true);

  // quality
  int v_quality_yoffset = 5;

  if (m_quality >= 0.0) {
    v_fg = drawLib->getFontSmall()->getGlyph(GAMETEXT_QUALITY);
    v_fm->printString(drawLib,
                      v_fg,
                      A.x - vborder * 2,
                      A.y - vborder * 2 - v_fg->realHeight() -
                        STATS_LEVELS_NOTES_SIZE - v_quality_yoffset,
                      MAKE_COLOR(220, 255, 255, 255),
                      -1.0,
                      true);

    if (XMSession::instance()->ugly()) {
      for (int i = 0; i < (int)(m_quality); i++) {
        drawLib->drawCircle(
          Vector2f(A.x - vborder * 2 + STATS_LEVELS_NOTES_SIZE / 2 +
                     (STATS_LEVELS_NOTES_SIZE * i),
                   A.y - vborder * 2 - STATS_LEVELS_NOTES_SIZE / 2 -
                     v_quality_yoffset),
          STATS_LEVELS_NOTES_SIZE / 2,
          1.0,
          0,
          MAKE_COLOR(255, 0, 0, 255));
      }
    } else {
      for (int i = 0; i < 5; i++) {
        drawLib->drawImage(
          Vector2f(A.x - vborder * 2 + (STATS_LEVELS_NOTES_SIZE * i),
                   A.y - vborder * 2 - STATS_LEVELS_NOTES_SIZE -
                     v_quality_yoffset),
          Vector2f(A.x - vborder * 2 + STATS_LEVELS_NOTES_SIZE +
                     (STATS_LEVELS_NOTES_SIZE * i),
                   A.y - vborder * 2 - v_quality_yoffset),
          (i < (int)m_quality) ? m_qualityTex : m_uncheckedTex,
          0xFFFFFFFF,
          true);
      }
    }
  }

  // difficulty
  int v_difficulty_yoffset =
    v_fg->realHeight() + STATS_LEVELS_NOTES_SIZE + v_quality_yoffset;

  if (m_difficulty >= 0.0) {
    v_fg = drawLib->getFontSmall()->getGlyph(GAMETEXT_DIFFICULTY);
    v_fm->printString(drawLib,
                      v_fg,
                      A.x - vborder * 2,
                      A.y - vborder * 2 - v_fg->realHeight() -
                        STATS_LEVELS_NOTES_SIZE - v_difficulty_yoffset,
                      MAKE_COLOR(220, 255, 255, 255),
                      -1.0,
                      true);

    if (XMSession::instance()->ugly()) {
      for (int i = 0; i < (int)(m_difficulty); i++) {
        drawLib->drawCircle(
          Vector2f(A.x - vborder * 2 + STATS_LEVELS_NOTES_SIZE / 2 +
                     (STATS_LEVELS_NOTES_SIZE * i),
                   A.y - vborder * 2 - STATS_LEVELS_NOTES_SIZE / 2 -
                     v_difficulty_yoffset),
          STATS_LEVELS_NOTES_SIZE / 2,
          1.0,
          0,
          MAKE_COLOR(255, 0, 0, 255));
      }
    } else {
      for (int i = 0; i < 5; i++) {
        drawLib->drawImage(
          Vector2f(A.x - vborder * 2 + (STATS_LEVELS_NOTES_SIZE * i),
                   A.y - vborder * 2 - STATS_LEVELS_NOTES_SIZE -
                     v_difficulty_yoffset),
          Vector2f(A.x - vborder * 2 + STATS_LEVELS_NOTES_SIZE +
                     (STATS_LEVELS_NOTES_SIZE * i),
                   A.y - vborder * 2 - v_difficulty_yoffset),
          (i < (int)m_difficulty) ? m_difficultyTex : m_uncheckedTex,
          0xFFFFFFFF,
          true);
      }
    }
  }
}

void StateScene::makeStatsStr() {
  m_statsStr = "";
  if (m_universe != NULL) {
    if (m_universe->getScenes().size() > 0) {
      // stats to display
      char **v_result;
      unsigned int nrow;
      xmDatabase *v_pDb = xmDatabase::instance("main");
      int v_nbPlayed, v_nbDied, v_nbCompleted, v_nbRestart, v_playedTime;
      std::string v_idLevel = m_universe->getScenes()[0]->getLevelSrc()->Id();

      v_result = v_pDb->readDB(
        "SELECT IFNULL(SUM(nbPlayed),0), IFNULL(SUM(nbDied),0), "
        "IFNULL(SUM(nbCompleted),0), IFNULL(SUM(nbRestarted),0), "
        "IFNULL(SUM(playedTime),0) "
        "FROM stats_profiles_levels "
        "WHERE id_profile=\"" +
          xmDatabase::protectString(XMSession::instance()->profile()) +
          "\" "
          "AND id_level=\"" +
          xmDatabase::protectString(v_idLevel) + "\" GROUP BY id_profile;",
        nrow);
      if (nrow != 1) {
        /* not statistics */
        v_nbPlayed = 0;
        v_nbDied = 0;
        v_nbCompleted = 0;
        v_nbRestart = 0;
        v_playedTime = 0;
      } else {
        v_nbPlayed = atoi(v_pDb->getResult(v_result, 5, 0, 0));
        v_nbDied = atoi(v_pDb->getResult(v_result, 5, 0, 1));
        v_nbCompleted = atoi(v_pDb->getResult(v_result, 5, 0, 2));
        v_nbRestart = atoi(v_pDb->getResult(v_result, 5, 0, 3));
        v_playedTime = atoi(v_pDb->getResult(v_result, 5, 0, 4));
      }
      char c_tmp[256];
      snprintf(
        c_tmp,
        256,
        std::string(
          GAMETEXT_XMOTOLEVELSTATS_PLAYS(v_nbPlayed) + std::string("\n") +
          GAMETEXT_XMOTOLEVELSTATS_FINISHED(v_nbCompleted) + std::string("\n") +
          GAMETEXT_XMOTOLEVELSTATS_DEATHS(v_nbDied) + std::string("\n") +
          GAMETEXT_XMOTOLEVELSTATS_RESTART(v_nbRestart) + std::string("\n") +
          GAMETEXT_XMOTOGLOBALSTATS_TIMEPLAYED)
          .c_str(),
        v_nbPlayed,
        v_nbCompleted,
        v_nbDied,
        v_nbRestart,
        formatTime(v_playedTime).c_str());
      m_statsStr = c_tmp;
      v_pDb->read_DB_free(v_result);

      // quality and difficulty
      v_result = v_pDb->readDB("SELECT difficulty, quality "
                               "FROM weblevels "
                               "WHERE id_level=\"" +
                                 xmDatabase::protectString(v_idLevel) + "\";",
                               nrow);
      if (nrow != 1) {
        /* not statistics */
        m_difficulty = -1.0;
        m_quality = -1.0;
      } else {
        m_difficulty = atof(v_pDb->getResult(v_result, 2, 0, 0));
        m_quality = atof(v_pDb->getResult(v_result, 2, 0, 1));
      }
      v_pDb->read_DB_free(v_result);
    }
  }
}

void StateScene::restartLevel(bool i_reloadLevel) {
  /* do nothing, it's depends of the scene ; often empty for animation steps */
}

void StateScene::nextLevel(bool i_positifOrder) {
  /* do nothing, it's depends of the scene ; often empty for animation steps */
}

void StateScene::playingNextLevel(bool i_positifOrder) {
  GameApp *pGame = GameApp::instance();
  std::string v_nextLevel;
  std::string v_currentLevel;

  // take the level id of the first world
  if (m_universe != NULL) {
    if (m_universe->getScenes().size() > 0) {
      v_currentLevel = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  if (i_positifOrder) {
    v_nextLevel = pGame->determineNextLevel(v_currentLevel);
  } else {
    v_nextLevel = pGame->determinePreviousLevel(v_currentLevel);
  }

  /* update stats */
  if (v_nextLevel != "") {
    if (m_universe != NULL) {
      if (m_universe->getScenes().size() == 1) {
        if (m_universe->getScenes()[0]->Players().size() == 1) {
          if (m_universe->getScenes()[0]->Players()[0]->isDead() == false &&
              m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
            StateManager::instance()->getDbStatsThread()->delay_abortedLevel(
              XMSession::instance()->profile(),
              v_currentLevel,
              m_universe->getScenes()[0]->getTime() -
                m_universe->getScenes()[0]->getCheckpointStartTime());
            StateManager::instance()->getDbStatsThread()->doJob();
          }
        }
      }
    }
  }

  nextLevelToPlay(i_positifOrder);
}

void StateScene::restartLevelToPlay(bool i_reloadLevel) {
  std::string v_level;

  // take the level id of the first world
  if (m_universe != NULL) {
    if (m_universe->getScenes().size() > 0) {
      v_level = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  closePlaying();

  if (i_reloadLevel) {
    try {
      Level::removeFromCache(xmDatabase::instance("main"), v_level);
    } catch (Exception &e) {
      // hum, not nice
    }
  }

  StateManager::instance()->replaceState(new StatePreplayingGame(v_level, true),
                                         getStateId());
}

void StateScene::nextLevelToPlay(bool i_positifOrder) {
  GameApp *pGame = GameApp::instance();
  std::string v_nextLevel;
  std::string v_currentLevel;

  // take the level id of the first world
  if (m_universe != NULL) {
    if (m_universe->getScenes().size() > 0) {
      v_currentLevel = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  if (i_positifOrder) {
    v_nextLevel = pGame->determineNextLevel(v_currentLevel);
  } else {
    v_nextLevel = pGame->determinePreviousLevel(v_currentLevel);
  }

  if (v_nextLevel != "") {
    closePlaying();
    StateManager::instance()->replaceState(
      new StatePreplayingGame(v_nextLevel, v_currentLevel == v_nextLevel),
      getStateId());
  }
}

void StateScene::playLevelMusic() {
  if (m_universe != NULL) {
    if (m_universe->getScenes().size() > 0) {
      // play music of the first world
      if (m_universe->getScenes()[0]->getLevelSrc()->Music() == "" &&
          XMSession::instance()->musicOnAllLevels()) {
        GameApp::instance()->playGameMusic(Theme::instance()->getHashMusic(
          m_universe->getScenes()[0]->getLevelSrc()->Id()));
      } else {
        GameApp::instance()->playGameMusic(
          m_universe->getScenes()[0]->getLevelSrc()->Music());
      }
    }
  }
}

void StateScene::playToCheckpoint() {
  if (m_universe == NULL) {
    return;
  }

  if (m_cameraAnim) {
    m_cameraAnim->resetSizeMultipliers();
  }

  m_universe->deleteCurrentReplay(); // delete the replay when using checkpoints
  for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
    m_universe->getScenes()[j]->playToCheckpoint();
  }

  return;
}
