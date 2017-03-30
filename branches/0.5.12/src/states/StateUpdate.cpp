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

#include "StateUpdate.h"
#include "../Game.h"
#include "../drawlib/DrawLib.h"
#include "../GameText.h"
#include "../SysMessage.h"
#include "../XMSession.h"
#include "StateMessageBox.h"
#include "../helpers/Log.h"

StateUpdate::StateUpdate(bool drawStateBehind,
			 bool updateStatesBehind):
  StateWaiting(drawStateBehind,
	       updateStatesBehind)
{
  m_name             = "StateUpdate";
  m_threadStarted    = false;
  m_threadFinished   = false;
  m_pThread          = NULL;
  m_msg              = "";
  m_messageOnSuccess = false;
  m_messageOnFailure = true;
  m_messageOnSuccessModal = true;
  m_messageOnFailureModal = true;
}

StateUpdate::~StateUpdate()
{
}

void StateUpdate::enter()
{
  StateWaiting::enter();
}

void StateUpdate::leave()
{
  StateWaiting::leave();
}

bool StateUpdate::update()
{
  if(StateWaiting::update() == false){
    return false;
  }

  // thread finished. we leave the state.
  if(m_threadStarted == true && m_pThread->isThreadRunning() == false){
    m_threadStarted  = false;
    m_threadFinished = true;
    int v_thread_res = m_pThread->waitForThreadEnd();
    callAfterThreadFinished(v_thread_res);

    if(v_thread_res == 0) {
      if(m_messageOnSuccess == true && m_msg != "") {
	if(m_messageOnSuccessModal) {
	  StateMessageBox* v_msgboxState = new StateMessageBox(this, m_msg, UI_MSGBOX_OK);
	  v_msgboxState->setMsgBxId("SUCCESS");
	  StateManager::instance()->pushState(v_msgboxState);
	} else {
	  SysMessage::instance()->displayInformation(m_msg);
	  m_requestForEnd = true;
	}
      } else {
	m_requestForEnd = true;
      }
    }
    else {
      if(m_messageOnFailure == true && m_msg != "") {
	if(m_messageOnFailureModal) {
	  StateMessageBox* v_msgboxState = new StateMessageBox(this, m_msg, UI_MSGBOX_OK);
	  v_msgboxState->setMsgBxId("ERROR");
	  StateManager::instance()->pushState(v_msgboxState);
	} else {
	  SysMessage::instance()->displayError(m_msg);
	  m_requestForEnd = true;
	}
      } else {
	m_requestForEnd = true;
      }
    }

    LogInfo("thread ended");

    return true;
  }

  if(m_threadStarted == false && m_threadFinished == false){
    if(callBeforeLaunchingThread() == false){
      return true;
    }

    m_pThread->startThread();
    m_threadStarted = true;

    LogInfo("thread started");
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

void StateUpdate::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
}

void StateUpdate::callAfterThreadFinished(int threadResult)
{
}

bool StateUpdate::callBeforeLaunchingThread()
{
  return true;
}

void StateUpdate::sendFromMessageBox(const std::string& i_id,
		       UIMsgBoxButton i_button,
		       const std::string& i_input)
{
  if(i_id == "ERROR" || i_id == "SUCCESS") {
    m_requestForEnd = true;
  }

  else {
    GameState::sendFromMessageBox(i_id, i_button, i_input);
  }
}

void StateUpdate::onThreadFinishes(bool i_res) {
}
