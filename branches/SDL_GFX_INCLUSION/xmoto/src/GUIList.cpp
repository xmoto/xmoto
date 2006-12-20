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
 *  GUI: list control
 */
#include "VXml.h"
#include "GUI.h"
#include "Sound.h"

namespace vapp {

  void UIList::_refreshByTime() {
    float v_time = getApp()->getRealTime();

    while(m_lastRefreshTime + 0.01 < v_time) {
      
      if(m_bScrollDownPressed) {
	      _Scroll(-4);
      }
      
      if(m_bScrollUpPressed) {
	      _Scroll(4);
      }
      
      m_lastRefreshTime = v_time;
    }
  }

  void UIList::_mouseDownManageScrollBar(int x, int y){
    int nHeaderHeight = 18;
    int nLX = 6, nLY = nHeaderHeight+6+4;
    int nLWidth = getPosition().nWidth-12 - 20;
    int nLHeight = getPosition().nHeight - nLY - 6;
    int nRowHeight = 16;

    int p = ((y - nLY) / (((float)nLHeight) - 20.0)) * ((float)m_Entries.size());
    if(p < 0) p = 0;
    if(p >= m_Entries.size()) p = m_Entries.size()-1;
    setSelected(p);
  }


  /*===========================================================================
  Context help at cursor position?
  ===========================================================================*/
  std::string UIList::subContextHelp(int x,int y) {
    int nHX = 6;
  
    for(int i=0;i<m_Columns.size();i++) {
      if(!(m_nColumnHideFlags & (1<<i))) {
        int nW = m_ColumnWidths[i];
      
        for(int j=i+1;j<m_Columns.size();j++) {
          if(m_nColumnHideFlags & (1<<j))
            nW += m_ColumnWidths[i]; 
        }           
        
        /* Mouse in this one? */
        if(x >= nHX && x <= nHX + nW)
          return m_ColumnHelpStrings[i];
        
        nHX += nW;
      }
    }
    
    return "";
  }

  /*===========================================================================
  Painting
  ===========================================================================*/
  void UIList::paint(void) {
    _refreshByTime();

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
      if(!(m_nColumnHideFlags & (1<<i))) {
        putText(nHX,nHY + (nHeaderHeight*2)/3,m_Columns[i]);
        nHX += m_ColumnWidths[i]; 
        
        /* Next columns disabled? If so, make more room to this one */
        for(int j=i+1;j<m_Columns.size();j++) {
          if(m_nColumnHideFlags & (1<<j))
            nHX += m_ColumnWidths[i]; 
        }           
      }
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

    /* scroll */
    int v_scroll_height;
    int v_scroll_pos;

    float v_visible = ((float)nLHeight-20.0) / ((float)nRowHeight) +1;
    if(v_visible >= ((float)m_Entries.size())) {
      v_scroll_height = nLHeight-20.0;
      v_scroll_pos    = nLY;
    } else {
      v_scroll_height = (int) (v_visible / ((float)m_Entries.size()) * ((float) nLHeight-20.0));
      v_scroll_pos = (int) ( ((float)nLY) + (((float)-m_nScroll)/((float)nRowHeight)) / ((float)m_Entries.size()) * ((float) nLHeight-20.0));
    }
    putRect(nLX+nLWidth+2, v_scroll_pos, 16, v_scroll_height-1, MAKE_COLOR(188, 186, 67, 255));

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
        
	if(isUglyMode()) {
	  if(bDisabled) {
	    putRect(nLX,m_nScroll + nLY+i*nRowHeight,nLWidth,nRowHeight,MAKE_COLOR(128,128,128,255));
	  } else {
	    if(bActive) {
	      putRect(nLX,m_nScroll + nLY+i*nRowHeight,nLWidth,nRowHeight,MAKE_COLOR(200,60,60,255));
	    } else {
	      putRect(nLX,m_nScroll + nLY+i*nRowHeight,nLWidth,nRowHeight,MAKE_COLOR(160,40,40,255));
	    }
	  }
	} else {
	  if(bActive && !bDisabled) {
	    float s = 50 + 50*sin(getApp()->getRealTime()*10);
	    int n = (int)s;
	    if(n<0) n=0;
	    if(n>255) n=255; /* just to be sure, i'm lazy */    
	    
	    putRect(nLX,m_nScroll+ nLY+i*nRowHeight,nLWidth,nRowHeight,MAKE_COLOR(255,255,255,n));                 
	  }
	}
      }              

