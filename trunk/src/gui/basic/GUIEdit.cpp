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
#include "../../drawlib/DrawLib.h"
#include "../../Game.h"
#include "../../helpers/utf8.h"

  /*===========================================================================
  Painting
  ===========================================================================*/
void UIEdit::paint(void) {
  bool bDisabled = isDisabled();
  bool bActive = isActive();
  std::string v_textToDisplay;
  std::string v_textPart1;

  if(m_hideText) {
    v_textToDisplay = "";
    for(unsigned int i=0; i<utf8::utf8_length(getCaption()); i++) {
      v_textToDisplay.append("*");
    }
  } else {
    v_textToDisplay = getCaption();
  }

  /* Where should cursor be located? */
  int nCursorOffset = 0;
  int nCursorWidth = 0;

  std::string s = utf8::utf8_substring(v_textToDisplay, 0, m_nCursorPos);

  /* cursor offset */
  if(s != "") {
    FontManager* fm = getFont();
    FontGlyph* fg   = fm->getGlyph(s);
    nCursorOffset   = fg->realWidth();
  }

  /* cursor */
  if(m_nCursorPos == utf8::utf8_length(v_textToDisplay)) {
    nCursorWidth = 6;
  } else {
    std::string s = utf8::utf8_substring(v_textToDisplay, m_nCursorPos, 1);
    FontManager* fm = getFont();
    FontGlyph* fg = fm->getGlyph(s);
    nCursorWidth = fg->realWidth();
  }

  /* Draw */
  if(bDisabled){
    putText(4, getPosition().nHeight/2, v_textToDisplay, 0.0, -0.5);
  }

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
      if(sin(getApp()->getXMTime()*13.0f) > 0.0f)
	putRect(4+nCursorOffset,3,nCursorWidth+1,18,MAKE_COLOR(255,0,0,255));
    }

    putText(4, getPosition().nHeight/2, v_textToDisplay, 0.0, -0.5);
  }
}

  /*===========================================================================
  Keyboard event handling
  ===========================================================================*/
  bool UIEdit::keyDown(int nKey, SDLMod mod, const std::string& i_utf8Char) {
    switch(nKey) {      
      case SDLK_UP:
        getRoot()->activateUp();
        return true;
      case SDLK_DOWN:
        getRoot()->activateDown();
        return true;
      case SDLK_TAB:
        getRoot()->activateNext();
        return true;
      case SDLK_LEFT:
        if(m_nCursorPos > 0)
          m_nCursorPos--;
        else
          getRoot()->activateLeft();
        return true;
      case SDLK_RIGHT:
	if(m_nCursorPos < utf8::utf8_length(getCaption())) {
	  m_nCursorPos++;
	}
        else
          getRoot()->activateRight();
        return true;
      case SDLK_RETURN:
        return true;
      case SDLK_END:
        m_nCursorPos = utf8::utf8_length(getCaption());
        return true;
      case SDLK_HOME:
        m_nCursorPos = 0;
        return true;
      case SDLK_DELETE: {
	std::string s = getCaption();

	if(m_nCursorPos < utf8::utf8_length(s)) {
	  setCaption(utf8::utf8_delete(s, m_nCursorPos+1));
	}
      }
        return true;
      case SDLK_BACKSPACE: {
	std::string s = getCaption();

	if(m_nCursorPos > 0) {
	  setCaption(utf8::utf8_delete(s, m_nCursorPos--)); // m_nCursorPos-- must be done before setCaption (setCaption alter it if a bad value is given)
	}

      }
        return true;
    default:
      if(utf8::utf8_length(i_utf8Char) == 1) { // alt/... and special keys must not be kept
	std::string s = getCaption();
 
	if((unsigned int)m_nCursorPos == s.length()) {
	  s = utf8::utf8_concat(s, i_utf8Char);
	} else {
	  s = utf8::utf8_insert(s, i_utf8Char, m_nCursorPos);
	}
	m_nCursorPos++;
	setCaption(s);
	return true;
      }
      break;
    }
    
    return false;
  }  

bool UIEdit::joystickAxisMotion(Uint8 i_joyNum, Uint8 i_joyAxis, Sint16 i_joyAxisValue) {
  return false;
}

bool UIEdit::joystickButtonDown(Uint8 i_joyNum, Uint8 i_joyButton) {
  //std::string  v_caption = getCaption();
  //unsigned int v_length  = utf8::utf8_length(v_caption);
  //std::string  v_begin;

  //if(v_length > 1) {
  //  v_begin = utf8_substring(getCaption(), v_length -1);
  //}

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

		if(m_nCursorPos > utf8::utf8_length(Caption)) {
		  m_nCursorPos = utf8::utf8_length(Caption);
		}
	}
