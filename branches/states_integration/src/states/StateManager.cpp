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

  /*
TODO of the states

MENU : une classe mère aux menus

PAUSE : gérer le voulez vous quitter yes/no
FINISHED : gérer le voulez vous quitter yes/no ET sauvegarde du replay
DEADMENU : gérer le voulez vous quitter yes/no ET sauvegarde du replay

  */

StateManager::StateManager()
{
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

  return pPreviousState;
}

void StateManager::update()
{
  std::vector<GameState*>::reverse_iterator stateIterator = m_statesStack.rbegin();

  while(stateIterator != m_statesStack.rend()){
    (*stateIterator)->update();

    if((*stateIterator)->updateStatesBehind() == false)
      break;
    
    stateIterator++;
  }
}

void StateManager::render()
{
  /* we have to draw states from the bottom of the stack to the top */
  std::vector<GameState*>::iterator stateIterator = m_statesStack.begin();

  while(stateIterator != m_statesStack.end()){
    if((*stateIterator)->isHide() == false)
      (*stateIterator)->render();
    
    stateIterator++;
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

void StateManager::cleanStates() {
  StateMessageBox::clean();
  StatePause::clean();
  StateFinished::clean();
  StateDeadMenu::clean();
  StateLevelInfoViewer::clean();
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
}

GameState::~GameState() {
}
