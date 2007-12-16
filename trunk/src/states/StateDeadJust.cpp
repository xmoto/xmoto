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
#include "StateDeadMenu.h"
#include "GameText.h"

StateDeadJust::StateDeadJust(GameApp* pGame):
StateScene(pGame, true, true)
{
  m_name    = "StateDeadJust";
}

StateDeadJust::~StateDeadJust()
{

}


void StateDeadJust::enter()
{
  StateScene::enter();
  m_pGame->getMotoGame()->clearGameMessages();
  m_pGame->getMotoGame()->gameMessage(GAMETEXT_JUSTDEAD_RESTART,     false, 15);
  m_pGame->getMotoGame()->gameMessage(GAMETEXT_JUSTDEAD_DISPLAYMENU, false, 15);
  m_pGame->getMotoGame()->setInfos(m_pGame->getMotoGame()->getLevelSrc()->Name());
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
  if(doUpdate() == false){
    return false;
  }
  
  StateScene::update();
}

bool StateDeadJust::render()
{
  StateScene::render();
  return true;
}

void StateDeadJust::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {

  case SDLK_ESCAPE:
    m_pGame->getStateManager()->pushState(new StateDeadMenu(m_pGame, false, this));
    break;

  case SDLK_RETURN:
    if(mod == KMOD_NONE) {
      /* retart immediatly the level */
      restartLevel();
    }
    break;

  default:
    StateScene::keyDown(nKey, mod, nChar);

  }
}

void StateDeadJust::keyUp(int nKey,   SDLMod mod)
{
  StateScene::keyUp(nKey, mod);
}

void StateDeadJust::mouseDown(int nButton)
{
  StateScene::mouseDown(nButton);
}

void StateDeadJust::mouseDoubleClick(int nButton)
{
  StateScene::mouseDoubleClick(nButton);
}

void StateDeadJust::mouseUp(int nButton)
{
  StateScene::mouseUp(nButton);
}
