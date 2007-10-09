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

#include "StateMessageBox.h"
#include "Game.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"

StateMessageBox::StateMessageBox(GameApp* pGame,
				 const std::string& i_text,
				 int i_buttons,
				 bool i_input,
				 bool i_query,
				 bool drawStateBehind,
				 bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{
  m_buttons = i_buttons;
  m_GUI = createGUI(m_pGame, i_text, i_input, i_query);
}

StateMessageBox::~StateMessageBox()
{
  delete m_GUI;
}

void StateMessageBox::enter()
{
  StateMenu::enter();

  m_pGame->setShowCursor(true);
}

void StateMessageBox::leave()
{
}

void StateMessageBox::enterAfterPop()
{
  StateMenu::enterAfterPop();
}

void StateMessageBox::leaveAfterPush()
{
  StateMenu::leaveAfterPush();
}

void StateMessageBox::checkEvents() {
  UIButton *pButton;

  if(m_buttons & UI_MSGBOX_OK) {
    pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("MSGBOX:OK"));
    if(pButton->isClicked()) {
      pButton->setClicked(false);
      m_requestForEnd = true;
    }
  }

  if(m_buttons & UI_MSGBOX_CANCEL) {
    pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("MSGBOX:CANCEL"));
    if(pButton->isClicked()) {
      pButton->setClicked(false);
      m_requestForEnd = true;
    }
  }

  if(m_buttons & UI_MSGBOX_YES) {
    pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("MSGBOX:YES"));
    if(pButton->isClicked()) {
      pButton->setClicked(false);
      m_requestForEnd = true;
    }
  }

  if(m_buttons & UI_MSGBOX_NO) {
    pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("MSGBOX:NO"));
    if(pButton->isClicked()) {
      pButton->setClicked(false);
      m_requestForEnd = true;
    }
  }

  if(m_buttons & UI_MSGBOX_YES_FOR_ALL) {
    pButton = reinterpret_cast<UIButton *>(m_GUI->getChild("MSGBOX:YES_FOR_ALL"));
    if(pButton->isClicked()) {
      pButton->setClicked(false);
      m_requestForEnd = true;
    }
  }
}

void StateMessageBox::update()
{
  StateMenu::update();
}

void StateMessageBox::render()
{
  StateMenu::render();
}

void StateMessageBox::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {
  default:
    StateMenu::keyDown(nKey, mod, nChar);
    checkEvents();
    break;
  }
}

void StateMessageBox::keyUp(int nKey, SDLMod mod)
{
  StateMenu::keyUp(nKey, mod);
}

void StateMessageBox::mouseDown(int nButton)
{
  StateMenu::mouseDown(nButton);
  m_requestForEnd = true;
}

void StateMessageBox::mouseDoubleClick(int nButton)
{
  StateMenu::mouseDoubleClick(nButton);
}

void StateMessageBox::mouseUp(int nButton)
{
  StateMenu::mouseUp(nButton);
}

void StateMessageBox::clean() {
}

