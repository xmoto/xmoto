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

#include "UploadHighscoreThread.h"
#include "helpers/Log.h"
#include "xmoto/GameText.h"
#include "xmoto/Game.h"
#include "common/XMSession.h"
#include "states/StateManager.h"
#include "common/WWW.h"
#include "common/VFileIO.h"
#include "xmoto/Replay.h"

UploadHighscoreThread::UploadHighscoreThread(const std::string& i_highscorePath)
  : XMThread("UHT")
{
  m_highscorePath = i_highscorePath;
}

UploadHighscoreThread::~UploadHighscoreThread()
{
}

int UploadHighscoreThread::realThreadFunction()
{
  bool v_msg_status_ok;
  std::string webRoomName;
  bool v_failed = false;
  int v_room_time;
  ReplayInfo* v_replayInfos;
  int v_replayTime;
  std::string v_replayLevel;
  std::string v_tmpMsg;

  m_msg = "";
  setThreadProgress(0);

  v_replayInfos = Replay::getReplayInfos(m_highscorePath);
  if(v_replayInfos == NULL) {
    LogWarning("Unable to read the replay to send");
    return 1;
  }
  v_replayTime  = v_replayInfos->finishTime;
  v_replayLevel = v_replayInfos->Level;
  delete v_replayInfos;

  for(unsigned int i=0; i<XMSession::instance()->nbRoomsEnabled(); i++) {    
    try {
      v_room_time = m_pDb->webrooms_getHighscoreTime(XMSession::instance()->idRoom(i), v_replayLevel);

      if(v_replayTime < v_room_time || v_room_time < 0) { /* upload only if it is a highscore in the room */
	
	webRoomName  = GameApp::instance()->getWebRoomName(i, m_pDb);
	setThreadCurrentOperation(GAMETEXT_UPLOADING_HIGHSCORE + std::string(" (") + webRoomName + ")");
	
	FSWeb::uploadReplay(m_highscorePath,
			    XMSession::instance()->idRoom(i),
			    XMSession::instance()->profile(),
			    XMSession::instance()->wwwPassword(),
			    XMSession::instance()->uploadHighscoreUrl(),
			    this, XMSession::instance()->proxySettings(), v_msg_status_ok, v_tmpMsg);
	if(v_msg_status_ok) {
	  m_msg = m_msg + webRoomName + ":\n" + v_tmpMsg + "\n \n";
	} else {
	  m_msg = m_msg + webRoomName + ":\n" + std::string(GAMETEXT_UPLOAD_HIGHSCORE_WEB_WARNING_BEFORE) + "\n" + v_tmpMsg + "\n \n";
	  v_failed = true;
	}
      }
    } catch(Exception &e) {
      m_msg = m_msg + webRoomName + ":\n" + GAMETEXT_UPLOAD_ERROR + std::string("\n") + v_tmpMsg + "\n \n";
      v_failed = true;
    }
  }
  return v_failed ? 1 : 0;
}

std::string UploadHighscoreThread::getMsg() const {
  return m_msg;
}

void UploadHighscoreThread::setTaskProgress(float p_percent) {
  setThreadProgress((int)p_percent);
}
