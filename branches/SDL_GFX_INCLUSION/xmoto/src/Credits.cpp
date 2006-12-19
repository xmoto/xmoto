/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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

#define CREDITS_DARKNESS 0.55    /* a number between 0 (not darkened) and 
                                    1 (black) */

namespace vapp {

  Credits::Credits() {
    m_bFinished = true;
    m_pApp = NULL;
    m_pFont = NULL;
  }
  
  Credits::~Credits() {
    /* Free all entries */
    for(int i=0;i<m_Entries.size();i++)
      delete m_Entries[i];
    m_Entries.clear();
  }
  
  bool Credits::isFinished(void) {
    return m_bFinished;
  }
  
  void Credits::init(float fBackgroundReplayLength,float fFadeInLength,float fFadeOutLength,const char *pcCredits) {
    if(fBackgroundReplayLength <= 0) {
      /* No replay or empty replay, just use black background */
      m_bBlackBackground = true;
    }
    else {
      /* Setup credits */
      m_bBlackBackground = false;

      m_fReplayLength = fBackgroundReplayLength;
      m_fFadeIn = fFadeInLength;
      m_fFadeOut = fFadeOutLength;
      
      if(m_fFadeIn + m_fFadeOut + 3 > m_fReplayLength) {
        m_fFadeIn = m_fFadeOut = (m_fReplayLength - 3) / 2;
      }
    }

    m_bFinished = false;
    
    m_pApp = UITextDraw::getApp();
    
    /* Get font */
    m_pFont = UITextDraw::getFont("MFont");
    if(m_pFont == NULL) {
      m_bFinished = true;
    }
    else {    
      /* Parse credits string */
      int nL = strlen(pcCredits);
      char cBuf[256];
      int nBufLen = 0;
      std::string Left,Right;
      
      m_nWidestLeft = 0;
      m_nWidestRight = 0;
      
      for(int i=0;i<nL;i++) {
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
          Entry *p = new Entry;
          p->Left = Left;
          p->Right = Right;
          m_Entries.push_back(p);
          
          /* Keep track of longest left and right strings */
          p->nLeftWidth = _GetStringWidth(Left);
          p->nRightWidth = _GetStringWidth(Right);
          
          m_nWidestLeft = p->nLeftWidth > m_nWidestLeft ? p->nLeftWidth : m_nWidestLeft;
          m_nWidestRight = p->nRightWidth > m_nWidestRight ? p->nRightWidth : m_nWidestRight;          
        }
        else {
          /* Add character to buffer */
          if(nBufLen < sizeof(cBuf) - 1) {
            cBuf[nBufLen] = pcCredits[i];
            nBufLen++;
          }
        }
      }
    }
    
