/*=============================================================================
XMOTO
Copyright (C) 2005 Rasmus Neckelmann (neckelmann@gmail.com)

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
 *  GUI: tab view control
 */
#include "VXml.h"
#include "GUI.h"

namespace vapp {

  /*===========================================================================
  Painting
  ===========================================================================*/
  void UITabView::paint(void) {    
    /* Header height */
    int nHeaderHeight=24;
  
    /* Render bottom part (common to all tabs) */
    putElem(getPosition().nWidth-8,nHeaderHeight,-1,-1,UI_ELEM_FRAME_TR,false);
    putElem(getPosition().nWidth-8,getPosition().nHeight-8,-1,-1,UI_ELEM_FRAME_BR,false);
    putElem(0,getPosition().nHeight-8,-1,-1,UI_ELEM_FRAME_BL,false);
    putElem(8,getPosition().nHeight-8,getPosition().nWidth-16,-1,UI_ELEM_FRAME_BM,false);
    putElem(0,8 + nHeaderHeight,-1,getPosition().nHeight-16-nHeaderHeight,UI_ELEM_FRAME_ML,false);
    putElem(getPosition().nWidth-8,8 + nHeaderHeight,-1,getPosition().nHeight-16-nHeaderHeight,UI_ELEM_FRAME_MR,false);
    putRect(8,8 + nHeaderHeight,getPosition().nWidth-16,getPosition().nHeight-16-nHeaderHeight,MAKE_COLOR(0,0,0,127));
    
    /* Render tabs */
    int nCX = 8;
    int nCY = (2*nHeaderHeight)/3;    

    for(int i=0;i<getChildren().size();i++) {
      int x1,x2;    
      getTextExt(getChildren()[i]->getCaption(),&x1,NULL,&x2,NULL);      
      if(m_nSelected != i) {
        putElem(nCX-8,0,-1,-1,UI_ELEM_FRAME_TL,false);        
        putElem(nCX,0,(x2-x1),8,UI_ELEM_FRAME_TM,false);
        putElem(nCX+(x2-x1),0,-1,-1,UI_ELEM_FRAME_TR,false);                
        //putElem(nCX+(x2-x1)+8,nHeaderHeight,getPosition().nWidth-(nCX+(x2-x1))-16,8,UI_ELEM_FRAME_TM,false);
        putElem(nCX-8,8,-1,nHeaderHeight-6,UI_ELEM_FRAME_ML,false);
        putElem(nCX+(x2-x1),8,-1,nHeaderHeight-8+2,UI_ELEM_FRAME_MR,false);
        //
        //if(i != 0) {
        //  putElem(8,nHeaderHeight,nCX-14,8,UI_ELEM_FRAME_TM,false);
        //}
        
        putRect(nCX,8,x2-x1,nHeaderHeight-8,MAKE_COLOR(0,0,0,127));
        setTextSolidColor(MAKE_COLOR(188,186,67,255));
        putText(nCX,nCY,getChildren()[i]->getCaption());
        setTextSolidColor(-1);      
      }      
      nCX += (x2-x1) + 18;
    }

    nCX = 8;
    nCY = (2*nHeaderHeight)/3;    

    for(int i=0;i<getChildren().size();i++) {
      int x1,x2;    
      getTextExt(getChildren()[i]->getCaption(),&x1,NULL,&x2,NULL);      
      if(m_nSelected == i) {
        putElem(nCX-8,0,-1,-1,UI_ELEM_FRAME_TL,false);        
        putElem(nCX,0,(x2-x1),8,UI_ELEM_FRAME_TM,false);
        putElem(nCX+(x2-x1),0,-1,-1,UI_ELEM_FRAME_TR,false);                
        putElem(nCX+(x2-x1)+8,nHeaderHeight,getPosition().nWidth-(nCX+(x2-x1))-16,8,UI_ELEM_FRAME_TM,false);
        putElem(nCX-8,8,-1,nHeaderHeight-8,UI_ELEM_FRAME_ML,false);
        putElem(nCX+(x2-x1),8,-1,nHeaderHeight-8+2,UI_ELEM_FRAME_MR,false);
        
        if(i != 0) {
          putElem(8,nHeaderHeight,nCX-14,8,UI_ELEM_FRAME_TM,false);
          putRect(nCX-6,8+nHeaderHeight-8,6,8,MAKE_COLOR(0,0,0,127));
        }
        
        putRect(nCX,8,x2-x1,nHeaderHeight,MAKE_COLOR(0,0,0,127));
        putRect(nCX+(x2-x1),8+nHeaderHeight-6,8,6,MAKE_COLOR(0,0,0,127));
        putText(nCX,nCY,getChildren()[i]->getCaption());
        break;
      }      
      nCX += (x2-x1) + 18;
    }
    
    if(m_nSelected == 0) {
      putElem(0,8 + nHeaderHeight -8,-1,-1,UI_ELEM_FRAME_ML,false);      
    }
    else {
      putElem(0,8 + nHeaderHeight -8,-1,-1,UI_ELEM_FRAME_TL,false);      
    }
  }

  /*===========================================================================
  Mouse event handling
  ===========================================================================*/
  void UITabView::mouseLDown(int x,int y) {
    /* Nice. Find out what tab was clicked (if any) */
    /* Header height */
    int nHeaderHeight=24;
    int nCX = 8;
    int nCY = (2*nHeaderHeight)/3;    

    for(int i=0;i<getChildren().size();i++) {
      int x1,x2;    
      getTextExt(getChildren()[i]->getCaption(),&x1,NULL,&x2,NULL);      
      if(x >= nCX-8 && y >= 0 && x < nCX+16+(x2-x1) && y < nHeaderHeight) {
        m_nSelected = i;
        
        /* Hide everything except this */
        for(int j=0;j<getChildren().size();j++) { 
          if(j == i) {
            getChildren()[j]->showWindow(true);
          }
          else {
            getChildren()[j]->showWindow(false);
          }          
        }
        
        break;
      }
      nCX += (x2-x1) + 18;
    }
  }
  
  void UITabView::mouseLUp(int x,int y) {
  }
  
  void UITabView::mouseRDown(int x,int y) {
  }
  
  void UITabView::mouseRUp(int x,int y) {
  }

  void UITabView::mouseHover(int x,int y) {
  }

};

