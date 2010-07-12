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
#include "StateMessageBox.h"
#include "StateServerConsole.h"
#include "../net/NetClient.h"
#include "StateHelp.h"
#include "../Renderer.h"

#define MENU_SHADING_TIME 0.3
#define MENU_SHADING_VALUE 150

GameState::GameState(bool drawStateBehind,
		     bool updateStatesBehind)
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

  m_showCursor = true;

  StateManager::instance()->registerAsObserver("CLIENT_DISCONNECTED_BY_ERROR", this);
  StateManager::instance()->registerAsObserver("MYHIGHSCORES_STOLEN", this);

  if(XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("CHANGE_WWW_ACCESS");
    StateManager::instance()->registerAsEmitter("INTERPOLATION_CHANGED");
    StateManager::instance()->registerAsEmitter("MIRRORMODE_CHANGED");
    StateManager::instance()->registerAsEmitter("ENABLEAUDIO_CHANGED");
  }

  m_commandsMutex = SDL_CreateMutex();
}

GameState::~GameState()
{
  StateManager::instance()->unregisterAsObserver("CLIENT_DISCONNECTED_BY_ERROR", this);
  StateManager::instance()->unregisterAsObserver("MYHIGHSCORES_STOLEN", this);
  SDL_DestroyMutex(m_commandsMutex);
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
  if(XMSession::instance()->ugly() == true)
    GameApp::instance()->displayCursor(showCursor());
}

void GameState::enterAfterPop()
{
  if(XMSession::instance()->ugly() == true)
    GameApp::instance()->displayCursor(showCursor());
}

bool GameState::render() {
  return renderOverShadow();
}

bool GameState::renderOverShadow() {
  return true;
}

void GameState::setStateId(const std::string& i_id) {
  m_stateId = i_id;
}

std::string GameState::getStateId() const {
  return m_stateId;
}

void GameState::setStateType(const std::string& i_type) {
  m_stateType = i_type;
}

std::string GameState::getStateType() const {
  return m_stateType;
}

void GameState::sendFromMessageBox(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  if(i_id == "CHATMESSAGE"){
    if(i_button == UI_MSGBOX_OK) {
      NA_chatMessage na(i_input, XMSession::instance()->profile());
      try {
	NetClient::instance()->send(&na, 0);	

	/* and not i_input because na can have transformations */
	SysMessage::instance()->addConsoleLine(NetClient::instance()->getDisplayMessage(na.getMessage(), XMSession::instance()->profile()));
      } catch(Exception &e) {
      }
    }
  }
}

void GameState::send(const std::string& i_message, const std::string& i_args)
{
  SDL_LockMutex(m_commandsMutex);
  m_commands.push(std::pair<std::string, std::string>(i_message, i_args));
  SDL_UnlockMutex(m_commandsMutex);
}

void GameState::executeCommands()
{
  std::string v_cmd, v_args;
  
  // there is not a lot of commands run, thus, lock/unlock for each command instead of global lock should not cost a lot (i hope)
  
  while(true) {
    SDL_LockMutex(m_commandsMutex);
    
    if(m_commands.empty()) {
      SDL_UnlockMutex(m_commandsMutex);
      return; // no more commands to run : finished
 
    } else {
      v_cmd  = m_commands.front().first;
      v_args = m_commands.front().second;
      m_commands.pop();
      SDL_UnlockMutex(m_commandsMutex);
      
      // execute the command, but be sure you've not the mutex on the commands
      executeOneCommand(v_cmd, v_args);
    }

  }
}

void GameState::executeOneCommand(std::string cmd, std::string args)
{
  if(cmd == "CLIENT_DISCONNECTED_BY_ERROR") {
    // to not multiply the messages, only the top message gives the error
    if(StateManager::instance()->isTopOfTheStates(this)) {
      SysMessage::instance()->displayError(GAMETEXT_CLIENTNETWORKERROR);
    }

  } else if(cmd == "MYHIGHSCORES_STOLEN") {
    // to not multiply the messages, only the top message gives the error
    if(StateManager::instance()->isTopOfTheStates(this)) {
      SysMessage::instance()->displayInformation(args);
    }
  } else{

    // default one do nothing.
    LogWarning("cmd [%s [%s]] executed by state [%s], but not handled by it.",
	       cmd.c_str(), args.c_str(), getName().c_str());

  }
}

