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
#include "../helpers/Log.h"
#include "../helpers/Text.h"
#include "../GameText.h"
#include "../Game.h"
#include "../xmscene/Camera.h"
#include "../XMSession.h"
#include "../xmscene/BikePlayer.h"
#include "../xmscene/BikeGhost.h"
#include "StateMessageBox.h"
#include "../drawlib/DrawLib.h"
#include "../CameraAnimation.h"
#include "../Universe.h"
#include "../VFileIO.h"
#include "../Renderer.h"
#include "../net/NetClient.h"
#include "../Sound.h"
#include "thread/DownloadReplaysThread.h"

#define PRESTART_ANIMATION_LEVEL_MSG_DURATION 100

StatePreplaying::StatePreplaying(const std::string i_idlevel, bool i_sameLevel):
  StateScene()
{
  m_name  = "StatePreplaying";
  m_idlevel = i_idlevel;

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
  GameApp*  pGame = GameApp::instance();
  unsigned int v_nbPlayer = XMSession::instance()->multiNbPlayers();

  m_universe =  new Universe();
  m_renderer = new GameRenderer();

  m_renderer->init(GameApp::instance()->getDrawLib(), &m_screen);

  // must be done once the renderer is initialized
  StateScene::enter();

  if(XMSession::instance()->gDebug()) {
    m_renderer->loadDebugInfo(XMSession::instance()->gDebugFile());
  }

  m_renderer->setShowEngineCounter(false);
  m_renderer->setShowMinimap(false);
  m_renderer->setShowTimePanel(false);
  m_renderer->hideReplayHelp();
  m_renderer->setShowGhostsText(false);
  m_renderer->setRenderGhostTrail(XMSession::instance()->renderGhostTrail());

  try {
    initUniverse();
  } catch(Exception &e) {
    delete m_universe;
    onLoadingFailure("Ooops");
    return;
  }  

  for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
    m_universe->getScenes()[i]->setShowGhostTimeDiff(XMSession::instance()->showGhostTimeDifference());
  }

  try {
    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->loadLevel(xmDatabase::instance("main"), m_idlevel);
    }
  } catch(Exception &e) {
    LogWarning("level '%s' cannot be loaded", m_idlevel.c_str());
    char cBuf[256];
    snprintf(cBuf, 256, GAMETEXT_LEVELCANNOTBELOADED, m_idlevel.c_str());
    delete m_universe;
    m_universe = NULL;
    onLoadingFailure(cBuf);
    return;
  }

  for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
    if(m_universe->getScenes()[i]->getLevelSrc()->isXMotoTooOld()) {
      LogWarning("level '%s' requires newer X-Moto",
		  m_universe->getScenes()[i]->getLevelSrc()->Name().c_str());
      
      char cBuf[256];
      snprintf(cBuf, 256, GAMETEXT_NEWERXMOTOREQUIRED,
	      m_universe->getScenes()[i]->getLevelSrc()->getRequiredVersion().c_str());
      
      delete m_universe;
      m_universe = NULL;
      onLoadingFailure(cBuf);
      return;
    }
  }

  try {
    preloadLevels();
    initPlayers();

    // if there's more camera than player (ex: 3 players and 4 cameras),
    // then, make the remaining cameras follow the first player          
    
    m_universe->getScenes()[0]->setCurrentCamera(0);
    if( m_universe->getScenes().size() == 1 && v_nbPlayer < m_universe->getScenes()[0]->getNumberCameras() ) {
      m_universe->getScenes()[0]->setCurrentCamera(3); 
    }
    else if(m_universe->getScenes().size() != 1 && v_nbPlayer == 3) {
      m_universe->getScenes()[0]->setCurrentCamera(1);
    }
    m_universe->getScenes()[0]->getCamera()->setPlayerToFollow(m_universe->getScenes()[0]->Players()[0]);
    m_universe->getScenes()[0]->getCamera()->setScroll(false, m_universe->getScenes()[0]->getGravity());

    // make the zoom camera follow the first player
    m_universe->getScenes()[0]->setAutoZoomCamera();
    m_universe->getScenes()[0]->getCamera()->setPlayerToFollow(m_universe->getScenes()[0]->Players()[0]);
    m_universe->getScenes()[0]->getCamera()->setScroll(false, m_universe->getScenes()[0]->getGravity());

    // reset handler, set mirror mode
    InputHandler::instance()->reset();
    for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
      for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
				m_universe->getScenes()[j]->Cameras()[i]->setMirrored(XMSession::instance()->mirrorMode());
      }
    }

  } catch(Exception &e) {
    LogWarning(std::string("failed to initialize level\n" + e.getMsg()).c_str());
    delete m_universe;
    m_universe = NULL;
    onLoadingFailure(e.getMsg());
    return;
  }

  // music
  if(m_playAnimation) {
    pGame->playGameMusic("");
  } else {
    playLevelMusic();
  }

  // preload sound
  Sound::findSample(Theme::instance()->getSound("EndOfLevel")->FilePath());
  Sound::findSample(Theme::instance()->getSound("NewHighscore")->FilePath());
  Sound::findSample(Theme::instance()->getSound("Headcrash")->FilePath());
 
  /* prepare stats */
  makeStatsStr();

  secondInitPhase();
}

