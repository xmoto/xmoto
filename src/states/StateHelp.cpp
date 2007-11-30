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
#include "StatePreplaying.h"
#include "StateCreditsMode.h"

StateHelp::StateHelp(GameApp* pGame,
		     bool drawStateBehind,
		     bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{
  m_name  = "StateHelp";
  createGUI(m_pGame); // create the gui each time because it's small and keys can change
}

StateHelp::~StateHelp()
{
  delete m_GUI;
}

void StateHelp::enter()
{
  StateMenu::enter();
}

void StateHelp::leave()
{
  StateMenu::leave();
  m_pGame->getMotoGame()->setInfos("");
}

void StateHelp::enterAfterPop()
{
  StateMenu::enterAfterPop();
}

void StateHelp::leaveAfterPush()
{
  StateMenu::leaveAfterPush();
}

void StateHelp::checkEvents() {

  /* Find first tutorial level */
  UIButton *pTutorialButton = (UIButton *) m_GUI->getChild("FRAME:TUTORIAL_BUTTON");
  if(pTutorialButton->isClicked()) {
    pTutorialButton->setClicked(false);

    try {
      m_pGame->setCurrentPlayingList(NULL);
      StatePreplaying::setPlayAnimation(true);
      m_pGame->getStateManager()->pushState(new StatePreplaying(m_pGame, "tut1"));
    } catch(Exception &e) {
    }
  }

  /* View credits? */
  UIButton *pCreditsButton = (UIButton *) m_GUI->getChild("FRAME:CREDITS_BUTTON");
  if(pCreditsButton->isClicked()) {
    pCreditsButton->setClicked(false);

    try {
      m_pGame->getStateManager()->pushState(new StateCreditsMode(m_pGame, "credits.rpl"));      
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

bool StateHelp::update()
{
  return StateMenu::update();
}

bool StateHelp::render()
{
  return StateMenu::render();
}

void StateHelp::keyDown(int nKey, SDLMod mod,int nChar)
{
  StateMenu::keyDown(nKey, mod, nChar);

  if(nKey == SDLK_ESCAPE){
    m_requestForEnd = true;
  }
}

void StateHelp::keyUp(int nKey, SDLMod mod)
{
  StateMenu::keyUp(nKey, mod);
}

void StateHelp::mouseDown(int nButton)
{
  StateMenu::mouseDown(nButton);
}

void StateHelp::mouseDoubleClick(int nButton)
{
  StateMenu::mouseDoubleClick(nButton);
}

void StateHelp::mouseUp(int nButton)
{
  StateMenu::mouseUp(nButton);
}

void StateHelp::createGUI(GameApp* pGame) {
  UIButton *v_button;
  UIFrame  *v_frame;
  UIStatic *v_someText;

  m_GUI = new UIRoot();
  m_GUI->setApp(pGame);
  m_GUI->setFont(pGame->getDrawLib()->getFontSmall()); 
  m_GUI->setPosition(0, 0,
		     pGame->getDrawLib()->getDispWidth(),
		     pGame->getDrawLib()->getDispHeight());
  
  
  int v_offsetX = pGame->getDrawLib()->getDispWidth()  / 10;
  int v_offsetY = pGame->getDrawLib()->getDispHeight() / 10;
  v_frame = new UIFrame(m_GUI, v_offsetX, v_offsetY, "",
			pGame->getDrawLib()->getDispWidth()  - 2*v_offsetX,
			pGame->getDrawLib()->getDispHeight() - 2*v_offsetY);
  v_frame->setID("FRAME");

  v_someText = new UIStatic(v_frame, 0, 0, GAMETEXT_HELP, v_frame->getPosition().nWidth, 36);
  v_someText->setFont(pGame->getDrawLib()->getFontMedium());
  v_someText = new UIStatic(v_frame, 10, 46,
			    GAMETEXT_HELPTEXT(pGame->getUserConfig()->getString("KeyDrive1"),
					      pGame->getUserConfig()->getString("KeyBrake1"),
					      pGame->getUserConfig()->getString("KeyFlipLeft1"),
					      pGame->getUserConfig()->getString("KeyFlipRight1"),
					      pGame->getUserConfig()->getString("KeyChangeDir1")
					      ),
			    v_frame->getPosition().nWidth-20, v_frame->getPosition().nHeight-56);
  v_someText->setFont(pGame->getDrawLib()->getFontSmall());
  v_someText->setVAlign(UI_ALIGN_TOP);
  v_someText->setHAlign(UI_ALIGN_LEFT);

  UIButton *pTutorialButton = new UIButton(v_frame, v_frame->getPosition().nWidth-240, v_frame->getPosition().nHeight-62,
					   GAMETEXT_TUTORIAL, 115, 57);
  pTutorialButton->setContextHelp(CONTEXTHELP_TUTORIAL);
  pTutorialButton->setFont(pGame->getDrawLib()->getFontSmall());
  pTutorialButton->setType(UI_BUTTON_TYPE_SMALL);
  pTutorialButton->setID("TUTORIAL_BUTTON");    

  UIButton *pCreditsButton = new UIButton(v_frame, v_frame->getPosition().nWidth-360, v_frame->getPosition().nHeight-62,
					  GAMETEXT_CREDITSBUTTON, 115, 57);
  pCreditsButton->setContextHelp(CONTEXTHELP_CREDITS);
  pCreditsButton->setFont(pGame->getDrawLib()->getFontSmall());
  pCreditsButton->setType(UI_BUTTON_TYPE_SMALL);
  pCreditsButton->setID("CREDITS_BUTTON");

  UIButton *pCloseButton = new UIButton(v_frame, v_frame->getPosition().nWidth-120, v_frame->getPosition().nHeight-62,
					GAMETEXT_CLOSE, 115, 57);
  pCloseButton->setFont(pGame->getDrawLib()->getFontSmall());
  pCloseButton->setType(UI_BUTTON_TYPE_SMALL);
  pCloseButton->setID("CLOSE_BUTTON");
}
