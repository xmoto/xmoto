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

#include "StateEditProfile.h"
#include "StateEditWebConfig.h"
#include "StateMessageBox.h"
#include "common/XMSession.h"
#include "drawlib/DrawLib.h"
#include "helpers/Log.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"

/* static members */
UIRoot *StateEditProfile::m_sGUI = NULL;

StateEditProfile::StateEditProfile(bool drawStateBehind,
                                   bool updateStatesBehind)
  : StateMenu(drawStateBehind, updateStatesBehind) {
  m_name = "StateEditProfile";

  if (XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter(std::string("UPDATEPROFILE"));
  }
}

StateEditProfile::~StateEditProfile() {}

void StateEditProfile::enter() {
  GameApp::instance()->playMenuMusic("menu1");

  createGUIIfNeeded(&m_screen);
  m_GUI = m_sGUI;

  updateOptions();

  StateMenu::enter();
}

void StateEditProfile::highlightSelectedProfile() {
  const auto &selected = XMSession::instance()->profile();
  UIList *pList = reinterpret_cast<UIList *>(
    m_sGUI->getChild("EDITPROFILE_FRAME:PROFILE_LIST"));

  for (int i = 0; i < pList->getEntries().size(); i++) {
    if (pList->getEntries()[i]->Text[0] != selected)
      continue;
    pList->setRealSelected(i);
    break;
  }
}

void StateEditProfile::updateOptions() {
  UIButton *v_button;

  v_button = reinterpret_cast<UIButton *>(
    m_GUI->getChild("EDITPROFILE_FRAME:CHILDRENCOMPLIANT"));
  v_button->setChecked(XMSession::instance()->useChildrenCompliant());

  if (XMSession::instance()->forceChildrenCompliant()) {
    v_button->setChecked(true);
    v_button->enableWindow(false);
  }

  UIButton *pCloseButton = reinterpret_cast<UIButton *>(
    m_sGUI->getChild("EDITPROFILE_FRAME:CLOSE_BUTTON"));

  UIList *list = reinterpret_cast<UIList *>(
    m_sGUI->getChild("EDITPROFILE_FRAME:PROFILE_LIST"));
  if (list->getEntries().size() > 0 && XMSession::instance()->profile() != "") {
    pCloseButton->enableWindow(true);
    highlightSelectedProfile();
  }
}

void StateEditProfile::checkEvents() {
  UIButton *v_button, *v_ccButton;

  // use profile
  v_button = reinterpret_cast<UIButton *>(
    m_GUI->getChild("EDITPROFILE_FRAME:USEPROFILE_BUTTON"));
  if (v_button->isClicked()) {
    v_button->setClicked(false);

    UIList *pList = reinterpret_cast<UIList *>(
      m_sGUI->getChild("EDITPROFILE_FRAME:PROFILE_LIST"));
    if (pList == NULL) {
      throw Exception("EDITPROFILE_FRAME:PROFILE_LIST is NULL");
    }
    unsigned int nIdx = pList->getSelected();
    if (nIdx >= 0 && nIdx < pList->getEntries().size()) {
      UIListEntry *pEntry = pList->getEntries()[nIdx];

      /* save previous profile before loading the previous one */
      XMSession::instance()->saveProfile(xmDatabase::instance("main"));
      Input::instance()->saveConfig(GameApp::instance()->getUserConfig(),
                                    xmDatabase::instance("main"),
                                    XMSession::instance()->profile());

      XMSession::instance()->setProfile(pEntry->Text[0]);
      // set children compliant
      v_ccButton = reinterpret_cast<UIButton *>(
        m_GUI->getChild("EDITPROFILE_FRAME:CHILDRENCOMPLIANT"));
      XMSession::instance()->setToDefault();
      XMSession::instance()->loadProfile(pEntry->Text[0],
                                         xmDatabase::instance("main"));

      XMSession::instance()->loadConfig(
        GameApp::instance()->getUserConfig(),
        /* don't reload the default profile because that breaks profile
           selection */
        false);

      XMSession::instance()->setChildrenCompliant(v_ccButton->getChecked());

      xmDatabase::instance("main")->stats_xmotoStarted(
        XMSession::instance()->sitekey(), pEntry->Text[0]);

      // tell the menu to update the displayed profile
      StateManager::instance()->sendAsynchronousMessage(
        std::string("UPDATEPROFILE"));
    }

    /* Should we jump to the web config now? */
    if (XMSession::instance()->webConfAtInit()) {
      StateManager::instance()->replaceState(new StateEditWebConfig(),
                                             getStateId());
    } else {
      m_requestForEnd = true;
    }
  }

  // new profile
  v_button = reinterpret_cast<UIButton *>(
    m_GUI->getChild("EDITPROFILE_FRAME:NEWPROFILE_BUTTON"));
  if (v_button->isClicked()) {
    v_button->setClicked(false);

    StateMessageBox *v_msgboxState =
      new StateMessageBox(this,
                          std::string(GAMETEXT_ENTERPLAYERNAME) + ":",
                          UI_MSGBOX_OK | UI_MSGBOX_CANCEL,
                          true,
                          "");
    v_msgboxState->setMsgBxId("NEWPROFILE");
    StateManager::instance()->pushState(v_msgboxState);
  }

  // delete profile
  v_button = reinterpret_cast<UIButton *>(
    m_GUI->getChild("EDITPROFILE_FRAME:DELETEPROFILE_BUTTON"));
  if (v_button->isClicked()) {
    v_button->setClicked(false);

    StateMessageBox *v_msgboxState =
      new StateMessageBox(this,
                          std::string(GAMETEXT_DELETEPLAYERMESSAGE),
                          UI_MSGBOX_YES | UI_MSGBOX_NO);
    v_msgboxState->setMsgBxId("DELETEPROFILE");
    StateManager::instance()->pushState(v_msgboxState);
  }

  // close profile menu
  v_button = reinterpret_cast<UIButton *>(
    m_GUI->getChild("EDITPROFILE_FRAME:CLOSE_BUTTON"));
  if (v_button->isClicked()) {
    v_button->setClicked(false);

    m_requestForEnd = true;
  }
}

