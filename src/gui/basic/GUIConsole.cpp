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
#include "common/TextEdit.h"
#include "drawlib/DrawLib.h"
#include "helpers/VMath.h"
#include "helpers/utf8.h"
#include "include/xm_SDL.h"
#include "xmoto/Game.h"

#include <algorithm> // std::max

const char *const PROMPT_CHAR = "$ ";
const char *const CURSOR_CHAR = "_";

const int32_t SCROLLBACK_LIMIT = 1000;

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
  m_scroll = 0;

  auto fm = GameApp::instance()->getDrawLib()->getFontMonospace();
  m_lineHeight = fm->getGlyph(" ")->realHeight();

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

  for (auto &completion : completionList)
    this->addCompletionCommand(completion);
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
  const uint32_t promptLength = utf8::utf8_length(PROMPT_CHAR);

  v_fm = GameApp::instance()->getDrawLib()->getFontMonospace();

  putRect(0,
          0,
          getPosition().nWidth,
          getPosition().nHeight,
          MAKE_COLOR(0, 0, 0, 220));

  auto drawTextLine = [&](const std::string &line, bool isPrompt) {
    v_fg = v_fm->getGlyphTabExtended(line);

    v_fm->printString(GameApp::instance()->getDrawLib(),
                      v_fg,
                      v_XOffset,
                      v_YOffset,
                      MAKE_COLOR(255, 255, 255, 255));

    if (isPrompt) {
      auto glyph = v_fg;

      if (m_textEdit.cursorPos() < utf8::utf8_length(line)) {
        auto s =
          utf8::utf8_substring(line, 0, promptLength + m_textEdit.cursorPos());
        glyph = v_fm->getGlyph(s);
      }

      v_cursorXOffset = v_XOffset + glyph->realWidth();
      v_cursorYOffset = v_YOffset;
    }

    v_YOffset += m_lineHeight;
  };

  uint32_t start = (uint32_t)std::max<int32_t>(m_scroll, 0);
  for (uint32_t i = start; i < m_scrollback.size(); i++) {
    drawTextLine(m_scrollback[i], false);
  }

  std::string line;
  // only draw the current line when not waiting for a response
  if (!m_waitForResponse)
    line = PROMPT_CHAR + m_textEdit.text();

  drawTextLine(line, true);

  bool blink = (GameApp::getXMTimeInt() / 100) % 10 < 5;

  // draw the cursor when waiting for a response or during a blink
  if (blink || m_waitForResponse) {
    v_fg = v_fm->getGlyph(CURSOR_CHAR);
    v_fm->printString(GameApp::instance()->getDrawLib(),
                      v_fg,
                      v_cursorXOffset,
                      v_cursorYOffset,
                      MAKE_COLOR(255, 255, 255, 255));
  }
}

bool UIConsole::offerActivation() {
  return false;
}

void UIConsole::reset(const std::string &i_cmd) {
  m_scroll = 0;
  m_waitForResponse = false;
  m_lastEdit = "";
  m_history_n = -1;

  if (i_cmd == "") {
    addNewLine();
  } else {
    execCommand(i_cmd);
  }
}

void UIConsole::clear() {
  int linesToKeep = m_waitForResponse ? 1 : 0;
  resetScroll(false);
  m_scroll -= linesToKeep;
}

void UIConsole::resetScroll(bool end) {
  int offset = end ? (-numScreenRows() + 1) : 0;
  m_scroll = m_scrollback.size() + offset;
}

void UIConsole::addNewLine() {
  m_textEdit.clear();
}

bool UIConsole::isScrollOutside() {
  return m_scroll + numScreenRows() <= (int32_t)m_scrollback.size();
}

void UIConsole::appendScrollback(const std::string &line) {
  m_scrollback.push_back(line);

  if (m_scrollback.size() > SCROLLBACK_LIMIT)
    m_scrollback.erase(m_scrollback.begin(), m_scrollback.begin() + 1);
}

void UIConsole::output(const std::string &i_line) {
  std::vector<std::string> lines;

  utf8::utf8_split(i_line, "\n", lines);

  for (auto &line : lines)
    appendScrollback(line);

  if (isScrollOutside())
    resetScroll(true);

  addNewLine();
  m_waitForResponse = false;
}

bool UIConsole::textInput(int nKey,
                          SDL_Keymod mod,
                          const std::string &i_utf8Char) {
  // alt/... and special keys must not be kept
  if (utf8::utf8_length(i_utf8Char) != 1)
    return true;

  if (isScrollOutside())
    resetScroll(true);

  m_textEdit.insert(i_utf8Char);
  m_lastEdit = m_textEdit.text();

  return true;
}

