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

#include "StatePreplayingNet.h"
#include "StatePlayingNet.h"
#include "StateManager.h"
#include "common/XMSession.h"
#include "xmoto/Universe.h"
#include "helpers/Log.h"
#include "xmscene/Camera.h"
#include "xmscene/Level.h"
#include "xmoto/Game.h"
#include "xmscene/BikePlayer.h"
#include "net/NetClient.h"
#include "StateWaitServerInstructions.h"
#include "StateMessageBox.h"
#include "helpers/Text.h"
#include "xmoto/SysMessage.h"

StatePreplayingNet::StatePreplayingNet(const std::string i_idlevel, bool i_sameLevel)
: StatePreplaying(i_idlevel, i_sameLevel) {
    m_name  = "StatePreplayingNet";

    StateManager::instance()->registerAsObserver("NET_PREPARE_PLAYING", this);
}

StatePreplayingNet::~StatePreplayingNet() {
  StateManager::instance()->unregisterAsObserver("NET_PREPARE_PLAYING", this);
}

void StatePreplayingNet::abortPlaying() {
  StateScene::abortPlaying();

  if(NetClient::instance()->isConnected()) {
    /* switch ghost mode */
    XMSession::instance()->setClientGhostMode(NETCLIENT_GHOST_MODE);
    NetClient::instance()->changeMode(XMSession::instance()->clientGhostMode() ? NETCLIENT_GHOST_MODE : NETCLIENT_SLAVE_MODE);
  }
}

void StatePreplayingNet::initUniverse() {
  m_universe->initPlay(&m_screen, 1, 1);
}

void StatePreplayingNet::preloadLevels() {
  m_universe->getScenes()[0]->prePlayLevel(NULL, false);
  m_universe->getScenes()[0]->setInfos("");
}

void StatePreplayingNet::initPlayers() {
  GameApp*  pGame  = GameApp::instance();

  Scene* v_world = m_universe->getScenes()[0];
    
  v_world->setCurrentCamera(0);
  v_world->getCamera()->setPlayerToFollow(
		  v_world->addPlayerNetClient(v_world->getLevelSrc()->PlayerStart(), DD_RIGHT,
					      Theme::instance(), Theme::instance()->getPlayerTheme(),
					      pGame->getColorFromPlayerNumber(0), pGame->getUglyColorFromPlayerNumber(0),
					      true));
  v_world->getCamera()->setScroll(false, v_world->getGravity());
}

void StatePreplayingNet::runPlaying() {
  StateManager::instance()->replaceState(new StatePlayingNet(m_universe, m_renderer), getStateId());
}

void StatePreplayingNet::executeOneCommand(std::string cmd, std::string args) {
  if(cmd == "NET_PREPARE_PLAYING") {
    closePlaying();
    StateManager::instance()->replaceState(new StatePreplayingNet(args, true), getStateId());
  } else {
    StateScene::executeOneCommand(cmd, args);
  }
}

bool StatePreplayingNet::allowGhosts() {
  return false;
}

void StatePreplayingNet::onLoadingFailure(const std::string& i_msg) {
  StateManager::instance()->replaceState(new StateWaitServerInstructions(), getStateId());

  // display a simple error to not break the states order
  SysMessage::instance()->displayError(i_msg);
}
