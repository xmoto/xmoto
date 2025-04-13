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
#include "Input.h"
#include "Joystick.h"
#include "helpers/VExcept.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"
#include <sstream>

XMKey::XMKey() {
  m_type = XMK_NONE;
  m_joyButton = SDL_CONTROLLER_BUTTON_INVALID;
  m_repetition = 0;
}

XMKey::XMKey(Uint8 nButton, unsigned int i_repetition) {
  m_type = XMK_MOUSEBUTTON;
  m_mouseButton_button = nButton;
  m_joyButton = SDL_CONTROLLER_BUTTON_INVALID;
  m_repetition = i_repetition;
}

XMKey::XMKey(SDL_Event &i_event) {
  m_repetition = 0;
  m_joyButton = SDL_CONTROLLER_BUTTON_INVALID;

  switch (i_event.type) {
    case SDL_KEYDOWN:
      m_type = XMK_KEYBOARD;
      m_keyboard_sym =
        i_event.key.keysym
          .sym; // Change this back to scancode if it doesn't work
      m_keyboard_mod = (SDL_Keymod)(i_event.key.keysym.mod &
                                    (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT |
                                     KMOD_GUI)); // only allow these modifiers
      m_repetition = i_event.key.repeat;
      break;

    case SDL_MOUSEBUTTONDOWN:
      m_type = XMK_MOUSEBUTTON;
      m_mouseButton_button = i_event.button.button;
      break;

    case SDL_MOUSEWHEEL:
      m_type = XMK_MOUSEWHEEL;
      m_wheelX = i_event.wheel.x;
      m_wheelY = i_event.wheel.y;
      break;

    case SDL_CONTROLLERAXISMOTION:
      m_type = XMK_JOYSTICKAXIS;
      m_joystick = Input::instance()->getJoyById(i_event.caxis.which);
      m_joyAxis = i_event.caxis.axis;
      m_joyAxisValue = i_event.caxis.value;
      break;

    case SDL_CONTROLLERBUTTONDOWN:
      m_type = XMK_JOYSTICKBUTTON;
      m_joystick = Input::instance()->getJoyById(i_event.cbutton.which);
      m_joyButton = i_event.cbutton.button;
      break;

    default:
      throw Exception("Unknown key");
  }
}

XMKey::XMKey(SDL_Keycode nKey,
             SDL_Keymod mod,
             const std::string &i_utf8Char,
             int repetition) {
  m_type = XMK_KEYBOARD;
  m_keyboard_sym = nKey;
  m_keyboard_mod = (SDL_Keymod)(mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT |
                                       KMOD_GUI)); // only allow these modifiers
  m_keyboard_utf8Char = i_utf8Char;
  m_joyButton = SDL_CONTROLLER_BUTTON_INVALID;
  m_repetition = repetition;
}

XMKey::XMKey(Joystick *joystick, Uint8 i_joyButton) {
  m_type = XMK_JOYSTICKBUTTON;
  m_joystick = joystick;
  m_joyButton = i_joyButton;
  m_repetition = 0;
}

XMKey::XMKey(Joystick *joystick, Uint8 i_joyAxis, Sint16 i_joyAxisValue) {
  m_type = XMK_JOYSTICKAXIS;
  m_joystick = joystick;
  m_joyAxis = i_joyAxis;
  m_joyAxisValue = i_joyAxisValue;
  m_joyButton = SDL_CONTROLLER_BUTTON_INVALID;
  m_repetition = 0;
}

