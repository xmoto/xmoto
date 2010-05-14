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

#include "UploadAllHighscoresThread.h"
#include "../helpers/Log.h"
#include "../GameText.h"
#include "../Game.h"
#include "../XMSession.h"
#include "../states/StateManager.h"
#include "../WWW.h"
#include "../VFileIO.h"

UploadAllHighscoresThread::UploadAllHighscoresThread(unsigned int i_number)
  : XMThread("UAHT")
{
  m_number     = i_number;
  m_percentage = 0;
  m_nbFiles    = 0;
}

UploadAllHighscoresThread::~UploadAllHighscoresThread()
{
}

int UploadAllHighscoresThread::realThreadFunction()
{
  std::string webRoomId;

  setThreadProgress(0);

  webRoomId = XMSession::instance()->idRoom(m_number);

  /* 1 is the main room ; don't allow full upload on it */
  if(webRoomId == "1") {
    return 0;
  }

  // update highscores
  try {
    setThreadCurrentOperation(GAMETEXT_DLHIGHSCORES);
    WebRoom *v_pWebRoom = new WebRoom(this);
    ProxySettings* pProxySettings = XMSession::instance()->proxySettings();
    std::string    webRoomUrl     = GameApp::instance()->getWebRoomURL(m_number, m_pDb);

    v_pWebRoom->setHighscoresUrl(webRoomUrl);
    v_pWebRoom->setProxy(pProxySettings);

    v_pWebRoom->update(webRoomId);
    v_pWebRoom->upgrade(webRoomId, m_pDb);
    m_pDb->updateMyHighscoresFromHighscores(XMSession::instance()->profile());
    delete v_pWebRoom;
  } catch (Exception& e) {
    LogWarning("Failed to analyse web-highscores file");   
  }

  setThreadCurrentOperation(GAMETEXT_UPLOAD_ALL_HIGHSCORES);

    char **v_result;
    unsigned int nrow;
    std::string v_previousIdLevel, v_currentIdLevel;
    std::string v_replay;
    std::string v_replayPath;
    std::string v_levelName;

    std::string query = "SELECT r.id_level, r.name, lvl.name FROM replays r "
      "LEFT OUTER JOIN webhighscores h "
      "ON (r.id_level = h.id_level AND h.id_room=" + XMSession::instance()->idRoom(m_number) + ") "
      "INNER JOIN weblevels l ON r.id_level = l.id_level "
      "INNER JOIN levels lvl ON r.id_level = lvl.id_level "
      "WHERE r.id_profile=\"" + xmDatabase::protectString(XMSession::instance()->profile()) + "\" "
      "AND r.isFinished=1 "
      "AND ( (h.id_room IS NULL) OR xm_floord(h.finishTime*100.0) > xm_floord(r.finishTime*100.0)) "
      "ORDER BY r.id_level, r.finishTime+0;";
    v_result = m_pDb->readDB(query, nrow);
    m_nbFiles = nrow;

    try {
      for (unsigned int i = 0; i<nrow; i++) {
	v_currentIdLevel = m_pDb->getResult(v_result, 3, i, 0);

	/* send only the best of the replay by level */
	if(v_previousIdLevel != v_currentIdLevel) {
	  v_previousIdLevel = v_currentIdLevel;

	  m_percentage = (int)(i*100.0/nrow);
	  setThreadProgress(m_percentage);
	  v_replay    = m_pDb->getResult(v_result, 3, i, 1);
	  v_levelName = m_pDb->getResult(v_result, 3, i, 2);
	  setThreadCurrentMicroOperation(v_replay + " (" + v_levelName + ")");

	  try {
	    bool v_msg_status_ok;
	    v_replayPath = XMFS::getUserReplaysDir() + "/" + v_replay + ".rpl";
	    FSWeb::uploadReplay(v_replayPath,
				XMSession::instance()->idRoom(m_number),
				XMSession::instance()->profile(),
				XMSession::instance()->wwwPassword(),
				XMSession::instance()->uploadHighscoreUrl(),
				this, XMSession::instance()->proxySettings(), v_msg_status_ok, m_msg);
	    if(v_msg_status_ok == false) {
	      LogError(std::string("Failed to upload " + v_replay).c_str());
	    }
	  } catch(Exception &e) {
	    LogError(std::string("Unable to upload " + v_replay).c_str());
	  }
	}
      }
      m_msg = GAMETEXT_OPERATION_COMPLETED;
    } catch(Exception &e) {
      m_msg = e.getMsg();
    }
    m_pDb->read_DB_free(v_result);
    
    return 0;
}

std::string UploadAllHighscoresThread::getMsg() const {
  return m_msg;
}

void UploadAllHighscoresThread::setTaskProgress(float p_percent) {
  setThreadProgress((int)(m_percentage + (p_percent /((float)m_nbFiles))));
}
