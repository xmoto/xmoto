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

#ifndef __GUICONSOLE_H__
#define __GUICONSOLE_H__

#include "GUI.h"
#include "common/TextEdit.h"
#include <vector>

class UIConsoleHook {
public:
  UIConsoleHook();
  virtual ~UIConsoleHook();

  virtual void exec(const std::string &i_cmd) = 0;
  virtual void exit() = 0;
};

class UIConsole : public UIWindow {
public:
  UIConsole(UIWindow *pParent,
            std::vector<std::string> &completionList,
            int x = 0,
            int y = 0,
            std::string Caption = "",
            int nWidth = 0,
            int nHeight = 0);
  UIConsole(UIWindow *pParent,
            int x = 0,
            int y = 0,
            std::string Caption = "",
            int nWidth = 0,
            int nHeight = 0);

  ~UIConsole();

  void setHook(UIConsoleHook *i_hook);

  virtual void paint() override;
  virtual bool offerActivation() override;

  virtual bool keyDown(int nKey,
                       SDL_Keymod mod,
                       const std::string &i_utf8Char) override;
  virtual bool textInput(int nKey,
                         SDL_Keymod mod,
                         const std::string &i_utf8Char) override;
  virtual void mouseWheelUp(int x, int y) override;
  virtual void mouseWheelDown(int x, int y) override;

  void output(const std::string &i_line);
  void execCommand(const std::string &i_action);
  void addCompletionCommand(const std::string &i_cmd);
  void reset(const std::string &i_cmd = ""); /* command to run at startup */

private:
  void initConsole(UIWindow *pParent,
                   int x,
                   int y,
                   std::string Caption,
                   int nWidth,
                   int nHeight);

  inline int32_t numScreenRows() {
    return getPosition().nHeight / m_lineHeight;
  }

  void clear();
  void resetScroll(bool end);
  bool isScrollOutside();
  void scroll(int count);
  void appendScrollback(const std::string &line);

  UIConsoleHook *m_hook;

  int m_lineHeight;

  TextEdit m_textEdit;
  int32_t m_scroll;
  std::vector<std::string> m_scrollback;
  std::vector<std::string> m_history;
  std::vector<std::string> m_completionList;
  int m_history_n;
  std::string m_lastEdit;
  bool m_waitForResponse;

  void changeLine(const std::string &i_action);
  void addNewLine();
  void addHistory(const std::string &i_action);
  void execLine(const std::string &i_line);
  bool execInternal(const std::string &i_action);
  void completeCommand();
};

#endif