XMKey::XMKey(const std::string &i_key, bool i_basicMode) {
  size_t pos;
  std::string v_current, v_rest;

  m_repetition = 0;
  m_joyButton = SDL_CONTROLLER_BUTTON_INVALID;

  if (i_basicMode) {
    m_type = XMK_KEYBOARD;

    if (i_key.length() != 1) {
      throw InvalidSystemKeyException();
    }
    m_keyboard_sym = (SDL_Keymod)((
      int)(tolower(i_key[0]))); // give ascii key while sdl does the same
    m_keyboard_mod = KMOD_NONE;
    return;
  }

  if (i_key == "N") {
    m_type = XMK_NONE;
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
    m_type = XMK_KEYBOARD;

    pos = v_rest.find(":", 0);
    if (pos == std::string::npos) {
      throw InvalidSystemKeyException();
    }
    v_current = v_rest.substr(0, pos);
    v_rest = v_rest.substr(pos + 1, v_rest.length() - pos - 1);

    m_keyboard_sym = (SDL_Keycode)std::strtoul(v_current.c_str(), nullptr, 0);
    m_keyboard_mod =
      (SDL_Keymod)(((SDL_Keymod)std::strtoul(v_rest.c_str(), nullptr, 0)) &
                   (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT |
                    KMOD_GUI)); // only allow these modifiers

  } else if (v_current == "M") { // mouse button
    m_type = XMK_MOUSEBUTTON;
    m_mouseButton_button = (Uint8)std::strtoul(v_rest.c_str(), nullptr, 0);

  } else if (v_current == "A") { // joystick axis
    m_type = XMK_JOYSTICKAXIS;

    // get the axis
    pos = v_rest.find(":", 0);
    if (pos == std::string::npos) {
      throw InvalidSystemKeyException();
    }
    v_current = v_rest.substr(0, pos);
    v_rest = v_rest.substr(pos + 1, v_rest.length() - pos - 1);
    m_joyAxis = (Uint8)std::strtoul(v_current.c_str(), nullptr, 0);

    // get the axis value
    pos = v_rest.find(":", 0);
    if (pos == std::string::npos) {
      throw InvalidSystemKeyException();
    }
    v_current = v_rest.substr(0, pos);
    v_rest = v_rest.substr(pos + 1, v_rest.length() - pos - 1);
    m_joyAxisValue = (Sint16)std::strtol(v_current.c_str(), nullptr, 0);

    SDL_JoystickID id =
      (SDL_JoystickID)std::strtoul(v_rest.c_str(), nullptr, 0);
    m_joystick = Input::instance()->getJoyById(id);
  } else if (v_current == "J") { // joystick button
    m_type = XMK_JOYSTICKBUTTON;

    pos = v_rest.find(":", 0);
    if (pos == std::string::npos) {
      throw Exception("Invalid key");
    }
    v_current = v_rest.substr(0, pos);
    v_rest = v_rest.substr(pos + 1, v_rest.length() - pos - 1);

    SDL_JoystickID id =
      (SDL_JoystickID)std::strtoul(v_rest.c_str(), nullptr, 0);
    m_joystick = Input::instance()->getJoyById(id);
    m_joyButton = (Uint8)std::strtoul(v_current.c_str(), nullptr, 0);
  } else {
    throw InvalidSystemKeyException();
  }
}

float XMKey::getAnalogValue() const {
  return JoystickInput::joyRawToFloat(
    m_joyAxisValue, (float)JOYSTICK_DEADZONE_BASE, (float)JOYSTICK_MAX_VALUE);
}

XMKey_direction XMKey::getDirection() const {
  if (JoystickInput::axisInside(m_joyAxisValue, JOYSTICK_DEADZONE_BASE))
    return XMKD_NODIRECTION;

  JoyDir dir = JoystickInput::getJoyAxisDir(m_joyAxisValue);

  if (m_joyAxis % 2 == 0) {
    return (dir < 0) ? XMKD_LEFT : XMKD_RIGHT;
  } else {
    return (dir < 0) ? XMKD_UP : XMKD_DOWN;
  }

  return XMKD_NODIRECTION;
}

