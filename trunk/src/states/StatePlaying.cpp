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

#include "StatePlaying.h"
#include "StatePause.h"
#include "Game.h"
#include "XMSession.h"
#include "helpers/Log.h"
#include "GameText.h"
#include "StateMessageBox.h"
#include "StateFinished.h"
#include "StateDeadJust.h"
#include "StateDeadMenu.h"
#include "Sound.h"
#include "xmscene/Camera.h"
#include "xmscene/Bike.h"
#include "CameraAnimation.h"

StatePlaying::StatePlaying():
  StateScene()
{
  m_name  = "StatePlaying";
}

StatePlaying::~StatePlaying()
{
}

void StatePlaying::enter()
{
  StateScene::enter();

  GameRenderer::instance()->setShowEngineCounter(XMSession::instance()->showEngineCounter());
  GameRenderer::instance()->setShowMinimap(XMSession::instance()->showMinimap());
  GameRenderer::instance()->setShowTimePanel(true);

  std::string v_level_name;
  try {
    for(unsigned int i=0; i<GameApp::instance()->getScenes().size(); i++) {
      v_level_name = GameApp::instance()->getScenes()[i]->getLevelSrc()->Name();
      GameApp::instance()->getScenes()[i]->playLevel();
    }
  }
  catch(Exception &e) {
    Logger::Log("** Warning ** : level '%s' cannot be loaded", v_level_name.c_str());

    char cBuf[256];
    sprintf(cBuf,GAMETEXT_LEVELCANNOTBELOADED, v_level_name.c_str());

    StateMessageBox* v_msgboxState = new StateMessageBox(this, cBuf, UI_MSGBOX_OK);
    v_msgboxState->setId("ERROR");
    StateManager::instance()->pushState(v_msgboxState);
  }

  setScoresTimes();

  if(GameApp::instance()->getScenes().size() > 0) {
    // play music of the first world
    GameApp::instance()->playMusic(GameApp::instance()->getScenes()[0]->getLevelSrc()->Music());
  }
}

void StatePlaying::leave()
{
  for(unsigned int i=0; i<GameApp::instance()->getScenes().size(); i++) {
    GameApp::instance()->getScenes()[i]->setInfos("");
  }
}

void StatePlaying::enterAfterPop()
{
  m_fLastPhysTime = GameApp::getXMTime();
}

bool StatePlaying::update()
{
  if(StateScene::update() == false)
    return false;

  GameApp*  pGame = GameApp::instance();

  if(isLockedScene() == false) {
    bool v_all_dead       = true;
    bool v_one_still_play = false;
    bool v_one_finished   = false;
    
    for(unsigned int j=0; j<GameApp::instance()->getScenes().size(); j++) {
      for(unsigned int i=0; i<GameApp::instance()->getScenes()[j]->Players().size(); i++) {
	if(GameApp::instance()->getScenes()[j]->Players()[i]->isDead() == false) {
	  v_all_dead = false;
	}
	if(GameApp::instance()->getScenes()[j]->Players()[i]->isFinished()) {
	  v_one_finished = true;
	}
	
	if(GameApp::instance()->getScenes()[j]->Players()[i]->isFinished() == false &&
	   GameApp::instance()->getScenes()[j]->Players()[i]->isDead() == false) {
	  v_one_still_play = true;
	}
      }
    }
    
    if(v_one_still_play == false || XMSession::instance()->MultiStopWhenOneFinishes()) { // let people continuing when one finished or not
      if(v_one_finished) {
	/* You're done maaaan! :D */

	/* finalize the replay */
	if(pGame->isAReplayToSave()) {
	  pGame->finalizeReplay(true);
	}

	/* update profiles */
	if(GameApp::instance()->getScenes().size() == 1) {
	  if(GameApp::instance()->getScenes()[0]->Players().size() == 1) {
	    float v_finish_time = 0.0;
	    std::string TimeStamp = pGame->getTimeStamp();
	    if(GameApp::instance()->getScenes()[0]->Players()[0]->isFinished()) {
	      v_finish_time  = GameApp::instance()->getScenes()[0]->Players()[0]->finishTime();
	    }
	    xmDatabase::instance("main")->profiles_addFinishTime(XMSession::instance()->profile(),
								 GameApp::instance()->getScenes()[0]->getLevelSrc()->Id(),
								 TimeStamp,
								 v_finish_time);
	  }
	}

	/* Update stats */
	/* update stats only in one player mode */
	if(GameApp::instance()->getScenes().size() == 1) {
	  if(GameApp::instance()->getScenes()[0]->Players().size() == 1) {
	    xmDatabase::instance("main")->stats_levelCompleted(XMSession::instance()->profile(),
							       GameApp::instance()->getScenes()[0]->getLevelSrc()->Id(),
							       GameApp::instance()->getScenes()[0]->Players()[0]->finishTime());
	    StateManager::instance()->sendAsynchronousMessage("LEVELS_UPDATED");
	  }
	}
	StateManager::instance()->pushState(new StateFinished(this));
      } else if(v_all_dead) {
	/* You're dead maan! */
	if(pGame->isAReplayToSave()) {
	  pGame->finalizeReplay(false);
	}

	/* Update stats */
	if(GameApp::instance()->getScenes().size() == 1) {
	  if(GameApp::instance()->getScenes()[0]->Players().size() == 1) {
	    xmDatabase::instance("main")->stats_died(XMSession::instance()->profile(),
						     GameApp::instance()->getScenes()[0]->getLevelSrc()->Id(),
						     GameApp::instance()->getScenes()[0]->getTime());
	  }
	}

	/* Play the DIE!!! sound */
	try {
	  Sound::playSampleByName(Theme::instance()->getSound("Headcrash")->FilePath(), 0.3);
	} catch(Exception &e) {
	}

	if(XMSession::instance()->enableDeadAnimation()) {
	  StateManager::instance()->replaceState(new StateDeadJust());
	} else {
	  StateManager::instance()->pushState(new StateDeadMenu(true, this));
	}
      }
    }
  }

  return true;
}

