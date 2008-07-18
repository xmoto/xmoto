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

#include "StateHelp.h"
#include "Game.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"
#include "StatePreplayingGame.h"
#include "StatePreplayingCredits.h"

StateHelp::StateHelp(bool drawStateBehind,
		     bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind)
{
  m_name  = "StateHelp";
  createGUI(); // create the gui each time because it's small and keys can change
}

StateHelp::~StateHelp()
{
  delete m_GUI;
}

void StateHelp::leave()
{
  StateMenu::leave();
}

void StateHelp::checkEvents() {

  /* Find first tutorial level */
  UIButton *pTutorialButton = (UIButton *) m_GUI->getChild("FRAME:TUTORIAL_BUTTON");
  if(pTutorialButton->isClicked()) {
    pTutorialButton->setClicked(false);

    try {
      GameApp::instance()->setCurrentPlayingList(NULL);
      StateManager::instance()->pushState(new StatePreplayingGame("tut1", false));
    } catch(Exception &e) {
    }
  }

  /* View credits? */
  UIButton *pCreditsButton = (UIButton *)m_GUI->getChild("FRAME:CREDITS_BUTTON");
  if(pCreditsButton->isClicked()) {
    pCreditsButton->setClicked(false);

    try {
      StateManager::instance()->pushState(new StatePreplayingCredits("credits.rpl"));      
    } catch(Exception &e) {
    }
  }

  /* Close */
  UIButton *pCloseButton = (UIButton *) m_GUI->getChild("FRAME:CLOSE_BUTTON");
  if(pCloseButton->isClicked()) {
    pCloseButton->setClicked(false);

    m_requestForEnd = true;
  }
}

void StateHelp::keyDown(SDLKey nKey, SDLMod mod,int nChar, const std::string& i_utf8Char)
{
  StateMenu::keyDown(nKey, mod, nChar, i_utf8Char);

  if(nKey == SDLK_ESCAPE){
    m_requestForEnd = true;
  }
}

void StateHelp::createGUI() {
  UIFrame  *v_frame;
  UIStatic *v_someText;
  GameApp* pGame = GameApp::instance();
  DrawLib* drawLib = pGame->getDrawLib();

  m_GUI = new UIRoot();
  m_GUI->setFont(drawLib->getFontSmall()); 
  m_GUI->setPosition(0, 0,
		     drawLib->getDispWidth(),
		     drawLib->getDispHeight());
  
  
  int v_offsetX = drawLib->getDispWidth()  / 10;
  int v_offsetY = drawLib->getDispHeight() / 10;
  v_frame = new UIFrame(m_GUI, v_offsetX, v_offsetY, "",
			drawLib->getDispWidth()  - 2*v_offsetX,
			drawLib->getDispHeight() - 2*v_offsetY);
  v_frame->setID("FRAME");

  v_someText = new UIStatic(v_frame, 0, 0, GAMETEXT_HELP, v_frame->getPosition().nWidth, 36);
  v_someText->setFont(drawLib->getFontMedium());
  v_someText = new UIStatic(v_frame, 10, 46,
			    GAMETEXT_HELPTEXT(InputHandler::instance()->getFancyKeyByAction("Drive"),
					      InputHandler::instance()->getFancyKeyByAction("Brake"),
					      InputHandler::instance()->getFancyKeyByAction("PullBack"),
					      InputHandler::instance()->getFancyKeyByAction("PushForward"),
					      InputHandler::instance()->getFancyKeyByAction("ChangeDir")
					      ),
			    v_frame->getPosition().nWidth-20, v_frame->getPosition().nHeight-56);
  v_someText->setFont(drawLib->getFontSmall());
  v_someText->setVAlign(UI_ALIGN_TOP);
  v_someText->setHAlign(UI_ALIGN_LEFT);

  UIButton *pTutorialButton = new UIButton(v_frame, v_frame->getPosition().nWidth-240, v_frame->getPosition().nHeight-62,
					   GAMETEXT_TUTORIAL, 115, 57);
  pTutorialButton->setContextHelp(CONTEXTHELP_TUTORIAL);
  pTutorialButton->setFont(drawLib->getFontSmall());
  pTutorialButton->setType(UI_BUTTON_TYPE_SMALL);
  pTutorialButton->setID("TUTORIAL_BUTTON");    

  UIButton *pCreditsButton = new UIButton(v_frame, v_frame->getPosition().nWidth-360, v_frame->getPosition().nHeight-62,
					  GAMETEXT_CREDITSBUTTON, 115, 57);
  pCreditsButton->setContextHelp(CONTEXTHELP_CREDITS);
  pCreditsButton->setFont(drawLib->getFontSmall());
  pCreditsButton->setType(UI_BUTTON_TYPE_SMALL);
  pCreditsButton->setID("CREDITS_BUTTON");

  UIButton *pCloseButton = new UIButton(v_frame, v_frame->getPosition().nWidth-120, v_frame->getPosition().nHeight-62,
					GAMETEXT_CLOSE, 115, 57);
  pCloseButton->setFont(drawLib->getFontSmall());
  pCloseButton->setType(UI_BUTTON_TYPE_SMALL);
  pCloseButton->setID("CLOSE_BUTTON");
}
