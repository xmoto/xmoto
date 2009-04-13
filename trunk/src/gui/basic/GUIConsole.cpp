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

#include "GUIConsole.h"
#include "../../drawlib/DrawLib.h"
#include "../../include/xm_SDL.h"
#include "../../helpers/utf8.h"

#define UIC_PROMPT "$ "

UIConsole::UIConsole(UIWindow *pParent, int x, int y, std::string Caption, int nWidth, int nHeight) {
  initW(pParent, x , y, Caption, nWidth, nHeight);

  m_cursorLine = 0;
  m_cursorChar = 0;

  addLine(UIC_PROMPT);
}      

void UIConsole::paint() {
  FontManager* v_fm;
  FontGlyph* v_fg;
  int v_XOffset = getPosition().nX;
  int v_YOffset = getPosition().nY;

  v_fm = getFont();

  putRect(0, 0, getPosition().nWidth, getPosition().nHeight, MAKE_COLOR(30, 0, 0, 255));

  // draw the text + cursor
  for(unsigned int i=0; i<m_lines.size(); i++) {
    v_fg = v_fm->getGlyph(m_lines[i]);
    v_fm->printString(v_fg, v_XOffset, v_YOffset,
		      MAKE_COLOR(255, 255, 255, 255));
    v_YOffset += v_fg->realHeight();
  }
  
}

bool UIConsole::offerActivation() {
  return false;
}

void UIConsole::addLine(const std::string& i_line) {
  m_lines.push_back(i_line);
}

bool UIConsole::keyDown(int nKey, SDLMod mod, const std::string& i_utf8Char) {
  if(nKey == SDLK_RETURN) {
    m_lines.push_back(UIC_PROMPT);
    m_cursorLine++;
    m_cursorChar = 0;
    return true;
  }

  if(nKey == SDLK_BACKSPACE) {
    m_lines[m_cursorLine] = utf8::utf8_delete(m_lines[m_cursorLine], m_cursorChar+1);
  }

  m_lines[m_cursorLine] += i_utf8Char;

  return true;
}
