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

#include "GameText.h"
#include "StateFinished.h"
#include "Game.h"
#include "XMSession.h"
#include "drawlib/DrawLib.h"
#include "Sound.h"

/* static members */
UIRoot* StateFinished::m_sGUI = NULL;

StateFinished::StateFinished(GameApp* pGame,
			     bool drawStateBehind,
			     bool updateStatesBehind
			     ) :
  StateMenu(drawStateBehind,
	    updateStatesBehind,
	    pGame,
	    true)
{

}

StateFinished::~StateFinished()
{

}

void StateFinished::enter()
{
  float v_finish_time = 0.0;
  std::string TimeStamp;
  bool v_is_a_room_highscore;
  bool v_is_a_personnal_highscore;

  StateMenu::enter();

  m_pGame->m_State = GS_FINISHED; // to be removed, just the time states are finished
  m_pGame->getMotoGame()->setInfos(m_pGame->getMotoGame()->getLevelSrc()->Name());
  m_pGame->setShowCursor(true);	
  m_pGame->playMusic("");

  createGUIIfNeeded(m_pGame);
  m_GUI = m_sGUI;

  /* reset the playnext button */
  UIButton *playNextButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FINISHED_FRAME:PLAYNEXT_BUTTON"));
  playNextButton->enableWindow(m_pGame->isThereANextLevel(m_pGame->getMotoGame()->getLevelSrc()->Id()));

  UIButton* saveReplayButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FINISHED_FRAME:SAVEREPLAY_BUTTON"));
  saveReplayButton->enableWindow(m_pGame->isAReplayToSave());

  UIButton* v_uploadButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FINISHED_FRAME:UPLOAD_BUTTON"));
  v_uploadButton->enableWindow(false);

  UIStatic* v_pNewHighscore_str = reinterpret_cast<UIStatic *>(m_GUI->getChild("HIGHSCORESTR_STATIC"));
  v_pNewHighscore_str->setCaption("");

  UIStatic* v_pNewHighscoreSaved_str = reinterpret_cast<UIStatic *>(m_GUI->getChild("HIGHSCORESAVEDSTR_STATIC"));
  v_pNewHighscoreSaved_str->setCaption("");

  m_pGame->isTheCurrentPlayAHighscore(v_is_a_personnal_highscore, v_is_a_room_highscore);

  /* replay */
  if(m_pGame->isAReplayToSave()) {

    /* upload button */
    if(v_is_a_room_highscore) {
      /* active upload button */
      if(m_pGame->getSession()->www()) {
	v_uploadButton->enableWindow(v_is_a_room_highscore);
      }
    }

    /* autosave */
    if(v_is_a_room_highscore || v_is_a_personnal_highscore) {
      if(m_pGame->getSession()->autosaveHighscoreReplays()) {
	std::string v_replayName;
	char v_str[256];
	v_replayName = Replay::giveAutomaticName();
	m_pGame->saveReplay(v_replayName);
	snprintf(v_str, 256, GAMETEXT_SAVE_AS, v_replayName.c_str());
	v_pNewHighscoreSaved_str->setCaption(v_str);
      }
    }
  }

  /* sound */
  if(v_is_a_room_highscore || v_is_a_personnal_highscore) {
    /* play a sound */
    try {
      Sound::playSampleByName(m_pGame->getTheme()->getSound("NewHighscore")->FilePath());
    } catch(Exception &e) {
    }
  }

  /* new highscores text */
  if(v_is_a_room_highscore) {
    v_pNewHighscore_str->setFont(m_pGame->getDrawLib()->getFontMedium());
    v_pNewHighscore_str->setCaption(GAMETEXT_NEWHIGHSCORE);
  } else {
    if(v_is_a_personnal_highscore) {
      v_pNewHighscore_str->setFont(m_pGame->getDrawLib()->getFontSmall());
      v_pNewHighscore_str->setCaption(GAMETEXT_NEWHIGHSCOREPERSONAL);
    }
  }

  UIBestTimes *v_pBestTimes = reinterpret_cast<UIBestTimes *>(m_GUI->getChild("BESTTIMES"));
  makeBestTimesWindow(v_pBestTimes, m_pGame->getDb(), m_pGame->getSession()->profile(), m_pGame->getMotoGame()->getLevelSrc()->Id(),
		      v_finish_time, TimeStamp);
}

void StateFinished::leave()
{
  m_pGame->getMotoGame()->setInfos("");
}

void StateFinished::enterAfterPop()
{
  StateMenu::enterAfterPop();
}

void StateFinished::leaveAfterPush()
{
  StateMenu::leaveAfterPush();
}

