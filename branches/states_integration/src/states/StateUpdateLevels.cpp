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

#include "StateUpdateLevels.h"
#include "thread/UpdateLevelsThread.h"
#include "helpers/Log.h"

StateUpdateLevels::StateUpdateLevels(GameApp* pGame,
				     bool drawStateBehind,
				     bool updateStatesBehind)
  : StateUpdate(pGame, drawStateBehind, updateStatesBehind)
{
  m_pThread          = new UpdateLevelsThread();
  m_name             = "StateUpdateLevels";
  m_progress         = -1;
  m_currentOperation = "";
}

StateUpdateLevels::~StateUpdateLevels()
{
  delete m_pThread;
}

void StateUpdateLevels::enter()
{
  StateUpdate::enter();

  enterAttractMode(NO_KEY);
  m_pThread->startThread(m_pGame);
}

bool StateUpdateLevels::update()
{
  if(StateUpdate::update() == false){
    return false;
  }

  // thread finished. we leave the state.
  if(m_pThread->isThreadRunning() == false){
    m_requestForEnd = true;
    return true;
  }

  // update the frame with the thread informations
  m_progress         = m_pThread->getThreadProgress();
  m_currentOperation = m_pThread->getThreadCurrentOperation();
}


void StateUpdateLevels::updateGUI()
{
  
}
