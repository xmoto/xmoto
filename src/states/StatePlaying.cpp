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
#include "Universe.h"
#include "Trainer.h"
#include "SysMessage.h"

StatePlaying::StatePlaying(Universe* i_universe):
  StateScene(i_universe)
{
  m_name = "StatePlaying";
  m_gameIsFinished = false;
  m_displayStats   = false;

  /* prepare stats */
  makeStatsStr();
}

StatePlaying::~StatePlaying()
{
}

void StatePlaying::enter()
{
  StateScene::enter();

  m_gameIsFinished = false;

  if(XMSession::instance()->hidePlayingInformation() == false) {
    GameRenderer::instance()->setShowEngineCounter(XMSession::instance()->showEngineCounter());
    GameRenderer::instance()->setShowMinimap(XMSession::instance()->showMinimap());
    GameRenderer::instance()->setShowTimePanel(true);
  } else {
    GameRenderer::instance()->setShowEngineCounter(false);
    GameRenderer::instance()->setShowMinimap(false);
    GameRenderer::instance()->setShowTimePanel(false);
  }

  std::string v_level_name;
  try {
    if(m_universe != NULL) {
      for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	v_level_name = m_universe->getScenes()[i]->getLevelSrc()->Name();
	m_universe->getScenes()[i]->playLevel();
      }
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

  if(m_universe != NULL) {
    if(m_universe->getScenes().size() > 0) {
      // play music of the first world
      GameApp::instance()->playMusic(m_universe->getScenes()[0]->getLevelSrc()->Music());
    }
  }

  // read keys for more reactivity
  InputHandler::instance()->dealWithActivedKeys(m_universe);
}

void StatePlaying::leave()
{
  if(m_universe != NULL) {
    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->setInfos("");
    }
  }

  if(GameApp::instance()->isRequestingEnd()) {
    // when end if forced, update stats as aborted
    if(m_universe != NULL) {
      if(m_universe->getScenes().size() == 1) {
	if(m_universe->getScenes()[0]->Players().size() == 1) {
	  if(m_universe->getScenes()[0]->Players()[0]->isDead()     == false &&
	     m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
	    
	    xmDatabase::instance("main")->stats_abortedLevel(XMSession::instance()->profile(),
							     m_universe->getScenes()[0]->getLevelSrc()->Id(),
							     m_universe->getScenes()[0]->getTime());
	  }
	}
      }
    }
  }
}

void StatePlaying::enterAfterPop()
{
  // recheck keys
  InputHandler::instance()->dealWithActivedKeys(m_universe);
  m_fLastPhysTime = GameApp::getXMTime();
  m_displayStats = false;
}

