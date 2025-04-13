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
#include "Game.h"
#include "drawlib/DrawLib.h"
#include "net/NetClient.h"

#define SYSMSG_DISPLAY_TIME 1.9
#define SYSMSG_DISPLAY_DECREASE_TIME 0.75

#define SYSMSG_DISPLAYBOXMSG_TIME 3.0
#define SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME 0.4
#define SYSMSG_DISPLAYBOXMSG_MARGIN 20

#define SYSMSG_CONSOLEDISPLAY_TIME 4.0
#define SYSMSG_CONSOLEDISPLAY_ANIMATIONTIME 1.0
#define SYSMSG_CONSOLEDISPLAY_MAXNBLINES 50
#define SYSMSG_DEFAULT_CONSOLEDISPLAY_DISPLAYNBLINES 5

SysMsg::SysMsg(const std::string &i_msg, SysMsgType i_type) {
  text = i_msg;
  time = GameApp::getXMTime();
  type = i_type;
}

SysMsg::~SysMsg() {}

SysMessage::SysMessage() {
  m_startDisplay = GameApp::getXMTime() - SYSMSG_DISPLAY_TIME;
  m_consoleLastShowTime = GameApp::getXMTime() - SYSMSG_CONSOLEDISPLAY_TIME -
                          SYSMSG_CONSOLEDISPLAY_ANIMATIONTIME;
  m_consoleSize = SYSMSG_DEFAULT_CONSOLEDISPLAY_DISPLAYNBLINES;
  m_consoleTextWidth = -1; // undefined
  m_consoleTextHeight = -1;
}

SysMessage::~SysMessage() {}

void SysMessage::setDrawLib(DrawLib *i_drawLib) {
  m_drawLib = i_drawLib;
  m_screen = RenderSurface(
    Vector2i(0, 0),
    Vector2i(m_drawLib->getDispWidth(), m_drawLib->getDispHeight()));
}

void SysMessage::displayText(const std::string &i_msg) {
  m_startDisplay = GameApp::getXMTime();
  m_txt = i_msg;
}

void SysMessage::displayMsg(const std::string &i_msg, SysMsgType i_type) {
  if (m_sysMsg.size() > 0) {
    if (m_sysMsg[m_sysMsg.size() - 1]->text == i_msg) {
      // don't add the message if that's alread the same as the previous one and
      // that the previous one is still displayed
      return;
    }
  }

  m_sysMsg.push_back(new SysMsg(i_msg, i_type));
}

void SysMessage::displayError(const std::string &i_msg) {
  displayMsg(i_msg, SYSMSG_ERROR);
}

void SysMessage::displayInformation(const std::string &i_msg) {
  displayMsg(i_msg, SYSMSG_INFORMATION);
}

unsigned int SysMessage::consoleSize() const {
  return m_consoleSize;
}

void SysMessage::setConsoleSize(unsigned int i_value) {
  m_consoleSize = i_value;
  resetBackgroundbox();
}

void SysMessage::alterConsoleSize(int i_diffLines) {
  if (((int)m_consoleSize) + i_diffLines < 1) {
    i_diffLines = 1;
  } else if (((int)m_consoleSize) + i_diffLines >=
             SYSMSG_CONSOLEDISPLAY_MAXNBLINES) {
    m_consoleSize = SYSMSG_CONSOLEDISPLAY_MAXNBLINES;
  } else {
    m_consoleSize += i_diffLines;
  }
  resetBackgroundbox();
}

/* basic system message */
void SysMessage::render_basic() {
  FontManager *v_fm;
  FontGlyph *v_fg;
  int v_shadow;

  float v_time = GameApp::getXMTime();

  v_fm = m_drawLib->getFontMedium();

  if (m_startDisplay + SYSMSG_DISPLAY_TIME > v_time) {
    if (m_startDisplay + SYSMSG_DISPLAY_DECREASE_TIME > v_time) {
      v_shadow = 255;
    } else {
      v_shadow =
        255 -
        static_cast<int>(
          ((v_time - m_startDisplay - SYSMSG_DISPLAY_DECREASE_TIME) * 255.0) /
          (SYSMSG_DISPLAY_TIME - SYSMSG_DISPLAY_DECREASE_TIME));
    }

    v_fg = v_fm->getGlyph(m_txt);
    v_fm->printString(m_drawLib,
                      v_fg,
                      m_drawLib->getDispWidth() / 2 - v_fg->realWidth() / 2,
                      5,
                      MAKE_COLOR(255, 255, 255, v_shadow),
                      0.0,
                      true);
  }
}

