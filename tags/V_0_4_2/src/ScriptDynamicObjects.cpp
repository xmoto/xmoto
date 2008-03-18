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

#include "ScriptDynamicObjects.h"
#include "xmscene/Scene.h"
#include "math.h"
#include "xmscene/Level.h"
#include "xmscene/Entity.h"
#include "xmscene/Block.h"

SDynamicObject::SDynamicObject(int p_startTime, int p_endTime, int pPeriod) {
  m_time = 0;
  m_startTime = p_startTime;
  m_endTime   = p_endTime;
  m_period    = pPeriod;
}

SDynamicObject::~SDynamicObject() {
}

bool SDynamicObject::nextState(MotoGame* v_motoGame, int i_nbCents) {
  int v_realNbCents;

  v_realNbCents = i_nbCents;

  if(m_time >= m_endTime && m_endTime != 0) {
    return false;
  }

  if(m_startTime > m_time) {
    v_realNbCents -= m_startTime - m_time;
  }

  if(m_time + i_nbCents > m_endTime && m_endTime != 0) {
    v_realNbCents -= m_time + i_nbCents - m_endTime;
  }
  
  performMove(v_motoGame, v_realNbCents);

  m_time += i_nbCents;
  if(m_time >= m_endTime && m_endTime != 0) {
    return false;
  }

  return true;
}

bool SDynamicObject::isTimeToMove() {
  return m_time >= m_startTime && (m_time <= m_endTime || m_endTime == 0);
}