void StatePlaying::keyDown(int nKey, SDLMod mod,int nChar)
{
  if(nKey == SDLK_ESCAPE){
    if(isLockedScene() == false) {
      /* Escape pauses */
      StateManager::instance()->pushState(new StatePause(this));
    }
  }
  else if(nKey == SDLK_RETURN && (mod & (KMOD_CTRL|KMOD_SHIFT|KMOD_ALT|KMOD_META)) == 0){
    restartLevel();
  }
  else if(nKey == SDLK_TAB){
    if(autoZoom() == false) {
      setAutoZoom(true);
    }
  }

#if defined(ENABLE_ZOOMING)
  else if(nKey == SDLK_KP7){
    /* Zoom in */
    for(unsigned int j=0; j<GameApp::instance()->getScenes().size(); j++) {
      for(unsigned int i=0; i<GameApp::instance()->getScenes()[j]->Cameras().size(); i++) {
	GameApp::instance()->getScenes()[j]->Cameras()[i]->zoom(0.002);
      }
    }
  }
  else if(nKey == SDLK_KP9){
    /* Zoom out */
    for(unsigned int j=0; j<GameApp::instance()->getScenes().size(); j++) {
      for(unsigned int i=0; i<GameApp::instance()->getScenes()[j]->Cameras().size(); i++) {
	GameApp::instance()->getScenes()[j]->Cameras()[i]->zoom(-0.002);
      }
    }
  }
  else if(nKey == SDLK_HOME){
    for(unsigned int j=0; j<GameApp::instance()->getScenes().size(); j++) {
      for(unsigned int i=0; i<GameApp::instance()->getScenes()[j]->Cameras().size(); i++) {
	GameApp::instance()->getScenes()[j]->Cameras()[i]->initCamera();
      }
    }
  }
  else if(nKey == SDLK_KP6){
    for(unsigned int j=0; j<GameApp::instance()->getScenes().size(); j++) {
      for(unsigned int i=0; i<GameApp::instance()->getScenes()[j]->Cameras().size(); i++) {
	GameApp::instance()->getScenes()[j]->Cameras()[i]->moveCamera(1.0, 0.0);
      }
    }
  }
  else if(nKey == SDLK_KP4){
    for(unsigned int j=0; j<GameApp::instance()->getScenes().size(); j++) {
      for(unsigned int i=0; i<GameApp::instance()->getScenes()[j]->Cameras().size(); i++) {
	GameApp::instance()->getScenes()[j]->Cameras()[i]->moveCamera(-1.0, 0.0);
      }
    }
  }
  else if(nKey == SDLK_KP8){
    for(unsigned int j=0; j<GameApp::instance()->getScenes().size(); j++) {
      for(unsigned int i=0; i<GameApp::instance()->getScenes()[j]->Cameras().size(); i++) {
	GameApp::instance()->getScenes()[j]->Cameras()[i]->moveCamera(0.0, 1.0);
      }
    }
  }
  else if(nKey == SDLK_KP2){
    for(unsigned int j=0; j<GameApp::instance()->getScenes().size(); j++) {
      for(unsigned int i=0; i<GameApp::instance()->getScenes()[j]->Cameras().size(); i++) {
	GameApp::instance()->getScenes()[j]->Cameras()[i]->moveCamera(0.0, -1.0);
      }
    }
  }
  else if(nKey == SDLK_KP0 && (mod & KMOD_LCTRL) == KMOD_LCTRL){
    for(unsigned int j=0; j<GameApp::instance()->getScenes().size(); j++) {
      for(unsigned int i=0; i<GameApp::instance()->getScenes()[j]->Players().size(); i++) {
	if(GameApp::instance()->getScenes()[j]->Cameras().size() > 0) {
	  GameApp::instance()->TeleportationCheatTo(i, Vector2f(GameApp::instance()->getScenes()[j]->Cameras()[0]->getCameraPositionX(),
								GameApp::instance()->getScenes()[j]->Cameras()[0]->getCameraPositionY()));
	}
      }
    }
  }
#endif

  else {
    // to avoid people changing direction during the autozoom
    if(m_autoZoom == false){
      /* Notify the controller */
      InputHandler::instance()->handleInput(INPUT_KEY_DOWN, nKey, mod);
    }
  }

  StateScene::keyDown(nKey, mod, nChar);
}

void StatePlaying::keyUp(int nKey, SDLMod mod)
{
  switch(nKey) {

  case SDLK_TAB:
    if(m_cameraAnim != NULL) {
      if(autoZoom() && m_cameraAnim->allowNextStep()) {
	m_cameraAnim->goNextStep();
      }
    }
    break;
    
  default:
    InputHandler::instance()->handleInput(INPUT_KEY_UP, nKey, mod);
    StateScene::keyUp(nKey, mod);
  }
}

void StatePlaying::mouseDown(int nButton)
{
  InputHandler::instance()->handleInput(INPUT_KEY_DOWN, nButton, KMOD_NONE);

  StateScene::mouseDown(nButton);
}

void StatePlaying::mouseUp(int nButton)
{
  InputHandler::instance()->handleInput(INPUT_KEY_UP, nButton, KMOD_NONE);

  StateScene::mouseUp(nButton);
}

void StatePlaying::send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  if(i_id == "ERROR") {
    m_commands.push("ERROR");
  }
}
