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

#include "StateCreditsMode.h"
#include "../Credits.h"
#include "../xmscene/BikePlayer.h"
#include "../GameText.h"
#include "../Game.h"
#include "../Universe.h"

StateCreditsMode::StateCreditsMode(Universe* i_universe, const std::string& i_replay, ReplayBiker* i_replayBiker):
StateReplaying(i_universe, i_replay, i_replayBiker)
{
  m_credits = new Credits();
  m_name    = "StateCreditsMode";
}

StateCreditsMode::~StateCreditsMode()
{
  delete m_credits;
}

void StateCreditsMode::enter()
{
  StateReplaying::enter();

  GameRenderer* renderer = GameRenderer::instance();
  renderer->hideReplayHelp();
  renderer->setShowTimePanel(false);
  renderer->setShowMinimap(false);

  if(m_universe != NULL) {
    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->setInfos("");
    }
  }

  m_credits->init(m_replayBiker->getFinishTime(), 400, 400, std::string(GAMETEXT_CREDITS).c_str());
}

bool StateCreditsMode::render()
{
  if(StateReplaying::render() == false){
    return false;
  }

  if(m_universe != NULL) {
    if(m_universe->getScenes().size() > 0) {
      m_credits->render(m_universe->getScenes()[0]->getTime());
    }
  }

  return true;
}

void StateCreditsMode::abort() {
  m_requestForEnd = true;
  closePlaying();
}

void StateCreditsMode::keyDown(SDLKey nKey, SDLMod mod,int nChar, const std::string& i_utf8Char)
{
  abort();
}

void StateCreditsMode::keyUp(SDLKey nKey, SDLMod mod, const std::string& i_utf8Char)
{
  /* declare to be sure it does nothing */
}

void StateCreditsMode::mouseDown(int nButton)
{
  abort();
}

void StateCreditsMode::mouseDoubleClick(int nButton)
{
  abort();
}

void StateCreditsMode::mouseUp(int nButton)
{
  /* declare to be sure it does nothing */
}

void StateCreditsMode::joystickAxisMotion(Uint8 i_joyNum, Uint8 i_joyAxis, Sint16 i_joyAxisValue) {
  abort();
}

void StateCreditsMode::joystickButtonDown(Uint8 i_joyNum, Uint8 i_joyButton) {
  abort();
}

void StateCreditsMode::joystickButtonUp(Uint8 i_joyNum, Uint8 i_joyButton) {
  /* declare to be sure it does nothing */
}
