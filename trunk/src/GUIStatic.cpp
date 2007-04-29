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
 *  GUI: Static text stuff
 */
#include "VXml.h"
#include "GUI.h"

namespace vapp {

  /*===========================================================================
  Painting
  ===========================================================================*/
  void UIStatic::paint(void) {  
    if(isUglyMode() == false) {
      /* Darken background? */
      if(m_bBackgroundShade) {
	putImage(0,0,getPosition().nWidth,getPosition().nHeight,m_pDarkBlobTexture);
      }
    
      /* Background image? */
      if(m_pCustomBackgroundTexture != NULL) {
	putImage(0,0,getPosition().nWidth,getPosition().nHeight,m_pCustomBackgroundTexture);    
      }    
    }  

    /* Determine text size */
    int v_x=0, v_y=0;
    float perX=0.0, perY=0.0;
    
    /* Find out where to draw the text */
    if(getHAlign() == UI_ALIGN_LEFT) {
      v_x = 0;
    } else if(getHAlign() == UI_ALIGN_RIGHT) {
      v_x = getPosition().nWidth;
      perX=-1.0;
    } else if(getHAlign() == UI_ALIGN_CENTER) {
      v_x = getPosition().nWidth/2;
      perX = -0.5;
    }

    if(getVAlign() == UI_ALIGN_TOP) {
      v_y = 0;
    } else if(getVAlign() == UI_ALIGN_BOTTOM) {
      v_y = getPosition().nHeight;
      perY=-1.0;
    } else if(getVAlign() == UI_ALIGN_CENTER) {
      v_y = getPosition().nHeight/2;
      perY = -0.5;
    }    

    putText(v_x, v_y, getCaption(), perX, perY);
  }

}

