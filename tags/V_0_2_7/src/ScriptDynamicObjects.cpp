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

bool SDynamicObject::nextState(vapp::MotoGame* v_motoGame, int i_nbCents) {
  int v_realNbCents;

  v_realNbCents = i_nbCents;

  if(m_time >= m_endTime && m_endTime != 0.0) {
    return false;
  }

  if(m_startTime > m_time) {
    v_realNbCents -= m_startTime - m_time;
  }

  if(m_time + i_nbCents > m_endTime && m_endTime != 0.0) {
    v_realNbCents -= m_time + i_nbCents - m_endTime;
  }

  performMove(v_motoGame, v_realNbCents);

  m_time += i_nbCents;
  if(m_time >= m_endTime && m_endTime != 0.0) {
    return false;
  }

  return true;
}

bool SDynamicObject::isTimeToMove() {
  return m_time >= m_startTime && (m_time <= m_endTime || m_endTime == 0.0);
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

SDynamicRotation::SDynamicRotation(float pInitAngle, float pRadius, float
pPeriod) {
  m_Speed = 0.0;
  if(pPeriod != 0.0) {
    m_Speed = (2 * M_PI) / pPeriod;
  }

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

  m_X     = pX;
  m_Y     = pY;

  m_sensUp = true;
  m_Z      = sqrt(m_X * m_X + m_Y * m_Y);

  m_Speed  = 0.0;
  if(pPeriod != 0.0) {
    m_Speed  = (m_Z * 2) / pPeriod;
  }

  m_moveX = 0.0;
  m_moveY = 0.0;
  if(m_Z != 0.0) {
    m_moveX  = (m_Speed * m_X) / m_Z;
    m_moveY  = (m_Speed * m_Y) / m_Z;
  }

  m_totalMoveX = 0.0;
  m_totalMoveY = 0.0;
}

SDynamicTranslation::~SDynamicTranslation() {
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

/* entity */

SDynamicEntityMove::SDynamicEntityMove(std::string pEntity, int p_startTime, int p_endTime, float pPeriod) : SDynamicObject(p_startTime, p_endTime, pPeriod){
  m_entity = pEntity;
}

SDynamicEntityMove::~SDynamicEntityMove() {
}

void SDynamicEntityMove::performMove(vapp::MotoGame* v_motoGame, int i_nbCents) {
  Entity* p = &(v_motoGame->getLevelSrc()->getEntityById(m_entity));
  if(! p->isAlive()){
    return;
  }
  float vx, vy;
  float addvx = 0.0, addvy = 0.0;

  if(i_nbCents > 0) {
    for(int i=0; i<i_nbCents; i++) {
      performXY(&vx, &vy);
      addvx += vx;
      addvy += vy;
    }
    v_motoGame->SetEntityPos(p,
			     addvx + p->DynamicPosition().x,
			     addvy + p->DynamicPosition().y);
  }
}

std::string SDynamicEntityMove::getObjectId() {
  return m_entity;
}

SDynamicEntityRotation::SDynamicEntityRotation(std::string pEntity, float pInitAngle, float pRadius, float pPeriod, int p_startTime, int p_endTime) : SDynamicEntityMove(pEntity, p_startTime, p_endTime, pPeriod), SDynamicRotation(pInitAngle, pRadius, pPeriod) {
}

SDynamicEntityRotation::~SDynamicEntityRotation() {
}

SDynamicEntityTranslation::SDynamicEntityTranslation(std::string pEntity, float pX, float pY, float pPeriod, int p_startTime, int p_endTime) : SDynamicEntityMove(pEntity, p_startTime, p_endTime, pPeriod), SDynamicTranslation(pX, pY, pPeriod) {
}

SDynamicEntityTranslation::~SDynamicEntityTranslation() {
}

void SDynamicEntityRotation::performXY(float *vx, float *vy) {
  SDynamicRotation::performXY(vx, vy);
}

void SDynamicEntityTranslation::performXY(float *vx, float *vy) {
  SDynamicTranslation::performXY(vx, vy);
}

/* block */

SDynamicBlockMove::SDynamicBlockMove(std::string pBlock, int p_startTime, int p_endTime, float pPeriod) : SDynamicObject(p_startTime, p_endTime, pPeriod){
  m_block = pBlock;
}

SDynamicBlockMove::~SDynamicBlockMove() {
}

void SDynamicBlockMove::performMove(vapp::MotoGame* v_motoGame, int i_nbCents) {
  Block *p = &(v_motoGame->getLevelSrc()->getBlockById(m_block));
  float vx, vy;
  float addvx = 0.0, addvy = 0.0;

  if(i_nbCents > 0) {
    for(int i=0; i<i_nbCents; i++) {
      performXY(&vx, &vy);
      addvx += vx;
      addvy += vy;
    }
    v_motoGame->SetBlockPos(p->Id(),
			    addvx + p->DynamicPosition().x,
			    addvy + p->DynamicPosition().y);
  }
}

std::string SDynamicBlockMove::getObjectId() {
  return m_block;
}

SDynamicBlockRotation::SDynamicBlockRotation(std::string pBlock, float pInitAngle, float pRadius, float pPeriod, int p_startTime, int p_endTime) : SDynamicBlockMove(pBlock, p_startTime, p_endTime, pPeriod), SDynamicRotation(pInitAngle, pRadius, pPeriod) {
}

SDynamicBlockRotation::~SDynamicBlockRotation() {
}

SDynamicBlockTranslation::SDynamicBlockTranslation(std::string pBlock, float pX, float pY, float pPeriod, int p_startTime, int p_endTime) : SDynamicBlockMove(pBlock, p_startTime, p_endTime, pPeriod), SDynamicTranslation(pX, pY, pPeriod) {
}

SDynamicBlockTranslation::~SDynamicBlockTranslation() {
}

void SDynamicBlockRotation::performXY(float *vx, float *vy) {
  SDynamicRotation::performXY(vx, vy);
}

void SDynamicBlockTranslation::performXY(float *vx, float *vy) {
  SDynamicTranslation::performXY(vx, vy);
}
