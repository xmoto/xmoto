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
#include "Credits.h"
#include "xmscene/BikePlayer.h"
#include "GameText.h"
#include "Game.h"

StateCreditsMode::StateCreditsMode(GameApp* pGame, const std::string& i_replay):
  StateReplaying(pGame, i_replay)
{
  m_credits = new Credits();
}

StateCreditsMode::~StateCreditsMode()
{
  delete m_credits;
}

void StateCreditsMode::enter()
{
  StateReplaying::enter();
  m_pGame->getGameRenderer()->hideReplayHelp();
  m_pGame->getGameRenderer()->setShowTimePanel(false);
  m_pGame->getGameRenderer()->setShowMinimap(false);
  m_pGame->getMotoGame()->setInfos("");
  m_credits->init(m_pGame, m_replayBiker->getFinishTime(), 4, 4, std::string(GAMETEXT_CREDITS).c_str());
}

void StateCreditsMode::leave()
{
  StateReplaying::leave();
}

void StateCreditsMode::enterAfterPop()
{
  StateReplaying::enterAfterPop();
}

void StateCreditsMode::leaveAfterPush()
{
  StateReplaying::leaveAfterPush();
}

void StateCreditsMode::update()
{
  StateReplaying::update();
}

void StateCreditsMode::render()
{
  StateReplaying::render();
  m_credits->render(m_pGame->getMotoGame()->getTime());
}

void StateCreditsMode::keyDown(int nKey, SDLMod mod,int nChar)
{
  m_pGame->closePlaying();
  m_pGame->setState(GS_MENU); // to be removed, just the time states are finished
  m_requestForEnd = true;
}

void StateCreditsMode::keyUp(int nKey,   SDLMod mod)
{
}

void StateCreditsMode::mouseDown(int nButton)
{
}

void StateCreditsMode::mouseDoubleClick(int nButton)
{
}

void StateCreditsMode::mouseUp(int nButton)
{
}
