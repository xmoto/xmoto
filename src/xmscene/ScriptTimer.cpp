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

#include "ScriptTimer.h"
#include "xmoto/LuaLibGame.h"

/*===========================================================================*/
/* ScriptTimer Class                                                         */
/*===========================================================================*/
// initialize a new timer to function properly
ScriptTimer::ScriptTimer(const std::string &i_name,
                         int i_delay,
                         int i_loops,
                         LuaLibGame *i_Script,
                         int i_GameTime) {
  m_TimerName = i_name;
  m_TimeBetweenCalls = i_delay;
  m_Script = i_Script;
  m_TimeOfLastCall = i_GameTime;
  m_isRunning = true;
  m_Numbr_Of_Loops = i_loops;
  m_Numbr_Of_Calls = 1;
}

// ran in every loop
void ScriptTimer::UpdateTimer(int i_GameTime) {
  if (m_isRunning == true) { // only run if not paused
    if (i_GameTime >
        m_TimeOfLastCall + m_TimeBetweenCalls) { // time to call again
      m_TimeOfLastCall = i_GameTime; // update the timer
      m_Script->scriptCallTblVoid(
        m_TimerName, "Tick", m_Numbr_Of_Calls); // call the function as needed
      m_Numbr_Of_Calls++; // count loops
    }
  } else { // paused
    m_TimeOfLastCall = i_GameTime; // just update the timer
  }
}
// functions to pause and continue the timer
void ScriptTimer::PauseTimer() {
  m_isRunning = false;
}
void ScriptTimer::StartTimer() {
  m_isRunning = true;
}
std::string ScriptTimer::GetName() {
  return m_TimerName;
}
void ScriptTimer::SetTimerDelay(int i_delay) {
  m_TimeBetweenCalls = i_delay;
}
bool ScriptTimer::isFinished() {
  return m_Numbr_Of_Loops != 0 && m_Numbr_Of_Loops < m_Numbr_Of_Calls;
}
void ScriptTimer::ResetTimer(int i_delay, int i_loops, int i_GameTime) {
  m_TimeBetweenCalls = i_delay;
  m_TimeOfLastCall = i_GameTime;
  m_Numbr_Of_Loops = i_loops;
  m_Numbr_Of_Calls = 1;
  m_isRunning = true;
}
