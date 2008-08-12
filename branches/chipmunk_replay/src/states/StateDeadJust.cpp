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
#include "../Game.h"
#include "StateDeadMenu.h"
#include "../GameText.h"
#include "../Universe.h"

StateDeadJust::StateDeadJust(Universe* i_universe)
: StateScene(i_universe, true, true)
{
  m_name    = "StateDeadJust";
}

StateDeadJust::~StateDeadJust()
{
}

void StateDeadJust::enter()
{
  StateScene::enter();

  if(m_universe != NULL) {
    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->clearGameMessages();
      m_universe->getScenes()[i]->gameMessage(GAMETEXT_JUSTDEAD_RESTART,     false, 15);
      m_universe->getScenes()[i]->gameMessage(GAMETEXT_JUSTDEAD_DISPLAYMENU, false, 15);
      m_universe->getScenes()[i]->setInfos(m_universe->getScenes()[i]->getLevelSrc()->Name());
    }
  }
}

void StateDeadJust::keyDown(SDLKey nKey, SDLMod mod,int nChar, const std::string& i_utf8Char)
{
  if(nKey == SDLK_ESCAPE) {
    StateManager::instance()->pushState(new StateDeadMenu(m_universe, false, this));
  }
  else if(nKey == SDLK_RETURN && (mod & (KMOD_CTRL|KMOD_SHIFT|KMOD_ALT|KMOD_META)) == 0) {
    /* retart immediatly the level */
    restartLevel();
  }
  else {
    StateScene::keyDown(nKey, mod, nChar, i_utf8Char);
  }
}

void StateDeadJust::mouseDown(int nButton) {
  XMKey v_key = XMKey(nButton);

  if(InputHandler::instance()->isANotSetKey(&v_key)) {
    /* retart immediatly the level */
    restartLevel();
  }
}

void StateDeadJust::joystickAxisMotion(Uint8 i_joyNum, Uint8 i_joyAxis, Sint16 i_joyAxisValue) {
  XMKey v_key = XMKey(InputHandler::instance()->getJoyId(i_joyNum), i_joyAxis, i_joyAxisValue);

  if(InputHandler::instance()->isANotSetKey(&v_key)) {
    /* retart immediatly the level */
    restartLevel();
  }
}

void StateDeadJust::joystickButtonDown(Uint8 i_joyNum, Uint8 i_joyButton) {
  XMKey v_key = XMKey(InputHandler::instance()->getJoyId(i_joyNum), i_joyButton);

  if(InputHandler::instance()->isANotSetKey(&v_key)) {
    /* retart immediatly the level */
    restartLevel();
  }
}

void StateDeadJust::restartLevel(bool i_reloadLevel) {
  restartLevelToPlay(i_reloadLevel);
}

void StateDeadJust::nextLevel(bool i_positifOrder) {
  nextLevelToPlay(i_positifOrder);
}
