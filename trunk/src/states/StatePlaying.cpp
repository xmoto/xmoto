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

StatePlaying::StatePlaying(GameApp* pGame):
  StateScene(pGame)
{
  m_name  = "StatePlaying";
}

StatePlaying::~StatePlaying()
{

}


void StatePlaying::enter()
{
  StateScene::enter();

  m_pGame->getGameRenderer()->setShowEngineCounter(m_pGame->getSession()->showEngineCounter());
  m_pGame->getGameRenderer()->setShowMinimap(m_pGame->getSession()->showMinimap());
  m_pGame->getGameRenderer()->setShowTimePanel(true);

  m_bAutoZoomInitialized = false;

  try {
    m_pGame->getMotoGame()->playLevel();
  } catch(Exception &e) {
    Logger::Log("** Warning ** : level '%s' cannot be loaded", m_pGame->getMotoGame()->getLevelSrc()->Name().c_str());

    char cBuf[256];
    sprintf(cBuf,GAMETEXT_LEVELCANNOTBELOADED, m_pGame->getMotoGame()->getLevelSrc()->Name().c_str());

    StateMessageBox* v_msgboxState = new StateMessageBox(this, m_pGame, cBuf, UI_MSGBOX_OK);
    v_msgboxState->setId("ERROR");
    m_pGame->getStateManager()->pushState(v_msgboxState);
  }

  setScoresTimes();
  m_pGame->playMusic(m_pGame->getMotoGame()->getLevelSrc()->Music());
}

void StatePlaying::leave()
{
  m_pGame->getMotoGame()->setInfos("");
}

void StatePlaying::enterAfterPop()
{
  m_fLastPhysTime = GameApp::getXMTime();
}

void StatePlaying::leaveAfterPush()
{

}

bool StatePlaying::update()
{
  if(doUpdate() == false){
    return false;
  }

  StateScene::update();

  if(isLockedScene() == false) {
    bool v_all_dead       = true;
    bool v_one_still_play = false;
    bool v_one_finished   = false;
    
    for(unsigned int i=0; i<m_pGame->getMotoGame()->Players().size(); i++) {
      if(m_pGame->getMotoGame()->Players()[i]->isDead() == false) {
	v_all_dead = false;
      }
      if(m_pGame->getMotoGame()->Players()[i]->isFinished()) {
	v_one_finished = true;
      }
      
      if(m_pGame->getMotoGame()->Players()[i]->isFinished() == false && m_pGame->getMotoGame()->Players()[i]->isDead() == false) {
	v_one_still_play = true;
      }
    }
    
    if(v_one_still_play == false || m_pGame->getSession()->MultiStopWhenOneFinishes()) { // let people continuing when one finished or not
      if(v_one_finished) {
	/* You're done maaaan! :D */
	
	/* finalize the replay */
	if(m_pGame->isAReplayToSave()) {
	  m_pGame->finalizeReplay(true);
	}
	
	/* update profiles */
	float v_finish_time = 0.0;
	std::string TimeStamp = m_pGame->getTimeStamp();
	for(unsigned int i=0; i<m_pGame->getMotoGame()->Players().size(); i++) {
	  if(m_pGame->getMotoGame()->Players()[i]->isFinished()) {
	    v_finish_time  = m_pGame->getMotoGame()->Players()[i]->finishTime();
	  }
	}
	if(m_pGame->getMotoGame()->Players().size() == 1) {
	  m_pGame->getDb()->profiles_addFinishTime(m_pGame->getSession()->profile(), m_pGame->getMotoGame()->getLevelSrc()->Id(),
						   TimeStamp, v_finish_time);
	}
	
	/* Update stats */
	/* update stats only in one player mode */
	if(m_pGame->getMotoGame()->Players().size() == 1) {       
	  m_pGame->getDb()->stats_levelCompleted(m_pGame->getSession()->profile(),
						 m_pGame->getMotoGame()->getLevelSrc()->Id(),
						 m_pGame->getMotoGame()->Players()[0]->finishTime());
	  m_pGame->getStateManager()->sendAsynchronousMessage("LEVELS_UPDATED");
	}
	m_pGame->getStateManager()->pushState(new StateFinished(m_pGame, this));
      } else if(v_all_dead) {
	/* You're dead maan! */
	if(m_pGame->isAReplayToSave()) {
	  m_pGame->finalizeReplay(false);
	}

	/* Update stats */        
	if(m_pGame->getMotoGame()->Players().size() == 1) {
	  m_pGame->getDb()->stats_died(m_pGame->getSession()->profile(),
				       m_pGame->getMotoGame()->getLevelSrc()->Id(),
				       m_pGame->getMotoGame()->getTime());
	}                

	/* Play the DIE!!! sound */
	try {
	  Sound::playSampleByName(m_pGame->getTheme()->getSound("Headcrash")->FilePath(), 0.3);
	} catch(Exception &e) {
	}

	if(m_pGame->getSession()->enableDeadAnimation()) {
	  m_pGame->getStateManager()->replaceState(new StateDeadJust(m_pGame));
	} else {
	  m_pGame->getStateManager()->pushState(new StateDeadMenu(m_pGame, true, this));
	}
      }
    }
  }    

  return true;
}

