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
}

StateScene::~StateScene()
{
}


void StateScene::enter()
{
  GameState::enter();

  ParticlesSource::setAllowParticleGeneration(true);
  m_isLockedScene = false;
  m_autoZoom      = false;
  m_autoZoomStep  = 0;
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
  try {
    if(autoZoom() == false){
      for(unsigned int i=0; i<m_pGame->getMotoGame()->getNumberCameras(); i++) {
	m_pGame->getMotoGame()->setCurrentCamera(i);
	m_pGame->getGameRenderer()->render();
      }
    } else {
      m_pGame->getMotoGame()->setAutoZoomCamera();
      m_pGame->getGameRenderer()->render();
    }

    ParticlesSource::setAllowParticleGeneration(m_pGame->getGameRenderer()->nbParticlesRendered() < NB_PARTICLES_TO_RENDER_LIMITATION);
  } catch(Exception &e) {
    m_pGame->getStateManager()->replaceState(new StateMessageBox(NULL, m_pGame, GameApp::splitText(e.getMsg(), 50), UI_MSGBOX_OK));
  }

  GameState::render();

  return true;
}

void StateScene::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {

  case SDLK_F2:
    m_pGame->switchFollowCamera();
    break;
    
  case SDLK_F3:
    m_pGame->switchLevelToFavorite(m_pGame->getMotoGame()->getLevelSrc()->Id(), true);
    break;
    
  case SDLK_PAGEUP:
    nextLevel();
    break;

  case SDLK_PAGEDOWN:
    nextLevel(false);
    break;

  default:
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
					"AND id_profile=\"" + xmDatabase::protectString(m_pGame->getSession()->profile())  + "\";",
					nrow);
    v_res = m_pGame->getDb()->getResult(v_result, 1, 0, 0);
    if(v_res != NULL) {
      T2 = GameApp::formatTime(atof(v_res));
    }
    m_pGame->getDb()->read_DB_free(v_result);
    
    m_pGame->getGameRenderer()->setBestTime(T1 + std::string(" / ") + T2);
    m_pGame->getGameRenderer()->setWorldRecordTime(m_pGame->getWorldRecord(m_pGame->getMotoGame()->getLevelSrc()->Id()));

}

void StateScene::restartLevel(bool i_reloadLevel) {
  std::string v_level;

  /* Update stats */        
  if(m_pGame->getMotoGame()->Players().size() == 1) {
    if(m_pGame->getMotoGame()->Players()[0]->isDead() == false) {
      m_pGame->getDb()->stats_levelRestarted(m_pGame->getSession()->profile(),
				 m_pGame->getMotoGame()->getLevelSrc()->Id(),
				 m_pGame->getMotoGame()->getTime());
    }
  }  

  v_level = m_pGame->getMotoGame()->getLevelSrc()->Id();
  m_pGame->getMotoGame()->resetFollow();
  m_pGame->getMotoGame()->endLevel();
  m_pGame->getGameRenderer()->unprepareForNewLevel();
  
  if(i_reloadLevel) {
    try {
      Level::removeFromCache(m_pGame->getDb(), v_level);
    } catch(Exception &e) {
      // hum, not nice
    }
  }

  StatePreplaying::setPlayAnimation(false);
  m_pGame->getStateManager()->replaceState(new StatePreplaying(m_pGame, v_level));
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
      m_pGame->getDb()->stats_abortedLevel(m_pGame->getSession()->profile(),
					   v_currentLevel,
					   m_pGame->getMotoGame()->getTime());
    }

    closePlaying();
    StatePreplaying::setPlayAnimation(true);
    m_pGame->getStateManager()->replaceState(new StatePreplaying(m_pGame, v_nextLevel));
  }
}


void StateScene::abortPlaying() {
  if(m_pGame->getMotoGame()->Players().size() == 1) {
    m_pGame->getDb()->stats_abortedLevel(m_pGame->getSession()->profile(),
					 m_pGame->getMotoGame()->getLevelSrc()->Id(),
					 m_pGame->getMotoGame()->getTime());
  }
  
  closePlaying();
}

void StateScene::closePlaying() {
  m_pGame->getMotoGame()->resetFollow();
  m_pGame->getMotoGame()->endLevel();
  m_pGame->getInputHandler()->resetScriptKeyHooks();                     
  m_pGame->getGameRenderer()->unprepareForNewLevel();
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
    zoomAnimation2_init();
  }

  m_autoZoom = i_value;
}

bool StateScene::autoZoom() const {
  return m_autoZoom;
}

int StateScene::autoZoomStep() const {
  return m_autoZoomStep;
}

void StateScene::setAutoZoomStep(int n) {
  m_autoZoomStep = n;
}

