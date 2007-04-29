/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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

namespace vapp {
  class DrawLib;
}

class SysMessage {
  public:

  SysMessage();
  ~SysMessage();

  void setDrawLib(vapp::DrawLib* i_drawLib);
  void displayText(std::string i_msg);
  void render();

  private:
  std::string m_txt;
  float m_startDisplay;
  vapp::DrawLib* m_drawLib;
};

#endif
