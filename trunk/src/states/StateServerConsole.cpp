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

#include "StateServerConsole.h"
#include "../Game.h"
#include "../drawlib/DrawLib.h"
#include "../gui/basic/GUIConsole.h"

/* static members */
UIRoot*  StateServerConsole::m_sGUI = NULL;

StateServerConsole::StateServerConsole(bool drawStateBehind,
				       bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind) {
  m_name          = "StateServerConsole";
}

StateServerConsole::~StateServerConsole() {
}

void StateServerConsole::enter()
{
  createGUIIfNeeded();
  m_GUI = m_sGUI;

  StateMenu::enter();
}

void StateServerConsole::checkEvents() {
}

void StateServerConsole::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  SDLKey v_nKey;
  SDLMod v_mod;
  std::string v_utf8Char;

  if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE)) {
    m_requestForEnd = true;
    return;
  }
  
  if(i_xmkey.toKeyboard(v_nKey, v_mod, v_utf8Char)) {
    if(i_type == INPUT_DOWN) {
      m_console->keyDown(v_nKey, v_mod, v_utf8Char);
      return;
    }
  }

  StateMenu::xmKey(i_type, i_xmkey);
}

void StateServerConsole::createGUIIfNeeded()
{
  if(m_sGUI != NULL)
    return;

  DrawLib* drawLib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot();
  m_sGUI->setFont(drawLib->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      drawLib->getDispWidth(),
		      drawLib->getDispHeight());


  m_console = new UIConsole(m_sGUI, 10, 10, "", m_sGUI->getPosition().nWidth-20, m_sGUI->getPosition().nHeight-20);
  m_console->setID("SRVCONSOLE");
  m_console->setFont(drawLib->getFontSmall());
}

void StateServerConsole::clean()
{
  if(StateServerConsole::m_sGUI != NULL) {
    delete StateServerConsole::m_sGUI;
    StateServerConsole::m_sGUI = NULL;
  }
}
