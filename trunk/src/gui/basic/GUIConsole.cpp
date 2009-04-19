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
#include "../../Game.h"

#define UIC_PROMPT "$ "
#define UIC_CURSOR "_"

UIConsoleHook::UIConsoleHook() {
}

UIConsoleHook::~UIConsoleHook() {
}

UIConsole::UIConsole(UIWindow *pParent, int x, int y, std::string Caption, int nWidth, int nHeight) {
  initW(pParent, x , y, Caption, nWidth, nHeight);

  m_hook = NULL;
  m_cursorLine = 0;
  m_history_n = -1;
  m_waitAnswer = false;

  addNewLine(UIC_PROMPT);
  m_cursorChar = utf8::utf8_length(UIC_PROMPT);
}      

UIConsole::~UIConsole() {
}

void UIConsole::setHook(UIConsoleHook* i_hook) {
  m_hook = i_hook;
}

void UIConsole::paint() {
  FontManager* v_fm;
  FontGlyph* v_fg;
  int v_XOffset = getPosition().nX;
  int v_YOffset = getPosition().nY;
  int v_cursorXOffset = 0;
  int v_cursorYOffset = 0;

  v_fm = getFont();

  putRect(0, 0, getPosition().nWidth, getPosition().nHeight, MAKE_COLOR(0, 0, 0, 220));

  // draw the text
  for(unsigned int i=0; i<m_lines.size(); i++) {
    v_fg = v_fm->getGlyph(m_lines[i]);
    v_fm->printString(v_fg, v_XOffset, v_YOffset,
		      MAKE_COLOR(255, 255, 255, 255));

    if(m_cursorLine == (int) i) {

      if(m_cursorChar == (int) utf8::utf8_length(m_lines[m_cursorLine])) {
	v_cursorXOffset = v_XOffset + v_fg->realWidth();
      } else {
	std::string s = utf8::utf8_substring(m_lines[m_cursorLine], 0, m_cursorChar);
	v_cursorXOffset = v_XOffset + v_fm->getGlyph(s)->realWidth();
      }
      v_cursorYOffset = v_YOffset;
    }

    v_YOffset += v_fg->realHeight();
  }
 
  // draw cursor
  if(m_waitAnswer == false) {
    if((GameApp::getXMTimeInt()/100) % 10 < 5) {
      v_fg = v_fm->getGlyph(UIC_CURSOR);
      v_fm->printString(v_fg, v_cursorXOffset, v_cursorYOffset, MAKE_COLOR(255, 255, 255, 255));
    }
  }

  // remove lines if on bottom
  if(v_YOffset > getPosition().nY + getPosition().nHeight) {
    m_lines.erase(m_lines.begin());
    m_cursorLine--;
  }
}

bool UIConsole::offerActivation() {
  return false;
}

void UIConsole::giveAnswer(const std::string& i_line) {
  m_lines.push_back(i_line);
  m_cursorLine++;
  addNewLine(UIC_PROMPT);
  m_cursorChar = utf8::utf8_length(UIC_PROMPT);
  m_cursorLine++;
  m_waitAnswer = false;
}

void UIConsole::addNewLine(const std::string& i_line) {
  m_lines.push_back(i_line);
}

