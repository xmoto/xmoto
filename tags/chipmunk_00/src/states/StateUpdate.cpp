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
#include "Game.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"
#include "XMSession.h"
#include "StateMessageBox.h"
#include "helpers/Log.h"


/* static members */
UIRoot*  StateUpdate::m_sGUI = NULL;

StateUpdate::StateUpdate(bool drawStateBehind,
			 bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind)
{
  m_name             = "StateUpdate";
  m_threadStarted    = false;
  m_threadFinished   = false;
  m_pThread          = NULL;
  m_msg              = "";
  m_messageOnSuccess = false;
  m_messageOnFailure = true;
  init();
}

void StateUpdate::init()
{
  m_progress              = -1;
  m_currentOperation      = "";
  m_currentMicroOperation = "";
}

StateUpdate::~StateUpdate()
{
}

void StateUpdate::enter()
{
  createGUIIfNeeded();
  m_GUI = m_sGUI;

  StateMenu::enter();
}

void StateUpdate::leave()
{
  // blank window
  init();
  updateGUI();

  StateMenu::leave();
}

bool StateUpdate::update()
{
  if(StateMenu::update() == false){
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
	StateMessageBox* v_msgboxState = new StateMessageBox(this, m_msg, UI_MSGBOX_OK);
	v_msgboxState->setId("SUCCESS");
	StateManager::instance()->pushState(v_msgboxState);
      } else {
	m_requestForEnd = true;
      }
    }
    else {
      if(m_messageOnFailure == true && m_msg != "") {
	StateMessageBox* v_msgboxState = new StateMessageBox(this, m_msg, UI_MSGBOX_OK);
	v_msgboxState->setId("ERROR");
	StateManager::instance()->pushState(v_msgboxState);
      } else {
	m_requestForEnd = true;
      }
    }

    Logger::Log("thread ended");

    return true;
  }

  if(m_threadStarted == false && m_threadFinished == false){
    if(callBeforeLaunchingThread() == false){
      return true;
    }

    m_pThread->startThread();
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

void StateUpdate::keyDown(int nKey, SDLMod mod,int nChar)
{
}

void StateUpdate::keyUp(int nKey,   SDLMod mod)
{
}

void StateUpdate::clean()
{
  if(StateUpdate::m_sGUI != NULL) {
    delete StateUpdate::m_sGUI;
    StateUpdate::m_sGUI = NULL;
  }
}

void StateUpdate::createGUIIfNeeded()
{
  if(m_sGUI != NULL)
    return;

  DrawLib* drawLib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot();
  m_sGUI->setFont(drawLib->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      drawLib->getDispWidth(),
		      drawLib->getDispHeight());

  /* Initialize level info viewer */
  int width = drawLib->getDispWidth();
  int height= drawLib->getDispHeight();

  int x = width / 8;
  int y = height / 4;
  std::string caption = "";
  int nWidth  = width * 3/4;
  int nHeight = height / 2;

  UIFrame* v_frame;
  v_frame = new UIFrame(m_sGUI, x, y, caption, nWidth, nHeight); 
  v_frame->setID("FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_TRANS);

  int proH = 15;
  int proX = nWidth / 32;
  int proY = nHeight - proH * 2;
  int proW = (int)(nWidth * (15.0/16.0));

  UIProgressBar* v_progress;
  v_progress = new UIProgressBar(v_frame, proX, proY, proW, proH);
  v_progress->setID("PROGRESS");

  UIStatic* v_static;
  v_static = new UIStatic(v_frame, 0, 0, "", v_frame->getPosition().nWidth, v_frame->getPosition().nHeight - proH * 2);
  v_static->setFont(drawLib->getFontMedium());            
  v_static->setHAlign(UI_ALIGN_CENTER);
  v_static->setID("TEXT");
}

void StateUpdate::updateGUI()
{
  UIProgressBar* v_progress = reinterpret_cast<UIProgressBar*>(m_GUI->getChild("FRAME:PROGRESS"));
  v_progress->setProgress(m_progress);
  v_progress->setCurrentOperation(m_currentMicroOperation);

  UIStatic* v_static = reinterpret_cast<UIStatic*>(m_GUI->getChild("FRAME:TEXT"));
  v_static->setCaption(m_currentOperation);
}

void StateUpdate::callAfterThreadFinished(int threadResult)
{
}

bool StateUpdate::callBeforeLaunchingThread()
{
  return true;
}

void StateUpdate::send(const std::string& i_id,
		       UIMsgBoxButton i_button,
		       const std::string& i_input)
{
  if(i_id == "ERROR" || i_id == "SUCCESS") {
    m_requestForEnd = true;
  }
}

void StateUpdate::onThreadFinishes(bool i_res) {
}
