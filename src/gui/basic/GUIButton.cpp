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
 *  GUI: button
 */
#include "GUI.h"
#include "xmoto/Game.h"
#include "xmoto/Sound.h"

/*===========================================================================
Painting
===========================================================================*/
void UIButton::paint(void) {
  bool bDisabled = isDisabled();
  bool bActive = isActive();

  /* HACK, right now we ignore "active" */
  // bActive = false;

  /* Draw button graphics */
  switch (m_State) {
    case UI_BUTTON_STATE_PRESSED:
      if (isUglyMode()) {
        putRect(1,
                1,
                getPosition().nWidth - 2,
                getPosition().nHeight - 2,
                MAKE_COLOR(255, 255, 255, 255));
      } else {
        if (m_bHover) {
          switch (m_Type) {
            case UI_BUTTON_TYPE_LARGE:
              putElem(0,
                      0,
                      getPosition().nWidth,
                      getPosition().nHeight,
                      UI_ELEM_LARGE_BUTTON_DOWN,
                      bDisabled,
                      bActive);
              break;
            case UI_BUTTON_TYPE_SMALL:
              putElem(0,
                      0,
                      getPosition().nWidth,
                      getPosition().nHeight,
                      UI_ELEM_SMALL_BUTTON_DOWN,
                      bDisabled,
                      bActive);
              break;
            case UI_BUTTON_TYPE_CHECK:
              if (getChecked()) {
                putElem(0,
                        getPosition().nHeight / 2 - 34 / 2,
                        34,
                        34,
                        UI_ELEM_CHECKBUTTON_CHECKED_DOWN,
                        bDisabled,
                        bActive);
              } else {
                putElem(0,
                        getPosition().nHeight / 2 - 34 / 2,
                        34,
                        34,
                        UI_ELEM_CHECKBUTTON_UNCHECKED_DOWN,
                        bDisabled,
                        bActive);
              }
              break;
            case UI_BUTTON_TYPE_RADIO:
              if (getChecked()) {
                putElem(0,
                        getPosition().nHeight / 2 - 34 / 2,
                        34,
                        34,
                        UI_ELEM_RADIOBUTTON_CHECKED_DOWN,
                        bDisabled,
                        bActive);
              } else {
                putElem(0,
                        getPosition().nHeight / 2 - 34 / 2,
                        34,
                        34,
                        UI_ELEM_RADIOBUTTON_UNCHECKED_DOWN,
                        bDisabled,
                        bActive);
              }
              break;
          }
        } else {
          switch (m_Type) {
            case UI_BUTTON_TYPE_LARGE:
              putElem(0,
                      0,
                      getPosition().nWidth,
                      getPosition().nHeight,
                      UI_ELEM_LARGE_BUTTON_UP,
                      bDisabled,
                      bActive);
              break;
            case UI_BUTTON_TYPE_SMALL:
              putElem(0,
                      0,
                      getPosition().nWidth,
                      getPosition().nHeight,
                      UI_ELEM_SMALL_BUTTON_UP,
                      bDisabled,
                      bActive);
              break;
            case UI_BUTTON_TYPE_CHECK:
              if (getChecked()) {
                putElem(0,
                        getPosition().nHeight / 2 - 34 / 2,
                        34,
                        34,
                        UI_ELEM_CHECKBUTTON_CHECKED_UP,
                        bDisabled,
                        bActive);
              } else {
                putElem(0,
                        getPosition().nHeight / 2 - 34 / 2,
                        34,
                        34,
                        UI_ELEM_CHECKBUTTON_UNCHECKED_UP,
                        bDisabled,
                        bActive);
              }
              break;
            case UI_BUTTON_TYPE_RADIO:
              if (getChecked()) {
                putElem(0,
                        getPosition().nHeight / 2 - 34 / 2,
                        34,
                        34,
                        UI_ELEM_RADIOBUTTON_CHECKED_UP,
                        bDisabled,
                        bActive);
              } else {
                putElem(0,
                        getPosition().nHeight / 2 - 34 / 2,
                        34,
                        34,
                        UI_ELEM_RADIOBUTTON_UNCHECKED_UP,
                        bDisabled,
                        bActive);
              }
              break;
          }
        }
      }

      if (!isMouseLDown())
        m_State = UI_BUTTON_STATE_UNPRESSED;

      break;
    case UI_BUTTON_STATE_UNPRESSED:

      if (isUglyMode()) {
        switch (m_Type) {
          case UI_BUTTON_TYPE_LARGE:
          case UI_BUTTON_TYPE_SMALL:
            if (bDisabled) {
              putRect(1,
                      1,
                      getPosition().nWidth - 2,
                      getPosition().nHeight - 2,
                      MAKE_COLOR(80, 20, 20, 255));
            } else {
              if (bActive) {
                putRect(1,
                        1,
                        getPosition().nWidth - 2,
                        getPosition().nHeight - 2,
                        MAKE_COLOR(200, 80, 80, 255));
              } else {
                putRect(1,
                        1,
                        getPosition().nWidth - 2,
                        getPosition().nHeight - 2,
                        MAKE_COLOR(160, 40, 40, 255));
              }
            }
            break;
          case UI_BUTTON_TYPE_CHECK:
          case UI_BUTTON_TYPE_RADIO:
            if (getChecked()) {
              if (bDisabled) {
                putRect(1,
                        1,
                        getPosition().nWidth - 2,
                        getPosition().nHeight - 2,
                        MAKE_COLOR(10, 80, 10, 255));
              } else {
                putRect(1,
                        1,
                        getPosition().nWidth - 2,
                        getPosition().nHeight - 2,
                        MAKE_COLOR(80, 200, 80, 255));
              }
            } else {
              putRect(1,
                      1,
                      getPosition().nWidth - 2,
                      getPosition().nHeight - 2,
                      MAKE_COLOR(50, 50, 50, 255));
            }
            break;
        }
      } else {
        switch (m_Type) {
          case UI_BUTTON_TYPE_LARGE:
            putElem(0,
                    0,
                    getPosition().nWidth,
                    getPosition().nHeight,
                    UI_ELEM_LARGE_BUTTON_UP,
                    bDisabled,
                    bActive);
            break;
          case UI_BUTTON_TYPE_SMALL:
            putElem(0,
                    0,
                    getPosition().nWidth,
                    getPosition().nHeight,
                    UI_ELEM_SMALL_BUTTON_UP,
                    bDisabled,
                    bActive);
            break;
          case UI_BUTTON_TYPE_CHECK:
            if (getChecked()) {
              putElem(0,
                      getPosition().nHeight / 2 - 34 / 2,
                      34,
                      34,
                      UI_ELEM_CHECKBUTTON_CHECKED_UP,
                      bDisabled,
                      bActive);
            } else {
              putElem(0,
                      getPosition().nHeight / 2 - 34 / 2,
                      34,
                      34,
                      UI_ELEM_CHECKBUTTON_UNCHECKED_UP,
                      bDisabled,
                      bActive);
            }
            break;
          case UI_BUTTON_TYPE_RADIO:
            if (getChecked()) {
              putElem(0,
                      getPosition().nHeight / 2 - 34 / 2,
                      34,
                      34,
                      UI_ELEM_RADIOBUTTON_CHECKED_UP,
                      bDisabled,
                      bActive);
            } else {
              putElem(0,
                      getPosition().nHeight / 2 - 34 / 2,
                      34,
                      34,
                      UI_ELEM_RADIOBUTTON_UNCHECKED_UP,
                      bDisabled,
                      bActive);
            }
            break;
        }
        break;
      }
  }

  if (bDisabled)
    setTextSolidColor(MAKE_COLOR(170, 170, 170, 128));
  else
    setTextSolidColor(MAKE_COLOR(255, 255, 255, 255));

  int x, y;

  switch (m_Type) {
    case UI_BUTTON_TYPE_CHECK:
    case UI_BUTTON_TYPE_RADIO:
      x = 34;
      y = getPosition().nHeight / 2;
      putText(x, y, getCaption(), 0.0, -0.5);
      break;
    default:
      x = getPosition().nWidth / 2;
      y = getPosition().nHeight / 2;
      putText(x, y, getCaption(), -0.5, -0.45, 0.0);
      break;
  }
}