bool XMKey::equalsIgnoreMods(const XMKey &i_other) const {
  // NOTE:
  // This function was temporarily split from `operator==(const XMKey &)` to fix
  // a specific bug with the way modifier keys are handled for the bike
  // controls. It should be merged back with it if/when the update code gets an
  // overhaul.

  if (m_type != i_other.m_type) {
    return false;
  }

  // The keyboard check is needed to keep old behavior
  // (before key repeat status was added)
  if (m_type != XMK_KEYBOARD && m_repetition != i_other.m_repetition) {
    return false;
  }

  if (m_type == XMK_NONE || i_other.m_type == XMK_NONE) {
    return false;
  }

  if (m_type == XMK_KEYBOARD) {
    return m_keyboard_sym == i_other.m_keyboard_sym;
  }

  if (m_type == XMK_MOUSEBUTTON) {
    return m_mouseButton_button == i_other.m_mouseButton_button;
  }

  if (m_type == XMK_JOYSTICKAXIS) {
    return (*m_joystick == *i_other.m_joystick) &&
           m_joyAxis == i_other.m_joyAxis &&
           !JoystickInput::axesOppose(
             m_joyAxis, i_other.m_joyAxis, JOYSTICK_DEADZONE_BASE);
  }

  if (m_type == XMK_JOYSTICKBUTTON) {
    return (*m_joystick == *i_other.m_joystick) &&
           m_joyButton == i_other.m_joyButton;
  }

  return false;
}

bool XMKey::operator==(const XMKey &i_other) const {
  bool equals = equalsIgnoreMods(i_other);

  if (m_type == XMK_KEYBOARD) {
    int mods = KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_GUI;
    return equals &&
           ((m_keyboard_mod & mods) == (i_other.m_keyboard_mod & mods));
  }

  return equals;
}

std::string XMKey::toString() const {
  std::ostringstream v_res;

  switch (m_type) {
    case XMK_KEYBOARD:
      v_res << "K:" << m_keyboard_sym << ":" << m_keyboard_mod;
      break;

    case XMK_MOUSEBUTTON:
      v_res << "M:" << ((int)m_mouseButton_button);
      break;

    case XMK_JOYSTICKAXIS:
      v_res << "A:" << ((int)m_joyAxis) << ":" << ((int)m_joyAxisValue) << ":"
            << m_joystick->id;
      break;

    case XMK_JOYSTICKBUTTON:
      v_res << "J:" << ((int)m_joyButton) << ":" << m_joystick->id;
      break;

    case XMK_NONE:
      v_res << "N";
      break;
  }

  return v_res.str();
}

std::string XMKey::toFancyString() const {
  std::ostringstream v_res;

  if (m_type == XMK_KEYBOARD) {
    std::string v_modifiers;

    static constexpr SDL_Keymod modKeys[] = {
      KMOD_LCTRL, KMOD_RCTRL, KMOD_LALT,   KMOD_RALT,
      KMOD_LGUI,  KMOD_RGUI,  KMOD_LSHIFT, KMOD_RSHIFT,
    };

    for (auto &mod : modKeys) {
      if (isModKeyDown(mod)) {
        v_modifiers += std::string(modKeyString(mod)) + " ";
      }
    }

    v_res << "[" << GAMETEXT_KEYBOARD << "] " << v_modifiers
          << SDL_GetKeyName(m_keyboard_sym);
  } else if (m_type == XMK_MOUSEBUTTON) {
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

      default:
        char v_button_tmp[256];
        snprintf(
          v_button_tmp, 256, GAMETEXT_MOUSE_BUTTON, m_mouseButton_button);
        v_button = v_button_tmp;
    }
    v_res << "[" << GAMETEXT_MOUSE << "] " << v_button;
  } else if (m_type == XMK_JOYSTICKAXIS || m_type == XMK_JOYSTICKBUTTON) {
    v_res << "[" << GAMETEXT_JOYSTICK << "] " << m_joystick->name
#if SDL_VERSION_ATLEAST(2, 0, 9)
          << " - player "
          << SDL_GameControllerGetPlayerIndex(m_joystick->handle)
#endif
          << " - ";

    if (m_type == XMK_JOYSTICKAXIS) {
      std::string v_direction;
      char v_axis_tmp[256];
      snprintf(v_axis_tmp, 256, GAMETEXT_JOYSTICK_AXIS, m_joyAxis);

      if (isDirectional()) {
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

      v_res << v_axis_tmp << v_direction;
    } else {
      char v_button[256];
      snprintf(v_button,
               256,
               GAMETEXT_JOYSTICK_BUTTON,
               m_joyButton + 1); // +1 because it starts at 0

      v_res << v_button;
    }
  } else if (m_type == XMK_NONE) {
    v_res << GAMETEXT_UNDEFINED;
  }

  return v_res.str();
}

