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

#include "include/xm_SDL.h"
#include <string>
#include <stdint.h>

static const int32_t INPUT_JOYSTICK_MAX_VALUE = 32760;
static const int32_t INPUT_JOYSTICK_DEADZONE_BASE = INPUT_JOYSTICK_MAX_VALUE / 4;
static const int32_t INPUT_JOYSTICK_DEADZONE_MENU = INPUT_JOYSTICK_MAX_VALUE / 2;

struct JoyAxisEvent {
  Uint8 joystickNum;
  Uint8 axis;
  Sint16 axisValue;
};

struct JoyAxis {
  bool isHeld;
  SDL_TimerID repeatTimer;
  int8_t dir;
  int8_t lastDir;
};

bool isAxisTrigger(Uint8 axis);
bool isAxisInsideDeadzone(Uint8 axis, Sint16 value);
int32_t getAxisDeadzone(Uint8 axis);

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
};

/* define a key to do something (keyboard:a, mouse:left, ...) */
class XMKey {
public:
  XMKey();
  XMKey(SDL_Event &i_event);
  XMKey(const std::string &i_key,
        bool i_basicMode = false); /* basic mode is to give a simple letter, for scripts key */
  XMKey(SDL_Keycode nKey,
        SDL_Keymod mod,
        const std::string &i_utf8Char = ""); // keyboard
  XMKey(Uint8 nButton, unsigned int i_repetition = 1); // mouse
  XMKey(std::string *i_joyId, Uint8 i_joyButton); // joystick button
  XMKey(std::string *i_joyId,
        Uint8 i_joyAxis,
        Sint16 i_joyAxisValue); // joystick axis

  bool operator==(const XMKey &i_other) const;
  std::string toString() const;
  std::string toFancyString() const;
  bool isPressed(const Uint8 *i_keystate, Uint8 i_mousestate) const;

  bool isDefined() const;

  unsigned int getRepetition() const;
  inline bool isBound() const { return m_input != XMK_NONE; }

  bool isAnalog() const;
  float getAnalogValue() const;
  bool isDirectional() const;
  XMKey_direction getDirection() const;

  bool toKeyboard(SDL_Keycode &nKey, SDL_Keymod &o_mod, std::string &o_utf8Char) const;
  bool toMouse(int &nX, int &nY, Uint8 &nButton) const;
  bool toMouseWheel(int &nX, int &nY, Sint32 &wheelX, Sint32 &wheelY) const;
  bool toJoystickButton(Uint8 &o_joyNum, Uint8 &o_joyButton) const;
  bool toJoystickAxisMotion(JoyAxisEvent &event) const;

  inline SDL_Keycode getKeyboardSym() const { return m_keyboard_sym; }
  inline SDL_Keymod  getKeyboardMod() const { return m_keyboard_mod; }

  inline Uint8 getJoyButton() const { return m_joyButton; }

private:
  XMKey_input m_input;
  SDL_Keycode m_keyboard_sym;
  SDL_Keymod m_keyboard_mod;
  std::string m_keyboard_utf8Char;
  Uint8 m_mouseButton_button;
  Sint32 m_wheelX, m_wheelY;

  // a pointer to avoid the copy while joyId are stored from load to unload
  std::string *m_joyId;
  Uint8 m_joyButton;
  Uint8 m_joyAxis;
  Sint16 m_joyAxisValue;
  unsigned int m_repetition;
};

#endif