/*===========================================================================
Mouse event handling
===========================================================================*/
void UIButton::mouseLDown(int x, int y) {
  m_State = UI_BUTTON_STATE_PRESSED;
}

void UIButton::mouseLUp(int x, int y) {
  if (m_State == UI_BUTTON_STATE_PRESSED) {
    m_bClicked = true;

    if (m_Type == UI_BUTTON_TYPE_CHECK) {
      m_bChecked = !m_bChecked;
      try {
        Sound::playSampleByName(
          Theme::instance()->getSound("Button3")->FilePath());
      } catch (Exception &e) {
      }
    } else if (m_Type == UI_BUTTON_TYPE_RADIO) {
      _UncheckGroup(getGroup());
      m_bChecked = true;
      try {
        Sound::playSampleByName(
          Theme::instance()->getSound("Button3")->FilePath());
      } catch (Exception &e) {
      }
    } else {
      try {
        Sound::playSampleByName(
          Theme::instance()->getSound("Button1")->FilePath());
      } catch (Exception &e) {
      }
    }
  }
  m_State = UI_BUTTON_STATE_UNPRESSED;
}

void UIButton::mouseRDown(int x, int y) {}

void UIButton::mouseRUp(int x, int y) {}

void UIButton::mouseHover(int x, int y) {
  m_bHover = true;
}

