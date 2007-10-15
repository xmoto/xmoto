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

#include "StatePreplaying.h"

StatePreplaying::StatePreplaying(bool drawStateBehind,
				 bool updateStatesBehind,
				 GameApp* pGame):
  GameState(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{

}

StatePreplaying::~StatePreplaying()
{

}


void StatePreplaying::enter()
{

}

void StatePreplaying::leave()
{

}

void StatePreplaying::enterAfterPop()
{

}

void StatePreplaying::leaveAfterPush()
{

}

bool StatePreplaying::update()
{
  return false;
}

bool StatePreplaying::render()
{
  return false;
}

void StatePreplaying::keyDown(int nKey, SDLMod mod,int nChar)
{

}

void StatePreplaying::keyUp(int nKey,   SDLMod mod)
{

}

void StatePreplaying::mouseDown(int nButton)
{

}

void StatePreplaying::mouseDoubleClick(int nButton)
{

}

void StatePreplaying::mouseUp(int nButton)
{

}
