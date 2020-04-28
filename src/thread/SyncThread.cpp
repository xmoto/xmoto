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
#include "common/VFileIO.h"
#include "common/WWW.h"
#include "common/XMSession.h"
#include "db/xmDatabase.h"
#include "helpers/FileCompression.h"
#include "helpers/Log.h"
#include "helpers/VExcept.h"
#include "states/StateManager.h"
#include "xmoto/GameText.h"

#define DEFAULT_DBSYNCUPLOAD_MSGFILE "sync_down.xml"
#define SYNC_UP_TMPFILE XMFS::getUserDir(FDT_CACHE) + "/sync_up.xml"
#define SYNC_UP_TMPFILEBZ2 SYNC_UP_TMPFILE ".bz2"

SyncThread::SyncThread()
  : XMThread("ST") {
  if (XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter(std::string("STATS_UPDATED"));
  }
}

SyncThread::~SyncThread() {}

int SyncThread::realThreadFunction() {
  bool v_msg_status_ok;
  std::string v_syncDownFile;

  setThreadProgress(0);

  setThreadCurrentOperation(GAMETEXT_SYNC_UP);

  v_syncDownFile =
    XMFS::getUserDir(FDT_CACHE) + "/" + DEFAULT_DBSYNCUPLOAD_MSGFILE;

  /* create the .xml sync file */
  try {
    /* first, remove in case of windows */
    remove(std::string(SYNC_UP_TMPFILE).c_str());
    remove(std::string(SYNC_UP_TMPFILEBZ2).c_str());
    m_pDb->sync_buildServerFile(SYNC_UP_TMPFILE,
                                XMSession::instance()->sitekey(),
                                XMSession::instance()->profile());
    FileCompression::bzip2(SYNC_UP_TMPFILE, SYNC_UP_TMPFILEBZ2);
    remove(std::string(SYNC_UP_TMPFILE).c_str());
  } catch (Exception &e) {
    LogWarning("%s", e.getMsg().c_str());
    remove(std::string(SYNC_UP_TMPFILEBZ2).c_str());
    m_msg = e.getMsg();
    return 1;
  }

  // upload it
  try {
    FSWeb::uploadDbSync(SYNC_UP_TMPFILEBZ2,
                        XMSession::instance()->profile(),
                        XMSession::instance()->wwwPassword(),
                        XMSession::instance()->sitekey(),
                        XMSession::instance()->dbSyncServer(
                          m_pDb, XMSession::instance()->profile()),
                        XMSession::instance()->uploadDbSyncUrl(),
                        this,
                        XMSession::instance()->proxySettings(),
                        v_msg_status_ok,
                        m_msg,
                        v_syncDownFile);
    if (v_msg_status_ok == false) {
      if (XMSession::instance()->debug() == false) {
        remove(std::string(SYNC_UP_TMPFILEBZ2).c_str());
      }
      return 1;
    }
  } catch (Exception &e) {
    m_msg = GAMETEXT_UPLOAD_ERROR + std::string("\n") + e.getMsg();
    LogWarning("%s", e.getMsg().c_str());
    remove(std::string(SYNC_UP_TMPFILEBZ2).c_str());
    return 1;
  }

  /* finally, remove, once synchronized */
  if (XMSession::instance()->debug() == false) {
    remove(std::string(SYNC_UP_TMPFILEBZ2).c_str());
  }

  setThreadProgress(50);

  /* mark lines as synchronized */
  try {
    m_pDb->setSynchronized();
  } catch (Exception &e) {
    m_msg = e.getMsg();
    remove(v_syncDownFile.c_str());
    return 1;
  }

  setThreadCurrentOperation(GAMETEXT_SYNC_DOWN);

  // sync down
  try {
    m_pDb->sync_updateDB(XMSession::instance()->profile(),
                         XMSession::instance()->sitekey(),
                         v_syncDownFile,
                         XMSession::instance()->dbSyncServer(
                           m_pDb, XMSession::instance()->profile()));
  } catch (Exception &e) {
    m_msg = e.getMsg();
    remove(v_syncDownFile.c_str());
    return 1;
  }

  // remove answer file
  if (XMSession::instance()->debug() == false) {
    remove(v_syncDownFile.c_str());
  }

  setThreadCurrentOperation(GAMETEXT_SYNC_DONE);
  setThreadProgress(100);
  StateManager::instance()->sendAsynchronousMessage(
    std::string("STATS_UPDATED"));

  return 0;
}

std::string SyncThread::getMsg() const {
  return m_msg;
}

void SyncThread::setTaskProgress(float p_percent) {
  setThreadProgress((int)p_percent);
}
