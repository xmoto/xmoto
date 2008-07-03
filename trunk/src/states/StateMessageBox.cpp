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
#include "Game.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"

StateMessageBox::StateMessageBox(StateMessageBoxReceiver* i_receiver,
				 const std::string& i_text,
				 int i_buttons,
				 bool i_input,
				 const std::string& i_inputText,
				 bool i_query,
				 bool drawStateBehind,
				 bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind)
{
  m_receiver      = i_receiver;
  m_clickedButton = UI_MSGBOX_NOTHING;
  createGUI(i_text, i_buttons, i_input, i_inputText, i_query);
  m_name          = "StateMessageBox";
}

StateMessageBox::~StateMessageBox()
{
  delete m_GUI;
}

void StateMessageBox::makeActiveButton(UIMsgBoxButton i_button) {
  m_msgbox->makeActiveButton(i_button);
}

void StateMessageBox::leave()
{
  if(m_receiver != NULL) {
    m_receiver->send(getId(), m_clickedButton, m_msgbox->getTextInput());
  }
}

void StateMessageBox::checkEvents() {
  UIMsgBoxButton Button = m_msgbox->getClicked();

  if(Button == UI_MSGBOX_NOTHING)
    return;

  m_clickedButton = Button;

  m_requestForEnd = true;
}

void StateMessageBox::keyDown(int nKey, SDLMod mod,int nChar, const std::string& i_utf8Char)
{
  switch(nKey) {
  default:
    StateMenu::keyDown(nKey, mod, nChar, i_utf8Char);
    break;
  }
}

void StateMessageBox::createGUI(const std::string& i_text, int i_buttons,
				bool i_input, const std::string& i_inputText, bool i_query) {
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  m_GUI = new UIRoot();
  m_GUI->setFont(drawlib->getFontSmall()); 
  m_GUI->setPosition(0, 0,
		     drawlib->getDispWidth(),
		     drawlib->getDispHeight());

  m_msgbox = m_GUI->msgBox(i_text, (UIMsgBoxButton)(i_buttons), i_input);
  if(i_input) {
    m_msgbox->setTextInputFont(drawlib->getFontMedium());
    m_msgbox->setTextInput(i_inputText);
  }
}
