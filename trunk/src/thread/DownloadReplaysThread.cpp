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

#include "DownloadReplaysThread.h"
#include "../db/xmDatabase.h"
#include "../helpers/VExcept.h"
#include "../states/StateManager.h"
#include "../helpers/Log.h"
#include "../XMSession.h"
#include "../Game.h"
#include "../VFileIO.h"

DownloadReplaysThread::DownloadReplaysThread(StateManager* i_manager) : XMThread("DRT") {
    m_pWebRoom  = new WebRoom(NULL);
    m_manager   = i_manager;
    m_urlsMutex = SDL_CreateMutex();
}

DownloadReplaysThread::~DownloadReplaysThread() {
  SDL_DestroyMutex(m_urlsMutex);
  delete m_pWebRoom;
}

void DownloadReplaysThread::add(const std::string i_url) {
  SDL_LockMutex(m_urlsMutex);

  // prevent multiple add of the same url
  for(unsigned int i=0; i<m_replaysUrls.size(); i++) {
    if(m_replaysUrls[i] == i_url) {
      SDL_UnlockMutex(m_urlsMutex);
      return;
    }
  }

  m_replaysUrls.push_back(i_url);
  SDL_UnlockMutex(m_urlsMutex);
}

int DownloadReplaysThread::realThreadFunction() {
  try {
    play();
  } catch(Exception &e) {
    return 1;
  }

  return 0;
}

void DownloadReplaysThread::play() {

  SDL_LockMutex(m_urlsMutex);

  try {    
    while(m_replaysUrls.size() > 0) {
      std::string    v_replayName;

      v_replayName = XMFS::getFileBaseName(m_replaysUrls[0]);

      // prevent double download just before downloading - add() duplicate check is not enough while it's done in an other thread
      if(m_pWebRoom->downloadReplayExists(m_replaysUrls[0]) == false) {
	// dwd here
	try {	      
	  ProxySettings* pProxySettings = XMSession::instance()->proxySettings();
	  
	  m_pWebRoom->setProxy(pProxySettings);
	  m_pWebRoom->downloadReplay(m_replaysUrls[0]);
	  GameApp::instance()->addReplay(v_replayName, m_pDb);
	  
	  m_manager->sendAsynchronousMessage(std::string("REPLAY_DOWNLOADED"), v_replayName);
	  
	} catch(Exception& e) {
	  m_manager->sendAsynchronousMessage(std::string("REPLAY_FAILEDTODOWNLOAD"), v_replayName);
	  LogError("Unable to download replay : %s", e.getMsg().c_str());
	}
      }
      m_replaysUrls.erase(m_replaysUrls.begin());
    }
  } catch(Exception &e) {
    LogError("Unable to download replay");
  }    

  SDL_UnlockMutex(m_urlsMutex);
}

void DownloadReplaysThread::doJob() {
  if(isThreadRunning()) {
    // it could happend that the previous thread is not finished, but some job will not be done. Anyway, it will be not next time
    return;
  }

  startThread();
}
