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
};

class SDynamicObject {
 public:
  SDynamicObject(int p_startTime, int p_endTime);
  ~SDynamicObject();
  
  /* return false if the dynamic is finished */
  virtual bool nextState(vapp::MotoGame* v_motoGame);
  
 protected:
  bool isTimeToMove();
  
 private:
  int m_time;
  int m_startTime;
  int m_endTime;  
};

class SDynamicEntityRotation : public SDynamicObject {
 public:
  SDynamicEntityRotation(std::string pEntity, float pInitAngle, float pRadius, float pSpeed, int p_startTime, int p_endTime);
  ~SDynamicEntityRotation();
  bool nextState(vapp::MotoGame* v_motoGame);

 private:
  std::string m_entity;
  float m_CenterX;
  float m_CenterY;
  float m_Angle;
  float m_Radius;
  float m_Speed;

  float m_previousVx;
  float m_previousVy;
};

#endif /* __SCRIPTDYNAMICOBJECTS_H__ */
