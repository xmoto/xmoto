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

#include "UpgradeLevelsThread.h"
#include "Game.h"
#include "helpers/Log.h"
#include "GameText.h"
#include "states/StateManager.h"
#include "states/StateUpgradeLevels.h"

UpgradeLevelsThread::UpgradeLevelsThread(GameState* pCallingState)
  : XMThread()
{
  m_pWebLevels    = new WebLevels(this);
  m_pCallingState = pCallingState;
  m_updateAutomaticallyLevels = false;
  m_msg = "";
}

UpgradeLevelsThread::~UpgradeLevelsThread()
{
  delete m_pWebLevels;
}

void UpgradeLevelsThread::setBeingDownloadedInformation(const std::string &p_information, bool p_isNew)
{
  Logger::Log(("setBeingDownloadedInformation" + p_information).c_str());
  setThreadCurrentMicroOperation(p_information);
}

void UpgradeLevelsThread::readEvents()
{
  if(m_askThreadToEnd == true){
    setCancelAsSoonAsPossible();
  }
}

bool UpgradeLevelsThread::shouldLevelBeUpdated(const std::string &LevelID)
{
  if(m_updateAutomaticallyLevels == true){
    return true;
  }

  /* Hmm... ask user whether this level should be updated */
  char **v_result;
  unsigned int nrow;
  std::string v_levelName;
  std::string v_levelFileName;

  v_result = m_pDb->readDB("SELECT name, filepath "
			   "FROM levels "
			   "WHERE id_level=\"" + xmDatabase::protectString(LevelID) + "\";",
			   nrow);
  if(nrow != 1) {
    m_pDb->read_DB_free(v_result);
    return true;
  }

  v_levelName     = m_pDb->getResult(v_result, 2, 0, 0);
  v_levelFileName = m_pDb->getResult(v_result, 2, 0, 1);
  m_pDb->read_DB_free(v_result);

  ((StateUpgradeLevels*)m_pCallingState)->setCurrentUpdatedLevel(v_levelName);

  // message box
  StateManager::instance()->sendAsynchronousMessage("ASKINGLEVELUPDATE");
  sleepThread();

  if(m_wakeUpInfos == "NO"){
    return false;
  }
  else if(m_wakeUpInfos == "YES"){
    return true;
  }
  else if(m_wakeUpInfos == "YES_FOR_ALL"){
    m_updateAutomaticallyLevels = true;
    return true;
  }
  else{
    return false;
  }
}

int UpgradeLevelsThread::realThreadFunction()
{
  /* Check for extra levels */
  try {
    setThreadCurrentOperation(GAMETEXT_CHECKINGFORLEVELS);
    setThreadProgress(0);
      
    ProxySettings* pProxySettings = XMSession::instance()->proxySettings();
    std::string    webLevelsUrl   = XMSession::instance()->webLevelsUrl();
    m_pWebLevels->setWebsiteInfos(webLevelsUrl, pProxySettings);

    Logger::Log("WWW: Checking for new or updated levels...");

    clearCancelAsSoonAsPossible();

    m_pWebLevels->update(m_pDb, XMSession::instance()->useCrappyPack());

    int nULevels=0;
    nULevels = m_pWebLevels->nbLevelsToGet(m_pDb);

    Logger::Log("WWW: %d new or updated level%s found",
		nULevels,
		(nULevels ==1 ) ? "" : "s");

    if(nULevels == 0) {
      m_msg = GAMETEXT_NONEWLEVELS;
      return 1;
    }
    else {
      StateManager::instance()->sendAsynchronousMessage("NEW_LEVELS_TO_DOWNLOAD");
      StateManager::instance()->sendAsynchronousMessage("NEWLEVELAVAILABLE");
    }
  } catch (Exception& e) {
    Logger::Log("** Warning ** : Unable to check for extra levels [%s]", e.getMsg().c_str());
    m_msg = GAMETEXT_FAILEDCHECKLEVELS + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW;

    return 1;
  }

  setThreadProgress(100);

  // sleep while we don't have the user response
  sleepThread();

  if(m_askThreadToEnd == true){
    return 0;
  }

  setThreadCurrentOperation(GAMETEXT_DLLEVELS);
  setThreadProgress(0);

  try {                  
    Logger::Log("WWW: Downloading levels...");

    clearCancelAsSoonAsPossible();

    setThreadCurrentMicroOperation("");
    m_pWebLevels->upgrade(m_pDb);
  }
  catch(Exception& e) {
    m_msg = GAMETEXT_FAILEDDLLEVELS + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW;
    Logger::Log("** Warning ** : Unable to download extra levels [%s]",e.getMsg().c_str());

    return 1;
  }

  if(m_askThreadToEnd == true){
    return 0;
  }

  /* Got some new levels... load them! */
  Logger::Log("Loading new and updated levels...");

  setThreadCurrentOperation(GAMETEXT_LOADNEWLEVELS);
  setThreadProgress(0);

  LevelsManager::instance()->updateLevelsFromLvl(m_pWebLevels->getNewDownloadedLevels(),
						 m_pWebLevels->getUpdatedDownloadedLevels(),
						 this, m_pDb);

  /* Update level lists */
  StateManager::instance()->sendAsynchronousMessage("NO_NEW_LEVELS_TO_DOWNLOAD");
  StateManager::instance()->sendAsynchronousMessage("LEVELS_UPDATED");

  return 0;
}

void UpgradeLevelsThread::setTaskProgress(float p_percent)
{
  setThreadProgress((int)p_percent);
}

std::string UpgradeLevelsThread::getMsg() const
{
  return m_msg;
}
