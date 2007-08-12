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
 *  GUI: text edit box
 */
#include "GUI.h"
#include "../../VDraw.h"
#include "../../Game.h"

  /*===========================================================================
  Painting
  ===========================================================================*/
  void UIEdit::paint(void) {
    bool bDisabled = isDisabled();
    bool bActive = isActive();
    std::string v_textToDisplay;    

    if(m_hideText) {
      v_textToDisplay = "";
      for(int i=0; i<getCaption().length(); i++) {
	v_textToDisplay.append("*");
      }
    } else {
      v_textToDisplay = getCaption();
    }

    /* Where should cursor be located? */
    int nCursorOffset = 0;
    int nCursorWidth = 0;
    if(m_nCursorPos<0) m_nCursorPos=0;
    if(m_nCursorPos>v_textToDisplay.length()) m_nCursorPos=v_textToDisplay.length();
    std::string s = v_textToDisplay.substr(0,m_nCursorPos);

    int nMinX,nMinY,nMaxX,nMaxY;
    if(!s.empty()) {
      FontManager* fm = getFont();
      FontGlyph* fg = fm->getGlyph(s);
      nCursorOffset = fg->realWidth();
    }
    
    if(m_nCursorPos == v_textToDisplay.length()) {
      nCursorWidth = 6;
    } else {
      s = v_textToDisplay.substr(m_nCursorPos,1);
      FontManager* fm = getFont();
      FontGlyph* fg = fm->getGlyph(s);
      nCursorWidth = fg->realWidth();
    }
    
    /* Draw */
    if(bDisabled)
      putText(4, getPosition().nHeight/2, v_textToDisplay, 0.0, -0.5);
          
    putElem(0,0,-1,-1,UI_ELEM_FRAME_TL,false);
    putElem(getPosition().nWidth-8,0,-1,-1,UI_ELEM_FRAME_TR,false);
    putElem(getPosition().nWidth-8,getPosition().nHeight-8,-1,-1,UI_ELEM_FRAME_BR,false);
    putElem(0,getPosition().nHeight-8,-1,-1,UI_ELEM_FRAME_BL,false);
    putElem(8,0,getPosition().nWidth-16,-1,UI_ELEM_FRAME_TM,false);
    putElem(8,getPosition().nHeight-8,getPosition().nWidth-16,-1,UI_ELEM_FRAME_BM,false);
    putElem(0,8,-1,getPosition().nHeight-16,UI_ELEM_FRAME_ML,false);
    putElem(getPosition().nWidth-8,8,-1,getPosition().nHeight-16,UI_ELEM_FRAME_MR,false);
    putRect(8,8,getPosition().nWidth-16,getPosition().nHeight-16,MAKE_COLOR(0,0,0,127));
            
    putRect(8,8,getPosition().nWidth-16,getPosition().nHeight-16,MAKE_COLOR(0,0,0,127));

    if(!bDisabled) {
      if(bActive) {
        if(sin(getApp()->getRealTime()*13.0f) > 0.0f)
          putRect(4+nCursorOffset,3,nCursorWidth+1,18,MAKE_COLOR(255,0,0,255));
      }      
    
      putText(4, getPosition().nHeight/2, v_textToDisplay, 0.0, -0.5);
    }
  }

  /*===========================================================================
  Keyboard event handling
  ===========================================================================*/
  bool UIEdit::keyDown(int nKey, SDLMod mod,int nChar) {
    switch(nKey) {      
      case SDLK_UP:
        getRoot()->activateUp();
        return true;
      case SDLK_DOWN:
        getRoot()->activateDown();
        return true;
      case SDLK_LEFT:
        if(m_nCursorPos > 0)
          m_nCursorPos--;
        else
          getRoot()->activateLeft();
        return true;
      case SDLK_RIGHT:
        if(m_nCursorPos < getCaption().length())
          m_nCursorPos++;
        else
          getRoot()->activateRight();
        return true;
      case SDLK_RETURN:
        return true;
      case SDLK_END:
        m_nCursorPos = getCaption().length();
        return true;
      case SDLK_HOME:
        m_nCursorPos = 0;
        return true;
      case SDLK_DELETE: {
          std::string s = getCaption();
          if(!s.empty() && m_nCursorPos<s.length()) {
            s.erase(s.begin()+m_nCursorPos);
            setCaption(s);
          }
        }
        return true;
      case SDLK_BACKSPACE: {
          std::string s = getCaption();
          if(!s.empty() && m_nCursorPos>0) {
            s.erase(s.begin()+m_nCursorPos-1);
            setCaption(s);
            m_nCursorPos--;
          }
        }
        return true;      
      default:
        if(nChar) {
          char c[2];
          c[0] = nChar;
          c[1] = '\0';
          std::string s = getCaption();
          
          if(m_nCursorPos == s.length()) {
            s.append(c);
          }
          else {
            s.insert(s.begin() + m_nCursorPos,c[0]);
          }
          m_nCursorPos++;
          setCaption(s);
          return true;
        }
        break;
    }
    
    return false;
  }  

	void UIEdit::setHasChanged(bool b_value) {
		m_hasChanged = b_value;
	}

	bool UIEdit::hasChanged() {
		return m_hasChanged;
	}

	void UIEdit::setCaption(std::string Caption) {
		if(Caption != getCaption()) {
			 m_hasChanged = true;
		}
		UIWindow::setCaption(Caption);
	}
