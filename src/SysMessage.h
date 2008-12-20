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

#ifndef __SYSMESSAGE_H__
#define __SYSMESSAGE_H__

#include <string>
#include <vector>
#include "helpers/Singleton.h"
#include "helpers/Color.h"

class DrawLib;

enum SysMsgType {SYSMSG_INFORMATION, SYSMSG_ERROR};

class SysMsg {
 public:
  SysMsg(const std::string& i_msg, SysMsgType i_type);
  ~SysMsg();

  std::string text;
  float time;
  SysMsgType type;
};

class SysMessage : public Singleton<SysMessage> {
  friend class Singleton<SysMessage>;

private:
  SysMessage();
  ~SysMessage();

public:
  void setDrawLib(DrawLib* i_drawLib);
  void displayText(const std::string& i_msg);
  void displayError(const std::string& i_msg);
  void displayInformation(const std::string& i_msg);
  void addConsoleLine(const std::string& i_line);
  void showConsole();
  void render();

private:
  /* information message */
  std::string m_txt;
  float m_startDisplay;

  /* error msg */
  /* information msg */
  std::vector<SysMsg*> m_sysMsg;

  void cleanBoxMsg();
  void drawBoxMsg();
  void drawBoxMsg_one(unsigned int i, float i_time, int x_offset, int y_offset);

  /* console */
  std::vector<std::string> m_console;
  float m_consoleLastShowTime;

  DrawLib* m_drawLib;
};

#endif
