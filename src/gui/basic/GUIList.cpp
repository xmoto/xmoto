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
 *  GUI: list control
 */
#include "GUI.h"
#include "drawlib/DrawLib.h"
#include "helpers/VMath.h"
#include "helpers/utf8.h"
#include "xmoto/Game.h"
#include "xmoto/Sound.h"
#include "xmoto/input/Joystick.h"
#include <sstream>

#define GUILIST_SCROLL_SIZE 10
#define GUILIST_JUMP_COUNT 10

int UIList::HeaderHeight() {
  return m_headerHeight;
}

int UIList::HeaderSubBorderHeight() {
  return m_headerSubBorderHeight;
}

int UIList::LineMargeX() {
  return m_lineMargeX;
}

int UIList::LineMargeY() {
  return m_lineMargeY;
}

int UIList::LinesStartX() {
  return m_lineMargeX;
}

int UIList::LinesStartY() {
  return HeaderHeight() + HeaderSubBorderHeight() + LineMargeY();
}

int UIList::RowHeight() {
  return m_rowHeight;
}

int UIList::LinesWidth() {
  return getPosition().nWidth - 2 * LineMargeX() - ScrollBarArrowWidth();
}

int UIList::LinesHeight() {
  return getPosition().nHeight - LineMargeY() - LinesStartY();
}

int UIList::ScrollBarArrowWidth() {
  return m_scrollBarArrowWidth;
}

int UIList::ScrollBarArrowHeight() {
  return m_scrollBarArrowHeight;
}

int UIList::ScrollBarBarWidth() {
  return ScrollBarArrowWidth();
}

int UIList::ScrollBarBarHeight() {
  return getPosition().nHeight - 2 * LineMargeY() - 2 * ScrollBarArrowHeight();
}

int UIList::ScrollBarScrollerWidth() {
  return ScrollBarArrowWidth();
}

int UIList::ScrollBarScrollerHeight() {
  float v_visible = LinesHeight() / ((float)RowHeight());

  if (v_visible >= ((float)m_Entries.size() - m_filteredItems)) {
    return ScrollBarBarHeight();
  }

  return (int)(v_visible / ((float)m_Entries.size() - m_filteredItems) *
                 ((float)ScrollBarBarHeight()) +
               1);
}

int UIList::ScrollBarScrollerStartX() {
  return LineMargeX() + LinesWidth();
}

int UIList::ScrollBarScrollerStartY() {
  float v_visible = ScrollNbVisibleItems();

  if (v_visible >= ((float)m_Entries.size() - m_filteredItems)) {
    return ScrollBarArrowHeight() + LineMargeY();
  }

  return (int)(LineMargeY() + ScrollBarArrowHeight() +
               (((float)-m_nScroll) / ((float)RowHeight()) /
                ((float)m_Entries.size() - m_filteredItems) *
                ((float)ScrollBarBarHeight())));
}

void UIList::setScrollBarScrollerStartY(float y) {
  float v_visible = ScrollNbVisibleItems();

  if (v_visible >= ((float)m_Entries.size() - m_filteredItems)) {
    return;
  }

  _Scroll((int)(-m_nScroll + ((-y + LineMargeY() + ScrollBarArrowHeight() +
                               ScrollBarScrollerHeight() / 2.0) *
                              ((float)RowHeight()) *
                              ((float)m_Entries.size() - m_filteredItems) /
                              ((float)ScrollBarBarHeight()))));
}

float UIList::ScrollNbVisibleItems() {
  return LinesHeight() / ((float)RowHeight());
}

bool UIList::isScrollBarRequired() {
  return ScrollNbVisibleItems() < m_Entries.size();
}

UIList::UIList(UIWindow *pParent,
               int x,
               int y,
               std::string Caption,
               int nWidth,
               int nHeight) {
  initW(pParent, x, y, Caption, nWidth, nHeight);
  m_nScroll = 0;
  m_nRealSelected = 0;
  m_nVisibleSelected = 0;
  m_pEnterButton = NULL;
  m_bSort = false;
  m_fsort = NULL;
  m_bNumeroted = false;
  m_bItemActivated = false;
  m_bScrollDownPressed = m_bScrollUpPressed = false;
  m_bScrollDownHover = m_bScrollUpHover = false;
  m_bClicked = false;
  m_bChanged = false;
  m_bScrolling = false;
  m_lastRefreshTime = getApp()->getXMTime();

  /* draw */
  m_headerHeight = 18;
  m_headerSubBorderHeight = 4;
  m_rowHeight = 16;
  m_lineMargeX = 6;
  m_lineMargeY = 6;
  m_scrollBarArrowWidth = 20;
  m_scrollBarArrowHeight = 20;
  /* **** */

  m_filteredItems = 0;

  unhideAllColumns();
}

UIList::~UIList() {
  _FreeUIList();
}