bool StatePlaying::update()
{
  if(StateScene::update() == false)
    return false;

  if(isLockedScene() == false) {
    bool v_all_dead       = true;
    bool v_one_still_play = false;
    bool v_one_finished   = false;
    
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Players().size(); i++) {
	  if(m_universe->getScenes()[j]->Players()[i]->isDead() == false) {
	    v_all_dead = false;
	  }
	  if(m_universe->getScenes()[j]->Players()[i]->isFinished()) {
	    v_one_finished = true;
	  }
	  
	  if(m_universe->getScenes()[j]->Players()[i]->isFinished() == false &&
	     m_universe->getScenes()[j]->Players()[i]->isDead() == false) {
	    v_one_still_play = true;
	  }
	}
      }
    }
    
    if(m_gameIsFinished == false) {
      if(v_one_still_play == false || XMSession::instance()->MultiStopWhenOneFinishes()) { // let people continuing when one finished or not
	if(v_one_finished) {
	  /* You're done maaaan! :D */
	  onOneFinish();
	  m_gameIsFinished = true;
	} else if(v_all_dead) {
	  /* You're dead maan! */
	  onAllDead();
	  m_gameIsFinished = true;
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
      m_displayStats = true;
      StateManager::instance()->pushState(new StatePause(m_universe, this));
    }
  }
  else if(nKey == SDLK_RETURN && (mod & (KMOD_CTRL|KMOD_SHIFT|KMOD_ALT|KMOD_META)) == 0){
    restartLevel();
  }

#if defined(ENABLE_ZOOMING)
  else if(nKey == SDLK_KP7){
    /* Zoom in */
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->zoom(0.002);
	}
      }
    }
  }
  else if(nKey == SDLK_KP9){
    /* Zoom out */
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->zoom(-0.002);
	}
      }
    }
  }
  else if(nKey == SDLK_HOME){
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->initCamera();
	}
      }
    }
  }
  else if(nKey == SDLK_KP6){
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->moveCamera(1.0, 0.0);
	}
      }
    }
  }
  else if(nKey == SDLK_KP4){
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->moveCamera(-1.0, 0.0);
	}
      }
    }
  }
  else if(nKey == SDLK_KP8){
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->moveCamera(0.0, 1.0);
	}
      }
    }
  }
  else if(nKey == SDLK_KP2){
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->moveCamera(0.0, -1.0);
	}
      }
    }
  }
  else if(nKey == SDLK_KP0 && (mod & KMOD_LCTRL) == KMOD_LCTRL){
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Players().size(); i++) {
	  if(m_universe->getScenes()[j]->Cameras().size() > 0) {
	    m_universe->TeleportationCheatTo(i, Vector2f(m_universe->getScenes()[j]->Cameras()[0]->getCameraPositionX(),
							 m_universe->getScenes()[j]->Cameras()[0]->getCameraPositionY()));
	  }
	}
      }
    }
  }
  else if(nKey == SDLK_KP0 && (mod & (KMOD_CTRL|KMOD_SHIFT|KMOD_ALT|KMOD_META)) == 0){        //TRAINER
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
        Trainer::instance()->storePosition( m_universe->getScenes()[j]->getLevelSrc()->Id(),
                                            m_universe->getScenes()[j]->getPlayerPosition(0) );
          //TODO: bool getPlayerFaceDir (int i_player)
        char sysmsg[256];
        snprintf(sysmsg, 256, SYS_MSG_TRAIN_STORED, Trainer::instance()->getMaxRestoreIndex()+1);
        SysMessage::instance()->displayText(sysmsg);
      }
    }
  }
  else if(nKey == SDLK_BACKSPACE){       //TRAINER
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
        if( Trainer::instance()->isRestorePositionAvailable( m_universe->getScenes()[j]->getLevelSrc()->Id() ) ) {
          Vector2f pos = Trainer::instance()->getCurrentRestorePosition( m_universe->getScenes()[j]->getLevelSrc()->Id() );
          m_universe->TeleportationCheatTo(0, pos );
          char sysmsg[256];
          snprintf(sysmsg, 256, SYS_MSG_TRAIN_RESTORING, Trainer::instance()->getCurrentRestoreIndex()+1,
                                                         Trainer::instance()->getMaxRestoreIndex()+1);
          SysMessage::instance()->displayText(sysmsg);
        } else {
          SysMessage::instance()->displayText(SYS_MSG_TRAIN_NO_RESTORE_AVAIL);
        }
      }
    }
  }
  else if(nKey == SDLK_KP_MINUS){        //TRAINER
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
        if( Trainer::instance()->isRestorePositionAvailable( m_universe->getScenes()[j]->getLevelSrc()->Id() ) ) {
          Vector2f pos = Trainer::instance()->getPreviousRestorePosition( m_universe->getScenes()[j]->getLevelSrc()->Id() );
          m_universe->TeleportationCheatTo(0, pos );
          char sysmsg[256];
          snprintf(sysmsg, 256, SYS_MSG_TRAIN_RESTORING, Trainer::instance()->getCurrentRestoreIndex()+1,
                                                         Trainer::instance()->getMaxRestoreIndex()+1);
          SysMessage::instance()->displayText(sysmsg);
        } else {
          SysMessage::instance()->displayText(SYS_MSG_TRAIN_NO_RESTORE_AVAIL);
        }
      }
    }
  }
  else if(nKey == SDLK_KP_PLUS){        //TRAINER
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
        if( Trainer::instance()->isRestorePositionAvailable( m_universe->getScenes()[j]->getLevelSrc()->Id() ) ) {
          Vector2f pos = Trainer::instance()->getNextRestorePosition( m_universe->getScenes()[j]->getLevelSrc()->Id() );
          m_universe->TeleportationCheatTo(0, pos );
          char sysmsg[256];
          snprintf(sysmsg, 256, SYS_MSG_TRAIN_RESTORING, Trainer::instance()->getCurrentRestoreIndex()+1,
                                                         Trainer::instance()->getMaxRestoreIndex()+1);
          SysMessage::instance()->displayText(sysmsg);
        } else {
          SysMessage::instance()->displayText(SYS_MSG_TRAIN_NO_RESTORE_AVAIL);
        }
      }
    }
  }
#endif

  else {
    // to avoid people changing direction during the autozoom
    if(m_autoZoom == false){
      /* Notify the controller */
      InputHandler::instance()->handleInput(m_universe, INPUT_KEY_DOWN, nKey, mod);
    }
  }

  StateScene::keyDown(nKey, mod, nChar);
}

void StatePlaying::keyUp(int nKey, SDLMod mod) {
  InputHandler::instance()->handleInput(m_universe, INPUT_KEY_UP, nKey, mod);
  StateScene::keyUp(nKey, mod);
}

void StatePlaying::mouseDown(int nButton)
{
  InputHandler::instance()->handleInput(m_universe, INPUT_KEY_DOWN, nButton, KMOD_NONE);

  StateScene::mouseDown(nButton);
}

void StatePlaying::mouseUp(int nButton)
{
  InputHandler::instance()->handleInput(m_universe, INPUT_KEY_UP, nButton, KMOD_NONE);

  StateScene::mouseUp(nButton);
}

