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

#include "StateFinished.h"
#include "Game.h"
#include "XMSession.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"

/* static members */
UIRoot* StateFinished::m_sGUI = NULL;

StateFinished::StateFinished(GameApp* pGame,
			     bool drawStateBehind,
			     bool updateStatesBehind
			     ) :
  StateMenu(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{

}

StateFinished::~StateFinished()
{

}

void StateFinished::enter()
{
  StateMenu::enter();

  m_pGame->m_State = GS_FINISHED; // to be removed, just the time states are finished
  m_pGame->getMotoGame()->setInfos(m_pGame->getMotoGame()->getLevelSrc()->Name());
  m_pGame->setShowCursor(true);	
  m_pGame->playMusic("");

  createGUIIfNeeded(m_pGame);
  m_GUI = m_sGUI;

//  /* reset the playnext button */
//  UIButton *playNextButton = reinterpret_cast<UIButton *>(m_GUI->getChild("FINISHED_FRAME:PLAYNEXT_BUTTON"));
//  playNextButton->enableWindow(m_pGame->isThereANextLevel(m_pGame->getMotoGame()->getLevelSrc()->Id()));
//
//
//  /* Finish replay */
//  if(m_pJustPlayReplay != NULL) {
//    if(m_MotoGame.Players().size() == 1) {
//      /* save the last state because scene don't record each frame */
//      SerializedBikeState BikeState;
//      MotoGame::getSerializedBikeState(m_MotoGame.Players()[0]->getState(), m_MotoGame.getTime(), &BikeState);
//      m_pJustPlayReplay->storeState(BikeState);
//      m_pJustPlayReplay->finishReplay(true,m_MotoGame.Players()[0]->finishTime());
//    }
//  }
//
//  if(m_MotoGame.Players().size() == 1) {
//    /* display message on finish and eventually save the replay */
//        
//    /* is it a highscore ? */
//    float v_best_personal_time;
//    float v_best_room_time;
//    float v_current_time;
//    bool v_is_a_highscore;
//    bool v_is_a_personal_highscore;
//    
//    /* get best player result */
//    v_result = m_db->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
//			    "id_level=\"" + 
//			    xmDatabase::protectString(m_MotoGame.getLevelSrc()->Id()) + "\" " + 
//			    "AND id_profile=\"" + xmDatabase::protectString(m_xmsession->profile())  + "\";",
//			    nrow);
//    v_res = m_db->getResult(v_result, 1, 0, 0);
//    if(v_res != NULL) {
//      v_best_personal_time = atof(v_res);
//    } else {
//      /* should never happend because the score is already stored */
//      v_best_personal_time = -1.0;
//    }
//    m_db->read_DB_free(v_result);
//    
//    v_current_time = m_MotoGame.Players()[0]->finishTime();
//    
//    v_is_a_personal_highscore = (v_current_time <= v_best_personal_time
//				 || v_best_personal_time < 0.0);
//    
//    /* search a better webhighscore */
//    v_best_room_time = m_db->webrooms_getHighscoreTime(m_WebHighscoresIdRoom,
//						       m_MotoGame.getLevelSrc()->Id());
//    v_is_a_highscore = (v_current_time < v_best_room_time
//			|| v_best_room_time < 0.0);
//    
//    // disable upload button
//    for(int i=0;i<m_nNumFinishMenuButtons;i++) {
//      if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_UPLOAD_HIGHSCORE) {
//	m_pFinishMenuButtons[i]->enableWindow(false);
//      }
//    }
//    
//    if(v_is_a_highscore) { /* best highscore */
//      try {
//	Sound::playSampleByName(m_theme.getSound("NewHighscore")->FilePath());
//      } catch(Exception &e) {
//      }
//      
//      // enable upload button
//      if(m_xmsession->www()) {
//	if(m_pJustPlayReplay != NULL) {
//	  for(int i=0;i<m_nNumFinishMenuButtons;i++) {
//	    if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_UPLOAD_HIGHSCORE) {
//	      m_pFinishMenuButtons[i]->enableWindow(true);
//	    }
//	  }
//	}
//      }
//      
//      if(m_pJustPlayReplay != NULL && m_bAutosaveHighscoreReplays) {
//	std::string v_replayName = Replay::giveAutomaticName();
//	_SaveReplay(v_replayName);
//	m_Renderer->showMsgNewBestHighscore(v_replayName);
//      } else {
//	m_Renderer->showMsgNewBestHighscore();
//      } /* ok i officially give up on indention in x-moto :P */
//    } else {
//      if(v_is_a_personal_highscore) { /* personal highscore */
//	try {
//	  Sound::playSampleByName(m_theme.getSound("NewHighscore")->FilePath());
//	} catch(Exception &e) {
//	}
//	if(m_pJustPlayReplay != NULL && m_bAutosaveHighscoreReplays) {
//	  std::string v_replayName = Replay::giveAutomaticName();
//	  _SaveReplay(v_replayName);
//	  m_Renderer->showMsgNewPersonalHighscore(v_replayName);
//	} else {
//	  m_Renderer->showMsgNewPersonalHighscore();
//	}
//	
//      } else { /* no highscore */
//	m_Renderer->hideMsgNewHighscore();
//      }
//    }
//  }
//  
//  /* update profiles */
//  float v_finish_time = 0.0;
//  std::string TimeStamp = getTimeStamp();
//  for(unsigned int i=0; i<m_MotoGame.Players().size(); i++) {
//    if(m_MotoGame.Players()[i]->isFinished()) {
//      v_finish_time  = m_MotoGame.Players()[i]->finishTime();
//    }
//  }
//  if(m_MotoGame.Players().size() == 1) {
//    m_db->profiles_addFinishTime(m_xmsession->profile(), m_MotoGame.getLevelSrc()->Id(),
//				 TimeStamp, v_finish_time);
//  }
//  _MakeBestTimesWindow(m_pBestTimes, m_xmsession->profile(), m_MotoGame.getLevelSrc()->Id(),
//		       v_finish_time,TimeStamp);
//  
//  /* Update stats */
//  /* update stats only in one player mode */
//  if(m_MotoGame.Players().size() == 1) {       
//    m_db->stats_levelCompleted(m_xmsession->profile(),
//			       m_MotoGame.getLevelSrc()->Id(),
//			       m_MotoGame.Players()[0]->finishTime());
//    _UpdateLevelsLists();
//    _UpdateCurrentPackList(m_MotoGame.getLevelSrc()->Id(),
//			   m_MotoGame.Players()[0]->finishTime());
//  }
//
}

void StateFinished::leave()
{
  m_pGame->getMotoGame()->setInfos("");
}

void StateFinished::enterAfterPop()
{

}

void StateFinished::leaveAfterPush()
{

}

void StateFinished::checkEvents() {
}

void StateFinished::update()
{
  StateMenu::update();
}

void StateFinished::render()
{
  StateMenu::render();
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

  m_sGUI = new UIRoot();
  m_sGUI->setApp(pGame);
  m_sGUI->setFont(pGame->getDrawLib()->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      pGame->getDrawLib()->getDispWidth(),
		      pGame->getDrawLib()->getDispHeight());
  
//
//              m_pFinishMenu->showWindow(false);
//        m_Renderer->hideMsgNewHighscore();
//              m_pBestTimes->showWindow(false);
//
//      /* In-game FINISH menu fun */
//      UIFrame *m_pFinishMenu;
//      int m_nFinishShade;
//      UIButton *m_pFinishMenuButtons[10];
//      UIStatic *m_pNewWorldRecord;
//
//      int m_nNumFinishMenuButtons;      
//      UIBestTimes *m_pBestTimes;

}

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
//    /* Any of the finish menu buttons clicked? */
//    for(int i=0;i<m_nNumFinishMenuButtons;i++) {
//      if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_SAVEREPLAY) {
//        /* Have we recorded a replay? If not then disable the "Save Replay" button */
//        if(m_pJustPlayReplay == NULL || m_MotoGame.Players().size() != 1) {
//          m_pFinishMenuButtons[i]->enableWindow(false);
//        }
//        else {
//          m_pFinishMenuButtons[i]->enableWindow(true);
//        }
//      }
//
//      if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_PLAYNEXT) {
//        /* Uhm... is it likely that there's a next level? */
//	m_pFinishMenuButtons[i]->enableWindow(isThereANextLevel(m_PlaySpecificLevelId));
//      }
//      
//      if(m_pFinishMenuButtons[i]->isClicked()) {
//        if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_QUIT) {
//          if(m_pQuitMsgBox == NULL) {
//	    m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
//            m_pQuitMsgBox = m_Renderer->getGUI()->msgBox(GAMETEXT_QUITMESSAGE,
//                                                        (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
//	  }
//        }
//        else if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_PLAYNEXT) {
//	  std::string NextLevel = _DetermineNextLevel(m_PlaySpecificLevelId);
//	  if(NextLevel != "") {        
//	    m_pFinishMenu->showWindow(false);
//	    m_Renderer->hideMsgNewHighscore();
//	    m_pBestTimes->showWindow(false);
//	    m_MotoGame.getCamera()->setPlayerToFollow(NULL);
//	    m_MotoGame.endLevel();
//	    m_InputHandler.resetScriptKeyHooks();                     
//	    m_Renderer->unprepareForNewLevel();                    
//	    
//	    m_PlaySpecificLevelId = NextLevel;
//	    
//	    setPrePlayAnim(true);
//	    setState(GS_PREPLAYING);                               
//          }
//        }
//        else if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_SAVEREPLAY) {
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
//        }
//        else if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_UPLOAD_HIGHSCORE) {
//	  _UploadHighscore("Latest");
//        }	
//        else if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_TRYAGAIN) {
//          Level *pCurLevel = m_MotoGame.getLevelSrc();
//          m_PlaySpecificLevelId = pCurLevel->Id();
//          m_pFinishMenu->showWindow(false);
//	  m_Renderer->hideMsgNewHighscore();
//          m_pBestTimes->showWindow(false);
//
//	  restartLevel();
//        }
//        else if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_ABORT) {
//          m_pFinishMenu->showWindow(false);
//	  m_Renderer->hideMsgNewHighscore();
//          m_pBestTimes->showWindow(false);
//	  m_MotoGame.getCamera()->setPlayerToFollow(NULL);
//          m_MotoGame.endLevel();
//          m_InputHandler.resetScriptKeyHooks();                     
//          m_Renderer->unprepareForNewLevel();
//          setState(m_StateAfterPlaying);                   
//        }
//
//        /* Don't process this clickin' more than once */
//        m_pFinishMenuButtons[i]->setClicked(false);
//      }
//    }
//  }
//}



//  /*===========================================================================
//  Fill a window with best times
//  ===========================================================================*/
//  void GameApp::_MakeBestTimesWindow(UIBestTimes *pWindow,std::string PlayerName,std::string LevelID,
//                                     float fFinishTime,std::string TimeStamp) {
//    char **v_result;
//    unsigned int nrow;
//    int n1=-1,n2=-1;
//    float v_finishTime;
//    std::string v_timeStamp;
//    std::string v_profile;
//    
//    pWindow->clear();
//    
//    v_result = m_db->readDB("SELECT finishTime, timeStamp, id_profile FROM profile_completedLevels "
//			    "WHERE id_level=\""   + xmDatabase::protectString(LevelID)    + "\" "
//			    "ORDER BY finishTime LIMIT 10;",
//			    nrow);
//    for(unsigned int i=0; i<nrow; i++) {
//      v_finishTime  = atof(m_db->getResult(v_result, 3, i, 0));
//      v_timeStamp   =      m_db->getResult(v_result, 3, i, 1);
//      v_profile     =      m_db->getResult(v_result, 3, i, 2);
//      pWindow->addRow1(formatTime(v_finishTime), v_profile);
//      if(v_profile == PlayerName &&
//	 v_timeStamp == TimeStamp) n1 = i;
//    }
//    m_db->read_DB_free(v_result);
//
//    v_result = m_db->readDB("SELECT finishTime, timeStamp FROM profile_completedLevels "
//			    "WHERE id_profile=\"" + xmDatabase::protectString(PlayerName) + "\" "
//			    "AND   id_level=\""   + xmDatabase::protectString(LevelID)    + "\" "
//			    "ORDER BY finishTime LIMIT 10;",
//			    nrow);
//    for(unsigned int i=0; i<nrow; i++) {
//      v_finishTime  = atof(m_db->getResult(v_result, 2, i, 0));
//      v_timeStamp   =      m_db->getResult(v_result, 2, i, 1);
//      pWindow->addRow2(formatTime(v_finishTime), PlayerName);
//      if(v_timeStamp == TimeStamp) n2 = i;
//    }
//    m_db->read_DB_free(v_result);
//
//    pWindow->setup(GAMETEXT_BESTTIMES,n1,n2);
//  }
//
//
//makeWIndow() {
//    /* Best times windows (at finish) */
//    m_pBestTimes = new UIBestTimes(m_Renderer->getGUI(),10,50,"",290,500);
//    m_pBestTimes->setFont(drawLib->getFontMedium());
//    m_pBestTimes->setHFont(drawLib->getFontMedium());
//
//    /* Initialize finish menu */
//    m_pFinishMenu = new UIFrame(m_Renderer->getGUI(),300,30,"",400,540);
//    m_pFinishMenu->setStyle(UI_FRAMESTYLE_MENU);
//
//    i=0;
//
//    m_pFinishMenuButtons[i] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_TRYAGAIN,207,57);
//    m_pFinishMenuButtons[i]->setContextHelp(CONTEXTHELP_PLAY_THIS_LEVEL_AGAIN);
//    i++;
//
//    m_pFinishMenuButtons[i] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_PLAYNEXT,207,57);
//    m_pFinishMenuButtons[i]->setContextHelp(CONTEXTHELP_PLAY_NEXT_LEVEL);
//    default_button = i;
//    i++;
//
//    m_pFinishMenuButtons[i] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_SAVEREPLAY,207,57);
//    m_pFinishMenuButtons[i]->setContextHelp(CONTEXTHELP_SAVE_A_REPLAY);
//    if(!m_bRecordReplays) m_pFinishMenuButtons[1]->enableWindow(false);
//    i++;
//
//    m_pFinishMenuButtons[i] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_UPLOAD_HIGHSCORE,207,57);
//    m_pFinishMenuButtons[i]->setContextHelp(CONTEXTHELP_UPLOAD_HIGHSCORE);
//    m_pFinishMenuButtons[i]->enableWindow(false);
//    i++;
//   
//    m_pFinishMenuButtons[i] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_ABORT,207,57);
//    m_pFinishMenuButtons[i]->setContextHelp(CONTEXTHELP_BACK_TO_MAIN_MENU);
//    i++;
//    
//    m_pFinishMenuButtons[i] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_QUIT,207,57);
//    m_pFinishMenuButtons[i]->setContextHelp(CONTEXTHELP_QUIT_THE_GAME);
//    i++;
//
//    m_nNumFinishMenuButtons = i;
//
//    UIStatic *pFinishText = new UIStatic(m_pFinishMenu,0,100,GAMETEXT_FINISH,m_pFinishMenu->getPosition().nWidth,36);
//    pFinishText->setFont(drawLib->getFontMedium());
//   
//    for(int i=0;i<m_nNumFinishMenuButtons;i++) {
//      m_pFinishMenuButtons[i]->setPosition(200-207/2,m_pFinishMenu->getPosition().nHeight/2 - (m_nNumFinishMenuButtons*57)/2 + i*49 + 25,207,57);
//      m_pFinishMenuButtons[i]->setFont(drawLib->getFontSmall());
//    }
//
//    m_pFinishMenu->setPrimaryChild(m_pFinishMenuButtons[default_button]); /* default button: Play next */
//
//
//}
//
//
//      void _MakeBestTimesWindow(UIBestTimes *pWindow,std::string PlayerName,std::string LevelID,
//                                float fFinishTime,std::string TimeStamp);      
