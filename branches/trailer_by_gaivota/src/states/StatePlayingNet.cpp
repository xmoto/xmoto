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
#include "StatePreplayingNet.h"
#include "../Universe.h"
#include "StateManager.h"
#include "../net/NetClient.h"

StatePlayingNet::StatePlayingNet(Universe* i_universe, const std::string& i_id):
StatePlaying(i_universe, i_id)
{
  m_name = "StatePlayingNet";

  StateManager::instance()->registerAsObserver("NET_PREPARE_PLAYING", this);
}

StatePlayingNet::~StatePlayingNet()
{
  StateManager::instance()->unregisterAsObserver("NET_PREPARE_PLAYING", this);
}

void StatePlayingNet::abortPlaying() {
  StateScene::abortPlaying();

  if(NetClient::instance()->isConnected()) {
    NetClient::instance()->disconnect();
  }
}

void StatePlayingNet::executeOneCommand(std::string cmd, std::string args) {
  if(cmd == "NET_PREPARE_PLAYING") {
    closePlaying();
    StateManager::instance()->replaceState(new StatePreplayingNet(getId(), args, true), this->getId());
  } else {
    StatePlaying::executeOneCommand(cmd, args);
  }
}

void StatePlayingNet::enter()
{
  StatePlaying::enter();
}

void StatePlayingNet::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE)) {
    StateManager::instance()->sendAsynchronousMessage("ABORT");
  } else {
    handleControllers(i_type, i_xmkey);
    StateScene::xmKey(i_type, i_xmkey);
  }
}
