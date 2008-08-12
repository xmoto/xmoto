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

#include "StateRequestKey.h"
#include "StateManager.h"
#include "../Game.h"
#include "../drawlib/DrawLib.h"
#include "../helpers/CmdArgumentParser.h"

/* static members */
UIRoot*  StateRequestKey::m_sGUI = NULL;

StateRequestKey::StateRequestKey(const std::string& i_txt,
				 bool drawStateBehind,
				 bool updateStatesBehind):
StateMenu(drawStateBehind,
	  updateStatesBehind,
	  false,
	  true)
{
  m_txt   = i_txt;
  m_name  = "StateRequestKey";
}

StateRequestKey::~StateRequestKey()
{
}

void StateRequestKey::enter()
{
  createGUIIfNeeded();
  m_GUI = m_sGUI;
  updateGUI();

  StateMenu::enter();
}

void StateRequestKey::keyDown(SDLKey nKey, SDLMod mod,int nChar, const std::string& i_utf8Char)
{
  std::string v_msg;

  switch(nKey) {
  case SDLK_ESCAPE:
    m_requestForEnd = true;
    break;

    /* don't allow simple modifier key */
  case SDLK_RSHIFT:
  case SDLK_LSHIFT:
  case SDLK_RCTRL:
  case SDLK_LCTRL:
  case SDLK_RALT:
  case SDLK_LALT:
  case SDLK_RMETA:
  case SDLK_LMETA:
    break;

  default:
    v_msg = XMKey((SDLKey)nKey, mod).toString();

    if(v_msg != "") {
      m_requestForEnd = true;
      std::string args = "";
      CmdArgumentParser::instance()->addString(v_msg, args);
      StateManager::instance()->sendAsynchronousMessage("REQUESTKEY", args);
    }
  }
}

void StateRequestKey::mouseDown(int nButton) {
  std::string v_msg;

  v_msg = XMKey(nButton).toString();

  if(v_msg != "") {
    m_requestForEnd = true;
    std::string args = "";
    CmdArgumentParser::instance()->addString(v_msg, args);
    StateManager::instance()->sendAsynchronousMessage("REQUESTKEY", args);
  }
}

void StateRequestKey::joystickAxisMotion(Uint8 i_joyNum, Uint8 i_joyAxis, Sint16 i_joyAxisValue) {
  std::string v_msg;

  // allow only minimum detection allowed
  if(i_joyAxisValue < INPUT_JOYSTICK_MINIMUM_DETECTION && i_joyAxisValue > -(INPUT_JOYSTICK_MINIMUM_DETECTION)) {
    return;
  }

  v_msg = XMKey(InputHandler::instance()->getJoyId(i_joyNum), i_joyAxis, i_joyAxisValue).toString();

  if(v_msg != "") {
    m_requestForEnd = true;
    std::string args = "";
    CmdArgumentParser::instance()->addString(v_msg, args);
    StateManager::instance()->sendAsynchronousMessage("REQUESTKEY", args);
  }
}

void StateRequestKey::joystickButtonDown(Uint8 i_joyNum, Uint8 i_joyButton) {
  std::string v_msg;

  v_msg = XMKey(InputHandler::instance()->getJoyId(i_joyNum), i_joyButton).toString();

  if(v_msg != "") {
    m_requestForEnd = true;
    std::string args = "";
    CmdArgumentParser::instance()->addString(v_msg, args);
    StateManager::instance()->sendAsynchronousMessage("REQUESTKEY", args);
  }
}

void StateRequestKey::clean() {
  if(StateRequestKey::m_sGUI != NULL) {
    delete StateRequestKey::m_sGUI;
    StateRequestKey::m_sGUI = NULL;
  }
}

void StateRequestKey::createGUIIfNeeded() {
  UIStatic *v_someText;
  UIFrame  *v_frame;

  if(m_sGUI != NULL)
    return;

  DrawLib* drawLib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot();
  m_sGUI->setFont(drawLib->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      drawLib->getDispWidth(),
		      drawLib->getDispHeight());

  v_frame = new UIFrame(m_sGUI,	30,
			drawLib->getDispHeight()/2 - 20, "",
			drawLib->getDispWidth() -30*2, 20*2);
  v_frame->setID("FRAME");

  v_someText = new UIStatic(v_frame, 0, 0, "", v_frame->getPosition().nWidth, v_frame->getPosition().nHeight);
  v_someText->setFont(drawLib->getFontSmall());            
  v_someText->setHAlign(UI_ALIGN_CENTER);
  v_someText->setID("TXT");
}

void StateRequestKey::updateGUI() {
  UIStatic *v_someText;

  v_someText = reinterpret_cast<UIStatic *>(m_GUI->getChild("FRAME:TXT"));
  v_someText->setCaption(m_txt);
}
