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

#include "StatePlaying.h"

StatePlaying::StatePlaying(bool drawStateBehind,
			   bool updateStatesBehind,
			   GameApp* pGame):
  GameState(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{

}

StatePlaying::~StatePlaying()
{

}


void StatePlaying::enter()
{

}

void StatePlaying::leave()
{

}

void StatePlaying::enterAfterPop()
{

}

void StatePlaying::leaveAfterPush()
{

}

bool StatePlaying::update()
{
  return false;
}

bool StatePlaying::render()
{
  return false;
}

void StatePlaying::keyDown(int nKey, SDLMod mod,int nChar)
{

}

void StatePlaying::keyUp(int nKey,   SDLMod mod)
{

}

void StatePlaying::mouseDown(int nButton)
{

}

void StatePlaying::mouseDoubleClick(int nButton)
{

}

void StatePlaying::mouseUp(int nButton)
{

}
