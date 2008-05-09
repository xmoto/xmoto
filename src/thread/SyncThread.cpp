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

#include "SyncThread.h"
#include "db/xmDatabase.h"
#include "VFileIO.h"
#include "XMSession.h"
#include "helpers/FileCompression.h"
#include "helpers/VExcept.h"
#include "helpers/Log.h"
#include "WWW.h"
#include "GameText.h"

#define SYNC_UP_TMPFILE    FS::getUserDir() + "/sync_up.xml"
#define SYNC_UP_TMPFILEBZ2 SYNC_UP_TMPFILE".bz2"

#define GAMETEXT_SYNC_UP           "Synchronisation up"
#define GAMETEXT_SYNC_DONE         "Synchronisation done successfully"

SyncThread::SyncThread() 
: XMThread() {
}

SyncThread::~SyncThread() {
}

int SyncThread::realThreadFunction() {
  bool v_msg_status_ok;

  setThreadProgress(0);

  setThreadCurrentOperation(GAMETEXT_SYNC_UP);

  /* create the .xml sync file */
  try {
    /* first, remove in case of windows */
    remove(std::string(SYNC_UP_TMPFILE).c_str());
    remove(std::string(SYNC_UP_TMPFILEBZ2).c_str());
    m_pDb->sync_buildServerFile(SYNC_UP_TMPFILE, XMSession::instance()->sitekey(), XMSession::instance()->profile());
    FileCompression::bzip2(SYNC_UP_TMPFILE, SYNC_UP_TMPFILEBZ2);
    remove(std::string(SYNC_UP_TMPFILE).c_str());
  } catch(Exception &e) {
    Logger::Log("** Warning **: %s", e.getMsg().c_str());
    remove(std::string(SYNC_UP_TMPFILEBZ2).c_str());
    m_msg = e.getMsg();
    return 1;
  }

  // upload it
  try {
    FSWeb::uploadDbSync(SYNC_UP_TMPFILEBZ2,
			XMSession::instance()->profile(),
			XMSession::instance()->wwwPassword(),
			XMSession::instance()->uploadDbSyncUrl(),
			this,
			XMSession::instance()->proxySettings(), v_msg_status_ok, m_msg);
    if(v_msg_status_ok == false) {
      remove(std::string(SYNC_UP_TMPFILEBZ2).c_str());
      return 1;
    }
  } catch(Exception &e) {
    m_msg = GAMETEXT_UPLOAD_HIGHSCORE_ERROR + std::string("\n") + e.getMsg();
    Logger::Log("** Warning **: %s", e.getMsg().c_str());
    remove(std::string(SYNC_UP_TMPFILEBZ2).c_str());
    return 1;
  }

  /* finally, remove, once synchronized */
  remove(std::string(SYNC_UP_TMPFILEBZ2).c_str());

  setThreadProgress(95);

  /* mark lines as synchronized */
  try {
    m_pDb->setSynchronized();
  } catch(Exception &e) {
    m_msg = e.getMsg();
    return 1;
  }

  setThreadCurrentOperation(GAMETEXT_SYNC_DONE);

  setThreadProgress(100);
  
  return 0;
}

std::string SyncThread::getMsg() const {
  return m_msg;
}

void SyncThread::setTaskProgress(float p_percent) {
  setThreadProgress((int)p_percent);
}