std::vector<UIListEntry *> &UIList::getEntries(void) {
  return m_Entries;
}

std::vector<std::string> &UIList::getColumns(void) {
  return m_Columns;
}

unsigned int UIList::getSelected(void) {
  return m_nRealSelected;
}

int UIList::getRowAtPosition(int x, int y) {
  if (y <= LinesStartY()) {
    return -1;
  }

  if (isScrollBarRequired() && x >= ScrollBarScrollerStartX()) {
    return -1;
  }

  int n = (y - LinesStartY() - m_nScroll) / m_rowHeight;

  unsigned int n_filtered = 0;
  unsigned int n_ok = 0;
  unsigned int i = 0;
  while ((n >= 0 && n_ok <= (unsigned int)n) && i < m_Entries.size()) {
    if (m_Entries[i]->bFiltered) {
      n_filtered++;
    } else {
      n_ok++;
    }
    i++;
  }
  n += n_filtered;

  if (n < 0 || (unsigned int)n >= m_Entries.size()) {
    return -1;
  }

  return n;
}

int UIList::getColumnAtPosition(int x, int y) {
  if (isScrollBarRequired() && x >= ScrollBarScrollerStartX()) {
    return -1;
  }

  int nHX = LineMargeX();

  for (unsigned int i = 0; i < m_Columns.size(); i++) {
    if (!(m_nColumnHideFlags & (1 << i))) {
      int nW = m_ColumnWidths[i];

      for (unsigned int j = i + 1; j < m_Columns.size(); j++) {
        if (m_nColumnHideFlags & (1 << j))
          nW += m_ColumnWidths[i];
      }

      /* Mouse in this one? */
      if (x >= nHX && x <= nHX + nW)
        return i;

      nHX += nW;
    }
  }

  return -1;
}

void UIList::addColumn(std::string Title, int nWidth, const std::string &Help) {
  m_Columns.push_back(Title);
  m_ColumnWidths.push_back(nWidth);
  m_ColumnHelpStrings.push_back(Help);
}

void UIList::setEnterButton(UIButton *pButton) {
  m_pEnterButton = pButton;
}

bool UIList::isItemActivated() {
  return m_bItemActivated;
}

void UIList::setItemActivated(bool i_value) {
  m_bItemActivated = i_value;
}

void UIList::setHideColumn(int n) {
  m_nColumnHideFlags |= (1 << n);
}

void UIList::unhideAllColumns(void) {
  m_nColumnHideFlags = 0;
}

void UIList::setSort(bool bSort, int (*f)(void *pvUser1, void *pvUser2)) {
  m_bSort = bSort;
  m_fsort = f;
}

void UIList::setNumeroted(bool bNumeroted) {
  m_bNumeroted = bNumeroted;
}

bool UIList::isClicked(void) {
  return m_bClicked;
}

void UIList::setClicked(bool b) {
  m_bClicked = b;
}

bool UIList::isChanged(void) {
  return m_bChanged;
}

void UIList::setChanged(bool b) {
  m_bChanged = b;
}

void UIList::_refreshByTime() {
  float v_time = getApp()->getXMTime();

  while (m_lastRefreshTime + 0.01 < v_time) {
    if (m_bScrollDownPressed) {
      _Scroll(-(GUILIST_SCROLL_SIZE));
    }

    if (m_bScrollUpPressed) {
      _Scroll(GUILIST_SCROLL_SIZE);
    }

    m_lastRefreshTime = v_time;
  }
}

void UIList::_mouseDownManageScrollBar(int x, int y) {
  setScrollBarScrollerStartY(y);
  // int p = ((y - ScrollBarArrowHeight()) / (float)ScrollBarBarHeight()) *
  // ((float)m_Entries.size());
  // if(p < 0) p = 0;
  // if(p >= m_Entries.size()) p = m_Entries.size()-1;
  // setSelected(p);
}

/*===========================================================================
Context help at cursor position?
===========================================================================*/
std::string UIList::subContextHelp(int x, int y) {
  int n = getColumnAtPosition(x, y);
  if (n != -1) {
    return m_ColumnHelpStrings[n];
  }
  return "";
}

