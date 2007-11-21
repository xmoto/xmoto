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

#include "Game.h"
#include "GameText.h"
#include "drawlib/DrawLib.h"
#include "StateUpdateTheme.h"
#include "StateMessageBox.h"
#include "thread/UpdateThemeThread.h"
#include "helpers/Log.h"

StateUpdateTheme::StateUpdateTheme(GameApp* pGame,
				   const std::string& i_id_theme,
				   bool drawStateBehind,
				   bool updateStatesBehind)
  : StateUpdate(pGame, drawStateBehind, updateStatesBehind)
{
  m_pThread          = new UpdateThemeThread(i_id_theme);
  m_name             = "StateUpdateTheme";
  m_id_theme         = i_id_theme;
}

StateUpdateTheme::~StateUpdateTheme()
{
  delete m_pThread;
}

void StateUpdateTheme::enter()
{
  StateUpdate::enter();

  enterAttractMode(NO_KEY);
}

bool StateUpdateTheme::update()
{
  if(StateUpdate::update() == false){
    return false;
  }

  // thread finished. we leave the state.
  if(m_threadStarted == true && m_pThread->isThreadRunning() == false){
    if(m_pThread->waitForThreadEnd() == 0) {
      m_requestForEnd = true;
      m_threadStarted = false;
      m_pGame->reloadTheme();
    } else {
      StateMessageBox* v_msgboxState = new StateMessageBox(this, m_pGame,
							   GAMETEXT_FAILEDGETSELECTEDTHEME + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW, UI_MSGBOX_OK);
      v_msgboxState->setId("ERROR");
      m_pGame->getStateManager()->pushState(v_msgboxState);
    }

    return true;
  }

  if(m_threadStarted == false){
   char **v_result;
   unsigned int nrow;
   std::string v_id_theme;
   std::string v_ck1, v_ck2;
   bool v_onDisk = false;
   bool v_onWeb  = true;

   v_result = m_pGame->getDb()->readDB("SELECT a.id_theme, a.checkSum, b.checkSum "
				       "FROM themes AS a LEFT OUTER JOIN webthemes AS b "
				       "ON a.id_theme=b.id_theme "
				       "WHERE a.id_theme=\"" + xmDatabase::protectString(m_id_theme) + "\";",
				       nrow);
   if(nrow == 1) {
     v_onDisk   = true;
     v_id_theme = m_pGame->getDb()->getResult(v_result, 3, 0, 0);
     v_ck1      = m_pGame->getDb()->getResult(v_result, 3, 0, 1);
     if(m_pGame->getDb()->getResult(v_result, 3, 0, 2) == NULL) {
	v_onWeb = false;
     } else {
	v_ck2 = m_pGame->getDb()->getResult(v_result, 3, 0, 2);
     }
   }
   m_pGame->getDb()->read_DB_free(v_result);

   if(v_onWeb == false) { /* available on the disk, not on the web */
     StateMessageBox* v_msgboxState = new StateMessageBox(this, m_pGame, GAMETEXT_UNUPDATABLETHEMEONWEB, UI_MSGBOX_OK);
     v_msgboxState->setId("ERROR");
     m_pGame->getStateManager()->pushState(v_msgboxState);
     return true;
   }

    m_pThread->startThread(m_pGame);
    m_threadStarted = true;

    Logger::Log("thread started");
  }

  if(m_threadStarted == true){

    // update the frame with the thread informations only when progress change
    // to avoid spending tooo much time waiting for mutexes.
    int progress = m_pThread->getThreadProgress();
    if(progress != m_progress){
      m_progress              = progress;
      m_currentOperation      = m_pThread->getThreadCurrentOperation();
      m_currentMicroOperation = m_pThread->getThreadCurrentMicroOperation();

      updateGUI();
    }
  }

  return true;
}

void StateUpdateTheme::send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  if(i_id == "ERROR") {
    m_requestForEnd = true;
  }
}
