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

#define INPUT_JOYSTICK_MINIMUM_DETECTION 100
#define INPUT_JOYSTICK_MAXIMUM_VALUE 32760

enum XMKey_input {
  XMK_NONE,
  XMK_KEYBOARD,
  XMK_MOUSEBUTTON,
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
enum InputEventType { INPUT_DOWN, INPUT_UP };

#include "include/xm_SDL.h"
#include <string>

/* define a key to do something (keyboard:a, mouse:left, ...) */
class XMKey {
public:
  XMKey();
  XMKey(SDL_Event &i_event);
  XMKey(const std::string &i_key,
        bool i_basicMode =
          false); /* basic mode is to give a simple letter, for scripts key */
  XMKey(SDLKey nKey,
        SDLMod mod,
        const std::string &i_utf8Char = ""); // keyboard
  XMKey(Uint8 nButton, unsigned int i_repetition = 1); // mouse
  XMKey(std::string *i_joyId, Uint8 i_joyButton); // joystick button
  XMKey(std::string *i_joyId,
        Uint8 i_joyAxis,
        Sint16 i_joyAxisValue); // joystick axis

  bool operator==(const XMKey &i_other) const;
  std::string toString() const;
  std::string toFancyString() const;
  bool isPressed(Uint8 *i_keystate, Uint8 i_mousestate) const;

  bool isDefined() const;

  unsigned int getRepetition() const;

  bool isAnalogic() const;
  float getAnalogicValue() const;
  bool isDirectionnel() const;
  XMKey_direction getDirection() const;

  bool toKeyboard(SDLKey &nKey, SDLMod &o_mod, std::string &o_utf8Char) const;
  bool toMouse(int &nX, int &nY, Uint8 &nButton) const;
  bool toJoystickButton(Uint8 &o_joyNum, Uint8 &o_joyButton) const;
  bool toJoystickAxisMotion(Uint8 &o_joyNum,
                            Uint8 &o_joyAxis,
                            Sint16 &o_joyAxisValue) const;

private:
  XMKey_input m_input;
  SDLKey m_keyboard_sym;
  SDLMod m_keyboard_mod;
  std::string m_keyboard_utf8Char;
  Uint8 m_mouseButton_button;
  std::string *m_joyId; // a pointer to avoid the copy while joyId are store
  // from load to unload
  Uint8 m_joyButton;
  Uint8 m_joyAxis;
  Sint16 m_joyAxisValue;
  unsigned int m_repetition;
};

#endif