/*===========================================================================
Painting
===========================================================================*/
void UIList::paint(void) {
  _refreshByTime();

  bool bDisabled = isDisabled();
  bool bActive = isActive();

  /* Draw list frame */
  if (isUglyMode()) {
    putRect(1,
            1,
            getPosition().nWidth - 2,
            getPosition().nHeight - 2,
            MAKE_COLOR(0, 0, 0, 127));

  } else {
    putElem(0, 0, -1, -1, UI_ELEM_FRAME_TL, false);
    putElem(getPosition().nWidth - 8, 0, -1, -1, UI_ELEM_FRAME_TR, false);
    putElem(getPosition().nWidth - 8,
            getPosition().nHeight - 8,
            -1,
            -1,
            UI_ELEM_FRAME_BR,
            false);
    putElem(0, getPosition().nHeight - 8, -1, -1, UI_ELEM_FRAME_BL, false);
    putElem(8, 0, getPosition().nWidth - 16, -1, UI_ELEM_FRAME_TM, false);
    putElem(8,
            getPosition().nHeight - 8,
            getPosition().nWidth - 16,
            -1,
            UI_ELEM_FRAME_BM,
            false);
    putElem(0, 8, -1, getPosition().nHeight - 16, UI_ELEM_FRAME_ML, false);
    putElem(getPosition().nWidth - 8,
            8,
            -1,
            getPosition().nHeight - 16,
            UI_ELEM_FRAME_MR,
            false);

    putRect(8,
            8,
            getPosition().nWidth - 16,
            getPosition().nHeight - 16,
            MAKE_COLOR(0, 0, 0, 127));
  }

  /* Draw column headers */
  int nHX = 6, nHY = 6;

  if (isDisabled())
    setTextSolidColor(MAKE_COLOR(170, 170, 170, 128));
  else
    setTextSolidColor(MAKE_COLOR(188, 186, 67, 255));

  for (unsigned int i = 0; i < m_Columns.size(); i++) {
    if (!(m_nColumnHideFlags & (1 << i))) {
      putText(nHX, nHY, m_Columns[i]);
      nHX += m_ColumnWidths[i];

      /* Next columns disabled? If so, make more room to this one */
      for (unsigned int j = i + 1; j < m_Columns.size(); j++) {
        if (m_nColumnHideFlags & (1 << j))
          nHX += m_ColumnWidths[i];
      }
    }
  }
  putRect(6,
          m_headerHeight + 6,
          getPosition().nWidth - 12,
          2,
          MAKE_COLOR(188, 186, 67, 255));

  /* Render list */
  if (!isMouseLDown()) {
    m_bScrollDownPressed = false;
    m_bScrollUpPressed = false;
  }

  if (isScrollBarRequired()) {
    if (isUglyMode()) {
    } else {
      if (m_bScrollUpPressed && m_bScrollUpHover) {
        putElem(m_lineMargeX + LinesWidth(),
                6,
                20,
                20,
                UI_ELEM_SCROLLBUTTON_UP_DOWN,
                false);
      } else {
        putElem(m_lineMargeX + LinesWidth(),
                6,
                20,
                20,
                UI_ELEM_SCROLLBUTTON_UP_UP,
                false);
      }

      if (m_bScrollDownPressed && m_bScrollDownHover) {
        putElem(m_lineMargeX + LinesWidth(),
                LinesStartY() + LinesHeight() - 20,
                20,
                20,
                UI_ELEM_SCROLLBUTTON_DOWN_DOWN,
                false);
      } else {
        putElem(m_lineMargeX + LinesWidth(),
                LinesStartY() + LinesHeight() - 20,
                20,
                20,
                UI_ELEM_SCROLLBUTTON_DOWN_UP,
                false);
      }
    }

    /* scroll */
    putRect(ScrollBarScrollerStartX() + 2,
            ScrollBarScrollerStartY(),
            ScrollBarScrollerWidth() - 4,
            ScrollBarScrollerHeight(),
            MAKE_COLOR(188, 186, 67, 255));
  }

  setScissor(m_lineMargeX, LinesStartY(), LinesWidth(), LinesHeight());

  int m_numEntryDisplayed = 0;
  for (unsigned int i = 0; i < m_Entries.size(); i++) {
    if (m_Entries[i]->bFiltered == false) {
      if (m_Entries[i]->bUseOwnProperties) {
        setTextSolidColor(m_Entries[i]->ownTextColor);
      } else {
        if (!bDisabled)
          setTextSolidColor(MAKE_COLOR(255, 255, 255, 255));
        else
          setTextSolidColor(MAKE_COLOR(128, 128, 128, 255));
      }

      int y = m_nScroll + m_numEntryDisplayed * m_rowHeight;

      if (m_nRealSelected == i) {
        if (m_Entries[i]->bUseOwnProperties) {
          putRect(m_lineMargeX,
                  m_nScroll + LinesStartY() + m_numEntryDisplayed * m_rowHeight,
                  LinesWidth(),
                  m_rowHeight,
                  m_Entries[i]->ownSelectedColor);
        } else {
          Color c = MAKE_COLOR(70, 70, 70, 255);
          if (!bDisabled)
            c = MAKE_COLOR(160, 40, 40, 255);
          putRect(m_lineMargeX,
                  m_nScroll + LinesStartY() + m_numEntryDisplayed * m_rowHeight,
                  LinesWidth(),
                  m_rowHeight,
                  c);
        }

        if (isUglyMode()) {
          if (bDisabled) {
            if (m_Entries[i]->bUseOwnProperties == false) {
              putRect(m_lineMargeX,
                      m_nScroll + LinesStartY() +
                        m_numEntryDisplayed * m_rowHeight,
                      LinesWidth(),
                      m_rowHeight,
                      MAKE_COLOR(128, 128, 128, 255));
            }
          } else {
            if (bActive) {
              if (m_Entries[i]->bUseOwnProperties == false) {
                putRect(m_lineMargeX,
                        m_nScroll + LinesStartY() +
                          m_numEntryDisplayed * m_rowHeight,
                        LinesWidth(),
                        m_rowHeight,
                        MAKE_COLOR(200, 60, 60, 255));
              } else {
                putRect(m_lineMargeX,
                        m_nScroll + LinesStartY() +
                          m_numEntryDisplayed * m_rowHeight,
                        LinesWidth(),
                        m_rowHeight,
                        m_Entries[i]->ownSelectedColor);
              }
            } else {
              if (m_Entries[i]->bUseOwnProperties == false) {
                putRect(m_lineMargeX,
                        m_nScroll + LinesStartY() +
                          m_numEntryDisplayed * m_rowHeight,
                        LinesWidth(),
                        m_rowHeight,
                        MAKE_COLOR(160, 40, 40, 255));
              } else {
                putRect(m_lineMargeX,
                        m_nScroll + LinesStartY() +
                          m_numEntryDisplayed * m_rowHeight,
                        LinesWidth(),
                        m_rowHeight,
                        m_Entries[i]->ownUnSelectedColor);
              }
            }
          }
        } else {
          if (bActive && !bDisabled &&
              m_Entries[i]->bUseOwnProperties == false) {
            float s = 50 + 50 * sin(getApp()->getXMTime() * 10);
            int n = (int)s;
            if (n < 0)
              n = 0;
            if (n > 255)
              n = 255; /* just to be sure, i'm lazy */

            putRect(m_lineMargeX,
                    m_nScroll + LinesStartY() +
                      m_numEntryDisplayed * m_rowHeight,
                    LinesWidth(),
                    m_rowHeight,
                    MAKE_COLOR(255, 255, 255, n));
          }
        }
      } else {
        if (m_Entries[i]->bUseOwnProperties) {
          putRect(m_lineMargeX,
                  m_nScroll + LinesStartY() + m_numEntryDisplayed * m_rowHeight,
                  LinesWidth(),
                  m_rowHeight,
                  m_Entries[i]->ownUnSelectedColor);
        }
      }

      int yy = m_nScroll + LinesStartY() + m_numEntryDisplayed * m_rowHeight;
      if (yy >= getPosition().nHeight - 6)
        break;
      int yy2 = m_nScroll + LinesStartY() + m_numEntryDisplayed * m_rowHeight +
                m_rowHeight;
      int nLRH = m_rowHeight;
      if (yy2 >= getPosition().nHeight - 6)
        nLRH = m_rowHeight - (yy2 - (getPosition().nHeight - 6));
      int yym1 = m_nScroll + LinesStartY() + m_numEntryDisplayed * m_rowHeight;
      if (yym1 + m_rowHeight > LinesStartY()) {
        if (yym1 < LinesStartY()) {
          yym1 += (LinesStartY() - yym1);
        }

        int nOldScissor[4];
        // glGetIntegerv(GL_SCISSOR_BOX,(GLint *) nOldScissor);
        getApp()->getDrawLib()->getClipRect(
          &nOldScissor[0], &nOldScissor[1], &nOldScissor[2], &nOldScissor[3]);
        std::string txt_to_display;

        int x = 0;
        for (unsigned int j = 0; j < m_Entries[i]->Text.size(); j++) {
          if (!(m_nColumnHideFlags & (1 << j))) {
            /* Next columns disabled? If so, make more room to this one */
            int nExtraRoom = 0;
            for (unsigned int k = j + 1; k < m_Columns.size(); k++) {
              if (m_nColumnHideFlags & (1 << k))
                nExtraRoom += m_ColumnWidths[i];
            }

            /* Draw */
            setScissor(
              m_lineMargeX + x, yym1, m_ColumnWidths[j] - 4 + nExtraRoom, nLRH);
            txt_to_display = m_Entries[i]->Text[j];
            if (j == 0 && m_bNumeroted) {
              std::ostringstream v_num;
              v_num << m_numEntryDisplayed + 1;

              txt_to_display = "#" + v_num.str() + " " + txt_to_display;
            }
            if (m_Entries[i]->bUseOwnProperties) {
              putText(m_lineMargeX + x + m_Entries[i]->ownXOffset,
                      LinesStartY() + y + m_Entries[i]->ownYOffset,
                      txt_to_display);
            } else {
              putText(m_lineMargeX + x, LinesStartY() + y, txt_to_display);
            }
            x += m_ColumnWidths[j] + nExtraRoom;
          }
        }

        getApp()->getDrawLib()->setClipRect(
          nOldScissor[0], nOldScissor[1], nOldScissor[2], nOldScissor[3]);
      }
      m_numEntryDisplayed++;
    }
  }

  /* Stuff */
  m_bScrollDownHover = false;
  m_bScrollUpHover = false;
  m_bClicked = false;
}

