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
#include "drawlib/DrawLib.h"
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

bool StateUpdateLevels::render()
{
  int ret = StateUpdate::render();

  DrawLib* drawLib = m_pGame->getDrawLib();


  // draw text
  m_GUI->setTextSolidColor(MAKE_COLOR(255,255,255,255));
  m_GUI->putText(drawLib->getDispWidth()/2,
		 drawLib->getDispHeight()/2,
		 m_currentOperation,
		 -0.5, -0.5);

  return ret;
}

void StateUpdateLevels::updateGUI()
{
  
}
