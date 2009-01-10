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

#include "StateWaitServerInstructions.h"
#include "../net/NetClient.h"
#include "StateManager.h"
#include "StatePreplayingNet.h"

StateWaitServerInstructions::StateWaitServerInstructions():
GameState(true, false, true, true)
{
  m_name             = "StateWaitServerInstructions";

  StateManager::instance()->registerAsObserver("CLIENT_DISCONNECTED_BY_ERROR", this);
  StateManager::instance()->registerAsObserver("NET_PREPARE_PLAYING", this);
}

StateWaitServerInstructions::~StateWaitServerInstructions()
{
  StateManager::instance()->unregisterAsObserver("CLIENT_DISCONNECTED_BY_ERROR", this);
  StateManager::instance()->unregisterAsObserver("NET_PREPARE_PLAYING", this);
}

bool StateWaitServerInstructions::render() {
  GameState::render();

  return true;
}

void StateWaitServerInstructions::executeOneCommand(std::string cmd, std::string args) {
  if(cmd == "CLIENT_DISCONNECTED_BY_ERROR") {
    m_requestForEnd = true;
  } else if(cmd == "NET_PREPARE_PLAYING") {
    StateManager::instance()->replaceState(new StatePreplayingNet(args, true));
  } else {
    GameState::executeOneCommand(cmd, args);
  }
}

void StateWaitServerInstructions::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE)) {
    /* quit this state */
    m_requestForEnd = true;
    if(NetClient::instance()->isConnected()) {
      NetClient::instance()->disconnect();
    }
  }

  else {
    GameState::xmKey(i_type, i_xmkey);
  }

}
