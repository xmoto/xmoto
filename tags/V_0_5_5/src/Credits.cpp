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

/* 
 *  Scrolling credits stuff
 */
#include "Credits.h"
#include "drawlib/DrawLib.h"
#include "Game.h"
#include "helpers/RenderSurface.h"

#define CREDITS_DARKNESS 0.55    /* a number between 0 (not darkened) and 
                                    1 (black) */
Credits::Credits() {
  m_bFinished = true;
}
  
Credits::~Credits() {
  /* Free all entries */
  for(unsigned int i=0;i<m_Entries.size();i++)
    delete m_Entries[i];
  m_Entries.clear();
}
  
bool Credits::isFinished(void) {
  return m_bFinished;
}
  
void Credits::init(int backgroundReplayLength,
		   int fadeInLength,
		   int fadeOutLength,
		   const char *pcCredits) {
  if(backgroundReplayLength <= 0) {
    /* No replay or empty replay, just use black background */
    m_bBlackBackground = true;
  }
  else {
    /* Setup credits */
    m_bBlackBackground = false;

    m_replayLength = backgroundReplayLength;
    m_fadeIn = fadeInLength;
    m_fadeOut = fadeOutLength;
      
    if(m_fadeIn + m_fadeOut + 300 > m_replayLength) {
      m_fadeIn = m_fadeOut = (m_replayLength - 300) / 2;
    }
  }

  m_bFinished = false;

  /* Parse credits string */
  unsigned int nL = strlen(pcCredits);
  char cBuf[256];
  unsigned int nBufLen = 0;
  std::string Left, Right;
    
  for(unsigned int i=0; i<nL; i++) {
    if(pcCredits[i] == ':') {
      /* Got left part of entry */
      cBuf[nBufLen] = '\0';
      nBufLen = 0;
      Left = cBuf;
    }
    else if(pcCredits[i] == ';') {
      /* Got right part of entry */
      cBuf[nBufLen] = '\0';
      nBufLen = 0;
      Right = cBuf;
	
      /* Register entry */
      Entry *p;

      if(Left != "") {
	p = new Entry;
	p->Left = Left;
	p->Right = "";
	m_Entries.push_back(p);
      }
      if(Right != "") {
	p = new Entry;
	p->Left = "";
	p->Right = Right;
	m_Entries.push_back(p);
      }
    }
    else {
      /* Add character to buffer */
      if(nBufLen < sizeof(cBuf) - 1) {
	cBuf[nBufLen] = pcCredits[i];
	nBufLen++;
      }
    }
  }
    
  /* Start from the beginning... */
  m_time = 0;
}
  
void Credits::render(RenderSurface* i_screen, int i_time) {
  m_time = i_time;
  
  if(!m_bFinished) {
    DrawLib* drawLib = GameApp::instance()->getDrawLib();

    /* Calculate fade... */
    float fade = 0.0;

    /* Fading in? */
    if(m_time < 300) {
      fade = 1;
    }
    else if(m_time < m_fadeIn + 300) {
      fade = 1.0 - ((m_time - 300) / ((float)m_fadeIn));
    }
    /* Fading out? */
    else if(m_time > m_replayLength - m_fadeOut) {
      fade = (m_time - (m_replayLength - m_fadeOut)) / m_fadeOut;
    }
            
    fade = CREDITS_DARKNESS + fade * (1.0 - CREDITS_DARKNESS);

    int nC = (int)(fade * 255);
    if(nC < 0) nC = 0;
    if(nC > 255) nC = 255;
      
    if(!XMSession::instance()->ugly())
      drawLib->drawBox(Vector2f(0,0),
		       Vector2f(i_screen->getDispWidth(),i_screen->getDispHeight()),
		       0,MAKE_COLOR(0,0,0,nC),0);      
      
    /* Render text */
    int nScroll = (int)(i_screen->getDispHeight() - i_time/100.0 * 15);
    int nY = 0;
      
    for(unsigned int i=0;i<m_Entries.size();i++) {
      if(nY < 0) continue;
      Color Yellow = MAKE_COLOR(255,255,64,32);
      Color White = MAKE_COLOR(255,255,255,255);

      FontManager* v_fm = drawLib->getFontMedium();
      FontGlyph* v_fg;

      v_fg = v_fm->getGlyph(m_Entries[i]->Left);
      v_fm->printStringGrad(drawLib, v_fg,
			    20, nY+nScroll,
			    White,White,Yellow,Yellow);
      v_fg = v_fm->getGlyph(m_Entries[i]->Right);
      v_fm->printStringGrad(drawLib, v_fg,
			    200, nY+nScroll,
			    White,White,Yellow,Yellow);
      nY += 30;
    }
      
    if(!XMSession::instance()->ugly()) {
      /* Render fancyness */
      drawLib->setBlendMode(BLEND_MODE_A);
      drawLib->startDraw(DRAW_MODE_POLYGON);
      drawLib->setColorRGBA(0,0,0,255);
      drawLib->glVertexSP(0,0); 
      drawLib->glVertexSP(i_screen->getDispWidth(),0);
      drawLib->setColorRGBA(0,0,0,0);
      drawLib->glVertexSP(i_screen->getDispWidth(),i_screen->getDispHeight() / 6); drawLib->glVertexSP(0,i_screen->getDispHeight() / 6); 
      drawLib->endDraw();

      drawLib->setBlendMode(BLEND_MODE_A);
      drawLib->startDraw(DRAW_MODE_POLYGON);
      drawLib->setColorRGBA(0,0,0,0);
      drawLib->glVertexSP(0,i_screen->getDispHeight() - i_screen->getDispHeight() / 6); 
      drawLib->glVertexSP(i_screen->getDispWidth(),i_screen->getDispHeight() - i_screen->getDispHeight() / 6);
      drawLib->setColorRGBA(0,0,0,255);
      drawLib->glVertexSP(i_screen->getDispWidth(),i_screen->getDispHeight()); 
      drawLib->glVertexSP(0,i_screen->getDispHeight()); 
      drawLib->endDraw();

    }
      
    if(m_time > m_replayLength - 400) {
      int finalFadeIn = (m_time - (m_replayLength - 400)) / 4;
      if(finalFadeIn > 100) finalFadeIn = 100;
      int nC2 = (int)(finalFadeIn/100.0 * 255);
      if(nC2 < 0) nC2 = 0;
      if(nC2 > 255) nC2 = 255;
      const char *pc = "X-Moto";
        
      if(!XMSession::instance()->ugly()) {
	drawLib->drawBox(Vector2f(0,0),Vector2f(i_screen->getDispWidth(),i_screen->getDispHeight()),0,MAKE_COLOR(0,0,0,nC2),0);      
      }
        
      Color Yellow = MAKE_COLOR(220,220,0,nC2);
      Color White = MAKE_COLOR(255,255,255,nC2);

      FontManager* v_fm = drawLib->getFontBig();
      FontGlyph* v_fg = v_fm->getGlyph(pc);
      v_fm->printStringGrad(drawLib, v_fg,
			    i_screen->getDispWidth()/2 - v_fg->realWidth()/2,
			    i_screen->getDispHeight()/2 - v_fg->realHeight()/2,
			    White,White,Yellow,Yellow);
    }
  }
}