int SDynamicObject::Period() const {
  return m_period;
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

SDynamicRotation::SDynamicRotation(float pInitAngle, float pRadius, int
pPeriod) {
  m_Speed = 0.0;
  if(pPeriod != 0) {
    m_Speed = (2 * M_PI) / ((float)(pPeriod));
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

SDynamicTranslation::SDynamicTranslation(float pX, float pY, int pPeriod) {
  float m_Z;

  m_X     = pX;
  m_Y     = pY;

  m_sensUp = true;
  m_Z      = sqrt(m_X * m_X + m_Y * m_Y);

  m_Speed  = 0.0;
  if(pPeriod != 0) {
    m_Speed  = (m_Z * 2) / ((float)(pPeriod));
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

SDynamicSelfRotation::SDynamicSelfRotation(int i_period) {
  m_period = i_period;
  m_totalAngle = 0.0;
  m_incr = 0;
}

SDynamicSelfRotation::~SDynamicSelfRotation() {
}

void SDynamicSelfRotation::performXY(float *vAngle) {
  *vAngle = (2 * M_PI) / ((float) m_period);
  m_incr++;
  m_totalAngle += *vAngle;

  /* to limit sum error */
  if( (m_period > 0 && m_incr >= m_period) ||
      (m_period < 0 && m_incr >= -m_period) ) {
    float v_realSum = (((float) m_incr) * (2 * M_PI)) / ((float) m_period);
    *vAngle += v_realSum - m_totalAngle;
    m_incr = 0;
    m_totalAngle = 0.0;
  }
}

/* entity */

SDynamicEntityMove::SDynamicEntityMove(std::string pEntity, int p_startTime, int p_endTime, int pPeriod) : SDynamicObject(p_startTime, p_endTime, pPeriod){
  m_entity = pEntity;
}

SDynamicEntityMove::~SDynamicEntityMove() {
}

void SDynamicEntityMove::performMove(MotoGame* v_motoGame, int i_nbCents) {
  Entity* p = v_motoGame->getLevelSrc()->getEntityById(m_entity);
  if(! p->isAlive()){
    return;
  }
  float vx, vy, vAngle;
  float addvx = 0.0, addvy = 0.0, addvAngle = 0.0;

  if(i_nbCents > 0) {
    for(int i=0; i<i_nbCents; i++) {
      performXY(&vx, &vy, &vAngle);
      addvx += vx;
      addvy += vy;
      addvAngle = vAngle;
    }

    if(addvx != 0.0 || addvy != 0) { /* a simple fast test because it's probably the main case */
      v_motoGame->SetEntityPos(p,
			       addvx + p->DynamicPosition().x,
			       addvy + p->DynamicPosition().y);
    }

    if(vAngle != 0.0) { /* a simple fast test because it's probably the main case */
      v_motoGame->SetEntityDrawAngle(p->Id(), addvAngle + p->DrawAngle());
    }
  }
}

std::string SDynamicEntityMove::getObjectId() {
  return m_entity;
}

SDynamicEntityRotation::SDynamicEntityRotation(std::string pEntity, float pInitAngle, float pRadius, int pPeriod, int p_startTime, int p_endTime)
  : SDynamicEntityMove(pEntity, p_startTime, p_endTime, pPeriod), SDynamicRotation(pInitAngle, pRadius, pPeriod) {
}

SDynamicEntityRotation::~SDynamicEntityRotation() {
}

SDynamicEntityTranslation::SDynamicEntityTranslation(std::string pEntity, float pX, float pY, int pPeriod, int p_startTime, int p_endTime) : SDynamicEntityMove(pEntity, p_startTime, p_endTime, pPeriod), SDynamicTranslation(pX, pY, pPeriod) {
}

SDynamicEntityTranslation::~SDynamicEntityTranslation() {
}

void SDynamicEntityRotation::performXY(float *vx, float *vy, float *vAngle) {
  *vAngle = 0.0;
  SDynamicRotation::performXY(vx, vy);
}

void SDynamicEntityTranslation::performXY(float *vx, float *vy, float *vAngle) {
  *vAngle = 0.0;
  SDynamicTranslation::performXY(vx, vy);
}

/* block */

SDynamicBlockMove::SDynamicBlockMove(std::string pBlock, int p_startTime, int p_endTime, int pPeriod) : SDynamicObject(p_startTime, p_endTime, pPeriod){
  m_block = pBlock;
}

SDynamicBlockMove::~SDynamicBlockMove() {
}

void SDynamicBlockMove::performMove(MotoGame* v_motoGame, int i_nbCents) {
  Block *p = v_motoGame->getLevelSrc()->getBlockById(m_block);
  float vx, vy, vAngle;
  float addvx = 0.0, addvy = 0.0, addvAngle = 0.0;

  if(i_nbCents > 0) {
    for(int i=0; i<i_nbCents; i++) {
      performXY(&vx, &vy, &vAngle);
      addvx += vx;
      addvy += vy;
      addvAngle = vAngle;
    }

    if(addvx != 0.0 || addvy != 0) { /* a simple fast test because it's probably the main case */
      v_motoGame->MoveBlock(p->Id(),
			    addvx,
			    addvy);
    }

    if(vAngle != 0.0) { /* a simple fast test because it's probably the main case */
      v_motoGame->SetBlockRotation(p->Id(), addvAngle + p->DynamicRotation());
    }
  }
}

std::string SDynamicBlockMove::getObjectId() {
  return m_block;
}

SDynamicBlockRotation::SDynamicBlockRotation(std::string pBlock, float pInitAngle, float pRadius, int pPeriod, int p_startTime, int p_endTime) : SDynamicBlockMove(pBlock, p_startTime, p_endTime, pPeriod), SDynamicRotation(pInitAngle, pRadius, pPeriod) {
}

SDynamicBlockRotation::~SDynamicBlockRotation() {
}

SDynamicBlockTranslation::SDynamicBlockTranslation(std::string pBlock, float pX, float pY, int pPeriod, int p_startTime, int p_endTime) : SDynamicBlockMove(pBlock, p_startTime, p_endTime, pPeriod), SDynamicTranslation(pX, pY, pPeriod) {
}

SDynamicBlockTranslation::~SDynamicBlockTranslation() {
}

void SDynamicBlockRotation::performXY(float *vx, float *vy, float *vAngle) {
  *vAngle = 0.0;
  SDynamicRotation::performXY(vx, vy);
}

void SDynamicBlockTranslation::performXY(float *vx, float *vy, float *vAngle) {
  *vAngle = 0.0;
  SDynamicTranslation::performXY(vx, vy);
}

SDynamicBlockSelfRotation::SDynamicBlockSelfRotation(std::string pBlock, int pPeriod, int p_startTime, int p_endTime)
  : SDynamicBlockMove(pBlock, p_startTime, p_endTime, pPeriod), SDynamicSelfRotation(pPeriod){
  
}

SDynamicBlockSelfRotation::~SDynamicBlockSelfRotation() {
}

void SDynamicBlockSelfRotation::performXY(float *vx, float *vy, float *vAngle) {
  *vx = *vy = 0.0;
  SDynamicSelfRotation::performXY(vAngle);
}

SDynamicEntitySelfRotation::SDynamicEntitySelfRotation(std::string pEntity, int pPeriod, int p_startTime, int p_endTime)
  : SDynamicEntityMove(pEntity, p_startTime, p_endTime, pPeriod), SDynamicSelfRotation(pPeriod){
  
}

SDynamicEntitySelfRotation::~SDynamicEntitySelfRotation() {
}

void SDynamicEntitySelfRotation::performXY(float *vx, float *vy, float *vAngle) {
  *vx = *vy = 0.0;
  SDynamicSelfRotation::performXY(vAngle);
}
