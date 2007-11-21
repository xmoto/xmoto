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
#include "WWW.h"

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
  setThreadProgress(0);

  try {
    std::string v_destinationFile = FS::getUserDir() + "/" + DEFAULT_WEBTHEMES_FILENAME;

    Logger::Log("WWW: Checking for new or updated themes...");
    FSWeb::downloadFileBz2UsingMd5(v_destinationFile, m_pGame->getSession()->webThemesURL(), NULL, NULL, m_pGame->getProxySettings());
    setThreadProgress(90);
    m_pDb->webthemes_updateDB(v_destinationFile);
    m_pGame->getStateManager()->sendSynchronousMessage("UPDATE_THEMES_LISTS");
  } catch(Exception &e) {
    /* file probably doesn't exist */
    Logger::Log("** Warning ** : Failed to analyse web-themes file");
    return 1;
  }

  return 0;
}
