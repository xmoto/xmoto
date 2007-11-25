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
#include "StateMainMenu.h"
#include "StateHelp.h"
#include "StateLevelInfoViewer.h"
#include "StateEditProfile.h"
#include "StateEditWebConfig.h"
#include "StateLevelPackViewer.h"
#include "StateRequestKey.h"
#include "Game.h"
#include "XMSession.h"
#include "drawlib/DrawLib.h"
#include "SysMessage.h"
#include "GameText.h"
#include "xmscene/Camera.h"

#define MENU_SHADING_TIME 0.3
#define MENU_SHADING_VALUE 150

StateManager::StateManager(GameApp* pGame)
{
  m_pGame = pGame;

  m_currentRenderFps = 0;
  m_currentUpdateFps = 0;

  m_renderFpsNbFrame = 0;
  m_updateFpsNbFrame = 0;

  m_maxUpdateFps  = 50;
  m_maxRenderFps  = 50;
  m_maxFps        = 50;
  m_renderCounter = 0;
  m_curRenderFps  = 50;

  m_cursor = NULL;
}

StateManager::~StateManager()
{
  while(m_statesStack.size() != 0){
    delete popState();
  }
}

void StateManager::pushState(GameState* pNewState)
{
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
  (m_statesStack.back())->leave();
  GameState* pState = m_statesStack.back();
  m_statesStack.pop_back();
  
  if(m_statesStack.size() != 0)
    (m_statesStack.back())->enterAfterPop();
  
  calculateWhichStateIsRendered();
  calculateFps();

  return pState;
}

