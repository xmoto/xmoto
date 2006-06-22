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


#ifndef __SCRIPTDYNAMICOBJECTS_H__
#define __SCRIPTDYNAMICOBJECTS_H__

#include <string>

namespace vapp {
  class MotoGame;
  class Entity;
};

class SDynamicObject {
 public:
  SDynamicObject(int p_startTime, int p_endTime, float pPeriod);
  ~SDynamicObject();
  
  /* return false if the dynamic is finished */
  bool nextState(vapp::MotoGame* v_motoGame);
  
 protected:
  bool isTimeToMove();
  virtual void performMove(vapp::MotoGame* v_motoGame) = 0;  

 private:
  int m_time;
  int m_startTime;
  int m_endTime;
  float m_period;
};

class SDynamicRotation {
 public:
  SDynamicRotation(float pInitAngle, float pRadius, float pPeriod);
  ~SDynamicRotation();
  void performXY(float *vx, float *vy);

 private:
  float m_Speed;

  float m_Angle;
  float m_Radius;

  float m_CenterX;
  float m_CenterY;

  float m_previousVx;
  float m_previousVy;
};

class SDynamicTranslation {
 public:
  SDynamicTranslation(float pX, float pY, float pPeriod);
  ~SDynamicTranslation();
  void performXY(float *vx, float *vy);

 private:
  float m_Speed;
  float m_X, m_Y;

  bool m_sensUp;
  float m_moveX;
  float m_moveY;
  float m_totalMoveX;
  float m_totalMoveY;
};

class SDynamicEntityMove : public SDynamicObject {
 public:
  SDynamicEntityMove(std::string pEntity, int p_startTime, int p_endTime, float pPeriod);
  ~SDynamicEntityMove();

  void performMove(vapp::MotoGame* p_motoGame);

 protected:
  virtual void performXY(float *vx, float *vy) = 0;

 private:
  std::string m_entity;
};

class SDynamicEntityRotation : public SDynamicEntityMove, public SDynamicRotation {
 public:
  SDynamicEntityRotation(std::string pEntity, float pInitAngle, float pRadius, float pPeriod, int p_startTime, int p_endTime);
  ~SDynamicEntityRotation();

  void performXY(float *vx, float *vy);

 private:
};

class SDynamicEntityTranslation : public SDynamicEntityMove, public SDynamicTranslation {
 public:
  SDynamicEntityTranslation(std::string pEntity, float pX, float pY, float pPeriod, int p_startTime, int p_endTime);
  ~SDynamicEntityTranslation();

  void performXY(float *vx, float *vy);

 private:
};

#endif /* __SCRIPTDYNAMICOBJECTS_H__ */