/*===========================================================================
Mouse event handling
===========================================================================*/
void UIList::mouseWheelDown(int x, int y) {
  /* Scroll down! */
  _Scroll(-16 * GUILIST_SCROLL_SIZE);
}

void UIList::mouseWheelUp(int x, int y) {
  /* Scroll up! */
  _Scroll(16 * GUILIST_SCROLL_SIZE);
}

void UIList::mouseLDoubleClick(int x, int y) {
  /* is it inside the scroll bar ? */
  if (x >= m_lineMargeX + LinesWidth() + 2 &&
      x <= m_lineMargeX + LinesWidth() + 2 + 16 && y >= 6 + 20 &&
      y <= LinesStartY() + LinesHeight() - 20) {
    _mouseDownManageScrollBar(x, y);
  }
  /* Is it down inside one of the scroll buttons? */
  else if (x >= m_lineMargeX + LinesWidth() &&
           x < m_lineMargeX + LinesWidth() + 20 && y >= 6 && y < 6 + 20) {
    /* Scroll up! */
    m_bScrollUpPressed = true;
  } else if (x >= m_lineMargeX + LinesWidth() &&
             x < m_lineMargeX + LinesWidth() + 20 &&
             y >= LinesStartY() + LinesHeight() - 20 &&
             y < LinesStartY() + LinesHeight()) {
    /* Scroll down! */
    m_bScrollDownPressed = true;
  } else {
    /* Find out what item is affected */
    for (int i = 0; i < (int)(m_Entries.size() - m_filteredItems); i++) {
      int yy = m_nScroll + LinesStartY() + i * m_rowHeight;
      if (x >= m_lineMargeX && x < getPosition().nWidth - 6 && y >= yy &&
          y < yy + m_rowHeight) {
        /* Select this */
        setVisibleSelected(i);

        /* AND invoke enter-button */
        if (m_nVisibleSelected >= 0 &&
            m_nVisibleSelected < (int)(m_Entries.size() - m_filteredItems) &&
            m_pEnterButton != NULL) {
          m_pEnterButton->setClicked(true);
          m_bChanged = true;
          try {
            Sound::playSampleByName(
              Theme::instance()->getSound("Button3")->FilePath());
          } catch (Exception &e) {
          }
        }

        /* Under all circumstances set the activation flag */
        m_bItemActivated = true;
        break;
      }
    }
  }
}