void StateManager::flush() {
  while(m_statesStack.size() > 0 && m_statesStack.back()->requestForEnd()) {
    //Logger::Log("Flush %s", m_statesStack.back()->getName().c_str());
    delete popState();
  }

  if(m_statesStack.size() == 0) {
    m_pGame->requestEnd();
  }
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
  // flush states
  flush();

  bool oneUpdate = false;
  // we need a temporary vector to iterate on, because in their update
  // function, some states can push/replace a new state
  std::vector<GameState*> tmp = m_statesStack;
  std::vector<GameState*>::iterator stateIterator = tmp.begin();

  while(stateIterator != tmp.end()){
    (*stateIterator)->executeCommands();
    stateIterator++;
  }

  // flush states
  flush();

  tmp = m_statesStack;
  std::vector<GameState*>::reverse_iterator RstateIterator = tmp.rbegin();
  while(RstateIterator != tmp.rend()){
    if((*RstateIterator)->update() == true){
      oneUpdate = true;
    }

    if((*RstateIterator)->updateStatesBehind() == false)
      break;
    
    RstateIterator++;
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
  if(doRender() == true){
    m_pGame->getDrawLib()->resetGraphics();

    /* we have to draw states from the bottom of the stack to the top */
    std::vector<GameState*>::iterator stateIterator = m_statesStack.begin();

    while(stateIterator != m_statesStack.end()){
      if((*stateIterator)->isHide() == false){
	(*stateIterator)->render();
      }

      stateIterator++;
    }

    // FPS
    if(m_pGame->getSession()->fps()) {
      drawFps();
    }

    // STACK
    //if(m_pGame->getSession()->debug()) {
      drawStack();
      //}

    // SYSMESSAGE
    m_pGame->getSysMessage()->render();

    // CURSOR
    if(m_statesStack.size() > 0) {
      if(m_statesStack.back()->showCursor()) {
	drawCursor();
      }
    } else {
      drawCursor(); // to remove after state managing
    }

    m_pGame->getDrawLib()->flushGraphics();
       
    m_renderFpsNbFrame++;
  }
}

void StateManager::drawFps() {
  char cTemp[256];        
  sprintf(cTemp, "u(%i) d(%i)", getCurrentUpdateFPS(), getCurrentRenderFPS());

  FontManager* v_fm = m_pGame->getDrawLib()->getFontSmall();
  FontGlyph* v_fg = v_fm->getGlyph(cTemp);
  v_fm->printString(v_fg, 0, 130, MAKE_COLOR(255,255,255,255), true);
}

void StateManager::drawStack() {
  int i = 0;
  FontGlyph* v_fg;
  FontManager* v_fm;
  
  int xoff = 0;
  int yoff = m_pGame->getDrawLib()->getDispHeight();
  int w = 180;
  int h =  30;
  Color bg = MAKE_COLOR(255,0,0,200);
  Color font_color = MAKE_COLOR(255,255,255,255);

  std::vector<GameState*>::iterator stateIterator = m_statesStack.begin();
  v_fm = m_pGame->getDrawLib()->getFontSmall();

  while(stateIterator != m_statesStack.end()){
    m_pGame->getDrawLib()->drawBox(Vector2f(xoff, yoff - (i * h)), Vector2f(xoff + w, yoff - ((i+1) * h)), 1.0, bg);
    v_fg = v_fm->getGlyph(m_statesStack[i]->getName());
    v_fm->printString(v_fg, (w-v_fg->realWidth())/2 + xoff, yoff - ((i+1) * h - v_fg->realHeight()/2), font_color, true);
    i++;
    stateIterator++;
  }
}

void StateManager::drawCursor() {
  Sprite* pSprite;

  if(m_cursor == NULL) {
    /* load cursor */
    pSprite = m_pGame->getTheme()->getSprite(SPRITE_TYPE_UI, "Cursor");
    if(pSprite != NULL) {
      m_cursor = pSprite->getTexture(false, true, FM_LINEAR);
    }
  }

  if(m_cursor != NULL && m_pGame->getSession()->ugly() == false) {
    int nMX,nMY;
    GameApp::getMousePos(&nMX, &nMY);      
    m_pGame->getDrawLib()->drawImage(Vector2f(nMX-2,nMY-2), Vector2f(nMX+30,nMY+30), m_cursor);
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
      renderFps         = (*stateIterator)->getRenderFps();
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

//  Logger::Log("MaxUpdateFps: %d MaxRenderFps: %d Max: %d TopStateRenderFps: %d",
//	      m_maxUpdateFps,
//	      m_maxRenderFps,
//	      m_maxFps,
//	      topStateRenderFps);

  stateIterator = m_statesStack.begin();
  while(stateIterator != m_statesStack.end()){
    (*stateIterator)->setCurrentRenderFps(topStateRenderFps);
    (*stateIterator)->setMaxFps(m_maxFps);
    stateIterator++;
  }

  m_curRenderFps = topStateRenderFps;
  m_renderPeriod = (float)m_maxFps / (float)m_curRenderFps;
}

void StateManager::cleanStates() {
  StateDeadMenu::clean();
  StateEditProfile::clean();
  StateEditWebConfig::clean();
  StateFinished::clean();
  StateLevelInfoViewer::clean();
  StateLevelPackViewer::clean();
  StateMainMenu::clean();
  StateMessageBox::clean();
  StatePause::clean();
  StateRequestKey::clean();
}

int StateManager::getCurrentUpdateFPS() {
  return m_currentUpdateFps;
}

int StateManager::getCurrentRenderFPS() {
  return m_currentRenderFps;
}

bool StateManager::doRender()
{
  m_renderCounter += 1.0;

  if(m_renderCounter >= m_renderPeriod){
    m_renderCounter -= m_renderPeriod;
    return true;
  }
  return false;
}

void StateManager::sendSynchronousMessage(std::string cmd)
{
  std::vector<GameState*>::iterator stateIterator = m_statesStack.begin();

  while(stateIterator != m_statesStack.end()){
    (*stateIterator)->executeOneCommand(cmd);

    stateIterator++;
  }
}

void StateManager::sendAsynchronousMessage(std::string cmd)
{
  std::vector<GameState*>::iterator stateIterator = m_statesStack.begin();

  while(stateIterator != m_statesStack.end()){
    (*stateIterator)->send("STATE_MANAGER", cmd);

    stateIterator++;
  }
}

GameState::GameState(bool drawStateBehind,
		     bool updateStatesBehind,
		     GameApp* pGame,
		     bool i_doShade,
		     bool i_doShadeAnim)
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

  m_maxFps             = 0;

  m_updatePeriod       = 0;
  m_updateCounter      = 0;

  // shade
  m_doShade     = i_doShade;
  m_doShadeAnim = i_doShadeAnim;

  m_showCursor = true;
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

void GameState::enter() {
  m_nShadeTime = GameApp::getXMTime();
}

bool GameState::render() {
  // shade
  if(m_pGame->getSession()->ugly() == false && m_doShade) {
    float v_currentTime = GameApp::getXMTime();
    int   v_nShade;
    
    if(v_currentTime - m_nShadeTime < MENU_SHADING_TIME && m_doShadeAnim) {
      v_nShade = (int ) ((v_currentTime - m_nShadeTime) * (MENU_SHADING_VALUE / MENU_SHADING_TIME));
    } else {
      v_nShade = MENU_SHADING_VALUE;
    }
    
    m_pGame->getDrawLib()->drawBox(Vector2f(0,0),
				   Vector2f(m_pGame->getDrawLib()->getDispWidth(),
					    m_pGame->getDrawLib()->getDispHeight()),
				   0,
				   MAKE_COLOR(0,0,0, v_nShade)
				   );
  }
}

std::string GameState::getId() const {
  return m_id;
}

void GameState::setId(const std::string& i_id) {
  m_id = i_id;
}

void GameState::send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  /* by default, do nothing */
  Logger::Log("** Warning ** : StateMessageBoxReceiver::send() received, but nothing done !");
}

void GameState::send(const std::string& i_id, const std::string& i_message)
{
  
}

void GameState::executeCommands()
{
  while(m_commands.empty() == false){
    executeOneCommand(m_commands.front());
    m_commands.pop();
  }

}

void GameState::executeOneCommand(std::string cmd)
{
  // default one do nothing.
}

void GameState::keyDown(int nKey, SDLMod mod,int nChar) {
  if(nKey == SDLK_F12) {
    m_pGame->gameScreenshot();
    return;        
  }

  if(nKey == SDLK_F8) {
    m_pGame->enableWWW(m_pGame->getSession()->www() == false);
    m_pGame->getStateManager()->sendAsynchronousMessage("CHANGE_WWW_ACCESS");
    return;        
  }

  if(nKey == SDLK_F7) {
    m_pGame->enableFps(m_pGame->getSession()->fps() == false);
    return;        
  }

  if(nKey == SDLK_F9) {
    m_pGame->switchUglyMode(m_pGame->getSession()->ugly() == false);
    if(m_pGame->getSession()->ugly()) {
      m_pGame->getSysMessage()->displayText(SYS_MSG_UGLY_MODE_ENABLED);
    } else {
      m_pGame->getSysMessage()->displayText(SYS_MSG_UGLY_MODE_DISABLED);
    }
    return;        
  }

  if(nKey == SDLK_RETURN && (((mod & KMOD_LALT) == KMOD_LALT) || ((mod & KMOD_RALT) == KMOD_RALT))) {
    m_pGame->getDrawLib()->toogleFullscreen();
    m_pGame->getSession()->setWindowed(m_pGame->getSession()->windowed() == false);
    return;
  }

  if(nKey == SDLK_F10) {
    m_pGame->switchTestThemeMode(m_pGame->getSession()->testTheme() == false);
    if(m_pGame->getSession()->testTheme()) {
      m_pGame->getSysMessage()->displayText(SYS_MSG_THEME_MODE_ENABLED);
    } else {
      m_pGame->getSysMessage()->displayText(SYS_MSG_THEME_MODE_DISABLED);
    }
    return;        
  }

  if(nKey == SDLK_F11) {
    m_pGame->switchUglyOverMode(m_pGame->getSession()->uglyOver() == false);
    if(m_pGame->getSession()->uglyOver()) {
      m_pGame->getSysMessage()->displayText(SYS_MSG_UGLY_OVER_MODE_ENABLED);
    } else {
      m_pGame->getSysMessage()->displayText(SYS_MSG_UGLY_OVER_MODE_DISABLED);
    }
    return;        
  }

  /* activate/desactivate interpolation */
  if(nKey == SDLK_i && ( (mod & KMOD_LCTRL) || (mod & KMOD_RCTRL) )) {
    m_pGame->getSession()->setEnableReplayInterpolation(!m_pGame->getSession()->enableReplayInterpolation());
    if(m_pGame->getSession()->enableReplayInterpolation()) {
      m_pGame->getSysMessage()->displayText(SYS_MSG_INTERPOLATION_ENABLED);
    } else {
      m_pGame->getSysMessage()->displayText(SYS_MSG_INTERPOLATION_DISABLED);
    }

    for(unsigned int i=0; i<m_pGame->getMotoGame()->Players().size(); i++) {
      m_pGame->getMotoGame()->Players()[i]->setInterpolation(m_pGame->getSession()->enableReplayInterpolation());
    }

    return;
  }

  if(nKey == SDLK_m && ( (mod & KMOD_LCTRL) || (mod & KMOD_RCTRL) )) {
    for(unsigned int i=0; i<m_pGame->getMotoGame()->Cameras().size(); i++) {
      m_pGame->getMotoGame()->Cameras()[i]->setMirrored(m_pGame->getMotoGame()->Cameras()[i]->isMirrored() == false);
    }
    m_pGame->getInputHandler()->setMirrored(m_pGame->getMotoGame()->Cameras()[0]->isMirrored());
  }

}
