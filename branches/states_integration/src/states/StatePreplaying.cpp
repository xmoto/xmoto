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

#define PRESTART_ANIMATION_MARGIN_SIZE 5
#define PRESTART_ANIMATION_TIME 2.0
#define PRESTART_ANIMATION_CURVE 3.0
#define PRESTART_ANIMATION_LEVEL_MSG_DURATION 1.0
/* logf(PRESTART_ANIMATION_CURVE + 1.0) = 1.386294361*/
#define LOGF_PRE_ANIM_TIME_ADDED_ONE 1.386294361

bool StatePreplaying::m_playAnimation = true;

StatePreplaying::StatePreplaying(GameApp* pGame, const std::string i_idlevel):
  StateScene(pGame)
{
  m_name  = "StatePreplaying";
  m_idlevel = i_idlevel;

  m_secondInitPhaseDone = false;
  m_ghostDownloaded     = false;
}

StatePreplaying::~StatePreplaying()
{

}


void StatePreplaying::enter()
{
  MotoGame* pWorld = m_pGame->getMotoGame();

  StateScene::enter();

  m_pGame->getGameRenderer()->setShowEngineCounter(false);
  m_pGame->getGameRenderer()->setShowMinimap(false);
  m_pGame->getGameRenderer()->setShowTimePanel(false);
  m_pGame->getGameRenderer()->hideReplayHelp();
  m_pGame->playMusic("");
  pWorld->setDeathAnim(m_pGame->getSession()->enableDeadAnimation());

  try {
    pWorld->loadLevel(m_pGame->getDb(), m_idlevel);
  } catch(Exception &e) {
    Logger::Log("** Warning ** : level '%s' cannot be loaded", m_idlevel.c_str());
    char cBuf[256];
    sprintf(cBuf,GAMETEXT_LEVELCANNOTBELOADED, m_idlevel.c_str());
    m_pGame->getStateManager()->replaceState(new StateMessageBox(NULL, m_pGame, cBuf, UI_MSGBOX_OK));
    return;
  }

  if(pWorld->getLevelSrc()->isXMotoTooOld()) {
    Logger::Log("** Warning ** : level '%s' requires newer X-Moto",
		pWorld->getLevelSrc()->Name().c_str());
    
    char cBuf[256];
    sprintf(cBuf,GAMETEXT_NEWERXMOTOREQUIRED,
	    pWorld->getLevelSrc()->getRequiredVersion().c_str());
    pWorld->endLevel();
    m_pGame->getStateManager()->replaceState(new StateMessageBox(NULL, m_pGame, cBuf, UI_MSGBOX_OK));
    return;
  }

  /* Start playing right away */     
  m_pGame->initReplay();
      
  try {
    m_pGame->getInputHandler()->reset();
    //m_InputHandler.setMirrored(m_MotoGame.getCamera()->isMirrored());
    pWorld->prePlayLevel(m_pGame->getInputHandler(), m_pGame->getCurrentReplay(), true);
    pWorld->setInfos("");
	
    /* add the players */
    int v_nbPlayer = m_pGame->getSession()->multiNbPlayers();
    Logger::Log("Preplay level for %i player(s)", v_nbPlayer);

    m_pGame->initCameras(v_nbPlayer);
    for(int i=0; i<v_nbPlayer; i++) {
      pWorld->setCurrentCamera(i);
      pWorld->getCamera()->setPlayerToFollow(pWorld->addPlayerBiker(pWorld->getLevelSrc()->PlayerStart(),
								    DD_RIGHT,
								    m_pGame->getTheme(), m_pGame->getTheme()->getPlayerTheme(),
								    m_pGame->getColorFromPlayerNumber(i),
								    m_pGame->getUglyColorFromPlayerNumber(i),
								    m_pGame->getSession()->enableEngineSound()));
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
  } catch(Exception &e) {
    Logger::Log(std::string("** Warning ** : failed to initialize level\n" + e.getMsg()).c_str());
    pWorld->endLevel();
    m_pGame->getStateManager()->replaceState(new StateMessageBox(NULL, m_pGame, GameApp::splitText(e.getMsg(), 50), UI_MSGBOX_OK));
    return;
  }

  if(needToDownloadGhost() == true){
    m_pGame->getStateManager()->pushState(new StateDownloadGhost(m_pGame, m_idlevel));
  } else {
    m_ghostDownloaded = true;
  }
}

bool StatePreplaying::shouldBeAnimated() const {
  return m_playAnimation && m_pGame->getSession()->enableInitZoom() && m_pGame->getSession()->ugly() == false;
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
    Logger::Log("ghostdownloaded");
    return true;
  }

  if(m_secondInitPhaseDone == false){
    Logger::Log("begin second init phase");
    secondInitPhase();
    m_secondInitPhaseDone = true;
  }

  if(shouldBeAnimated()) {
    if(zoomAnimation1_step() == false) {
      setPlayAnimation(false); // disable anim
    }
  } else { /* animation has been rupted */
    setPlayAnimation(false); // disable anim
    zoomAnimation1_abort();
    m_pGame->getStateManager()->replaceState(new StatePlaying(m_pGame));
  }
  m_pGame->getMotoGame()->updateGameMessages();

  return true;
}

