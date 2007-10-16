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

#include "StateManager.h"
#include "helpers/Log.h"
#include "StatePause.h"
#include "StateFinished.h"
#include "StateDeadMenu.h"
#include "StateMessageBox.h"
#include "StateLevelInfoViewer.h"
#include "Game.h"
#include "XMSession.h"

StateManager::StateManager(GameApp* pGame)
{
  m_pGame = pGame;

  m_currentRenderFps = 0;
  m_currentUpdateFps = 0;

  m_renderFpsNbFrame = 0;
  m_updateFpsNbFrame = 0;

  m_maxUpdateFps = 50;
  m_maxRenderFps = 50;
  m_maxFps       = 50;
}

StateManager::~StateManager()
{
}

void StateManager::pushState(GameState* pNewState)
{
  Logger::Log("pushState");

  if(m_statesStack.size() != 0){
    (m_statesStack.back())->leaveAfterPush();
  }

  m_statesStack.push_back(pNewState);
  (m_statesStack.back())->enter();

  calculateWhichStateIsRendered();
  calculateFps();
}

GameState* StateManager::popState()
{
  Logger::Log("popState");

  (m_statesStack.back())->leave();
  GameState* pState = m_statesStack.back();
  m_statesStack.pop_back();
  
  if(m_statesStack.size() != 0)
    (m_statesStack.back())->enterAfterPop();
  
  calculateWhichStateIsRendered();
  calculateFps();
  
  return pState;
}

GameState* StateManager::flush() {
  if(m_statesStack.size() != 0){
    if(m_statesStack.back()->requestForEnd()) {
      return popState();
    }
  }

  return NULL;
}

GameState* StateManager::replaceState(GameState* pNewState)
{
  GameState* pPreviousState = NULL;

  if(m_statesStack.size() != 0){
    (m_statesStack.back())->leave();
    pPreviousState = m_statesStack.back();
    m_statesStack.pop_back();
  }
  
  m_statesStack.push_back(pNewState);
  (m_statesStack.back())->enter();

  calculateWhichStateIsRendered();
  calculateFps();

  return pPreviousState;
}

void StateManager::update()
{
  bool oneUpdate = false;
  std::vector<GameState*>::reverse_iterator stateIterator = m_statesStack.rbegin();

  while(stateIterator != m_statesStack.rend()){
    if((*stateIterator)->update() == true){
      oneUpdate = true;
    }

    if((*stateIterator)->updateStatesBehind() == false)
      break;
    
    stateIterator++;
  }

  if(oneUpdate == true){
    m_updateFpsNbFrame++;
  }

  /* update fps */
  if(m_lastFpsTime + 1000 < GameApp::getXMTimeInt()) {
    m_currentRenderFps = m_renderFpsNbFrame;
    m_currentUpdateFps = m_updateFpsNbFrame;
    m_renderFpsNbFrame = 0;
    m_updateFpsNbFrame = 0;
    m_lastFpsTime += 1000;
  }
}

void StateManager::render()
{
  bool oneRender = false;
  /* we have to draw states from the bottom of the stack to the top */
  std::vector<GameState*>::iterator stateIterator = m_statesStack.begin();

  while(stateIterator != m_statesStack.end()){
    if((*stateIterator)->isHide() == false){
      if((*stateIterator)->render() == true){
	oneRender = true;
      }
    }
    
    stateIterator++;
  }

  if(oneRender == true){
    m_renderFpsNbFrame++;
  }
}

void StateManager::keyDown(int nKey, SDLMod mod,int nChar)
{
  if(m_statesStack.size() == 0) return;
  (m_statesStack.back())->keyDown(nKey, mod, nChar);
}

void StateManager::keyUp(int nKey,   SDLMod mod)
{
  if(m_statesStack.size() == 0) return;
  (m_statesStack.back())->keyUp(nKey, mod);
}

