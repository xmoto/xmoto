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
#include "drawlib/DrawLib.h"
#include "helpers/System.h"
#include "helpers/utf8.h"
#include "include/xm_SDL.h"
#include "xmoto/Game.h"
#include "common/TextEdit.h"

#include <algorithm>

#define UIC_PROMPT "$ "
#define UIC_CURSOR "_"

UIConsoleHook::UIConsoleHook() {}

UIConsoleHook::~UIConsoleHook() {}

void UIConsole::initConsole(UIWindow *pParent,
                            int x,
                            int y,
                            std::string Caption,
                            int nWidth,
                            int nHeight) {
  initW(pParent, x, y, Caption, nWidth, nHeight);
  m_hook = NULL;
  reset();
}

UIConsole::UIConsole(UIWindow *pParent,
                     int x,
                     int y,
                     std::string Caption,
                     int nWidth,
                     int nHeight) {
  initConsole(pParent, x, y, Caption, nWidth, nHeight);
}

UIConsole::UIConsole(UIWindow *pParent,
                     std::vector<std::string> &completionList,
                     int x,
                     int y,
                     std::string Caption,
                     int nWidth,
                     int nHeight) {
  initConsole(pParent, x, y, Caption, nWidth, nHeight);
  for (int i = 0, n = completionList.size(); i < n; i++) {
    this->addCompletionCommand(completionList[i]);
  }
}

UIConsole::~UIConsole() {}

void UIConsole::addCompletionCommand(const std::string &i_cmd) {
  m_completionList.push_back(i_cmd);
}

void UIConsole::setHook(UIConsoleHook *i_hook) {
  m_hook = i_hook;
}

void UIConsole::paint() {
  FontManager *v_fm;
  FontGlyph *v_fg;
  int v_XOffset = getPosition().nX;
  int v_YOffset = getPosition().nY;
  int v_cursorXOffset = 0;
  int v_cursorYOffset = 0;
  int v_nbToRemove;

  v_fm = GameApp::instance()->getDrawLib()->getFontMonospace();

  putRect(0,
          0,
          getPosition().nWidth,
          getPosition().nHeight,
          MAKE_COLOR(0, 0, 0, 220));

  auto drawLine = [&](const std::string &line, bool lastLine) {
    v_fg = v_fm->getGlyphTabExtended(line);

    // only print the line if there is space at the bottom
    if (v_YOffset + v_fg->realHeight() <
        getPosition().nY + getPosition().nHeight) {
      v_fm->printString(GameApp::instance()->getDrawLib(),
                        v_fg,
                        v_XOffset,
                        v_YOffset,
                        MAKE_COLOR(255, 255, 255, 255));
    }


    if (lastLine) {
      // compute the cursor for the last line
      if (m_textEdit.cursorPos() >= utf8::utf8_length(line)) {
        v_cursorXOffset = v_XOffset + v_fg->realWidth();
      } else {
        auto promptLength = utf8::utf8_length(UIC_PROMPT);

        std::string s = utf8::utf8_substring(line, 0, promptLength + m_textEdit.cursorPos());
        v_cursorXOffset = v_XOffset + v_fm->getGlyph(s)->realWidth();
      }
      v_cursorYOffset = v_YOffset;
    }

    // update the offset
    v_YOffset += v_fg->realHeight();
    if (v_YOffset > getPosition().nY + getPosition().nHeight) {
      v_nbToRemove++;
    }
  };


  for (auto &line : m_scrollback) {
    drawLine(line, false);
  }

  std::string line;
  // only draw the current line when not waiting for a response
  if (!m_waitForResponse)
    line = UIC_PROMPT + m_textEdit.text();

  drawLine(line, true);



  bool blink = (GameApp::getXMTimeInt() / 100) % 10 < 5;

  // draw the cursor when waiting for a response or during a blink
  if (blink || m_waitForResponse) {
    v_fg = v_fm->getGlyph(UIC_CURSOR);
    v_fm->printString(GameApp::instance()->getDrawLib(),
                      v_fg,
                      v_cursorXOffset,
                      v_cursorYOffset,
                      MAKE_COLOR(255, 255, 255, 255));
  }

  // remove lines if on bottom
  if (v_YOffset > getPosition().nY + getPosition().nHeight) {
    m_scrollback.erase(m_scrollback.begin(), m_scrollback.begin() + v_nbToRemove);
  }
}

bool UIConsole::offerActivation() {
  return false;
}

void UIConsole::reset(const std::string &i_cmd) {
  m_scrollback.clear();
  m_waitForResponse = false;
  m_lastEdit = "";
  m_history_n = -1;

  if (i_cmd == "") {
    addNewLine();
  } else {
    execCommand(i_cmd);
  }
}

void UIConsole::giveAnswer(const std::string &i_line) {
  std::vector<std::string> lines;

  utf8::utf8_split(i_line, "\n", lines);
  for (auto &line : lines) {
    m_scrollback.push_back(line);
  }
  addNewLine();
  m_waitForResponse = false;
}