void StateScene::runAutoZoom() {
  if(autoZoom()) {
    if(zoomAnimation2_step() == false) {
      lockScene(false);
      setAutoZoom(false);
    }
  }
}

// this function is duplicated with StatePreplaying => classes for animation if required
void StateScene::zoomAnimation1_init() {
  DrawLib* drawLib = m_pGame->getDrawLib();

  m_fPrePlayStartTime = GameApp::getXMTime();
  m_fPrePlayStartInitZoom = m_pGame->getMotoGame()->getCamera()->getCurrentZoom();  // because the man can change ugly mode while the animation
  m_fPrePlayStartCameraX  = m_pGame->getMotoGame()->getCamera()->getCameraPositionX();
  m_fPrePlayStartCameraY  = m_pGame->getMotoGame()->getCamera()->getCameraPositionY();
  
  m_zoomX = (2.0 * ((float)drawLib->getDispWidth() / (float)drawLib->getDispHeight())) / (m_pGame->getMotoGame()->getLevelSrc()->RightLimit() - m_pGame->getMotoGame()->getLevelSrc()->LeftLimit() + 2*PRESTART_ANIMATION_MARGIN_SIZE);
  m_zoomY = 2.0 /(m_pGame->getMotoGame()->getLevelSrc()->TopLimit() - m_pGame->getMotoGame()->getLevelSrc()->BottomLimit()+2*PRESTART_ANIMATION_MARGIN_SIZE);
  
  if (m_zoomX > m_zoomY){
    float visibleHeight,cameraStartHeight;
    
    m_zoomU=m_zoomX;
    static_time = (m_pGame->getMotoGame()->getLevelSrc()->TopLimit() - m_pGame->getMotoGame()->getLevelSrc()->BottomLimit()) / (2.0/m_zoomU);
    
    visibleHeight = 2.0/m_zoomU;
    cameraStartHeight= visibleHeight/2.0;
    
    m_fPreCameraStartX = (m_pGame->getMotoGame()->getLevelSrc()->RightLimit() + m_pGame->getMotoGame()->getLevelSrc()->LeftLimit())/2;
    m_fPreCameraStartY = m_pGame->getMotoGame()->getLevelSrc()->TopLimit() - cameraStartHeight + PRESTART_ANIMATION_MARGIN_SIZE;
    m_fPreCameraFinalX = (m_pGame->getMotoGame()->getLevelSrc()->RightLimit() + m_pGame->getMotoGame()->getLevelSrc()->LeftLimit())/2;
    m_fPreCameraFinalY = m_pGame->getMotoGame()->getLevelSrc()->BottomLimit() + cameraStartHeight - PRESTART_ANIMATION_MARGIN_SIZE;
    
    if ( fabs(m_fPreCameraStartY - m_fPrePlayStartCameraY) > fabs(m_fPreCameraFinalY - m_fPrePlayStartCameraY)) {
      float f;
      f = m_fPreCameraFinalY;
      m_fPreCameraFinalY = m_fPreCameraStartY;
      m_fPreCameraStartY = f;
    }
    
  } else {
    float visibleWidth,cameraStartLeft;
    
    m_zoomU=m_zoomY;
    static_time = (m_pGame->getMotoGame()->getLevelSrc()->RightLimit() - m_pGame->getMotoGame()->getLevelSrc()->LeftLimit()) / ((2.0 * ((float)drawLib->getDispWidth() / (float)drawLib->getDispHeight()))/m_zoomU);
    
    visibleWidth = (2.0 * ((float)drawLib->getDispWidth() / (float)drawLib->getDispHeight()))/m_zoomU;
    cameraStartLeft = visibleWidth/2.0;
    
    m_fPreCameraStartX = m_pGame->getMotoGame()->getLevelSrc()->RightLimit() - cameraStartLeft + PRESTART_ANIMATION_MARGIN_SIZE;
    m_fPreCameraStartY = (m_pGame->getMotoGame()->getLevelSrc()->BottomLimit() + m_pGame->getMotoGame()->getLevelSrc()->TopLimit())/2;
    m_fPreCameraFinalX = m_pGame->getMotoGame()->getLevelSrc()->LeftLimit() + cameraStartLeft - PRESTART_ANIMATION_MARGIN_SIZE;
    m_fPreCameraFinalY = (m_pGame->getMotoGame()->getLevelSrc()->BottomLimit() + m_pGame->getMotoGame()->getLevelSrc()->TopLimit())/2;
   
    if ( fabs(m_fPreCameraStartX - m_fPrePlayStartCameraX) > fabs(m_fPreCameraFinalX - m_fPrePlayStartCameraX)) {
      float f;
	f = m_fPreCameraFinalX;
	m_fPreCameraFinalX = m_fPreCameraStartX;
	m_fPreCameraStartX = f;
    }
  } 
}

