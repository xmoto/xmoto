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

#define SYSMSG_DISPLAYERROR_TIME 3.0
#define SYSMSG_DISPLAYERROR_ANIMATIONTIME 0.4
#define SYSMSG_DISPLAYERROR_MARGIN 20

SysMessage::SysMessage() {
  m_startDisplay = GameApp::getXMTime() - SYSMSG_DISPLAY_TIME;
  m_startDisplayError = GameApp::getXMTime() - SYSMSG_DISPLAYERROR_TIME - 2.0*(SYSMSG_DISPLAYERROR_ANIMATIONTIME);
}

SysMessage::~SysMessage() {
}

void SysMessage::setDrawLib(DrawLib* i_drawLib)
{
  m_drawLib = i_drawLib;  
}

void SysMessage::displayText(std::string i_msg) {
  m_startDisplay = GameApp::getXMTime();
  m_txt      = i_msg;
}

void SysMessage::displayError(std::string i_msg) {
  m_startDisplayError = GameApp::getXMTime();
  m_errorTxt          = i_msg;
}

void SysMessage::render() {
  if(m_drawLib == NULL)
    return;

  float v_time = GameApp::getXMTime();
  FontManager* v_fm = m_drawLib->getFontMedium();
  FontManager* v_fs = m_drawLib->getFontSmall();
  FontGlyph* v_fg;

  /* basic system message */
  if(m_startDisplay + SYSMSG_DISPLAY_TIME > v_time) {
    int v_shadow;

    if(m_startDisplay + SYSMSG_DISPLAY_DECREASE_TIME > v_time) {
      v_shadow = 255;
    } else {
      v_shadow = 255 - static_cast<int>(((v_time - m_startDisplay - SYSMSG_DISPLAY_DECREASE_TIME)
					 * 255.0)
					/ (SYSMSG_DISPLAY_TIME-SYSMSG_DISPLAY_DECREASE_TIME));
    }

    v_fg = v_fm->getGlyph(m_txt);
    v_fm->printString(v_fg,
		      m_drawLib->getDispWidth()/2 - v_fg->realWidth()/2,
		      m_drawLib->getDispHeight()/3 - v_fg->realHeight()/2,
		      MAKE_COLOR(255, 255, 255, v_shadow), true);
  }

  /* error msg */
  if(m_startDisplayError + SYSMSG_DISPLAYERROR_TIME + 2.0*(SYSMSG_DISPLAYERROR_ANIMATIONTIME) >= v_time) {
    int x_offset = 0, y_offset = 0;
    v_fg = v_fs->getGlyph(m_errorTxt);

    if(m_startDisplayError + SYSMSG_DISPLAYERROR_ANIMATIONTIME > v_time) {
      // start anim
      y_offset = +(((m_startDisplayError + SYSMSG_DISPLAYERROR_ANIMATIONTIME - v_time)
		    * (v_fg->realHeight() + 2*(SYSMSG_DISPLAYERROR_MARGIN)))
		   / SYSMSG_DISPLAYERROR_ANIMATIONTIME);
    } else if(m_startDisplayError + SYSMSG_DISPLAYERROR_TIME + SYSMSG_DISPLAYERROR_ANIMATIONTIME < v_time) {
      // end anim
      y_offset = -(((m_startDisplayError + SYSMSG_DISPLAYERROR_TIME + SYSMSG_DISPLAYERROR_ANIMATIONTIME - v_time)
		    * (v_fg->realHeight() + 2*(SYSMSG_DISPLAYERROR_MARGIN)))
		   / SYSMSG_DISPLAYERROR_ANIMATIONTIME);
    } else {
      // normal
    }

    m_drawLib->drawBox(Vector2f(x_offset, y_offset + m_drawLib->getDispHeight() - v_fg->realHeight() - 2*(SYSMSG_DISPLAYERROR_MARGIN)),
		       Vector2f(x_offset + v_fg->realWidth() + 2*(SYSMSG_DISPLAYERROR_MARGIN), y_offset + m_drawLib->getDispHeight()),
		       1.0, MAKE_COLOR(255,255,255,255), MAKE_COLOR(255,0,0,255));


    v_fs->printString(v_fg,
		      x_offset + SYSMSG_DISPLAYERROR_MARGIN,
		      y_offset + m_drawLib->getDispHeight() - v_fg->realHeight() - SYSMSG_DISPLAYERROR_MARGIN,
		      MAKE_COLOR(0, 0, 0, 255));
  }
}