void UIList::mouseLDown(int x, int y) {
  /* is it inside the scroll bar ? */
  if (x >= m_lineMargeX + LinesWidth() + 2 &&
      x <= m_lineMargeX + LinesWidth() + 2 + 16 && y >= 6 + 20 &&
      y <= LinesStartY() + LinesHeight() - 20) {
    _mouseDownManageScrollBar(x, y);
    m_bScrolling = true;
  }
  /* Is it down inside one of the scroll buttons? */
  else if (x >= m_lineMargeX + LinesWidth() &&
           x < m_lineMargeX + LinesWidth() + 20 && y >= 6 && y < 6 + 20) {
    /* Scroll up! */
    //_Scroll(16);
    m_bScrollUpPressed = true;
  } else if (x >= m_lineMargeX + LinesWidth() &&
             x < m_lineMargeX + LinesWidth() + 20 &&
             y >= LinesStartY() + LinesHeight() - 20 &&
             y < LinesStartY() + LinesHeight()) {
    /* Scroll down! */
    // _Scroll(-16);
    m_bScrollDownPressed = true;
  } else {
    /* Find out what item is affected */
    for (int i = 0; i < (int)(m_Entries.size() - m_filteredItems); i++) {
      int yy = m_nScroll + LinesStartY() + i * m_rowHeight;
      if (x >= m_lineMargeX && x < getPosition().nWidth - 6 && y >= yy &&
          y < yy + m_rowHeight) {
        /* Select this */
        setVisibleSelected(i);
        break;
      }
    }
  }
}

void UIList::mouseLUp(int x, int y) {
  /* Is it down inside one of the scroll buttons? */
  if (x >= m_lineMargeX + LinesWidth() &&
      x < m_lineMargeX + LinesWidth() + 20 && y >= 6 && y < 26) {
    /* Scroll up! */
    //_Scroll(16);
  } else if (x >= m_lineMargeX + LinesWidth() &&
             x < m_lineMargeX + LinesWidth() + 20 &&
             y >= LinesStartY() + LinesHeight() - 20 &&
             y < LinesStartY() + LinesHeight()) {
    /* Scroll down! */
    //_Scroll(-16);
  }

  m_bClicked = true;
  m_bScrolling = false;
}

void UIList::mouseRDown(int x, int y) {}

