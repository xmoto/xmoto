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

StatePreplaying::StatePreplaying(GameApp* pGame, const std::string i_idlevel, bool i_sameLevel):
  StateScene(pGame)
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
  MotoGame* pWorld = m_pGame->getMotoGame();

  StateScene::enter();

  GameRenderer::instance()->setShowEngineCounter(false);
  GameRenderer::instance()->setShowMinimap(false);
  GameRenderer::instance()->setShowTimePanel(false);
  GameRenderer::instance()->hideReplayHelp();

  pWorld->setDeathAnim(XMSession::instance()->enableDeadAnimation());
  pWorld->setShowGhostTimeDiff(XMSession::instance()->showGhostTimeDifference());

  try {
    pWorld->loadLevel(m_pGame->getDb(), m_idlevel);
  } catch(Exception &e) {
    Logger::Log("** Warning ** : level '%s' cannot be loaded", m_idlevel.c_str());
    char cBuf[256];
    sprintf(cBuf,GAMETEXT_LEVELCANNOTBELOADED, m_idlevel.c_str());
    StateManager::instance()->replaceState(new StateMessageBox(NULL, m_pGame, cBuf, UI_MSGBOX_OK));
    return;
  }

  if(pWorld->getLevelSrc()->isXMotoTooOld()) {
    Logger::Log("** Warning ** : level '%s' requires newer X-Moto",
		pWorld->getLevelSrc()->Name().c_str());
    
    char cBuf[256];
    sprintf(cBuf,GAMETEXT_NEWERXMOTOREQUIRED,
	    pWorld->getLevelSrc()->getRequiredVersion().c_str());
    pWorld->endLevel();
    StateManager::instance()->replaceState(new StateMessageBox(NULL, m_pGame, cBuf, UI_MSGBOX_OK));
    return;
  }

  /* Start playing right away */     
  m_pGame->initReplay();
      
  try {
    pWorld->prePlayLevel(InputHandler::instance(), m_pGame->getCurrentReplay(), true);
    pWorld->setInfos("");
	
    /* add the players */
    int v_nbPlayer = XMSession::instance()->multiNbPlayers();
    Logger::Log("Preplay level for %i player(s)", v_nbPlayer);

    m_pGame->initCameras(v_nbPlayer);
    for(int i=0; i<v_nbPlayer; i++) {
      pWorld->setCurrentCamera(i);
      pWorld->getCamera()->setPlayerToFollow(pWorld->addPlayerBiker(pWorld->getLevelSrc()->PlayerStart(),
								    DD_RIGHT,
								    Theme::instance(), Theme::instance()->getPlayerTheme(),
								    m_pGame->getColorFromPlayerNumber(i),
								    m_pGame->getUglyColorFromPlayerNumber(i),
								    XMSession::instance()->enableEngineSound()));
    }

    // if there's more camera than player (ex: 3 players and 4 cameras),
    // then, make the remaining cameras follow the first player
    if(v_nbPlayer < pWorld->getNumberCameras()){
      for(int i=v_nbPlayer; i<pWorld->getNumberCameras(); i++){
	pWorld->setCurrentCamera(i);
	pWorld->getCamera()->setPlayerToFollow(pWorld->Players()[0]);
      }
    }
    
    if(pWorld->getNumberCameras() > 1){
      // make the zoom camera follow the first player
      pWorld->setAutoZoomCamera();
      pWorld->getCamera()->setPlayerToFollow(pWorld->Players()[0]);
    }

    // reset handler, set mirror mode
    InputHandler::instance()->reset();
    for(unsigned int i=0; i<m_pGame->getMotoGame()->Cameras().size(); i++) {
      pWorld->Cameras()[i]->setMirrored(XMSession::instance()->mirrorMode());
    }
    InputHandler::instance()->setMirrored(XMSession::instance()->mirrorMode());

  } catch(Exception &e) {
    Logger::Log(std::string("** Warning ** : failed to initialize level\n" + e.getMsg()).c_str());
    pWorld->endLevel();
    StateManager::instance()->replaceState(new StateMessageBox(NULL, m_pGame, GameApp::splitText(e.getMsg(), 50), UI_MSGBOX_OK));
    return;
  }

  if(needToDownloadGhost() == true){
    StateManager::instance()->pushState(new StateDownloadGhost(m_pGame, m_idlevel));
  } else {
    m_ghostDownloaded = true;
  }

  // music
  if(m_playAnimation) {
    m_pGame->playMusic("");
  } else {
    m_pGame->playMusic(m_pGame->getMotoGame()->getLevelSrc()->Music());
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
    StateManager::instance()->replaceState(new StatePlaying(m_pGame));
  }
  m_pGame->getMotoGame()->updateGameMessages();

  return true;
}

bool StatePreplaying::render()
{
  if(m_secondInitPhaseDone == false){
    int width  = m_pGame->getDrawLib()->getDispWidth();
    int height = m_pGame->getDrawLib()->getDispHeight();

    m_pGame->getDrawLib()->drawBox(Vector2f(0,     height),
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

void StatePreplaying::keyUp(int nKey,   SDLMod mod)
{
}

void StatePreplaying::mouseDown(int nButton)
{
}

void StatePreplaying::mouseDoubleClick(int nButton)
{
}

void StatePreplaying::mouseUp(int nButton)
{
}

void StatePreplaying::secondInitPhase()
{
  MotoGame* pWorld = m_pGame->getMotoGame();

  try {
    /* add the ghosts */
    if(XMSession::instance()->enableGhosts()) {
      try {
	m_pGame->addGhosts(pWorld, Theme::instance());
      } catch(Exception &e) {
	/* anyway */
      }
    }
  } catch(Exception &e) {
    Logger::Log(std::string("** Warning ** : failed to initialize level\n" + e.getMsg()).c_str());
    pWorld->endLevel();
    StateManager::instance()->replaceState(new StateMessageBox(NULL, m_pGame, GameApp::splitText(e.getMsg(), 50), UI_MSGBOX_OK));
    return;
  }

  /* Prepare level */
  GameRenderer::instance()->prepareForNewLevel();

  /* If "preplaying" / "initial-zoom" is enabled, this is where it's done */
  // animation
  pWorld->setAutoZoomCamera();
  if(m_cameraAnim != NULL) {
    delete m_cameraAnim;
  }
  m_cameraAnim = new ZoomingCameraAnimation(pWorld->getCamera(), m_pGame->getDrawLib(), m_pGame->getMotoGame());
  m_cameraAnim->init();

  /* display level name */
  if(m_sameLevel == false) {
    pWorld->gameMessage(pWorld->getLevelSrc()->Name(),
			false,
			PRESTART_ANIMATION_LEVEL_MSG_DURATION);
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

  v_result = m_pGame->getDb()->readDB("SELECT fileUrl FROM webhighscores "
			   "WHERE id_room=" + XMSession::instance()->idRoom() + " "
			   "AND id_level=\"" + xmDatabase::protectString(m_idlevel) + "\";",
			   nrow);    
  if(nrow == 0) {
    m_pGame->getDb()->read_DB_free(v_result);
    return false;
  }

  v_fileUrl = m_pGame->getDb()->getResult(v_result, 1, 0, 0);
  v_replayName = FS::getFileBaseName(v_fileUrl);
  m_pGame->getDb()->read_DB_free(v_result);

  /* search if the replay is already downloaded */
  return (m_pGame->getDb()->replays_exists(v_replayName) == false);
}