UIRoot* StateMessageBox::createGUI(GameApp* pGame, const std::string& i_text, bool i_input, bool i_query) {
  UIRoot* v_GUI;
  int nNumButtons=0;
  int nButtonSize = 57;
  int w, h;

  v_GUI = new UIRoot();
  v_GUI->setApp(pGame);
  v_GUI->setFont(pGame->getDrawLib()->getFontSmall()); 
  v_GUI->setPosition(0, 0,
		     pGame->getDrawLib()->getDispWidth(),
		     pGame->getDrawLib()->getDispHeight());
    
  if(m_buttons & UI_MSGBOX_OK)          nNumButtons++;
  if(m_buttons & UI_MSGBOX_CANCEL)      nNumButtons++;
  if(m_buttons & UI_MSGBOX_YES)         nNumButtons++;
  if(m_buttons & UI_MSGBOX_NO)          nNumButtons++;
  if(m_buttons & UI_MSGBOX_YES_FOR_ALL) nNumButtons++;

  /* Determine size of contents */
  FontManager* v_fm = pGame->getDrawLib()->getFontSmall();
  FontGlyph* v_fg   = v_fm->getGlyph(i_text);

  w = (v_fg->realWidth() > nNumButtons*115 ? (v_fg->realWidth()) : nNumButtons*115) + 16 + 100;
  h = v_fg->realHeight() + nButtonSize + 24 + 100;
    
  if(i_input) h+=40;    
    
  /* Create the box */
  UIMsgBox *pMsgBox;
  if(i_query) {
    pMsgBox = (UIMsgBox *) new UIQueryKeyBox(v_GUI,
					     pGame->getDrawLib()->getDispWidth()/2 - w/2,
					     pGame->getDrawLib()->getDispHeight()/2/2 - h/2,
					     "",w,h);
  } else {
    pMsgBox = new UIMsgBox(v_GUI,
			   pGame->getDrawLib()->getDispWidth()/2 - w/2,
			   pGame->getDrawLib()->getDispHeight()/2 - h/2,
			   "",w,h);
  }    
  pMsgBox->setID("MSGBOX");

  if(i_input) {
    pMsgBox->enableTextInput();
  }    

  /* Create text static */
  int nStaticY=0;
  if(i_input) nStaticY=40;
  UIStatic *pText = new UIStatic(pMsgBox,8,8,i_text,pMsgBox->getPosition().nWidth-16,
				 pMsgBox->getPosition().nHeight-24-nButtonSize-nStaticY);   
  pText->setFont(pGame->getDrawLib()->getFontSmall());
  pText->setBackgroundShade(true); /* make text more easy to read */
    
  /* Create buttons */
  int nCX = pMsgBox->getPosition().nWidth/2 - (nNumButtons*115)/2;
  int nCY = pMsgBox->getPosition().nHeight-16-nButtonSize;
  
  UIButton *pButton;
  if(m_buttons & UI_MSGBOX_OK) {
    pButton = new UIButton(pMsgBox,nCX,nCY,GAMETEXT_OK,115,57);
    pButton->setID("OK");
    pButton->setFont(pGame->getDrawLib()->getFontSmall());
    pButton->setType(UI_BUTTON_TYPE_SMALL);
    pMsgBox->addButton(pButton);
    nCX+=115;
  }
  if(m_buttons & UI_MSGBOX_CANCEL) {
    pButton = new UIButton(pMsgBox,nCX,nCY,GAMETEXT_CANCEL,115,57);
    pButton->setID("CANCEL");
    pButton->setFont(pGame->getDrawLib()->getFontSmall());
    pButton->setType(UI_BUTTON_TYPE_SMALL);
    pMsgBox->addButton(pButton);
    nCX+=115;
  }
  if(m_buttons & UI_MSGBOX_YES) {
    pButton = new UIButton(pMsgBox,nCX,nCY,GAMETEXT_YES,115,57);
    pButton->setID("YES");
    pButton->setFont(pGame->getDrawLib()->getFontSmall());
    pButton->setType(UI_BUTTON_TYPE_SMALL);
    pMsgBox->addButton(pButton);
    nCX+=115;
  }
  if(m_buttons & UI_MSGBOX_NO) {
    pButton = new UIButton(pMsgBox,nCX,nCY,GAMETEXT_NO,115,57);
    pButton->setID("NO");
    pButton->setFont(pGame->getDrawLib()->getFontSmall());
    pButton->setType(UI_BUTTON_TYPE_SMALL);
    pMsgBox->addButton(pButton);
    nCX+=115;
  }
  if(m_buttons & UI_MSGBOX_YES_FOR_ALL) {
    pButton = new UIButton(pMsgBox,nCX,nCY,GAMETEXT_YES_FOR_ALL,115,57);
    pButton->setID("YES_FOR_ALL");
    pButton->setFont(pGame->getDrawLib()->getFontSmall());
    pButton->setType(UI_BUTTON_TYPE_SMALL);
    pMsgBox->addButton(pButton);
    nCX+=115;
  }    

  return v_GUI;
}
