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

#include "XMKey.h"
#include "Game.h"
#include "GameText.h"
#include "Input.h"
#include "helpers/VExcept.h"
#include <sstream>

XMKey::XMKey() {
  m_input = XMK_NONE;
  m_repetition = 1;
}

XMKey::XMKey(Uint8 nButton, unsigned int i_repetition) {
  m_input = XMK_MOUSEBUTTON;
  m_mouseButton_button = nButton;
  m_repetition = i_repetition;
}

XMKey::XMKey(SDL_Event &i_event) {
  m_repetition = 1;

  switch (i_event.type) {
    case SDL_KEYDOWN:
      m_input = XMK_KEYBOARD;
      m_keyboard_sym = i_event.key.keysym.sym; // Change this back to scancode if it doesn't work
      m_keyboard_mod = (SDL_Keymod)(i_event.key.keysym.mod &
                                (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT |
                                 KMOD_GUI)); // only allow these modifiers
      break;

    case SDL_MOUSEBUTTONDOWN:
      m_input = XMK_MOUSEBUTTON;
      m_mouseButton_button = i_event.button.button;
      break;

    case SDL_MOUSEWHEEL:
      m_input = XMK_MOUSEWHEEL;
      m_wheelX = i_event.wheel.x;
      m_wheelY = i_event.wheel.y;
      break;

    case SDL_JOYAXISMOTION:
      m_input = XMK_JOYSTICKAXIS;
      m_joyId = InputHandler::instance()->getJoyId(i_event.jaxis.which);
      m_joyAxis = i_event.jaxis.axis;
      m_joyAxisValue = i_event.jaxis.value;
      break;

    case SDL_JOYBUTTONDOWN:
      m_input = XMK_JOYSTICKBUTTON;
      m_joyId = InputHandler::instance()->getJoyId(i_event.jbutton.which);
      m_joyButton = i_event.jbutton.button;
      break;

    default:
      throw Exception("Unknown key");
  }
}

XMKey::XMKey(SDL_Keycode nKey, SDL_Keymod mod, const std::string &i_utf8Char) {
  m_input = XMK_KEYBOARD;
  m_keyboard_sym = nKey;
  m_keyboard_mod = (SDL_Keymod)(mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT |
                                       KMOD_GUI)); // only allow these modifiers
  m_keyboard_utf8Char = i_utf8Char;
  m_repetition = 1;
}

XMKey::XMKey(std::string *i_joyId, Uint8 i_joyButton) {
  m_input = XMK_JOYSTICKBUTTON;
  m_joyId = i_joyId;
  m_joyButton = i_joyButton;
  m_repetition = 1;
}

XMKey::XMKey(std::string *i_joyId, Uint8 i_joyAxis, Sint16 i_joyAxisValue) {
  m_input = XMK_JOYSTICKAXIS;
  m_joyId = i_joyId;
  m_joyAxis = i_joyAxis;
  m_joyAxisValue = i_joyAxisValue;
  m_repetition = 1;
}

XMKey::XMKey(const std::string &i_key, bool i_basicMode) {
  size_t pos;
  std::string v_current, v_rest;

  m_repetition = 1;

  if (i_basicMode) {
    m_input = XMK_KEYBOARD;

    if (i_key.length() != 1) {
      throw InvalidSystemKeyException();
    }
    m_keyboard_sym = (SDL_Keymod)(
      (int)(tolower(i_key[0]))); // give ascii key while sdl does the same
    m_keyboard_mod = KMOD_NONE;
    return;
  }

  if (i_key == "N") {
    m_input = XMK_NONE;
    return;
  }

  v_rest = i_key;
  pos = v_rest.find(":", 0);

  if (pos == std::string::npos) {
    throw InvalidSystemKeyException();
  }
  v_current = v_rest.substr(0, pos);
  v_rest = v_rest.substr(pos + 1, v_rest.length() - pos - 1);

  if (v_current == "K") { // keyboard
    m_input = XMK_KEYBOARD;

    pos = v_rest.find(":", 0);
    if (pos == std::string::npos) {
      throw InvalidSystemKeyException();
    }
    v_current = v_rest.substr(0, pos);
    v_rest = v_rest.substr(pos + 1, v_rest.length() - pos - 1);

    m_keyboard_sym = (SDL_Keycode)atoi(v_current.c_str());
    m_keyboard_mod = (SDL_Keymod)(((SDL_Keymod)atoi(v_rest.c_str())) &
                              (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT |
                               KMOD_GUI)); // only allow these modifiers

  } else if (v_current == "M") { // mouse button
    m_input = XMK_MOUSEBUTTON;
    m_mouseButton_button = (Uint8)atoi(v_rest.c_str());

  } else if (v_current == "A") { // joystick axis
    m_input = XMK_JOYSTICKAXIS;

    // get the axis
    pos = v_rest.find(":", 0);
    if (pos == std::string::npos) {
      throw InvalidSystemKeyException();
    }
    v_current = v_rest.substr(0, pos);
    v_rest = v_rest.substr(pos + 1, v_rest.length() - pos - 1);
    m_joyAxis = (Uint8)atoi(v_current.c_str());

    // get the axis value
    pos = v_rest.find(":", 0);
    if (pos == std::string::npos) {
      throw InvalidSystemKeyException();
    }
    v_current = v_rest.substr(0, pos);
    v_rest = v_rest.substr(pos + 1, v_rest.length() - pos - 1);
    m_joyAxisValue = (Sint16)atoi(v_current.c_str());

    // get the joyid
    m_joyId = InputHandler::instance()->getJoyIdByStrId(v_rest);
  } else if (v_current == "J") { // joystick button
    m_input = XMK_JOYSTICKBUTTON;

    pos = v_rest.find(":", 0);
    if (pos == std::string::npos) {
      throw Exception("Invalid key");
    }
    v_current = v_rest.substr(0, pos);
    v_rest = v_rest.substr(pos + 1, v_rest.length() - pos - 1);

    m_joyId = InputHandler::instance()->getJoyIdByStrId(v_rest);
    m_joyButton = (Uint8)atoi(v_current.c_str());
  } else {
    throw InvalidSystemKeyException();
  }
}

