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

#include "UpdateRoomsListThread.h"
#include "common/VFileIO.h"
#include "common/WWW.h"
#include "common/XMSession.h"
#include "helpers/Log.h"
#include "states/StateManager.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"

UpdateRoomsListThread::UpdateRoomsListThread()
  : XMThread("URLT") {}

UpdateRoomsListThread::~UpdateRoomsListThread() {}

int UpdateRoomsListThread::realThreadFunction() {
  setThreadCurrentOperation(GAMETEXT_DLROOMSLISTCHECK);
  setThreadProgress(0);

  try {
    std::string v_destinationFile =
      XMFS::getUserDir(FDT_CACHE) + "/" + DEFAULT_WEBROOMS_FILENAME;

    LogInfo("WWW: Checking for rooms...");

    /* download xml file */
    FSWeb::downloadFileBz2UsingMd5(v_destinationFile,
                                   XMSession::instance()->webRoomsURL(),
                                   NULL,
                                   NULL,
                                   XMSession::instance()->proxySettings());
    setThreadProgress(90);
    m_pDb->webrooms_updateDB(FDT_CACHE, v_destinationFile);
    StateManager::instance()->sendAsynchronousMessage(
      std::string("ROOMS_UPDATED"));
  } catch (Exception &e) {
    /* file probably doesn't exist */
    LogWarning("Failed to analyse webrooms file");
    return 1;
  }

  return 0;
}
