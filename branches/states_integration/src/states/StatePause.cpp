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

#include "StatePause.h"
#include "Game.h"
#include "XMSession.h"
#include "drawlib/DrawLib.h"

StatePause::StatePause(GameApp* pGame,
		       bool drawStateBehind,
		       bool updateStatesBehind):
  GameState(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{
}

StatePause::~StatePause()
{

}

void StatePause::enter()
{
  m_nPauseShade = 0; 
  m_pGame->m_State = GS_PAUSE; // to be removed, just the time states are finished
  m_pGame->getMotoGame()->setInfos(m_pGame->getMotoGame()->getLevelSrc()->Name());
  m_pGame->setShowCursor(true);
  
  // m_pPauseMenu->showWindow(true);
}

void StatePause::leave()
{
  m_pGame->m_State = GS_PLAYING; // to be removed, just the time states are finished
  m_pGame->getMotoGame()->setInfos("");
  // m_pPauseMenu->showWindow(false);
}

void StatePause::enterAfterPop()
{

}

void StatePause::leaveAfterPush()
{

}

void StatePause::update()
{

}

void StatePause::render()
{
  // rendering of the gui must be done by the mother call : to add here when states will be almost finished
  m_pGame->setFrameDelay(10);

  if(m_pGame->getSession()->ugly() == false) {
    if(m_nPauseShade < 150) m_nPauseShade+=8;

    m_pGame->getDrawLib()->drawBox(Vector2f(0,0),
				   Vector2f(m_pGame->getDrawLib()->getDispWidth(),
					    m_pGame->getDrawLib()->getDispHeight()),
				   0,
				   MAKE_COLOR(0,0,0, m_nPauseShade)
				   );
  }

//    /* Update mouse stuff */
//    _DispatchMouseHover();
//    
//    /* Blah... */
//    _HandlePauseMenu();
}

void StatePause::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {

  case SDLK_ESCAPE:
    /* quit this state */
    m_requestForEnd = true;
    break;

//  case SDLK_F3:
//    m_pGame->switchLevelToFavorite(m_MotoGame.getLevelSrc()->Id(), true);
//    break;
//
//  default:
//    m_Renderer->getGUI()->keyDown(nKey, mod,nChar);
//    break;      
//
  }
}

void StatePause::keyUp(int nKey,   SDLMod mod)
{

}

void StatePause::mouseDown(int nButton)
{

}

void StatePause::mouseDoubleClick(int nButton)
{

}

void StatePause::mouseUp(int nButton)
{

}
