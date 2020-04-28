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

#include "StateMultiUpdate.h"
#include "StateMessageBox.h"
#include "drawlib/DrawLib.h"
#include "helpers/Log.h"
#include "thread/XMThread.h"
#include "xmoto/Game.h"

/* static members */
UIRoot *StateMultiUpdate::m_sGUI = NULL;

StateMultiUpdate::StateMultiUpdate(bool drawStateBehind,
                                   bool updateStatesBehind)
  : StateMenu(drawStateBehind, updateStatesBehind) {
  m_numberThreadDisplayed = 0;
  m_numberThreadRunning = 0;
}

void StateMultiUpdate::init() {
  std::map<std::string, ThreadInfos *>::iterator iter;
  for (iter = m_threadsInfos.begin(); iter != m_threadsInfos.end(); iter++) {
    ThreadInfos *pInfos = iter->second;
    initThreadInfos(pInfos);
  }
}

StateMultiUpdate::~StateMultiUpdate() {}

void StateMultiUpdate::enter() {
  createGUIIfNeeded(&m_screen);
  m_GUI = m_sGUI;

  StateMenu::enter();
}

void StateMultiUpdate::leave() {
  // blank window
  init();
  updateGUI();

  StateMenu::leave();
}

bool StateMultiUpdate::update() {
  if (StateMenu::update() == false) {
    return false;
  }

  std::map<std::string, ThreadInfos *>::iterator iter;
  for (iter = m_threadsInfos.begin(); iter != m_threadsInfos.end(); iter++) {
    std::string id = iter->first;
    ThreadInfos *pInfos = iter->second;
    XMThread *pThread = m_threads.getThread(id);

    if (pInfos->m_threadStarted == true &&
        pThread->isThreadRunning() == false) {
      pInfos->m_threadStarted = false;
      pInfos->m_threadFinished = true;
      if (pThread->waitForThreadEnd() != 0) {
        StateMessageBox *v_msgboxState =
          new StateMessageBox(this, pInfos->m_errorMessage, UI_MSGBOX_OK);
        v_msgboxState->setMsgBxId("ERROR");
        StateManager::instance()->pushState(v_msgboxState);
      }
    }

    if (pInfos->m_threadStarted == false && pInfos->m_threadFinished == false) {
      pThread->startThread();
      pInfos->m_threadStarted = true;

      std::string msg = "thread " + id + " started";
      LogInfo(msg.c_str());
    }

    if (pInfos->m_threadStarted == true) {
      int progress = pThread->getThreadProgress();
      if (progress != pInfos->m_progress) {
        pInfos->m_progress = progress;
        pInfos->m_currentOperation = pThread->getThreadCurrentOperation();
        pInfos->m_currentMicroOperation =
          pThread->getThreadCurrentMicroOperation();
      }
    }
  }

  return true;
}

void StateMultiUpdate::xmKey(InputEventType i_type, const XMKey &i_xmkey) {}

void StateMultiUpdate::clean() {
  if (StateMultiUpdate::m_sGUI != NULL) {
    delete StateMultiUpdate::m_sGUI;
    StateMultiUpdate::m_sGUI = NULL;
  }
}

void StateMultiUpdate::checkEvents() {}

void StateMultiUpdate::updateGUI() {
  int nbRunning = m_threads.getNumberRunningThreads();
  if (m_numberThreadDisplayed != nbRunning) {
    // create of remove progress bars
  }

  // update progress bars
}

void StateMultiUpdate::sendFromMessageBox(const std::string &i_id,
                                          UIMsgBoxButton i_button,
                                          const std::string &i_input) {
  StateMenu::sendFromMessageBox(i_id, i_button, i_input);
}

void StateMultiUpdate::initThreadInfos(ThreadInfos *pInfos) {
  pInfos->m_threadStarted = false;
  pInfos->m_threadFinished = false;
  pInfos->m_progress = -1;
  pInfos->m_currentOperation = "";
  pInfos->m_currentMicroOperation = "";
  pInfos->m_errorMessage = "";
}

void StateMultiUpdate::createGUIIfNeeded(RenderSurface *i_screen) {
  if (m_sGUI != NULL)
    return;

  DrawLib *drawLib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot(i_screen);
  m_sGUI->setFont(drawLib->getFontSmall());
  m_sGUI->setPosition(
    0, 0, i_screen->getDispWidth(), i_screen->getDispHeight());

  int width = i_screen->getDispWidth();
  int height = i_screen->getDispHeight();

  int x = width / 8;
  int y = height / 4;
  std::string caption = "Multi threads in progress";
  int nWidth = width * 3 / 4;
  int nHeight = height / 2;

  UIFrame *v_frame;
  v_frame = new UIFrame(m_sGUI, x, y, caption, nWidth, nHeight);
  v_frame->setID("FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_TRANS);
}
