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

#include "Game.h"
#include "GameText.h"
#include "drawlib/DrawLib.h"
#include "StateUpdateRoomsList.h"
#include "StateMessageBox.h"
#include "thread/UpdateRoomsListThread.h"
#include "helpers/Log.h"

StateUpdateRoomsList::StateUpdateRoomsList(GameApp* pGame,
					   bool drawStateBehind,
					   bool updateStatesBehind)
  : StateUpdate(pGame, drawStateBehind, updateStatesBehind)
{
  m_pThread          = new UpdateRoomsListThread();
  m_name             = "StateUpdateRoomsList";
}

StateUpdateRoomsList::~StateUpdateRoomsList()
{
  delete m_pThread;
}

void StateUpdateRoomsList::enter()
{
  StateUpdate::enter();

  enterAttractMode(NO_KEY);
}

bool StateUpdateRoomsList::update()
{
  if(StateUpdate::update() == false){
    return false;
  }

  // thread finished. we leave the state.
  if(m_threadStarted == true && m_pThread->isThreadRunning() == false){
    if(m_pThread->waitForThreadEnd() == 0) {
      m_requestForEnd = true;
      m_threadStarted = false;
    } else {
      StateMessageBox* v_msgboxState = new StateMessageBox(this, m_pGame,
							   GAMETEXT_FAILEDDLROOMSLIST + std::string("\n") +
							   GAMETEXT_CHECK_YOUR_WWW, UI_MSGBOX_OK);
      v_msgboxState->setId("ERROR");
      m_pGame->getStateManager()->pushState(v_msgboxState);
    }

    return true;
  }

  if(m_threadStarted == false){
    m_pThread->startThread(m_pGame);
    m_threadStarted = true;

    Logger::Log("thread started");
  }

  if(m_threadStarted == true){

    // update the frame with the thread informations only when progress change
    // to avoid spending tooo much time waiting for mutexes.
    int progress = m_pThread->getThreadProgress();
    if(progress != m_progress){
      m_progress              = progress;
      m_currentOperation      = m_pThread->getThreadCurrentOperation();
      m_currentMicroOperation = m_pThread->getThreadCurrentMicroOperation();

      updateGUI();
    }
  }

  return true;
}

void StateUpdateRoomsList::send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  if(i_id == "ERROR") {
    m_requestForEnd = true;
  }
}
