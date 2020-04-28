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

#include "StateMessageBox.h"
#include "StateUpdate.h"
#include "common/XMSession.h"
#include "drawlib/DrawLib.h"
#include "helpers/Log.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"
#include "xmoto/SysMessage.h"

/* static members */
UIRoot *StateWaiting::m_sGUI = NULL;

StateWaiting::StateWaiting(bool drawStateBehind, bool updateStatesBehind)
  : StateMenu(drawStateBehind, updateStatesBehind) {
  m_name = "StateWaiting";
  init();
}

void StateWaiting::init() {
  m_progress = -1;
  m_currentOperation = "";
  m_currentMicroOperation = "";
}

StateWaiting::~StateWaiting() {}

void StateWaiting::enter() {
  createGUIIfNeeded(&m_screen);
  m_GUI = m_sGUI;

  init();
  updateGUI();

  StateMenu::enter();
}

void StateWaiting::leave() {
  StateMenu::leave();
}

void StateWaiting::xmKey(InputEventType i_type, const XMKey &i_xmkey) {}

void StateWaiting::clean() {
  if (StateWaiting::m_sGUI != NULL) {
    delete StateWaiting::m_sGUI;
    StateWaiting::m_sGUI = NULL;
  }
}

void StateWaiting::createGUIIfNeeded(RenderSurface *i_screen) {
  if (m_sGUI != NULL)
    return;

  DrawLib *drawLib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot(i_screen);
  m_sGUI->setFont(drawLib->getFontSmall());
  m_sGUI->setPosition(
    0, 0, i_screen->getDispWidth(), i_screen->getDispHeight());

  /* Initialize level info viewer */
  int width = i_screen->getDispWidth();
  int height = i_screen->getDispHeight();

  int x = width / 8;
  int y = height / 4;
  std::string caption = "";
  int nWidth = width * 3 / 4;
  int nHeight = height / 2;

  UIFrame *v_frame;
  v_frame = new UIFrame(m_sGUI, x, y, caption, nWidth, nHeight);
  v_frame->setID("FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_TRANS);

  int proH = 15;
  int proX = nWidth / 32;
  int proY = nHeight - proH * 2;
  int proW = (int)(nWidth * (15.0 / 16.0));

  UIProgressBar *v_progress;
  v_progress = new UIProgressBar(v_frame, proX, proY, proW, proH);
  v_progress->setID("PROGRESS");

  UIStatic *v_static;
  v_static = new UIStatic(v_frame,
                          0,
                          0,
                          "",
                          v_frame->getPosition().nWidth,
                          v_frame->getPosition().nHeight - proH * 2);
  v_static->setFont(drawLib->getFontMedium());
  v_static->setHAlign(UI_ALIGN_CENTER);
  v_static->setID("TEXT");
}

void StateWaiting::updateGUI() {
  UIProgressBar *v_progress =
    reinterpret_cast<UIProgressBar *>(m_GUI->getChild("FRAME:PROGRESS"));
  v_progress->setProgress(m_progress);
  v_progress->setCurrentOperation(m_currentMicroOperation);

  UIStatic *v_static =
    reinterpret_cast<UIStatic *>(m_GUI->getChild("FRAME:TEXT"));
  v_static->setCaption(m_currentOperation);
}

bool StateWaiting::update() {
  if (StateMenu::update() == false) {
    return false;
  }

  return true;
}