const char *XMKey::modKeyString(SDL_Keymod modKey) {
  switch (modKey) {
    case KMOD_LCTRL:
      return GAMETEXT_KEY_LEFTCONTROL;
    case KMOD_RCTRL:
      return GAMETEXT_KEY_RIGHTCONTROL;
    case KMOD_LALT:
      return GAMETEXT_KEY_LEFTALT;
    case KMOD_RALT:
      return GAMETEXT_KEY_RIGHTALT;
    case KMOD_LGUI:
      return GAMETEXT_KEY_LEFTMETA;
    case KMOD_RGUI:
      return GAMETEXT_KEY_RIGHTMETA;
    case KMOD_LSHIFT:
      return GAMETEXT_KEY_LEFTSHIFT;
    case KMOD_RSHIFT:
      return GAMETEXT_KEY_RIGHTSHIFT;
    default:
      return "?";
  }
}

bool XMKey::isModKeyDown(SDL_Keymod modKey) const {
  return (m_keyboard_mod & modKey) == modKey;
}

bool XMKey::isPressed(const Uint8 *i_keystate,
                      Uint8 i_mousestate,
                      int numkeys) const {
  if (m_type == XMK_KEYBOARD) {
    SDL_Scancode scancode = SDL_GetScancodeFromKey(m_keyboard_sym);

    if (scancode >= numkeys)
      return false;

    return i_keystate[scancode] == 1;
  }

  if (m_type == XMK_MOUSEBUTTON) {
    return (i_mousestate & SDL_BUTTON(m_mouseButton_button)) ==
           SDL_BUTTON(m_mouseButton_button);
  }

  if (m_type == XMK_JOYSTICKAXIS) {
    Sint16 v_axisValue = SDL_GameControllerGetAxis(
      m_joystick->handle, (SDL_GameControllerAxis)m_joyAxis);

    return JoystickInput::axesMatch(
      v_axisValue, m_joyAxisValue, JOYSTICK_DEADZONE_BASE);
  }

  if (m_type == XMK_JOYSTICKBUTTON) {
    return SDL_GameControllerGetButton(
             m_joystick->handle, (SDL_GameControllerButton)m_joyButton) == 1;
  }

  return false;
}

bool XMKey::toKeyboard(SDL_Keycode &nKey,
                       SDL_Keymod &o_mod,
                       std::string &o_utf8Char) const {
  if (m_type != XMK_KEYBOARD) {
    return false;
  }

  nKey = m_keyboard_sym;
  o_mod = m_keyboard_mod;
  o_utf8Char = m_keyboard_utf8Char;

  return true;
}

bool XMKey::toMouse(int &nX, int &nY, Uint8 &nButton) const {
  if (m_type != XMK_MOUSEBUTTON) {
    return false;
  }

  GameApp::getMousePos(&nX, &nY);
  nButton = m_mouseButton_button;
  return true;
}

bool XMKey::toMouseWheel(int &nX,
                         int &nY,
                         Sint32 &wheelX,
                         Sint32 &wheelY) const {
  if (m_type != XMK_MOUSEWHEEL) {
    return false;
  }

  // used by UIRoot::_RootMouseEvent
  GameApp::getMousePos(&nX, &nY);

  wheelX = m_wheelX;
  wheelY = m_wheelY;

  return true;
}

bool XMKey::toJoystickButton(Uint8 &o_joyNum, Uint8 &o_joyButton) const {
  if (m_type != XMK_JOYSTICKBUTTON) {
    return false;
  }

  o_joyNum = Input::instance()->getJoyNum(*m_joystick);
  o_joyButton = m_joyButton;

  return true;
}

bool XMKey::toJoystickAxisMotion(JoyAxisEvent &event) const {
  if (m_type != XMK_JOYSTICKAXIS) {
    return false;
  }

  event.joystickNum = Input::instance()->getJoyNum(*m_joystick);
  event.axis = m_joyAxis;
  event.axisValue = m_joyAxisValue;

  return true;
}