void UIList::mouseRUp(int x, int y) {}

void UIList::mouseHover(int x, int y) {
  /* is it inside the scroll bar ? */
  if (m_bScrolling && isMouseLDown()) {
    _mouseDownManageScrollBar(x, y);
  }
  /* Is it down inside one of the scroll buttons? */
  else if (x >= m_lineMargeX + LinesWidth() &&
           x < m_lineMargeX + LinesWidth() + 20 && y >= 6 && y < 6 + 20) {
    /* Scroll up! */
    m_bScrollUpHover = true;
  }

  if (x >= m_lineMargeX + LinesWidth() &&
      x < m_lineMargeX + LinesWidth() + 20 &&
      y >= LinesStartY() + LinesHeight() - 20 &&
      y < LinesStartY() + LinesHeight()) {
    /* Scroll down! */
    m_bScrollDownHover = true;
  }
}

/*===========================================================================
Allocate entry / vice versa
===========================================================================*/
UIListEntry *UIList::addEntry(std::string Text, void *pvUser, int i_position) {
  UIListEntry *p = new UIListEntry;
  p->Text.push_back(Text);
  p->pvUser = pvUser;
  p->bFiltered = false;
  p->bUseOwnProperties = false;

  if (i_position >= 0 && (unsigned int)i_position < m_Entries.size()) {
    m_Entries.insert(m_Entries.begin() + i_position, p);
    return p;
  }

  /* Make a temp. lowercase text */
  std::string LCText = Text;
  for (unsigned int i = 0; i < LCText.length(); i++)
    LCText[i] = tolower(LCText[i]);

  /* Sorted? */
  if (m_bSort) {
    /* Yeah, keep it alphabetical, please */
    for (unsigned int i = 0; i < m_Entries.size(); i++) {
      if (pvUser != NULL && m_Entries[i]->pvUser != NULL && m_fsort != NULL) {
        if (m_fsort(pvUser, m_Entries[i]->pvUser) < 0) {
          /* Here! */
          m_Entries.insert(m_Entries.begin() + i, p);
          return p;
        }
      } else {
        /* Make lowercase before comparing */
        std::string LC = m_Entries[i]->Text[0];
        for (unsigned int j = 0; j < LC.length(); j++)
          LC[j] = tolower(LC[j]);

        if (LCText < LC) {
          /* Here! */
          m_Entries.insert(m_Entries.begin() + i, p);
          return p;
        }
      }
    }
  }

  /* Just add it to the end */
  m_Entries.push_back(p);
  return p;
}

void UIList::clear(void) {
  _FreeUIList();
  m_nRealSelected = 0;
  m_nVisibleSelected = 0;
  m_nScroll = 0;
}

/* Implements Knuth shuffle
 * http://en.wikipedia.org/wiki/Knuth_shuffle
 * */
void UIList::randomize() {
  UIListEntry *v_tmp;
  int r;
  int n = m_Entries.size();
  while (n > 1) {
    r = randomIntNum(0, n);
    n--;
    v_tmp = m_Entries[n];
    m_Entries[n] = m_Entries[r];
    m_Entries[r] = v_tmp;
  }
}

/*===========================================================================
Free entries
===========================================================================*/
void UIList::_FreeUIList(void) {
  for (unsigned int i = 0; i < m_Entries.size(); i++)
    delete m_Entries[i];
  m_Entries.clear();
}

/*===========================================================================
Accept activation if there's actually anything in the list
===========================================================================*/
bool UIList::offerActivation(void) {
  if (m_Entries.empty())
    return false;
  return true;
}

void UIList::eventGo() {
  /* Uhh... send this to the default button, if any. And if anything is selected
   */
  if (m_nVisibleSelected >= 0 &&
      m_nVisibleSelected < (int)(m_Entries.size() - m_filteredItems) &&
      m_pEnterButton != NULL) {
    m_pEnterButton->setClicked(true);
    m_bChanged = true;

    try {
      Sound::playSampleByName(
        Theme::instance()->getSound("Button3")->FilePath());
    } catch (Exception &e) {
    }
  }

  /* Under all circumstances set the activation flag */
  m_bItemActivated = true;
}

void UIList::eventDown() {
  if (m_nVisibleSelected >= (int)(m_Entries.size() - m_filteredItems - 1)) {
    getRoot()->activateDown();
  } else {
    setVisibleSelected(m_nVisibleSelected + 1);
  }
}

void UIList::eventUp() {
  if (m_nVisibleSelected <= 0) {
    getRoot()->activateUp();
  } else {
    setVisibleSelected(m_nVisibleSelected - 1);
  }
}

void UIList::eventLeft() {
  getRoot()->activateLeft();
}

void UIList::eventRight() {
  getRoot()->activateRight();
}

bool UIList::eventJump(int count) {
  return eventJumpAbs(m_nVisibleSelected + count);
}

