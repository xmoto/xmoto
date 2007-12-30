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
#include "Game.h"
#include "StatePlaying.h"
#include "helpers/Log.h"
#include "GameText.h"
#include "xmscene/Camera.h"
#include "XMSession.h"
#include "xmscene/BikePlayer.h"
#include "StateMessageBox.h"
#include "StateDownloadGhost.h"
#include "drawlib/DrawLib.h"
#include "CameraAnimation.h"

#define PRESTART_ANIMATION_LEVEL_MSG_DURATION 1.0

StatePreplaying::StatePreplaying(const std::string i_idlevel, bool i_sameLevel):
  StateScene()
{
  m_name  = "StatePreplaying";
  m_idlevel = i_idlevel;

  m_secondInitPhaseDone = false;
  m_ghostDownloaded     = false;

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

  StateScene::enter();

  GameRenderer::instance()->setShowEngineCounter(false);
  GameRenderer::instance()->setShowMinimap(false);
  GameRenderer::instance()->setShowTimePanel(false);
  GameRenderer::instance()->hideReplayHelp();

  for(unsigned int i=0; i<GameApp::instance()->getScenes().size(); i++) {
    GameApp::instance()->getScenes()[i]->setDeathAnim(XMSession::instance()->enableDeadAnimation());
    GameApp::instance()->getScenes()[i]->setShowGhostTimeDiff(XMSession::instance()->showGhostTimeDifference());
  }

  try {
    for(unsigned int i=0; i<GameApp::instance()->getScenes().size(); i++) {
      GameApp::instance()->getScenes()[i]->loadLevel(xmDatabase::instance("main"), m_idlevel);
    }
  } catch(Exception &e) {
    Logger::Log("** Warning ** : level '%s' cannot be loaded", m_idlevel.c_str());
    char cBuf[256];
    sprintf(cBuf,GAMETEXT_LEVELCANNOTBELOADED, m_idlevel.c_str());
    StateManager::instance()->replaceState(new StateMessageBox(NULL, cBuf, UI_MSGBOX_OK));
    return;
  }

  for(unsigned int i=0; i<GameApp::instance()->getScenes().size(); i++) {
    if(GameApp::instance()->getScenes()[i]->getLevelSrc()->isXMotoTooOld()) {
      Logger::Log("** Warning ** : level '%s' requires newer X-Moto",
		  GameApp::instance()->getScenes()[i]->getLevelSrc()->Name().c_str());
      
      char cBuf[256];
      sprintf(cBuf,GAMETEXT_NEWERXMOTOREQUIRED,
	      GameApp::instance()->getScenes()[i]->getLevelSrc()->getRequiredVersion().c_str());

      for(unsigned int j=0; j<GameApp::instance()->getScenes().size(); j++) {
	GameApp::instance()->getScenes()[j]->endLevel();
      }
      StateManager::instance()->replaceState(new StateMessageBox(NULL, cBuf, UI_MSGBOX_OK));
      return;
    }
  }

  /* Start playing right away */     
  pGame->initReplay();
      
  try {
    for(unsigned int i=0; i<GameApp::instance()->getScenes().size(); i++) {
      GameApp::instance()->getScenes()[i]->prePlayLevel(pGame->getCurrentReplay(), true);
      GameApp::instance()->getScenes()[i]->setInfos("");
    }
	
    /* add the players */
    unsigned int v_nbPlayer = XMSession::instance()->multiNbPlayers();
    Logger::Log("Preplay level for %i player(s)", v_nbPlayer);

    pGame->initCameras(v_nbPlayer);
   
    if(true) { // monoworld
      MotoGame* v_world = GameApp::instance()->getScenes()[0];

      for(unsigned int i=0; i<v_nbPlayer; i++) {
	v_world = GameApp::instance()->getScenes()[0];
	v_world->setCurrentCamera(i);
	v_world->getCamera()->setPlayerToFollow(v_world->addPlayerBiker(v_world->getLevelSrc()->PlayerStart(),
									DD_RIGHT,
									Theme::instance(), Theme::instance()->getPlayerTheme(),
									pGame->getColorFromPlayerNumber(i),
									pGame->getUglyColorFromPlayerNumber(i),
									XMSession::instance()->enableEngineSound()));
	v_world->getCamera()->setScroll(false, v_world->getGravity());
      }
    }

    // if there's more camera than player (ex: 3 players and 4 cameras),
    // then, make the remaining cameras follow the first player
    if(GameApp::instance()->getScenes().size() == 1) {
      if(v_nbPlayer < GameApp::instance()->getScenes()[0]->getNumberCameras()){
	for(unsigned int i=v_nbPlayer; i<GameApp::instance()->getScenes()[0]->getNumberCameras(); i++){
	  GameApp::instance()->getScenes()[0]->setCurrentCamera(i);
	  GameApp::instance()->getScenes()[0]->getCamera()->setPlayerToFollow(GameApp::instance()->getScenes()[0]->Players()[0]);
	  GameApp::instance()->getScenes()[0]->getCamera()->setScroll(false, GameApp::instance()->getScenes()[0]->getGravity());
	}
      }
    }

      for(unsigned int i=0; i<GameApp::instance()->getScenes().size(); i++) {
	if(GameApp::instance()->getScenes()[i]->getNumberCameras() > 1){
	  // make the zoom camera follow the first player
	  GameApp::instance()->getScenes()[i]->setAutoZoomCamera();
	  GameApp::instance()->getScenes()[i]->getCamera()->setPlayerToFollow(GameApp::instance()->getScenes()[i]->Players()[0]);
	  GameApp::instance()->getScenes()[i]->getCamera()->setScroll(false, GameApp::instance()->getScenes()[i]->getGravity());
	}
      }

    // reset handler, set mirror mode
    InputHandler::instance()->reset();
    for(unsigned int j=0; j<GameApp::instance()->getScenes().size(); j++) {
      for(unsigned int i=0; i<GameApp::instance()->getScenes()[j]->Cameras().size(); i++) {
	GameApp::instance()->getScenes()[j]->Cameras()[i]->setMirrored(XMSession::instance()->mirrorMode());
      }
    }
    InputHandler::instance()->setMirrored(XMSession::instance()->mirrorMode());

  } catch(Exception &e) {
    Logger::Log(std::string("** Warning ** : failed to initialize level\n" + e.getMsg()).c_str());
    for(unsigned int i=0; i<GameApp::instance()->getScenes().size(); i++) {
      GameApp::instance()->getScenes()[i]->endLevel();
    }
    StateManager::instance()->replaceState(new StateMessageBox(NULL, GameApp::splitText(e.getMsg(), 50), UI_MSGBOX_OK));
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
    if(GameApp::instance()->getScenes().size() > 0) {
      // play music of the first world
      GameApp::instance()->playMusic(GameApp::instance()->getScenes()[0]->getLevelSrc()->Music());
    }
  }
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

  if(m_ghostDownloaded == false){
    return true;
  }

  if(m_secondInitPhaseDone == false){
    Logger::Log("begin second init phase");
    secondInitPhase();
    m_secondInitPhaseDone = true;
  }

  if(shouldBeAnimated()) {
    if(m_cameraAnim->step() == false) {
      m_playAnimation = false;
    }
  } else { /* animation has been rupted */
    m_playAnimation = false; // disable anim
    m_cameraAnim->uninit();
    StateManager::instance()->replaceState(new StatePlaying());
  }
  for(unsigned int i=0; i<GameApp::instance()->getScenes().size(); i++) {
    GameApp::instance()->getScenes()[i]->updateGameMessages();
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
    if(XMSession::instance()->enableGhosts()) {
      try {
	for(unsigned int i=0; i<GameApp::instance()->getScenes().size(); i++) {
	  pGame->addGhosts(GameApp::instance()->getScenes()[i], Theme::instance());
	}
      } catch(Exception &e) {
	/* anyway */
      }
    }
  } catch(Exception &e) {
    Logger::Log(std::string("** Warning ** : failed to initialize level\n" + e.getMsg()).c_str());
    for(unsigned int i=0; i<GameApp::instance()->getScenes().size(); i++) {
      GameApp::instance()->getScenes()[i]->endLevel();
    }
    StateManager::instance()->replaceState(new StateMessageBox(NULL, GameApp::splitText(e.getMsg(), 50), UI_MSGBOX_OK));
    return;
  }

  /* Prepare level */
  GameRenderer::instance()->prepareForNewLevel();

  /* If "preplaying" / "initial-zoom" is enabled, this is where it's done */
  // animation
  if(m_cameraAnim != NULL) {
    delete m_cameraAnim;
  }
  if(GameApp::instance()->getScenes().size() > 0) {
    GameApp::instance()->getScenes()[0]->setAutoZoomCamera();
    m_cameraAnim = new ZoomingCameraAnimation(GameApp::instance()->getScenes()[0]->getCamera(), pGame->getDrawLib(), GameApp::instance()->getScenes()[0]);
    m_cameraAnim->init();
  }

  /* display level name */
  if(m_sameLevel == false) {
    for(unsigned int i=0; i<GameApp::instance()->getScenes().size(); i++) {
      GameApp::instance()->getScenes()[i]->gameMessage(GameApp::instance()->getScenes()[i]->getLevelSrc()->Name(),
						       false,
						       PRESTART_ANIMATION_LEVEL_MSG_DURATION);
    }
  }
  
  setAutoZoom(shouldBeAnimated());
}

void StatePreplaying::executeOneCommand(std::string cmd)
{
  if(cmd == "GHOST_DOWNLOADED"){
    m_ghostDownloaded = true;
  }
  else {
    StateScene::executeOneCommand(cmd);
  }
}

bool StatePreplaying::needToDownloadGhost()
{
  if(XMSession::instance()->www() == false){
    return false;
  }

  char **v_result;
  unsigned int nrow;
  std::string res;
  std::string v_replayName;
  std::string v_fileUrl;

  v_result = xmDatabase::instance("main")->readDB("SELECT fileUrl FROM webhighscores "
			   "WHERE id_room=" + XMSession::instance()->idRoom() + " "
			   "AND id_level=\"" + xmDatabase::protectString(m_idlevel) + "\";",
			   nrow);    
  if(nrow == 0) {
    xmDatabase::instance("main")->read_DB_free(v_result);
    return false;
  }

  v_fileUrl = xmDatabase::instance("main")->getResult(v_result, 1, 0, 0);
  v_replayName = FS::getFileBaseName(v_fileUrl);
  xmDatabase::instance("main")->read_DB_free(v_result);

  /* search if the replay is already downloaded */
  return (xmDatabase::instance("main")->replays_exists(v_replayName) == false);
}
