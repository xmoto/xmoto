#include "Joystick.h"
#include "Input.h"
#include "helpers/VMath.h"

void JoystickInput::init(int8_t numJoysticks) {
  for (uint8_t i = 0; i < numJoysticks; i++)
    m_joyAxes.push_back(JoyAxes());
}

/**
 * converts a raw joystick axis value to a float, according to specified minimum
 * and maximum values, as well as the deadzone.
 *
 *                 (+)      ____
 *           result |      /|
 *                  |     / |
 *                  |    /  |
 *  (-)________ ____|___/___|____(+)
 *             /|   |   |   |    input
 *            / |   |   |   |
 *           /  |   |   |   |
 *     _____/   |   |   |   |
 *          |   |  (-)  |   |
 *         neg  dead-zone  pos
 *
 */
float JoystickInput::joyRawToFloat(float raw, float max, float deadzone) {
  if (raw > max)
    return +1.0f;
  if (raw < -max)
    return -1.0f;
  if (raw > deadzone)
    return +((raw - deadzone) / (max - deadzone));
  if (raw < -deadzone)
    return -((raw + deadzone) / (-max + deadzone));

  return 0.0f;
}

uint32_t JoystickInput::repeatTimerCallback(uint32_t interval, void *state) {
  SDL_Event event = {};
  SDL_UserEvent userEvent = {};

  userEvent.type = SDL_USEREVENT;

  userEvent.code = SDL_CONTROLLERAXISMOTION;
  userEvent.data1 = state;

  event.type = SDL_USEREVENT;
  event.user = userEvent;

  SDL_PushEvent(&event);

  return JOYSTICK_REPEAT_RATE_HZ;
}

void JoystickInput::resetJoyAxis(JoyAxis &axis) {
  clearRepeatTimer(axis.repeatTimer);
  axis.isHeld = false;
  axis.dir = 0;
}

void JoystickInput::resetJoyAxes() {
  for (auto &axes : m_joyAxes)
    for (auto &axis : axes)
      resetJoyAxis(axis);
}

void JoystickInput::clearRepeatTimer(SDL_TimerID &timer) {
  if (timer) {
    SDL_RemoveTimer(timer);
    timer = 0;
  }
}

bool JoystickInput::joyAxisRepeat(JoyAxisEvent event) {
  if (event.axis == (uint8_t)SDL_CONTROLLER_AXIS_INVALID)
    return false;

  auto &axis = getAxesByJoyIndex(event.joystickNum)[event.axis];

  if (axisInside(event.axisValue, JOYSTICK_DEADZONE_MENU)) {
    axis.lastDir = axis.dir;
    resetJoyAxis(axis);
    return false;
  }

  JoyDir dir = getJoyAxisDir(event.axisValue);
  bool dirSame = dir != 0 && std::abs(dir) == std::abs(axis.lastDir);

  if (!dirSame && axis.isHeld)
    return false;

  for (auto &axes : m_joyAxes)
    for (auto &axis : axes)
      clearRepeatTimer(axis.repeatTimer);

  m_joystickRepeat = event;

  axis.isHeld = true;
  axis.repeatTimer = SDL_AddTimer(
    JOYSTICK_REPEAT_DELAY_MS, repeatTimerCallback, &m_joystickRepeat);
  axis.lastDir = axis.dir;
  axis.dir = dir;

  return true;
}
