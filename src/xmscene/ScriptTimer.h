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

#ifndef __SCRIPTTIMER_H__
#define __SCRIPTTIMER_H__

#include <string>

class LuaLibGame;
/*===========================================================================
  Script Timer Class
===========================================================================*/
class ScriptTimer {
public:
  ScriptTimer(const std::string &i_name,
              int i_delay,
              int i_loops,
              LuaLibGame *i_Script,
              int i_GameTime);
  void UpdateTimer(int i_GameTime);
  void PauseTimer();
  void StartTimer();
  void SetTimerDelay(int i_delay);
  std::string GetName();
  bool isFinished();
  void ResetTimer(int i_delay, int i_loops, int i_GameTime);

private:
  int m_TimeBetweenCalls;
  int m_TimeOfLastCall;
  LuaLibGame *m_Script;
  std::string m_TimerName;
  bool m_isRunning;
  int m_Numbr_Of_Loops;
  int m_Numbr_Of_Calls;
};

#endif