      int yy = m_nScroll+nLY+i*nRowHeight;
      if(yy >= getPosition().nHeight - 6) break;
      int yy2 = m_nScroll+nLY+i*nRowHeight + nRowHeight;
      int nLRH = nRowHeight;
      if(yy2 >= getPosition().nHeight - 6) nLRH = nRowHeight - (yy2 - (getPosition().nHeight - 6));
      int yym1 = m_nScroll+nLY+i*nRowHeight;
      if(yym1 + nRowHeight > nLY) {
        if(yym1 < nLY) {
          yym1 += (nLY-yym1);
        }
      
        int nOldScissor[4];
        //glGetIntegerv(GL_SCISSOR_BOX,(GLint *) nOldScissor);
        getApp()->getDrawLib()->getClipRect(&nOldScissor[0],&nOldScissor[1],&nOldScissor[2],&nOldScissor[3]);
	std::string txt_to_display;

        int x = 0;      
        for(int j=0;j<m_Entries[i]->Text.size();j++) {    
          if(!(m_nColumnHideFlags & (1<<j))) {
            /* Next columns disabled? If so, make more room to this one */
            int nExtraRoom = 0;
            for(int k=j+1;k<m_Columns.size();k++) {
              if(m_nColumnHideFlags & (1<<k))
                nExtraRoom += m_ColumnWidths[i]; 
            }           
          
            /* Draw */          
            setScissor(nLX+x,yym1,m_ColumnWidths[j]-4+nExtraRoom,nLRH);
	    txt_to_display = m_Entries[i]->Text[j];
	    if(j==0 && m_bNumeroted) {
	      std::ostringstream v_num;
	      v_num << i+1;

	      txt_to_display = "#" + v_num.str() + " " + txt_to_display; 
	    }
            putText(nLX+x,nLY+y,txt_to_display);
            x += m_ColumnWidths[j] + nExtraRoom;                
          }
        }
        
        getApp()->getDrawLib()->setClipRect(nOldScissor[0],nOldScissor[1],nOldScissor[2],nOldScissor[3]);
      }
    }
    
    /* Stuff */
    m_bScrollDownHover = false;
    m_bScrollUpHover = false;
    m_bClicked = false;
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

    /* is it inside the scroll bar ? */
    if(x >= nLX+nLWidth+2 && x <= nLX+nLWidth+2+16 &&
       y >= 6+20 && y<= nLY + nLHeight - 20) {
      _mouseDownManageScrollBar(x, y);
    }
    /* Is it down inside one of the scroll buttons? */
    else if(x >= nLX+nLWidth && x < nLX+nLWidth + 20 &&
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
	  setSelected(i);
          
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

    /* is it inside the scroll bar ? */
    if(x >= nLX+nLWidth+2 && x <= nLX+nLWidth+2+16 &&
       y >= 6+20 && y<= nLY + nLHeight - 20) {
      _mouseDownManageScrollBar(x, y);
      m_bScrolling = true;
    }
    /* Is it down inside one of the scroll buttons? */
    else if(x >= nLX+nLWidth && x < nLX+nLWidth + 20 &&
       y >= 6 && y < 6+20) {
      /* Scroll up! */
      //_Scroll(16);
      m_bScrollUpPressed = true;
    } 
    else if(x >= nLX+nLWidth && x < nLX+nLWidth + 20 &&
       y >= nLY+nLHeight-20 && y < nLY+nLHeight) {
      /* Scroll down! */
	 // _Scroll(-16);
      m_bScrollDownPressed = true;
    }    
    else {
      /* Find out what item is affected */
      for(int i=0;i<m_Entries.size();i++) {
        int yy = m_nScroll + nLY + i*nRowHeight;
        if(x >= nLX && x <getPosition().nWidth-6 &&
          y >= yy && y < yy+nRowHeight) {
          /* Select this */
	  setSelected(i);
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
      //_Scroll(16);
    } 
    else if(x >= nLX+nLWidth && x < nLX+nLWidth + 20 &&
       y >= nLY+nLHeight-20 && y < nLY+nLHeight) {
      /* Scroll down! */
      //_Scroll(-16);
    }

    m_bClicked = true;
    m_bScrolling = false; 
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

    /* is it inside the scroll bar ? */
    if(m_bScrolling) {
      _mouseDownManageScrollBar(x, y);
    }
    /* Is it down inside one of the scroll buttons? */
    else if(x >= nLX+nLWidth && x < nLX+nLWidth+20 &&
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
    
    /* Make a temp. lowercase text */
    std::string LCText = Text;
    for(int i=0;i<LCText.length();i++)
      LCText[i] = tolower(LCText[i]);
    
    /* Sorted? */
    if(m_bSort) {
      /* Yeah, keep it alphabetical, please */
      for(int i=0;i<m_Entries.size();i++) {

	if(pvUser != NULL && m_Entries[i]->pvUser != NULL && m_fsort != NULL) {
	  if(m_fsort(pvUser, m_Entries[i]->pvUser) < 0) {
	    /* Here! */
	    m_Entries.insert(m_Entries.begin() + i,p);
	    return p;
	  }
	} else {
	  /* Make lowercase before comparing */
	  std::string LC = m_Entries[i]->Text[0];
	  for(int j=0;j<LC.length();j++)  
	    LC[j] = tolower(LC[j]);
	  
	  if(LCText < LC) {
	    /* Here! */
	    m_Entries.insert(m_Entries.begin() + i,p);
	    return p;
	  }
	}
      }
    }

    /* Just add it to the end */
    m_Entries.push_back(p);
    return p;
  }
  
  void UIList::clear(void) {
    _FreeUIList();
    m_Entries.clear();
    m_nSelected = 0;
    m_nScroll   = 0;
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
        }
        return true;
      case SDLK_DOWN:
        if(getSelected() >= m_Entries.size() - 1)
          getRoot()->activateDown();
        else {
          setSelected(getSelected() + 1);
        }          
        return true;
      case SDLK_PAGEUP:
        for(int i=0;i<10;i++) {
          if(getSelected() <= 0)
            break;
          else {
            setSelected(getSelected() - 1);
          }
        }
        return true;
      case SDLK_PAGEDOWN:
        for(int i=0;i<10;i++) {
          if(getSelected() >= m_Entries.size() - 1)
            break;
          else {
            setSelected(getSelected() + 1);
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
    
    /* Maybe it's a character? If so, try jumping to a suitable place in the list */
    if(nChar != 0) {
      bool bContinue;
      int nPos = 0;
      
      do {
        bContinue = false;
        
        /* Look at character number 'nPos' in all entries */
        for(int i=0;i<m_Entries.size();i++) {
          int nEntryIdx = i + getSelected() + 1; /* always start looking at the next */
          nEntryIdx %= m_Entries.size();
        
          if(m_Entries[nEntryIdx]->Text[0].length() > nPos) {
            if(tolower(m_Entries[nEntryIdx]->Text[0].at(nPos)) == tolower(nChar)) {
              /* Nice, select this one */
              setSelected(nEntryIdx);
              bContinue = false;
              return true;
            }
            
            bContinue = true;
          }
        }
        
        nPos++;
      } while(bContinue);
    }
    
    /* w00t? we don't want that silly keypress! */
    return false;
  }
  
  void UIList::setSelected(int n) {
    m_bChanged = true;
    m_nSelected = n;
    _NewlySelectedItem();
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
    int nRowHeight = 16;
    int nHeaderHeight = 18 + 6 + 4;

    if(m_nScroll+nPixels > 0) {
      m_nScroll = 0;
      return;
    }

    if(m_nScroll+nPixels < -nRowHeight * ((int)m_Entries.size()+1) + getPosition().nHeight - nHeaderHeight) { /* keep the cast ; under my linux box, it doesn't work without it */
      return;
    } 

    m_nScroll += nPixels;
  }


}

