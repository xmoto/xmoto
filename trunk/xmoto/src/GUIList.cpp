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
 *  GUI: list control
 */
#include "VXml.h"
#include "GUI.h"
#include "Sound.h"

namespace vapp {

  /*===========================================================================
  Painting
  ===========================================================================*/
  void UIList::paint(void) {
    bool bDisabled = isDisabled();
    bool bActive = isActive();
    
    m_bItemActivated = false;
  
    /* Draw list frame */
    putElem(0,0,-1,-1,UI_ELEM_FRAME_TL,false);
    putElem(getPosition().nWidth-8,0,-1,-1,UI_ELEM_FRAME_TR,false);
    putElem(getPosition().nWidth-8,getPosition().nHeight-8,-1,-1,UI_ELEM_FRAME_BR,false);
    putElem(0,getPosition().nHeight-8,-1,-1,UI_ELEM_FRAME_BL,false);
    putElem(8,0,getPosition().nWidth-16,-1,UI_ELEM_FRAME_TM,false);
    putElem(8,getPosition().nHeight-8,getPosition().nWidth-16,-1,UI_ELEM_FRAME_BM,false);
    putElem(0,8,-1,getPosition().nHeight-16,UI_ELEM_FRAME_ML,false);
    putElem(getPosition().nWidth-8,8,-1,getPosition().nHeight-16,UI_ELEM_FRAME_MR,false);
    putRect(8,8,getPosition().nWidth-16,getPosition().nHeight-16,MAKE_COLOR(0,0,0,127));
        
    /* Draw column headers */
    int nHeaderHeight = 18;
    int nHX = 6,nHY = 6;
    setTextSolidColor(MAKE_COLOR(188,186,67,255));
    for(int i=0;i<m_Columns.size();i++) {
      putText(nHX,nHY + (nHeaderHeight*2)/3,m_Columns[i]);
      nHX += m_ColumnWidths[i];            
    }
    putRect(6,nHeaderHeight+6,getPosition().nWidth-12 - 20,2,MAKE_COLOR(188,186,67,255));
        
    /* Render list */    
    int nLX = 6, nLY = nHeaderHeight+6+4;
    int nLWidth = getPosition().nWidth-12 - 20;
    int nLHeight = getPosition().nHeight - nLY - 6;
    int nRowHeight = 16;

    if(!isMouseLDown()) {
      m_bScrollDownPressed = false;
      m_bScrollUpPressed = false;
    }

    if(m_bScrollUpPressed && m_bScrollUpHover) {
      putElem(nLX+nLWidth,6,20,20,UI_ELEM_SCROLLBUTTON_UP_DOWN,false);
    }
    else
      putElem(nLX+nLWidth,6,20,20,UI_ELEM_SCROLLBUTTON_UP_UP,false);
      
    if(m_bScrollDownPressed && m_bScrollDownHover) {
      putElem(nLX+nLWidth,nLY+nLHeight-20,20,20,UI_ELEM_SCROLLBUTTON_DOWN_DOWN,false);
    }
    else
      putElem(nLX+nLWidth,nLY+nLHeight-20,20,20,UI_ELEM_SCROLLBUTTON_DOWN_UP,false);    

    setScissor(nLX,nLY,nLWidth,nLHeight);
   
    if(!bDisabled)                     
      setTextSolidColor(MAKE_COLOR(255,255,255,255));
    else
      setTextSolidColor(MAKE_COLOR(128,128,128,255));
    
    for(int i=0;i<m_Entries.size();i++) {
      int y = m_nScroll + i*nRowHeight + (nRowHeight*2)/3;
      
      if(m_nSelected == i) {
        Color c = MAKE_COLOR(70,70,70,255);
        if(!bDisabled) c = MAKE_COLOR(160,40,40,255);
        putRect(nLX,m_nScroll + nLY+i*nRowHeight,nLWidth,nRowHeight,c);       
        
        if(bActive && !bDisabled) {
          float s = 50 + 50*sin(getApp()->getRealTime()*10);
          int n = (int)s;
          if(n<0) n=0;
          if(n>255) n=255; /* just to be sure, i'm lazy */    

          putRect(nLX,m_nScroll+ nLY+i*nRowHeight,nLWidth,nRowHeight,MAKE_COLOR(255,255,255,n));                 
        }
      }              
      
      int x = 0;      
      for(int j=0;j<m_Entries[i]->Text.size();j++) {              
        putText(nLX+x,nLY+y,m_Entries[i]->Text[j]);
        x += m_ColumnWidths[j];                
      }
    }
    
    /* Stuff */
    m_bScrollDownHover = false;
    m_bScrollUpHover = false;
  }

