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

StateDeadJust::StateDeadJust()
  : StateScene(true, true)
{
  m_name    = "StateDeadJust";
}

StateDeadJust::~StateDeadJust()
{
}


void StateDeadJust::enter()
{
  StateScene::enter();

  MotoGame* world = GameApp::instance()->getMotoGame();
  
  world->clearGameMessages();
  world->gameMessage(GAMETEXT_JUSTDEAD_RESTART,     false, 15);
  world->gameMessage(GAMETEXT_JUSTDEAD_DISPLAYMENU, false, 15);
  world->setInfos(world->getLevelSrc()->Name());
}

void StateDeadJust::keyDown(int nKey, SDLMod mod,int nChar)
{
  if(nKey == SDLK_ESCAPE) {
    StateManager::instance()->pushState(new StateDeadMenu(false, this));
  }
  else if(nKey == SDLK_RETURN && (mod & (KMOD_CTRL|KMOD_SHIFT|KMOD_ALT|KMOD_META)) == 0) {
    /* retart immediatly the level */
    restartLevel();
  }
  else {
    StateScene::keyDown(nKey, mod, nChar);
  }
}
