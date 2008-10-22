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

class SysMessage : public Singleton<SysMessage> {
  friend class Singleton<SysMessage>;

private:
  SysMessage();
  ~SysMessage();

public:
  void setDrawLib(DrawLib* i_drawLib);
  void displayText(std::string i_msg);
  void displayError(std::string i_msg);
  void displayInformation(std::string i_msg);
  void render();

private:
  /* information message */
  std::string m_txt;
  float m_startDisplay;

  /* error msg */
  std::vector<std::string> m_errorTxt;
  float m_startDisplayError;
  void drawBoxMsg(float& io_nextStartTime, std::vector<std::string>& i_msg, Color i_color);

  /* information msg */
  std::vector<std::string> m_informationTxt;
  float m_startDisplayInformation;

  DrawLib* m_drawLib;
};

#endif