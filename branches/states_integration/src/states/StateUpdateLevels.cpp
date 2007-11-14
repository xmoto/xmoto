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

#include "Game.h"
#include "drawlib/DrawLib.h"
#include "StateUpdateLevels.h"
#include "thread/UpdateLevelsThread.h"
#include "helpers/Log.h"

StateUpdateLevels::StateUpdateLevels(GameApp* pGame,
				     bool drawStateBehind,
				     bool updateStatesBehind)
  : StateUpdate(pGame, drawStateBehind, updateStatesBehind)
{
  m_pThread          = new UpdateLevelsThread();
  m_name             = "StateUpdateLevels";
  m_progress         = -1;
  m_currentOperation = "";
}

StateUpdateLevels::~StateUpdateLevels()
{
  delete m_pThread;
}

void StateUpdateLevels::enter()
{
  StateUpdate::enter();

  enterAttractMode(NO_KEY);
}

bool StateUpdateLevels::update()
{
  if(StateUpdate::update() == false){
    return false;
  }

  // thread finished. we leave the state.
  if(m_threadStarted == true && m_pThread->isThreadRunning() == false){
    m_pThread->waitForThreadEnd();
    m_requestForEnd = true;
    m_threadStarted = false;

    Logger::Log("thread end");

    return true;
  }

  if(m_threadStarted == false){
    m_pThread->startThread(m_pGame);
    m_threadStarted = true;

    Logger::Log("thread started");

  }

  // update the frame with the thread informations
  m_progress         = m_pThread->getThreadProgress();
  m_currentOperation = m_pThread->getThreadCurrentOperation();

  return true;
}

bool StateUpdateLevels::render()
{
  int ret = StateUpdate::render();

  DrawLib* drawLib = m_pGame->getDrawLib();

  int width = drawLib->getDispWidth();
  int height= drawLib->getDispHeight();

  int x = width / 8;
  int y = height / 4;
  std::string caption = "";
  int nWidth  = width * 3/4;
  int nHeight = height / 2;


  // draw text
  m_GUI->setTextSolidColor(MAKE_COLOR(255,255,255,255));
  m_GUI->putText(drawLib->getDispWidth()/2,
		 drawLib->getDispHeight()/2,
		 m_currentOperation,
		 -0.5, -0.5);

#if 0
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

  return ret;
}

void StateUpdateLevels::updateGUI()
{
  
}
