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
#include "GameState.h"
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
#include "StateOptions.h"
#include "StateServerConsole.h"
#include "StateVote.h"
#include "../net/NetClient.h"

#include "../GameText.h"
#include "../XMSession.h"
#include "../drawlib/DrawLib.h"
#include "../Game.h"
#include "../GeomsManager.h"
#include "../helpers/Log.h"
#include "../SysMessage.h"
#include "../xmscene/Camera.h"
#include "../VideoRecorder.h"
#include "../thread/XMThreadStats.h"
#include "../thread/DownloadReplaysThread.h"
#include <sstream>

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

  m_currentUniqueId = 0;

  // the full xmoto windows it the screen of the statemanager
  m_screen = RenderSurface(Vector2i(0, 0),
			   Vector2i(GameApp::instance()->getDrawLib()->getDispWidth(),
				    GameApp::instance()->getDrawLib()->getDispHeight()));

  // create the db stats thread
  m_xmtstas = new XMThreadStats(XMSession::instance()->sitekey(), this);

  // create the replay downloader thread
  m_drt = new DownloadReplaysThread(this);
}

StateManager::~StateManager()
{
  while(m_statesStack.size() != 0){
    delete popState();
  }

  /* stats */
  if(m_xmtstas != NULL) {
    // do the stats job to confirm there are no more jobs waiting
    m_xmtstas->doJob();
    
    // be sure the thread is finished before closing xmoto
    if(m_xmtstas->waitForThreadEnd()) {
      LogError("stats thread failed");
    }
  }

  /* replays */
  if(m_drt != NULL) {
    // be sure the thread is finished before closing xmoto
    if(m_drt->waitForThreadEnd()) {
      LogError("replays downloader thread failed");
    }
  }


  deleteToDeleteState();

  if(m_videoRecorder != NULL) {
    delete m_videoRecorder;
  }

  m_registeredStates.clear();

  // stats thread
  if(m_xmtstas != NULL) {
    delete m_xmtstas;
  }

}

std::string StateManager::getUniqueId() {
  std::ostringstream v_n;
  v_n << m_currentUniqueId;
  m_currentUniqueId++;
  return v_n.str();
}

void StateManager::setNewStateScreen(GameState* pNewState) {
  /* to test rendering a state to its own render surface, uncomment these :
  if(pNewState->getType() == "SCENE") {
    int offset = 100;
    pNewState->setScreen(RenderSurface(Vector2i(offset, offset),
    				       Vector2i(m_screen.getDispWidth()-offset, m_screen.getDispHeight()-offset)));
  } else {
    pNewState->setScreen(RenderSurface(Vector2i(0, 0),
				       Vector2i(m_screen.getDispWidth(), m_screen.getDispHeight())));
  } */
  pNewState->setScreen(RenderSurface(Vector2i(0, 0),
				     Vector2i(m_screen.getDispWidth(), m_screen.getDispHeight())));
}

void StateManager::pushState(GameState* pNewState)
{
  if(m_statesStack.size() != 0){
    (m_statesStack.back())->leaveAfterPush();
  }

  setNewStateScreen(pNewState);
  m_statesStack.push_back(pNewState);
  pNewState->setStateId(getUniqueId());
  (m_statesStack.back())->enter();

  calculateWhichStateIsRendered();
  calculateFps();
}

GameState* StateManager::popState()
{
  (m_statesStack.back())->leave();
  if(m_statesStack.back()->getType() != "") {
    (m_statesStack.back())->leaveType();
  }
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
    LogDebug("Flush %s", m_statesStack.back()->getName().c_str());
    delete popState();
  }

  deleteToDeleteState();

  if(m_statesStack.size() == 0) {
    GameApp::instance()->requestEnd();
  }
}

