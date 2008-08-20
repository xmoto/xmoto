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

#include "GameState.h"
#include "StateManager.h"
#include "../XMSession.h"
#include "../helpers/Log.h"
#include "../Game.h"
#include "../SysMessage.h"
#include "../GameText.h"
#include "../drawlib/DrawLib.h"
#include "../Sound.h"
#include "StateOptions.h"

#define MENU_SHADING_TIME 0.3
#define MENU_SHADING_VALUE 150

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
  m_renderFps          = XMSession::instance()->maxRenderFps();;
  m_curRenderFps       = m_renderFps;

  m_maxFps             = 0;

  m_updatePeriod       = 0;
  m_updateCounter      = 0;

  // shade
  m_doShade     = i_doShade;
  m_doShadeAnim = i_doShadeAnim;

  m_showCursor = true;

  if(XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("CHANGE_WWW_ACCESS");
    StateManager::instance()->registerAsEmitter("INTERPOLATION_CHANGED");
    StateManager::instance()->registerAsEmitter("MIRRORMODE_CHANGED");
    StateManager::instance()->registerAsEmitter("ENABLEAUDIO_CHANGED");
  }
}

GameState::~GameState()
{
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
  if(XMSession::instance()->ugly() == true)
    GameApp::instance()->displayCursor(showCursor());
}

void GameState::enterAfterPop()
{
  if(XMSession::instance()->ugly() == true)
    GameApp::instance()->displayCursor(showCursor());
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

  return renderOverShadow();
}

bool GameState::renderOverShadow() {
  // do nothing by default
  return true;
}

std::string GameState::getId() const {
  return m_id;
}

void GameState::setId(const std::string& i_id) {
  m_id = i_id;
}

void GameState::sendFromMessageBox(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  /* by default, do nothing */
  LogWarning("StateMessageBoxReceiver::send() received, but nothing done !");
}

void GameState::send(const std::string& i_message, const std::string& i_args)
{
  m_commands.push(std::pair<std::string, std::string>(i_message, i_args));
}

void GameState::executeCommands()
{
  while(m_commands.empty() == false){
    executeOneCommand(m_commands.front().first, m_commands.front().second);
    m_commands.pop();
  }

}

void GameState::executeOneCommand(std::string cmd, std::string args)
{
  // default one do nothing.
  LogWarning("cmd [%s [%s]] executed by state [%s], but not handled by it.",
	      cmd.c_str(), args.c_str(), getName().c_str());
}

void GameState::addCommand(std::string cmd, std::string args)
{
  m_commands.push(std::pair<std::string, std::string>(cmd, args));
}

void GameState::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  GameApp* gameApp = GameApp::instance();

  if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_F12, KMOD_NONE)) {
    gameApp->gameScreenshot();
    return; 
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_F8, KMOD_NONE)) {
    gameApp->enableWWW(XMSession::instance()->www() == false);
    StateManager::instance()->sendAsynchronousMessage("CHANGE_WWW_ACCESS");
    return;
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_F7, KMOD_NONE)) {
    gameApp->enableFps(XMSession::instance()->fps() == false);
    return;
  }

  else if(i_type == INPUT_DOWN && i_xmkey == InputHandler::instance()->getSwitchUglyMode()) {
    gameApp->switchUglyMode(XMSession::instance()->ugly() == false);
    if(XMSession::instance()->ugly()) {
      SysMessage::instance()->displayText(SYS_MSG_UGLY_MODE_ENABLED);
    } else {
      SysMessage::instance()->displayText(SYS_MSG_UGLY_MODE_DISABLED);
    }
    
    if(XMSession::instance()->ugly() == true)
      GameApp::instance()->displayCursor(showCursor());
    
    return;
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_F10, KMOD_NONE)) {
    gameApp->switchTestThemeMode(XMSession::instance()->testTheme() == false);
    if(XMSession::instance()->testTheme()) {
      SysMessage::instance()->displayText(SYS_MSG_THEME_MODE_ENABLED);
    } else {
      SysMessage::instance()->displayText(SYS_MSG_THEME_MODE_DISABLED);
    }
    return;
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_F11, KMOD_NONE)) {
    gameApp->switchUglyOverMode(XMSession::instance()->uglyOver() == false);
    if(XMSession::instance()->uglyOver()) {
      SysMessage::instance()->displayText(SYS_MSG_UGLY_OVER_MODE_ENABLED);
    } else {
      SysMessage::instance()->displayText(SYS_MSG_UGLY_OVER_MODE_DISABLED);
    }
    return;
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_RETURN, KMOD_LALT)) {
    gameApp->getDrawLib()->toogleFullscreen();
    XMSession::instance()->setWindowed(XMSession::instance()->windowed() == false);
    return;
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_i, KMOD_LCTRL)) {
    /* activate/desactivate interpolation */
    XMSession::instance()->setEnableReplayInterpolation(!XMSession::instance()->enableReplayInterpolation());
    if(XMSession::instance()->enableReplayInterpolation()) {
      SysMessage::instance()->displayText(SYS_MSG_INTERPOLATION_ENABLED);
    } else {
      SysMessage::instance()->displayText(SYS_MSG_INTERPOLATION_DISABLED);
    }
    StateManager::instance()->sendAsynchronousMessage("INTERPOLATION_CHANGED");

    return;
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_m, KMOD_LCTRL)) {
    XMSession::instance()->setMirrorMode(XMSession::instance()->mirrorMode() == false);
    InputHandler::instance()->setMirrored(XMSession::instance()->mirrorMode());
    StateManager::instance()->sendAsynchronousMessage("MIRRORMODE_CHANGED");
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_o, KMOD_LCTRL)) {
    if(StateManager::instance()->isThereASuchState("StateOptions") == false) { // do not open stateOptions over stateOptions
      StateManager::instance()->pushState(new StateOptions());
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_s, KMOD_LCTRL)) {
    GameApp::instance()->toogleEnableMusic();
  }
}
