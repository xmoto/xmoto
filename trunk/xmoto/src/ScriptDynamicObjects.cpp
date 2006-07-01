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

SDynamicObject::SDynamicObject(int p_startTime, int p_endTime, float pPeriod) {
  m_time = 0;
  m_startTime = p_startTime;
  m_endTime   = p_endTime;
  m_period    = pPeriod;
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

SDynamicEntityMove::SDynamicEntityMove(std::string pEntity, int p_startTime, int p_endTime, float pPeriod) : SDynamicObject(p_startTime, p_endTime, pPeriod){
  m_entity = pEntity;
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
			       vx + p->Pos.x,
			       vy + p->Pos.y);
    }
  }
}

std::string SDynamicEntityMove::getObjectId() {
  return m_entity;
}

SDynamicEntityRotation::SDynamicEntityRotation(std::string pEntity, float pInitAngle, float pRadius, float pPeriod, int p_startTime, int p_endTime) : SDynamicEntityMove(pEntity, p_startTime, p_endTime, pPeriod), SDynamicRotation(pInitAngle, pRadius, pPeriod) {
}

SDynamicEntityRotation::~SDynamicEntityRotation() {
}

void SDynamicRotation::performXY(float *vx, float *vy) {
  if(m_Angle >= 2 * M_PI) {m_Angle -= 2 * M_PI;} /* because of float limit */
  float x,y;

  x = cos(m_Angle) * m_Radius - m_CenterX;
  y = sin(m_Angle) * m_Radius - m_CenterY;
  *vx = x - m_previousVx;
  *vy = y - m_previousVy;

  m_previousVx = x;
  m_previousVy = y;
  m_Angle += m_Speed;
}

SDynamicEntityTranslation::SDynamicEntityTranslation(std::string pEntity, float pX, float pY, float pPeriod, int p_startTime, int p_endTime) : SDynamicEntityMove(pEntity, p_startTime, p_endTime, pPeriod), SDynamicTranslation(pX, pY, pPeriod) {
}

SDynamicEntityTranslation::~SDynamicEntityTranslation() {
}

void SDynamicTranslation::performXY(float *vx, float *vy) {
  *vx = m_sensUp ? m_moveX : -m_moveX;
  *vy = m_sensUp ? m_moveY : -m_moveY;

  m_totalMoveX += m_sensUp ? fabs(*vx) : -fabs(*vx);
  m_totalMoveY += m_sensUp ? fabs(*vy) : -fabs(*vy);

  if(
     (m_totalMoveX < 0 || m_totalMoveX > fabs(m_X)) ||
     (m_totalMoveY < 0 || m_totalMoveY > fabs(m_Y))
     ) {
    m_sensUp = !m_sensUp;
  }
}

SDynamicRotation::SDynamicRotation(float pInitAngle, float pRadius, float pPeriod) {
  m_Speed = (2 * M_PI) / pPeriod;

  m_Angle   = pInitAngle;
  m_Radius  = pRadius;

  m_CenterX = cos(m_Angle) * m_Radius;
  m_CenterY = sin(m_Angle) * m_Radius;
  m_previousVx = 0.0;
  m_previousVy = 0.0;
}

SDynamicRotation::~SDynamicRotation() {
}

SDynamicTranslation::SDynamicTranslation(float pX, float pY, float pPeriod) {
  float m_Z;

  m_X 	  = pX;
  m_Y 	  = pY;

  m_sensUp = true;
  m_Z      = sqrt(m_X * m_X + m_Y * m_Y); 
  m_Speed  = (m_Z * 2) / pPeriod;
  m_moveX  = (m_Speed * m_X) / m_Z;
  m_moveY  = (m_Speed * m_Y) / m_Z;
  
  m_totalMoveX = 0.0;
  m_totalMoveY = 0.0;
}

SDynamicTranslation::~SDynamicTranslation() {
}

void SDynamicEntityRotation::performXY(float *vx, float *vy) {
  SDynamicRotation::performXY(vx, vy);
}

void SDynamicEntityTranslation::performXY(float *vx, float *vy) {
  SDynamicTranslation::performXY(vx, vy);
}
