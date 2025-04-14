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

#include "StateUpdateTheme.h"
#include "StateMessageBox.h"
#include "helpers/Log.h"
#include "thread/UpdateThemeThread.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"

StateUpdateTheme::StateUpdateTheme(const std::string &i_id_theme,
                                   bool drawStateBehind,
                                   bool updateStatesBehind)
  : StateUpdate(drawStateBehind, updateStatesBehind) {
  m_pThread = new UpdateThemeThread(i_id_theme);
  m_name = "StateUpdateTheme";
  m_id_theme = i_id_theme;
  m_msg = GAMETEXT_FAILEDGETSELECTEDTHEME + std::string("\n") +
          GAMETEXT_CHECK_YOUR_WWW;
}

StateUpdateTheme::~StateUpdateTheme() {
  delete m_pThread;
}

void StateUpdateTheme::callAfterThreadFinished(int threadResult) {
  if (threadResult == 0) {
    GameApp::instance()->reloadTheme();
  }
}

bool StateUpdateTheme::callBeforeLaunchingThread() {
  char **v_result;
  unsigned int nrow;
  bool v_onWeb = true;

  v_result = xmDatabase::instance("main")->readDB(
    "SELECT a.id_theme, a.checkSum, b.checkSum "
    "FROM themes AS a LEFT OUTER JOIN webthemes AS b "
    "ON a.id_theme=b.id_theme "
    "WHERE a.id_theme=\"" +
      xmDatabase::protectString(m_id_theme) + "\";",
    nrow);
  if (nrow == 1) {
    if (xmDatabase::instance("main")->getResult(v_result, 3, 0, 2) == NULL) {
      v_onWeb = false;
    }
  }
  xmDatabase::instance("main")->read_DB_free(v_result);

  if (v_onWeb == false) { /* available on the disk, not on the web */
    StateMessageBox *v_msgboxState =
      new StateMessageBox(this, GAMETEXT_UNUPDATABLETHEMEONWEB, UI_MSGBOX_OK);
    v_msgboxState->setMsgBxId("ERROR");
    StateManager::instance()->pushState(v_msgboxState);
    return false;
  }

  return true;
}

void StateUpdateTheme::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (i_type == INPUT_DOWN &&
      (i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE) ||
       i_xmkey.getJoyButton() == SDL_CONTROLLER_BUTTON_B)) {
    m_pThread->askThreadToEnd();
  }

  else if (i_type == INPUT_DOWN &&
           i_xmkey == (*Input::instance()->getGlobalKey(INPUT_KILLPROCESS))) {
    if (m_threadStarted == true) {
      m_messageOnFailure = false;
      m_pThread->safeKill();
    }
  }
}