void UIButton::mouseOut(int x, int y) {
  m_bHover = false;
}

bool UIButton::offerActivation(void) {
  /* Uhh, buttons like being clicked to acquire focus */
  return true;
}

/*===========================================================================
Keyboard event handling
===========================================================================*/
bool UIButton::keyDown(int nKey,
                       SDL_Keymod mod,
                       const std::string &i_utf8Char) {
  switch (nKey) {
    case SDLK_UP:
      getRoot()->activateUp();
      return true;
    case SDLK_DOWN:
      getRoot()->activateDown();
      return true;
    case SDLK_LEFT:
      getRoot()->activateLeft();
      return true;
    case SDLK_RIGHT:
      getRoot()->activateRight();
      return true;
    case SDLK_SPACE:
    case SDLK_RETURN:
      toggle();
      return true;
  }

  return false;
}

bool UIButton::joystickButtonDown(Uint8 i_joyNum, Uint8 i_joyButton) {
  switch (i_joyButton) {
    case SDL_CONTROLLER_BUTTON_A:
      toggle();
      return true;
  }
  return false;
}

void UIButton::toggle() {
  if (m_Type == UI_BUTTON_TYPE_CHECK) {
    m_bChecked = !m_bChecked;
    try {
      Sound::playSampleByName(
        Theme::instance()->getSound("Button3")->FilePath());
    } catch (Exception &e) {
    }
  } else if (m_Type == UI_BUTTON_TYPE_RADIO) {
    _UncheckGroup(getGroup());
    m_bChecked = true;
    try {
      Sound::playSampleByName(
        Theme::instance()->getSound("Button3")->FilePath());
    } catch (Exception &e) {
    }
  } else {
    setClicked(true);
    try {
      Sound::playSampleByName(
        Theme::instance()->getSound("Button1")->FilePath());
    } catch (Exception &e) {
    }
  }
}

/*===========================================================================
Misc
===========================================================================*/
void UIButton::_UncheckGroup(int nGroup) {
  if (nGroup != 0) {
    UIWindow *pParent = getParent();
    if (pParent != NULL) {
      for (unsigned int i = 0; i < pParent->getChildren().size(); i++) {
        if (pParent->getChildren()[i]->getGroup() == nGroup) {
          /* This is dangerous. But I'm very lazy right now, and just want to
           * get stuff done. :| */
          ((UIButton *)pParent->getChildren()[i])->m_bChecked = false;
        }
      }
    }
  }
}
