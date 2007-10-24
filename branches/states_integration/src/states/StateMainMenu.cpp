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

#include "StateMainMenu.h"

StateMainMenu::StateMainMenu(bool drawStateBehind,
			     bool updateStatesBehind,
			     GameApp* pGame):
  GameState(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{
  m_name    = "StateMainMenu";
}

StateMainMenu::~StateMainMenu()
{

}


void StateMainMenu::enter()
{

}

void StateMainMenu::leave()
{

}

void StateMainMenu::enterAfterPop()
{

}

void StateMainMenu::leaveAfterPush()
{

}

bool StateMainMenu::update()
{
  return false;
}

bool StateMainMenu::render()
{
  return false;
}

void StateMainMenu::keyDown(int nKey, SDLMod mod,int nChar)
{

}

void StateMainMenu::keyUp(int nKey,   SDLMod mod)
{

}

void StateMainMenu::mouseDown(int nButton)
{

}

void StateMainMenu::mouseDoubleClick(int nButton)
{

}

void StateMainMenu::mouseUp(int nButton)
{

}