void StateScene::zoomAnimation2_init() {
  zoomAnimation1_init();
  fAnimPlayStartZoom = m_pGame->getMotoGame()->getCamera()->getCurrentZoom(); 
  fAnimPlayStartCameraX = m_pGame->getMotoGame()->getCamera()->getCameraPositionX();
  fAnimPlayStartCameraY = m_pGame->getMotoGame()->getCamera()->getCameraPositionY();
  fAnimPlayFinalZoom = m_zoomU;
  fAnimPlayFinalCameraX1 = m_fPreCameraStartX;
  fAnimPlayFinalCameraY1 = m_fPreCameraStartY;
  fAnimPlayFinalCameraX2 = m_fPreCameraFinalX;
  fAnimPlayFinalCameraY2 = m_fPreCameraFinalY;
  m_fPrePlayStartTime = GameApp::getXMTime();
  
  m_autoZoomStep = 1;
}

bool StateScene::zoomAnimation2_step() {
  switch(m_autoZoomStep) {
    
  case 1:
    if(GameApp::getXMTime() > m_fPrePlayStartTime + INPLAY_ANIMATION_TIME) {
      float zx, zy;
      zx = (fAnimPlayFinalCameraX1 - fAnimPlayFinalCameraX2) * (sin((GameApp::getXMTime() - m_fPrePlayStartTime - INPLAY_ANIMATION_TIME) * 2 * 3.1415927 / INPLAY_ANIMATION_SPEED - 3.1415927/2) + 1) / 2;
      zy = (fAnimPlayFinalCameraY1 - fAnimPlayFinalCameraY2) * (sin((GameApp::getXMTime() - m_fPrePlayStartTime - INPLAY_ANIMATION_TIME) * 2 * 3.1415927 / INPLAY_ANIMATION_SPEED - 3.1415927/2) + 1) / 2;
      m_pGame->getMotoGame()->getCamera()->setCameraPosition(fAnimPlayFinalCameraX1 - zx,fAnimPlayFinalCameraY1 - zy);
      return true;
    }
    if(GameApp::getXMTime() > m_fPrePlayStartTime){
      float zx, zy, zz, coeff;
      coeff = (GameApp::getXMTime() - m_fPrePlayStartTime) / (INPLAY_ANIMATION_TIME);
      zx = coeff * (fAnimPlayStartCameraX - fAnimPlayFinalCameraX1);
      zy = coeff * (fAnimPlayStartCameraY - fAnimPlayFinalCameraY1);
      zz = coeff * (fAnimPlayStartZoom - fAnimPlayFinalZoom);
      
      m_pGame->getMotoGame()->getCamera()->setZoom(fAnimPlayStartZoom - zz);
      m_pGame->getMotoGame()->getCamera()->setCameraPosition(fAnimPlayStartCameraX - zx,fAnimPlayStartCameraY - zy);
    }
    
    return true;
    break;
    
  case 2:
    zoomAnimation2_init_unzoom();
    m_autoZoomStep = 3;
    break;
    
  case 3:
    return zoomAnimation2_unstep();
    break;
  }
  
  return true;
}

void StateScene::zoomAnimation2_init_unzoom() {
  m_fPrePlayStartTime = GameApp::getXMTime();
  fAnimPlayFinalZoom = fAnimPlayStartZoom;
  fAnimPlayStartZoom = m_pGame->getMotoGame()->getCamera()->getCurrentZoom();
  fAnimPlayFinalCameraX1 = fAnimPlayStartCameraX;
  fAnimPlayFinalCameraY1 = fAnimPlayStartCameraY;
  fAnimPlayStartCameraX = m_pGame->getMotoGame()->getCamera()->getCameraPositionX();
  fAnimPlayStartCameraY = m_pGame->getMotoGame()->getCamera()->getCameraPositionY();
}

bool StateScene::zoomAnimation2_unstep() {
  if(GameApp::getXMTime() > m_fPrePlayStartTime + INPLAY_ANIMATION_TIME) {
    return false;
  }
  if(GameApp::getXMTime() > m_fPrePlayStartTime){
    float zx, zy, zz, coeff;
    coeff = (GameApp::getXMTime() - m_fPrePlayStartTime) / (INPLAY_ANIMATION_TIME);
    zx = coeff * (fAnimPlayStartCameraX - fAnimPlayFinalCameraX1);
    zy = coeff * (fAnimPlayStartCameraY - fAnimPlayFinalCameraY1);
    zz = coeff * (fAnimPlayStartZoom - fAnimPlayFinalZoom);
    
    m_pGame->getMotoGame()->getCamera()->setZoom(fAnimPlayStartZoom - zz);
    m_pGame->getMotoGame()->getCamera()->setCameraPosition(fAnimPlayStartCameraX - zx,fAnimPlayStartCameraY - zy);
    return true;
  }
  return false;
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
