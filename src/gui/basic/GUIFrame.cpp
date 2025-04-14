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
 *  GUI: Framed window class
 */
#include "GUI.h"
#include "helpers/utf8.h"
#include "states/GameState.h"
#include "states/StateManager.h"
#include "xmoto/Game.h"

UIFrame::UIFrame() {
  m_bMinimizable = false;
  m_bHover = false;
}

UIFrame::UIFrame(UIWindow *pParent,
                 int x,
                 int y,
                 std::string Caption,
                 int nWidth,
                 int nHeight) {
  initW(pParent, x, y, Caption, nWidth, nHeight);

  m_bMinimizable = false;
  m_bHover = false;
  m_fMinMaxTime = 0.0f;

  m_Style = UI_FRAMESTYLE_TRANS;

  m_pMenuTL = NULL;
  m_pMenuTR = NULL;
  m_pMenuBL = NULL;
  m_pMenuBR = NULL;

  Sprite *pSprite;
  pSprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "MenuTL");
  if (pSprite != NULL) {
    m_pMenuTL = pSprite->getTexture(false, WrapMode::Clamp, FM_NEAREST);
  }

  pSprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "MenuTR");
  if (pSprite != NULL) {
    m_pMenuTR = pSprite->getTexture(false, WrapMode::Clamp, FM_NEAREST);
  }

  pSprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "MenuBL");
  if (pSprite != NULL) {
    m_pMenuBL = pSprite->getTexture(false, WrapMode::Clamp, FM_NEAREST);
  }

  pSprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "MenuBR");
  if (pSprite != NULL) {
    m_pMenuBR = pSprite->getTexture(false, WrapMode::Clamp, FM_NEAREST);
  }
}

/*===========================================================================
Painting
===========================================================================*/
void UIFrame::paint(void) {
  /* This depends on the style */
  switch (m_Style) {
    case UI_FRAMESTYLE_MENU:
      if (isUglyMode() == false) {
        putImage(
          0, 0, getPosition().nWidth / 2, getPosition().nHeight / 2, m_pMenuTL);
        putImage(getPosition().nWidth / 2,
                 0,
                 getPosition().nWidth / 2,
                 getPosition().nHeight / 2,
                 m_pMenuTR);
        putImage(getPosition().nWidth / 2,
                 getPosition().nHeight / 2,
                 getPosition().nWidth / 2,
                 getPosition().nHeight / 2,
                 m_pMenuBR);
        putImage(0,
                 getPosition().nHeight / 2,
                 getPosition().nWidth / 2,
                 getPosition().nHeight / 2,
                 m_pMenuBL);
      } else {
        // getPosition().nWidth/3 -- arbitrary values for the shadow
        int v_shadow = 10;
        putRect(0 + getPosition().nWidth / v_shadow,
                getPosition().nHeight / v_shadow,
                getPosition().nWidth - 2 * getPosition().nWidth / v_shadow,
                getPosition().nHeight - 2 * getPosition().nHeight / v_shadow,
                UGLY_MODE_WINDOW_BG);
      }
      break;
    case UI_FRAMESTYLE_TRANS:
      if (isUglyMode()) {
        putRect(0,
                0,
                getPosition().nWidth,
                getPosition().nHeight,
                UGLY_MODE_WINDOW_BG);
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
                MAKE_COLOR(0, 0, 0, 200));
      }
      break;
    case UI_FRAMESTYLE_LEFTTAG: { // currently only used for stats window in
      // main menu
      if (isUglyMode()) {
        putRect(0,
                0,
                getPosition().nWidth,
                getPosition().nHeight,
                UGLY_MODE_WINDOW_BG);
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
        putRect(2,
                2,
                getPosition().nWidth - 4,
                getPosition().nHeight - 4,
                MAKE_COLOR(0, 0, 70, 150));
      }

      if (m_bHover)
        setTextSolidColor(MAKE_COLOR(255, 255, 255, 255));
      else
        setTextSolidColor(MAKE_COLOR(188, 186, 67, 255));

      putText(4, 14, utf8::txt2vertical(getCaption()), 0.0, 0.0, 0.0);
    } break;
  }

  /* Update position according to minimization/maximation? */
  if (m_bMinimizable) {
    if (!isActive() && !m_bMinimized) {
      /* No longer active, minimize */
      m_bMinimized = true;
      m_fMinMaxTime = getApp()->getXMTime();
    }

    int nTargetX, nTargetY;
    if (m_bMinimized) {
      nTargetX = m_nMinimizedX;
      nTargetY = m_nMinimizedY;
    } else {
      nTargetX = m_nMaximizedX;
      nTargetY = m_nMaximizedY;
    }

    float targetFps =
      (float)StateManager::instance()->getTopState()->getRenderFps();
    float animationSpeed = targetFps > 0.0f ? (30.0f / targetFps) : 1.0f;

    /* Is the window at target position? */
    if (getPosition().nX != nTargetX) {
      int nDiffX = nTargetX - getPosition().nX;
      int nVelX = ((float)nDiffX * animationSpeed) / 4;
      if (getPosition().nX < nTargetX && nVelX == 0)
        nVelX = 1;
      if (getPosition().nX > nTargetX && nVelX == 0)
        nVelX = -1;
      setPosition(getPosition().nX + nVelX,
                  getPosition().nY,
                  getPosition().nWidth,
                  getPosition().nHeight);
    }

    if (getPosition().nY != nTargetY) {
      int nDiffY = nTargetY - getPosition().nY;
      int nVelY = ((float)nDiffY * animationSpeed) / 4;
      if (getPosition().nY < nTargetY && nVelY == 0)
        nVelY = 1;
      if (getPosition().nY > nTargetY && nVelY == 0)
        nVelY = -1;
      setPosition(getPosition().nX,
                  getPosition().nY + nVelY,
                  getPosition().nWidth,
                  getPosition().nHeight);
    }

    /* If this takes too long, just set the position */
    if (getApp()->getXMTime() - m_fMinMaxTime > 1.0f) {
      setPosition(
        nTargetX, nTargetY, getPosition().nWidth, getPosition().nHeight);
    }
  }
}

/*===========================================================================
Clicking
===========================================================================*/
void UIFrame::mouseLDown(int x, int y) {
  toggle();
}

void UIFrame::toggle() {
  m_bMinimized = !m_bMinimized;
  m_fMinMaxTime = getApp()->getXMTime();

  if (!m_bMinimized) {
    makeActive();
  }
}

/*===========================================================================
Clicking
===========================================================================*/
void UIFrame::mouseHover(int x, int y) {
  m_bHover = true;
}

void UIFrame::mouseOut(int x, int y) {
  m_bHover = false;
}

/*===========================================================================
For minimizable frames
===========================================================================*/
void UIFrame::makeMinimizable(int nMinX, int nMinY) {
  m_bMinimizable = true;
  m_bMinimized = false;

  m_nMinimizedX = nMinX;
  m_nMinimizedY = nMinY;

  m_nMaximizedX = getPosition().nX;
  m_nMaximizedY = getPosition().nY;
}

void UIFrame::setMinimized(bool b) {
  m_bMinimized = b;

  if (m_bMinimizable) {
    int nTargetX, nTargetY;
    if (m_bMinimized) {
      nTargetX = m_nMinimizedX;
      nTargetY = m_nMinimizedY;
    } else {
      nTargetX = m_nMaximizedX;
      nTargetY = m_nMaximizedY;
    }

    setPosition(
      nTargetX, nTargetY, getPosition().nWidth, getPosition().nHeight);
  }
}
