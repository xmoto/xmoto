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

#include "StateVote.h"
#include "../Game.h"
#include "../drawlib/Drawlib.h"

/* static members */
UIRoot*  StateVote::m_sGUI = NULL;

StateVote::StateVote(const std::string& i_idlevel,
		     bool drawStateBehind,
		     bool updateStatesBehind
		     ):
StateMenu(drawStateBehind, updateStatesBehind, true) {
    m_idlevel = i_idlevel;
}


StateVote::~StateVote() {
}

void StateVote::enter() {
  createGUIIfNeeded();
  m_GUI = m_sGUI;

  StateMenu::enter();
}

void StateVote::leave() {
  StateMenu::leave();
}

void StateVote::clean() {
  if(StateVote::m_sGUI != NULL) {
    delete StateVote::m_sGUI;
    StateVote::m_sGUI = NULL;
  }
}

void StateVote::checkEvents() {
}

void StateVote::createGUIIfNeeded() {
  if(m_sGUI != NULL)
    return;

  DrawLib* drawLib = GameApp::instance()->getDrawLib();
  
  m_sGUI = new UIRoot();
  m_sGUI->setFont(drawLib->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      drawLib->getDispWidth(),
		      drawLib->getDispHeight());
}

void StateVote::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE)) {
    /* quit this state */
    m_requestForEnd = true;
  }
}
