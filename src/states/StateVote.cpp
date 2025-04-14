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

#include "StateVote.h"
#include "StateSendVote.h"
#include "drawlib/DrawLib.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"

/* static members */
UIRoot *StateVote::m_sGUI = NULL;

StateVote::StateVote(const std::string &i_idlevel,
                     bool drawStateBehind,
                     bool updateStatesBehind)
  : StateMenu(drawStateBehind, updateStatesBehind) {
  m_idlevel = i_idlevel;
}

StateVote::~StateVote() {}

void StateVote::enter() {
  UIButton *pButtonQ0, *pButtonD0;

  createGUIIfNeeded(&m_screen);
  m_GUI = m_sGUI;

  pButtonD0 = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:D_0"));
  pButtonQ0 = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:Q_0"));
  pButtonQ0->setChecked(true);
  pButtonD0->setChecked(true);

  pButtonD0->makeActive(); // make active a simple radio button to avoid the
  // send button to be pressed by error

  updateRights();

  StateMenu::enter();
}

void StateVote::leave() {
  StateMenu::leave();
}

void StateVote::clean() {
  if (StateVote::m_sGUI != NULL) {
    delete StateVote::m_sGUI;
    StateVote::m_sGUI = NULL;
  }
}

void StateVote::updateRights() {
  UIButton *pButton;

  pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:SEND"));
  if (isToSkip()) {
    pButton->setCaption(GAMETEXT_SKIPTHISREPORT);
    pButton->setContextHelp(CONTEXTHELP_SKIPTHISREPORT);
  } else {
    pButton->setCaption(GAMETEXT_SENDTHISREPORT);
    pButton->setContextHelp(CONTEXTHELP_SENDTHISREPORT);
  }
}

void StateVote::checkEvents() {
  UIButton *pButton;

  std::string v_difficulty;
  std::string v_quality;
  ;

  updateRights();

  pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:SEND"));
  if (pButton->isClicked()) {
    pButton->setClicked(false);

    if (isToSkip()) {
      // the gamer won't have to vote for this level anymore
      xmDatabase::instance("main")->markAsVoted(
        XMSession::instance()->profile(), m_idlevel);
      m_requestForEnd = true;
    } else {
      pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:D_0"));
      if (pButton->getChecked())
        v_difficulty = "0";
      pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:D_1"));
      if (pButton->getChecked())
        v_difficulty = "1";
      pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:D_2"));
      if (pButton->getChecked())
        v_difficulty = "2";
      pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:D_3"));
      if (pButton->getChecked())
        v_difficulty = "3";
      pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:D_4"));
      if (pButton->getChecked())
        v_difficulty = "4";
      pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:D_5"));
      if (pButton->getChecked())
        v_difficulty = "5";
      pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:Q_0"));
      if (pButton->getChecked())
        v_quality = "0";
      pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:Q_1"));
      if (pButton->getChecked())
        v_quality = "1";
      pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:Q_2"));
      if (pButton->getChecked())
        v_quality = "2";
      pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:Q_3"));
      if (pButton->getChecked())
        v_quality = "3";
      pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:Q_4"));
      if (pButton->getChecked())
        v_quality = "4";
      pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:Q_5"));
      if (pButton->getChecked())
        v_quality = "5";

      StateManager::instance()->replaceState(
        new StateSendVote(m_idlevel, v_difficulty, v_quality), getStateId());
    }
  }
}

bool StateVote::isToSkip() {
  UIButton *pButtonQ0, *pButtonD0;
  pButtonD0 = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:D_0"));
  pButtonQ0 = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:Q_0"));
  return pButtonQ0->getChecked() && pButtonD0->getChecked();
}

