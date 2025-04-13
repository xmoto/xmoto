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
#include "drawlib/DrawLib.h"
#include "helpers/CmdArgumentParser.h"
#include "xmoto/Game.h"

/* static members */
UIRoot *StateRequestKey::m_sGUI = NULL;

StateRequestKey::StateRequestKey(const std::string &i_txt,
                                 const std::string &i_parentId,
                                 bool drawStateBehind,
                                 bool updateStatesBehind)
  : StateMenu(drawStateBehind, updateStatesBehind) {
  m_txt = i_txt;
  m_name = "StateRequestKey";
  m_parentId = i_parentId;
}

StateRequestKey::~StateRequestKey() {}

void StateRequestKey::enter() {
  createGUIIfNeeded(&m_screen);
  m_GUI = m_sGUI;
  updateGUI();

  StateMenu::enter();
}

void StateRequestKey::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (i_type == INPUT_DOWN &&
      (i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE) ||
       i_xmkey.getJoyButton() == SDL_CONTROLLER_BUTTON_B)) {
    m_requestForEnd = true;
  }

  else {
    if (i_type == INPUT_DOWN) {
      std::string v_msg;

      v_msg = i_xmkey.toString();
      if (v_msg != "") {
        m_requestForEnd = true;
        std::string args = "";
        CmdArgumentParser::instance()->addString(v_msg, args);
        StateManager::instance()->sendAsynchronousMessage(
          "REQUESTKEY", args, m_parentId);
      }
    }
  }
}

void StateRequestKey::clean() {
  if (StateRequestKey::m_sGUI != NULL) {
    delete StateRequestKey::m_sGUI;
    StateRequestKey::m_sGUI = NULL;
  }
}

void StateRequestKey::createGUIIfNeeded(RenderSurface *i_screen) {
  UIStatic *v_someText;
  UIFrame *v_frame;

  if (m_sGUI != NULL)
    return;

  DrawLib *drawLib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot(i_screen);
  m_sGUI->setFont(drawLib->getFontSmall());
  m_sGUI->setPosition(
    0, 0, i_screen->getDispWidth(), i_screen->getDispHeight());

  v_frame = new UIFrame(m_sGUI,
                        30,
                        i_screen->getDispHeight() / 2 - 20,
                        "",
                        i_screen->getDispWidth() - 30 * 2,
                        20 * 2);
  v_frame->setID("FRAME");

  v_someText = new UIStatic(v_frame,
                            0,
                            0,
                            "",
                            v_frame->getPosition().nWidth,
                            v_frame->getPosition().nHeight);
  v_someText->setFont(drawLib->getFontSmall());
  v_someText->setHAlign(UI_ALIGN_CENTER);
  v_someText->setID("TXT");
}

void StateRequestKey::updateGUI() {
  UIStatic *v_someText;

  v_someText = reinterpret_cast<UIStatic *>(m_GUI->getChild("FRAME:TXT"));
  v_someText->setCaption(m_txt);
}
