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
#include "common/TextEdit.h"
#include "drawlib/DrawLib.h"
#include "helpers/utf8.h"
#include "xmoto/Game.h"

#include <algorithm>

/*===========================================================================
Painting
===========================================================================*/
void UIEdit::paint(void) {
  bool bDisabled = isDisabled();
  bool bActive = isActive();
  std::string v_textToDisplay;
  std::string v_textPart1;

  if (m_hideText) {
    v_textToDisplay = "";
    for (unsigned int i = 0; i < utf8::utf8_length(m_textEdit.text()); i++) {
      v_textToDisplay.append("*");
    }
  } else {
    v_textToDisplay = m_textEdit.text();
  }

  /* Where should cursor be located? */
  int nCursorOffset = 0;
  int nCursorWidth = 0;

  std::string s =
    utf8::utf8_substring(v_textToDisplay, 0, m_textEdit.cursorPos());

  /* cursor offset */
  if (s != "") {
    FontManager *fm = getFont();
    FontGlyph *fg = fm->getGlyph(s);
    nCursorOffset = fg->realWidth();
  }

  // For future use
  bool selection = false;

  if (!selection) {
    nCursorWidth = IBeamWidth;
  } else {
    if (m_textEdit.cursorPos() >= utf8::utf8_length(v_textToDisplay)) {
      nCursorWidth = BlockCursorWidth;
    } else {
      std::string s =
        utf8::utf8_substring(v_textToDisplay, m_textEdit.cursorPos(), 1);
      FontManager *fm = getFont();
      FontGlyph *fg = fm->getGlyph(s);
      nCursorWidth = fg->realWidth();
    }
  }

  /* Draw */
  if (bDisabled) {
    putText(4, getPosition().nHeight / 2, v_textToDisplay, 0.0, -0.5);
  }
  if (isUglyMode() == false) {
    putElem(0, 0, -1, -1, UI_ELEM_FRAME_TL, false);
    putElem(getPosition().nWidth - 8, 0, -1, -1, UI_ELEM_FRAME_TR, false);
    putElem(getPosition().nWidth - 8,
            getPosition().nHeight - 8,
            -1,
            -1,
            UI_ELEM_FRAME_BR,
            false);
    putElem(0, getPosition().nHeight - 8, -1, -1, UI_ELEM_FRAME_BL, false);
    putElem(8, 0, getPosition().nWidth - 16, -1, UI_ELEM_FRAME_TM, false);
    putElem(8,
            getPosition().nHeight - 8,
            getPosition().nWidth - 16,
            -1,
            UI_ELEM_FRAME_BM,
            false);
    putElem(0, 8, -1, getPosition().nHeight - 16, UI_ELEM_FRAME_ML, false);
    putElem(getPosition().nWidth - 8,
            8,
            -1,
            getPosition().nHeight - 16,
            UI_ELEM_FRAME_MR,
            false);
    putRect(8,
            8,
            getPosition().nWidth - 16,
            getPosition().nHeight - 16,
            MAKE_COLOR(0, 0, 0, 168));
  } else {
    putRect(0,
            0,
            getPosition().nWidth,
            getPosition().nHeight,
            MAKE_COLOR(0, 0, 0, 127));
  }

  // draw cursor
  if (!bDisabled) {
    if (bActive) {
      if (sin(getApp()->getXMTime() * 13.0f) > 0.0f)
        putRect(4 + nCursorOffset,
                3,
                nCursorWidth + 1,
                18,
                MAKE_COLOR(255, 0, 0, 255));
    }

    putText(4, getPosition().nHeight / 2, v_textToDisplay, 0.0, -0.5);
  }
}

/*===========================================================================
Keyboard event handling
===========================================================================*/
bool UIEdit::keyDown(int nKey, SDL_Keymod mod, const std::string &i_utf8Char) {
  switch (nKey) {
    case SDLK_UP:
      getRoot()->activateUp();
      return true;

    case SDLK_DOWN:
      getRoot()->activateDown();
      return true;

    case SDLK_TAB:
      if (mod & KMOD_SHIFT)
        getRoot()->activatePrevious();
      else
        getRoot()->activateNext();
      return true;

    case SDLK_v:
      if (mod & KMOD_CTRL) {
        m_textEdit.insertFromClipboard();
        updateCaption();
      }
      return true;

    case SDLK_LEFT:
      if (m_textEdit.cursorPos() > 0) {
        if (mod & KMOD_CTRL) {
          m_textEdit.jumpWordLeft();
        } else {
          m_textEdit.moveCursor(-1);
        }
      } else {
        getRoot()->activateLeft();
      }
      return true;

    case SDLK_RIGHT:
      if (m_textEdit.cursorPos() < utf8::utf8_length(m_textEdit.text())) {
        if (mod & KMOD_CTRL) {
          m_textEdit.jumpWordRight();
        } else {
          m_textEdit.moveCursor(1);
        }
      } else {
        getRoot()->activateRight();
      }
      return true;

    case SDLK_RETURN:
      return true;

    case SDLK_END:
      m_textEdit.jumpToEnd();
      return true;

    case SDLK_HOME:
      m_textEdit.jumpToStart();
      return true;

    case SDLK_DELETE: {
      if (mod & KMOD_CTRL) {
        m_textEdit.deleteWordRight();
      } else {
        m_textEdit.deleteRight();
      }

      updateCaption();
      return true;
    }

    case SDLK_BACKSPACE: {
      if (mod & KMOD_CTRL) {
        m_textEdit.deleteWordLeft();
      } else {
        m_textEdit.deleteLeft();
      }

      updateCaption();
      return true;
    }
  }

  return false;
}

bool UIEdit::textInput(int nKey,
                       SDL_Keymod mod,
                       const std::string &i_utf8Char) {
  // alt/... and special keys must not be kept
  if (utf8::utf8_length(i_utf8Char) != 1) {
    return false;
  }

  m_textEdit.insert(i_utf8Char);
  updateCaption();
  return true;
}

bool UIEdit::joystickAxisMotion(JoyAxisEvent event) {
  return false;
}

bool UIEdit::joystickButtonDown(Uint8 i_joyNum, Uint8 i_joyButton) {
  // std::string  v_caption = getCaption();
  // unsigned int v_length  = utf8::utf8_length(v_caption);
  // std::string  v_begin;

  // if(v_length > 1) {
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

void UIEdit::updateCaption() {
  auto caption = m_textEdit.text();

  if (caption != getCaption()) {
    m_hasChanged = true;
  }

  UIWindow::setCaption(caption);
}
