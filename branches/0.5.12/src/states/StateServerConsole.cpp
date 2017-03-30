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

#include "StateServerConsole.h"
#include "../Game.h"
#include "../GameText.h"
#include "../drawlib/DrawLib.h"
#include "../gui/basic/GUIConsole.h"
#include "../net/NetClient.h"
#include "StateManager.h"

/* static members */
UIRoot*  StateServerConsole::m_sGUI = NULL;

StateServerConsole::StateServerConsole(bool drawStateBehind,
				       bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind) {
  m_name          = "StateServerConsole";

  StateManager::instance()->registerAsObserver(std::string("NET_SRVCMDASW"), this);
}

StateServerConsole::~StateServerConsole() {
  StateManager::instance()->unregisterAsObserver(std::string("NET_SRVCMDASW"), this);
}

void StateServerConsole::enter()
{
  createGUIIfNeeded();
  m_console->reset("banner");
  m_GUI = m_sGUI;

  StateMenu::enter();
}

void StateServerConsole::checkEvents() {
}

void StateServerConsole::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  SDLKey v_nKey;
  SDLMod v_mod;
  std::string v_utf8Char;

  if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE)) {
    m_requestForEnd = true;
    return;
  }
  
  if(i_xmkey.toKeyboard(v_nKey, v_mod, v_utf8Char)) {
    if(i_type == INPUT_DOWN) {
      m_console->keyDown(v_nKey, v_mod, v_utf8Char);
      return;
    }
  }

  StateMenu::xmKey(i_type, i_xmkey);
}

void StateServerConsole::createGUIIfNeeded()
{
  if(m_sGUI != NULL) {
    m_console = reinterpret_cast<UIConsole *>(m_sGUI->getChild("SRVCONSOLE"));
    m_console->setHook(this);
    return;
  }

  DrawLib* drawLib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot(&m_screen);
  m_sGUI->setFont(drawLib->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      m_screen.getDispWidth(),
		      m_screen.getDispHeight());


  m_console = new UIConsole(m_sGUI, 10, 10, "", m_sGUI->getPosition().nWidth-20, m_sGUI->getPosition().nHeight-20);
  m_console->setHook(this);
  m_console->setID("SRVCONSOLE");
  m_console->setFont(drawLib->getFontSmall());

  m_console->addCompletionCommand("help");
  m_console->addCompletionCommand("login");
  m_console->addCompletionCommand("logout");
  m_console->addCompletionCommand("changepassword");
  m_console->addCompletionCommand("lsplayers");
  m_console->addCompletionCommand("lsscores");
  m_console->addCompletionCommand("lsbans");
  m_console->addCompletionCommand("ban");
  m_console->addCompletionCommand("unban");
  m_console->addCompletionCommand("lsadmins");
  m_console->addCompletionCommand("addadmin");
  m_console->addCompletionCommand("rmadmin");
  m_console->addCompletionCommand("reloadrules");
  m_console->addCompletionCommand("stats");
  m_console->addCompletionCommand("msg");
  m_console->addCompletionCommand("ping");
}

void StateServerConsole::clean()
{
  if(StateServerConsole::m_sGUI != NULL) {
    delete StateServerConsole::m_sGUI;
    StateServerConsole::m_sGUI = NULL;
  }
}

void StateServerConsole::exec(const std::string& i_cmd) {
  if(NetClient::instance()->isConnected()) {
    try {
      NA_srvCmd na(i_cmd);
      NetClient::instance()->send(&na, 0);
    } catch(Exception &e) {
      m_console->giveAnswer(GAMETEXT_UNABLETOCONNECTONTHESERVER);
    }
  } else {
    m_console->giveAnswer(GAMETEXT_STATUS_DECONNECTED);
  }
}

void StateServerConsole::exit() {
  m_requestForEnd = true;
}

void StateServerConsole::executeOneCommand(std::string cmd, std::string args) {

  if(cmd == "NET_SRVCMDASW") {
    m_console->giveAnswer(args);
    return;
  }
}
