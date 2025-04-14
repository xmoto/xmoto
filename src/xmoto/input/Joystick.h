#ifndef __INPUT_JOYSTICK_H__
#define __INPUT_JOYSTICK_H__

#include "helpers/Singleton.h"
#include "helpers/VMath.h"
#include "include/xm_SDL.h"
#include <array>
#include <stdint.h>
#include <string>
#include <vector>

static const uint32_t JOYSTICK_REPEAT_DELAY_MS = 500;
static const float JOYSTICK_REPEAT_RATE_HZ = 1000 / 33.0f;

static const int32_t JOYSTICK_MAX_VALUE = 32760;
static const int32_t JOYSTICK_DEADZONE_BASE = JOYSTICK_MAX_VALUE / 4;
static const int32_t JOYSTICK_DEADZONE_MENU = JOYSTICK_MAX_VALUE / 2;

using JoyDir = int8_t;

struct Joystick {
  SDL_GameController *handle;
  std::string name;
  SDL_JoystickID id;

  bool operator==(const Joystick &other) const {
    return handle == other.handle && name == other.name && id == other.id;
  }
};

struct JoyAxisEvent {
  uint8_t joystickNum;
  uint8_t axis;
  int16_t axisValue;
};

struct JoyAxis {
  bool isHeld;
  SDL_TimerID repeatTimer;
  JoyDir dir;
  JoyDir lastDir;
};

class JoystickInput : public Singleton<JoystickInput> {
  friend class Singleton<JoystickInput>;

private:
  JoystickInput() {}
  ~JoystickInput() {}

public:
  void init(int8_t numJoysticks);

  static inline bool axesEqual(int16_t a, int16_t b) {
    return signum(a) == signum(b);
  }

  static inline bool axisOutside(int16_t axis, int32_t deadzone) {
    return abs(axis) > abs(deadzone);
  }

  static inline bool axisInside(int16_t axis, int32_t deadzone) {
    return !axisOutside(axis, deadzone);
  }

  static inline bool axesOppose(int16_t a, int16_t b, int32_t deadzone) {
    return !axesEqual(a, b) && axisOutside(a, deadzone);
  }

  static inline bool axesMatch(int16_t a, int16_t b, int32_t deadzone) {
    return axesEqual(a, b) && axisOutside(a, deadzone);
  }

  static JoyDir getJoyAxisDir(int16_t axisValue) { return signum(axisValue); }

  static float joyRawToFloat(float raw, float max, float deadzone);

  static uint32_t repeatTimerCallback(uint32_t interval, void *state);

  bool joyAxisRepeat(JoyAxisEvent event);
  void clearRepeatTimer(SDL_TimerID &timer);

  void resetJoyAxis(JoyAxis &axis);
  void resetJoyAxes();

private:
  using JoyAxes = std::array<JoyAxis, SDL_CONTROLLER_AXIS_MAX>;
  std::vector<JoyAxes> m_joyAxes;
  JoyAxisEvent m_joystickRepeat;

  JoyAxes &getAxesByJoyIndex(uint8_t index) { return m_joyAxes[index]; }
};

#endif
