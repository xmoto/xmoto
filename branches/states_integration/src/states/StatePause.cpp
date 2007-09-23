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

#include "StatePause.h"
#include "Game.h"
#include "XMSession.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"

#define PAUSE_SHADING_TIME 0.3
#define PAUSE_SHADING_VALUE 150

/* static members */
UIRoot*  StatePause::m_GUI        = NULL;

StatePause::StatePause(GameApp* pGame,
		       bool drawStateBehind,
		       bool updateStatesBehind):
  GameState(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{
}

StatePause::~StatePause()
{

}

void StatePause::enter()
{
  m_nPauseShadeTime = GameApp::getXMTime();
  m_pGame->m_State = GS_PAUSE; // to be removed, just the time states are finished
  m_pGame->getMotoGame()->setInfos(m_pGame->getMotoGame()->getLevelSrc()->Name());
  m_pGame->setShowCursor(true);
  
  createGUIIfNeeded(m_pGame);

  /* reset the playnext button */
  UIButton *playNextButton = reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:PLAYNEXT_BUTTON"));
  playNextButton->enableWindow(m_pGame->isThereANextLevel(m_pGame->getMotoGame()->getLevelSrc()->Id()));
}

void StatePause::leave()
{
  m_pGame->getMotoGame()->setInfos("");
}

void StatePause::enterAfterPop()
{

}

void StatePause::leaveAfterPush()
{

}

void StatePause::checkEvents() {
  UIButton *pResumeButton = reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:RESUME_BUTTON"));
  if(pResumeButton->isClicked()) {
    pResumeButton->setClicked(false);

    m_pGame->m_State = GS_PLAYING; // to be removed, just the time states are finished
    m_requestForEnd = true;
  }

  UIButton *pRestartButton = reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:RESTART_BUTTON"));
  if(pRestartButton->isClicked()) {
    pRestartButton->setClicked(false);

    m_pGame->m_State = GS_PLAYING; // to be removed, just the time states are finished
    m_pGame->restartLevel();
    m_requestForEnd = true;
  }

  UIButton *pPlayNextButton = reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:PLAYNEXT_BUTTON"));
  if(pPlayNextButton->isClicked()) {
    pPlayNextButton->setClicked(false);

    m_pGame->playNextLevel();
    m_pGame->setState(GS_PREPLAYING);
    m_requestForEnd = true;
  }

  UIButton *pAbortButton = reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:ABORT_BUTTON"));
  if(pAbortButton->isClicked()) {
    pAbortButton->setClicked(false);

    m_pGame->abortPlaying();
    m_pGame->setState(m_pGame->m_StateAfterPlaying); // to be removed once states will be finished
    m_requestForEnd = true;
  }

  UIButton *pQuitButton = reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:QUIT_BUTTON"));
  if(pQuitButton->isClicked()) {
    pQuitButton->setClicked(false);

    //UIMsgBox *m_pQuitMsgBox;

    //m_pQuitMsgBox = m_GUI->msgBox(GAMETEXT_QUITMESSAGE,
    //			  (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));

    m_pGame->requestEnd(); 

    m_requestForEnd = true;
  }
}

void StatePause::update()
{
  m_GUI->dispatchMouseHover();
}

void StatePause::render()
{
  // rendering of the gui must be done by the mother call : to add here when states will be almost finished
  m_pGame->setFrameDelay(10);

  if(m_pGame->getSession()->ugly() == false) {
    float v_currentTime = GameApp::getXMTime();
    int   v_nPauseShade;

    if(v_currentTime - m_nPauseShadeTime < PAUSE_SHADING_TIME) {
      v_nPauseShade = (int ) ((v_currentTime - m_nPauseShadeTime) * (PAUSE_SHADING_VALUE / PAUSE_SHADING_TIME));
    } else {
      v_nPauseShade = PAUSE_SHADING_VALUE;
    }

    m_pGame->getDrawLib()->drawBox(Vector2f(0,0),
				   Vector2f(m_pGame->getDrawLib()->getDispWidth(),
					    m_pGame->getDrawLib()->getDispHeight()),
				   0,
				   MAKE_COLOR(0,0,0, v_nPauseShade)
				   );
  }

  m_GUI->paint();
}

void StatePause::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {

  case SDLK_ESCAPE:
    /* quit this state */
    m_pGame->m_State = GS_PLAYING; // to be removed, just the time states are finished
    m_requestForEnd = true;
    break;

  case SDLK_F3:
    m_pGame->switchLevelToFavorite(m_pGame->getMotoGame()->getLevelSrc()->Id(), true);
    break;

  default:
    m_GUI->keyDown(nKey, mod, nChar);
    checkEvents();
    break;

  }
}