void SysMessage::render_boxes() {
  /* error/information msg */
  cleanBoxMsg();
  drawBoxMsg();
}

void SysMessage::resetBackgroundbox() {
  m_consoleTextWidth = m_consoleTextHeight = -1;
}

void SysMessage::consoleText_computeAndDraw(int i_shadow,
                                            int i_xoffset,
                                            int i_yoffset,
                                            bool i_draw) {
  bool v_useShadow;
  Color c;
  unsigned int v_firstLine;
  FontManager *v_fm;
  FontGlyph *v_fg;
  int v_yoffset = i_yoffset;

  // no need to compute
  if (m_consoleTextWidth > 0 && m_consoleTextHeight > 0 && i_draw == false) {
    return;
  }

  v_fm = m_drawLib->getFontSmall();

  // don't display old history of the console
  v_firstLine = 0;
  if (((int)m_console.size()) - ((int)m_consoleSize) > 0) {
    v_firstLine = m_console.size() - m_consoleSize;
  }

  for (unsigned int i = v_firstLine; i < m_console.size(); i++) {
    v_fg = v_fm->getGlyph(m_console[i].cltxt);

    v_useShadow = false;

    switch (m_console[i].cltype) {
      case CLT_NORMAL:
        c = MAKE_COLOR(255, 255, 255, i_shadow);
        v_useShadow = true;
        break;
      case CLT_INFORMATION:
        c = MAKE_COLOR(255, 255, 55, i_shadow);
        break;
      case CLT_GAMEINFORMATION:
        c = MAKE_COLOR(55, 255, 255, i_shadow);
        break;
      case CLT_SERVER:
        c = MAKE_COLOR(255, 0, 0, i_shadow);
        break;
      case CLT_PRIVATE:
        c = MAKE_COLOR(80, 200, 80, i_shadow);
        break;
    }

    // print only if console dimension are set ; else, just a run for
    // computation
    if (i_draw) {
      v_fm->printString(
        m_drawLib, v_fg, i_xoffset, v_yoffset, c, 0.0, v_useShadow);
    }
    v_yoffset += v_fg->realHeight();

    if (((int)v_fg->realWidth()) > m_consoleTextWidth) {
      m_consoleTextWidth = v_fg->realWidth();
    }
  }
  m_consoleTextHeight = v_yoffset - i_yoffset; // only what has been added
}

void SysMessage::render_console() {
  int v_shadow;

  int v_consoleXOffset = m_drawLib->getDispWidth() / 3;
  int v_consoleYOffsetInit = 10;
  int v_consoleYOffset = v_consoleYOffsetInit;
  int v_bboxborder = 5;

  // don't draw if nothing to draw
  if (m_console.size() == 0) {
    return;
  }

  float v_time = GameApp::getXMTime();

  if ((m_consoleLastShowTime + SYSMSG_CONSOLEDISPLAY_TIME +
           SYSMSG_CONSOLEDISPLAY_ANIMATIONTIME >
         v_time ||
       XMSession::instance()->permanentConsole() ||
       StateManager::instance()->isThereASuchState("StateMessageBox")) &&
      NetClient::instance()->isConnected()) {
    if ((m_consoleLastShowTime + SYSMSG_CONSOLEDISPLAY_TIME > v_time ||
         XMSession::instance()->permanentConsole() ||
         StateManager::instance()->isThereASuchState("StateMessageBox")) &&
        NetClient::instance()->isConnected()) {
      v_shadow = 255;
    } else {
      v_shadow = ((m_consoleLastShowTime + SYSMSG_CONSOLEDISPLAY_TIME +
                   SYSMSG_CONSOLEDISPLAY_ANIMATIONTIME - v_time) *
                  255) /
                 SYSMSG_CONSOLEDISPLAY_ANIMATIONTIME;
    }

    // compute text with for box rendering
    consoleText_computeAndDraw(
      v_shadow, v_consoleXOffset, v_consoleYOffset, false);
    // background box
    m_drawLib->drawBox(
      Vector2f(v_consoleXOffset - v_bboxborder,
               v_consoleYOffset - v_bboxborder),
      Vector2f(v_consoleXOffset + m_consoleTextWidth + v_bboxborder,
               v_consoleYOffset + m_consoleTextHeight + v_bboxborder),
      0.0,
      MAKE_COLOR(0, 0, 0, v_shadow / 2));
    //
    consoleText_computeAndDraw(
      v_shadow, v_consoleXOffset, v_consoleYOffset, true);
  }
}

