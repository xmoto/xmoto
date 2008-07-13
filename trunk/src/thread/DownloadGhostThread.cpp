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
#include "VFileIO.h"
#include "helpers/CmdArgumentParser.h"

DownloadGhostThread::DownloadGhostThread(std::string levelId,
					 bool i_onlyMainRoomGhost)
  : XMThread()
{
  m_pWebRoom      = new WebRoom(this);
  m_msg     = "";
  m_levelId = levelId;
  m_onlyMainRoomGhost = i_onlyMainRoomGhost;
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
  bool v_failed = false;

  for(unsigned int i=0; i<XMSession::instance()->nbRoomsEnabled(); i++) {
    if(v_failed == false) {
      if(
	 (m_onlyMainRoomGhost && i == 0)
	 ||
	 (m_onlyMainRoomGhost == false &&
	  (
	   (XMSession::instance()->ghostStrategy_BESTOFREFROOM()    && i==0) ||
	   (XMSession::instance()->ghostStrategy_BESTOFOTHERROOMS() && i!=0)
	   )
	  )
	 ) {
	v_result = m_pDb->readDB("SELECT fileUrl "
				 "FROM webhighscores WHERE id_level=\"" + 
				 xmDatabase::protectString(m_levelId) + "\" "
				 "AND id_room=" + XMSession::instance()->idRoom(i) + ";",
				 nrow);
	if(nrow != 0) {
	  v_fileUrl     = m_pDb->getResult(v_result, 1, 0, 0);
	  v_replayName  = FS::getFileBaseName(v_fileUrl);
	  
	  if(m_pDb->replays_exists(v_replayName) == false) {
	    if(XMSession::instance()->www()) {
	      try {	      
		ProxySettings* pProxySettings = XMSession::instance()->proxySettings();
		std::string    webRoomUrl     = GameApp::instance()->getWebRoomURL(i, m_pDb);
		std::string    webRoomName    = GameApp::instance()->getWebRoomName(i, m_pDb);
		
		setThreadCurrentOperation(GAMETEXT_DLGHOST + std::string(" (") + webRoomName + ")");
		setThreadProgress(0);
		
		m_pWebRoom->setWebsiteInfos(webRoomName, webRoomUrl, pProxySettings);
		m_pWebRoom->downloadReplay(v_fileUrl);
		GameApp::instance()->addReplay(v_replayName);
		
		setThreadProgress(100);
		
		/* not very nice : make a new search to be sure the replay is here */
		/* because it could have been downloaded but unplayable */
		if(m_pDb->replays_exists(v_replayName) == false) {
		  m_msg = GAMETEXT_FAILEDTOLOADREPLAY;
		  v_failed = true;
		}
	      } catch(Exception& e) {
		m_msg = GAMETEXT_FAILEDDLREPLAY + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW;
		v_failed = true;
	      }
	    }
	  }
	}
	m_pDb->read_DB_free(v_result);
      }
      if(m_onlyMainRoomGhost) {
	/* only for the main room */
	std::string args = "";
	CmdArgumentParser::instance()->addString(v_replayName, args);
	StateManager::instance()->sendAsynchronousMessage("GHOST_DOWNLOADED_REPLAY_FILE", args);
      }
    }
  }

  return v_failed ? 1 : 0;
}

void DownloadGhostThread::setTaskProgress(float p_percent)
{
  setThreadProgress((int)p_percent);
}

std::string DownloadGhostThread::getMsg() const
{
  return m_msg;
}