bool StatePlaying::render()
{
  StateScene::render();
  return true;
}

void StatePlaying::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {

  case SDLK_ESCAPE:
    if(isLockedScene() == false) {
      /* Escape pauses */
      m_pGame->getStateManager()->pushState(new StatePause(m_pGame, this));
    }
    break;

  case SDLK_RETURN:
    /* retart immediatly the level */
    restartLevel();
    break;

  case SDLK_KP5:
    if(autoZoom() == false) {
      setAutoZoom(true);
    }
    break;

#if defined(ENABLE_ZOOMING)
  case SDLK_KP7:
    /* Zoom in */
    for(unsigned int i=0; i<m_pGame->getMotoGame()->Cameras().size(); i++) {
      m_pGame->getMotoGame()->Cameras()[i]->zoom(0.002);
    }
    break;

  case SDLK_KP9:
    /* Zoom out */
    for(unsigned int i=0; i<m_pGame->getMotoGame()->Cameras().size(); i++) {
      m_pGame->getMotoGame()->Cameras()[i]->zoom(-0.002);
    }
    break;

  case SDLK_HOME:
    for(unsigned int i=0; i<m_pGame->getMotoGame()->Cameras().size(); i++) {
      m_pGame->getMotoGame()->Cameras()[i]->initCamera();
    }
    break;

  case SDLK_KP6:
    for(unsigned int i=0; i<m_pGame->getMotoGame()->Cameras().size(); i++) {
      m_pGame->getMotoGame()->Cameras()[i]->moveCamera(1.0, 0.0);
    }
    break;

  case SDLK_KP4:
    for(unsigned int i=0; i<m_pGame->getMotoGame()->Cameras().size(); i++) {
      m_pGame->getMotoGame()->Cameras()[i]->moveCamera(-1.0, 0.0);
    }
    break;

  case SDLK_KP8:
    for(unsigned int i=0; i<m_pGame->getMotoGame()->Cameras().size(); i++) {
      m_pGame->getMotoGame()->Cameras()[i]->moveCamera(0.0, 1.0);
    }
    break;

  case SDLK_KP2:
    for(unsigned int i=0; i<m_pGame->getMotoGame()->Cameras().size(); i++) {
      m_pGame->getMotoGame()->Cameras()[i]->moveCamera(0.0, -1.0);
    }
    break;

  case SDLK_KP0:
    if((mod & KMOD_LCTRL) == KMOD_LCTRL) {
      for(unsigned int i=0; i<m_pGame->getMotoGame()->Players().size(); i++) {
	if(m_pGame->getMotoGame()->Cameras().size() > 0) {
	  m_pGame->TeleportationCheatTo(i, Vector2f(m_pGame->getMotoGame()->Cameras()[0]->getCameraPositionX(),
						    m_pGame->getMotoGame()->Cameras()[0]->getCameraPositionY()));
	}
      }
    }
    break;
#endif
    
  default:
    /* Notify the controller */
    m_pGame->getInputHandler()->handleInput(INPUT_KEY_DOWN, nKey, mod,
					    m_pGame->getMotoGame()->Players(),
					    m_pGame->getMotoGame()->Cameras(),
					    m_pGame);
  }
  
  StateScene::keyDown(nKey, mod, nChar);
}

void StatePlaying::keyUp(int nKey, SDLMod mod)
{
  switch(nKey) {

  case SDLK_KP5:
    if(autoZoom() && autoZoomStep() == 1) {
      setAutoZoomStep(2);
    }
    break;
    
  default:
    m_pGame->getInputHandler()->handleInput(INPUT_KEY_UP,nKey,mod,
					    m_pGame->getMotoGame()->Players(),
					    m_pGame->getMotoGame()->Cameras(),
					    m_pGame);
    StateScene::keyUp(nKey, mod);
  }
}

void StatePlaying::mouseDown(int nButton)
{
  m_pGame->getInputHandler()->handleInput(INPUT_KEY_DOWN,nButton,KMOD_NONE,
			     m_pGame->getMotoGame()->Players(),
			     m_pGame->getMotoGame()->Cameras(),
			     m_pGame);

  StateScene::mouseDown(nButton);
}

void StatePlaying::mouseDoubleClick(int nButton)
{
  StateScene::mouseDoubleClick(nButton);
}

void StatePlaying::mouseUp(int nButton)
{
  m_pGame->getInputHandler()->handleInput(INPUT_KEY_UP,nButton,KMOD_NONE,
			     m_pGame->getMotoGame()->Players(),
			     m_pGame->getMotoGame()->Cameras(),
			     m_pGame);

  StateScene::mouseUp(nButton);
}

void StatePlaying::send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  if(i_id == "ERROR") {
    m_commands.push("ERROR");
  }
}
