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

#include "UpdateDbThread.h"
#include "helpers/Log.h"
#include "GameText.h"
#include "Game.h"
#include "states/StateManager.h"
#include "Theme.h"

UpdateDbThread::UpdateDbThread()
  : XMThread()
{
  if(XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("UPDATE_LEVELS_LISTS");
    StateManager::instance()->registerAsEmitter("UPDATE_REPLAYS_LISTS");
    StateManager::instance()->registerAsEmitter("UPDATE_THEMES_LISTS");
  }
}

UpdateDbThread::~UpdateDbThread()
{
}

int UpdateDbThread::realThreadFunction()
{
  setThreadCurrentOperation(GAMETEXT_RELOADINGLEVELS);
  // initialize to -1, so we put it to 0 so that the current operation
  // text is update in the state
  setThreadProgress(0);
  LevelsManager::instance()->reloadLevelsFromLvl(m_pDb, this);
  StateManager::instance()->sendAsynchronousMessage("UPDATE_LEVELS_LISTS");

  setThreadCurrentOperation(GAMETEXT_RELOADINGREPLAYS);
  GameApp::instance()->initReplaysFromDir(m_pDb, this);
  StateManager::instance()->sendAsynchronousMessage("UPDATE_REPLAYS_LISTS");
  
  setThreadCurrentOperation(GAMETEXT_RELOADINGTHEMES);
  ThemeChoicer::initThemesFromDir(m_pDb);
  StateManager::instance()->sendAsynchronousMessage("UPDATE_THEMES_LISTS");

  return 0;
}

void UpdateDbThread::loadLevelHook(std::string i_level,
				       int i_percentage)
{
  setThreadProgress(i_percentage);
  setThreadCurrentMicroOperation(i_level);
}

void UpdateDbThread::loadReplayHook(std::string i_replay,
					int i_percentage)
{
  setThreadProgress(i_percentage);
  setThreadCurrentMicroOperation(i_replay);
}