void StateFinished::checkEvents() {

  UIButton *pTryAgainButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FINISHED_FRAME:TRYAGAIN_BUTTON"));
  if(pTryAgainButton->isClicked()) {
    pTryAgainButton->setClicked(false);

    m_pGame->m_State = GS_PLAYING; // to be removed, just the time states are finished
    m_pGame->restartLevel();
    m_requestForEnd = true;
  }

  UIButton *pPlaynextButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FINISHED_FRAME:PLAYNEXT_BUTTON"));
  if(pPlaynextButton->isClicked()) {
    pPlaynextButton->setClicked(false);

    m_pGame->playNextLevel();
    m_pGame->setPrePlayAnim(true);
    m_pGame->setState(GS_PREPLAYING);
    m_requestForEnd = true;
  }

  UIButton *pSavereplayButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FINISHED_FRAME:SAVEREPLAY_BUTTON"));
  if(pSavereplayButton->isClicked()) {
    pSavereplayButton->setClicked(false);
  }

  UIButton *pUploadButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FINISHED_FRAME:UPLOAD_BUTTON"));
  if(pUploadButton->isClicked()) {
    pUploadButton->setClicked(false);

    m_pGame->uploadHighscore("Latest");
  }

  UIButton *pAbortButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FINISHED_FRAME:ABORT_BUTTON"));
  if(pAbortButton->isClicked()) {
    pAbortButton->setClicked(false);

    m_pGame->abortPlaying();
    m_pGame->setState(m_pGame->m_StateAfterPlaying); // to be removed once states will be finished
    m_requestForEnd = true;
  }

  UIButton *pQuitButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FINISHED_FRAME:QUIT_BUTTON"));
  if(pQuitButton->isClicked()) {
    pQuitButton->setClicked(false);

    //UIMsgBox *m_pQuitMsgBox;
    //m_pQuitMsgBox = m_GUI->msgBox(GAMETEXT_QUITMESSAGE,
    //			  (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));

    m_pGame->requestEnd(); 
    m_requestForEnd = true;
  }

//      UIStatic *m_pNewWorldRecord;
//void StatePause::checkEvents() {
//    /* Is savereplay box open? */
//    if(m_pSaveReplayMsgBox != NULL) {
//      UIMsgBoxButton Clicked = m_pSaveReplayMsgBox->getClicked();
//      if(Clicked != UI_MSGBOX_NOTHING) {
//        std::string Name = m_pSaveReplayMsgBox->getTextInput();
//      
//        delete m_pSaveReplayMsgBox;
//        m_pSaveReplayMsgBox = NULL;
//
//        if(Clicked == UI_MSGBOX_OK) {
//          _SaveReplay(Name);
//        }    
//      }
//    }
//      
//        else if(v_frameButtons[i]->getCaption() == GAMETEXT_SAVEREPLAY) {
//          if(m_pJustPlayReplay != NULL) {
//            if(m_pSaveReplayMsgBox == NULL) {
//	      m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
//              m_pSaveReplayMsgBox = m_Renderer->getGUI()->msgBox(std::string(GAMETEXT_ENTERREPLAYNAME) + ":",
//                                                                (UIMsgBoxButton)(UI_MSGBOX_OK|UI_MSGBOX_CANCEL),
//                                                                true);
//              m_pSaveReplayMsgBox->setTextInputFont(drawLib->getFontMedium());
//	      m_pSaveReplayMsgBox->setTextInput(Replay::giveAutomaticName());
//            }          
//          }
	

}

bool StateFinished::update()
{
  return StateMenu::update();
}

bool StateFinished::render()
{
  return StateMenu::render();
}

void StateFinished::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {

  case SDLK_ESCAPE:
    /* quit this state */
    m_pGame->closePlaying();
    m_pGame->setState(m_pGame->m_StateAfterPlaying); // to be removed, just the time states are finished
    m_requestForEnd = true;
    break;

  default:
    StateMenu::keyDown(nKey, mod, nChar);
    break;

  }

}

void StateFinished::keyUp(int nKey, SDLMod mod)
{
  StateMenu::keyUp(nKey, mod);
}

void StateFinished::mouseDown(int nButton)
{
  StateMenu::mouseDown(nButton);
}

void StateFinished::mouseDoubleClick(int nButton)
{
  StateMenu::mouseDoubleClick(nButton);
}

void StateFinished::mouseUp(int nButton)
{
  StateMenu::mouseUp(nButton);
}

void StateFinished::clean() {
  if(StateFinished::m_sGUI != NULL) {
    delete StateFinished::m_sGUI;
    StateFinished::m_sGUI = NULL;
  }
}

