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

#include "StateScene.h"
#include "Game.h"
#include "PhysSettings.h"
#include "xmscene/Camera.h"
#include "XMSession.h"
#include "xmscene/Entity.h"
#include "StateMessageBox.h"

StateScene::StateScene(GameApp* pGame):
  GameState(false, false, pGame)
{
  m_fLastPhysTime = -1.0;
  // while playing, we want 100 fps for the physic
  m_updateFps     = 100;
}

StateScene::~StateScene()
{
}


void StateScene::enter()
{
  m_pGame->setShowCursor(false);
}

void StateScene::leave()
{

}

void StateScene::enterAfterPop()
{
}

void StateScene::leaveAfterPush()
{
}

bool StateScene::update()
{
  int nPhysSteps = 0;

  if(m_fLastPhysTime < 0.0) {
    m_fLastPhysTime = GameApp::getXMTime();
  }

  // don't update if that's not required
  // don't do this infinitely, maximum miss 10 frames, then give up
  while ((m_fLastPhysTime + PHYS_STEP_SIZE <= GameApp::getXMTime()) && (nPhysSteps < 10)) {
    m_pGame->getMotoGame()->updateLevel(PHYS_STEP_SIZE);
    m_fLastPhysTime += PHYS_STEP_SIZE;
    nPhysSteps++;    
  }

  return true;
}

bool StateScene::render()
{
  try {
    for(unsigned int i=0; i<m_pGame->getMotoGame()->getNumberCameras(); i++) {
      m_pGame->getMotoGame()->setCurrentCamera(i);
      m_pGame->getGameRenderer()->render();
    }

#if SIMULATE_SLOW_PHYSICS
      SDL_Delay(SIMULATE_SLOW_PHYSICS);
#endif

    ParticlesSource::setAllowParticleGeneration(m_pGame->getGameRenderer()->nbParticlesRendered() < NB_PARTICLES_TO_RENDER_LIMITATION);
  } catch(Exception &e) {
    m_pGame->getStateManager()->replaceState(new StateMessageBox(NULL, m_pGame, GameApp::splitText(e.getMsg(), 50), UI_MSGBOX_OK));
  }

  return true;
}

void StateScene::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {

  case SDLK_F2:
    m_pGame->switchFollowCamera();
    break;

  case SDLK_F3:
    m_pGame->switchLevelToFavorite(m_pGame->getMotoGame()->getLevelSrc()->Id(), true);
    break;

  }
}

void StateScene::keyUp(int nKey, SDLMod mod)
{
}

void StateScene::mouseDown(int nButton)
{
}

void StateScene::mouseDoubleClick(int nButton)
{
}

void StateScene::mouseUp(int nButton)
{
}

void StateScene::send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  if(i_id == "ERROR") {
    m_pGame->closePlaying();
    m_requestForEnd = true;
    m_pGame->setState(GS_MENU); // to be removed once states will be finished
  }
}