  /*===========================================================================
  Mouse event handling
  ===========================================================================*/
  void UIList::mouseWheelDown(int x,int y) {
    /* Scroll down! */
    _Scroll(-16);
  }
  
  void UIList::mouseWheelUp(int x,int y) {
    /* Scroll up! */
    _Scroll(16);
  }
  
  void UIList::mouseLDoubleClick(int x,int y) {
    int nHeaderHeight=18;
    int nLX = 6, nLY = nHeaderHeight+6+4;
    int nLWidth = getPosition().nWidth-12 - 20;
    int nLHeight = getPosition().nHeight - nLY - 6;
    int nRowHeight = 16;    

    /* Is it down inside one of the scroll buttons? */
    if(x >= nLX+nLWidth && x < nLX+nLWidth + 20 &&
       y >= 6 && y < 6+20) {
      /* Scroll up! */
      m_bScrollUpPressed = true;
    } 
    else if(x >= nLX+nLWidth && x < nLX+nLWidth + 20 &&
       y >= nLY+nLHeight-20 && y < nLY+nLHeight) {
      /* Scroll down! */
      m_bScrollDownPressed = true;
    }    
    else {
      /* Find out what item is affected */
      for(int i=0;i<m_Entries.size();i++) {
        int yy = m_nScroll + nLY + i*nRowHeight;
        if(x >= nLX && x <getPosition().nWidth-6 &&
          y >= yy && y < yy+nRowHeight) {
          /* Select this */
          m_nSelected = i;
          _NewlySelectedItem();
          
          /* AND invoke enter-button */
          if(getSelected()>=0 && getSelected()<m_Entries.size() && m_pEnterButton != NULL) {
            m_pEnterButton->setClicked(true);

            Sound::playSampleByName("Sounds/Button3.ogg");
          }          

          /* Under all circumstances set the activation flag */
          m_bItemActivated = true;
          break;
        }
      }
    }
  }
  
  void UIList::mouseLDown(int x,int y) {
    int nHeaderHeight=18;
    int nLX = 6, nLY = nHeaderHeight+6+4;
    int nLWidth = getPosition().nWidth-12 - 20;
    int nLHeight = getPosition().nHeight - nLY - 6;
    int nRowHeight = 16;    

    /* Is it down inside one of the scroll buttons? */
    if(x >= nLX+nLWidth && x < nLX+nLWidth + 20 &&
       y >= 6 && y < 6+20) {
      /* Scroll up! */
      m_bScrollUpPressed = true;
    } 
    else if(x >= nLX+nLWidth && x < nLX+nLWidth + 20 &&
       y >= nLY+nLHeight-20 && y < nLY+nLHeight) {
      /* Scroll down! */
      m_bScrollDownPressed = true;
    }    
    else {
      /* Find out what item is affected */
      for(int i=0;i<m_Entries.size();i++) {
        int yy = m_nScroll + nLY + i*nRowHeight;
        if(x >= nLX && x <getPosition().nWidth-6 &&
          y >= yy && y < yy+nRowHeight) {
          /* Select this */
          m_nSelected = i;
          _NewlySelectedItem();
          break;
        }
      }
    }
  }
  
  void UIList::mouseLUp(int x,int y) {
    int nHeaderHeight=18;
    int nLX = 6, nLY = nHeaderHeight+10;
    int nLWidth = getPosition().nWidth-32;
    int nLHeight = getPosition().nHeight - nLY - 6;
    int nRowHeight = 16;    

    /* Is it down inside one of the scroll buttons? */
    if(x >= nLX+nLWidth && x < nLX+nLWidth + 20 &&
       y >= 6 && y < 26) {
      /* Scroll up! */
      _Scroll(16);
    } 
    else if(x >= nLX+nLWidth && x < nLX+nLWidth + 20 &&
       y >= nLY+nLHeight-20 && y < nLY+nLHeight) {
      /* Scroll down! */
      _Scroll(-16);
    }    
  }
  
  void UIList::mouseRDown(int x,int y) {
  }
  
  void UIList::mouseRUp(int x,int y) {
  }