void GameState::addCommand(std::string cmd, std::string args)
{
  SDL_LockMutex(m_commandsMutex);
  m_commands.push(std::pair<std::string, std::string>(cmd, args));
  SDL_UnlockMutex(m_commandsMutex);
}

void GameState::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  GameApp* gameApp = GameApp::instance();

  if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_F1, KMOD_NONE)) {
    if (StateManager::instance()->isThereASuchState("StateHelp") == false) StateManager::instance()->pushState(new StateHelp(true,false,true, false));
    return;
  }

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
    /* Toggle GFX Modes using F10 */
    if(XMSession::instance()->gameGraphics() == GFX_LOW) {
      XMSession::instance()->setGameGraphics(GFX_MEDIUM);
      SysMessage::instance()->displayText(SYS_MSG_GFX_MEDIUM_ACTIVATED);
    }
    else if(XMSession::instance()->gameGraphics() == GFX_MEDIUM) {
      XMSession::instance()->setGameGraphics(GFX_HIGH);
      SysMessage::instance()->displayText(SYS_MSG_GFX_HIGH_ACTIVATED);
    }
    else {
      XMSession::instance()->setGameGraphics(GFX_LOW);
      SysMessage::instance()->displayText(SYS_MSG_GFX_LOW_ACTIVATED);
    }
    return;
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_F11, KMOD_NONE)) {
    /* F11 toggles TestThemeMode, UglyOver Mode and Normal Mode */
    if(XMSession::instance()->testTheme() == false && XMSession::instance()->uglyOver() == false) {
      gameApp->switchTestThemeMode(true);
      SysMessage::instance()->displayText(SYS_MSG_THEME_MODE_ENABLED);
    } 
    else if(XMSession::instance()->testTheme() == true) {
      gameApp->switchTestThemeMode(false);
      gameApp->switchUglyOverMode(true);
      SysMessage::instance()->displayText(SYS_MSG_UGLY_OVER_MODE_ENABLED);
    }
    else {
      gameApp->switchUglyOverMode(false);
      SysMessage::instance()->displayText(SYS_MSG_NORMAL_MODE_ENABLED);
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

  // net chat
  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_c, KMOD_LCTRL) && NetClient::instance()->isConnected()) {
    if(StateManager::instance()->isThereASuchStateType("CHATMESSAGE") == false) { // do not open several chat box
      std::vector<std::string> clientList = NetClient::instance()->getOtherClientsNameList(" "); // forced to do cause i couldn't include in init netclient function
      NetClient::instance()->addChatTransformations(clientList, " ");
      StateMessageBox* v_msgboxState = new StateMessageBox(NULL, clientList, std::string(GAMETEXT_CHATMESSAGE) + ":", UI_MSGBOX_OK|UI_MSGBOX_CANCEL, true, "", false, true, false, true);
      v_msgboxState->setMsgBxId("CHATMESSAGE");
      v_msgboxState->setStateType("CHATMESSAGE");
      StateManager::instance()->pushState(v_msgboxState);
      return; 
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_s, (SDLMod) (KMOD_LCTRL | KMOD_LALT)) && NetClient::instance()->isConnected()) {
    if(StateManager::instance()->isThereASuchState("StateServerConsole") == false) { // do not open several console
      StateServerConsole* v_console = new StateServerConsole(true, true);
      StateManager::instance()->pushState(v_console);
      return; 
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == InputHandler::instance()->getShowConsole()) {
    SysMessage::instance()->showConsole();
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_t, KMOD_LCTRL)) {
    XMSession::instance()->setEnableTrailCam(!XMSession::instance()->enableTrailCam());
    StateManager::instance()->sendAsynchronousMessage("CHANGE_TRAILCAM");

    if(XMSession::instance()->enableTrailCam()) {
      SysMessage::instance()->displayText(SYS_MSG_TRAILCAM_ACTIVATED);
    } else {
      SysMessage::instance()->displayText(SYS_MSG_TRAILCAM_DEACTIVATED);
    }
  }
}
