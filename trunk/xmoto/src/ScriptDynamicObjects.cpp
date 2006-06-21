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
  if(m_time >= m_endTime && m_endTime != 0.0) {
    return false;
  }

  if(isTimeToMove() == true) {
    performMove(v_motoGame);
  }

  m_time++;
  return true;
}

bool SDynamicObject::isTimeToMove() {
  return m_time >= m_startTime && (m_time <= m_endTime || m_endTime == 0.0);
}

SDynamicEntityMove::SDynamicEntityMove(std::string pEntity, int p_startTime, int p_endTime) : SDynamicObject(p_startTime, p_endTime){
  m_entity = pEntity;
  m_previousVx = 0.0;
  m_previousVy = 0.0;
}

SDynamicEntityMove::~SDynamicEntityMove() {
}

void SDynamicEntityMove::performMove(vapp::MotoGame* v_motoGame) {
  /* Find the specified entity and return its position */
  for(int i=0;i<v_motoGame->getEntities().size();i++) {
    vapp::Entity *p = v_motoGame->getEntities()[i];
    if(p->ID == m_entity) {
      float vx, vy;
      performXY(&vx, &vy);

      v_motoGame->SetEntityPos(p->ID,
			       vx + p->Pos.x - m_previousVx,
			       vy + p->Pos.y - m_previousVy);
      m_previousVx = vx;
      m_previousVy = vy;
    }
  }
}

SDynamicEntityRotation::SDynamicEntityRotation(std::string pEntity, float pInitAngle, float pRadius, float pSpeed, int p_startTime, int p_endTime) : SDynamicEntityMove(pEntity, p_startTime, p_endTime) {
  m_Angle   = pInitAngle;
  m_Radius  = pRadius;
  m_Speed   = pSpeed;

  m_CenterX = cos(m_Angle) * m_Radius;
  m_CenterY = sin(m_Angle) * m_Radius;
}

SDynamicEntityRotation::~SDynamicEntityRotation() {
}

void SDynamicEntityRotation::performXY(float *vx, float *vy) {
  if(m_Angle >= 2 * M_PI) {m_Angle -= 2 * M_PI;} /* because of float limit */
  
  *vx = cos(m_Angle) * m_Radius - m_CenterX;
  *vy = sin(m_Angle) * m_Radius - m_CenterY;

  m_Angle += m_Speed;
}

SDynamicEntityTranslation::SDynamicEntityTranslation(std::string pEntity, float pX, float pY, float pSpeed, int p_startTime, int p_endTime) : SDynamicEntityMove(pEntity, p_startTime, p_endTime) {
  float m_Z;

  m_X 	  = pX;
  m_Y 	  = pY;
  m_Speed = pSpeed;

  m_sensUp = true;
  m_Z      = m_X * m_X + m_Y * m_Y; 
  m_moveX  = (m_Speed * m_X) / m_Z;
  m_moveY  = (m_Speed * m_Y) / m_Z;
  m_totalMoveX = 0.0;
}

SDynamicEntityTranslation::~SDynamicEntityTranslation() {
}

void SDynamicEntityTranslation::performXY(float *vx, float *vy) {
  *vx = m_sensUp ? m_moveX : -m_moveX;
  *vy = m_sensUp ? m_moveY : -m_moveY;

  m_totalMoveX += *vx;

  if(m_totalMoveX >= m_X) {
    m_sensUp = !m_sensUp;
  }
    
  if(m_totalMoveX <= 0) {
    m_sensUp = !m_sensUp;
  }
}

