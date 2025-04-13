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
 *  GUI: Static text stuff
 */
#include "GUI.h"
#include "common/VXml.h"
#include "xmoto/Game.h"

UIStatic::UIStatic(UIWindow *pParent,
                   int x,
                   int y,
                   std::string Caption,
                   int nWidth,
                   int nHeight) {
  initW(pParent, x, y, Caption, nWidth, nHeight);

  m_VAlign = UI_ALIGN_CENTER;
  m_HAlign = UI_ALIGN_CENTER;

  m_bBackgroundShade = false;

  m_pDarkBlobTexture = NULL;
  Sprite *pSprite;
  pSprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "DarkBlob");
  if (pSprite != NULL) {
    m_pDarkBlobTexture =
      pSprite->getTexture(false, WrapMode::Clamp, FM_NEAREST);
  }

  m_pCustomBackgroundTexture = NULL;
  m_allowContextHelp = false;

  m_normalColor = MAKE_COLOR(255, 255, 255, 255);
}

void UIStatic::setAllowContextHelp(bool i_value) {
  m_allowContextHelp = true;
}

/*===========================================================================
Painting
===========================================================================*/
void UIStatic::paint(void) {
  if (isUglyMode() == false) {
    /* Darken background? */
    if (m_bBackgroundShade) {
      putImage(
        0, 0, getPosition().nWidth, getPosition().nHeight, m_pDarkBlobTexture);
    }

    /* Background image? */
    if (m_pCustomBackgroundTexture != NULL) {
      putImage(0,
               0,
               getPosition().nWidth,
               getPosition().nHeight,
               m_pCustomBackgroundTexture);
    }
  }

  /* Determine text size */
  int v_x = 0, v_y = 0;
  float perX = 0.0, perY = 0.0;
  float perCentered = -1.0;

  /* Find out where to draw the text */
  if (getHAlign() == UI_ALIGN_LEFT) {
    v_x = 0;
    perCentered = -1.0;
  } else if (getHAlign() == UI_ALIGN_RIGHT) {
    v_x = getPosition().nWidth;
    perX = -1.0;
    perCentered = 1.0;
  } else if (getHAlign() == UI_ALIGN_CENTER) {
    v_x = getPosition().nWidth / 2;
    perX = -0.5;
    perCentered = 0.0;
  }

  if (getVAlign() == UI_ALIGN_TOP) {
    v_y = 0;
  } else if (getVAlign() == UI_ALIGN_BOTTOM) {
    v_y = getPosition().nHeight;
    perY = -1.0;
  } else if (getVAlign() == UI_ALIGN_CENTER) {
    v_y = getPosition().nHeight / 2;
    perY = -0.5;
  }

  if (isDisabled())
    setTextSolidColor(MAKE_COLOR(170, 170, 170, 128));
  else
    setTextSolidColor(m_normalColor);

  putText(v_x, v_y, getCaption(), perX, perY, perCentered);
}

void UIStatic::setNormalColor(Color c) {
  m_normalColor = c;
}
