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

#include "CheckWwwThread.h"
#include "../GameText.h"
#include "../Game.h"
#include "../states/StateManager.h"
#include "../helpers/Log.h"
#include "../VFileIO.h"
#include "../WWW.h"

#define DEFAULT_WEBHIGHSCORES_FILENAME "webhighscores.xml"

CheckWwwThread::CheckWwwThread(bool forceUpdate)
  : XMThread("CWT")
{
  m_forceUpdate   = forceUpdate;
  m_pWebRoom      = new WebRoom(this);
  m_pWebLevels    = new WebLevels(this);
  m_realHighscoresUpdate = false;

  if(XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter(std::string("HIGHSCORES_UPDATED"));
    StateManager::instance()->registerAsEmitter(std::string("NEW_LEVELS_TO_DOWNLOAD"));
    StateManager::instance()->registerAsEmitter(std::string("NO_NEW_LEVELS_TO_DOWNLOAD"));
  }
}

CheckWwwThread::~CheckWwwThread()
{
  delete m_pWebRoom;
  delete m_pWebLevels;
}

void CheckWwwThread::updateWebHighscores(const std::string& i_id_room)
{
  LogInfo("WWW: Checking for new highscores...");

  setSafeKill(true);
  m_pWebRoom->update(i_id_room);
  setSafeKill(false);
}

void CheckWwwThread::upgradeWebHighscores(const std::string& i_id_room)
{
  try {
    m_pWebRoom->upgrade(i_id_room, m_pDb);
    m_realHighscoresUpdate = true;
  } catch (Exception& e) {
    LogWarning("Failed to analyse web-highscores file");
  }
}

void CheckWwwThread::updateWebLevels()
{
  LogInfo("WWW: Checking for new or updated levels...");

  /* Try download levels list */
  setSafeKill(true);
  m_pWebLevels->update(m_pDb);
  setSafeKill(false);

  if(m_pWebLevels->nbLevelsToGet(m_pDb) != 0){
    StateManager::instance()->sendAsynchronousMessage(std::string("NEW_LEVELS_TO_DOWNLOAD"));
  } else {
    StateManager::instance()->sendAsynchronousMessage(std::string("NO_NEW_LEVELS_TO_DOWNLOAD"));
  }
}

bool CheckWwwThread::isNeeded() {
  return 
    XMSession::instance()->www() &&
    (XMSession::instance()->checkNewHighscoresAtStartup() || XMSession::instance()->checkNewLevelsAtStartup());
}

int CheckWwwThread::realThreadFunction()
{
  std::string webRoomId;
  std::string webRoomUrl;
  std::string webRoomName;
  std::string webLevelsUrl;
  std::string v_stolen_msg;
  ProxySettings* pProxySettings;

  if(XMSession::instance()->www() == true){
    pProxySettings = XMSession::instance()->proxySettings();

    /* check highscores */
    try {
      setThreadCurrentOperation(GAMETEXT_DLHIGHSCORES);
      setThreadProgress(0);

      for(unsigned int i=0; i<XMSession::instance()->nbRoomsEnabled(); i++) {
	if(m_forceUpdate == true
	   || XMSession::instance()->checkNewHighscoresAtStartup() == true){
	  webRoomId    = XMSession::instance()->idRoom(i);
	  webRoomUrl   = GameApp::instance()->getWebRoomURL(i, m_pDb);
	  webRoomName  = GameApp::instance()->getWebRoomName(i, m_pDb);

	  m_pWebRoom->setHighscoresUrl(webRoomUrl);
	  m_pWebRoom->setProxy(pProxySettings);

	  setThreadCurrentOperation(GAMETEXT_DLHIGHSCORES + std::string(" (") + webRoomName + ")");
	  updateWebHighscores(webRoomId);
	  upgradeWebHighscores(webRoomId);
	  setThreadProgress((100 * (i+1)) / XMSession::instance()->nbRoomsEnabled());
	}
      }
      m_pDb->updateMyHighscoresFromHighscores(XMSession::instance()->profile());

      if(m_pDb->markMyHighscoresKnownStolen(XMSession::instance()->profile(), v_stolen_msg)) {
	StateManager::instance()->sendAsynchronousMessage(std::string("MYHIGHSCORES_STOLEN"), v_stolen_msg);
      }

      if(m_realHighscoresUpdate) {
	StateManager::instance()->sendAsynchronousMessage(std::string("HIGHSCORES_UPDATED"));
      }
    } catch (Exception& e) {
      if(m_realHighscoresUpdate) {
	StateManager::instance()->sendAsynchronousMessage(std::string("HIGHSCORES_UPDATED"));
      }
      LogWarning("Failed to update web-highscores [%s]",e.getMsg().c_str());
      m_msg = GAMETEXT_FAILEDDLHIGHSCORES + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW;
      StateManager::instance()->sendAsynchronousMessage(std::string("CHECKWWWW_DONE"));
      return 1;
    }
    setThreadProgress(100);

    /* check levels */
    webLevelsUrl = XMSession::instance()->webLevelsUrl();
    m_pWebLevels->setWebsiteInfos(webLevelsUrl, pProxySettings);
    if(m_forceUpdate == true
       || XMSession::instance()->checkNewLevelsAtStartup() == true){
      try {
	setThreadCurrentOperation(GAMETEXT_DLLEVELSCHECK);
	setThreadProgress(0);

	updateWebLevels();

      } catch (Exception& e){
	LogWarning("Failed to update web-levels [%s]",e.getMsg().c_str());
	m_msg = GAMETEXT_FAILEDCHECKLEVELS + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW;
	StateManager::instance()->sendAsynchronousMessage(std::string("CHECKWWWW_DONE"));
	return 1;
      }
    }
  }

  setThreadProgress(100);
  StateManager::instance()->sendAsynchronousMessage(std::string("CHECKWWWW_DONE"));

  return 0;
}

std::string CheckWwwThread::getMsg() const
{
  return m_msg;
}

void CheckWwwThread::setTaskProgress(float p_percent)
{
  setThreadProgress((int)p_percent);
}