bool XMKey::isDefined() const {
  return m_input != XMK_NONE;
}

unsigned int XMKey::getRepetition() const {
  return m_repetition;
}

bool XMKey::isAnalogic() const {
  return m_input == XMK_JOYSTICKAXIS;
}

float XMKey::getAnalogicValue() const {
  return InputHandler::joyRawToFloat(m_joyAxisValue,
                                     -(INPUT_JOYSTICK_MAXIMUM_VALUE),
                                     -(INPUT_JOYSTICK_MINIMUM_DETECTION),
                                     INPUT_JOYSTICK_MINIMUM_DETECTION,
                                     INPUT_JOYSTICK_MAXIMUM_VALUE);
}

bool XMKey::isDirectionnel() const {
  return m_input == XMK_JOYSTICKAXIS;
}

XMKey_direction XMKey::getDirection() const {
  if (m_joyAxis % 2 == 0) { // horizontal
    if (m_joyAxisValue < -(INPUT_JOYSTICK_MINIMUM_DETECTION)) {
      return XMKD_LEFT;
    } else if (m_joyAxisValue > INPUT_JOYSTICK_MINIMUM_DETECTION) {
      return XMKD_RIGHT;
    }
  } else { // vertical
    if (m_joyAxisValue < -(INPUT_JOYSTICK_MINIMUM_DETECTION)) {
      return XMKD_UP;
    } else if (m_joyAxisValue > INPUT_JOYSTICK_MINIMUM_DETECTION) {
      return XMKD_DOWN;
    }
  }

  return XMKD_NODIRECTION;
}

bool XMKey::operator==(const XMKey &i_other) const {
  if (m_input != i_other.m_input) {
    return false;
  }

  if (m_repetition != i_other.m_repetition) {
    return false;
  }

  if (m_input == XMK_NONE || i_other.m_input == XMK_NONE) {
    return false;
  }

  if (m_input == XMK_KEYBOARD) {
    return m_keyboard_sym == i_other.m_keyboard_sym &&
           ((m_keyboard_mod &
             (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_GUI)) ==
            (i_other.m_keyboard_mod &
             (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT |
              KMOD_GUI))); // only allow these modifiers
  }

  if (m_input == XMK_MOUSEBUTTON) {
    return m_mouseButton_button == i_other.m_mouseButton_button;
  }

  if (m_input == XMK_JOYSTICKAXIS) {
    // for m_joyId, pointer instead of strings can be compared safely while
    // strings are required only when reading config
    return m_joyId == i_other.m_joyId && m_joyAxis == i_other.m_joyAxis &&
           !(
             // axes are not on the same side if the value are on the opposite
             // sides
             (m_joyAxisValue > INPUT_JOYSTICK_MINIMUM_DETECTION &&
              i_other.m_joyAxisValue < -(INPUT_JOYSTICK_MINIMUM_DETECTION)) ||
             (m_joyAxisValue < -(INPUT_JOYSTICK_MINIMUM_DETECTION) &&
              i_other.m_joyAxisValue > INPUT_JOYSTICK_MINIMUM_DETECTION));
  }

  if (m_input == XMK_JOYSTICKBUTTON) {
    // for m_joyId, pointer instead of strings can be compared safely while
    // strings are required only when reading config
    return m_joyId == i_other.m_joyId && m_joyButton == i_other.m_joyButton;
  }

  return false;
}