void StatePlaying::send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  if(i_id == "ERROR") {
    m_commands.push("ERROR");
  }
}

void StatePlaying::onOneFinish() {
  GameApp*  pGame = GameApp::instance();

  /* finalize the replay */
  if(m_universe != NULL) {
    if(m_universe->isAReplayToSave()) {
      m_universe->finalizeReplay(true);
    }
  }
  
  /* update profiles */
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(m_universe->getScenes()[0]->Players().size() == 1) {
	int v_finish_time = 0;
	std::string TimeStamp = pGame->getTimeStamp();
	if(m_universe->getScenes()[0]->Players()[0]->isFinished()) {
	  v_finish_time  = m_universe->getScenes()[0]->Players()[0]->finishTime();
	}
	xmDatabase::instance("main")->profiles_addFinishTime(XMSession::instance()->profile(),
							     m_universe->getScenes()[0]->getLevelSrc()->Id(),
							     TimeStamp,
							     v_finish_time);
	StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
      }
    }
  }	  
  
  /* Update stats */
  /* update stats only in one player mode */
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(m_universe->getScenes()[0]->Players().size() == 1) {
	xmDatabase::instance("main")->stats_levelCompleted(XMSession::instance()->profile(),
							   m_universe->getScenes()[0]->getLevelSrc()->Id(),
							   m_universe->getScenes()[0]->Players()[0]->finishTime());
	StateManager::instance()->sendAsynchronousMessage("LEVELS_UPDATED");
	StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
      }
    }
  }
  StateManager::instance()->pushState(new StateFinished(m_universe, this));
}

void StatePlaying::onAllDead() {  
  if(m_universe != NULL) {	
    if(m_universe->isAReplayToSave()) {
      m_universe->finalizeReplay(false);
    }
  }
  
  /* Update stats */
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(m_universe->getScenes()[0]->Players().size() == 1) {
	xmDatabase::instance("main")->stats_died(XMSession::instance()->profile(),
						 m_universe->getScenes()[0]->getLevelSrc()->Id(),
						 m_universe->getScenes()[0]->getTime());
	StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
      }
    }
  }
  
  /* Play the DIE!!! sound */
  try {
    Sound::playSampleByName(Theme::instance()->getSound("Headcrash")->FilePath(), 0.3);
  } catch(Exception &e) {
  }
  
  if(XMSession::instance()->enableDeadAnimation()) {
    StateManager::instance()->replaceState(new StateDeadJust(m_universe));
  } else {
    StateManager::instance()->pushState(new StateDeadMenu(m_universe, true, this));
  }
}

void StatePlaying::abortPlaying() {
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(m_universe->getScenes()[0]->Players().size() == 1) {
	if(m_universe->getScenes()[0]->Players()[0]->isDead()     == false &&
	   m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
	  xmDatabase::instance("main")->stats_abortedLevel(XMSession::instance()->profile(),
							   m_universe->getScenes()[0]->getLevelSrc()->Id(),
							   m_universe->getScenes()[0]->getTime());
	  StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
	}
      }
    }
  }

  StateScene::abortPlaying();
}

void StatePlaying::nextLevel(bool i_positifOrder) {
  GameApp*  pGame  = GameApp::instance();
  std::string v_nextLevel;
  std::string v_currentLevel;
  
  // take the level id of the first world
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() > 0) {
      v_currentLevel = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  if(i_positifOrder) {
    v_nextLevel = pGame->determineNextLevel(v_currentLevel);
  } else {
    v_nextLevel = pGame->determinePreviousLevel(v_currentLevel);
  }

  /* update stats */
  if(v_nextLevel != "") {
    if(m_universe != NULL) {
      if(m_universe->getScenes().size() == 1) {
	if(m_universe->getScenes()[0]->Players().size() == 1) {
	  if(m_universe->getScenes()[0]->Players()[0]->isDead()     == false &&
	     m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
	    xmDatabase::instance("main")->stats_abortedLevel(XMSession::instance()->profile(),
							     v_currentLevel,
							     m_universe->getScenes()[0]->getTime());
	    StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
	  }
	}
      }
    }
  }

  StateScene::nextLevel(i_positifOrder);
}

void StatePlaying::restartLevel(bool i_reloadLevel) {
  /* Update stats */
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(m_universe->getScenes()[0]->Players().size() == 1) {
	if(m_universe->getScenes()[0]->Players()[0]->isDead()     == false &&
	   m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
	  xmDatabase::instance("main")->stats_levelRestarted(XMSession::instance()->profile(),
							     m_universe->getScenes()[0]->getLevelSrc()->Id(),
							     m_universe->getScenes()[0]->getTime());
	  StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
	}
      }
    }
  }

  StateScene::restartLevel(i_reloadLevel);
}

bool StatePlaying::render() {
  StateScene::render();

  if(m_displayStats) {
    displayStats();
  }

  return true;
}
