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
#include "GameText.h"
#include "Game.h"
#include "XMSession.h"
#include "states/StateManager.h"
#include "WWW.h"

UploadHighscoreThread::UploadHighscoreThread(const std::string& i_highscorePath)
  : XMThread()
{
  m_highscorePath = i_highscorePath;
}

UploadHighscoreThread::~UploadHighscoreThread()
{
}

int UploadHighscoreThread::realThreadFunction()
{
  setThreadCurrentOperation(GAMETEXT_UPLOADING_HIGHSCORE);
  setThreadProgress(0);

  try {
    bool v_msg_status_ok;
    
    FSWeb::uploadReplay(m_highscorePath,
			XMSession::instance()->idRoom(),
			XMSession::instance()->uploadLogin(),
			XMSession::instance()->uploadPassword(),
			XMSession::instance()->uploadHighscoreUrl(),
			this, XMSession::instance()->proxySettings(), v_msg_status_ok, m_msg);
    if(v_msg_status_ok) {
      return 0;
    } else {
      m_msg = std::string(GAMETEXT_UPLOAD_HIGHSCORE_WEB_WARNING_BEFORE) + "\n" + m_msg;
      return 1;
    }
  } catch(Exception &e) {
    m_msg = GAMETEXT_UPLOAD_HIGHSCORE_ERROR + std::string("\n") + m_msg;
    return 1;
  }
}

std::string UploadHighscoreThread::getMsg() const {
  return m_msg;
}

void UploadHighscoreThread::setTaskProgress(float p_percent) {
  setThreadProgress(p_percent);
}
