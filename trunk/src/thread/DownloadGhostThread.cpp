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

#include "DownloadGhostThread.h"
#include "Game.h"
#include "GameText.h"
#include "states/StateManager.h"
#include "states/StateDownloadGhost.h"
#include "helpers/Log.h"

DownloadGhostThread::DownloadGhostThread(GameState* pCallingState,
					 std::string levelId)
  : XMThread()
{
  m_pWebRoom      = new WebRoom(this);
  m_pCallingState = pCallingState;
  m_msg     = "";
  m_levelId = levelId;
}

DownloadGhostThread::~DownloadGhostThread()
{
  delete m_pWebRoom;
}

int DownloadGhostThread::realThreadFunction()
{
  char **v_result;
  unsigned int nrow;
  std::string v_levelAuthor;
  std::string v_fileUrl;
  std::string v_replayName;

  v_result = m_pDb->readDB("SELECT fileUrl "
			   "FROM webhighscores WHERE id_level=\"" + 
			   xmDatabase::protectString(m_levelId) + "\" "
			   "AND id_room=" + XMSession::instance()->idRoom() + ";",
			   nrow);
  if(nrow == 0) {
    Logger::Log("nrow == 0");
    m_pDb->read_DB_free(v_result);
    return 0;
  }

  v_fileUrl     = m_pDb->getResult(v_result, 1, 0, 0);
  v_replayName  = FS::getFileBaseName(v_fileUrl);
  m_pDb->read_DB_free(v_result);

  if(m_pDb->replays_exists(v_replayName)) {
    Logger::Log("replay exists");
    ((StateDownloadGhost*)m_pCallingState)->setReplay(v_replayName);
    return 0;
  }

  if(XMSession::instance()->www() == false) {
    Logger::Log("www == false");
    return 0;
  }

  try {
    setThreadCurrentOperation(GAMETEXT_DLGHOST);
    setThreadProgress(0);

    ProxySettings* pProxySettings = XMSession::instance()->proxySettings();
    std::string    webRoomUrl     = GameApp::instance()->getWebRoomURL(m_pDb);
    std::string    webRoomName    = GameApp::instance()->getWebRoomName(m_pDb);

    m_pWebRoom->setWebsiteInfos(webRoomName, webRoomUrl, pProxySettings);

    m_pWebRoom->downloadReplay(v_fileUrl);
    GameApp::instance()->addReplay(v_replayName);
    StateManager::instance()->sendAsynchronousMessage("REPLAYS_UPDATED");
      
    setThreadProgress(100);

    /* not very nice : make a new search to be sure the replay is here */
    /* because it could have been downloaded but unplayable : for macosx for example */
    if(m_pDb->replays_exists(v_replayName) == false) {
      m_msg = GAMETEXT_FAILEDTOLOADREPLAY;
      return 1;
    }
  } catch(Exception& e) {
    m_msg = GAMETEXT_FAILEDDLREPLAY + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW;
    return 1;
  }

  ((StateDownloadGhost*)m_pCallingState)->setReplay(v_replayName);

  return 0;
}

void DownloadGhostThread::setTaskProgress(float p_percent)
{
  setThreadProgress(p_percent);
}

std::string DownloadGhostThread::getMsg() const
{
  return m_msg;
}
