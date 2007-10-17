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

#include "SysMessage.h"
#include "drawlib/DrawLib.h"
#include "Game.h"

#define SYSMSG_DISPLAY_TIME 1.6
#define SYSMSG_DISPLAY_DECREASE_TIME 0.75

SysMessage::SysMessage(DrawLib* i_drawLib) {
  m_startDisplay = GameApp::getXMTime() - SYSMSG_DISPLAY_TIME;
  m_drawLib = i_drawLib;
}

SysMessage::~SysMessage() {
}

void SysMessage::displayText(std::string i_msg) {
  m_startDisplay = GameApp::getXMTime();
  m_txt      = i_msg;
}

void SysMessage::render() {
  if(m_drawLib == NULL) return;

  float v_time = GameApp::getXMTime();

  if(m_startDisplay + SYSMSG_DISPLAY_TIME > v_time) {
    int v_shadow;

    if(m_startDisplay + SYSMSG_DISPLAY_DECREASE_TIME > v_time) {
      v_shadow = 255;
    } else {
      v_shadow = 255 - static_cast<int>(((v_time - m_startDisplay - SYSMSG_DISPLAY_DECREASE_TIME)
					 * 255.0)
					/ (SYSMSG_DISPLAY_TIME-SYSMSG_DISPLAY_DECREASE_TIME));
    }

    FontManager* v_fm = m_drawLib->getFontMedium();
    FontGlyph* v_fg = v_fm->getGlyph(m_txt);
    v_fm->printString(v_fg,
		      m_drawLib->getDispWidth()/2 - v_fg->realWidth()/2,
		      m_drawLib->getDispHeight()/3 - v_fg->realHeight()/2,
		      MAKE_COLOR(255, 255, 255, v_shadow), true);
  }
}
