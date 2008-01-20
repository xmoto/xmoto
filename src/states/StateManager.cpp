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
#include "VideoRecorder.h"

#define MENU_SHADING_TIME 0.3
#define MENU_SHADING_VALUE 150

StateManager::StateManager()
{
  m_currentRenderFps = 0;
  m_currentUpdateFps = 0;

  m_renderFpsNbFrame = 0;
  m_updateFpsNbFrame = 0;

  m_lastFpsTime  = 0;
  m_renderPeriod = 1.0;

  m_maxUpdateFps  = 50;
  m_maxRenderFps  = 50;
  m_maxFps        = 50;
  m_renderCounter = 0;
  m_curRenderFps  = 50;

  m_cursor = NULL;

  // assume focus and visibility at startup
  m_isVisible = true;
  m_hasFocus  = true;

  m_videoRecorder = NULL;
  // video
  if(XMSession::instance()->enableVideoRecording()) {
    m_videoRecorder = new VideoRecorder(XMSession::instance()->videoRecordName(),
					XMSession::instance()->videoRecordingDivision(),
					XMSession::instance()->videoRecordingFramerate());
  }
}

StateManager::~StateManager()
{
  while(m_statesStack.size() != 0){
    delete popState();
  }

  if(m_videoRecorder != NULL) {
    delete m_videoRecorder;
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
    GameApp::instance()->requestEnd();
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

bool StateManager::needUpdateOrRender() {
  if(m_hasFocus) { // m_isVisible is not needed while xmoto is rendered after each event (including expose event)
    return true;
  }

  if(m_statesStack.size() == 0) {
    return false;
  }

  return m_statesStack[m_statesStack.size()-1]->updateWhenUnvisible();
}

void StateManager::update()
{
  // if there is no focus, don't update if the top state won't be updated
  if(m_hasFocus == false) {
    if(m_statesStack.size() > 0) {
      if(m_statesStack[m_statesStack.size()-1]->updateWhenUnvisible() == false) {
	return;
      }
    }
  }

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
  std::vector<GameState*>::iterator stateIterator;

  if(m_isVisible == false) {
    return;
  }

  if(doRender() == true){
    DrawLib* drawLib = GameApp::instance()->getDrawLib();
    drawLib->resetGraphics();

    // erase screen if the first state allow somebody to write before (it means that it has potentially some transparent parts)
    if(m_statesStack.size() > 0) {
      if(m_statesStack[0]->drawStatesBehind()) {
	drawLib->clearGraphics();
      }
    }

    /* we have to draw states from the bottom of the stack to the top */
    stateIterator = m_statesStack.begin();
    while(stateIterator != m_statesStack.end()){
      if((*stateIterator)->isHide() == false){
	(*stateIterator)->render();
      }

      stateIterator++;
    }

    // FPS
    if(XMSession::instance()->fps()) {
      drawFps();
    }

    // STACK
    if(XMSession::instance()->debug()) {
      drawStack();
    }

    // SYSMESSAGE
    SysMessage::instance()->render();

    // CURSOR
    if(m_statesStack.size() > 0) {
      if(m_statesStack.back()->showCursor()) {
	drawCursor();
      }
    } else {
      drawCursor(); // to remove after state managing
    }

    drawLib->flushGraphics();
    m_renderFpsNbFrame++;

    stateIterator = m_statesStack.begin();
    while(stateIterator != m_statesStack.end()){
      if((*stateIterator)->isHide() == false){
	(*stateIterator)->onRenderFlush();
      }
      
      stateIterator++;
    }
  }
}

VideoRecorder* StateManager::getVideoRecorder() {
  return m_videoRecorder;
}

void StateManager::drawFps() {
  char cTemp[256];        
  sprintf(cTemp, "u(%i) d(%i)", getCurrentUpdateFPS(), getCurrentRenderFPS());

  FontManager* v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
  FontGlyph* v_fg = v_fm->getGlyph(cTemp);
  v_fm->printString(v_fg, 0, 130, MAKE_COLOR(255,255,255,255), true);
}

void StateManager::drawStack() {
  int i = 0;
  FontGlyph* v_fg;
  FontManager* v_fm;
  DrawLib* drawLib = GameApp::instance()->getDrawLib();

  int xoff = 0;
  int yoff = drawLib->getDispHeight();
  int w = 180;
  int h =  30;
  Color bg_none     = MAKE_COLOR(0,0,0,200);
  Color bg_updated  = MAKE_COLOR(255,0,0,200);
  Color bg_rendered = MAKE_COLOR(0,255,0,200);
  Color font_color  = MAKE_COLOR(255,255,255,255);

  std::vector<GameState*>::iterator stateIterator = m_statesStack.begin();
  v_fm = drawLib->getFontSmall();

  while(stateIterator != m_statesStack.end()){
    Color bg_render = bg_none;
    Color bg_update = bg_none;

    if(m_statesStack[i]->updateStatesBehind() == true){
      bg_update = bg_updated;
    }
    if(m_statesStack[i]->isHide() == false){
      bg_render = bg_rendered;
    }

    drawLib->drawBox(Vector2f(xoff,     yoff - (i * h)), Vector2f(xoff + w/2, yoff - ((i+1) * h)), 1.0, bg_update);
    drawLib->drawBox(Vector2f(xoff+w/2, yoff - (i * h)), Vector2f(xoff + w,   yoff - ((i+1) * h)), 1.0, bg_render);
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
    pSprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "Cursor");
    if(pSprite != NULL) {
      m_cursor = pSprite->getTexture(false, true, FM_LINEAR);
    }
  }

  if(m_cursor != NULL && XMSession::instance()->ugly() == false) {
    int nMX,nMY;
    GameApp::getMousePos(&nMX, &nMY);      
    GameApp::instance()->getDrawLib()->drawImage(Vector2f(nMX-2,nMY-2), Vector2f(nMX+30,nMY+30), m_cursor, 0xFFFFFFFF, true);
  }
}

void StateManager::keyDown(int nKey, SDLMod mod,int nChar)
{
  if(m_statesStack.size() == 0)
    return;
  (m_statesStack.back())->keyDown(nKey, mod, nChar);
}

void StateManager::keyUp(int nKey,   SDLMod mod)
{
  if(m_statesStack.size() == 0)
    return;
  (m_statesStack.back())->keyUp(nKey, mod);
}

void StateManager::mouseDown(int nButton)
{
  if(m_statesStack.size() == 0)
    return;
  (m_statesStack.back())->mouseDown(nButton);
}

void StateManager::mouseDoubleClick(int nButton)
{
  if(m_statesStack.size() == 0)
    return;
  (m_statesStack.back())->mouseDoubleClick(nButton);
}

void StateManager::mouseUp(int nButton)
{
  if(m_statesStack.size() == 0)
    return;
  (m_statesStack.back())->mouseUp(nButton);
}

void StateManager::changeFocus(bool i_hasFocus) {
  m_hasFocus = i_hasFocus;
}

void StateManager::changeVisibility(bool i_visible) {
  m_isVisible = i_visible;
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
  StatePause::clean();
  StateRequestKey::clean();
}

bool StateManager::isTopOfTheStates(GameState* i_state) {
  if(m_statesStack.size() == 0) {
    return false;
  }

  return m_statesStack[m_statesStack.size()-1] == i_state;
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

  if(XMSession::instance()->timedemo()) {
    return true;
  }

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
		     bool i_doShade,
		     bool i_doShadeAnim)
{
  m_isHide             = false;
  m_drawStateBehind    = drawStateBehind;
  m_updateStatesBehind = updateStatesBehind;
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
  if(XMSession::instance()->ugly() == false && m_doShade) {
    float v_currentTime = GameApp::getXMTime();
    int   v_nShade;
    
    if(v_currentTime - m_nShadeTime < MENU_SHADING_TIME && m_doShadeAnim) {
      v_nShade = (int ) ((v_currentTime - m_nShadeTime) * (MENU_SHADING_VALUE / MENU_SHADING_TIME));
    } else {
      v_nShade = MENU_SHADING_VALUE;
    }

    DrawLib* drawLib = GameApp::instance()->getDrawLib();
    drawLib->drawBox(Vector2f(0,0),
		     Vector2f(drawLib->getDispWidth(),
			      drawLib->getDispHeight()),
		     0, MAKE_COLOR(0,0,0, v_nShade));
  }

  return true;
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
  GameApp* gameApp = GameApp::instance();

  if(nKey == SDLK_F12) {
    gameApp->gameScreenshot();
    return;        
  }

  if(nKey == SDLK_F8) {
    gameApp->enableWWW(XMSession::instance()->www() == false);
    StateManager::instance()->sendAsynchronousMessage("CHANGE_WWW_ACCESS");
    return;        
  }

  if(nKey == SDLK_F7) {
    gameApp->enableFps(XMSession::instance()->fps() == false);
    return;        
  }

  if(nKey == SDLK_F9) {
    gameApp->switchUglyMode(XMSession::instance()->ugly() == false);
    if(XMSession::instance()->ugly()) {
      SysMessage::instance()->displayText(SYS_MSG_UGLY_MODE_ENABLED);
    } else {
      SysMessage::instance()->displayText(SYS_MSG_UGLY_MODE_DISABLED);
    }
    return;        
  }

  if(nKey == SDLK_RETURN && (mod & KMOD_ALT) != 0) {
    gameApp->getDrawLib()->toogleFullscreen();
    XMSession::instance()->setWindowed(XMSession::instance()->windowed() == false);
    return;
  }

  if(nKey == SDLK_F10) {
    gameApp->switchTestThemeMode(XMSession::instance()->testTheme() == false);
    if(XMSession::instance()->testTheme()) {
      SysMessage::instance()->displayText(SYS_MSG_THEME_MODE_ENABLED);
    } else {
      SysMessage::instance()->displayText(SYS_MSG_THEME_MODE_DISABLED);
    }
    return;        
  }

  if(nKey == SDLK_F11) {
    gameApp->switchUglyOverMode(XMSession::instance()->uglyOver() == false);
    if(XMSession::instance()->uglyOver()) {
      SysMessage::instance()->displayText(SYS_MSG_UGLY_OVER_MODE_ENABLED);
    } else {
      SysMessage::instance()->displayText(SYS_MSG_UGLY_OVER_MODE_DISABLED);
    }
    return;        
  }

  /* activate/desactivate interpolation */
  if(nKey == SDLK_i && (mod & KMOD_CTRL) != 0) {
    XMSession::instance()->setEnableReplayInterpolation(!XMSession::instance()->enableReplayInterpolation());
    if(XMSession::instance()->enableReplayInterpolation()) {
      SysMessage::instance()->displayText(SYS_MSG_INTERPOLATION_ENABLED);
    } else {
      SysMessage::instance()->displayText(SYS_MSG_INTERPOLATION_DISABLED);
    }
    StateManager::instance()->sendAsynchronousMessage("INTERPOLATION_CHANGED");

    return;
  }

  if(nKey == SDLK_m && (mod & KMOD_CTRL) != 0) {
    XMSession::instance()->setMirrorMode(XMSession::instance()->mirrorMode() == false);
    InputHandler::instance()->setMirrored(XMSession::instance()->mirrorMode());
    StateManager::instance()->sendAsynchronousMessage("MIRRORMODE_CHANGED");
  }

}
