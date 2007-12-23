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
    f_curl_download_data v_data;

    Logger::Log("WWW: Checking for new or updated themes...");
    v_data.v_WebApp = this;
    v_data.v_nb_files_performed   = 0;
    v_data.v_nb_files_to_download = 1;
    FSWeb::downloadFileBz2UsingMd5(v_destinationFile, XMSession::instance()->webThemesURL(),
				   FSWeb::f_curl_progress_callback_download, &v_data, XMSession::instance()->proxySettings());
    m_pDb->webthemes_updateDB(v_destinationFile);
    StateManager::instance()->sendSynchronousMessage("UPDATE_THEMES_LISTS");
  } catch(Exception &e) {
    /* file probably doesn't exist */
    Logger::Log("** Warning ** : Failed to analyse web-themes file");
    return 1;
  }

  return 0;
}

void UpdateThemesListThread::setTaskProgress(float p_percent) {
  setThreadProgress((int)p_percent);
}
