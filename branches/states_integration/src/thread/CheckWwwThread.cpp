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
#include "GameText.h"
#include "Game.h"
#include "states/StateManager.h"
#include "helpers/Log.h"
#include "VFileIO.h"
#include "WWW.h"

#define DEFAULT_WEBHIGHSCORES_FILENAME "webhighscores.xml"

CheckWwwThread::CheckWwwThread(bool forceUpdate)
  : XMThread()
{
  m_forceUpdate   = forceUpdatep;
  m_pWebRoom      = new WebRoom();
  m_pWebLevels    = new WebLevels(this);
}

CheckWwwThread::~CheckWwwThread()
{
  delete m_pWebRoom();
  delete m_pWebLevels();
}

void CheckWwwThread::updateWebHighscores()
{
  Logger::Log("WWW: Checking for new highscores...");

  m_pWebRoom->update();
}

void CheckWwwThread::upgradeWebHighscores()
{
  try {
    m_pWebRoom->upgrade(m_pDb);
  } catch (Exception& e) {
    Logger::Log("** Warning ** : Failed to analyse web-highscores file");   
  }
}

void CheckWwwThread::updateWebLevels()
{
  //_SimpleMessage(GAMETEXT_DLLEVELSCHECK, &m_InfoMsgBoxRect);

  Logger::Log("WWW: Checking for new or updated levels...");

  /* Try download levels list */
  m_pWebLevels->update(m_db);
  if(m_pWebLevels->nbLevelsToGet(m_db) != 0){
    m_pGame->getStateManager()->sendAsynchronousMessage("NEW_LEVELS_TO_DOWNLOAD");
  }
}

int CheckWwwThread::realThreadFunction()
{
  if(m_pGame->getSession()->www() == true){
    ProxySettings* pProxySettings = m_pGame->getSession()->proxySettings();
    std::string    webRoomUrl     = getWebRoomURL(m_pDb);
    std::string    webRoomName    = getWebRoomName(m_pDb);
    std::string    webLevelsUrl   = m_pGame->getSession()->webLevelsUrl();

    m_pWebRoom->setWebsiteInfos(webRoomName, webRoomUrl, pProxySettings);
    m_pWebLevels->setWebsiteInfos(webLevelsUrl, pProxySettings);

    if(m_forceUpdate == true
       || m_pGame->getSession()->checkNewHighscoresAtStartup() == true){
      try {
	setThreadCurrentOperation(GAMETEXT_DLHIGHSCORES);
	setThreadProgress(0);

	updateWebHighscores();
	upgradeWebHighscores();

      } catch (Exception& e) {
	Logger::Log("** Warning ** : Failed to update web-highscores [%s]",e.getMsg().c_str());
	if(m_forceUpdate == true){
	  //notifyMsg(GAMETEXT_FAILEDDLHIGHSCORES + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
	}
      }
    }

    if(m_forceUpdate == true
       || m_pGame->getSession()->checkNewLevelsAtStartup() == true){
      try {
	setThreadCurrentOperation(GAMETEXT_DLLEVELSCHECK);
	setThreadProgress(50);

	updateWebLevels();

      } catch (Exception& e){
	Logger::Log("** Warning ** : Failed to update web-levels [%s]",e.getMsg().c_str());
	if(m_forceUpdate == true){
	  //notifyMsg(GAMETEXT_FAILEDDLHIGHSCORES + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
	}
      }
    }
  }

  setThreadProgress(100);
}

std::string CheckWwwThread::getMsg() const
{
  return m_msg;
}

void CheckWwwThread::setTaskProgress(float p_percent)
{
  setThreadProgress(p_percent);
}