void StatePause::keyUp(int nKey, SDLMod mod)
{
  m_GUI->keyUp(nKey, mod);
  checkEvents();
}

void StatePause::mouseDown(int nButton)
{
  int nX,nY;        
  GameApp::getMousePos(&nX,&nY);
        
  if(nButton == SDL_BUTTON_LEFT) {
    m_GUI->mouseLDown(nX,nY);
    checkEvents();
  } else if(nButton == SDL_BUTTON_RIGHT) {
    m_GUI->mouseRDown(nX,nY);
    checkEvents();
  } else if(nButton == SDL_BUTTON_WHEELUP) {
    m_GUI->mouseWheelUp(nX,nY);
    checkEvents();
  } else if(nButton == SDL_BUTTON_WHEELDOWN) {
    m_GUI->mouseWheelDown(nX,nY);
    checkEvents();
  }
}

void StatePause::mouseDoubleClick(int nButton)
{
  int nX,nY;        
  GameApp::getMousePos(&nX,&nY);
        
  if(nButton == SDL_BUTTON_LEFT) {
    m_GUI->mouseLDoubleClick(nX,nY);
    checkEvents();
  }
}

void StatePause::mouseUp(int nButton)
{
  int nX,nY;
  GameApp::getMousePos(&nX,&nY);
  
  if(nButton == SDL_BUTTON_LEFT) {
    m_GUI->mouseLUp(nX,nY);
    checkEvents();
  } else if(nButton == SDL_BUTTON_RIGHT) {
    m_GUI->mouseRUp(nX,nY);
    checkEvents();
  }
}

void StatePause::clean() {
  if(StatePause::m_GUI != NULL) {
    delete StatePause::m_GUI;
    StatePause::m_GUI = NULL;
  }
}

void StatePause::createGUIIfNeeded(GameApp* pGame) {
  UIButton *v_button;
  UIFrame  *v_frame;

  if(m_GUI != NULL) return;

  m_GUI = new UIRoot();
  m_GUI->setApp(pGame);
  m_GUI->setFont(pGame->getDrawLib()->getFontSmall()); 
  m_GUI->setPosition(0, 0,
		     pGame->getDrawLib()->getDispWidth(),
		     pGame->getDrawLib()->getDispHeight());


  v_frame = new UIFrame(m_GUI,
			     pGame->getDrawLib()->getDispWidth()/2  - 400/2,
			     pGame->getDrawLib()->getDispHeight()/2 - 540/2,
			     "", 400, 540);
  v_frame->setID("PAUSE_FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_MENU);

  UIStatic *pPauseText = new UIStatic(v_frame, 0, 100, GAMETEXT_PAUSE, v_frame->getPosition().nWidth, 36);
  pPauseText->setFont(pGame->getDrawLib()->getFontMedium());

  v_button = new UIButton(v_frame, 0, 0, GAMETEXT_RESUME, 207, 57);
  v_button->setID("RESUME_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_BACK_TO_GAME);
  v_button->setPosition(400/2 - 207/2, v_frame->getPosition().nHeight/2 - 5*57/2 + 0*57, 207, 57);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  v_frame->setPrimaryChild(v_button); /* default button */

  v_button = new UIButton(v_frame,0,0,GAMETEXT_RESTART,207,57);
  v_button->setID("RESTART_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_TRY_LEVEL_AGAIN_FROM_BEGINNING);
  v_button->setPosition(400/2 - 207/2, v_frame->getPosition().nHeight/2 - 5*57/2 + 1*57, 207, 57);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

  v_button = new UIButton(v_frame,0,0,GAMETEXT_PLAYNEXT,207,57);
  v_button->setID("PLAYNEXT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_PLAY_NEXT_INSTEAD);
  v_button->setPosition(400/2 - 207/2, v_frame->getPosition().nHeight/2 - 5*57/2 + 2*57, 207, 57);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

  v_button = new UIButton(v_frame,0,0,GAMETEXT_ABORT,207,57);
  v_button->setID("ABORT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_BACK_TO_MAIN_MENU);
  v_button->setPosition(400/2 - 207/2, v_frame->getPosition().nHeight/2 - 5*57/2 + 3*57, 207, 57);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  
  v_button = new UIButton(v_frame,0,0,GAMETEXT_QUIT,207,57);
  v_button->setID("QUIT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_QUIT_THE_GAME);
  v_button->setPosition(400/2 - 207/2, v_frame->getPosition().nHeight/2 - 5*57/2 + 4*57, 207, 57);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

}
