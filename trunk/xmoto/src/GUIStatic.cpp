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
    int x1,y1,x2,y2,x=0,y=0;  
    
    getTextExt(getCaption(),&x1,&y1,&x2,&y2);
          
    /* Find out where to draw the text */
    if(getHAlign() == UI_ALIGN_LEFT)
      x = -x1;
    else if(getHAlign() == UI_ALIGN_RIGHT)
      x = getPosition().nWidth - x2;
    else if(getHAlign() == UI_ALIGN_CENTER)
      x = getPosition().nWidth/2 - (x2-x1)/2 - x1;

    if(getVAlign() == UI_ALIGN_TOP)
      y = -y1;
    else if(getVAlign() == UI_ALIGN_BOTTOM)
      y = getPosition().nHeight - y2;
    else if(getVAlign() == UI_ALIGN_CENTER)
      y = getPosition().nHeight/2 - (y2-y1)/2 - y1;
    
    putText(x,y,getCaption());
  }

}

