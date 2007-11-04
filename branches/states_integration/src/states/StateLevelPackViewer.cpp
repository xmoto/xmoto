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

#include "StateLevelPackViewer.h"
#include "Game.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"
  //#include "LevelsManager.h"
#include "gui/specific/GUIXMoto.h"
#include "XMSession.h"
#include "states/StatePreplaying.h"
#include "states/StateLevelInfoViewer.h"

/* static members */
UIRoot*  StateLevelPackViewer::m_sGUI = NULL;

StateLevelPackViewer::StateLevelPackViewer(GameApp*    pGame,
					   LevelsPack* pActiveLevelPack,
					   bool drawStateBehind,
					   bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{
  m_name             = "StateLevelPackViewer";
  m_pActiveLevelPack = pActiveLevelPack;
}

StateLevelPackViewer::~StateLevelPackViewer()
{

}


void StateLevelPackViewer::enter()
{
  StateMenu::enter();

  m_pGame->m_State = GS_LEVELPACK_VIEWER; // to be removed, just the time states are finished
  
  createGUIIfNeeded(m_pGame);
  m_GUI = m_sGUI;

  updateGUI();
}

void StateLevelPackViewer::leave()
{

}

void StateLevelPackViewer::enterAfterPop()
{
  StateMenu::enterAfterPop();
}

void StateLevelPackViewer::leaveAfterPush()
{
  StateMenu::leaveAfterPush();
}

void StateLevelPackViewer::checkEvents()
{
  /* Get buttons and list */
  UIButton *pCancelButton             = reinterpret_cast<UIButton *>(m_GUI->getChild("LEVELPACK_VIEWER_FRAME:LEVELPACK_CANCEL_BUTTON"));
  UIButton *pPlayButton               = reinterpret_cast<UIButton *>(m_GUI->getChild("LEVELPACK_VIEWER_FRAME:LEVELPACK_PLAY_BUTTON"));
  UIButton *pLevelInfoButton          = reinterpret_cast<UIButton *>(m_GUI->getChild("LEVELPACK_VIEWER_FRAME:LEVELPACK_INFO_BUTTON"));
  UIButton *pLevelAddToFavoriteButton = reinterpret_cast<UIButton *>(m_GUI->getChild("LEVELPACK_VIEWER_FRAME:LEVELPACK_ADDTOFAVORITE_BUTTON"));
  UIButton *pLevelRandomizeButton     = reinterpret_cast<UIButton *>(m_GUI->getChild("LEVELPACK_VIEWER_FRAME:LEVELPACK_RANDOMIZE_BUTTON"));
  UILevelList *pList                  = (UILevelList *)m_GUI->getChild("LEVELPACK_VIEWER_FRAME:LEVELPACK_LEVEL_LIST");
  UIEdit *pLevelFilterEdit            = reinterpret_cast<UIEdit *>(m_GUI->getChild("LEVELPACK_VIEWER_FRAME:LEVELPACK_LEVEL_FILTER"));   

  /* check filter */
  if(pLevelFilterEdit != NULL) {
    if(pLevelFilterEdit->hasChanged()) {
      pLevelFilterEdit->setHasChanged(false);
      pList->setFilter(pLevelFilterEdit->getCaption());
    }
  }

  /* Check buttons */
  if(pCancelButton!=NULL && pCancelButton->isClicked()) {
    pCancelButton->setClicked(false);
      
    m_requestForEnd = true;
    /*
    m_State = GS_MENU;
    m_GUI->showWindow(false);
    m_pMainMenu->enableChildren(true);
    m_pMainMenu->enableWindow(true);
    */
  }
    
  if(pPlayButton!=NULL && pPlayButton->isClicked()) {
    pPlayButton->setClicked(false);
    std::string i_level = pList->getSelectedLevel();
    if(i_level != "") {
      //m_GUI->showWindow(false);
      //m_pMainMenu->showWindow(false);      
      //m_PlaySpecificLevelId = i_level;
      //m_StateAfterPlaying = GS_LEVELPACK_VIEWER;
      //m_currentPlayingList = pList;
      StatePreplaying::setPlayAnimation(true);
      m_pGame->getStateManager()->pushState(new StatePreplaying(m_pGame, i_level));
    }
  }

  if(pLevelAddToFavoriteButton != NULL && pLevelAddToFavoriteButton->isClicked()) {
    pLevelAddToFavoriteButton->setClicked(false);

    std::string v_id_level = pList->getSelectedLevel();

    if(v_id_level != "") {
      m_pGame->addLevelToFavorite(v_id_level);
    }
  }

  if(pLevelRandomizeButton!=NULL && pLevelRandomizeButton->isClicked()) {
    pLevelRandomizeButton->setClicked(false);
      
    pList->randomize();
  }

#if 0  
  /* level menu : */
  /* any list clicked ? */
  if(pList->isChanged()) {
    pList->setChanged(false);
    std::string v_id_level = pList->getSelectedLevel();
    if(v_id_level != "") {
      // ??
      m_pGame->setLevelInfoFrameBestPlayer(v_id_level,
					   m_pPackLevelInfoFrame,
					   m_pPackLevelInfoViewReplayButton,
					   m_pPackBestPlayerText
					   );

    }
  }
#endif

#if 0  
  /* view highscore button clicked */
  if(m_pPackLevelInfoViewReplayButton->isClicked()) {
    m_pPackLevelInfoViewReplayButton->setClicked(false);
    m_pGame->viewHighscoreOf();
    m_GUI->showWindow(false);
    m_pMainMenu->showWindow(false);      
    m_StateAfterPlaying = GS_LEVELPACK_VIEWER;
    m_pGame->getStateManager()->pushState(new StateReplaying(m_pGame, m_PlaySpecificReplay));
  }
#endif

  if(pLevelInfoButton!=NULL && pLevelInfoButton->isClicked()) {
    pLevelInfoButton->setClicked(false);

    std::string v_id_level = pList->getSelectedLevel();
    if(v_id_level != "") {
      m_pGame->getStateManager()->pushState(new StateLevelInfoViewer(m_pGame, v_id_level));
    }
  }
}

bool StateLevelPackViewer::update()
{
  return StateMenu::update();
}

bool StateLevelPackViewer::render()
{
  return StateMenu::render();
}

void StateLevelPackViewer::keyDown(int nKey, SDLMod mod,int nChar)
{
  StateMenu::keyDown(nKey, mod, nChar);
  checkEvents();
}

void StateLevelPackViewer::keyUp(int nKey,   SDLMod mod)
{
  StateMenu::keyUp(nKey, mod);
}

void StateLevelPackViewer::mouseDown(int nButton)
{
  StateMenu::mouseDown(nButton);
}

void StateLevelPackViewer::mouseDoubleClick(int nButton)
{
  StateMenu::mouseDoubleClick(nButton);
}

void StateLevelPackViewer::mouseUp(int nButton)
{
  StateMenu::mouseUp(nButton);
}

void StateLevelPackViewer::clean()
{
  if(StateLevelPackViewer::m_sGUI != NULL) {
    delete StateLevelPackViewer::m_sGUI;
    StateLevelPackViewer::m_sGUI = NULL;
  }
}

void StateLevelPackViewer::createGUIIfNeeded(GameApp* pGame)
{
  if(m_sGUI != NULL)
    return;

  DrawLib* drawLib = pGame->getDrawLib();

  m_sGUI = new UIRoot();
  m_sGUI->setApp(pGame);
  m_sGUI->setFont(drawLib->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      drawLib->getDispWidth(),
		      drawLib->getDispHeight());

  /* Initialize level pack viewer */
  int x = drawLib->getDispWidth()/2-350;
  int y = drawLib->getDispHeight()/2-250;
  std::string caption = "";
  int nWidth = 700;
  int nHeight = 500;

  UIFrame  *v_frame;
  v_frame = new UIFrame(m_sGUI, x, y, caption, nWidth, nHeight); 
  v_frame->setID("LEVELPACK_VIEWER_FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_TRANS);

  UIStatic *pLevelPackViewerTitle = new UIStatic(v_frame,0,0,"(level pack name goes here)",700,40);
  pLevelPackViewerTitle->setID("LEVELPACK_VIEWER_TITLE");
  pLevelPackViewerTitle->setFont(drawLib->getFontMedium());

  UIButton *pLevelPackPlay = new UIButton(v_frame,450,50,GAMETEXT_STARTLEVEL,207,57);
  pLevelPackPlay->setFont(drawLib->getFontSmall());
  pLevelPackPlay->setID("LEVELPACK_PLAY_BUTTON");
  pLevelPackPlay->setContextHelp(CONTEXTHELP_PLAY_SELECTED_LEVEL);

  UIStatic* pSomeText = new UIStatic(v_frame, 20, 70, std::string(GAMETEXT_FILTER) + ":", 90, 25);
  pSomeText->setFont(drawLib->getFontSmall());
  pSomeText->setHAlign(UI_ALIGN_RIGHT);

  UIEdit *pLevelFilterEdit = new UIEdit(v_frame, 120, 70, "", 200, 25);
  pLevelFilterEdit->setFont(drawLib->getFontSmall());
  pLevelFilterEdit->setID("LEVELPACK_LEVEL_FILTER");
  pLevelFilterEdit->setContextHelp(CONTEXTHELP_LEVEL_FILTER);

  UILevelList *pLevelPackLevelList = new UILevelList(v_frame,20,100,"",400, 380);
  pLevelPackLevelList->setFont(drawLib->getFontSmall());
  pLevelPackLevelList->setContextHelp(CONTEXTHELP_SELECT_LEVEL_IN_LEVEL_PACK);
  pLevelPackLevelList->setID("LEVELPACK_LEVEL_LIST");
  pLevelPackLevelList->setEnterButton( pLevelPackPlay );

  UIButton *pLevelPackInfo = new UIButton(v_frame,450,107,GAMETEXT_LEVELINFO,207,57);
  pLevelPackInfo->setFont(drawLib->getFontSmall());
  pLevelPackInfo->setID("LEVELPACK_INFO_BUTTON");
  pLevelPackInfo->setContextHelp(CONTEXTHELP_LEVEL_INFO);

  UIButton *pLevelPackAddToFavorite = new UIButton(v_frame,450,164,GAMETEXT_ADDTOFAVORITE,207,57);
  pLevelPackAddToFavorite->setFont(drawLib->getFontSmall());
  pLevelPackAddToFavorite->setID("LEVELPACK_ADDTOFAVORITE_BUTTON");
  pLevelPackAddToFavorite->setContextHelp(CONTEXTHELP_ADDTOFAVORITE);

  UIButton *pLevelPackRandomize = new UIButton(v_frame,450,221,GAMETEXT_RANDOMIZE,207,57);
  pLevelPackRandomize->setFont(drawLib->getFontSmall());
  pLevelPackRandomize->setID("LEVELPACK_RANDOMIZE_BUTTON");
  pLevelPackRandomize->setContextHelp(CONTEXTHELP_RANDOMIZE);

  UIButton *pLevelPackCancel = new UIButton(v_frame,450,278,GAMETEXT_CLOSE,207,57);
  pLevelPackCancel->setFont(drawLib->getFontSmall());
  pLevelPackCancel->setID("LEVELPACK_CANCEL_BUTTON");
  pLevelPackCancel->setContextHelp(CONTEXTHELP_CLOSE_LEVEL_PACK);

  /* level info frame */
#if 0
  m_pPackLevelInfoFrame = new UIWindow(v_frame,419,400,"",275,100);
  m_pPackLevelInfoFrame->showWindow(false);
  m_pPackBestPlayerText = new UIStatic(m_pPackLevelInfoFrame, 0, 5,"", 275, 50);
  m_pPackBestPlayerText->setHAlign(UI_ALIGN_RIGHT);
  m_pPackBestPlayerText->setFont(drawLib->getFontSmall());
  m_pPackBestPlayerText->setHAlign(UI_ALIGN_CENTER);
  m_pPackBestPlayerText->showWindow(true);
  m_pPackLevelInfoViewReplayButton = new UIButton(m_pPackLevelInfoFrame,50,40, GAMETEXT_VIEWTHEHIGHSCORE,175,40);
  m_pPackLevelInfoViewReplayButton->setFont(drawLib->getFontSmall());
  m_pPackLevelInfoViewReplayButton->setContextHelp(CONTEXTHELP_VIEWTHEHIGHSCORE);
#endif
}

void StateLevelPackViewer::updateGUI()
{
  char **v_result;
  unsigned int nrow;
  float v_playerHighscore;
  float v_roomHighscore;
  xmDatabase* pDb = m_pGame->getDb();

  UIStatic *pTitle = (UIStatic *)m_GUI->getChild("LEVELPACK_VIEWER_FRAME:LEVELPACK_VIEWER_TITLE");
  pTitle->setCaption(m_pActiveLevelPack->Name());

  UILevelList *pList = (UILevelList *)m_GUI->getChild("LEVELPACK_VIEWER_FRAME:LEVELPACK_LEVEL_LIST");    
  pList->setNumeroted(true);
  pList->makeActive();

  /* get selected item */
  std::string v_selected_levelName = "";
  if(pList->getSelected() >= 0 && pList->getSelected() < pList->getEntries().size()) {
    UIListEntry *pEntry = pList->getEntries()[pList->getSelected()];
    v_selected_levelName = pEntry->Text[0];
  }

  pList->clear();

  // clear the filter
  UIEdit *pLevelFilterEdit = reinterpret_cast<UIEdit *>(m_GUI->getChild("LEVELPACK_VIEWER_FRAME:LEVELPACK_LEVEL_FILTER"));  
  pLevelFilterEdit->setCaption("");
  pList->setFilter("");
  
  /* Obey hints */
  pList->unhideAllColumns();
  if(!m_pActiveLevelPack->ShowTimes()) {
    pList->hideBestTime();
  }
  if(!m_pActiveLevelPack->ShowWebTimes()) {
    pList->hideRoomBestTime();
  }

  v_result = pDb->readDB(m_pActiveLevelPack->getLevelsWithHighscoresQuery(m_pGame->getSession()->profile(),
									  m_pGame->getWebHighscoresIdRoom()),
				      nrow);
  for(unsigned int i=0; i<nrow; i++) {
    if(pDb->getResult(v_result, 4, i, 2) == NULL) {
      v_playerHighscore = -1.0;
    } else {
      v_playerHighscore = atof(pDb->getResult(v_result, 4, i, 2));
    }

    if(pDb->getResult(v_result, 4, i, 3) == NULL) {
      v_roomHighscore = -1.0;
    } else {
      v_roomHighscore = atof(pDb->getResult(v_result, 4, i, 3));
    }

    pList->addLevel(pDb->getResult(v_result, 4, i, 0),
		    pDb->getResult(v_result, 4, i, 1),
		    v_playerHighscore,
		    v_roomHighscore);
  }
  pDb->read_DB_free(v_result);

  /* reselect the previous level */
  if(v_selected_levelName != "") {
    int nLevel = 0;
    for(int i=0; i<pList->getEntries().size(); i++) {
      if(pList->getEntries()[i]->Text[0] == v_selected_levelName) {
	nLevel = i;
	break;
      }
    }
    pList->setRealSelected(nLevel);
  }
}
