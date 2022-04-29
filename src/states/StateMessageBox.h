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

#ifndef __STATEMESSAGEBOX_H__
#define __STATEMESSAGEBOX_H__

#include "../gui/basic/GUI.h"
#include "StateManager.h"
#include "StateMenu.h"

class UIRoot;
class UIMsgBox;
class StateMessageBoxReceiver;

class StateMessageBox : public StateMenu {
public:
  StateMessageBox(StateMessageBoxReceiver *i_receiver,
                  const std::string &i_text,
                  int i_buttons,
                  bool i_input = false,
                  const std::string &i_inputText = "",
                  bool i_query = false,
                  bool drawStateBehind = true,
                  bool updateStatesBehind = false,
                  bool i_verticallyLarge = false);
  StateMessageBox(StateMessageBoxReceiver *i_receiver,
                  std::vector<std::string> &completionList,
                  const std::string &i_text,
                  int i_buttons,
                  bool i_input = false,
                  const std::string &i_inputText = "",
                  bool i_query = false,
                  bool drawStateBehind = true,
                  bool updateStatesBehind = false,
                  bool i_verticallyLarge = false);
  virtual ~StateMessageBox();

  virtual void enter();
  virtual void leave();

  std::string getMsgBxId() const;
  void setMsgBxId(const std::string &i_id);

  bool isExitable() const { return m_exitable; }
  void setExitable(bool value) { m_exitable = value; }

  /* input */
  virtual void xmKey(InputEventType i_type, const XMKey &i_xmkey);
  void makeActiveButton(UIMsgBoxButton i_button);
  void setCustom(const std::string &i_custom1,
                 const std::string &i_custom2 = "");
  void setHelp(const std::string &i_help);

protected:
  virtual void checkEvents();

private:
  void initStateMessageBox(StateMessageBoxReceiver *i_receiver,
                           const std::string &i_text,
                           int i_buttons,
                           bool i_input,
                           const std::string &i_inputText,
                           bool i_query,
                           bool i_verticallyLarge);

  UIMsgBox *m_msgbox;
  StateMessageBoxReceiver *m_receiver;
  UIMsgBoxButton m_clickedButton;
  std::string m_msgbxid;

  void createGUI();

  // requirement to build the msgbox. Note that it's not directly done in the
  // constructor while it's forbiden to use the XMScreen into the constructor
  // (set after)
  std::string m_text;
  int m_buttons;
  bool m_input;
  std::string m_inputText;
  bool m_query;
  bool m_verticallyLarge;
  std::vector<std::string> m_completionList;
  std::string m_custom1, m_custom2;
  std::string m_help;
  bool m_exitable;
};

#endif
