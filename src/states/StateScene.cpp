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

#define INPLAY_ANIMATION_TIME 1.0
#define INPLAY_ANIMATION_SPEED 10
#define PRESTART_ANIMATION_MARGIN_SIZE 5

/* control the particle generation by ask the particle renders to limit themself if there are too much particles on the screen */
#define NB_PARTICLES_TO_RENDER_LIMITATION 130


StateScene::StateScene(GameApp* pGame, bool i_doShade, bool i_doShadeAnim):
GameState(false, false, pGame, i_doShade, i_doShadeAnim)
{
  m_fLastPhysTime = -1.0;
  // while playing, we want 100 fps for the physic
  m_updateFps     = 100;
  m_showCursor = false;
  m_cameraAnim = NULL;
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
}

void StateScene::leave()
{

}

void StateScene::enterAfterPop()
{
}

void StateScene::leaveAfterPush()
{
}

bool StateScene::update()
{
  int nPhysSteps = 0;

  if(isLockedScene() == false) {  
    if(m_fLastPhysTime < 0.0) {
      m_fLastPhysTime = GameApp::getXMTime();
    }

    // don't update if that's not required
    // don't do this infinitely, maximum miss 10 frames, then give up
    while ((m_fLastPhysTime + PHYS_STEP_SIZE <= GameApp::getXMTime()) && (nPhysSteps < 10)) {
      m_pGame->getMotoGame()->updateLevel(PHYS_STEP_SIZE, m_pGame->getCurrentReplay());
      m_fLastPhysTime += PHYS_STEP_SIZE;
      nPhysSteps++;    
    }
  }

  runAutoZoom();

  return true;
}

bool StateScene::render()
{
  if (XMSession::instance()->ugly()) {
    m_pGame->getDrawLib()->clearGraphics();
  }

  try {
    if(autoZoom() == false){
      for(unsigned int i=0; i<m_pGame->getMotoGame()->getNumberCameras(); i++) {
	m_pGame->getMotoGame()->setCurrentCamera(i);
	GameRenderer::instance()->render();
      }
    } else {
      m_pGame->getMotoGame()->setAutoZoomCamera();
      GameRenderer::instance()->render();
    }

    ParticlesSource::setAllowParticleGeneration(GameRenderer::instance()->nbParticlesRendered() < NB_PARTICLES_TO_RENDER_LIMITATION);
  } catch(Exception &e) {
    StateManager::instance()->replaceState(new StateMessageBox(NULL, m_pGame, GameApp::splitText(e.getMsg(), 50), UI_MSGBOX_OK));
  }

  GameState::render();

  return true;
}

void StateScene::keyDown(int nKey, SDLMod mod,int nChar)
{
  if(nKey == SDLK_F2){
    m_pGame->switchFollowCamera();
  }
  else if(nKey == SDLK_F3){
    m_pGame->switchLevelToFavorite(m_pGame->getMotoGame()->getLevelSrc()->Id(), true);
    StateManager::instance()->sendAsynchronousMessage("FAVORITES_UPDATED");
  }
  else if(nKey == SDLK_b && (mod & KMOD_CTRL) != 0){
    m_pGame->switchLevelToBlacklist(m_pGame->getMotoGame()->getLevelSrc()->Id(), true);
    StateManager::instance()->sendAsynchronousMessage("BLACKLISTEDLEVELS_UPDATED");
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

void StateScene::keyUp(int nKey, SDLMod mod)
{
}

void StateScene::mouseDown(int nButton)
{
}

void StateScene::mouseDoubleClick(int nButton)
{
}

void StateScene::mouseUp(int nButton)
{
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

    /* get best result */
    v_result = m_pGame->getDb()->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
					"id_level=\"" + 
					xmDatabase::protectString(m_pGame->getMotoGame()->getLevelSrc()->Id()) + "\";",
					nrow);
    v_res = m_pGame->getDb()->getResult(v_result, 1, 0, 0);
    if(v_res != NULL) {
      T1 = GameApp::formatTime(atof(v_res));
    }
    m_pGame->getDb()->read_DB_free(v_result);
    
    /* get best player result */
    v_result = m_pGame->getDb()->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
					"id_level=\"" + 
					xmDatabase::protectString(m_pGame->getMotoGame()->getLevelSrc()->Id()) + "\" " + 
					"AND id_profile=\"" + xmDatabase::protectString(XMSession::instance()->profile())  + "\";",
					nrow);
    v_res = m_pGame->getDb()->getResult(v_result, 1, 0, 0);
    if(v_res != NULL) {
      T2 = GameApp::formatTime(atof(v_res));
    }
    m_pGame->getDb()->read_DB_free(v_result);
    
    GameRenderer::instance()->setBestTime(T1 + std::string(" / ") + T2);

    if(XMSession::instance()->showHighscoreInGame()) {
      GameRenderer::instance()->setWorldRecordTime(m_pGame->getWorldRecord(m_pGame->getMotoGame()->getLevelSrc()->Id()));
    } else {
      GameRenderer::instance()->setWorldRecordTime("");
    }

}

