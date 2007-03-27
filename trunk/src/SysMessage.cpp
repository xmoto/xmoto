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

#include "SysMessage.h"
#include "GUI.h"

#define SYSMSG_DISPLAY_TIME 1.0
#define SYSMSG_DISPLAY_DECREASE_TIME 0.75

SysMessage::SysMessage() {
  m_startDisplay = vapp::App::getTime() - SYSMSG_DISPLAY_TIME;
  m_font = NULL;
}

SysMessage::~SysMessage() {
}

void SysMessage::setFont(vapp::UIFont *i_font) {
  m_font = i_font;
}

void SysMessage::displayText(std::string i_msg) {
  m_startDisplay = vapp::App::getTime();
  m_txt      = i_msg;
}

void SysMessage::render() {
  if(m_font != NULL) {
    float v_time = vapp::App::getTime();

    if(m_startDisplay + SYSMSG_DISPLAY_TIME > v_time) {
      int v_shadow;

      if(m_startDisplay + SYSMSG_DISPLAY_DECREASE_TIME > v_time) {
	v_shadow = 255;
      } else {
	v_shadow = 255 - static_cast<int>(((v_time - m_startDisplay - SYSMSG_DISPLAY_DECREASE_TIME)
					   * 255.0)
					  / (SYSMSG_DISPLAY_TIME-SYSMSG_DISPLAY_DECREASE_TIME));
      }

      vapp::UITextDraw::printRaw(m_font, 5, 25,
				 m_txt,
				 MAKE_COLOR(255,255,255,v_shadow));
    }
  }
}