void StateFinished::createGUIIfNeeded(GameApp* pGame) {
  if(m_sGUI != NULL) return;

  UIFrame*     v_frame;
  UIBestTimes* v_pBestTimes;
  UIButton*    v_button;
  UIStatic*    v_pFinishText;
  UIStatic*    v_pNewHighscore_str;
  UIStatic*    v_pNewHighscoreSaved_str;

  m_sGUI = new UIRoot();
  m_sGUI->setApp(pGame);
  m_sGUI->setFont(pGame->getDrawLib()->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      pGame->getDrawLib()->getDispWidth(),
		      pGame->getDrawLib()->getDispHeight());
  

  v_frame = new UIFrame(m_sGUI, 300, 30, "", 400, 540);
  v_frame->setID("FINISHED_FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_MENU);

  v_pFinishText = new UIStatic(v_frame, 0, 100, GAMETEXT_FINISH, v_frame->getPosition().nWidth, 36);
  v_pFinishText->setFont(pGame->getDrawLib()->getFontMedium());

  v_pBestTimes = new UIBestTimes(m_sGUI, 10, 50, "", 290, m_sGUI->getPosition().nHeight - 50*2);
  v_pBestTimes->setID("BESTTIMES");
  v_pBestTimes->setFont(pGame->getDrawLib()->getFontMedium());
  v_pBestTimes->setHFont(pGame->getDrawLib()->getFontMedium());

  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 6*57/2 + 0*49 + 25, GAMETEXT_TRYAGAIN, 207, 57);
  v_button->setID("TRYAGAIN_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_PLAY_THIS_LEVEL_AGAIN);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  v_frame->setPrimaryChild(v_button); /* default button */

  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 6*57/2 + 1*49 + 25, GAMETEXT_PLAYNEXT, 207, 57);
  v_button->setID("PLAYNEXT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_PLAY_NEXT_LEVEL);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 6*57/2 + 2*49 + 25, GAMETEXT_SAVEREPLAY, 207, 57);
  v_button->setID("SAVEREPLAY_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_SAVE_A_REPLAY);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 6*57/2 + 3*49 + 25, GAMETEXT_UPLOAD_HIGHSCORE, 207, 57);
  v_button->setID("UPLOAD_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_UPLOAD_HIGHSCORE);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 6*57/2 + 4*49 + 25, GAMETEXT_ABORT, 207, 57);
  v_button->setID("ABORT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_BACK_TO_MAIN_MENU);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 6*57/2 + 5*49 + 25, GAMETEXT_QUIT, 207, 57);
  v_button->setID("QUIT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_QUIT_THE_GAME);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

  v_pNewHighscore_str = new UIStatic(m_sGUI, 0, m_sGUI->getPosition().nHeight - 20 - 20, "Where are you ?", m_sGUI->getPosition().nWidth, 20);
  v_pNewHighscore_str->setID("HIGHSCORESTR_STATIC");
  v_pNewHighscore_str->setFont(pGame->getDrawLib()->getFontSmall());
  v_pNewHighscore_str->setHAlign(UI_ALIGN_CENTER);

  v_pNewHighscoreSaved_str = new UIStatic(m_sGUI, 0, m_sGUI->getPosition().nHeight - 20, "And you ?", m_sGUI->getPosition().nWidth, 20);
  v_pNewHighscoreSaved_str->setID("HIGHSCORESAVEDSTR_STATIC");
  v_pNewHighscoreSaved_str->setFont(pGame->getDrawLib()->getFontSmall());
  v_pNewHighscoreSaved_str->setHAlign(UI_ALIGN_CENTER);

}

void StateFinished::makeBestTimesWindow(UIBestTimes *pWindow,
					xmDatabase *i_db,
					const std::string& PlayerName, const std::string& LevelID,
					float fFinishTime, const std::string& TimeStamp) {
  char **v_result;
  unsigned int nrow;
  int n1=-1,n2=-1;
  float v_finishTime;
  std::string v_timeStamp;
  std::string v_profile;
  
  pWindow->clear();
  
  v_result = i_db->readDB("SELECT finishTime, timeStamp, id_profile FROM profile_completedLevels "
			  "WHERE id_level=\""   + xmDatabase::protectString(LevelID)    + "\" "
			  "ORDER BY finishTime LIMIT 10;",
			  nrow);
  for(unsigned int i=0; i<nrow; i++) {
    v_finishTime  = atof(i_db->getResult(v_result, 3, i, 0));
    v_timeStamp   =      i_db->getResult(v_result, 3, i, 1);
    v_profile     =      i_db->getResult(v_result, 3, i, 2);
    pWindow->addRow1(GameApp::formatTime(v_finishTime), v_profile);
    if(v_profile == PlayerName &&
       v_timeStamp == TimeStamp) n1 = i;
  }
  i_db->read_DB_free(v_result);
  
  v_result = i_db->readDB("SELECT finishTime, timeStamp FROM profile_completedLevels "
			  "WHERE id_profile=\"" + xmDatabase::protectString(PlayerName) + "\" "
			  "AND   id_level=\""   + xmDatabase::protectString(LevelID)    + "\" "
			  "ORDER BY finishTime LIMIT 10;",
			    nrow);
    for(unsigned int i=0; i<nrow; i++) {
      v_finishTime  = atof(i_db->getResult(v_result, 2, i, 0));
      v_timeStamp   =      i_db->getResult(v_result, 2, i, 1);
      pWindow->addRow2(GameApp::formatTime(v_finishTime), PlayerName);
      if(v_timeStamp == TimeStamp) n2 = i;
    }
    i_db->read_DB_free(v_result);
    
    pWindow->setup(GAMETEXT_BESTTIMES,n1,n2);
}