bool UIConsole::keyDown(int nKey, SDLMod mod, const std::string& i_utf8Char) {
  if(nKey == SDLK_d && (mod & KMOD_LCTRL) == KMOD_LCTRL) {
    execInternal("exit");
    return true;
  }

  if(nKey == SDLK_l && (mod & KMOD_LCTRL) == KMOD_LCTRL) {
    if(m_waitAnswer) {
      m_lines.erase(m_lines.begin(), m_lines.end());
      m_cursorLine = -1;
    } else {
      m_lines.erase(m_lines.begin(), m_lines.end()-1);
      m_cursorLine = 0;
    }
    return true;
  }


  // console is very limited is waiting for an answer
  if(m_waitAnswer) {
    return false;
  }

  if(nKey == SDLK_RETURN) {
    execLine(m_lines[m_cursorLine]);
    return true;
  }

  if(nKey == SDLK_BACKSPACE) {
    if(m_cursorChar > (int) utf8::utf8_length(UIC_PROMPT)) {
      m_lines[m_cursorLine] = utf8::utf8_delete(m_lines[m_cursorLine], m_cursorChar);
      m_cursorChar--;
    }
    return true;
  }

  if(nKey == SDLK_UP) {
    if(m_history_n < (int) m_history.size()-1) {
      m_history_n++;
      changeLine(m_history[m_history.size()-1 - m_history_n]);
    }
    return true;
  }

  if(nKey == SDLK_DOWN) {
    if(m_history_n >= 0) {
      m_history_n--;
      
      if(m_history_n < 0) {
	changeLine(actionFromLine(m_lastEdit));
      } else {
	changeLine(m_history[m_history.size()-1 - m_history_n]);
      }
    }
    return true;
  }

  if(nKey == SDLK_LEFT) {
    if(m_cursorChar > (int) utf8::utf8_length(UIC_PROMPT)) {
      m_cursorChar--;
    }
    return true;
  }

  if(nKey == SDLK_RIGHT) {
    if(m_cursorChar < (int) utf8::utf8_length(m_lines[m_cursorLine])) {
      m_cursorChar++;
    }
    return true;
  }

  if(nKey == SDLK_DELETE) {
    if(m_cursorChar < (int) utf8::utf8_length(m_lines[m_cursorLine])) {
      m_lines[m_cursorLine] = utf8::utf8_delete(m_lines[m_cursorLine], m_cursorChar+1);
    }
    return true;
  }

  // add the key
  if(utf8::utf8_length(i_utf8Char) == 1) { // alt/... and special keys must not be kept
    if(m_cursorChar == (int) utf8::utf8_length(m_lines[m_cursorLine])) {
      m_lines[m_cursorLine] += i_utf8Char;
    } else {
      std::string s;
      s = utf8::utf8_substring(m_lines[m_cursorLine], 0, m_cursorChar);
      s += i_utf8Char;
      s += utf8::utf8_substring(m_lines[m_cursorLine], m_cursorChar,
				utf8::utf8_length(m_lines[m_cursorLine]) - m_cursorChar);
      m_lines[m_cursorLine] = s;
    }
    m_cursorChar++;
    m_lastEdit = m_lines[m_cursorLine];
  }

  return true;
}

void UIConsole::changeLine(const std::string& i_action) {
  m_lines[m_cursorLine] = UIC_PROMPT + i_action; 
  m_cursorChar = utf8::utf8_length(UIC_PROMPT) + utf8::utf8_length(i_action);
}

std::string UIConsole::actionFromLine(const std::string& i_line) {
  unsigned int v_prompt_length = utf8::utf8_length(UIC_PROMPT);
  return utf8::utf8_substring(i_line, v_prompt_length,
			      utf8::utf8_length(i_line) - v_prompt_length);
}

void UIConsole::addHistory(const std::string& i_action) {
  bool v_samelast = false;

  if(i_action != "") {
    if(m_history.size() != 0) {
      if(i_action == m_history[m_history.size() -1]) {
	v_samelast = true;
      }
    }
    if(v_samelast == false) {
      m_history.push_back(i_action);
    }
  }
}

bool UIConsole::execInternal(const std::string& i_action) {
  if(i_action == "exit") {
    m_hook->exit();
    return true;
  }
  return false;
}

void UIConsole::execLine(const std::string& i_line) {
  std::string v_action = actionFromLine(i_line);

  // add in history
  addHistory(v_action);
  m_history_n = -1;

  m_lastEdit = UIC_PROMPT;

  // call
  if(v_action != "") {
    if(execInternal(v_action)) {
      m_cursorLine++;
      addNewLine(UIC_PROMPT);
      m_cursorChar = utf8::utf8_length(UIC_PROMPT);
    } else {
      m_hook->exec(v_action);
      m_waitAnswer = true;
    }
  } else { // simple new line without command
    m_cursorLine++;
    addNewLine(UIC_PROMPT);
    m_cursorChar = utf8::utf8_length(UIC_PROMPT);
  }

}