void StateVote::createGUIIfNeeded(RenderSurface *i_screen) {
  UIStatic *v_someText;
  UIButton *v_button;
  UIFrame *v_frame;

  if (m_sGUI != NULL)
    return;

  DrawLib *drawLib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot(i_screen);
  m_sGUI->setFont(drawLib->getFontSmall());
  m_sGUI->setPosition(
    0, 0, i_screen->getDispWidth(), i_screen->getDispHeight());

  v_frame = new UIFrame(
    m_sGUI, 0, 0, "", i_screen->getDispWidth(), i_screen->getDispHeight());
  v_frame->setID("FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_MENU);

  v_someText = new UIStatic(v_frame,
                            0,
                            m_sGUI->getPosition().nHeight / 5,
                            GAMETEXT_GIVEYOURMING,
                            m_sGUI->getPosition().nWidth,
                            50);
  v_someText->setFont(drawLib->getFontMedium());
  v_someText->setHAlign(UI_ALIGN_CENTER);

  // send
  v_button = new UIButton(v_frame,
                          v_frame->getPosition().nWidth / 2 - 230 / 2,
                          v_frame->getPosition().nHeight - 57 - 30,
                          GAMETEXT_SKIPTHISREPORT,
                          230,
                          57);
  v_button->setID("SEND");
  v_button->setFont(drawLib->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setContextHelp(CONTEXTHELP_SKIPTHISREPORT);

  // difficulty
  v_someText = new UIStatic(v_frame,
                            m_sGUI->getPosition().nWidth / 2 - 150 - 10 + 33,
                            m_sGUI->getPosition().nHeight / 5 + 60,
                            GAMETEXT_DIFFICULTY,
                            150,
                            30);
  v_someText->setFont(drawLib->getFontSmall());
  v_someText->setHAlign(UI_ALIGN_LEFT);

  v_button = new UIButton(v_frame,
                          m_sGUI->getPosition().nWidth / 2 - 150 - 10,
                          m_sGUI->getPosition().nHeight / 5 + 80 + 30,
                          GAMETEXT_NOIDEA,
                          150,
                          28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setGroup(11000);
  v_button->setChecked(true);
  v_button->setID("D_0");
  v_button->setFont(drawLib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_NOIDEA);

  v_button = new UIButton(v_frame,
                          m_sGUI->getPosition().nWidth / 2 - 150 - 10,
                          m_sGUI->getPosition().nHeight / 5 + 80 + 65,
                          GAMETEXT_BEGINNER,
                          150,
                          28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setGroup(11000);
  v_button->setID("D_1");
  v_button->setFont(drawLib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_DBEGINNER);

  v_button = new UIButton(v_frame,
                          m_sGUI->getPosition().nWidth / 2 - 150 - 10,
                          m_sGUI->getPosition().nHeight / 5 + 80 + 100,
                          GAMETEXT_MEDIUM,
                          150,
                          28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setGroup(11000);
  v_button->setID("D_2");
  v_button->setFont(drawLib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_DMEDIUM);

  v_button = new UIButton(v_frame,
                          m_sGUI->getPosition().nWidth / 2 - 150 - 10,
                          m_sGUI->getPosition().nHeight / 5 + 80 + 135,
                          GAMETEXT_EXPERT,
                          150,
                          28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setGroup(11000);
  v_button->setID("D_3");
  v_button->setFont(drawLib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_DEXPERT);

  v_button = new UIButton(v_frame,
                          m_sGUI->getPosition().nWidth / 2 - 150 - 10,
                          m_sGUI->getPosition().nHeight / 5 + 80 + 170,
                          GAMETEXT_MASTER,
                          150,
                          28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setGroup(11000);
  v_button->setID("D_4");
  v_button->setFont(drawLib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_DMASTER);

  v_button = new UIButton(v_frame,
                          m_sGUI->getPosition().nWidth / 2 - 150 - 10,
                          m_sGUI->getPosition().nHeight / 5 + 80 + 205,
                          GAMETEXT_GOD,
                          150,
                          28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setGroup(11000);
  v_button->setID("D_5");
  v_button->setFont(drawLib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_DGOD);

  // quality
  v_someText = new UIStatic(v_frame,
                            m_sGUI->getPosition().nWidth / 2 + 30 + 33,
                            m_sGUI->getPosition().nHeight / 5 + 60,
                            GAMETEXT_QUALITY,
                            150,
                            30);
  v_someText->setFont(drawLib->getFontSmall());
  v_someText->setHAlign(UI_ALIGN_LEFT);

  v_button = new UIButton(v_frame,
                          m_sGUI->getPosition().nWidth / 2 + 30,
                          m_sGUI->getPosition().nHeight / 5 + 80 + 30,
                          GAMETEXT_NOIDEA,
                          150,
                          28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setGroup(12000);
  v_button->setChecked(true);
  v_button->setID("Q_0");
  v_button->setFont(drawLib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_NOIDEA);

  v_button = new UIButton(v_frame,
                          m_sGUI->getPosition().nWidth / 2 + 30,
                          m_sGUI->getPosition().nHeight / 5 + 80 + 65,
                          GAMETEXT_NOTNICEATALL,
                          150,
                          28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setGroup(12000);
  v_button->setID("Q_1");
  v_button->setFont(drawLib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_QNOTNICEATALL);

  v_button = new UIButton(v_frame,
                          m_sGUI->getPosition().nWidth / 2 + 30,
                          m_sGUI->getPosition().nHeight / 5 + 80 + 100,
                          GAMETEXT_NOTNICE,
                          150,
                          28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setGroup(12000);
  v_button->setID("Q_2");
  v_button->setFont(drawLib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_QNOTNICE);

  v_button = new UIButton(v_frame,
                          m_sGUI->getPosition().nWidth / 2 + 30,
                          m_sGUI->getPosition().nHeight / 5 + 80 + 135,
                          GAMETEXT_MEDIUM,
                          150,
                          28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setGroup(12000);
  v_button->setID("Q_3");
  v_button->setFont(drawLib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_QMEDIUM);

  v_button = new UIButton(v_frame,
                          m_sGUI->getPosition().nWidth / 2 + 30,
                          m_sGUI->getPosition().nHeight / 5 + 80 + 170,
                          GAMETEXT_NICE,
                          150,
                          28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setGroup(12000);
  v_button->setID("Q_4");
  v_button->setFont(drawLib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_QNICE);

  v_button = new UIButton(v_frame,
                          m_sGUI->getPosition().nWidth / 2 + 30,
                          m_sGUI->getPosition().nHeight / 5 + 80 + 205,
                          GAMETEXT_REALLYNICE,
                          150,
                          28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setGroup(12000);
  v_button->setID("Q_5");
  v_button->setFont(drawLib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_QREALLYNICE);
}

void StateVote::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (i_type == INPUT_DOWN &&
      (i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE) ||
       i_xmkey.getJoyButton() == SDL_CONTROLLER_BUTTON_B)) {
    /* quit this state */
    m_requestForEnd = true;
  }

  else {
    StateMenu::xmKey(i_type, i_xmkey);
  }
}
