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

#include "StatePreplaying.h"
#include "StatePlaying.h"
#include "helpers/Log.h"
#include "helpers/Text.h"
#include "GameText.h"
#include "Game.h"
#include "xmscene/Camera.h"
#include "XMSession.h"
#include "xmscene/BikePlayer.h"
#include "StateMessageBox.h"
#include "StateDownloadGhost.h"
#include "drawlib/DrawLib.h"
#include "CameraAnimation.h"
#include "Universe.h"
#include "VFileIO.h"

#define PRESTART_ANIMATION_LEVEL_MSG_DURATION 100

StatePreplaying::StatePreplaying(const std::string i_idlevel, bool i_sameLevel):
  StateScene()
{
  m_name  = "StatePreplaying";
  m_idlevel = i_idlevel;

  m_secondInitPhaseDone = false;
  m_ghostDownloaded     = false;
  m_ghostDownloading_failed = false;

  m_sameLevel = i_sameLevel;
  /* if the level is not the same, ask to play the animation */
  m_playAnimation = m_sameLevel == false;

  m_cameraAnim = NULL;
}

StatePreplaying::~StatePreplaying()
{
  if(m_cameraAnim != NULL) {
    delete m_cameraAnim;
  }
}


void StatePreplaying::enter()
{
  GameApp*  pGame  = GameApp::instance();
 unsigned int v_nbPlayer = XMSession::instance()->multiNbPlayers();

  StateScene::enter();

  GameRenderer::instance()->setShowEngineCounter(false);
  GameRenderer::instance()->setShowMinimap(false);
  GameRenderer::instance()->setShowTimePanel(false);
  GameRenderer::instance()->hideReplayHelp();

  m_universe =  new Universe();

  try {
    initUniverse();
  } catch(Exception &e) {
    delete m_universe;
    StateManager::instance()->replaceState(new StateMessageBox(NULL, "Ooops", UI_MSGBOX_OK));
    return;
  }  

  for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
    m_universe->getScenes()[i]->setDeathAnim(XMSession::instance()->enableDeadAnimation());
    m_universe->getScenes()[i]->setShowGhostTimeDiff(XMSession::instance()->showGhostTimeDifference());
  }

  try {
    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->loadLevel(xmDatabase::instance("main"), m_idlevel);
    }
  } catch(Exception &e) {
    Logger::Log("** Warning ** : level '%s' cannot be loaded", m_idlevel.c_str());
    char cBuf[256];
    sprintf(cBuf,GAMETEXT_LEVELCANNOTBELOADED, m_idlevel.c_str());
    delete m_universe;
    m_universe = NULL;
    StateManager::instance()->replaceState(new StateMessageBox(NULL, cBuf, UI_MSGBOX_OK));
    return;
  }

  for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
    if(m_universe->getScenes()[i]->getLevelSrc()->isXMotoTooOld()) {
      Logger::Log("** Warning ** : level '%s' requires newer X-Moto",
		  m_universe->getScenes()[i]->getLevelSrc()->Name().c_str());
      
      char cBuf[256];
      sprintf(cBuf,GAMETEXT_NEWERXMOTOREQUIRED,
	      m_universe->getScenes()[i]->getLevelSrc()->getRequiredVersion().c_str());
      
      delete m_universe;
      m_universe = NULL;
      StateManager::instance()->replaceState(new StateMessageBox(NULL, cBuf, UI_MSGBOX_OK));
      return;
    }
  }

  try {
    preloadLevels();
    initPlayers();

    // if there's more camera than player (ex: 3 players and 4 cameras),
    // then, make the remaining cameras follow the first player
    if(m_universe->getScenes().size() == 1) {
      if(v_nbPlayer < m_universe->getScenes()[0]->getNumberCameras()){
	for(unsigned int i=v_nbPlayer; i<m_universe->getScenes()[0]->getNumberCameras(); i++){
	  m_universe->getScenes()[0]->setCurrentCamera(i);
	  m_universe->getScenes()[0]->getCamera()->setPlayerToFollow(m_universe->getScenes()[0]->Players()[0]);
	  m_universe->getScenes()[0]->getCamera()->setScroll(false, m_universe->getScenes()[0]->getGravity());
	}
      }
    }
    
    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      if(m_universe->getScenes()[i]->getNumberCameras() > 1){
	// make the zoom camera follow the first player
	m_universe->getScenes()[i]->setAutoZoomCamera();
	m_universe->getScenes()[i]->getCamera()->setPlayerToFollow(m_universe->getScenes()[i]->Players()[0]);
	m_universe->getScenes()[i]->getCamera()->setScroll(false, m_universe->getScenes()[i]->getGravity());
      }
    }

    // reset handler, set mirror mode
    InputHandler::instance()->reset();
    for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
      for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	m_universe->getScenes()[j]->Cameras()[i]->setMirrored(XMSession::instance()->mirrorMode());
      }
    }
    InputHandler::instance()->setMirrored(XMSession::instance()->mirrorMode());

  } catch(Exception &e) {
    Logger::Log(std::string("** Warning ** : failed to initialize level\n" + e.getMsg()).c_str());
    delete m_universe;
    m_universe = NULL;
    StateManager::instance()->replaceState(new StateMessageBox(NULL, splitText(e.getMsg(), 50), UI_MSGBOX_OK));
    return;
  }

  if(needToDownloadGhost() == true){
    StateManager::instance()->pushState(new StateDownloadGhost(m_idlevel));
  } else {
    m_ghostDownloaded = true;
  }

  // music
  if(m_playAnimation) {
    pGame->playMusic("");
  } else {
    if(m_universe->getScenes().size() > 0) {
      // play music of the first world
      GameApp::instance()->playMusic(m_universe->getScenes()[0]->getLevelSrc()->Music());
    }
  }

  /* prepare stats */
  makeStatsStr();
}