void StateManager::replaceState(GameState* pNewState, const std::string& i_parentId)
{
  for(int i=m_statesStack.size()-1; i>=0; i--) {
    if(m_statesStack[i]->getStateId() == i_parentId) {
      m_statesStack[i]->leave();
      if(m_statesStack[i]->getType() != "") {
	if(pNewState->getType() != m_statesStack[i]->getType()) {
	  m_statesStack[i]->leaveType();
	}
      }
      // dude::can't use the same screen size, as a scene screen may
      // be smaller than the whole screen, and so the main menu state
      // which replace it will inherit its screen size.
      //      pNewState->setScreen(*(m_statesStack[i]->getScreen()));
      setNewStateScreen(pNewState);
      m_toDeleteStates.push_back(m_statesStack[i]);
      m_statesStack[i] = pNewState;
      pNewState->setStateId(i_parentId);
      pNewState->enter();
      calculateWhichStateIsRendered();
      calculateFps();
      return;
    }
  }

  throw Exception("No state to replace");
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
    ++stateIterator;
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
    
    ++RstateIterator;
  }

  if(oneUpdate == true){
    m_updateFpsNbFrame++;
  }

  /* update net */
  NetClient::instance()->executeNetActions(xmDatabase::instance("main"));

  /* update fps */
  if(m_lastFpsTime + 1000 < GameApp::getXMTimeInt()) {
    m_currentRenderFps = m_renderFpsNbFrame;
    m_currentUpdateFps = m_updateFpsNbFrame;
    m_renderFpsNbFrame = 0;
    m_updateFpsNbFrame = 0;
    m_lastFpsTime += 1000;
  }
}

void StateManager::renderOverAll() {

  // net infos
  if(NetClient::instance()->isConnected()) {
    FontManager* v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
    FontGlyph* v_fg;
    int vborder = 10;
    int v_voffset = 0;
    
    v_fg = GameApp::instance()->getDrawLib()->getFontSmall()->getGlyph(GAMETEXT_CONNECTED_PLAYERS);
    v_fm->printString(GameApp::instance()->getDrawLib(), v_fg,
		      m_screen.getDispWidth() - v_fg->realWidth() - vborder, vborder,
		      MAKE_COLOR(240,240,240,255), -1.0, true);
    v_voffset += v_fg->realHeight();
    
    // you
    v_fg = GameApp::instance()->getDrawLib()->getFontSmall()->getGlyph(XMSession::instance()->profile());
    v_fm->printString(GameApp::instance()->getDrawLib(), v_fg,
		      m_screen.getDispWidth() - v_fg->realWidth() - vborder, vborder+v_voffset,
		      MAKE_COLOR(200,200,200,255), -1.0, true);     
    v_voffset += v_fg->realHeight();
    
    for(unsigned int i=0; i<NetClient::instance()->otherClients().size(); i++) {
      v_fg = GameApp::instance()->getDrawLib()->getFontSmall()->getGlyph(NetClient::instance()->otherClients()[i]->name());

      v_fm->printString(GameApp::instance()->getDrawLib(), v_fg,
			m_screen.getDispWidth() - v_fg->realWidth() - vborder, vborder+v_voffset,
			MAKE_COLOR(200,200,200,255), -1.0, true);     
      v_voffset += v_fg->realHeight();
    }
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

      ++stateIterator;
    }

    renderOverAll();

    // FPS
    if(XMSession::instance()->fps()) {
      drawFps();
    }

    // STACK
    if(XMSession::instance()->debug()) {
      drawStack();
      drawTexturesLoading();
      drawGeomsLoading();
    }

    // SYSMESSAGE
    SysMessage::instance()->render();

    // CURSOR
    if(m_statesStack.size() > 0) {
      if(m_statesStack.back()->showCursor()) {
	drawCursor();
      }
    }

    drawLib->flushGraphics();
    m_renderFpsNbFrame++;

    stateIterator = m_statesStack.begin();
    while(stateIterator != m_statesStack.end()){
      if((*stateIterator)->isHide() == false){
	(*stateIterator)->onRenderFlush();
      }
      
      ++stateIterator;
    }
  }
}

VideoRecorder* StateManager::getVideoRecorder() {
  return m_videoRecorder;
}

void StateManager::drawFps() {
  char cTemp[128];        

  if(NetClient::instance()->isConnected()) {
    snprintf(cTemp, 128, "u(%i) d(%i) n(%i)", getCurrentUpdateFPS(), getCurrentRenderFPS(), NetClient::instance()->getOwnFrameFPS());
  } else {
    snprintf(cTemp, 128, "u(%i) d(%i)", getCurrentUpdateFPS(), getCurrentRenderFPS());
  }

  FontManager* v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
  FontGlyph* v_fg = v_fm->getGlyph(cTemp);
  v_fm->printString(GameApp::instance()->getDrawLib(), v_fg, 0, 130, MAKE_COLOR(255,255,255,255), -1.0, true);
}