  void UIList::mouseHover(int x,int y) {
    int nHeaderHeight=18;
    int nLX = 6, nLY = nHeaderHeight+6+4;
    int nLWidth = getPosition().nWidth-12 - 20;
    int nLHeight = getPosition().nHeight - nLY - 6;
    int nRowHeight = 16;    

    /* Is it down inside one of the scroll buttons? */
    if(x >= nLX+nLWidth && x < nLX+nLWidth+20 &&
       y >= 6 && y < 6+20) {
      /* Scroll up! */
      m_bScrollUpHover = true;
    } 

    if(x >= nLX+nLWidth && x < nLX+nLWidth+20 &&
       y >= nLY+nLHeight-20 && y < nLY+nLHeight) {
      /* Scroll down! */
      m_bScrollDownHover = true;
    }    
  }

  /*===========================================================================
  Allocate entry / vice versa
  ===========================================================================*/
  UIListEntry *UIList::addEntry(std::string Text,void *pvUser) {
    UIListEntry *p = new UIListEntry;
    p->Text.push_back(Text);
    p->pvUser = pvUser;
    m_Entries.push_back(p);
    return p;
  }
  
  void UIList::clear(void) {
    _FreeUIList();
    m_Entries.clear();
    m_nSelected = 0;
  }

  /*===========================================================================
  Free entries
  ===========================================================================*/
  void UIList::_FreeUIList(void) {
    for(int i=0;i<m_Entries.size();i++)
      delete m_Entries[i];
  }

  /*===========================================================================
  Accept activation if there's actually anything in the list
  ===========================================================================*/
  bool UIList::offerActivation(void) {
    if(m_Entries.empty()) return false;
    return true;    
  }

  /*===========================================================================
  Up/down keys select elements
  ===========================================================================*/
  bool UIList::keyDown(int nKey,int nChar) {
    switch(nKey) {
      case SDLK_RETURN: 
        /* Uhh... send this to the default button, if any. And if anything is selected */
        if(getSelected()>=0 && getSelected()<m_Entries.size() && m_pEnterButton != NULL) {
          m_pEnterButton->setClicked(true);       
          
          Sound::playSampleByName("Sounds/Button3.ogg");
        }
        
        /* Under all circumstances set the activation flag */
        m_bItemActivated = true;
        return true;
    
      case SDLK_UP:
        if(getSelected() <= 0)
          getRoot()->activateUp();
        else {
          setSelected(getSelected() - 1);
          _NewlySelectedItem();
        }
        return true;
      case SDLK_DOWN:
        if(getSelected() >= m_Entries.size() - 1)
          getRoot()->activateDown();
        else {
          setSelected(getSelected() + 1);
          _NewlySelectedItem();
        }          
        return true;
      case SDLK_PAGEUP:
        for(int i=0;i<10;i++) {
          if(getSelected() <= 0)
            break;
          else {
            setSelected(getSelected() - 1);
            _NewlySelectedItem();
          }
        }
        return true;
      case SDLK_PAGEDOWN:
        for(int i=0;i<10;i++) {
          if(getSelected() >= m_Entries.size() - 1)
            break;
          else {
            setSelected(getSelected() + 1);
            _NewlySelectedItem();
          }          
        }
        return true;
      
      /* Left and right always selects another window */
      case SDLK_LEFT:
        getRoot()->activateLeft();
        return true;
      case SDLK_RIGHT:
        getRoot()->activateRight();
        return true;
    }
    
    /* w00t? we don't want that silly keypress! */
    return false;
  }
  
  /*===========================================================================
  Called when a entry is selected
  ===========================================================================*/
  void UIList::_NewlySelectedItem(void) {
    /* HACK HACK HACK HACK! */
  
    int nHeaderHeight=18;
    int nLX = 6, nLY = nHeaderHeight+6+4;
    int nLWidth = getPosition().nWidth-12 - 20;
    int nLHeight = getPosition().nHeight - nLY - 6;
    int nRowHeight = 16;    
    
    int nSelY1 = m_nScroll + nLY + m_nSelected*nRowHeight;
    int nSelY2 = m_nScroll + nLY + m_nSelected*nRowHeight + nRowHeight;

    /* Inside view? */
    if(nSelY1 < nHeaderHeight + 6 + 4) {
      /* UP! */
      _Scroll((nHeaderHeight + 10) - nSelY1);
    }    
    else if(nSelY2 > nHeaderHeight + 6 + 4 + nLHeight) {
      /* DOWN! */
      _Scroll(nHeaderHeight + 10 + nLHeight - nSelY2);
    }    
  }
  
  /*===========================================================================
  Misc helpers
  ===========================================================================*/
  void UIList::_Scroll(int nPixels) {
    m_nScroll += nPixels;
    if(m_nScroll > 0) m_nScroll = 0;
  }


};