void StateScene::restartLevel(bool i_reloadLevel) {
  std::string v_level;

  /* Update stats */        
  if(m_pGame->getMotoGame()->Players().size() == 1) {
    if(m_pGame->getMotoGame()->Players()[0]->isDead() == false) {
      m_pGame->getDb()->stats_levelRestarted(XMSession::instance()->profile(),
				 m_pGame->getMotoGame()->getLevelSrc()->Id(),
				 m_pGame->getMotoGame()->getTime());
    }
  }  

  v_level = m_pGame->getMotoGame()->getLevelSrc()->Id();
  m_pGame->getMotoGame()->resetFollow();
  m_pGame->getMotoGame()->endLevel();
  GameRenderer::instance()->unprepareForNewLevel();
  
  if(i_reloadLevel) {
    try {
      Level::removeFromCache(m_pGame->getDb(), v_level);
    } catch(Exception &e) {
      // hum, not nice
    }
  }

  StateManager::instance()->replaceState(new StatePreplaying(m_pGame, v_level, true));
}

void StateScene::nextLevel(bool i_positifOrder) {
  std::string v_currentLevel = m_pGame->getMotoGame()->getLevelSrc()->Id();
  std::string v_nextLevel;

  if(i_positifOrder) {
    v_nextLevel = m_pGame->determineNextLevel(v_currentLevel);
  } else {
    v_nextLevel = m_pGame->determinePreviousLevel(v_currentLevel);
  }

  if(v_nextLevel != "") {
    if(m_pGame->getMotoGame()->Players().size() == 1) {
      m_pGame->getDb()->stats_abortedLevel(XMSession::instance()->profile(),
					   v_currentLevel,
					   m_pGame->getMotoGame()->getTime());
    }

    closePlaying();
    StateManager::instance()->replaceState(new StatePreplaying(m_pGame, v_nextLevel, v_currentLevel == v_nextLevel));
  }
}


void StateScene::abortPlaying() {
  if(m_pGame->getMotoGame()->Players().size() == 1) {
    m_pGame->getDb()->stats_abortedLevel(XMSession::instance()->profile(),
					 m_pGame->getMotoGame()->getLevelSrc()->Id(),
					 m_pGame->getMotoGame()->getTime());
  }
  
  closePlaying();
}

void StateScene::closePlaying() {
  m_pGame->getMotoGame()->resetFollow();
  m_pGame->getMotoGame()->endLevel();
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
    m_cameraAnim = new AutoZoomCameraAnimation(m_pGame->getMotoGame()->getCamera(), m_pGame->getDrawLib(), m_pGame->getMotoGame());
    m_cameraAnim->init();
  }

  m_autoZoom = i_value;
}

bool StateScene::autoZoom() const {
  return m_autoZoom;
}

void StateScene::runAutoZoom() {
  if(autoZoom()) {
    if(m_cameraAnim->step() == false) {
      lockScene(false);
      m_cameraAnim->uninit();
      setAutoZoom(false);
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
}
