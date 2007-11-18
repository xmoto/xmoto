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

#include "StateUpdate.h"
#include "Game.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"
#include "XMSession.h"


/* static members */
UIRoot*  StateUpdate::m_sGUI = NULL;

StateUpdate::StateUpdate(GameApp* pGame,
			 bool drawStateBehind,
			 bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{
  m_name             = "StateUpdate";
  m_threadStarted    = false;
  m_progress         = -1;
  m_currentOperation = "";
  m_currentMicroOperation = "";
}

StateUpdate::~StateUpdate()
{

}

void StateUpdate::enter()
{
  createGUIIfNeeded(m_pGame);
  m_GUI = m_sGUI;

  StateMenu::enter();
}

void StateUpdate::leave()
{
  StateMenu::leave();
}

void StateUpdate::enterAfterPop()
{
  StateMenu::enterAfterPop();
}

void StateUpdate::leaveAfterPush()
{
  StateMenu::leaveAfterPush();
}

void StateUpdate::checkEvents()
{
}

bool StateUpdate::update()
{
  return StateMenu::update();
}

bool StateUpdate::render()
{
  bool ret = StateMenu::render();


  return ret;

#if 0
  // draw text
  m_GUI->setTextSolidColor(MAKE_COLOR(255,255,255,255));
  m_GUI->putText(drawLib->getDispWidth()/2,
		 drawLib->getDispHeight()/2,
		 m_currentOperation,
		 -0.5, -0.5);

  FontGlyph* fg = drawlib->getFontMedium()->getGlyph(msg);
    
  int border = 75;
  int nW = fg->realWidth() + border*2, nH = fg->realHeight() + border*2;
  int nx = drawlib->getDispWidth()/2 - nW/2, ny = drawlib->getDispHeight()/2 - nH/2;

  //_SimpleMessage(m_DownloadingMessage,&m_InfoMsgBoxRect,true);
  m_GUI->setTextSolidColor(MAKE_COLOR(255,255,255,255));
  m_GUI->putText(drawlib->getDispWidth()/2,drawlib->getDispHeight()/2, msg, -0.5, -0.5);



    int nBarHeight = 15;
    m_progress = fPercent;

    
    drawLib->drawBox(Vector2f(m_InfoMsgBoxRect.nX+10,m_InfoMsgBoxRect.nY+ m_InfoMsgBoxRect.nHeight-
                                                   nBarHeight*2),
            Vector2f(m_InfoMsgBoxRect.nX+m_InfoMsgBoxRect.nWidth-10,
                     m_InfoMsgBoxRect.nY+m_InfoMsgBoxRect.nHeight-nBarHeight),
            0,MAKE_COLOR(0,0,0,255),0);
            
                
    drawLib->drawBox(Vector2f(m_InfoMsgBoxRect.nX+10,m_InfoMsgBoxRect.nY+
                                                   m_InfoMsgBoxRect.nHeight-
                                                   nBarHeight*2),
            Vector2f(m_InfoMsgBoxRect.nX + 10 + ((m_InfoMsgBoxRect.nWidth-20) * m_progress)/100,
                     m_InfoMsgBoxRect.nY+m_InfoMsgBoxRect.nHeight-nBarHeight),
            0,MAKE_COLOR(255,0,0,255),0);

    FontManager* v_fm = drawLib->getFontSmall();
    FontGlyph* v_fg = v_fm->getGlyph(m_DownloadingInformation);
    v_fm->printString(v_fg,
		      m_InfoMsgBoxRect.nX+10,
		      m_InfoMsgBoxRect.nY+m_InfoMsgBoxRect.nHeight-nBarHeight*2,
		      MAKE_COLOR(255,255,255,128));
    drawLib->flushGraphics();
#endif

}

void StateUpdate::keyDown(int nKey, SDLMod mod,int nChar)
{
  // don't call the parent keydown function. 
  // because it would allow to press F5 again for example
  attractModeKeyDown(nKey);
}

void StateUpdate::keyUp(int nKey,   SDLMod mod)
{
}

void StateUpdate::mouseDown(int nButton)
{
  StateMenu::mouseDown(nButton);
}

void StateUpdate::mouseDoubleClick(int nButton)
{
  StateMenu::mouseDoubleClick(nButton);
}

void StateUpdate::mouseUp(int nButton)
{
  StateMenu::mouseUp(nButton);
}

void StateUpdate::clean()
{
  if(StateUpdate::m_sGUI != NULL) {
    delete StateUpdate::m_sGUI;
    StateUpdate::m_sGUI = NULL;
  }
}

void StateUpdate::createGUIIfNeeded(GameApp* pGame)
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

  /* Initialize level info viewer */
  int width = drawLib->getDispWidth();
  int height= drawLib->getDispHeight();

  int x = width / 8;
  int y = height / 4;
  std::string caption = "";
  int nWidth  = width * 3/4;
  int nHeight = height / 2;

  UIFrame* v_frame;
  v_frame = new UIFrame(m_sGUI, x, y, caption, nWidth, nHeight); 
  v_frame->setID("UPDATE_FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_TRANS);

  int proH = 15;
  int proX = nWidth / 32;
  int proY = (0.75)*nHeight - proH;
  int proW = nWidth * (15.0/16.0);

  UIProgressBar* v_progress;
  v_progress = new UIProgressBar(v_frame, proX, proY, proW, proH);
  v_progress->setID("UPDATE_PROGRESS");

  UIStatic* v_static;
  v_static = new UIStatic(v_frame, x, y, "", width, height - proH);
  v_static->setID("UPDATE_TEXT");
}

void StateUpdate::updateGUI()
{
  UIProgressBar* v_progress = reinterpret_cast<UIProgressBar*>(m_GUI->getChild("UPDATE_FRAME:UPDATE_PROGRESS"));
  v_progress->setProgress(m_progress);
  v_progress->setCurrentOperation(m_currentMicroOperation);

  UIStatic* v_static = reinterpret_cast<UIStatic*>(m_GUI->getChild("UPDATE_FRAME:UPDATE_TEXT"));
  v_static->setCaption(m_currentOperation);
}