void UIConsole::addNewLine() {
  m_textEdit.clear();
}

bool UIConsole::textInput(int nKey, SDL_Keymod mod, const std::string &i_utf8Char) {
  // alt/... and special keys must not be kept
  if (utf8::utf8_length(i_utf8Char) == 1) {
    m_textEdit.insert(i_utf8Char);
    m_lastEdit = m_textEdit.text();
  }

  return true;
}

bool UIConsole::keyDown(int nKey, SDL_Keymod mod, const std::string &i_utf8Char) {
  // EOF
  if (nKey == SDLK_d && (mod & KMOD_CTRL)
      && m_textEdit.text().empty()) {
    execInternal("exit");
    return true;
  }

  // clear
  if (nKey == SDLK_l && (mod & KMOD_CTRL)) {
    int linesToKeep = m_waitForResponse ? 1 : 0;
    m_scrollback.erase(m_scrollback.begin(), m_scrollback.end() - linesToKeep);

    return true;
  }

  if (m_waitForResponse)
    return false;


  if (nKey == SDLK_RETURN) {
    execLine(m_textEdit.text());
    return true;
  }

  if (nKey == SDLK_BACKSPACE) {
    if (mod & KMOD_CTRL) {
      m_textEdit.deleteWordLeft();
    } else {
      m_textEdit.deleteLeft();
    }
    return true;
  }

  if (nKey == SDLK_DELETE) {
    if (mod & KMOD_CTRL) {
      m_textEdit.deleteWordRight();
    } else {
      m_textEdit.deleteRight();
    }
    return true;
  }

  if (nKey == SDLK_UP) {
    if (m_history_n < (int)m_history.size() - 1) {
      m_history_n++;
      changeLine(m_history[m_history.size() - 1 - m_history_n]);
    }
    return true;
  }

  if (nKey == SDLK_DOWN) {
    if (m_history_n >= 0) {
      m_history_n--;

      if (m_history_n < 0) {
        changeLine(m_lastEdit);
      } else {
        changeLine(m_history[m_history.size() - 1 - m_history_n]);
      }
    }
    return true;
  }

  if (nKey == SDLK_LEFT) {
    if (mod & KMOD_CTRL) {
      m_textEdit.jumpWordLeft();
    } else {
      m_textEdit.moveCursor(-1);
    }
    return true;
  }

  if (nKey == SDLK_RIGHT) {
    if (mod & KMOD_CTRL) {
      m_textEdit.jumpWordRight();
    } else {
      m_textEdit.moveCursor(1);
    }
    return true;
  }

  if (nKey == SDLK_TAB) {
    completeCommand();
    return true;
  }

  if (nKey == SDLK_END) {
    m_textEdit.jumpToEnd();
  }

  if (nKey == SDLK_HOME) {
    m_textEdit.jumpToStart();
  }

  if (nKey == SDLK_v && (mod & KMOD_CTRL)) {
    m_textEdit.insert(System::getClipboardText());
  }

  return true;
}

void UIConsole::changeLine(const std::string &i_action) {
  m_textEdit.setText(i_action);
  m_textEdit.jumpToEnd();
}

void UIConsole::addHistory(const std::string &i_action) {
  if (i_action == "")
    return;

  m_scrollback.push_back(UIC_PROMPT + i_action);

  bool sameAsLast = m_history.size() > 0 && i_action == m_history.back();

  if (!sameAsLast)
    m_history.push_back(i_action);
}

bool UIConsole::execInternal(const std::string &i_action) {
  if (i_action == "exit") {
    m_hook->exit();
    return true;
  }

  return false;
}

void UIConsole::execCommand(const std::string &i_action) {
  if (i_action == "") { // empty command
    m_scrollback.push_back(UIC_PROMPT);
    addNewLine();
    return;
  }

  if (execInternal(i_action)) {
    addNewLine();
    return;
  }

  m_hook->exec(i_action);
  m_waitForResponse = true;
}

void UIConsole::execLine(const std::string &i_line) {
  std::string v_action = i_line;

  m_lastEdit = "";

  addHistory(v_action);
  m_history_n = -1;

  execCommand(v_action);
}

void UIConsole::completeCommand() {
  int lastSpace = m_textEdit.text().rfind(" ");
  std::string lastWord = m_textEdit.text().substr(lastSpace + 1);

  std::vector<std::string> foundList;
  for (auto &completion : m_completionList) {
    if (completion.find(lastWord) == 0)
      foundList.push_back(completion);
  }

  if (foundList.size() < 1)
    return;

  if (foundList.size() > 1) {
    std::string found_list_str;

    for (auto &found : foundList)
      found_list_str += found + "  ";

    m_scrollback.push_back(found_list_str);
  } else {
    m_textEdit.insert(foundList[0].substr(lastWord.size(), 1000));
  }
}
