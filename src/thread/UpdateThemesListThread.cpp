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
#include "common/VFileIO.h"
#include "common/WWW.h"
#include "common/XMSession.h"
#include "helpers/Log.h"
#include "states/StateManager.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"

UpdateThemesListThread::UpdateThemesListThread()
  : XMThread("UTLT") {}

UpdateThemesListThread::~UpdateThemesListThread() {}

int UpdateThemesListThread::realThreadFunction() {
  setThreadCurrentOperation(GAMETEXT_DLTHEMESLISTCHECK);
  setThreadProgress(0);

  try {
    clearCancelAsSoonAsPossible();
    setSafeKill(true);
    WebThemes::updateThemeList(m_pDb, this);
    setSafeKill(false);
    StateManager::instance()->sendAsynchronousMessage(
      std::string("THEMES_UPDATED"));
  } catch (Exception &e) {
    /* file probably doesn't exist */
    LogWarning("Failed to analyse web-themes file");
    return 1;
  }

  return 0;
}

void UpdateThemesListThread::setTaskProgress(float p_percent) {
  setThreadProgress((int)p_percent);
}
