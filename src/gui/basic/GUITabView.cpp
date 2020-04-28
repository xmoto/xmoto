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

/*
 *  GUI: tab view control
 */
#include "GUI.h"
#include "common/VXml.h"
#include "drawlib/DrawLib.h"

/*===========================================================================
Painting
===========================================================================*/
void UITabView::paint(void) {
  /* Header height */
  int nHeaderHeight = 24;

  if (isUglyMode()) {
    putRect(0, 0, 2, getPosition().nHeight, MAKE_COLOR(188, 186, 67, 255));
    putRect(0,
            getPosition().nHeight - 2,
            getPosition().nWidth,
            2,
            MAKE_COLOR(188, 186, 67, 255));
    putRect(getPosition().nWidth - 2,
            nHeaderHeight,
            2,
            getPosition().nHeight - nHeaderHeight,
            MAKE_COLOR(188, 186, 67, 255));
  } else {
    /* Render bottom part (common to all tabs) */
    putElem(
      getPosition().nWidth - 8, nHeaderHeight, -1, -1, UI_ELEM_FRAME_TR, false);
    putElem(getPosition().nWidth - 8,
            getPosition().nHeight - 8,
            -1,
            -1,
            UI_ELEM_FRAME_BR,
            false);
    putElem(0, getPosition().nHeight - 8, -1, -1, UI_ELEM_FRAME_BL, false);
    putElem(8,
            getPosition().nHeight - 8,
            getPosition().nWidth - 16,
            -1,
            UI_ELEM_FRAME_BM,
            false);
    putElem(0,
            nHeaderHeight,
            -1,
            getPosition().nHeight - nHeaderHeight - 2,
            UI_ELEM_FRAME_ML,
            false);
    putElem(getPosition().nWidth - 8,
            8 + nHeaderHeight,
            -1,
            getPosition().nHeight - 16 - nHeaderHeight,
            UI_ELEM_FRAME_MR,
            false);
  }

  /* Render tabs */
  int nCX = 8;
  int nCY = 6;
  int v_width, v_height;

  FontManager *v_fm = m_drawLib->getFontSmall();
  FontGlyph *v_fg;
  Color v_color;

  for (unsigned int i = 0; i < getChildren().size(); i++) {
    if (m_hideDisabledTabs == false ||
        getChildren()[i]->isDisabled() == false) {
      v_fg = v_fm->getGlyph(getChildren()[i]->getCaption());
      v_width = v_fg->realWidth();
      v_height = v_fg->realHeight();

      v_color = getChildren()[i]->isDisabled() ? MAKE_COLOR(70, 70, 70, 255)
                                               : MAKE_COLOR(188, 186, 67, 255);
      setTextSolidColor(v_color);

      if (isUglyMode()) {
        putRect(nCX - 8, 0, v_width + 16, 2, MAKE_COLOR(188, 186, 67, 255));
        putRect(nCX - 8, 0, 2, nHeaderHeight, MAKE_COLOR(188, 186, 67, 255));
        putRect(nCX + v_width + 8 - 2,
                0,
                2,
                nHeaderHeight,
                MAKE_COLOR(188, 186, 67, 255));
      } else {
        putElem(nCX - 8, 0, -1, -1, UI_ELEM_FRAME_TL, false);
        putElem(nCX, 0, v_width, 8, UI_ELEM_FRAME_TM, false);
        putElem(nCX + v_width, 0, -1, -1, UI_ELEM_FRAME_TR, false);
        putElem(nCX - 8, 8, -1, nHeaderHeight - 8, UI_ELEM_FRAME_ML, false);
        putElem(
          nCX + v_width, 8, -1, nHeaderHeight - 8, UI_ELEM_FRAME_MR, false);
      }

      if (i == m_nSelected) {
        if (isUglyMode() == false) {
          putElem(2, nHeaderHeight, nCX - 8, 8, UI_ELEM_FRAME_TM, false);
          putElem(nCX + v_width + 6,
                  nHeaderHeight,
                  getPosition().nWidth - nCX - v_width - 6 - 2,
                  8,
                  UI_ELEM_FRAME_TM,
                  false);
        } else {
          putRect(2, nHeaderHeight, nCX - 8, 2, MAKE_COLOR(188, 186, 67, 255));
          putRect(nCX + v_width + 6,
                  nHeaderHeight,
                  getPosition().nWidth - nCX - v_width - 6 - 2,
                  2,
                  MAKE_COLOR(188, 186, 67, 255));
        }
      }

      if (i != m_nSelected && isUglyMode() == false) {
        putRect(nCX - 6,
                2,
                v_width + 16 - 4,
                nHeaderHeight - 2,
                MAKE_COLOR(40, 30, 30, 255));
      }
      putTextS(nCX, nCY, getChildren()[i]->getCaption(), v_width, v_height);

      nCX += v_width + 18;
    }
  }
  m_bChanged = false;
}

