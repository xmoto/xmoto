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
 *  GUI: Framed window class
 */
#include "VXml.h"
#include "GUI.h"

namespace vapp {

  /*===========================================================================
  Painting
  ===========================================================================*/
  void UIFrame::paint(void) {
    /* This depends on the style */
    switch(m_Style) {
      case UI_FRAMESTYLE_MENU:
        putImage(0,0,getPosition().nWidth/2,getPosition().nHeight/2,m_pMenuTL);
        putImage(getPosition().nWidth/2,0,getPosition().nWidth/2,getPosition().nHeight/2,m_pMenuTR);
        putImage(getPosition().nWidth/2,getPosition().nHeight/2,getPosition().nWidth/2,getPosition().nHeight/2,m_pMenuBR);
        putImage(0,getPosition().nHeight/2,getPosition().nWidth/2,getPosition().nHeight/2,m_pMenuBL);
        break;
      case UI_FRAMESTYLE_TRANS:
        putElem(0,0,-1,-1,UI_ELEM_FRAME_TL,false);
        putElem(getPosition().nWidth-8,0,-1,-1,UI_ELEM_FRAME_TR,false);
        putElem(getPosition().nWidth-8,getPosition().nHeight-8,-1,-1,UI_ELEM_FRAME_BR,false);
        putElem(0,getPosition().nHeight-8,-1,-1,UI_ELEM_FRAME_BL,false);
        putElem(8,0,getPosition().nWidth-16,-1,UI_ELEM_FRAME_TM,false);
        putElem(8,getPosition().nHeight-8,getPosition().nWidth-16,-1,UI_ELEM_FRAME_BM,false);
        putElem(0,8,-1,getPosition().nHeight-16,UI_ELEM_FRAME_ML,false);
        putElem(getPosition().nWidth-8,8,-1,getPosition().nHeight-16,UI_ELEM_FRAME_MR,false);
        putRect(8,8,getPosition().nWidth-16,getPosition().nHeight-16,MAKE_COLOR(0,0,0,127));
        break;
    }
  }

};
