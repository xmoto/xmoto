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
#include "Game.h"
#include "XMSession.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"
#include "StateMessageBox.h"
#include "helpers/Log.h"
#include "StateEditWebConfig.h"

/* static members */
UIRoot*  StateEditProfile::m_sGUI = NULL;

StateEditProfile::StateEditProfile(StateMenuContextReceiver* i_receiver,
				   bool drawStateBehind,
				   bool updateStatesBehind
				   ):
  StateMenu(drawStateBehind,
	    updateStatesBehind)
{
  m_name     = "StateEditProfile";
  m_receiver = i_receiver;
}

StateEditProfile::~StateEditProfile()
{
}

void StateEditProfile::enter()
{
  GameApp::instance()->playMusic("menu1");
  
  createGUIIfNeeded();
  m_GUI = m_sGUI;

  StateMenu::enter();
}

void StateEditProfile::checkEvents() {
  UIButton *v_button;

  // use profile
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITPROFILE_FRAME:USEPROFILE_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    UIList *pList = reinterpret_cast<UIList *>(m_sGUI->getChild("EDITPROFILE_FRAME:PROFILE_LIST"));
    if(pList == NULL) {
      throw Exception("EDITPROFILE_FRAME:PROFILE_LIST is NULL");
    }
    unsigned int nIdx = pList->getSelected();
    if(nIdx >= 0 && nIdx < pList->getEntries().size()) {
      UIListEntry *pEntry = pList->getEntries()[nIdx];

      XMSession::instance()->setProfile(pEntry->Text[0]);
      xmDatabase::instance("main")->stats_xmotoStarted(pEntry->Text[0]);

      // tell the menu to update the displayed profile
      if(m_receiver != NULL){
	m_receiver->send(getId(), "UPDATEPROFILE");
      }
    }

    /* Should we jump to the web config now? */
    if(XMSession::instance()->webConfAtInit()) {
      StateManager::instance()->replaceState(new StateEditWebConfig());
    }else{
      m_requestForEnd = true;
    }
  }

  // new profile
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITPROFILE_FRAME:NEWPROFILE_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    
    StateMessageBox* v_msgboxState = new StateMessageBox(this, std::string(GAMETEXT_ENTERPLAYERNAME) + ":",
							 UI_MSGBOX_OK|UI_MSGBOX_CANCEL, true, "");
    v_msgboxState->setId("NEWPROFILE");
    StateManager::instance()->pushState(v_msgboxState);
  }

  // delete profile
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITPROFILE_FRAME:DELETEPROFILE_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    
    StateMessageBox* v_msgboxState = new StateMessageBox(this, std::string(GAMETEXT_DELETEPLAYERMESSAGE),
							 UI_MSGBOX_YES|UI_MSGBOX_NO);
    v_msgboxState->setId("DELETEPROFILE");
    StateManager::instance()->pushState(v_msgboxState);
  }
}

void StateEditProfile::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {

  default:
    StateMenu::keyDown(nKey, mod, nChar);
    break;
  }
}

void StateEditProfile::clean() {
  if(StateEditProfile::m_sGUI != NULL) {
    delete StateEditProfile::m_sGUI;
    StateEditProfile::m_sGUI = NULL;
  }
}