bool UIList::eventJumpAbs(int index, bool allowNegative) {
  int last = (int)(m_Entries.size() - m_filteredItems - 1);
  if (allowNegative && index < 0)
    index = last + index + 1;
  index = clamp(index, 0, last);

  setVisibleSelected(index);

  return index == 0 || index == last;
}

/*===========================================================================
Up/down keys select elements
===========================================================================*/
bool UIList::keyDown(int nKey, SDL_Keymod mod, const std::string &i_utf8Char) {
  switch (nKey) {
    case SDLK_RETURN:
      eventGo();
      return true;
    case SDLK_UP:
      eventUp();
      return true;
    case SDLK_DOWN:
      eventDown();
      return true;
    case SDLK_PAGEUP:
      if (eventJump(-GUILIST_JUMP_COUNT))
        break;
      return true;
    case SDLK_PAGEDOWN:
      if (eventJump(GUILIST_JUMP_COUNT))
        break;
      return true;
    case SDLK_HOME:
      if (eventJumpAbs(0, true))
        break;
      return true;
    case SDLK_END:
      if (eventJumpAbs(-1, true))
        break;
      return true;

    /* Left and right always selects another window */
    case SDLK_LEFT:
      eventLeft();
      return true;
    case SDLK_RIGHT:
      eventRight();
      return true;
  }

  // this must be adapted to utf8
  //    /* Maybe it's a character? If so, try jumping to a suitable place in the
  //    list */
  //    if(utf8::utf8_length(i_utf8Char) == 1) {
  //      bool bContinue;
  //      unsigned int nPos = 0;
  //
  //      do {
  //        bContinue = false;
  //
  //        /* Look at character number 'nPos' in all entries */
  //        for(unsigned int i=0; i<m_Entries.size(); i++) {
  //	  if(m_Entries[i]->bFiltered == false) {
  //	    int nEntryIdx = i + m_nRealSelected + 1; /* always start looking at
  // the next */
  //	    nEntryIdx %= m_Entries.size();
  //
  //	    if(m_Entries[nEntryIdx]->Text[0].length() > nPos) {
  //	      if(tolower(m_Entries[nEntryIdx]->Text[0].at(nPos)) ==
  // tolower(nChar)) {
  //		/* Nice, select this one */
  //		setVisibleSelected(nEntryIdx);
  //		bContinue = false;
  //		return true;
  //	      }
  //
  //	      bContinue = true;
  //	    }
  //	  }
  //        }
  //
  //        nPos++;
  //      } while(bContinue);
  //    }

  /* w00t? we don't want that silly keypress! */
  return false;
}

bool UIList::joystickAxisMotion(JoyAxisEvent event) {
  if (JoystickInput::axisInside(event.axisValue, JOYSTICK_DEADZONE_MENU))
    return false;

  JoyDir dir = JoystickInput::getJoyAxisDir(event.axisValue);

  switch (event.axis) {
    case SDL_CONTROLLER_AXIS_LEFTX:
      if (dir < 0)
        eventLeft();
      else
        eventRight();
      return true;
    case SDL_CONTROLLER_AXIS_LEFTY: {
      if (dir < 0)
        eventUp();
      else
        eventDown();
      return true;
    }

    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
      if (!eventJump(-GUILIST_JUMP_COUNT))
        return true;
      break;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
      if (!eventJump(GUILIST_JUMP_COUNT))
        return true;
      break;
  }

  return false;
}

bool UIList::joystickButtonDown(Uint8 i_joyNum, Uint8 i_joyButton) {
  switch (i_joyButton) {
    case SDL_CONTROLLER_BUTTON_A:
      eventGo();
      return true;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
      eventUp();
      return true;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
      eventDown();
      return true;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
      eventLeft();
      return true;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
      eventRight();
      return true;
  }
  return false;
}

void UIList::adaptRealSelectedOnVisibleEntries() { // m_nRealSelected must be >=
  // 0 and < size()
  bool v_found;

  if (m_Entries.size() == 0) {
    return;
  }

  // search after
  int v_nx = m_nRealSelected; // don't do +1 because it could go over list size
  // ; signed because it can be under 0
  v_found = false;
  while (v_nx < (int)getEntries().size() && v_found == false) {
    if (m_Entries[v_nx]->bFiltered == false) {
      v_found = true;
    } else {
      v_nx++; // get the next one
    }
  }
  if (v_found) {
    m_nRealSelected = v_nx;
    return;
  }

  // search before
  v_nx = m_nRealSelected; // don't do -1 to not go under 0
  v_found = false;
  while (v_nx >= 0 && v_found == false) {
    if (m_Entries[v_nx]->bFiltered == false) {
      v_found = true;
    } else {
      v_nx--; // get the previous one
    }
  }
  if (v_found) {
    m_nRealSelected = v_nx;
    return;
  }

  // no visible entry
  m_nRealSelected = 0;
}