void UITabView::selectChildren(unsigned int i) {
  /* Hide everything except this */
  for (unsigned int j = 0; j < getChildren().size(); j++) {
    if (getChildren()[i]->isDisabled() == false) {
      if (j == i) {
        getChildren()[j]->showWindow(true);
        if (m_nSelected != j) {
          m_bChanged = true;
        }
        m_nSelected = j;
      } else {
        getChildren()[j]->showWindow(false);
      }
    }
  }
}

void UITabView::selectChildrenById(const std::string &i_id) {
  /* Hide everything except this */
  for (unsigned int j = 0; j < getChildren().size(); j++) {
    if (getChildren()[j]->isDisabled() == false) {
      if (getChildren()[j]->getID() == i_id) {
        getChildren()[j]->showWindow(true);
        if (m_nSelected != j) {
          m_bChanged = true;
        }
        m_nSelected = j;
      } else {
        getChildren()[j]->showWindow(false);
      }
    }
  }
}

/*===========================================================================
Mouse event handling
===========================================================================*/
void UITabView::mouseLDown(int x, int y) {
  /* Nice. Find out what tab was clicked (if any) */
  /* Header height */
  int nHeaderHeight = 24;
  int nCX = 8;
  FontManager *v_fm = m_drawLib->getFontSmall();
  FontGlyph *v_fg;
  int v_width;

  for (unsigned int i = 0; i < getChildren().size(); i++) {
    if (m_hideDisabledTabs == false ||
        getChildren()[i]->isDisabled() == false) {
      v_fg = v_fm->getGlyph(getChildren()[i]->getCaption());
      v_width = v_fg->realWidth();

      if (getChildren()[i]->isDisabled() == false) {
        if (x >= nCX - 8 && y >= -4 && x < nCX + 8 + v_width &&
            y < nHeaderHeight) {
          selectChildren(i);
          break;
        }
      }
      nCX += v_width + 18;
    }
  }
}

void UITabView::setTabContextHelp(unsigned int nTab, const std::string &s) {
  if (nTab >= m_TabContextHelp.size()) {
    m_TabContextHelp.resize(nTab + 1);
    m_TabContextHelp[nTab] = s;
  }
}

std::string UITabView::subContextHelp(int x, int y) {
  /* Oh... cursor inside a tab-button? */
  int nHeaderHeight = 24;
  int nCX = 8;
  FontManager *v_fm = m_drawLib->getFontSmall();
  FontGlyph *v_fg;
  int v_width;

  for (unsigned int i = 0; i < getChildren().size(); i++) {
    if (m_hideDisabledTabs == false ||
        getChildren()[i]->isDisabled() == false) {
      v_fg = v_fm->getGlyph(getChildren()[i]->getCaption());
      v_width = v_fg->realWidth();

      if (getChildren()[i]->isDisabled() == false) {
        if (x >= nCX - 8 && y >= -4 && x < nCX + 16 + v_width &&
            y < nHeaderHeight) {
          /* This one! */
          if (i < m_TabContextHelp.size())
            return m_TabContextHelp[i];
          return "";
        }
      }
      nCX += v_width + 18;
    }
  }

  return "";
}

void UITabView::mouseLUp(int x, int y) {}

void UITabView::mouseRDown(int x, int y) {}

void UITabView::mouseRUp(int x, int y) {}

void UITabView::mouseHover(int x, int y) {}
