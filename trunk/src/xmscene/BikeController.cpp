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

#include "BikeController.h"

BikeController::BikeController()
{
  m_brokenBreaks = false;
  stopContols();
}

void BikeController::stopContols() {
  m_drive      = 0.0;      
  m_pull       = 0.0;  
  m_changeDir  = false;
  m_throttle   = 0.0;
  m_break      = 0.0;
}

float BikeController::Drive() const {
  return m_drive;
}

float BikeController::Pull() const {
  return m_pull;
}

bool BikeController::ChangeDir() const {
  return m_changeDir;
}

void BikeController::setDrive(float i_drive)
{
  m_drive = i_drive;
}

void BikeController::setThrottle(float i_throttle)
{
  m_throttle = i_throttle;
  if(m_throttle > 0.0f){
    m_drive = m_throttle;
  }
  else if(m_throttle == 0.0f){
    if(m_break != 0.0f){
      m_drive = -m_break;
    } else {
      m_drive = 0.0f;
    }
  }
}

void BikeController::setBreak(float i_break)
{
  if(m_brokenBreaks) {
    return;
  }

  m_break = i_break;
  if(m_break > 0.0f){
    m_drive = -m_break;
  }
  else if(m_break == 0.0f){
    if(m_throttle != 0.0f){
      m_drive = m_throttle;
    } else {
      m_drive = 0.0f;
    }
  }
}

void BikeController::setPull(float i_pull) {
  m_pull = i_pull;
}

void BikeController::setChangeDir(bool i_changeDir) {
  m_changeDir = i_changeDir;
}

void BikeController::breakBreaks() {
  m_break = 0.0;
  m_brokenBreaks = true;
}
