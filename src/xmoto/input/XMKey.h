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

#ifndef __XMKEY_H__
#define __XMKEY_H__

#include "Joystick.h"
#include "include/xm_SDL.h"
#include <stdint.h>
#include <string>

enum XMKey_input {
  XMK_NONE,
  XMK_KEYBOARD,
  XMK_MOUSEBUTTON,
  XMK_MOUSEWHEEL,
  XMK_JOYSTICKBUTTON,
  XMK_JOYSTICKAXIS
};
enum XMKey_direction {
  XMKD_LEFT,
  XMKD_RIGHT,
  XMKD_UP,
  XMKD_DOWN,
  XMKD_NODIRECTION
};
enum InputEventType {
  INPUT_DOWN,
  INPUT_UP,
  INPUT_SCROLL,
  INPUT_TEXT,
  INPUT_INVALID
};

/* define a key to do something (keyboard:a, mouse:left, ...) */
class XMKey {
public:
  XMKey();
  XMKey(SDL_Event &i_event);
  XMKey(const std::string &i_key,
        bool i_basicMode =
          false); /* basic mode is to give a simple letter, for scripts key */
  XMKey(SDL_Keycode nKey,
        SDL_Keymod mod,
        const std::string &i_utf8Char = "",
        int repetition = 0); // keyboard
  XMKey(Uint8 nButton, unsigned int i_repetition = 0); // mouse
  XMKey(Joystick *joystick, Uint8 i_joyButton); // joystick button
  XMKey(Joystick *joystick,
        Uint8 i_joyAxis,
        Sint16 i_joyAxisValue); // joystick axis

  bool operator==(const XMKey &i_other) const;
  bool equalsIgnoreMods(const XMKey &i_other) const;
  std::string toString() const;
  std::string toFancyString() const;
  static const char *modKeyString(SDL_Keymod modKey);
  bool isModKeyDown(SDL_Keymod modKey) const;
  bool isPressed(const Uint8 *i_keystate,
                 Uint8 i_mousestate,
                 int numkeys) const;

  inline bool isDefined() const { return m_type != XMK_NONE; }

  inline unsigned int getRepetition() const { return m_repetition; }

  inline bool isAnalog() const { return m_type == XMK_JOYSTICKAXIS; }
  inline bool isDirectional() const { return m_type == XMK_JOYSTICKAXIS; };
  float getAnalogValue() const;
  XMKey_direction getDirection() const;

  bool toKeyboard(SDL_Keycode &nKey,
                  SDL_Keymod &o_mod,
                  std::string &o_utf8Char) const;
  bool toMouse(int &nX, int &nY, Uint8 &nButton) const;
  bool toMouseWheel(int &nX, int &nY, Sint32 &wheelX, Sint32 &wheelY) const;
  bool toJoystickButton(Uint8 &o_joyNum, Uint8 &o_joyButton) const;
  bool toJoystickAxisMotion(JoyAxisEvent &event) const;

  inline SDL_Keycode getKeyboardSym() const { return m_keyboard_sym; }
  inline SDL_Keymod getKeyboardMod() const { return m_keyboard_mod; }

  inline Uint8 getJoyButton() const { return m_joyButton; }

private:
  XMKey_input m_type;
  SDL_Keycode m_keyboard_sym;
  SDL_Keymod m_keyboard_mod;
  std::string m_keyboard_utf8Char;
  Uint8 m_mouseButton_button;
  Sint32 m_wheelX, m_wheelY;

  Joystick *m_joystick;
  Uint8 m_joyButton;
  Uint8 m_joyAxis;
  Sint16 m_joyAxisValue;
  unsigned int m_repetition;
};

#endif