void StatePreplaying::onLoadingFailure(const std::string& i_msg) {
  StateManager::instance()->replaceState(new StateMessageBox(NULL, splitText(i_msg, 50), UI_MSGBOX_OK), getStateId());
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
  StateScene::render();
  displayStats();
  
  return true;
}

void StatePreplaying::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  StateScene::xmKey(i_type, i_xmkey);

  if(i_type == INPUT_DOWN && !i_xmkey.isDirectionnel()) {
    // don't allow down key so that xmoto -l 1 works with the animation at startup : some pad give events at startup about their status
    m_playAnimation = false;
  }
}

void StatePreplaying::secondInitPhase()
{
  try {

    /* add the ghosts */
    if(XMSession::instance()->enableGhosts() && allowGhosts()) {
      try {
	m_universe->addAvailableGhosts(xmDatabase::instance("main"));
      } catch(Exception &e) {
	/* anyway */
      }
    }

    addLocalGhosts();

  if(allowGhosts() && XMSession::instance()->enableGhosts()) {
    // download requested ghosts available only via the web or remove it www is disabled
    addWebGhosts();
  }

  if(m_renderer != NULL) {
    m_renderer->prepareForNewLevel(m_universe);
  }

  } catch(Exception &e) {
    LogWarning(std::string("failed to initialize level\n" + e.getMsg()).c_str());
    closePlaying();
    onLoadingFailure(e.getMsg());
    return;
  }

  /* Prepare level */
  if(NetClient::instance()->isConnected()) {
    NetClient::instance()->startPlay(m_universe);
  }

  /* If "preplaying" / "initial-zoom" is enabled, this is where it's done */
  // animation
  if(m_cameraAnim != NULL) {
    delete m_cameraAnim;
  }
  if(m_universe != NULL && m_renderer != NULL) {
    if(m_universe->getScenes().size() > 0) {
      m_universe->getScenes()[0]->setAutoZoomCamera();
      m_cameraAnim = new ZoomingCameraAnimation(m_universe->getScenes()[0]->getCamera(), &m_screen, m_renderer,
						m_universe->getScenes()[0]);
      m_cameraAnim->init();
    }
  }

  /* display level name */
  if(m_sameLevel == false) {
    if(m_universe != NULL) {
      m_universe->getScenes()[0]->gameMessage(m_universe->getScenes()[0]->getLevelSrc()->Name(),
					      false,
					      PRESTART_ANIMATION_LEVEL_MSG_DURATION, levelID);
    }
  }
  
  setAutoZoom(shouldBeAnimated());
}

void StatePreplaying::addLocalGhosts() {
  for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
    for(unsigned j=0; j<m_universe->getScenes()[i]->RequestedGhosts().size(); j++) {
      if(m_universe->getScenes()[i]->RequestedGhosts()[j].external == false) {
	try {
	  m_universe->addGhost(m_universe->getScenes()[i], m_universe->getScenes()[i]->RequestedGhosts()[j]);
	} catch(Exception &e) {
	  // hum, ok, not nice
	}
      }
    }
  }
}

void StatePreplaying::addWebGhosts()
{
  bool v_need = false;
  bool v_ghostsRemoved = false; // if true, it means that some ghosts are not available, so, universe must be informed
  for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {

    // copy ghosts add infos because adding ghosts can remove them inside the loop !
    std::vector<GhostsAddInfos> v_requestedGhosts = m_universe->getScenes()[i]->RequestedGhosts();

    for(unsigned j=0; j<v_requestedGhosts.size(); j++) {
      if(v_requestedGhosts[j].external) {
	if(xmDatabase::instance("main")->replays_exists(v_requestedGhosts[j].name)) {
	  try {
	    m_universe->addGhost(m_universe->getScenes()[i], v_requestedGhosts[j]);
	  } catch(Exception &e) {
	    // hum, ok, not nice
	  }
	} else {
	  // to download
	  if(XMSession::instance()->www()) { // ok only if it's allowed to use www
	    StateManager::instance()->getReplayDownloaderThread()->add(v_requestedGhosts[j].url);
	    v_need = true;
	  } else {
	    m_universe->getScenes()[i]->removeRequestedGhost(v_requestedGhosts[j].name);
	    v_ghostsRemoved = true;
	  }
	}
      }
    }
  }

  if(v_ghostsRemoved) {
    m_universe->updateWaitingGhosts();
  }

  if(v_need) {
    StateManager::instance()->getReplayDownloaderThread()->doJob();
  }
}

bool StatePreplaying::allowGhosts() {
  return true;
}

