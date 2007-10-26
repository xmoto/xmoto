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
				 GameApp* pGame,
				 const std::string& i_text,
				 int i_buttons,
				 bool i_input,
				 const std::string& i_inputText,
				 bool i_query,
				 bool drawStateBehind,
				 bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{
  m_receiver = i_receiver;
  m_clickedButton = UI_MSGBOX_NOTHING;
  createGUI(m_pGame, i_text, i_buttons, i_input, i_inputText, i_query);
  m_name  = "StateMessageBox";
}

StateMessageBox::~StateMessageBox()
{
  delete m_GUI;
}

void StateMessageBox::enter()
{
  StateMenu::enter();
}

void StateMessageBox::leave()
{
  if(m_receiver != NULL) {
    m_receiver->send(getId(), m_clickedButton, m_msgbox->getTextInput());
  }
}

void StateMessageBox::enterAfterPop()
{
  StateMenu::enterAfterPop();
}

void StateMessageBox::leaveAfterPush()
{
  StateMenu::leaveAfterPush();
}

void StateMessageBox::checkEvents() {
  UIMsgBoxButton Button = m_msgbox->getClicked();

  if(Button == UI_MSGBOX_NOTHING) return;
  m_clickedButton = Button;

  switch(Button) {
  case UI_MSGBOX_OK:
    break;
  case UI_MSGBOX_CANCEL:
    break;
  case UI_MSGBOX_YES:
    break;
  case UI_MSGBOX_NO:
    break;
  case UI_MSGBOX_YES_FOR_ALL:
    break;
  }

  m_requestForEnd = true;
}

bool StateMessageBox::update()
{
  return StateMenu::update();
}

bool StateMessageBox::render()
{
  return StateMenu::render();
}

void StateMessageBox::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {
  default:
    StateMenu::keyDown(nKey, mod, nChar);
    checkEvents();
    break;
  }
}

void StateMessageBox::keyUp(int nKey, SDLMod mod)
{
  StateMenu::keyUp(nKey, mod);
}

void StateMessageBox::mouseDown(int nButton)
{
  StateMenu::mouseDown(nButton);
}

void StateMessageBox::mouseDoubleClick(int nButton)
{
  StateMenu::mouseDoubleClick(nButton);
}

void StateMessageBox::mouseUp(int nButton)
{
  StateMenu::mouseUp(nButton);
}

void StateMessageBox::clean() {
}

void StateMessageBox::createGUI(GameApp* pGame, const std::string& i_text, int i_buttons,
				bool i_input, const std::string& i_inputText, bool i_query) {
  m_GUI = new UIRoot();
  m_GUI->setApp(pGame);
  m_GUI->setFont(pGame->getDrawLib()->getFontSmall()); 
  m_GUI->setPosition(0, 0,
		     pGame->getDrawLib()->getDispWidth(),
		     pGame->getDrawLib()->getDispHeight());

  m_msgbox = m_GUI->msgBox(i_text, (UIMsgBoxButton)(i_buttons), i_input);
  if(i_input) {
    m_msgbox->setTextInputFont(pGame->getDrawLib()->getFontMedium());
    m_msgbox->setTextInput(i_inputText);
  }
}
