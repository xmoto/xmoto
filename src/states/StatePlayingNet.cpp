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

#include "StatePlayingNet.h"
#include "StateLevelInfoViewer.h"
#include "StateManager.h"
#include "StatePreplayingNet.h"
#include "common/XMSession.h"
#include "net/NetClient.h"
#include "xmoto/Universe.h"
#include "xmscene/Level.h"

StatePlayingNet::StatePlayingNet(Universe *i_universe, GameRenderer *i_renderer)
  : StatePlaying(i_universe, i_renderer) {
  m_name = "StatePlayingNet";

  StateManager::instance()->registerAsObserver("NET_PREPARE_PLAYING", this);
}

StatePlayingNet::~StatePlayingNet() {
  StateManager::instance()->unregisterAsObserver("NET_PREPARE_PLAYING", this);
}

void StatePlayingNet::abortPlaying() {
  StateScene::abortPlaying();

  if (NetClient::instance()->isConnected()) {
    /* switch ghost mode */
    XMSession::instance()->setClientGhostMode(NETCLIENT_GHOST_MODE);
    StateManager::instance()->sendAsynchronousMessage("CLIENT_MODE_CHANGED");
    NetClient::instance()->changeMode(XMSession::instance()->clientGhostMode()
                                        ? NETCLIENT_GHOST_MODE
                                        : NETCLIENT_SLAVE_MODE);
  }
}

void StatePlayingNet::executeOneCommand(std::string cmd, std::string args) {
  if (cmd == "NET_PREPARE_PLAYING") {
    closePlaying();
    StateManager::instance()->replaceState(new StatePreplayingNet(args, true),
                                           getStateId());
  } else {
    StatePlaying::executeOneCommand(cmd, args);
  }
}

void StatePlayingNet::enter() {
  StatePlaying::enter();

  // pause immediatly to not move objects until the game starts
  Scene *v_world = m_universe->getScenes()[0];
  v_world->pause();
}

void StatePlayingNet::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (i_type == INPUT_DOWN &&
      (i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE) ||
       i_xmkey.getJoyButton() == SDL_CONTROLLER_BUTTON_B ||
       i_xmkey == (*Input::instance()->getGlobalKey(INPUT_SWITCHNETMODE)))) {
    StateManager::instance()->sendAsynchronousMessage(
      "ABORT", "", getStateId()); /* self sending */
  }

  else if (i_type == INPUT_DOWN &&
           i_xmkey == (*Input::instance()->getGlobalKey(INPUT_LEVELINFO))) {
    if (!isLockedScene()) {
      m_displayStats = true;
      StateManager::instance()->pushState(new StateLevelInfoViewer(
        m_universe->getScenes()[0]->getLevelSrc()->Id(), true, false));
    }
  }

  else {
    handleControllers(i_type, i_xmkey);
    StateScene::xmKey(i_type, i_xmkey);
  }
}