void SysMessage::render() {
  if (m_drawLib == NULL)
    return;

  render_basic();
  render_boxes();
  render_console();
}

void SysMessage::addConsoleLine(const std::string &i_line,
                                consoleLineType i_clt) {
  consoleLine clt;

  // do not add game information in console if not wanted
  if (i_clt == CLT_GAMEINFORMATION &&
      XMSession::instance()->showGameInformationInConsole() == false) {
    return;
  }

  clt.cltxt = i_line;
  clt.cltype = i_clt;
  m_console.push_back(clt);

  if (m_console.size() > SYSMSG_CONSOLEDISPLAY_MAXNBLINES) {
    m_console.erase(m_console.begin());
  }

  resetBackgroundbox();
  showConsole();
}

void SysMessage::showConsole() {
  m_consoleLastShowTime = GameApp::getXMTime();
}

void SysMessage::cleanBoxMsg() {
  float v_time;

  if (m_sysMsg.size() == 0) {
    return;
  }

  v_time = GameApp::getXMTime();
  while (m_sysMsg[0]->time + SYSMSG_DISPLAYBOXMSG_TIME +
           2.0 * (SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME) <
         v_time) {
    delete m_sysMsg[0];
    m_sysMsg.erase(m_sysMsg.begin());

    if (m_sysMsg.size() == 0) {
      return;
    }
  }
}

/* draw recursively to be able to compute offset before drawing */
void SysMessage::drawBoxMsg_one(unsigned int i,
                                float i_time,
                                int x_offset,
                                int y_offset) {
  FontManager *v_fs = m_drawLib->getFontSmall();
  FontGlyph *v_fg;
  int v_boxHeight;
  Color c;

  v_fg = v_fs->getGlyph(m_sysMsg[i]->text);

  /* remove the hidden part */
  if (m_sysMsg[i]->time + SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME > i_time) {
    /* apparition (start animation) */
    y_offset +=
      -(((m_sysMsg[i]->time + SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME - i_time) *
         (v_fg->realHeight() + 2 * (SYSMSG_DISPLAYBOXMSG_MARGIN))) /
        SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME);

  } else if (m_sysMsg[i]->time + SYSMSG_DISPLAYBOXMSG_TIME +
               SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME <
             i_time) {
    /* disparition (end animation) */

    y_offset += (((m_sysMsg[i]->time + SYSMSG_DISPLAYBOXMSG_TIME +
                   SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME - i_time) *
                  (v_fg->realHeight() + 2 * (SYSMSG_DISPLAYBOXMSG_MARGIN))) /
                 SYSMSG_DISPLAYBOXMSG_ANIMATIONTIME);
  }

  v_boxHeight = v_fg->realHeight() + 2 * (SYSMSG_DISPLAYBOXMSG_MARGIN);

  /* draw next boxes */
  if (m_sysMsg.size() > i + 1) {
    /* add offset for the next box */
    drawBoxMsg_one(i + 1, i_time, x_offset, y_offset + v_boxHeight);
  }

  /* draw the box */
  c = m_sysMsg[i]->type == SYSMSG_INFORMATION ? MAKE_COLOR(0, 0, 255, 255)
                                              : MAKE_COLOR(255, 0, 0, 255);

  m_drawLib->drawBox(
    Vector2f(x_offset, m_drawLib->getDispHeight() - y_offset - v_boxHeight),
    Vector2f(x_offset + v_fg->realWidth() + 2 * (SYSMSG_DISPLAYBOXMSG_MARGIN),
             m_drawLib->getDispHeight() - y_offset),
    1.0,
    MAKE_COLOR(255, 255, 255, 255),
    c);

  v_fs->printString(m_drawLib,
                    v_fg,
                    x_offset + SYSMSG_DISPLAYBOXMSG_MARGIN,
                    m_drawLib->getDispHeight() - y_offset - v_fg->realHeight() -
                      SYSMSG_DISPLAYBOXMSG_MARGIN,
                    MAKE_COLOR(0, 0, 0, 255),
                    0.0);
}

void SysMessage::drawBoxMsg() {
  if (m_sysMsg.size() > 0) {
    drawBoxMsg_one(0, GameApp::getXMTime(), 0, 0);
  }
}
