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

#define SYSMSG_DISPLAYBOXMSG_TIME 3.0
#define SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME 0.4
#define SYSMSG_DISPLAYBOXMSG_MARGIN 20

#define SYSMSG_CONSOLEDISPLAY_TIME 4.0
#define SYSMSG_CONSOLEDISPLAY_ANIMATIONTIME 1.0
#define SYSMSG_CONSOLEDISPLAY_MAXNBLINES 5

SysMessage::SysMessage() {
  m_startDisplay = GameApp::getXMTime() - SYSMSG_DISPLAY_TIME;
  m_startDisplayError = GameApp::getXMTime() - SYSMSG_DISPLAYBOXMSG_TIME - 2.0*(SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME);
  m_startDisplayInformation = GameApp::getXMTime() - SYSMSG_DISPLAYBOXMSG_TIME - 2.0*(SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME);
  m_consoleLastShowTime = GameApp::getXMTime() - SYSMSG_CONSOLEDISPLAY_TIME - SYSMSG_CONSOLEDISPLAY_ANIMATIONTIME;
}

SysMessage::~SysMessage() {
}

void SysMessage::setDrawLib(DrawLib* i_drawLib)
{
  m_drawLib = i_drawLib;  
}

void SysMessage::displayText(const std::string& i_msg) {
  m_startDisplay = GameApp::getXMTime();
  m_txt = i_msg;
}

void SysMessage::displayError(const std::string& i_msg) {
  if(m_errorTxt.size() == 0) {
    m_startDisplayError = GameApp::getXMTime();
  }
  m_errorTxt.push_back(i_msg);
}

void SysMessage::displayInformation(const std::string& i_msg) {
  if(m_informationTxt.size() == 0) {
    m_startDisplayInformation = GameApp::getXMTime();
  }
  m_informationTxt.push_back(i_msg);
}

void SysMessage::render() {
  if(m_drawLib == NULL)
    return;

  float v_time = GameApp::getXMTime();
  FontManager* v_fm;
  FontGlyph* v_fg;
  int v_shadow;

  v_fm = m_drawLib->getFontMedium();

  /* basic system message */
  if(m_startDisplay + SYSMSG_DISPLAY_TIME > v_time) {
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
		      MAKE_COLOR(255, 255, 255, v_shadow), 0.0, true);
  }

  /* error/information msg */
  if(m_errorTxt.size() > 0) {
    drawBoxMsg(m_startDisplayError, m_errorTxt, MAKE_COLOR(255,0,0,255));
    m_startDisplayInformation = GameApp::getXMTime(); // reset information until it can be displayed
  } else if(m_informationTxt.size() > 0) {
    drawBoxMsg(m_startDisplayInformation, m_informationTxt, MAKE_COLOR(0,0,255,255));
  }

  /* console */
  int v_consoleBorder = 10;
  int v_consoleYOffset = 0;

  v_fm = m_drawLib->getFontSmall();

  if(m_consoleLastShowTime + SYSMSG_CONSOLEDISPLAY_TIME + SYSMSG_CONSOLEDISPLAY_ANIMATIONTIME > v_time) {
    if(m_consoleLastShowTime + SYSMSG_CONSOLEDISPLAY_TIME > v_time) {
      v_shadow = 255;
    } else {
      v_shadow = ((m_consoleLastShowTime + SYSMSG_CONSOLEDISPLAY_TIME + SYSMSG_CONSOLEDISPLAY_ANIMATIONTIME - v_time)
		  * 255)/SYSMSG_CONSOLEDISPLAY_ANIMATIONTIME;
    }

    for(unsigned int i=0; i<m_console.size(); i++) {
      v_fg = v_fm->getGlyph(m_console[i]);
      v_fm->printString(v_fg,
			m_drawLib->getDispWidth()/3,
			v_consoleBorder+v_consoleYOffset,
			MAKE_COLOR(255, 255, 255, v_shadow), 0.0, true);
      v_consoleYOffset += v_fg->realHeight();
    }
  }
}

void SysMessage::addConsoleLine(const std::string& i_line) {
  m_console.push_back(i_line);

  if(m_console.size() > SYSMSG_CONSOLEDISPLAY_MAXNBLINES) {
    m_console.erase(m_console.begin());
  }

  m_consoleLastShowTime = GameApp::getXMTime();
}

void SysMessage::drawBoxMsg(float& io_nextStartTime, std::vector<std::string>& i_msg, Color i_color) {
  float v_time = GameApp::getXMTime();
  FontManager* v_fs = m_drawLib->getFontSmall();
  FontGlyph* v_fg;

  if(io_nextStartTime + SYSMSG_DISPLAYBOXMSG_TIME + 2.0*(SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME) >= v_time) {
    int x_offset = 0, y_offset = 0;
    v_fg = v_fs->getGlyph(i_msg[0]);
    
    if(io_nextStartTime + SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME > v_time) {
      // start anim
      y_offset = +(((io_nextStartTime + SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME - v_time)
		    * (v_fg->realHeight() + 2*(SYSMSG_DISPLAYBOXMSG_MARGIN)))
		   / SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME);
    } else if(io_nextStartTime + SYSMSG_DISPLAYBOXMSG_TIME + SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME < v_time) {
      // end anim
      y_offset = -(((io_nextStartTime + SYSMSG_DISPLAYBOXMSG_TIME + SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME - v_time)
		    * (v_fg->realHeight() + 2*(SYSMSG_DISPLAYBOXMSG_MARGIN)))
		   / SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME);
    } else {
      // normal
    }
    
    m_drawLib->drawBox(Vector2f(x_offset,
				y_offset + m_drawLib->getDispHeight() - v_fg->realHeight() - 2*(SYSMSG_DISPLAYBOXMSG_MARGIN)),
		       Vector2f(x_offset + v_fg->realWidth() + 2*(SYSMSG_DISPLAYBOXMSG_MARGIN),
				y_offset + m_drawLib->getDispHeight()),
		       1.0, MAKE_COLOR(255,255,255,255), i_color);
    
    v_fs->printString(v_fg,
		      x_offset + SYSMSG_DISPLAYBOXMSG_MARGIN,
		      y_offset + m_drawLib->getDispHeight() - v_fg->realHeight() - SYSMSG_DISPLAYBOXMSG_MARGIN,
		      MAKE_COLOR(0, 0, 0, 255), 0.0);
  } else {
    i_msg.erase(i_msg.begin());
    io_nextStartTime = GameApp::getXMTime();
  }
}
