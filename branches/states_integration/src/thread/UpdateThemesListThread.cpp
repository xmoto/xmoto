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

#include "UpdateThemesListThread.h"
#include "helpers/Log.h"
#include "GameText.h"
#include "Game.h"
#include "XMSession.h"
#include "states/StateManager.h"

UpdateThemesListThread::UpdateThemesListThread()
  : XMThread()
{
}

UpdateThemesListThread::~UpdateThemesListThread()
{
}

int UpdateThemesListThread::realThreadFunction()
{
  setThreadCurrentOperation(GAMETEXT_DLTHEMESLISTCHECK);

  SDL_Delay(1000);

  try {
    Logger::Log("WWW: Checking for new or updated themes...");
    m_pGame->getThemeChoicer()->setURL(m_pGame->getSession()->webThemesURL());
    m_pGame->getThemeChoicer()->updateFromWWW(m_pDb);
    m_pGame->getStateManager()->sendSynchronousMessage("UPDATE_THEMES_LISTS");
  } catch(Exception &e) {
    /* file probably doesn't exist */
    Logger::Log("** Warning ** : Failed to analyse web-themes file");   
  }

  return 0;
}
