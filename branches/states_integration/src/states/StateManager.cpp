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
  m_fLastRenderingTime = -1.0;
  m_fLastUpdateTime    = -1.0;

  m_fLastFpsTime = -1.0;

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
  float fOneRunTime        = GameApp::getXMTime() - m_fLastUpdateTime;
  float fAllowedOneRunTime = 1.0 / ((double)m_maxUpdateFps);

  if(fOneRunTime < fAllowedOneRunTime) {
    return;
  }
  m_fLastUpdateTime = GameApp::getXMTime();

  std::vector<GameState*>::reverse_iterator stateIterator = m_statesStack.rbegin();

  while(stateIterator != m_statesStack.rend()){
    (*stateIterator)->update();

    if((*stateIterator)->updateStatesBehind() == false)
      break;
    
    stateIterator++;
  }

  m_updateFpsNbFrame++;

  /* update fps */
  if(m_fLastFpsTime + 1.0 < GameApp::getXMTime()) {
    m_currentRenderFps = m_renderFpsNbFrame;
    m_currentUpdateFps = m_updateFpsNbFrame;
    m_fLastFpsTime     = GameApp::getXMTime();
    m_renderFpsNbFrame = 0;
    m_updateFpsNbFrame = 0;
  }
}

void StateManager::render()
{
  float fOneRunTime        = GameApp::getXMTime() - m_fLastRenderingTime;
  float fAllowedOneRunTime = 1.0 / ((double)m_maxRenderFps);

  /* don't exceed the fps limitation */
  if(fOneRunTime < fAllowedOneRunTime) {
    return;
  }
  m_fLastRenderingTime = GameApp::getXMTime();

  /* we have to draw states from the bottom of the stack to the top */
  std::vector<GameState*>::iterator stateIterator = m_statesStack.begin();

  while(stateIterator != m_statesStack.end()){
    if((*stateIterator)->isHide() == false)
      (*stateIterator)->render();
    
    stateIterator++;
  }

  m_renderFpsNbFrame++;
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
  int maxUpdateFps = 50;
  int maxRenderFps = 50;

  std::vector<GameState*>::iterator stateIterator = m_statesStack.begin();

  while(stateIterator != m_statesStack.end()){
    int updateFps = (*stateIterator)->getUpdateFps();
    int renderFps = 0;
    if((*stateIterator)->isHide() == false){
      renderFps = (*stateIterator)->getRenderFps();
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

  m_fLastUpdateTime = m_fLastRenderingTime = GameApp::getXMTime();
}

GameState::~GameState() {
}

bool GameState::doUpdate()
{
  float currentTime        = GameApp::getXMTime();
  float fOneRunTime        = currentTime - m_fLastUpdateTime;
  float fAllowedOneRunTime = 1.0 / ((double)m_updateFps);

  /* don't exceed the fps limitation */
  if(fOneRunTime < fAllowedOneRunTime) {
    return false;
  }
  m_fLastUpdateTime = currentTime;

  return true;  
}

bool GameState::doRender()
{
  float currentTime        = GameApp::getXMTime();
  float fOneRunTime        = currentTime - m_fLastRenderingTime;
  float fAllowedOneRunTime = 1.0 / ((double)m_renderFps);

  /* don't exceed the fps limitation */
  if(fOneRunTime < fAllowedOneRunTime) {
    return false;
  }
  m_fLastRenderingTime = currentTime;

  return true;
}