void StateEditProfile::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (XMSession::instance()->profile() != "" && i_type == INPUT_DOWN &&
      (i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE) ||
       i_xmkey.getJoyButton() == SDL_CONTROLLER_BUTTON_B)) {
    m_requestForEnd = true;
    return;
  }

  StateMenu::xmKey(i_type, i_xmkey);
}

void StateEditProfile::clean() {
  if (StateEditProfile::m_sGUI != NULL) {
    delete StateEditProfile::m_sGUI;
    StateEditProfile::m_sGUI = NULL;
  }
}

void StateEditProfile::createGUIIfNeeded(RenderSurface *i_screen) {
  UIButton *v_button;
  UIFrame *v_frame;

  if (m_sGUI != NULL)
    return;

  DrawLib *drawLib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot(i_screen);
  m_sGUI->setFont(drawLib->getFontSmall());
  m_sGUI->setPosition(
    0, 0, i_screen->getDispWidth(), i_screen->getDispHeight());

  v_frame = new UIFrame(m_sGUI,
                        i_screen->getDispWidth() / 2 - 350,
                        i_screen->getDispHeight() / 2 - 250,
                        "",
                        700,
                        500);
  v_frame->setID("EDITPROFILE_FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_TRANS);

  UIStatic *pProfileEditorTitle = new UIStatic(
    v_frame, 0, 0, GAMETEXT_PLAYERPROFILES, v_frame->getPosition().nWidth, 50);
  pProfileEditorTitle->setFont(drawLib->getFontMedium());

  UIList *pProfileList =
    new UIList(v_frame,
               20,
               50,
               "",
               v_frame->getPosition().nWidth - 20 * 2 - 20 - 207,
               v_frame->getPosition().nHeight - 50 - 30 - 20);
  pProfileList->setFont(drawLib->getFontSmall());
  pProfileList->addColumn(GAMETEXT_PLAYERPROFILE, 128);
  pProfileList->setID("PROFILE_LIST");
  pProfileList->setContextHelp(CONTEXTHELP_SELECT_PLAYER_PROFILE);

  v_button = new UIButton(v_frame,
                          20,
                          v_frame->getPosition().nHeight - 40,
                          GAMETEXT_CHILDREN_COMPLIANT,
                          v_frame->getPosition().nWidth - 20 * 2 - 20 - 207,
                          28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("CHILDRENCOMPLIANT");
  v_button->setFont(drawLib->getFontSmall());
  v_button->setGroup(20023);
  v_button->setContextHelp(CONTEXTHELP_CHILDREN_COMPLIANT);

  v_button = new UIButton(v_frame,
                          v_frame->getPosition().nWidth - 20 - 207,
                          50 + 0 * 57,
                          GAMETEXT_USEPROFILE,
                          207,
                          57);
  v_button->setID("USEPROFILE_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_USE_PLAYER_PROFILE);
  v_button->setFont(drawLib->getFontSmall());
  v_frame->setPrimaryChild(v_button); /* default button */
  pProfileList->setEnterButton(v_button);

  v_button = new UIButton(v_frame,
                          v_frame->getPosition().nWidth - 20 - 207,
                          50 + 1 * 57,
                          GAMETEXT_NEWPROFILE,
                          207,
                          57);
  v_button->setID("NEWPROFILE_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_CREATE_PLAYER_PROFILE);
  v_button->setFont(drawLib->getFontSmall());

  v_button = new UIButton(v_frame,
                          v_frame->getPosition().nWidth - 20 - 207,
                          v_frame->getPosition().nHeight - 40 - 57 - 57,
                          GAMETEXT_DELETEPROFILE,
                          207,
                          57);
  v_button->setID("DELETEPROFILE_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_DELETE_PROFILE);
  v_button->setFont(drawLib->getFontSmall());

  v_button = new UIButton(v_frame,
                          v_frame->getPosition().nWidth - 20 - 207,
                          v_frame->getPosition().nHeight - 40 - 57,
                          GAMETEXT_CLOSE,
                          207,
                          57);
  v_button->setID("CLOSE_BUTTON");
  v_button->setFont(drawLib->getFontSmall());

  createProfileList();
}

void StateEditProfile::createProfileList() {
  if (m_sGUI == NULL)
    return;

  UIList *pList = reinterpret_cast<UIList *>(
    m_sGUI->getChild("EDITPROFILE_FRAME:PROFILE_LIST"));

  char **v_result;
  unsigned int nrow;
  std::string v_profile;

  if (pList != NULL) {
    /* Clear it */
    pList->clear();

    /* Add all player profiles to it */
    v_result = xmDatabase::instance("main")->readDB(
      "SELECT id_profile FROM stats_profiles "
      "WHERE sitekey=\"" +
        xmDatabase::protectString(XMSession::instance()->sitekey()) +
        "\" "
        "ORDER BY id_profile;",
      nrow);
    for (unsigned int i = 0; i < nrow; i++) {
      v_profile = xmDatabase::instance("main")->getResult(v_result, 1, i, 0);
      pList->addEntry(v_profile);
      if (XMSession::instance()->profile() == v_profile) {
        pList->setRealSelected(i);
      }
    }
    xmDatabase::instance("main")->read_DB_free(v_result);

    /* Update buttons */
    UIButton *pUseButton = reinterpret_cast<UIButton *>(
      m_sGUI->getChild("EDITPROFILE_FRAME:USEPROFILE_BUTTON"));
    UIButton *pDeleteButton = reinterpret_cast<UIButton *>(
      m_sGUI->getChild("EDITPROFILE_FRAME:DELETEPROFILE_BUTTON"));
    UIButton *pCloseButton = reinterpret_cast<UIButton *>(
      m_sGUI->getChild("EDITPROFILE_FRAME:CLOSE_BUTTON"));

    if (nrow == 0) {
      pUseButton->enableWindow(false);
      pDeleteButton->enableWindow(false);
      pCloseButton->enableWindow(false);
    } else {
      pUseButton->enableWindow(true);
      pDeleteButton->enableWindow(true);
      pCloseButton->enableWindow(true);
    }

    if (XMSession::instance()->profile() == "") {
      pCloseButton->enableWindow(false);
    }
  }
}

void StateEditProfile::sendFromMessageBox(const std::string &i_id,
                                          UIMsgBoxButton i_button,
                                          const std::string &i_input) {
  if (i_id == "NEWPROFILE") {
    switch (i_button) {
      case UI_MSGBOX_OK: {
        std::string PlayerName = i_input;
        try {
          xmDatabase::instance("main")->stats_createProfile(
            XMSession::instance()->sitekey(), PlayerName);
        } catch (Exception &e) {
          LogWarning("Unable to create the profile");
          break;
        }

        createProfileList();
        highlightSelectedProfile();
        break;
      }
      default:
        break;
    }
  } else if (i_id == "DELETEPROFILE") {
    switch (i_button) {
      case UI_MSGBOX_YES: {
        /* Delete selected profile */
        UIList *pList = reinterpret_cast<UIList *>(
          m_GUI->getChild("EDITPROFILE_FRAME:PROFILE_LIST"));
        if (pList != NULL) {
          unsigned int nIdx = pList->getSelected();
          if (nIdx >= 0 && nIdx < pList->getEntries().size()) {
            UIListEntry *pEntry = pList->getEntries()[nIdx];

            auto &profile = pEntry->Text[0];
            bool isActive = profile == XMSession::instance()->profile();
            xmDatabase::instance("main")->stats_destroyProfile(profile);

            if (isActive)
              XMSession::instance()->setProfile("");

            pList->setRealSelected(0);
            createProfileList();
          }
        }
        break;
      }
      default:
        break;
    }
  } else {
    StateMenu::sendFromMessageBox(i_id, i_button, i_input);
  }
}
