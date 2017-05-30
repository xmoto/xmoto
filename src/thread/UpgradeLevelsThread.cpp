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
#include "helpers/CmdArgumentParser.h"
#include "helpers/Log.h"
#include "states/StateManager.h"
#include "states/StateUpgradeLevels.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"

UpgradeLevelsThread::UpgradeLevelsThread(const std::string &i_id_theme,
                                         bool i_loadMainLayerOnly,
                                         bool i_updateAutomaticallyLevels)
  : XMThread("ULT") {
  m_pWebLevels = new WebLevels(this);
  m_updateAutomaticallyLevels = i_updateAutomaticallyLevels;
  m_msg = "";
  m_id_theme = i_id_theme;
  m_nb_levels = -1;
  m_loadMainLayerOnly = i_loadMainLayerOnly;
}

UpgradeLevelsThread::~UpgradeLevelsThread() {
  delete m_pWebLevels;
}

void UpgradeLevelsThread::setBeingDownloadedInformation(
  const std::string &p_information,
  bool p_isNew) {
  setThreadCurrentMicroOperation(p_information);
}

void UpgradeLevelsThread::setNbLevels(unsigned int i_nb_levels) {
  m_nb_levels = i_nb_levels;
}

bool UpgradeLevelsThread::shouldLevelBeUpdated(const std::string &LevelID) {
  if (m_updateAutomaticallyLevels == true) {
    return true;
  }

  /* Hmm... ask user whether this level should be updated */
  char **v_result;
  unsigned int nrow;
  std::string v_levelName;
  std::string v_levelFileName;

  v_result = m_pDb->readDB("SELECT name, filepath "
                           "FROM levels "
                           "WHERE id_level=\"" +
                             xmDatabase::protectString(LevelID) + "\";",
                           nrow);
  if (nrow != 1) {
    m_pDb->read_DB_free(v_result);
    return true;
  }

  v_levelName = m_pDb->getResult(v_result, 2, 0, 0);
  v_levelFileName = m_pDb->getResult(v_result, 2, 0, 1);
  m_pDb->read_DB_free(v_result);

  std::string args = "";
  CmdArgumentParser::instance()->addString(v_levelName, args);

  // message box
  if (StateManager::exists()) {
    StateManager::instance()->sendAsynchronousMessage(
      std::string("ASKINGLEVELUPDATE"), args);
  }
  sleepThread();

  if (m_wakeUpInfos == "NO") {
    return false;
  } else if (m_wakeUpInfos == "YES") {
    return true;
  } else if (m_wakeUpInfos == "YES_FOR_ALL") {
    m_updateAutomaticallyLevels = true;
    return true;
  } else {
    return false;
  }
}

int UpgradeLevelsThread::realThreadFunction() {
  int v_exit_code = 0;
  bool v_shouldBeLoadedInBase = false;

  /* Check for extra levels */
  try {
    setThreadCurrentOperation(GAMETEXT_CHECKINGFORLEVELS);
    setThreadProgress(0);

    ProxySettings *pProxySettings = XMSession::instance()->proxySettings();
    std::string webLevelsUrl = XMSession::instance()->webLevelsUrl();
    m_pWebLevels->setWebsiteInfos(webLevelsUrl, pProxySettings);

    LogInfo("WWW: Checking for new or updated levels...");

    clearCancelAsSoonAsPossible();

    setSafeKill(true);
    m_pWebLevels->update(m_pDb);
    setSafeKill(false);

    int nULevels = 0;
    nULevels = m_pWebLevels->nbLevelsToGet(m_pDb);

    LogInfo("WWW: %d new or updated level%s found",
            nULevels,
            (nULevels == 1) ? "" : "s");

    if (nULevels == 0) {
      m_msg = GAMETEXT_NONEWLEVELS;
      return 2 /* 2 for no new levels to update */;
    } else {
      if (StateManager::exists()) {
        StateManager::instance()->sendAsynchronousMessage(
          std::string("NEW_LEVELS_TO_DOWNLOAD"));
        StateManager::instance()->sendAsynchronousMessage(
          std::string("NEWLEVELAVAILABLE"));
      }
    }
  } catch (Exception &e) {
    LogWarning("Unable to check for extra levels [%s]", e.getMsg().c_str());
    m_msg =
      GAMETEXT_FAILEDCHECKLEVELS + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW;

    return 1;
  }

  setThreadProgress(100);

  if (m_updateAutomaticallyLevels == false) {
    // sleep while we don't have the user response
    sleepThread();
  }

  if (m_askThreadToEnd == true) {
    return 0;
  }

  setThreadCurrentOperation(GAMETEXT_DLLEVELS);
  setThreadProgress(0);

  try {
    LogInfo("WWW: Downloading levels...");

    clearCancelAsSoonAsPossible();

    setThreadCurrentMicroOperation("");

    // update theme before upgrading levels
    setSafeKill(true);
    WebThemes::updateThemeList(m_pDb, this);
    /* update the theme only if that's an updatable theme to allow people having
     * custom theme to update levels */
    /* if a texture is missing, level rendering could be weired */
    if (WebThemes::isUpdatable(m_pDb, m_id_theme)) {
      WebThemes::updateTheme(m_pDb, m_id_theme, this);
    }
    setSafeKill(false);

    if (StateManager::exists()) {
      StateManager::instance()->sendAsynchronousMessage(
        std::string("THEMES_UPDATED"));
    }

    // don't safe kill here
    v_shouldBeLoadedInBase = true;
    m_pWebLevels->upgrade(m_pDb, m_nb_levels);
  } catch (Exception &e) {
    m_msg =
      GAMETEXT_FAILEDDLLEVELS + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW;
    LogWarning("Unable to download extra levels [%s]", e.getMsg().c_str());

    v_exit_code = 1;
  }

  if (v_exit_code == 0 || v_shouldBeLoadedInBase) {
    /* Got some new levels... load them! */
    LogInfo("Loading new and updated levels...");

    setThreadCurrentOperation(GAMETEXT_LOADNEWLEVELS);
    setThreadProgress(0);

    LevelsManager::instance()->updateLevelsFromLvl(
      m_pWebLevels->getNewDownloadedLevels(),
      m_pWebLevels->getUpdatedDownloadedLevels(),
      m_loadMainLayerOnly,
      this,
      m_pDb);

    /* Update level lists */
    if (StateManager::exists()) {
      StateManager::instance()->sendAsynchronousMessage(
        std::string("NO_NEW_LEVELS_TO_DOWNLOAD"));
      StateManager::instance()->sendAsynchronousMessage(
        std::string("LEVELS_UPDATED"));
    }
  }

  return v_exit_code;
}

void UpgradeLevelsThread::setTaskProgress(float p_percent) {
  setThreadProgress((int)p_percent);
}

std::string UpgradeLevelsThread::getMsg() const {
  return m_msg;
}

void UpgradeLevelsThread::askThreadToEnd() {
  setCancelAsSoonAsPossible();
  XMThread::askThreadToEnd();
}