bool StatePreplaying::render()
{
  if(m_secondInitPhaseDone == false){
    return false;
  }

  StateScene::render();
  return true;
}

void StatePreplaying::keyDown(int nKey, SDLMod mod,int nChar)
{
  setPlayAnimation(false);
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

void StatePreplaying::setPlayAnimation(bool i_value) {
  m_playAnimation = i_value;
}

void StatePreplaying::zoomAnimation1_init() {
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

bool StatePreplaying::zoomAnimation1_step() {
  if(GameApp::getXMTime() > m_fPrePlayStartTime + static_time + PRESTART_ANIMATION_TIME) {
    return false;
  }
  if(GameApp::getXMTime() > m_fPrePlayStartTime + static_time){
    float zx, zy, zz;

    zz = logf(PRESTART_ANIMATION_CURVE * ((PRESTART_ANIMATION_TIME + static_time - GameApp::getXMTime() + m_fPrePlayStartTime) / (PRESTART_ANIMATION_TIME)) + 1.0) / LOGF_PRE_ANIM_TIME_ADDED_ONE * (m_fPrePlayStartInitZoom - m_zoomU);
    
    m_pGame->getMotoGame()->getCamera()->setZoom(m_fPrePlayStartInitZoom - zz);
    
    zx = (PRESTART_ANIMATION_TIME + static_time - GameApp::getXMTime() + m_fPrePlayStartTime)
      / (PRESTART_ANIMATION_TIME) 
      * (m_fPrePlayStartCameraX - m_fPrePlayCameraLastX);
    zy =  (PRESTART_ANIMATION_TIME + static_time - GameApp::getXMTime() + m_fPrePlayStartTime)
      / (PRESTART_ANIMATION_TIME) 
      * (m_fPrePlayStartCameraY - m_fPrePlayCameraLastY);
    
    m_pGame->getMotoGame()->getCamera()->setCameraPosition(m_fPrePlayStartCameraX-zx, m_fPrePlayStartCameraY-zy);
  } else {
    float zx,zy;
    
    m_pGame->getMotoGame()->getCamera()->setZoom(m_zoomU);
    
    zx  = (static_time - GameApp::getXMTime() + m_fPrePlayStartTime) / (static_time) 
      * (m_fPreCameraStartX - m_fPreCameraFinalX);
    
    zy = (static_time - GameApp::getXMTime() + m_fPrePlayStartTime) / (static_time) 
      * (m_fPreCameraStartY - m_fPreCameraFinalY);
    
    m_pGame->getMotoGame()->getCamera()->setCameraPosition( m_fPreCameraStartX  - zx, m_fPreCameraStartY - zy);
    
    m_fPrePlayCameraLastX= m_fPreCameraStartX - zx;
    m_fPrePlayCameraLastY= m_fPreCameraStartY - zy;
  }
  return true;
}

void StatePreplaying::zoomAnimation1_abort() {
  m_pGame->getMotoGame()->getCamera()->setZoom(m_fPrePlayStartInitZoom); // because the man can change ugly mode while the animation
  m_pGame->getMotoGame()->getCamera()->setCameraPosition(m_fPrePlayStartCameraX, m_fPrePlayStartCameraY);
}

void StatePreplaying::secondInitPhase()
{
  MotoGame* pWorld = m_pGame->getMotoGame();

  try {
    /* add the ghosts */
    if(m_pGame->getSession()->enableGhosts()) {
      try {
	m_pGame->addGhosts(pWorld, m_pGame->getTheme());
      } catch(Exception &e) {
	/* anyway */
      }
    }
  } catch(Exception &e) {
    Logger::Log(std::string("** Warning ** : failed to initialize level\n" + e.getMsg()).c_str());
    pWorld->endLevel();
    m_pGame->getStateManager()->replaceState(new StateMessageBox(NULL, m_pGame, GameApp::splitText(e.getMsg(), 50), UI_MSGBOX_OK));
    return;
  }

  /* Prepare level */
  m_pGame->getGameRenderer()->prepareForNewLevel();

  /* go directly to playing */
  if(m_playAnimation == false) {
    m_pGame->getStateManager()->replaceState(new StatePlaying(m_pGame)); 
  }

  /* If "preplaying" / "initial-zoom" is enabled, this is where it's done */
  pWorld->setAutoZoomCamera();

  /* display level name */
  pWorld->gameMessage(pWorld->getLevelSrc()->Name(),
		      false,
		      PRESTART_ANIMATION_LEVEL_MSG_DURATION);

  zoomAnimation1_init();
  
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
  char **v_result;
  unsigned int nrow;
  std::string res;
  std::string v_replayName;
  std::string v_fileUrl;

  v_result = m_pGame->getDb()->readDB("SELECT fileUrl FROM webhighscores "
			   "WHERE id_room=" + m_pGame->getSession()->idRoom() + " "
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
