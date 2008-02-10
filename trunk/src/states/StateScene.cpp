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
#include "Game.h"
#include "PhysSettings.h"
#include "xmscene/Camera.h"
#include "XMSession.h"
#include "xmscene/Entity.h"
#include "StateMessageBox.h"
#include "drawlib/DrawLib.h"
#include "StatePreplaying.h"
#include "helpers/Log.h"
#include "CameraAnimation.h"
#include "Renderer.h"
#include "Universe.h"
#include "VideoRecorder.h"

#define INPLAY_ANIMATION_TIME 1.0
#define INPLAY_ANIMATION_SPEED 10
#define PRESTART_ANIMATION_MARGIN_SIZE 5

/* control the particle generation by ask the particle renders to limit themself if there are too much particles on the screen */
#define NB_PARTICLES_TO_RENDER_LIMITATION 130

StateScene::StateScene(bool i_doShade, bool i_doShadeAnim)
: GameState(false, false, i_doShade, i_doShadeAnim) {
    m_fLastPhysTime = -1.0;
    // while playing, we want 100 fps for the physic
    m_updateFps     = 100;
    m_showCursor = false;
    m_cameraAnim = NULL;
    m_universe   = NULL;

    m_benchmarkNbFrame   = 0;
    m_benchmarkStartTime = GameApp::getXMTime();
}

StateScene::StateScene(Universe* i_universe, bool i_doShade, bool i_doShadeAnim)
  : GameState(false, false, i_doShade, i_doShadeAnim)
{
  m_fLastPhysTime = -1.0;
  // while playing, we want 100 fps for the physic
  m_updateFps     = 100;
  m_showCursor = false;
  m_cameraAnim = NULL;
  m_universe   = i_universe;

  m_benchmarkNbFrame   = 0;
  m_benchmarkStartTime = GameApp::getXMTime();  
}

StateScene::~StateScene()
{
  if(m_cameraAnim != NULL) {
    delete m_cameraAnim;
  }
}

void StateScene::enter()
{
  GameState::enter();

  ParticlesSource::setAllowParticleGeneration(true);
  m_isLockedScene = false;
  m_autoZoom      = false;

  m_benchmarkNbFrame   = 0;
  m_benchmarkStartTime = GameApp::getXMTime();

  m_fLastPhysTime = GameApp::getXMTime();

}

bool StateScene::update()
{
  if(doUpdate() == false){
    return false;
  }
  
  InputHandler::instance()->updateUniverseInput(m_universe); // update input for the universe

  int nPhysSteps = 0;

  if(isLockedScene() == false) {  
    // don't update if that's not required
    // don't do this infinitely, maximum miss 10 frames, then give up
		// in videoRecording mode, don't try to do more to allow to record at a good framerate
    while ((m_fLastPhysTime + PHYS_STEP_SIZE <= GameApp::getXMTime()) && nPhysSteps < 10 && (XMSession::instance()->enableVideoRecording() == false || nPhysSteps == 0)) {
      if(m_universe != NULL) {
	for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	  m_universe->getScenes()[i]->updateLevel(PHYS_STEP_SIZE, m_universe->getCurrentReplay());
	}
      }
      m_fLastPhysTime += PHYS_STEP_SIZE;
      nPhysSteps++;
    }

    // if the delay is too long, reinitialize
    if(m_fLastPhysTime + PHYS_STEP_SIZE < GameApp::getXMTime()) {
      m_fLastPhysTime = GameApp::getXMTime();
    }

    // update camera scrolling
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->setScroll(true, m_universe->getScenes()[j]->getGravity());
	}
      }
    }
  }

  runAutoZoom();

  return true;
}

bool StateScene::render()
{
  GameApp*  pGame = GameApp::instance();

  if (XMSession::instance()->ugly()) {
    pGame->getDrawLib()->clearGraphics();
  }

  try {
    if(autoZoom() == false){
      if(m_universe != NULL) {
	for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	  for(unsigned int i=0; i<m_universe->getScenes()[j]->getNumberCameras(); i++) {
	    m_universe->getScenes()[j]->setCurrentCamera(i);
	    GameRenderer::instance()->render(m_universe->getScenes()[j]);
	  }
	}
      }
    } else {
      if(m_universe != NULL) {
	if(m_universe->getScenes().size() > 0) {
	  m_universe->getScenes()[0]->setAutoZoomCamera();
	  GameRenderer::instance()->render(m_universe->getScenes()[0]);
	}
      }
    }

    ParticlesSource::setAllowParticleGeneration(GameRenderer::instance()->nbParticlesRendered() < NB_PARTICLES_TO_RENDER_LIMITATION);
  } catch(Exception &e) {
    StateManager::instance()->replaceState(new StateMessageBox(NULL, GameApp::splitText(e.getMsg(), 50), UI_MSGBOX_OK));
  }

  GameState::render();
  m_benchmarkNbFrame++;

  return true;
}

