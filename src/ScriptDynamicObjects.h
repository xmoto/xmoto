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

#ifndef __SCRIPTDYNAMICOBJECTS_H__
#define __SCRIPTDYNAMICOBJECTS_H__

#include <string>

class MotoGame;

class SDynamicObject {
 public:
  SDynamicObject(int p_startTime, int p_endTime, int pPeriod);
  virtual ~SDynamicObject();
  
  /* return false if the dynamic is finished */
  bool nextState(MotoGame* v_motoGame, int i_nbCents);
  virtual std::string getObjectId() = 0;
  
 protected:
  bool isTimeToMove();
  virtual void performMove(MotoGame* v_motoGame, int i_nbCents) = 0;  
  int Period() const;

 private:
  int m_time;
  int m_startTime;
  int m_endTime;
  int m_period;
};

class SDynamicRotation {
 public:
  SDynamicRotation(float pInitAngle, float pRadius, int pPeriod);
  virtual ~SDynamicRotation();
  virtual void performXY(float *vx, float *vy);

 private:
  float m_Speed;

  float m_Angle;
  float m_Radius;

  float m_CenterX;
  float m_CenterY;

  float m_previousVx;
  float m_previousVy;
};

class SDynamicSelfRotation {
 public:
  SDynamicSelfRotation(int i_period);
  virtual ~SDynamicSelfRotation();
  virtual void performXY(float *vAngle);

 private:
  int m_incr;
  float m_totalAngle;
  int m_period;
};

class SDynamicTranslation {
 public:
  SDynamicTranslation(float pX, float pY, int pPeriod);
  virtual ~SDynamicTranslation();
  virtual void performXY(float *vx, float *vy);

 private:
  float m_Speed;
  float m_X, m_Y;

  bool m_sensUp;
  float m_moveX;
  float m_moveY;
  float m_totalMoveX;
  float m_totalMoveY;
};

/* entity */
class SDynamicEntityMove : public SDynamicObject {
 public:
  SDynamicEntityMove(std::string pEntity, int p_startTime, int p_endTime, int pPeriod);
  virtual ~SDynamicEntityMove();

  void performMove(MotoGame* p_motoGame, int i_nbCents);
  std::string getObjectId();

 protected:
  virtual void performXY(float *vx, float *vy, float *vAngle) = 0;

 private:
  std::string m_entity;
};

class SDynamicEntityRotation : public SDynamicEntityMove, public SDynamicRotation {
 public:
  SDynamicEntityRotation(std::string pEntity, float pInitAngle, float pRadius, int pPeriod, int p_startTime, int p_endTime);
  virtual ~SDynamicEntityRotation();

  void performXY(float *vx, float *vy, float *vAngle);

 private:
};

class SDynamicEntityTranslation : public SDynamicEntityMove, public SDynamicTranslation {
 public:
  SDynamicEntityTranslation(std::string pEntity, float pX, float pY, int pPeriod, int p_startTime, int p_endTime);
  virtual ~SDynamicEntityTranslation();

  void performXY(float *vx, float *vy, float *vAngle);

 private:
};

class SDynamicEntitySelfRotation : public SDynamicEntityMove, public SDynamicSelfRotation {
 public:
  SDynamicEntitySelfRotation(std::string pEntity, int pPeriod, int p_startTime, int p_endTime);
  virtual ~SDynamicEntitySelfRotation();

  void performXY(float *vx, float *vy, float *vAngle);

 private:
};

/* block */
class SDynamicBlockMove : public SDynamicObject {
 public:
  SDynamicBlockMove(std::string pBlock, int p_startTime, int p_endTime, int pPeriod);
  virtual ~SDynamicBlockMove();

  void performMove(MotoGame* p_motoGame, int i_nbCents);
  std::string getObjectId();

 protected:
  virtual void performXY(float *vx, float *vy, float *vAngle) = 0;

 private:
  std::string m_block;
};

class SDynamicBlockRotation : public SDynamicBlockMove, public SDynamicRotation {
 public:
  SDynamicBlockRotation(std::string pBlock, float pInitAngle, float pRadius, int pPeriod, int p_startTime, int p_endTime);
  virtual ~SDynamicBlockRotation();

  void performXY(float *vx, float *vy, float *vAngle);

 private:
};

class SDynamicBlockSelfRotation : public SDynamicBlockMove, public SDynamicSelfRotation {
 public:
  SDynamicBlockSelfRotation(std::string pBlock, int pPeriod, int p_startTime, int p_endTime);
  virtual ~SDynamicBlockSelfRotation();

  void performXY(float *vx, float *vy, float *vAngle);

 private:
};

class SDynamicBlockTranslation : public SDynamicBlockMove, public SDynamicTranslation {
 public:
  SDynamicBlockTranslation(std::string pBlock, float pX, float pY, int pPeriod, int p_startTime, int p_endTime);
  virtual ~SDynamicBlockTranslation();

  void performXY(float *vx, float *vy, float *vAngle);

 private:
};

#endif /* __SCRIPTDYNAMICOBJECTS_H__ */