std::string XMKey::toString() const {
  std::ostringstream v_res;

  switch (m_input) {
    case XMK_KEYBOARD:
      v_res << "K:" << m_keyboard_sym << ":" << m_keyboard_mod;
      break;

    case XMK_MOUSEBUTTON:
      v_res << "M:" << ((int)m_mouseButton_button);
      break;

    case XMK_JOYSTICKAXIS:
      // put the joyid at the end while it can contain any char
      v_res << "A:"
            << ((int)m_joyAxis) << ":" << ((int)m_joyAxisValue) << ":"
            << *m_joyId;
      break;

    case XMK_JOYSTICKBUTTON:
      // put the joyid at the end while it can contain any char
      v_res << "J:" << ((int)m_joyButton) << ":" << *m_joyId;
      break;

    case XMK_NONE:
      v_res << "N";
      break;
  }

  return v_res.str();
}

std::string XMKey::toFancyString() const {
  std::ostringstream v_res;

  if (m_input == XMK_KEYBOARD) {
    std::string v_modifiers;

    if ((m_keyboard_mod & KMOD_LCTRL) == KMOD_LCTRL) {
      v_modifiers += GAMETEXT_KEY_LEFTCONTROL;
      v_modifiers += " ";
    }

    if ((m_keyboard_mod & KMOD_RCTRL) == KMOD_RCTRL) {
      v_modifiers += GAMETEXT_KEY_RIGHTCONTROL;
      v_modifiers += " ";
    }

    if ((m_keyboard_mod & KMOD_LALT) == KMOD_LALT) {
      v_modifiers += GAMETEXT_KEY_LEFTALT;
      v_modifiers += " ";
    }

    if ((m_keyboard_mod & KMOD_RALT) == KMOD_RALT) {
      v_modifiers += GAMETEXT_KEY_RIGHTALT;
      v_modifiers += " ";
    }

    if ((m_keyboard_mod & KMOD_LGUI) == KMOD_LGUI) {
      v_modifiers += GAMETEXT_KEY_LEFTMETA;
      v_modifiers += " ";
    }

    if ((m_keyboard_mod & KMOD_RGUI) == KMOD_RGUI) {
      v_modifiers += GAMETEXT_KEY_RIGHTMETA;
      v_modifiers += " ";
    }

    if ((m_keyboard_mod & KMOD_LSHIFT) == KMOD_LSHIFT) {
      v_modifiers += GAMETEXT_KEY_LEFTSHIFT;
      v_modifiers += " ";
    }

    if ((m_keyboard_mod & KMOD_RSHIFT) == KMOD_RSHIFT) {
      v_modifiers += GAMETEXT_KEY_RIGHTSHIFT;
      v_modifiers += " ";
    }

    v_res << "[" << GAMETEXT_KEYBOARD << "] " << v_modifiers
          << SDL_GetKeyName(m_keyboard_sym);
  } else if (m_input == XMK_MOUSEBUTTON) {
    std::string v_button;

    switch (m_mouseButton_button) {
      case SDL_BUTTON_LEFT:
        v_button = GAMETEXT_MOUSE_LEFTBUTTON;
        break;

      case SDL_BUTTON_MIDDLE:
        v_button = GAMETEXT_MOUSE_MIDDLEBUTTON;
        break;

      case SDL_BUTTON_RIGHT:
        v_button = GAMETEXT_MOUSE_RIGHTBUTTON;
        break;

      /* TODO:
      case SDL_MOUSEWHEEL:
        v_button = GAMETEXT_MOUSE_WHEELUPBUTTON;
        v_button = GAMETEXT_MOUSE_WHEELDOWNBUTTON;
        break;
      */

      default:
        char v_button_tmp[256];
        snprintf(
          v_button_tmp, 256, GAMETEXT_MOUSE_BUTTON, m_mouseButton_button);
        v_button = v_button_tmp;
    }
    v_res << "[" << GAMETEXT_MOUSE << "] " << v_button;
  } else if (m_input == XMK_JOYSTICKAXIS) {
    std::string v_direction;
    char v_axis_tmp[256];
    snprintf(v_axis_tmp, 256, GAMETEXT_JOYSTICK_AXIS, m_joyAxis);

    if (isDirectionnel()) {
      v_direction = " - ";
      switch (getDirection()) {
        case XMKD_LEFT:
          v_direction += GAMETEXT_JOYSTICK_DIRECTION_LEFT;
          break;
        case XMKD_RIGHT:
          v_direction += GAMETEXT_JOYSTICK_DIRECTION_RIGHT;
          break;
        case XMKD_UP:
          v_direction += GAMETEXT_JOYSTICK_DIRECTION_UP;
          break;
        case XMKD_DOWN:
          v_direction += GAMETEXT_JOYSTICK_DIRECTION_DOWN;
          break;
        case XMKD_NODIRECTION:
          v_direction += GAMETEXT_JOYSTICK_DIRECTION_NONE;
      }
    }
    v_res << "[" << GAMETEXT_JOYSTICK << "] " << *m_joyId << " - " << v_axis_tmp
          << v_direction;

  } else if (m_input == XMK_JOYSTICKBUTTON) {
    char v_button[256];
    snprintf(v_button,
             256,
             GAMETEXT_JOYSTICK_BUTTON,
             m_joyButton + 1); // +1 because it starts at 0
    v_res << "[" << GAMETEXT_JOYSTICK << "] " << *m_joyId << " - " << v_button;
  } else if (m_input == XMK_NONE) {
    v_res << GAMETEXT_UNDEFINED;
  }

  return v_res.str();
}