void UIList::setRealSelected(unsigned int n) {
  if (n < 0 || n >= m_Entries.size()) {
    // error case
    m_nRealSelected = 0;
    m_nVisibleSelected = 0;
    adaptRealSelectedOnVisibleEntries();
    for (unsigned int i = 0; i < m_nRealSelected; i++) {
      if (m_Entries[i]->bFiltered == false) {
        m_nVisibleSelected++;
      }
    }
  } else {
    m_bChanged = true;
    if (m_filteredItems == 0) { // special case because it happends often
      m_nRealSelected = n;
      m_nVisibleSelected = n;
    } else {
      m_nRealSelected = n;

      if (m_Entries[m_nRealSelected]->bFiltered) {
        adaptRealSelectedOnVisibleEntries();
      }

      m_nVisibleSelected = 0;
      for (unsigned int i = 0; i < m_nRealSelected; i++) {
        if (m_Entries[i]->bFiltered == false) {
          m_nVisibleSelected++;
        }
      }
    }
  }
  _NewlySelectedItem();
}

void UIList::setVisibleSelected(unsigned int n) {
  if (n >= m_Entries.size())
    return;

  m_bChanged = true;
  if (m_filteredItems == 0) { // special case because it happends often
    m_nRealSelected = n;
    m_nVisibleSelected = n;
  } else {
    m_nVisibleSelected = n;
    m_nRealSelected = 0;

    unsigned int v = 0;
    for (unsigned int i = 0; i < m_Entries.size(); i++) {
      if (m_Entries[i]->bFiltered == false) {
        if (v == n) {
          m_nRealSelected = i;
        }
        v++;
      }
    }
  }
  _NewlySelectedItem();
}

/*===========================================================================
Called when a entry is selected
===========================================================================*/
void UIList::_NewlySelectedItem(void) {
  /* HACK HACK HACK HACK! */
  int nSelY1 = m_nScroll + LinesStartY() + m_nVisibleSelected * m_rowHeight;
  int nSelY2 =
    m_nScroll + LinesStartY() + m_nVisibleSelected * m_rowHeight + m_rowHeight;

  /* Inside view? */
  if (nSelY1 < m_headerHeight + 6 + 4) {
    /* UP! */
    _Scroll((m_headerHeight + 10) - nSelY1);
  } else if (nSelY2 > m_headerHeight + 6 + 4 + LinesHeight()) {
    /* DOWN! */
    _Scroll(m_headerHeight + 10 + LinesHeight() - nSelY2);
  }
}

/*===========================================================================
Misc helpers
===========================================================================*/
void UIList::_Scroll(int nPixels) {
  if (m_nScroll + nPixels > 0) {
    m_nScroll = 0;
    return;
  }

  int v_scroll_max =
    (int)(-(float)RowHeight() *
          ((float)m_Entries.size() - m_filteredItems - ScrollNbVisibleItems()));

  if (m_nScroll + nPixels < v_scroll_max) { /* keep the cast ; under my linux
                                               box, it doesn't work without it
                                               */
    if (ScrollNbVisibleItems() < m_Entries.size() - m_filteredItems) {
      m_nScroll = v_scroll_max;
    }
    return;
  }

  m_nScroll += nPixels;
}

void UIList::setFilter(std::string i_filter) {
  m_filter = i_filter;

  std::string v_entry_lower;
  std::string v_filter_lower;

  v_filter_lower = m_filter;
  for (unsigned int j = 0; j < v_filter_lower.length(); j++) {
    v_filter_lower[j] = tolower(v_filter_lower[j]);
  }

  for (unsigned int i = 0; i < m_Entries.size(); i++) {
    bool v_filter = true;
    for (unsigned int j = 0; j < m_Entries[i]->Text.size(); j++) {
      v_entry_lower = m_Entries[i]->Text[j];
      for (unsigned int k = 0; k < v_entry_lower.length(); k++) {
        v_entry_lower[k] = tolower(v_entry_lower[k]);
      }

      if (v_entry_lower.find(v_filter_lower, 0) != std::string::npos) {
        v_filter = false;
      }
    }

    m_Entries[i]->bFiltered = v_filter;
  }

  checkForFilteredEntries();
}

void UIList::checkForFilteredEntries() {
  m_filteredItems = 0;

  for (unsigned int i = 0; i < m_Entries.size(); i++) {
    if (m_Entries[i]->bFiltered) {
      m_filteredItems++;
    }
  }

  /* repair the scroll bar */
  m_nScroll = 0;
  setRealSelected(getSelected());
}

std::string UIList::getSelectedEntry() {
  if (getSelected() >= 0 && getSelected() < getEntries().size()) {
    UIListEntry *pEntry = getEntries()[getSelected()];
    return pEntry->Text[0];
  }
  return "";
}

int UIList::nbVisibleItems() const {
  return m_Entries.size() - m_filteredItems;
}
