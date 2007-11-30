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

StateEditProfile::StateEditProfile(GameApp* pGame,
				   StateMenuContextReceiver* i_receiver,
				   bool drawStateBehind,
				   bool updateStatesBehind
				   ):
  StateMenu(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{
  m_name     = "StateEditProfile";
  m_receiver = i_receiver;
}

StateEditProfile::~StateEditProfile()
{

}


void StateEditProfile::enter()
{
  m_pGame->playMusic("menu1");
  
  createGUIIfNeeded(m_pGame);
  m_GUI = m_sGUI;

  StateMenu::enter();
}

void StateEditProfile::leave()
{
  StateMenu::leave();
}

void StateEditProfile::enterAfterPop()
{
  StateMenu::enterAfterPop();
}

void StateEditProfile::leaveAfterPush()
{
  StateMenu::leaveAfterPush();
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
    int nIdx = pList->getSelected();
    if(nIdx >= 0 && nIdx < pList->getEntries().size()) {
      UIListEntry *pEntry = pList->getEntries()[nIdx];

      m_pGame->getSession()->setProfile(pEntry->Text[0]);
      m_pGame->getDb()->stats_xmotoStarted(pEntry->Text[0]);

      // tell the menu to update the displayed profile
      if(m_receiver != NULL){
	m_receiver->send(getId(), "UPDATEPROFILE");
      }
    }

    /* Should we jump to the web config now? */
    if(m_pGame->getSession()->webConfAtInit()) {
      m_pGame->getStateManager()->replaceState(new StateEditWebConfig(m_pGame));
    }else{
      m_requestForEnd = true;
    }
  }

  // new profile
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITPROFILE_FRAME:NEWPROFILE_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    
    StateMessageBox* v_msgboxState = new StateMessageBox(this, m_pGame, std::string(GAMETEXT_ENTERPLAYERNAME) + ":",
							 UI_MSGBOX_OK|UI_MSGBOX_CANCEL, true, "");
    v_msgboxState->setId("NEWPROFILE");
    m_pGame->getStateManager()->pushState(v_msgboxState);
  }

  // delete profile
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITPROFILE_FRAME:DELETEPROFILE_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    
    StateMessageBox* v_msgboxState = new StateMessageBox(this, m_pGame, std::string(GAMETEXT_DELETEPLAYERMESSAGE),
							 UI_MSGBOX_YES|UI_MSGBOX_NO);
    v_msgboxState->setId("DELETEPROFILE");
    m_pGame->getStateManager()->pushState(v_msgboxState);
  }
}

bool StateEditProfile::update()
{
  return StateMenu::update();
}

bool StateEditProfile::render()
{
  return StateMenu::render();
}

void StateEditProfile::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {

  default:
    StateMenu::keyDown(nKey, mod, nChar);
    checkEvents();
    break;

  }
}

void StateEditProfile::keyUp(int nKey,   SDLMod mod)
{
  StateMenu::keyUp(nKey, mod);
}

void StateEditProfile::mouseDown(int nButton)
{
  StateMenu::mouseDown(nButton);
}

void StateEditProfile::mouseDoubleClick(int nButton)
{
  StateMenu::mouseDoubleClick(nButton);
}

void StateEditProfile::mouseUp(int nButton)
{
  StateMenu::mouseUp(nButton);
}

void StateEditProfile::clean() {
  if(StateEditProfile::m_sGUI != NULL) {
    delete StateEditProfile::m_sGUI;
    StateEditProfile::m_sGUI = NULL;
  }
}

void StateEditProfile::createGUIIfNeeded(GameApp* pGame) {
  UIButton *v_button;
  UIFrame  *v_frame;

  if(m_sGUI != NULL)
    return;

  m_sGUI = new UIRoot();
  m_sGUI->setApp(pGame);
  m_sGUI->setFont(pGame->getDrawLib()->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      pGame->getDrawLib()->getDispWidth(),
		      pGame->getDrawLib()->getDispHeight());

  v_frame = new UIFrame(m_sGUI,
			pGame->getDrawLib()->getDispWidth()/2  - 350,
			pGame->getDrawLib()->getDispHeight()/2 - 250,
			"", 700, 500);
  v_frame->setID("EDITPROFILE_FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_TRANS);           

  UIStatic *pProfileEditorTitle = new UIStatic(v_frame, 0, 0, GAMETEXT_PLAYERPROFILES, v_frame->getPosition().nWidth, 50);
  pProfileEditorTitle->setFont(pGame->getDrawLib()->getFontMedium());

  UIList *pProfileList = new UIList(v_frame, 20, 50, "", v_frame->getPosition().nWidth - 20*2 -20 - 207, v_frame->getPosition().nHeight - 50 - 20);
  pProfileList->setFont(pGame->getDrawLib()->getFontSmall());
  pProfileList->addColumn(GAMETEXT_PLAYERPROFILE, 128);
  pProfileList->setID("PROFILE_LIST");
  pProfileList->setContextHelp(CONTEXTHELP_SELECT_PLAYER_PROFILE);

  v_button = new UIButton(v_frame, v_frame->getPosition().nWidth - 20 - 207, 50 + 0*57, GAMETEXT_USEPROFILE, 207, 57);
  v_button->setID("USEPROFILE_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_USE_PLAYER_PROFILE);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  v_frame->setPrimaryChild(v_button); /* default button */
  pProfileList->setEnterButton(v_button);

  v_button = new UIButton(v_frame, v_frame->getPosition().nWidth - 20 - 207, 50 + 1*57, GAMETEXT_NEWPROFILE, 207, 57);
  v_button->setID("NEWPROFILE_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_CREATE_PLAYER_PROFILE);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

  v_button = new UIButton(v_frame, v_frame->getPosition().nWidth - 20 - 207, v_frame->getPosition().nHeight - 20 - 57,
			  GAMETEXT_DELETEPROFILE, 207, 57);
  v_button->setID("DELETEPROFILE_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_DELETE_PROFILE);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

  createProfileList(pGame);
}

void StateEditProfile::createProfileList(GameApp* pGame) {
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
    v_result = pGame->getDb()->readDB("SELECT id_profile FROM stats_profiles ORDER BY id_profile;",
					nrow);
    for(unsigned int i=0; i<nrow; i++) {
      v_profile = pGame->getDb()->getResult(v_result, 1, i, 0);
      pList->addEntry(v_profile);
      if(pGame->getSession()->profile() == v_profile) {
	pList->setRealSelected(i);
      }
    }
    pGame->getDb()->read_DB_free(v_result);
    
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
    case UI_MSGBOX_OK:
      std::string PlayerName = i_input;
      try {
	m_pGame->getDb()->stats_createProfile(PlayerName);
      } catch(Exception &e) {
	Logger::Log("Unable to create the profile");
      }
      createProfileList(m_pGame);
      break;
    }
  }
  else if (i_id == "DELETEPROFILE"){
    switch(i_button){
    case UI_MSGBOX_YES:
      /* Delete selected profile */
      UIList *pList = reinterpret_cast<UIList *>(m_GUI->getChild("EDITPROFILE_FRAME:PROFILE_LIST"));
      if(pList != NULL) {
	int nIdx = pList->getSelected();
	if(nIdx >= 0 && nIdx < pList->getEntries().size()) {
	  UIListEntry *pEntry = pList->getEntries()[nIdx];
	  
	  m_pGame->getDb()->stats_destroyProfile(pEntry->Text[0]);
	  pList->setRealSelected(0);
	  createProfileList(m_pGame);              
	}
      }
      break;
    }
  }
}
