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
#include "drawlib/DrawLib.h"

#define PRESTART_ANIMATION_LEVEL_MSG_DURATION 1.0

bool StatePreplaying::m_playAnimation = true;

StatePreplaying::StatePreplaying(GameApp* pGame, const std::string i_idlevel):
  StateScene(pGame)
{
  m_name  = "StatePreplaying";
  m_idlevel = i_idlevel;
}

StatePreplaying::~StatePreplaying()
{

}


void StatePreplaying::enter()
{
  setPlayAnimation(true); // must be removed, just for tests

  StateScene::enter();

  m_pGame->m_State = GS_PREPLAYING; // to be removed, just the time states are finished

  m_pGame->getGameRenderer()->setShowEngineCounter(false);
  m_pGame->getGameRenderer()->setShowMinimap(false);
  m_pGame->getGameRenderer()->setShowTimePanel(false);
  m_pGame->playMusic("");

  /* Initialize controls */
  m_pGame->getInputHandler()->configure(m_pGame->getUserConfig());

  try {
    m_pGame->getMotoGame()->loadLevel(m_pGame->getDb(), m_idlevel);
  } catch(Exception &e) {
    Logger::Log("** Warning ** : level '%s' cannot be loaded", m_idlevel.c_str());
    char cBuf[256];
    sprintf(cBuf,GAMETEXT_LEVELCANNOTBELOADED, m_idlevel.c_str());
    m_pGame->setState(GS_MENU);
    m_pGame->getStateManager()->replaceState(new StateMessageBox(NULL, m_pGame, cBuf, UI_MSGBOX_OK));
    return;
  }

  if(m_pGame->getMotoGame()->getLevelSrc()->isXMotoTooOld()) {
    Logger::Log("** Warning ** : level '%s' requires newer X-Moto",
		m_pGame->getMotoGame()->getLevelSrc()->Name().c_str());
    
    char cBuf[256];
    sprintf(cBuf,GAMETEXT_NEWERXMOTOREQUIRED,
	    m_pGame->getMotoGame()->getLevelSrc()->getRequiredVersion().c_str());
    m_pGame->getMotoGame()->endLevel();
    m_pGame->setState(GS_MENU);
    m_pGame->getStateManager()->replaceState(new StateMessageBox(NULL, m_pGame, cBuf, UI_MSGBOX_OK));
    return;
  }

  /* Start playing right away */     
  m_pGame->initReplay();
      
  try {
    m_pGame->getInputHandler()->reset();
    //m_InputHandler.setMirrored(m_MotoGame.getCamera()->isMirrored());
    m_pGame->getMotoGame()->prePlayLevel(m_pGame->getInputHandler(), m_pGame->getCurrentReplay(), true);
    m_pGame->getMotoGame()->setInfos("");
	
    /* add the players */
    int v_nbPlayer = m_pGame->getNumberOfPlayersToPlay();
    Logger::Log("Preplay level for %i player(s)", v_nbPlayer);

    m_pGame->initCameras(v_nbPlayer);
    for(int i=0; i<v_nbPlayer; i++) {
      m_pGame->getMotoGame()->setCurrentCamera(i);
      m_pGame->getMotoGame()->getCamera()->setPlayerToFollow(m_pGame->getMotoGame()->addPlayerBiker(m_pGame->getMotoGame()->getLevelSrc()->PlayerStart(),
												    DD_RIGHT,
												    m_pGame->getTheme(), m_pGame->getTheme()->getPlayerTheme(),
												    m_pGame->getColorFromPlayerNumber(i),
												    m_pGame->getUglyColorFromPlayerNumber(i),
												    m_pGame->getSession()->enableEngineSound()));
    }

    // if there's more camera than player (ex: 3 players and 4 cameras),
    // then, make the remaining cameras follow the first player
    if(v_nbPlayer < m_pGame->getMotoGame()->getNumberCameras()){
      for(int i=v_nbPlayer; i<m_pGame->getMotoGame()->getNumberCameras(); i++){
	m_pGame->getMotoGame()->setCurrentCamera(i);
	m_pGame->getMotoGame()->getCamera()->setPlayerToFollow(m_pGame->getMotoGame()->Players()[0]);
      }
    }
    
    if(m_pGame->getMotoGame()->getNumberCameras() > 1){
      // make the zoom camera follow the first player
      m_pGame->getMotoGame()->setCurrentCamera(m_pGame->getMotoGame()->getNumberCameras());
      m_pGame->getMotoGame()->getCamera()->setPlayerToFollow(m_pGame->getMotoGame()->Players()[0]);
    }

    /* add the ghosts */
    if(m_pGame->getSession()->enableGhosts()) {
      try {
	m_pGame->addGhosts(m_pGame->getMotoGame(), m_pGame->getTheme());
      } catch(Exception &e) {
	/* anyway */
      }
    }
  } catch(Exception &e) {
    Logger::Log(std::string("** Warning ** : failed to initialize level\n" + e.getMsg()).c_str());
    m_pGame->getMotoGame()->endLevel();
    m_pGame->setState(GS_MENU);
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
  if(m_pGame->getMotoGame()->getNumberCameras() > 1){
    m_pGame->getMotoGame()->setCurrentCamera(m_pGame->getMotoGame()->getNumberCameras());
  }

  /* display level name */
  m_pGame->getMotoGame()->gameMessage(m_pGame->getMotoGame()->getLevelSrc()->Name(),
				      false,
				      PRESTART_ANIMATION_LEVEL_MSG_DURATION);

  zoomAnimation1_init();
  
}

bool StatePreplaying::shouldBeAnimated() const {
  return m_playAnimation && m_pGame->getSession()->enableInitZoom() && m_pGame->getSession()->ugly() == false;
}

void StatePreplaying::leave()
{
}

void StatePreplaying::enterAfterPop()
{

}

void StatePreplaying::leaveAfterPush()
{

}

bool StatePreplaying::update()
{
  if(doUpdate() == false){
    return false;
  }

  //StateScene::update(); // don't update the scene in preplaying mode

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
