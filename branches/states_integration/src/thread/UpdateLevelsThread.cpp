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

#include "UpdateLevelsThread.h"
#include "helpers/Log.h"
#include "VCommon.h"
#include "GameText.h"
#include "Game.h"
#include "states/StateManager.h"

UpdateLevelsThread::UpdateLevelsThread()
  : XMThread()
{
}

UpdateLevelsThread::~UpdateLevelsThread()
{
}

int UpdateLevelsThread::realThreadFunction()
{
  /*
  for(int i=0; i<10; i++){
    Logger::Log("UpdateLevelsThread::realThreadFunction %d", i);
    SDL_Delay(500);
  }
  */

  //_SimpleMessage(GAMETEXT_RELOADINGLEVELS, &m_InfoMsgBoxRect);
  m_currentOperation = GAMETEXT_RELOADINGLEVELS;

  m_pGame->getLevelsManager()->reloadLevelsFromLvl(m_pGame->getDb(), m_pGame);
  m_pGame->getStateManager()->sendSynchronousMessage("UPDATE_LEVELS_LISTS");

  //_SimpleMessage(GAMETEXT_RELOADINGREPLAYS, &m_InfoMsgBoxRect);
  m_currentOperation = GAMETEXT_RELOADINGREPLAYS;

  m_pGame->initReplaysFromDir();
  m_pGame->getStateManager()->sendSynchronousMessage("UPDATE_REPLAYS_LISTS");
  
  m_currentOperation = "Reloading themes";

  m_pGame->getThemeChoicer()->initThemesFromDir(m_pGame->getDb());
  m_pGame->getStateManager()->sendSynchronousMessage("UPDATE_THEMES_LISTS");

  return 0;
}
