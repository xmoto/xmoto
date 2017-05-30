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
#include "math.h"
#include "xmscene/Block.h"
#include "xmscene/Entity.h"
#include "xmscene/Level.h"
#include "xmscene/Scene.h"
#include <chipmunk.h>

SDynamicObject::SDynamicObject(int p_startTime, int p_endTime, int pPeriod) {
  m_time = 0;
  m_startTime = p_startTime;
  m_endTime = p_endTime;
  m_period = pPeriod;

  m_objectId = "";
}

SDynamicObject::~SDynamicObject() {}

bool SDynamicObject::nextState(Scene *v_motoGame, int i_nbCents) {
  int v_realNbCents;

  v_realNbCents = i_nbCents;

  if (m_time >= m_endTime && m_endTime != 0) {
    return false;
  }

  if (m_startTime > m_time) {
    v_realNbCents -= m_startTime - m_time;
  }

  if (m_time + i_nbCents > m_endTime && m_endTime != 0) {
    v_realNbCents -= m_time + i_nbCents - m_endTime;
  }

  performMove(v_motoGame, v_realNbCents);

  m_time += i_nbCents;
  if (m_time >= m_endTime && m_endTime != 0) {
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

SDynamicTranslation::SDynamicTranslation(float pX, float pY, int pPeriod) {
  float Z;

  m_X = pX;
  m_Y = pY;

  m_sensUp = true;
  Z = sqrt(m_X * m_X + m_Y * m_Y);

  m_Speed = 0.0;
  if (pPeriod != 0) {
    m_Speed = (Z * 2) / ((float)(pPeriod));
  }

  m_moveX = 0.0;
  m_moveY = 0.0;
  if (Z != 0.0) {
    m_moveX = (m_Speed * m_X) / Z;
    m_moveY = (m_Speed * m_Y) / Z;
  }

  m_totalMoveX = 0.0;
  m_totalMoveY = 0.0;
}

SDynamicTranslation::~SDynamicTranslation() {}

void SDynamicTranslation::performXY(float *vx, float *vy) {
  *vx = m_sensUp ? m_moveX : -m_moveX;
  *vy = m_sensUp ? m_moveY : -m_moveY;

  m_totalMoveX += m_sensUp ? fabs(*vx) : -fabs(*vx);
  m_totalMoveY += m_sensUp ? fabs(*vy) : -fabs(*vy);

  if ((m_totalMoveX < 0 || m_totalMoveX > fabs(m_X)) ||
      (m_totalMoveY < 0 || m_totalMoveY > fabs(m_Y))) {
    m_sensUp = !m_sensUp;
  }
}

SDynamicRotation::SDynamicRotation(float pInitAngle,
                                   float pRadius,
                                   int pPeriod) {
  m_Speed = 0.0;
  if (pPeriod != 0) {
    m_Speed = (2 * M_PI) / ((float)(pPeriod));
  }

  m_Angle = pInitAngle;
  m_Radius = pRadius;

  m_CenterX = cos(m_Angle) * m_Radius;
  m_CenterY = sin(m_Angle) * m_Radius;
  m_previousVx = 0.0;
  m_previousVy = 0.0;
}

SDynamicRotation::~SDynamicRotation() {}

void SDynamicRotation::performXY(float *vx, float *vy) {
  if (m_Angle >= 2 * M_PI) {
    m_Angle -= 2 * M_PI;
  } /* because of float limit */
  float x, y;

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

SDynamicSelfRotation::~SDynamicSelfRotation() {}

void SDynamicSelfRotation::performXY(float *vAngle) {
  *vAngle = (2 * M_PI) / ((float)m_period);
  m_incr++;
  m_totalAngle += *vAngle;

  /* to limit sum error */
  if ((m_period > 0 && m_incr >= m_period) ||
      (m_period < 0 && m_incr >= -m_period)) {
    float v_realSum = (((float)m_incr) * (2 * M_PI)) / ((float)m_period);
    *vAngle += v_realSum - m_totalAngle;
    m_incr = 0;
    m_totalAngle = 0.0;
  }
}

/* entity */

SDynamicEntityMove::SDynamicEntityMove(std::string pEntity,
                                       int p_startTime,
                                       int p_endTime,
                                       int pPeriod)
  : SDynamicObject(p_startTime, p_endTime, pPeriod) {
  m_entityName = pEntity;
  m_objectId = pEntity;
  m_entity = NULL;
}

SDynamicEntityMove::~SDynamicEntityMove() {}

void SDynamicEntityMove::performMove(Scene *v_motoGame, int i_nbCents) {
  if (m_entity == NULL)
    m_entity = v_motoGame->getLevelSrc()->getEntityById(m_entityName);

  if (m_entity->isAlive() == false) {
    return;
  }

  float vx, vy, vAngle;
  float addvx = 0.0, addvy = 0.0, addvAngle = 0.0;

  if (i_nbCents > 0) {
    for (int i = 0; i < i_nbCents; i++) {
      performXY(&vx, &vy, &vAngle);
      addvx += vx;
      addvy += vy;
      addvAngle = vAngle;
    }

    /* a simple fast test because it's probably the main case */
    if (addvx != 0.0 || addvy != 0) {
      v_motoGame->translateEntity(m_entity, addvx, addvy);
    }

    /* a simple fast test because it's probably the main case */
    if (vAngle != 0.0)
      m_entity->setDrawAngle(addvAngle + m_entity->DrawAngle());
  }
}

SDynamicEntityRotation::SDynamicEntityRotation(std::string pEntity,
                                               float pInitAngle,
                                               float pRadius,
                                               int pPeriod,
                                               int p_startTime,
                                               int p_endTime)
  : SDynamicEntityMove(pEntity, p_startTime, p_endTime, pPeriod)
  , SDynamicRotation(pInitAngle, pRadius, pPeriod) {}

SDynamicEntityRotation::~SDynamicEntityRotation() {}

SDynamicEntityTranslation::SDynamicEntityTranslation(std::string pEntity,
                                                     float pX,
                                                     float pY,
                                                     int pPeriod,
                                                     int p_startTime,
                                                     int p_endTime)
  : SDynamicEntityMove(pEntity, p_startTime, p_endTime, pPeriod)
  , SDynamicTranslation(pX, pY, pPeriod) {}

SDynamicEntityTranslation::~SDynamicEntityTranslation() {}

void SDynamicEntityRotation::performXY(float *vx, float *vy, float *vAngle) {
  *vAngle = 0.0;
  SDynamicRotation::performXY(vx, vy);
}

void SDynamicEntityTranslation::performXY(float *vx, float *vy, float *vAngle) {
  *vAngle = 0.0;
  SDynamicTranslation::performXY(vx, vy);
}

/* block */

SDynamicBlockMove::SDynamicBlockMove(std::string pBlock,
                                     int p_startTime,
                                     int p_endTime,
                                     int pPeriod)
  : SDynamicObject(p_startTime, p_endTime, pPeriod) {
  m_blockName = pBlock;
  m_objectId = pBlock;
  m_block = NULL;
}

SDynamicBlockMove::~SDynamicBlockMove() {}

void SDynamicBlockMove::performMove(Scene *v_motoGame, int i_nbCents) {
  if (m_block == NULL)
    m_block = v_motoGame->getLevelSrc()->getBlockById(m_blockName);

  float vx, vy, vAngle;
  float addvx = 0.0, addvy = 0.0, addvAngle = 0.0;

  if (i_nbCents > 0) {
    for (int i = 0; i < i_nbCents; i++) {
      performXY(&vx, &vy, &vAngle);
      addvx += vx;
      addvy += vy;
      addvAngle = vAngle;
    }

    if (addvx != 0.0 || addvy != 0)
      v_motoGame->MoveBlock(m_block, addvx, addvy);

    if (vAngle != 0.0)
      v_motoGame->SetBlockRotation(m_block,
                                   addvAngle + m_block->DynamicRotation());
  }
}

SDynamicBlockRotation::SDynamicBlockRotation(std::string pBlock,
                                             float pInitAngle,
                                             float pRadius,
                                             int pPeriod,
                                             int p_startTime,
                                             int p_endTime)
  : SDynamicBlockMove(pBlock, p_startTime, p_endTime, pPeriod)
  , SDynamicRotation(pInitAngle, pRadius, pPeriod) {}

SDynamicBlockRotation::~SDynamicBlockRotation() {}

SDynamicBlockTranslation::SDynamicBlockTranslation(std::string pBlock,
                                                   float pX,
                                                   float pY,
                                                   int pPeriod,
                                                   int p_startTime,
                                                   int p_endTime)
  : SDynamicBlockMove(pBlock, p_startTime, p_endTime, pPeriod)
  , SDynamicTranslation(pX, pY, pPeriod) {}

SDynamicBlockTranslation::~SDynamicBlockTranslation() {}

void SDynamicBlockRotation::performXY(float *vx, float *vy, float *vAngle) {
  *vAngle = 0.0;
  SDynamicRotation::performXY(vx, vy);
}

void SDynamicBlockTranslation::performXY(float *vx, float *vy, float *vAngle) {
  *vAngle = 0.0;
  SDynamicTranslation::performXY(vx, vy);
}

SDynamicBlockSelfRotation::SDynamicBlockSelfRotation(std::string pBlock,
                                                     int pPeriod,
                                                     int p_startTime,
                                                     int p_endTime)
  : SDynamicBlockMove(pBlock, p_startTime, p_endTime, pPeriod)
  , SDynamicSelfRotation(pPeriod) {}

SDynamicBlockSelfRotation::~SDynamicBlockSelfRotation() {}

void SDynamicBlockSelfRotation::performXY(float *vx, float *vy, float *vAngle) {
  *vx = *vy = 0.0;
  SDynamicSelfRotation::performXY(vAngle);
}

SDynamicEntitySelfRotation::SDynamicEntitySelfRotation(std::string pEntity,
                                                       int pPeriod,
                                                       int p_startTime,
                                                       int p_endTime)
  : SDynamicEntityMove(pEntity, p_startTime, p_endTime, pPeriod)
  , SDynamicSelfRotation(pPeriod) {}

SDynamicEntitySelfRotation::~SDynamicEntitySelfRotation() {}

void SDynamicEntitySelfRotation::performXY(float *vx,
                                           float *vy,
                                           float *vAngle) {
  *vx = *vy = 0.0;
  SDynamicSelfRotation::performXY(vAngle);
}

SPhysicBlockMove::SPhysicBlockMove(std::string blockName,
                                   int startTime,
                                   int endTime,
                                   int period)
  : SDynamicObject(startTime, endTime, period) {
  m_blockName = blockName;
  m_objectId = blockName;
  m_block = NULL;
}

void SPhysicBlockMove::performMove(Scene *pScene, int i_nbCents) {
  if (m_block == NULL)
    m_block = pScene->getLevelSrc()->getBlockById(m_blockName);

  if (i_nbCents > 0) {
    for (int i = 0; i < i_nbCents; i++) {
      applyForce();
    }
  }
}

SPhysicBlockSelfRotation::SPhysicBlockSelfRotation(std::string blockName,
                                                   int startTime,
                                                   int endTime,
                                                   int torque)
  : SPhysicBlockMove(blockName, startTime, endTime, 1) {
  m_torque = torque;
}

void SPhysicBlockSelfRotation::applyForce() {
  // magical formula to change the asked period into a force
  cpBody *body = m_block->getPhysicBody();
  if (body == NULL)
    return;

  // in the moon buggy example, the author manually update
  // the torque instead of using applyforce, let's do the same
  body->t += m_torque;
}

SPhysicBlockTranslation::SPhysicBlockTranslation(std::string blockName,
                                                 float x,
                                                 float y,
                                                 int period,
                                                 int startTime,
                                                 int endTime)
  : SPhysicBlockMove(blockName, startTime, endTime, period) {
  float Z;

  m_X = x;
  m_Y = y;

  m_sensUp = true;
  Z = sqrt(m_X * m_X + m_Y * m_Y);

  m_Speed = 0.0;
  if (period != 0) {
    m_Speed = (Z * 2) / ((float)(period));
  }

  m_moveX = 0.0;
  m_moveY = 0.0;
  if (Z != 0.0) {
    m_moveX = (m_Speed * m_X) / Z;
    m_moveY = (m_Speed * m_Y) / Z;
  }

  m_totalMoveX = 0.0;
  m_totalMoveY = 0.0;
}

void SPhysicBlockTranslation::applyForce() {
  cpBody *body = m_block->getPhysicBody();
  if (body == NULL)
    return;

  float x = m_sensUp ? m_moveX : -m_moveX;
  float y = m_sensUp ? m_moveY : -m_moveY;

  m_totalMoveX += m_sensUp ? fabs(x) : -fabs(x);
  m_totalMoveY += m_sensUp ? fabs(y) : -fabs(y);

  if ((m_totalMoveX < 0 || m_totalMoveX > fabs(m_X)) ||
      (m_totalMoveY < 0 || m_totalMoveY > fabs(m_Y))) {
    m_sensUp = !m_sensUp;
  }

  // apply a force so that the blocks moves of (x,y)
  // TODO::this mult need tweaking
  float mult = 25000.0f;
  cpBodyApplyForce(body, cpv(x * mult, y * mult), cpvzero);
}
