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
#include "thread/UpgradeLevelsThread.h"
#include "GameText.h"
#include "db/xmDatabase.h"
#include "XMSession.h"

StateUpgradeLevels::StateUpgradeLevels(bool drawStateBehind,
				       bool updateStatesBehind)
  : StateUpdate(drawStateBehind, updateStatesBehind)
{
  m_pThread = new UpgradeLevelsThread(this, XMSession::instance()->theme());
  m_name    = "StateUpgradeLevels";
  m_curUpdLevelName = "";
}

StateUpgradeLevels::~StateUpgradeLevels()
{
  delete m_pThread;
}

void StateUpgradeLevels::send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input)
{
  if(i_id == "DOWNLOAD_LEVELS"){
    switch(i_button){
    case UI_MSGBOX_YES:
      m_pThread->unsleepThread();
      break;
    case UI_MSGBOX_NO:
      m_pThread->askThreadToEnd();
      m_pThread->unsleepThread();
      break;
    default:
      break;
    }
  }
  else if(i_id == "ASKING_LEVEL_UPDATE"){
    switch(i_button){
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
  }
  else {
    StateUpdate::send(i_id, i_button, i_input);
  }
}

void StateUpgradeLevels::executeOneCommand(std::string cmd)
{
  if(cmd == "NEWLEVELAVAILABLE"){
    int nULevels = xmDatabase::instance("main")->levels_nbLevelsToDownload();
    char cBuf[256];
    snprintf(cBuf, 256, GAMETEXT_NEWLEVELAVAIL(nULevels), nULevels);

    /* Ask user whether he want to download levels or snot */
    StateMessageBox* v_state = new StateMessageBox(this, cBuf,
						   (UI_MSGBOX_YES|UI_MSGBOX_NO));
    v_state->setId("DOWNLOAD_LEVELS");
    StateManager::instance()->pushState(v_state);
  }
  else if(cmd == "ASKINGLEVELUPDATE"){
    char cBuf[256];
    snprintf(cBuf, 256, GAMETEXT_WANTTOUPDATELEVEL, m_curUpdLevelName.c_str());

    StateMessageBox* v_state = new StateMessageBox(this, cBuf,
						   (UI_MSGBOX_YES|UI_MSGBOX_NO|UI_MSGBOX_YES_FOR_ALL));
    v_state->setId("ASKING_LEVEL_UPDATE");
    StateManager::instance()->pushState(v_state);
  }
}

void StateUpgradeLevels::callAfterThreadFinished(int threadResult)
{
  m_msg = ((UpgradeLevelsThread*)m_pThread)->getMsg();
}

void StateUpgradeLevels::keyDown(int nKey, SDLMod mod,int nChar, const std::string& i_utf8Char) {
  if(nKey == SDLK_ESCAPE) {
    m_pThread->askThreadToEnd();
  } else if(nKey == SDLK_k && (mod & KMOD_CTRL) != 0) {
    if(m_threadStarted == true) {
      m_pThread->safeKill();
    }
  }
}

void StateUpgradeLevels::setCurrentUpdatedLevel(std::string levelName)
{
  m_curUpdLevelName = levelName;
}
