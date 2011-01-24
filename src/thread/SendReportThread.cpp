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

#include "SendReportThread.h"
#include "../GameText.h"
#include "../XMSession.h"
#include "../WWW.h"
#include "../helpers/Log.h"
#include "../db/xmDatabase.h"

SendReportThread::SendReportThread(const std::string& i_author, const std::string& i_msg)
  : XMThread("SRT")
{
  m_author = i_author;
  m_msg    = i_msg;
}

SendReportThread::~SendReportThread()
{
}

int SendReportThread::realThreadFunction()
{
  bool v_msg_status_ok;

  setThreadCurrentOperation(GAMETEXT_SENDING_REPORT);
  setThreadProgress(0);

  try {
    FSWeb::sendReport(m_author, m_msg,
		      DEFAULT_SENDREPORT_URL,
		      this, XMSession::instance()->proxySettings(), v_msg_status_ok, m_msg);
    if(v_msg_status_ok == false) {
      return 1;
    }
  } catch(Exception &e) {
    m_msg = GAMETEXT_UPLOAD_ERROR + std::string("\n") + e.getMsg();
    LogWarning("%s", e.getMsg().c_str());
    return 1;
  }
  setThreadProgress(100);

  return 0;
}

std::string SendReportThread::getMsg() const
{
  return m_msg;
}

void SendReportThread::setTaskProgress(float p_percent)
{
  setThreadProgress((int)p_percent);
}
