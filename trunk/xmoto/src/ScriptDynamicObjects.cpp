/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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

#include "ScriptDynamicObjects.h"
#include "MotoGame.h"
#include "math.h"

SDynamicObject::SDynamicObject(int p_startTime, int p_endTime) {
  m_time = 0;
  m_startTime = p_startTime;
  m_endTime   = p_endTime;
}

SDynamicObject::~SDynamicObject() {
}

bool SDynamicObject::nextState(vapp::MotoGame* v_motoGame) {
  if(m_time == m_endTime) {
    return false;
  }
  m_time++;
  return true;
}

bool SDynamicObject::isTimeToMove() {
  return m_time >= m_startTime && m_time <= m_endTime;
}

SDynamicEntityRotation::SDynamicEntityRotation(std::string pEntity, float pCenterX, float pCenterY, float pInitAngle, float pRadius, float pSpeed, int p_startTime, int p_endTime) : SDynamicObject(p_startTime, p_endTime) {
  m_entity = pEntity;
  m_CenterX = pCenterX;
  m_CenterY = pCenterY;
  m_Angle   = pInitAngle;
  m_Radius  = pRadius;
  m_Speed   = pSpeed;
}

SDynamicEntityRotation::~SDynamicEntityRotation() {
}

bool SDynamicEntityRotation::nextState(vapp::MotoGame* v_motoGame) {
  if(SDynamicObject::nextState(v_motoGame) == false) {
    return false;
  }

  if(isTimeToMove() == false) {
    return true;
  }

  float x,y;

  m_Angle += m_Speed;
  if(m_Angle >= 360) {m_Angle -= 360.0;} /* because of float limit */

  x = cos(m_Angle) * m_Radius + m_CenterX;
  y = sin(m_Angle) * m_Radius + m_CenterY;

  v_motoGame->SetEntityPos(m_entity, x, y);

  return true;
}
