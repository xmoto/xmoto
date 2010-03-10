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

#include "StateShowcase.h"
#include "../Game.h"
#include "../drawlib/DrawLib.h"
#include "../GameText.h"
#include "StatePreplayingGame.h"
#include "StatePreplayingCredits.h"

StateShowcase::StateShowcase(bool drawStateBehind,
		     bool updateStatesBehind, bool i_gameShowcase, bool i_allowSceneOver):
  StateMenu(drawStateBehind,
	    updateStatesBehind, true, false)
{
  m_name  = "StateShowcase";
  m_gameShowcase = i_gameShowcase;
  m_allowSceneOver = i_allowSceneOver;
  createGUI(); // create the gui each time because it's small and keys can change
}

StateShowcase::~StateShowcase()
{
  delete m_GUI;
}

void StateShowcase::enterAfterPop() {
  StateMenu::enterAfterPop();
}

void StateShowcase::leave()
{
  StateMenu::leave();
}

void StateShowcase::checkEvents() {

  /* Close */
  UIButton *pCloseButton = (UIButton *) m_GUI->getChild("FRAME:CLOSE_BUTTON");
  if(pCloseButton->isClicked()) {
    pCloseButton->setClicked(false);

    m_requestForEnd = true;
  }
}

void StateShowcase::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  StateMenu::xmKey(i_type, i_xmkey);

  if(i_type == INPUT_DOWN && ( i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE) ||
			       i_xmkey == XMKey(SDLK_F1,     KMOD_NONE) )) {
    m_requestForEnd = true;
  }
}

void StateShowcase::createGUI() {
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

  v_frame = new UIFrame(m_GUI, v_offsetX, m_gameShowcase ? v_offsetY / 2 : v_offsetY, "",
			drawLib->getDispWidth()  - 2*v_offsetX,
			m_gameShowcase ? drawLib->getDispHeight() - v_offsetY : drawLib->getDispHeight() - 2*v_offsetY
			);
  v_frame->setID("FRAME");
  
  v_someText = new UIStatic(v_frame, 0, 0, "SHOWCASE", v_frame->getPosition().nWidth, 36);
  v_someText->setFont(drawLib->getFontMedium());
  v_someText = new UIStatic(v_frame, 10, 40, "BALABLA",
			    v_frame->getPosition().nWidth-20,
			    m_gameShowcase ? v_frame->getPosition().nHeight-46 :
			                 v_frame->getPosition().nHeight-56);
  v_someText->setFont(drawLib->getFontSmall());
  v_someText->setVAlign(UI_ALIGN_TOP);
  v_someText->setHAlign(UI_ALIGN_LEFT);
  
  UIButton *pCloseButton = new UIButton(v_frame, v_frame->getPosition().nWidth-120, v_frame->getPosition().nHeight-62,
					GAMETEXT_CLOSE, 115, 57);
  pCloseButton->setFont(drawLib->getFontSmall());
  pCloseButton->setType(UI_BUTTON_TYPE_SMALL);
  pCloseButton->setID("CLOSE_BUTTON");
}