bool StatePreplaying::shouldBeAnimated() const {
  return m_playAnimation && XMSession::instance()->enableInitZoom() && XMSession::instance()->ugly() == false;
}

void StatePreplaying::leave()
{
  setAutoZoom(false);
}

void StatePreplaying::enterAfterPop()
{
  setAutoZoom(shouldBeAnimated());
}

void StatePreplaying::leaveAfterPush()
{
  setAutoZoom(false);
}

bool StatePreplaying::update()
{
  if(doUpdate() == false){
    return false;
  }

  if(m_ghostDownloaded == false && m_ghostDownloading_failed == false){
    return true;
  }

  if(m_secondInitPhaseDone == false){
    secondInitPhase();
    m_secondInitPhaseDone = true;
  }

  if(shouldBeAnimated()) {
    if(m_cameraAnim != NULL) {
      if(m_cameraAnim->step() == false) {
	m_playAnimation = false;
      }
    }
  } else { /* animation has been rupted */
    m_playAnimation = false; // disable anim
    if(m_cameraAnim != NULL) {
      m_cameraAnim->uninit();
    }
    runPlaying();
  }
  if(m_universe != NULL) {
    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->updateGameMessages();
    }
  }

  return true;
}

bool StatePreplaying::render()
{
  if(m_secondInitPhaseDone == false){
    DrawLib* drawLib = GameApp::instance()->getDrawLib();
    int width  = drawLib->getDispWidth();
    int height = drawLib->getDispHeight();

    drawLib->drawBox(Vector2f(0,     height),
		     Vector2f(width, 0),
		     0.0, MAKE_COLOR(0,0,0,255));
  } else {
    StateScene::render();
  }

  displayStats();
  
  return true;
}

void StatePreplaying::keyDown(int nKey, SDLMod mod,int nChar)
{
  m_playAnimation = false;
}

void StatePreplaying::secondInitPhase()
{
  GameApp*  pGame  = GameApp::instance();

  try {
    /* add the ghosts */
    if(XMSession::instance()->enableGhosts() && allowGhosts()) {
      try {
	if(m_universe != NULL) {
	  for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	    pGame->addGhosts(m_universe->getScenes()[i], Theme::instance());
	  }
	}
      } catch(Exception &e) {
	/* anyway */
      }
    }
  } catch(Exception &e) {
    Logger::Log(std::string("** Warning ** : failed to initialize level\n" + e.getMsg()).c_str());
    closePlaying();
    StateManager::instance()->replaceState(new StateMessageBox(NULL, splitText(e.getMsg(), 50), UI_MSGBOX_OK));
    return;
  }

  /* Prepare level */
  GameRenderer::instance()->prepareForNewLevel(m_universe);

  /* If "preplaying" / "initial-zoom" is enabled, this is where it's done */
  // animation
  if(m_cameraAnim != NULL) {
    delete m_cameraAnim;
  }
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() > 0) {
      m_universe->getScenes()[0]->setAutoZoomCamera();
      m_cameraAnim = new ZoomingCameraAnimation(m_universe->getScenes()[0]->getCamera(), pGame->getDrawLib(), m_universe->getScenes()[0]);
      m_cameraAnim->init();
    }
  }

  /* display level name */
  if(m_sameLevel == false) {
    if(m_universe != NULL) {
      for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	m_universe->getScenes()[i]->gameMessage(m_universe->getScenes()[i]->getLevelSrc()->Name(),
						false,
						PRESTART_ANIMATION_LEVEL_MSG_DURATION);
      }
    }
  }
  
  setAutoZoom(shouldBeAnimated());
}

void StatePreplaying::executeOneCommand(std::string cmd)
{
  if(cmd == "GHOST_DOWNLOADED"){
    m_ghostDownloaded = true;
  } else if (cmd == "GHOST_DOWNLOADING_FAILED") {
    m_ghostDownloading_failed = true;
  }
  else {
    StateScene::executeOneCommand(cmd);
  }
}

bool StatePreplaying::needToDownloadGhost()
{
  if(XMSession::instance()->www() == false           ||
     XMSession::instance()->enableGhosts() == false ||
     (XMSession::instance()->ghostStrategy_BESTOFREFROOM()    == false &&
      XMSession::instance()->ghostStrategy_BESTOFOTHERROOMS() == false)
     ) {
    return false;
  }

  if(allowGhosts() == false) {
    return false;
  }

  char **v_result;
  unsigned int nrow;
  std::string res;
  std::string v_replayName;
  std::string v_fileUrl;

  bool v_need_one = false;
  for(unsigned int i=0; i<XMSession::instance()->nbRoomsEnabled(); i++) {
    v_result = xmDatabase::instance("main")->readDB("SELECT fileUrl FROM webhighscores "
						    "WHERE id_room=" + XMSession::instance()->idRoom(i) + " "
						    "AND id_level=\"" + xmDatabase::protectString(m_idlevel) + "\";",
						    nrow);
    if(nrow != 0) {
      v_fileUrl    = xmDatabase::instance("main")->getResult(v_result, 1, 0, 0);
      v_replayName = FS::getFileBaseName(v_fileUrl);
    }
    xmDatabase::instance("main")->read_DB_free(v_result);

    /* search if the replay is already downloaded */
    if(xmDatabase::instance("main")->replays_exists(v_replayName) == false) {
      v_need_one = true;
    }
  }

  return v_need_one;
}

bool StatePreplaying::allowGhosts() {
  return true;
}