bool XMKey::isPressed(const Uint8 *i_keystate, Uint8 i_mousestate) const {
  if (m_input == XMK_KEYBOARD) {
    /* It'd probably be a good idea to perform a range-check here,
     * just to be safe.. */
    return i_keystate[SDL_GetScancodeFromKey(m_keyboard_sym)] == 1;
  }

  if (m_input == XMK_MOUSEBUTTON) {
    return (i_mousestate & SDL_BUTTON(m_mouseButton_button)) ==
           SDL_BUTTON(m_mouseButton_button);
  }

  if (m_input == XMK_JOYSTICKAXIS) {
    Sint16 v_axisValue = SDL_JoystickGetAxis(
      InputHandler::instance()->getJoyById(m_joyId), m_joyAxis);
    return ((v_axisValue > INPUT_JOYSTICK_MINIMUM_DETECTION &&
             m_joyAxisValue > INPUT_JOYSTICK_MINIMUM_DETECTION) ||
            (v_axisValue < -(INPUT_JOYSTICK_MINIMUM_DETECTION) &&
             m_joyAxisValue < -(INPUT_JOYSTICK_MINIMUM_DETECTION)));
  }

  if (m_input == XMK_JOYSTICKBUTTON) {
    return SDL_JoystickGetButton(InputHandler::instance()->getJoyById(m_joyId),
                                 m_joyButton) == 1;
  }

  return false;
}

bool XMKey::toKeyboard(SDL_Keycode &nKey,
                       SDL_Keymod &o_mod,
                       std::string &o_utf8Char) const {
  if (m_input != XMK_KEYBOARD) {
    return false;
  }

  nKey = m_keyboard_sym;
  o_mod = m_keyboard_mod;
  o_utf8Char = m_keyboard_utf8Char;

  return true;
}

bool XMKey::toMouse(int &nX, int &nY, Uint8 &nButton) const {
  if (m_input != XMK_MOUSEBUTTON) {
    return false;
  }

  GameApp::getMousePos(&nX, &nY);
  nButton = m_mouseButton_button;
  return true;
}

bool XMKey::toMouseWheel(int &nX, int &nY, Sint32 &wheelX, Sint32 &wheelY) const {
  if (m_input != XMK_MOUSEWHEEL) {
    return false;
  }

  // used by UIRoot::_RootMouseEvent
  GameApp::getMousePos(&nX, &nY);

  wheelX = m_wheelX;
  wheelY = m_wheelY;

  return true;
}

bool XMKey::toJoystickButton(Uint8 &o_joyNum, Uint8 &o_joyButton) const {
  if (m_input != XMK_JOYSTICKBUTTON) {
    return false;
  }

  // takes potentially time. could be better by storing the joy id
  o_joyNum = InputHandler::instance()->getJoyNum(*m_joyId);

  o_joyButton = m_joyButton;

  return true;
}

bool XMKey::toJoystickAxisMotion(Uint8 &o_joyNum,
                                 Uint8 &o_joyAxis,
                                 Sint16 &o_joyAxisValue) const {
  if (m_input != XMK_JOYSTICKAXIS) {
    return false;
  }

  // takes potentially time. could be better by storing the joy id
  o_joyNum = InputHandler::instance()->getJoyNum(*m_joyId);

  o_joyAxis = m_joyAxis;
  o_joyAxisValue = m_joyAxisValue;

  return true;
}