bool UIConsole::keyDown(int nKey,
                        SDL_Keymod mod,
                        const std::string &i_utf8Char) {
  // EOF
  if (nKey == SDLK_d && (mod & KMOD_CTRL) && m_textEdit.text().empty()) {
    execInternal("exit");
    return true;
  }

  if (nKey == SDLK_l && (mod & KMOD_CTRL)) {
    clear();
    return true;
  }

  if (m_waitForResponse)
    return false;

  bool needScrollReset = true;

  switch (nKey) {
    case SDLK_RETURN: {
      execLine(m_textEdit.text());
      break;
    }

    case SDLK_BACKSPACE: {
      if (mod & KMOD_CTRL)
        m_textEdit.deleteWordLeft();
      else
        m_textEdit.deleteLeft();

      break;
    }

    case SDLK_DELETE: {
      if (mod & KMOD_CTRL)
        m_textEdit.deleteWordRight();
      else
        m_textEdit.deleteRight();

      break;
    }

    case SDLK_PAGEUP: {
      needScrollReset = false;
      scroll(-15);
      break;
    }

    case SDLK_PAGEDOWN: {
      needScrollReset = false;
      scroll(15);
      break;
    }

    case SDLK_UP: {
      if (m_history_n < (int)m_history.size() - 1) {
        m_history_n++;
        changeLine(m_history[m_history.size() - 1 - m_history_n]);
      }

      break;
    }

    case SDLK_DOWN: {
      if (m_history_n >= 0) {
        m_history_n--;

        if (m_history_n < 0) {
          changeLine(m_lastEdit);
        } else {
          changeLine(m_history[m_history.size() - 1 - m_history_n]);
        }
      }

      break;
    }

    case SDLK_LEFT: {
      if (mod & KMOD_CTRL)
        m_textEdit.jumpWordLeft();
      else
        m_textEdit.moveCursor(-1);

      break;
    }

    case SDLK_RIGHT: {
      if (mod & KMOD_CTRL)
        m_textEdit.jumpWordRight();
      else
        m_textEdit.moveCursor(1);

      break;
    }

    case SDLK_TAB: {
      completeCommand();
      break;
    }

    case SDLK_END: {
      m_textEdit.jumpToEnd();
      break;
    }

    case SDLK_HOME: {
      m_textEdit.jumpToStart();
      break;
    }

    case SDLK_v: {
      if (mod & KMOD_CTRL)
        m_textEdit.insertFromClipboard();

      break;
    }

    case SDLK_c: {
      if (mod & KMOD_CTRL) {
        appendScrollback(PROMPT_CHAR + m_textEdit.text() + "^C");
        addNewLine();

        if (isScrollOutside())
          ++m_scroll;
      }

      break;
    }

    case SDLK_w: {
      if (mod & KMOD_CTRL)
        m_textEdit.deleteWordLeft();

      break;
    }
  }

  if (needScrollReset && isScrollOutside())
    resetScroll(true);

  return true;
}

void UIConsole::scroll(int count) {
  m_scroll = clamp<int32_t>(m_scroll + count, 0, (int32_t)m_scrollback.size());
}

void UIConsole::mouseWheelUp(int x, int y) {
  scroll(-1);
}

void UIConsole::mouseWheelDown(int x, int y) {
  scroll(1);
}

void UIConsole::changeLine(const std::string &i_action) {
  m_textEdit.setText(i_action);
  m_textEdit.jumpToEnd();
}

void UIConsole::addHistory(const std::string &i_action) {
  if (i_action == "")
    return;

  bool sameAsLast = m_history.size() > 0 && i_action == m_history.back();

  if (!sameAsLast)
    m_history.push_back(i_action);
}

bool UIConsole::execInternal(const std::string &i_action) {
  if (i_action == "exit") {
    m_hook->exit();
    return true;
  }

  if (i_action == "clear") {
    clear();
    return true;
  }

  return false;
}

void UIConsole::execCommand(const std::string &i_action) {
  if (i_action == "") { // empty command
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

  appendScrollback(PROMPT_CHAR + v_action);

  if (isScrollOutside())
    ++m_scroll;

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
    std::string foundListStr;

    for (auto &found : foundList)
      foundListStr += found + "  ";

    appendScrollback(foundListStr);
  } else {
    m_textEdit.insert(foundList[0].substr(lastWord.size(), 1000));
  }
}
