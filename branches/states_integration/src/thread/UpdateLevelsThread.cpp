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
  setThreadCurrentOperation(GAMETEXT_RELOADINGLEVELS);
  // initialize to -1, so we put it to 0 so that the current operation
  // text is update in the state
  setThreadProgress(0);
  m_pGame->getLevelsManager()->reloadLevelsFromLvl(m_pDb, this);
  m_pGame->getStateManager()->sendSynchronousMessage("UPDATE_LEVELS_LISTS");

  setThreadCurrentOperation(GAMETEXT_RELOADINGREPLAYS);
  m_pGame->initReplaysFromDir(m_pDb, this);
  m_pGame->getStateManager()->sendSynchronousMessage("UPDATE_REPLAYS_LISTS");
  
  setThreadCurrentOperation(GAMETEXT_RELOADINGTHEMES);
  m_pGame->getThemeChoicer()->initThemesFromDir(m_pDb);
  m_pGame->getStateManager()->sendSynchronousMessage("UPDATE_THEMES_LISTS");

  return 0;
}

void UpdateLevelsThread::loadLevelHook(std::string i_level,
				       int i_percentage)
{
  setThreadProgress(i_percentage);
  setThreadCurrentMicroOperation(i_level);
}

void UpdateLevelsThread::loadReplayHook(std::string i_replay,
					int i_percentage)
{
  setThreadProgress(i_percentage);
  setThreadCurrentMicroOperation(i_replay);
}
