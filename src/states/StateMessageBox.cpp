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
#include "drawlib/DrawLib.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"

void StateMessageBox::initStateMessageBox(StateMessageBoxReceiver *i_receiver,
                                          const std::string &i_text,
                                          int i_buttons,
                                          bool i_input,
                                          const std::string &i_inputText,
                                          bool i_query,
                                          bool i_verticallyLarge) {
  m_receiver = i_receiver;
  m_clickedButton = UI_MSGBOX_NOTHING;
  m_name = "StateMessageBox";
  m_msgbox = NULL;

  m_text = i_text;
  m_buttons = i_buttons;
  m_input = i_input;
  m_inputText = i_inputText;
  m_query = i_query;
  m_verticallyLarge = i_verticallyLarge;
  m_exitable = false;
}

StateMessageBox::StateMessageBox(StateMessageBoxReceiver *i_receiver,
                                 const std::string &i_text,
                                 int i_buttons,
                                 bool i_input,
                                 const std::string &i_inputText,
                                 bool i_query,
                                 bool drawStateBehind,
                                 bool updateStatesBehind,
                                 bool i_verticallyLarge)
  : StateMenu(drawStateBehind, updateStatesBehind) {
  initStateMessageBox(i_receiver,
                      i_text,
                      i_buttons,
                      i_input,
                      i_inputText,
                      i_query,
                      i_verticallyLarge);
}

StateMessageBox::StateMessageBox(StateMessageBoxReceiver *i_receiver,
                                 std::vector<std::string> &completionList,
                                 const std::string &i_text,
                                 int i_buttons,
                                 bool i_input,
                                 const std::string &i_inputText,
                                 bool i_query,
                                 bool drawStateBehind,
                                 bool updateStatesBehind,
                                 bool i_verticallyLarge)
  : StateMenu(drawStateBehind, updateStatesBehind) {
  initStateMessageBox(i_receiver,
                      i_text,
                      i_buttons,
                      i_input,
                      i_inputText,
                      i_query,
                      i_verticallyLarge);
  m_completionList = completionList;
}

StateMessageBox::~StateMessageBox() {}

void StateMessageBox::makeActiveButton(UIMsgBoxButton i_button) {
  if (m_msgbox != NULL) {
    m_msgbox->makeActiveButton(i_button);
  } else {
    throw Exception("Active buttons can be set only once the state is entered");
  }
}

void StateMessageBox::enter() {
  createGUI();
  StateMenu::enter();
}

void StateMessageBox::leave() {
  if (m_receiver != NULL) {
    m_receiver->sendFromMessageBox(
      getMsgBxId(), m_clickedButton, m_msgbox->getTextInput());
  } else {
    sendFromMessageBox(getMsgBxId(), m_clickedButton, m_msgbox->getTextInput());
  }

  delete m_GUI;
}

void StateMessageBox::checkEvents() {
  UIMsgBoxButton Button = m_msgbox->getClicked();

  if (Button == UI_MSGBOX_NOTHING)
    return;

  m_clickedButton = Button;

  m_requestForEnd = true;
}

void StateMessageBox::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (m_exitable && i_type == INPUT_DOWN &&
      (i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE) ||
       i_xmkey.getJoyButton() == SDL_CONTROLLER_BUTTON_B)) {
    m_requestForEnd = true;
    return;
  }

  StateMenu::xmKey(i_type, i_xmkey);
}

void StateMessageBox::createGUI() {
  DrawLib *drawlib = GameApp::instance()->getDrawLib();

  m_GUI = new UIRoot(&m_screen);
  m_GUI->setFont(drawlib->getFontSmall());
  m_GUI->setPosition(0, 0, m_screen.getDispWidth(), m_screen.getDispHeight());

  m_msgbox = m_GUI->msgBox(m_text,
                           (UIMsgBoxButton)(m_buttons),
                           m_help,
                           m_custom1,
                           m_custom2,
                           m_input,
                           false,
                           m_verticallyLarge);
  if (m_input) {
    m_msgbox->setTextInputFont(drawlib->getFontMedium());
    m_msgbox->setTextInput(m_inputText);
  }
  m_msgbox->addCompletionWord(m_completionList);
}

std::string StateMessageBox::getMsgBxId() const {
  return m_msgbxid;
}

void StateMessageBox::setMsgBxId(const std::string &i_id) {
  m_msgbxid = i_id;
}

void StateMessageBox::setCustom(const std::string &i_custom1,
                                const std::string &i_custom2) {
  m_custom1 = i_custom1;
  m_custom2 = i_custom2;
}

void StateMessageBox::setHelp(const std::string &i_help) {
  m_help = i_help;
}