void StateScene::onRenderFlush() {
  // take a screenshot
  if(XMSession::instance()->enableVideoRecording()) {
    if(StateManager::instance()->getVideoRecorder() != NULL) {
      if(m_universe->getScenes().size() > 0) {
	if( (XMSession::instance()->videoRecordingStartTime() < 0 ||
	     XMSession::instance()->videoRecordingStartTime() <= m_universe->getScenes()[0]->getTime() * 100
	     )
	    &&
	    (XMSession::instance()->videoRecordingEndTime() < 0 ||
	     XMSession::instance()->videoRecordingEndTime() >= m_universe->getScenes()[0]->getTime() * 100
	     )
	    ) {
	  StateManager::instance()->getVideoRecorder()->read(m_universe->getScenes()[0]->getTime());
	}
      }
    }
  }
}

void StateScene::keyDown(int nKey, SDLMod mod,int nChar)
{
  GameApp*  pGame = GameApp::instance();

  if(nKey == SDLK_F2){
    if(m_universe != NULL) {
      m_universe->switchFollowCamera();
    }
  }
  else if(nKey == SDLK_F3){
    if(m_universe != NULL) {
      if(m_universe->getScenes().size() > 0) { // just add the first world
	pGame->switchLevelToFavorite(m_universe->getScenes()[0]->getLevelSrc()->Id(), true);
	StateManager::instance()->sendAsynchronousMessage("FAVORITES_UPDATED");
      }
    }
  }
  else if(nKey == SDLK_b && (mod & KMOD_CTRL) != 0){
    if(m_universe != NULL) {
      if(m_universe->getScenes().size() > 0) { // just blacklist the first world
	pGame->switchLevelToBlacklist(m_universe->getScenes()[0]->getLevelSrc()->Id(), true);
	StateManager::instance()->sendAsynchronousMessage("BLACKLISTEDLEVELS_UPDATED");
      }
    }
  }
  else if(nKey == SDLK_PAGEUP){
    nextLevel();
  }
  else if(nKey == SDLK_PAGEDOWN){
    nextLevel(false);
  }
  else{
    GameState::keyDown(nKey, mod, nChar);
  }
}

void StateScene::send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  if(i_id == "ERROR") {
    m_commands.push("ERROR");
  }
}

void StateScene::send(const std::string& i_id, const std::string& i_message) {
  m_commands.push(i_message);
}

void StateScene::setScoresTimes() {
  char **v_result;
  unsigned int nrow;
  char *v_res;  
  std::string T1 = "--:--:--", T2 = "--:--:--";

  std::string v_id_level;
  // take the level id of the first world
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() > 0) {
      v_id_level = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  /* get best result */
  v_result = xmDatabase::instance("main")->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
						  "id_level=\"" + 
						  xmDatabase::protectString(v_id_level) + "\";",
						  nrow);
  v_res = xmDatabase::instance("main")->getResult(v_result, 1, 0, 0);
  if(v_res != NULL) {
    T1 = GameApp::formatTime(atof(v_res));
  }
  xmDatabase::instance("main")->read_DB_free(v_result);
    
  /* get best player result */
  v_result = xmDatabase::instance("main")->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
						  "id_level=\"" + 
						  xmDatabase::protectString(v_id_level) + "\" " + 
						  "AND id_profile=\"" + xmDatabase::protectString(XMSession::instance()->profile())  + "\";",
						  nrow);
  v_res = xmDatabase::instance("main")->getResult(v_result, 1, 0, 0);
  if(v_res != NULL) {
    T2 = GameApp::formatTime(atof(v_res));
  }
  xmDatabase::instance("main")->read_DB_free(v_result);
    
  if(XMSession::instance()->hidePlayingInformation() == false) {
    GameRenderer::instance()->setBestTime(T1 + std::string(" / ") + T2);
  } else {
    GameRenderer::instance()->setBestTime("");
  }

  if(XMSession::instance()->showHighscoreInGame() && XMSession::instance()->hidePlayingInformation() == false) {
    GameRenderer::instance()->setWorldRecordTime(GameApp::instance()->getWorldRecord(v_id_level));
  } else {
    GameRenderer::instance()->setWorldRecordTime("");
  }
}

