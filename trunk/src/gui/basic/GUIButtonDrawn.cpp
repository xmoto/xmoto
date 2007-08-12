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

#include "GUI.h"
#include "../../VDraw.h"
#include "../../Game.h"

UIButtonDrawn::UIButtonDrawn(UIWindow *pParent,
			     const std::string& i_spriteUnpressed,
			     const std::string& i_spritePressed,
			     const std::string& i_spriteHover,
			     int x, int y,
			     std::string Caption,
			     int nWidth, int nHeight)
  : UIButton(pParent, x, y, Caption, nWidth, nHeight) {
    Sprite *v_sprite;

    m_border = 0;
    m_textureUnpressed = m_texturePressed = m_textureHover = NULL;

    v_sprite = getApp()->getTheme()->getSprite(SPRITE_TYPE_UI, i_spritePressed);
    if(v_sprite != NULL) {
      m_texturePressed = v_sprite->getTexture();
    }

    v_sprite = getApp()->getTheme()->getSprite(SPRITE_TYPE_UI, i_spriteUnpressed);
    if(v_sprite != NULL) {
      m_textureUnpressed = v_sprite->getTexture();
    }

    v_sprite = getApp()->getTheme()->getSprite(SPRITE_TYPE_UI, i_spriteHover);
    if(v_sprite != NULL) {
      m_textureHover = v_sprite->getTexture();
    }
  }

UIButtonDrawn::~UIButtonDrawn() {
}

void UIButtonDrawn::setBorder(int i_border) {
  m_border = i_border;
}

void UIButtonDrawn::paint() {
  int x, y;
  Color c1,c2,c3,c4;
  c1=c2=c3=c4=MAKE_COLOR(255,255,255,(int)(255*getOpacity()/100));

  float fX1 = 0.0;
  float fY1 = 0.0;
  float fX2 = 1.0;
  float fY2 = 1.0;
  int cx = getAbsPosX() + m_border;
  int cy = getAbsPosY() + m_border;
  int w = getPosition().nWidth  - m_border * 2;
  int h = getPosition().nHeight - m_border * 2;

  if(isUglyMode()) {
  } else {
    if(m_textureUnpressed != NULL && m_texturePressed != NULL) {
      switch(m_State) {
	case UI_BUTTON_STATE_PRESSED:
	if(m_bHover) {
	  getApp()->getDrawLib()->setTexture(m_textureHover, BLEND_MODE_A);
	} else {
	  getApp()->getDrawLib()->setTexture(m_texturePressed, BLEND_MODE_A);
	}
        if(!isMouseLDown()) {
	  m_State = UI_BUTTON_STATE_UNPRESSED;
	}
	break;
	default:
	getApp()->getDrawLib()->setTexture(m_textureUnpressed, BLEND_MODE_A);
      }
      
      getApp()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
      getApp()->getDrawLib()->setColor(c1);
      getApp()->getDrawLib()->glTexCoord(fX1,fY1);
      getApp()->getDrawLib()->glVertexSP(cx,cy);
      getApp()->getDrawLib()->setColor(c2);
      getApp()->getDrawLib()->glTexCoord(fX2,fY1);        
      getApp()->getDrawLib()->glVertexSP(cx+w,cy);
      getApp()->getDrawLib()->setColor(c3);
      getApp()->getDrawLib()->glTexCoord(fX2,fY2);
      getApp()->getDrawLib()->glVertexSP(cx+w,cy+h);
      getApp()->getDrawLib()->setColor(c4);
      getApp()->getDrawLib()->glTexCoord(fX1,fY2);
      getApp()->getDrawLib()->glVertexSP(cx,cy+h);
      getApp()->getDrawLib()->endDraw();
    }
  }
  
  /* draw the caption */
  x = getPosition().nWidth/2;
  y = getPosition().nHeight/2;
  putText(x, y, getCaption(), -0.5, -0.5);

  /* Clear stuff */
  m_bHover = false;
  m_bClicked = false;
}
