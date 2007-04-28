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
 *  GUI: Framed besttimes window
 */
#include "GameText.h"
#include "VXml.h"
#include "GUI.h"

namespace vapp {

  /*===========================================================================
  Clearing
  ===========================================================================*/
  void UIBestTimes::clear(void) {
    m_Col1.clear();
    m_Col2.clear();
    m_Col3.clear();
    m_Col4.clear();
  }

  /*===========================================================================
  Painting
  ===========================================================================*/
  void UIBestTimes::paint(void) {
    /* Do background */
    UIFrame::paint();
    
    /* Stuff */
    FontManager* pOldFont = getFont();
    setFont(m_hFont);
    putText(20,20,m_Header);

    setFont(m_drawLib->getFontSmall());    
    putText(40,64,std::string(GAMETEXT_ALLRECORDS) + ": ");
    for(int i=0;i<m_Col1.size();i++) {
      if(i == m_nHighlight1) {
        setTextSolidColor(MAKE_COLOR(255,255,0,255));
      }
      putText(30,90+i*16,m_Col1[i]);
      putText(110,90+i*16,m_Col2[i]);
      setTextSolidColor(MAKE_COLOR(255,255,255,255));
    }

    putText(40,100 + 16*m_Col1.size(),std::string(GAMETEXT_PERSONALRECORDS) + ":");
    for(int i=0;i<m_Col3.size();i++) {
      if(i == m_nHighlight2) {
        setTextSolidColor(MAKE_COLOR(255,255,0,255));
      }
      putText(30,126+i*16 + 16*m_Col1.size(),m_Col3[i]);
      putText(110,126+i*16 + 16*m_Col1.size(),m_Col4[i]);
      setTextSolidColor(MAKE_COLOR(255,255,255,255));
    }
    
    setFont(pOldFont);
  }

}

