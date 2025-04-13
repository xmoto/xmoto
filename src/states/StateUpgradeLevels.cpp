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

#include "StateUpgradeLevels.h"
#include "StateMessageBox.h"
#include "common/XMSession.h"
#include "db/xmDatabase.h"
#include "helpers/CmdArgumentParser.h"
#include "helpers/Log.h"
#include "thread/UpgradeLevelsThread.h"
#include "xmoto/GameText.h"
#include "xmoto/input/Input.h"

#define UPGRADE_LEVELS_NB_PROPOSAL 20

StateUpgradeLevels::StateUpgradeLevels(bool drawStateBehind,
                                       bool updateStatesBehind)
  : StateUpdate(drawStateBehind, updateStatesBehind) {
  m_ult = new UpgradeLevelsThread(XMSession::instance()->theme(), false, false);
  m_pThread = m_ult;
  m_name = "StateUpgradeLevels";
  m_messageOnFailureModal = false;

  StateManager::instance()->registerAsObserver("NEWLEVELAVAILABLE", this);
  StateManager::instance()->registerAsObserver("ASKINGLEVELUPDATE", this);
}

StateUpgradeLevels::~StateUpgradeLevels() {
  StateManager::instance()->unregisterAsObserver("NEWLEVELAVAILABLE", this);
  StateManager::instance()->unregisterAsObserver("ASKINGLEVELUPDATE", this);

  delete m_pThread;
}

void StateUpgradeLevels::sendFromMessageBox(const std::string &i_id,
                                            UIMsgBoxButton i_button,
                                            const std::string &i_input) {
  if (i_id == "DOWNLOAD_LEVELS") {
    switch (i_button) {
      case UI_MSGBOX_YES:
        m_pThread->unsleepThread();
        break;
      case UI_MSGBOX_NO:
        m_pThread->askThreadToEnd();
        m_pThread->unsleepThread();
        break;
      case UI_MSGBOX_CUSTOM1:
        m_ult->setNbLevels(UPGRADE_LEVELS_NB_PROPOSAL);
        m_pThread->unsleepThread();
      default:
        break;
    }
  } else if (i_id == "ASKING_LEVEL_UPDATE") {
    switch (i_button) {
      case UI_MSGBOX_YES:
        m_pThread->unsleepThread("YES");
        break;
      case UI_MSGBOX_NO:
        m_pThread->unsleepThread("NO");
        break;
      case UI_MSGBOX_YES_FOR_ALL:
        m_pThread->unsleepThread("YES_FOR_ALL");
        break;
      default:
        break;
    }
  } else {
    StateUpdate::sendFromMessageBox(i_id, i_button, i_input);
  }
}

void StateUpgradeLevels::executeOneCommand(std::string cmd, std::string args) {
  LogDebug("cmd [%s [%s]] executed by state [%s].",
           cmd.c_str(),
           args.c_str(),
           getName().c_str());

  if (cmd == "NEWLEVELAVAILABLE") {
    int nULevels = xmDatabase::instance("main")->levels_nbLevelsToDownload();
    char cBuf[256];
    snprintf(cBuf, 256, GAMETEXT_NEWLEVELAVAIL(nULevels), nULevels);

    /* Ask user whether he want to download levels or snot */
    StateMessageBox *v_state;

    if (nULevels > UPGRADE_LEVELS_NB_PROPOSAL) {
      v_state = new StateMessageBox(
        this, cBuf, (UI_MSGBOX_YES | UI_MSGBOX_NO | UI_MSGBOX_CUSTOM1));
      char buf[32];
      snprintf(buf,
               32,
               GAMETEXT_XONLY(UPGRADE_LEVELS_NB_PROPOSAL),
               UPGRADE_LEVELS_NB_PROPOSAL);
      v_state->setCustom(buf);
    } else {
      v_state = new StateMessageBox(this, cBuf, (UI_MSGBOX_YES | UI_MSGBOX_NO));
    }
    v_state->setMsgBxId("DOWNLOAD_LEVELS");
    StateManager::instance()->pushState(v_state);
  } else if (cmd == "ASKINGLEVELUPDATE") {
    std::string curUpdLevelName =
      CmdArgumentParser::instance()->getString(args);

    char cBuf[256];
    snprintf(cBuf, 256, GAMETEXT_WANTTOUPDATELEVEL, curUpdLevelName.c_str());

    StateMessageBox *v_state = new StateMessageBox(
      this, cBuf, (UI_MSGBOX_YES | UI_MSGBOX_NO | UI_MSGBOX_YES_FOR_ALL));
    v_state->setMsgBxId("ASKING_LEVEL_UPDATE");
    StateManager::instance()->pushState(v_state);
  } else {
    GameState::executeOneCommand(cmd, args);
  }
}

void StateUpgradeLevels::callAfterThreadFinished(int threadResult) {
  m_msg = ((UpgradeLevelsThread *)m_pThread)->getMsg();
}

void StateUpgradeLevels::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (i_type == INPUT_DOWN &&
      (i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE) ||
       i_xmkey.getJoyButton() == SDL_CONTROLLER_BUTTON_B)) {
    m_pThread->askThreadToEnd();
  }

  else if (i_type == INPUT_DOWN &&
           i_xmkey == (*Input::instance()->getGlobalKey(INPUT_KILLPROCESS))) {
    if (m_threadStarted == true) {
      m_pThread->safeKill();
    }
  }
}
