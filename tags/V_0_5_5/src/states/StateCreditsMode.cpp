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
#include "../Renderer.h"

StateCreditsMode::StateCreditsMode(Universe* i_universe, GameRenderer* i_renderer, const std::string& i_replay, ReplayBiker* i_replayBiker):
StateReplaying(i_universe, i_renderer, i_replay, i_replayBiker)
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

  if(m_universe != NULL) {
    m_renderer->hideReplayHelp();
    m_renderer->setShowTimePanel(false);
    m_renderer->setShowMinimap(false);
    m_renderer->setShowGhostsText(false);
  }

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
      m_credits->render(&m_screen, m_universe->getScenes()[0]->getTime());
    }
  }

  return true;
}

void StateCreditsMode::abort() {
  m_requestForEnd = true;
  closePlaying();
}

void StateCreditsMode::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  if(!i_xmkey.isDirectionnel()) {
    abort();
  }
}