void StateScene::restartLevel(bool i_reloadLevel) {
  std::string v_level;

  /* Update stats */   
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(m_universe->getScenes()[0]->Players().size() == 1) {
	if(m_universe->getScenes()[0]->Players()[0]->isDead() == false) {
	  xmDatabase::instance("main")->stats_levelRestarted(XMSession::instance()->profile(),
							     m_universe->getScenes()[0]->getLevelSrc()->Id(),
							     m_universe->getScenes()[0]->getTime());
	}
      }
    }
  }

  // take the level id of the first world
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() > 0) {
      v_level = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  closePlaying();

  GameRenderer::instance()->unprepareForNewLevel();
  
  if(i_reloadLevel) {
    try {
      Level::removeFromCache(xmDatabase::instance("main"), v_level);
    } catch(Exception &e) {
      // hum, not nice
    }
  }

  StateManager::instance()->replaceState(new StatePreplaying(v_level, true));
}

void StateScene::nextLevel(bool i_positifOrder) {
  GameApp*  pGame  = GameApp::instance();
  std::string v_currentLevel;

  // take the level id of the first world
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() > 0) {
      v_currentLevel = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  std::string v_nextLevel;

  if(i_positifOrder) {
    v_nextLevel = pGame->determineNextLevel(v_currentLevel);
  } else {
    v_nextLevel = pGame->determinePreviousLevel(v_currentLevel);
  }

  if(v_nextLevel != "") {
    if(m_universe != NULL) {
      if(m_universe->getScenes().size() == 1) {
	if(m_universe->getScenes()[0]->Players().size() == 1) {
	  xmDatabase::instance("main")->stats_abortedLevel(XMSession::instance()->profile(),
							   v_currentLevel,
							   m_universe->getScenes()[0]->getTime());
	}
      }
    }

    closePlaying();
    StateManager::instance()->replaceState(new StatePreplaying(v_nextLevel, v_currentLevel == v_nextLevel));
  }
}

void StateScene::abortPlaying() {
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(m_universe->getScenes()[0]->Players().size() == 1) {
	xmDatabase::instance("main")->stats_abortedLevel(XMSession::instance()->profile(),
							 m_universe->getScenes()[0]->getLevelSrc()->Id(),
							 m_universe->getScenes()[0]->getTime());
      }
    }
  }
  
  closePlaying();
}

void StateScene::closePlaying() {
  if(m_universe != NULL) {
    delete m_universe;
    m_universe = NULL;
  }

  InputHandler::instance()->resetScriptKeyHooks();                     
  GameRenderer::instance()->unprepareForNewLevel();
}

bool StateScene::isLockedScene() const {
  return m_isLockedScene;
}

void StateScene::lockScene(bool i_value) {
  m_isLockedScene = i_value;
  if(m_isLockedScene == false){
    m_fLastPhysTime = GameApp::getXMTime();
  }
}

void StateScene::setAutoZoom(bool i_value) {
  if(m_autoZoom == false && i_value == true) {
    lockScene(true);

    if(m_cameraAnim != NULL) {
      delete m_cameraAnim;
    }
    GameApp*  pGame = GameApp::instance();

    if(m_universe != NULL) {
      if(m_universe->getScenes().size() > 0) { // do only for the first world for the moment
	m_universe->getScenes()[0]->setAutoZoomCamera();
	m_cameraAnim = new AutoZoomCameraAnimation(m_universe->getScenes()[0]->getCamera(),
						   pGame->getDrawLib(),
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
  if(autoZoom()) {
    if(m_cameraAnim != NULL) {
      if(m_cameraAnim->step() == false) {
	lockScene(false);
	m_cameraAnim->uninit();
	setAutoZoom(false);
      }
    }
  }
}

void StateScene::executeOneCommand(std::string cmd)
{
  Logger::Log("StateScene::executeOneCommand::%s", cmd.c_str());

  if(cmd == "ERROR") {
    closePlaying();
    m_requestForEnd = true;
    return;
  }

  if(cmd == "FINISH") {
    closePlaying();
    m_requestForEnd = true;
    return;
  }

  if(cmd == "RESTART") {
    restartLevel();
    return;
  }

  if(cmd == "NEXTLEVEL") {
    nextLevel();
    return;
  }

  if(cmd == "PREVIOUSLEVEL") {
    nextLevel(false);
    return;
  }

  if(cmd == "ABORT") {
    abortPlaying();
    m_requestForEnd = true;
    return;
  }

  if(cmd == "INTERPOLATION_CHANGED") {
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Players().size(); i++) {
	  m_universe->getScenes()[j]->Players()[i]->setInterpolation(XMSession::instance()->enableReplayInterpolation());
	}
      }
    }
  }

  if(cmd == "MIRRORMODE_CHANGED") {
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->setMirrored(XMSession::instance()->mirrorMode());
	}
      }
    }
  }

}
