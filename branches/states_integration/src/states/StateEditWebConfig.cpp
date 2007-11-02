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

#include "StateEditWebConfig.h"

StateEditWebConfig::StateEditWebConfig(GameApp* pGame,
				       bool drawStateBehind,
				       bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{
  m_name    = "StateEditWebConfig";
}

StateEditWebConfig::~StateEditWebConfig()
{

}


void StateEditWebConfig::enter()
{
  StateMenu::enter();
  m_requestForEnd = true;
}

void StateEditWebConfig::leave()
{
  StateMenu::leave();
}

void StateEditWebConfig::enterAfterPop()
{
  StateMenu::enterAfterPop();
}

void StateEditWebConfig::leaveAfterPush()
{
  StateMenu::leaveAfterPush();
}

void StateEditWebConfig::checkEvents()
{
}

bool StateEditWebConfig::update()
{
  return StateMenu::update();
}

bool StateEditWebConfig::render()
{
  return StateMenu::render();
}

void StateEditWebConfig::keyDown(int nKey, SDLMod mod,int nChar)
{
  StateMenu::keyDown(nKey, mod, nChar);
}

void StateEditWebConfig::keyUp(int nKey,   SDLMod mod)
{
  StateMenu::keyUp(nKey, mod);
}

void StateEditWebConfig::mouseDown(int nButton)
{
  StateMenu::mouseDown(nButton);
}

void StateEditWebConfig::mouseDoubleClick(int nButton)
{
  StateMenu::mouseDoubleClick(nButton);
}

void StateEditWebConfig::mouseUp(int nButton)
{
  StateMenu::mouseUp(nButton);
}

void StateEditWebConfig::send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input)
{
}
