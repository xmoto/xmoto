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

/* static members */
UIRoot*  StateEditProfile::m_sGUI = NULL;

StateEditProfile::StateEditProfile(GameApp* pGame,
				   bool drawStateBehind,
				   bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{

}

StateEditProfile::~StateEditProfile()
{

}


void StateEditProfile::enter()
{
  StateMenu::enter();

  m_pGame->m_State = GS_EDIT_PROFILES; // to be removed, just the time states are finished
  m_pGame->setShowCursor(true);
  m_pGame->playMusic("menu1");
  
  createGUIIfNeeded(m_pGame);
  m_GUI = m_sGUI;
}

void StateEditProfile::leave()
{

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

  UIButton *pUseProfileButton = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITPROFILE_FRAME:USEPROFILE_BUTTON"));
  if(pUseProfileButton->isClicked()) {
    pUseProfileButton->setClicked(false);

//    UIList *pList = reinterpret_cast<UIList *>(m_GUI->getChild("EDITPROFILE_FRAME:PROFILE_LIST"));
//    if(pList != NULL) {
//      int nIdx = pList->getSelected();
//      if(nIdx >= 0 && nIdx < pList->getEntries().size()) {
//	UIListEntry *pEntry = pList->getEntries()[nIdx];
//	
//	m_xmsession->setProfile(pEntry->Text[0]);
//	m_db->stats_xmotoStarted(m_xmsession->profile());

//	  delete m_pStatsReport;
//          m_pStatsReport = stats_generateReport(m_xmsession->profile(),m_pStatsWindow,30,36,m_pStatsWindow->getPosition().nWidth-45,m_pStatsWindow->getPosition().nHeight-36, drawLib->getFontSmall());
//        }
//      }      
//      
//      if(m_xmsession->profile() == "") throw Exception("failed to set profile");
//
//      /* remake the packs with the new profile */
//      m_levelsManager.makePacks(m_db,
//				m_xmsession->profile(),
//				m_WebHighscoresIdRoom,
//				m_xmsession->debug());
//      _UpdateLevelsLists();
//      _UpdateReplaysList();      
//                  
//      updatePlayerTag();
//           
//      m_pProfileEditor->showWindow(false);
//
//      /* Should we jump to the web config now? */
//      if(m_Config.getBool("WebConfAtInit")) {
//        _InitWebConf();
//        setState(GS_EDIT_WEBCONFIG);
//      }
   
    m_pGame->setState(GS_MENU); 
    m_requestForEnd = true;
  }

//  /*===========================================================================
//  Update profile editor
//  ===========================================================================*/
//  void GameApp::_HandleProfileEditor(void) {    
//    /* Is newplayer box open? */
//    if(m_pNewProfileMsgBox != NULL) {
//      UIMsgBoxButton Clicked = m_pNewProfileMsgBox->getClicked();
//      if(Clicked != UI_MSGBOX_NOTHING) {
//        if(Clicked == UI_MSGBOX_OK) {
//          /* Create new profile */
//          std::string PlayerName = m_pNewProfileMsgBox->getTextInput();
//	  try {
//	    m_db->stats_createProfile(PlayerName);
//	  } catch(Exception &e) {
//	    Logger::Log("Unable to create the profile");
//	  }
//	  _CreateProfileList();
//        }
//        
//        delete m_pNewProfileMsgBox;
//        m_pNewProfileMsgBox = NULL;
//      }
//    }
//    /* What about the delete player box? */
//    if(m_pDeleteProfileMsgBox != NULL) {
//      UIMsgBoxButton Clicked = m_pDeleteProfileMsgBox->getClicked();
//      if(Clicked != UI_MSGBOX_NOTHING) {
//        if(Clicked == UI_MSGBOX_YES) {
//          /* Delete selected profile */
//          UIList *pList = reinterpret_cast<UIList *>(m_pProfileEditor->getChild("PROFILE_LIST"));
//          if(pList != NULL) {
//            int nIdx = pList->getSelected();
//            if(nIdx >= 0 && nIdx < pList->getEntries().size()) {
//              UIListEntry *pEntry = pList->getEntries()[nIdx];
//      
//	      m_db->stats_destroyProfile(pEntry->Text[0]);
//              pList->setRealSelected(0);
//              _CreateProfileList();              
//            }
//          }
//        }
//        delete m_pDeleteProfileMsgBox;
//        m_pDeleteProfileMsgBox = NULL;
//      }      
//    }
//  
//    /* Get buttons */
//    UIButton *pUseButton = reinterpret_cast<UIButton *>(m_pProfileEditor->getChild("USEPROFILE_BUTTON"));
//    UIButton *pDeleteButton = reinterpret_cast<UIButton *>(m_pProfileEditor->getChild("DELETEPROFILE_BUTTON"));
//    UIButton *pNewButton = reinterpret_cast<UIButton *>(m_pProfileEditor->getChild("NEWPROFILE_BUTTON"));
//    
//    /* Check them */
//    if(pUseButton->isClicked()) {      

//    }    
//    else if(pDeleteButton->isClicked()) {
//      if(m_pDeleteProfileMsgBox == NULL) {
//	m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
//        m_pDeleteProfileMsgBox = m_Renderer->getGUI()->msgBox(GAMETEXT_DELETEPLAYERMESSAGE,
//                                                          (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
//      }
//    }
//    else if(pNewButton->isClicked()) {
//      if(m_pNewProfileMsgBox == NULL) {
//	m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
//        m_pNewProfileMsgBox = m_Renderer->getGUI()->msgBox(std::string(GAMETEXT_ENTERPLAYERNAME) + ":",
//                                                          (UIMsgBoxButton)(UI_MSGBOX_OK|UI_MSGBOX_CANCEL),
//                                                          true);
//        m_pNewProfileMsgBox->setTextInputFont(drawLib->getFontMedium());
//      }
//    }
//  }
// 
}

void StateEditProfile::update()
{
  StateMenu::update();
}

void StateEditProfile::render()
{
  StateMenu::render();
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

  if(m_sGUI != NULL) return;

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
  if(m_sGUI == NULL) return;

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