    /* Start from the beginning... */
    m_fTime = 0;
  }
  
  void Credits::render(float fTime) {
    m_fTime = fTime;
  
    if(!m_bFinished && m_pApp!=NULL && m_pFont!=NULL) {
      /* Calculate fade... */
      float fFade = 0;        
        
      /* Fading in? */
      if(m_fTime < 3) {
        fFade = 1;
      }
      else if(m_fTime < m_fFadeIn + 3) {
        fFade = 1 - ((m_fTime - 3) / m_fFadeIn);
      }
      /* Fading out? */
      else if(m_fTime > m_fReplayLength - m_fFadeOut) {
        fFade = (m_fTime - (m_fReplayLength - m_fFadeOut)) / m_fFadeOut;
      }
            
      fFade = CREDITS_DARKNESS + fFade * (1 - CREDITS_DARKNESS);

      int nC = (int)(fFade * 255);
      if(nC < 0) nC = 0;
      if(nC > 255) nC = 255;
      
      if(!m_pApp->isUglyMode())
        m_pApp->getDrawLib()->drawBox(Vector2f(0,0),Vector2f(m_pApp->getDrawLib()->getDispWidth(),m_pApp->getDrawLib()->getDispHeight()),0,MAKE_COLOR(0,0,0,nC),0);      
      
      /* Render text */
      int nScroll = 20 + m_pApp->getDrawLib()->getDispHeight() - fTime * 20;
      int nY = 0;
      
      for(int i=0;i<m_Entries.size();i++) {
        if(nY < 0) continue;
        
        int nX = m_pApp->getDrawLib()->getDispWidth() - m_Entries[i]->nLeftWidth - 40 - m_nWidestRight;
        Color Yellow = MAKE_COLOR(255,255,64,32);
        Color White = MAKE_COLOR(255,255,255,255);
        UITextDraw::printRawGrad(m_pFont,nX,nY+nScroll,m_Entries[i]->Left,White,White,Yellow,Yellow);
        UITextDraw::printRawGrad(m_pFont,nX + m_Entries[i]->nLeftWidth + 40,nY+nScroll,m_Entries[i]->Right,White,White,Yellow,Yellow);
        
        nY += 30;
      }
      
      if(!m_pApp->isUglyMode()) {
        /* Render fancyness */
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);      
	m_pApp->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
	m_pApp->getDrawLib()->setColorRGBA(0,0,0,255);
        m_pApp->getDrawLib()->glVertexSP(0,0); 
	m_pApp->getDrawLib()->glVertexSP(m_pApp->getDrawLib()->getDispWidth(),0);
	m_pApp->getDrawLib()->setColorRGBA(0,0,0,0);
        m_pApp->getDrawLib()->glVertexSP(m_pApp->getDrawLib()->getDispWidth(),m_pApp->getDrawLib()->getDispHeight() / 6); m_pApp->getDrawLib()->glVertexSP(0,m_pApp->getDrawLib()->getDispHeight() / 6); 
	m_pApp->getDrawLib()->endDraw();
        m_pApp->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
	m_pApp->getDrawLib()->setColorRGBA(0,0,0,0);
        m_pApp->getDrawLib()->glVertexSP(0,m_pApp->getDrawLib()->getDispHeight() - m_pApp->getDrawLib()->getDispHeight() / 6); 
        m_pApp->getDrawLib()->glVertexSP(m_pApp->getDrawLib()->getDispWidth(),m_pApp->getDrawLib()->getDispHeight() - m_pApp->getDrawLib()->getDispHeight() / 6);
	m_pApp->getDrawLib()->setColorRGBA(0,0,0,255);
        m_pApp->getDrawLib()->glVertexSP(m_pApp->getDrawLib()->getDispWidth(),m_pApp->getDrawLib()->getDispHeight()); 
	m_pApp->getDrawLib()->glVertexSP(0,m_pApp->getDrawLib()->getDispHeight()); 
        m_pApp->getDrawLib()->endDraw();

        glDisable(GL_BLEND);
      }
      
      if(m_fTime > m_fReplayLength - 4) {
        float fFinalFadeIn = (m_fTime - (m_fReplayLength - 4)) / 4;
        if(fFinalFadeIn > 1) fFinalFadeIn = 1;
        int nC2 = (int)(fFinalFadeIn * 255);
        if(nC2 < 0) nC2 = 0;
        if(nC2 > 255) nC2 = 255;
        const char *pc = "X-Moto";
        
        int nTWidth = _GetStringWidth(pc);
        
        if(!m_pApp->isUglyMode()) {
          m_pApp->getDrawLib()->drawBox(Vector2f(0,0),Vector2f(m_pApp->getDrawLib()->getDispWidth(),m_pApp->getDrawLib()->getDispHeight()),0,MAKE_COLOR(0,0,0,nC2),0);      
        }
        
        Color Yellow = MAKE_COLOR(255,255,0,nC2);
        Color White = MAKE_COLOR(255,255,255,nC2);
        UITextDraw::printRawGrad(m_pFont,m_pApp->getDrawLib()->getDispWidth()/2 - nTWidth/2,m_pApp->getDrawLib()->getDispHeight()/2,
                                 pc,White,White,Yellow,Yellow);        
      }
    }
  }

  int Credits::_GetStringWidth(const std::string &s) {
    int nMinX,nMinY,nMaxX,nMaxY;
    UITextDraw::getTextExt(m_pFont,s,&nMinX,&nMinY,&nMaxX,&nMaxY);
    return nMaxX - nMinX;    
  }

}