void StateManager::drawTexturesLoading() {
  std::ostringstream v_n;
  v_n << "Textures: " << Theme::instance()->getTextureManager()->getTextures().size();

  FontManager* v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
  FontGlyph* v_fg = v_fm->getGlyph(v_n.str());
  v_fm->printString(GameApp::instance()->getDrawLib(), v_fg, 0, 100, MAKE_COLOR(255,255,255,255), -1.0, true);
}

void StateManager::drawGeomsLoading() {
  std::ostringstream v_n;
  v_n << "Geoms (Blocks/Edges): " << GeomsManager::instance()->getNumberOfBlockGeoms() << "/" << GeomsManager::instance()->getNumberOfEdgeGeoms();

  FontManager* v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
  FontGlyph* v_fg = v_fm->getGlyph(v_n.str());
  v_fm->printString(GameApp::instance()->getDrawLib(), v_fg, 0, 120, MAKE_COLOR(255,255,255,255), -1.0, true);
}

void StateManager::drawStack() {
  int i = 0;
  FontGlyph* v_fg;
  FontManager* v_fm;
  DrawLib* drawLib = GameApp::instance()->getDrawLib();

  int xoff = 0;
  int yoff = m_screen.getDispHeight();
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

    if(m_statesStack[i]->getStateId() == "") {
      v_fg = v_fm->getGlyph(m_statesStack[i]->getName());
    } else {
      v_fg = v_fm->getGlyph(m_statesStack[i]->getName() + " (" + m_statesStack[i]->getStateId() + ")");
    }
    v_fm->printString(drawLib, v_fg, (w-v_fg->realWidth())/2 + xoff, yoff - ((i+1) * h - v_fg->realHeight()/2), font_color, 0.0, true);

    i++;
    ++stateIterator;
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

void StateManager::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  if(m_statesStack.size() == 0)
    return;
  (m_statesStack.back())->xmKey(i_type, i_xmkey);
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
    
    ++stateIterator;
  }
}

void StateManager::calculateFps()
{
  // we can't put zero. because there's divisions by that number, and
  // when there's no states, there are used for frame duration
  // calculation
  int maxUpdateFps = 1;
  int maxRenderFps = 1;

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

    ++stateIterator;
  }

  m_maxUpdateFps = maxUpdateFps;
  m_maxRenderFps = maxRenderFps;
  m_maxFps = (m_maxUpdateFps > m_maxRenderFps) ? m_maxUpdateFps : m_maxRenderFps;

  stateIterator = m_statesStack.begin();
  while(stateIterator != m_statesStack.end()){
    (*stateIterator)->setCurrentRenderFps(topStateRenderFps);
    (*stateIterator)->setMaxFps(m_maxFps);
    ++stateIterator;
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
  StateOptions::clean();
  StateVote::clean();
  StateServerConsole::clean();
}

void StateManager::refreshStaticCaptions() {
//  StateDeadMenu::refreshStaticCaptions();
//  StateEditProfile::refreshStaticCaptions();
//  StateEditWebConfig::refreshStaticCaptions();
//  StateFinished::refreshStaticCaptions();
//  StateLevelInfoViewer::refreshStaticCaptions();
//  StateLevelPackViewer::refreshStaticCaptions();
  StateMainMenu::refreshStaticCaptions();
//  StatePause::refreshStaticCaptions();
//  StateRequestKey::refreshStaticCaptions();
}

bool StateManager::isTopOfTheStates(GameState* i_state) {
  if(m_statesStack.size() == 0) {
    return false;
  }

  return m_statesStack[m_statesStack.size()-1] == i_state;
}

