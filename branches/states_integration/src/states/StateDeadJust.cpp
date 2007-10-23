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

#include "StateDeadJust.h"
#include "Game.h"

StateDeadJust::StateDeadJust(GameApp* pGame) :
  StateScene(pGame)
{

}

StateDeadJust::~StateDeadJust()
{

}


void StateDeadJust::enter()
{

}

void StateDeadJust::leave()
{

}

void StateDeadJust::enterAfterPop()
{

}

void StateDeadJust::leaveAfterPush()
{

}

bool StateDeadJust::update()
{

}

bool StateDeadJust::render()
{

}

void StateDeadJust::keyDown(int nKey, SDLMod mod,int nChar)
{
  m_requestForEnd = true;
  m_pGame->setState(m_pGame->m_StateAfterPlaying);
}

void StateDeadJust::keyUp(int nKey,   SDLMod mod)
{

}

void StateDeadJust::mouseDown(int nButton)
{

}

void StateDeadJust::mouseDoubleClick(int nButton)
{

}

void StateDeadJust::mouseUp(int nButton)
{

}