void StateEditProfile::createGUIIfNeeded() {
  UIButton *v_button;
  UIFrame  *v_frame;

  if(m_sGUI != NULL)
    return;

  DrawLib* drawLib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot();
  m_sGUI->setFont(drawLib->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      drawLib->getDispWidth(),
		      drawLib->getDispHeight());

  v_frame = new UIFrame(m_sGUI,
			drawLib->getDispWidth()/2  - 350,
			drawLib->getDispHeight()/2 - 250,
			"", 700, 500);
  v_frame->setID("EDITPROFILE_FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_TRANS);           

  UIStatic *pProfileEditorTitle = new UIStatic(v_frame, 0, 0, GAMETEXT_PLAYERPROFILES, v_frame->getPosition().nWidth, 50);
  pProfileEditorTitle->setFont(drawLib->getFontMedium());

  UIList *pProfileList = new UIList(v_frame, 20, 50, "", v_frame->getPosition().nWidth - 20*2 -20 - 207, v_frame->getPosition().nHeight - 50 - 20);
  pProfileList->setFont(drawLib->getFontSmall());
  pProfileList->addColumn(GAMETEXT_PLAYERPROFILE, 128);
  pProfileList->setID("PROFILE_LIST");
  pProfileList->setContextHelp(CONTEXTHELP_SELECT_PLAYER_PROFILE);

  v_button = new UIButton(v_frame, v_frame->getPosition().nWidth - 20 - 207, 50 + 0*57, GAMETEXT_USEPROFILE, 207, 57);
  v_button->setID("USEPROFILE_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_USE_PLAYER_PROFILE);
  v_button->setFont(drawLib->getFontSmall());
  v_frame->setPrimaryChild(v_button); /* default button */
  pProfileList->setEnterButton(v_button);

  v_button = new UIButton(v_frame, v_frame->getPosition().nWidth - 20 - 207, 50 + 1*57, GAMETEXT_NEWPROFILE, 207, 57);
  v_button->setID("NEWPROFILE_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_CREATE_PLAYER_PROFILE);
  v_button->setFont(drawLib->getFontSmall());

  v_button = new UIButton(v_frame, v_frame->getPosition().nWidth - 20 - 207, v_frame->getPosition().nHeight - 20 - 57,
			  GAMETEXT_DELETEPROFILE, 207, 57);
  v_button->setID("DELETEPROFILE_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_DELETE_PROFILE);
  v_button->setFont(drawLib->getFontSmall());

  createProfileList();
}

void StateEditProfile::createProfileList() {
  if(m_sGUI == NULL)
    return;

  UIList *pList = reinterpret_cast<UIList *>(m_sGUI->getChild("EDITPROFILE_FRAME:PROFILE_LIST"));

  char **v_result;
  unsigned int nrow;
  std::string v_profile;
  
  if(pList != NULL) {
    /* Clear it */
    pList->clear();
    
    /* Add all player profiles to it */
    v_result = xmDatabase::instance("main")->readDB("SELECT id_profile FROM stats_profiles ORDER BY id_profile;",
					nrow);
    for(unsigned int i=0; i<nrow; i++) {
      v_profile = xmDatabase::instance("main")->getResult(v_result, 1, i, 0);
      pList->addEntry(v_profile);
      if(XMSession::instance()->profile() == v_profile) {
	pList->setRealSelected(i);
      }
    }
    xmDatabase::instance("main")->read_DB_free(v_result);
    
    /* Update buttons */
    UIButton *pUseButton = reinterpret_cast<UIButton *>(m_sGUI->getChild("EDITPROFILE_FRAME:USEPROFILE_BUTTON"));
    UIButton *pDeleteButton = reinterpret_cast<UIButton *>(m_sGUI->getChild("EDITPROFILE_FRAME:DELETEPROFILE_BUTTON"));
    
    if(nrow == 0) {
      pUseButton->enableWindow(false);
      pDeleteButton->enableWindow(false);
    } else {
      pUseButton->enableWindow(true);
      pDeleteButton->enableWindow(true);
    }
  }
}

void StateEditProfile::send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input)
{
  if(i_id == "NEWPROFILE"){
    switch(i_button){
    case UI_MSGBOX_OK:{
      std::string PlayerName = i_input;
      try {
	xmDatabase::instance("main")->stats_createProfile(PlayerName);
      } catch(Exception &e) {
	Logger::Log("Unable to create the profile");
      }
      createProfileList();
      break;
    }
    default:
      break;
    }
  }
  else if (i_id == "DELETEPROFILE"){
    switch(i_button){
    case UI_MSGBOX_YES:{
      /* Delete selected profile */
      UIList *pList = reinterpret_cast<UIList *>(m_GUI->getChild("EDITPROFILE_FRAME:PROFILE_LIST"));
      if(pList != NULL) {
	unsigned int nIdx = pList->getSelected();
	if(nIdx >= 0 && nIdx < pList->getEntries().size()) {
	  UIListEntry *pEntry = pList->getEntries()[nIdx];
	  
	  xmDatabase::instance("main")->stats_destroyProfile(pEntry->Text[0]);
	  pList->setRealSelected(0);
	  createProfileList();              
	}
      }
      break;
    }
    default:
      break;
    }
  }
}