void StateManager::mouseDown(int nButton)
{
  if(m_statesStack.size() == 0) return;
  (m_statesStack.back())->mouseDown(nButton);
}

void StateManager::mouseDoubleClick(int nButton)
{
  if(m_statesStack.size() == 0) return;
  (m_statesStack.back())->mouseDoubleClick(nButton);
}

void StateManager::mouseUp(int nButton)
{
  if(m_statesStack.size() == 0) return;
  (m_statesStack.back())->mouseUp(nButton);
}

void StateManager::calculateWhichStateIsRendered()
{
  /* calculate which state will be rendered */
  std::vector<GameState*>::reverse_iterator stateIterator = m_statesStack.rbegin();
  bool hideState = false;

  while(stateIterator != m_statesStack.rend()){
    (*stateIterator)->setHide(hideState);

    if(hideState == false
       && (*stateIterator)->drawStatesBehind() == false){
      hideState = true;
    }
    
    stateIterator++;
  }
}

void StateManager::calculateFps()
{
  // while every states are not done, we can't initialize with zero
  int maxUpdateFps = 50;
  int maxRenderFps = 50;

  int topStateRenderFps = 0;

  std::vector<GameState*>::iterator stateIterator = m_statesStack.begin();

  while(stateIterator != m_statesStack.end()){
    int updateFps = (*stateIterator)->getUpdateFps();
    int renderFps = 0;
    if((*stateIterator)->isHide() == false){
      renderFps = (*stateIterator)->getRenderFps();
      topStateRenderFps = renderFps;
    }

    if(updateFps > maxUpdateFps){
      maxUpdateFps = updateFps;
    }
    if(renderFps > maxRenderFps){
      maxRenderFps = renderFps;
    }

    stateIterator++;
  }

  m_maxUpdateFps = maxUpdateFps;
  m_maxRenderFps = maxRenderFps;
  m_maxFps = (m_maxUpdateFps > m_maxRenderFps) ? m_maxUpdateFps : m_maxRenderFps;

  Logger::Log("MaxUpdateFps: %d MaxRenderFps: %d Max: %d TopStateRenderFps: %d",
	      m_maxUpdateFps,
	      m_maxRenderFps,
	      m_maxFps,
	      topStateRenderFps);

  stateIterator = m_statesStack.begin();
  while(stateIterator != m_statesStack.end()){
    (*stateIterator)->setCurrentRenderFps(topStateRenderFps);
    (*stateIterator)->setMaxFps(m_maxFps);
    stateIterator++;
  }
}

void StateManager::cleanStates() {
  StateMessageBox::clean();
  StatePause::clean();
  StateFinished::clean();
  StateDeadMenu::clean();
  StateLevelInfoViewer::clean();
}

int StateManager::getCurrentUpdateFPS() {
  return m_currentUpdateFps;
}

int StateManager::getCurrentRenderFPS() {
  return m_currentRenderFps;
}

GameState::GameState(bool drawStateBehind,
		     bool updateStatesBehind,
		     GameApp* pGame)
{
  m_isHide             = false;
  m_drawStateBehind    = drawStateBehind;
  m_updateStatesBehind = updateStatesBehind;
  m_pGame              = pGame;
  m_requestForEnd      = false;

  // default rendering and update fps
  m_updateFps          = 50;
  m_renderFps          = 50;
  m_curRenderFps       = m_renderFps;

  m_maxFps = m_updatePeriod = m_renderPeriod = 0;

  m_updateCounter      = 0;
  m_renderCounter      = 0;
}

GameState::~GameState() {
}

bool GameState::doUpdate()
{
  m_updateCounter += 1.0;

  if(m_updateCounter >= m_updatePeriod){
    m_updateCounter -= m_updatePeriod;
    return true;
  }
  return false;
}

bool GameState::doRender()
{
  m_renderCounter += 1.0;

  if(m_renderCounter >= m_renderPeriod){
    m_renderCounter -= m_renderPeriod;
    return true;
  }
  return false;
}