int StateManager::numberOfStates() {
  return m_statesStack.size();
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

void
StateManager::registerAsObserver(std::string message,
				 GameState* self)
{
  std::map<std::string, std::vector<GameState*> >::iterator itFind;

  itFind = m_registeredStates.find(message);
  if(itFind != m_registeredStates.end()){
    if(self != NULL) {

      // check if the state is already registered
      if(XMSession::instance()->debug() == true) {
	std::vector<GameState*>& states = (*itFind).second;
	std::vector<GameState*>::iterator it = states.begin();
	std::string name = self->getName();

	while(it != states.end()){
	  if((*it)->getName() == name){
	    LogWarning("state [%s] already registered as observer for this message [%s].",
			name.c_str(), message.c_str());
	    break;
	  }
	  ++it;
	}
      }

      (*itFind).second.push_back(self);
    }
  } else {
    m_registeredStates[message] = std::vector<GameState*>();
    if(self != NULL) {
      itFind = m_registeredStates.find(message);
      (*itFind).second.push_back(self);
    }
  }
}

void
StateManager::unregisterAsObserver(std::string message, GameState* self)
{
  std::map<std::string, std::vector<GameState*> >::iterator itFind;

  itFind = m_registeredStates.find(message);
  if(itFind != m_registeredStates.end()){
    std::vector<GameState*>& states = (*itFind).second;
    std::vector<GameState*>::iterator stateIterator = states.begin();

    while(stateIterator != states.end()){
      if(self == (*stateIterator)) {
	states.erase(stateIterator);
	return;
      }

      ++stateIterator;
    }
  } else {
    LogWarning("unregisterAsObserver message [%s] state [%], but not registered.",
		message.c_str(),
		self->getName().c_str());
  }
}

void
StateManager::registerAsEmitter(std::string message)
{
  // TODO::show debug informations
  return registerAsObserver(message, NULL);
}

void
StateManager::sendSynchronousMessage(std::string message, std::string args)
{
  std::map<std::string, std::vector<GameState*> >::iterator itFind;

  itFind = m_registeredStates.find(message);
  if(itFind != m_registeredStates.end()){
    std::vector<GameState*>& states = (*itFind).second;
    std::vector<GameState*>::iterator stateIterator = states.begin();

    if(states.size() == 0){
      if(XMSession::instance()->debug()) {
	LogWarning("sendSynchronousMessage message [%s [%s]] sent and there's no state to receive it.",
		   message.c_str(), args.c_str());
      }
    }

    while(stateIterator != states.end()){
      (*stateIterator)->executeOneCommand(message, args);

      LogDebug("sendSynchronousMessage [%s [%s]] to [%s]",
	       message.c_str(), args.c_str(), (*stateIterator)->getName().c_str());

      ++stateIterator;
    }
  } else {
    if(XMSession::instance()->debug()) {
      LogWarning("sendSynchronousMessage message [%s [%s]] sent and there's no state to receive it.",
		 message.c_str(), args.c_str());
    }
  }
}

void
StateManager::sendAsynchronousMessage(std::string message, std::string args)
{
  std::map<std::string, std::vector<GameState*> >::iterator itFind;

  itFind = m_registeredStates.find(message);
  if(itFind != m_registeredStates.end()){
    std::vector<GameState*>& states = (*itFind).second;
    std::vector<GameState*>::iterator stateIterator = states.begin();

    if(states.size() == 0){
      if(XMSession::instance()->debug()) {
	LogWarning("sendAsynchronousMessage message [%s [%s]] sent and there's no state to receive it.",
		   message.c_str(), args.c_str());
      }
    }

    while(stateIterator != states.end()){
      LogDebug("sendAsynchronousMessage [%s [%s]] to [%s]",
	       message.c_str(), args.c_str(),
	       (*stateIterator)->getName().c_str());

      (*stateIterator)->send(message, args);
      ++stateIterator;
    }
  } else {
    if(XMSession::instance()->debug()) {
      LogWarning("sendAsynchronousMessage message [% [%s]s] sent and there's no state to receive it.",
		 message.c_str(),
		 args.c_str());
    }
  }
}

void StateManager::deleteToDeleteState()
{
  while(m_toDeleteStates.size() != 0){
    delete m_toDeleteStates.back();
    m_toDeleteStates.pop_back();
  }
}

bool StateManager::isThereASuchState(const std::string& i_name) {
  for(unsigned int i=0; i<m_statesStack.size(); i++) {
    if(m_statesStack[i]->getName() == i_name) {
      return true;
    }
  }

  return false;
}

bool StateManager::isThereASuchStateType(const std::string& i_type) {
  for(unsigned int i=0; i<m_statesStack.size(); i++) {
    if(m_statesStack[i]->getStateType() == i_type) {
      return true;
    }
  }

  return false;
}

XMThreadStats* StateManager::getDbStatsThread() {
  return m_xmtstas;
}

DownloadReplaysThread* StateManager::getReplayDownloaderThread() {
  return m_drt;
}