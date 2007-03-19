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

#ifndef __BIKEPLAYER_H__
#define __BIKEPLAYER_H__

#include "Bike.h"

class PlayerBiker : public Biker {
 public:
  PlayerBiker(Vector2f i_position, DriveDir i_direction, Vector2f i_gravity, Theme *i_theme, BikerTheme* i_bikerTheme);
  ~PlayerBiker();

  OnBikerHooks* getOnBikerHooks();
  void setOnBikerHooks(OnBikerHooks* i_bikerHooks);

  void updateToTime(float i_time, float i_timeStep, vapp::CollisionSystem *v_collisionSystem, Vector2f i_gravity);
  void initToPosition(Vector2f i_position, DriveDir i_direction, Vector2f i_gravity);
  BikeController* getControler();

  std::string getDescription() const;

  /* added=added to m_touching */
  /* removed=removed from m_touching */
  typedef enum {none, added, removed} touch;

  std::vector<Entity *> &EntitiesTouching();
  std::vector<Zone *> &ZonesTouching();

  bool isTouching(const Entity& i_entity) const;
  touch setTouching(Entity& i_entity, bool i_touching);     
  bool isTouching(const Zone& i_zone) const;
  touch setTouching(Zone& i_zone, bool i_isTouching);

  float getBikeEngineSpeed();
  bool  isWheelSpinning();
  Vector2f getWheelSpinPoint();
  Vector2f getWheelSpinDir();

  bool getRenderBikeFront();

  void setBodyDetach(bool state);
  void resetAutoDisabler();
  bool isSqueeking();
  float howMuchSqueek();

 private:
  SomersaultCounter m_somersaultCounter;
  BikeController m_BikeC;
  bool m_bFirstPhysicsUpdate;
  bool m_bodyDetach;
  OnBikerHooks* m_bikerHooks;

  float m_fAttitudeCon;
  float m_fNextAttitudeCon;
  int m_nStillFrames;
  double m_fLastSqueekTime;
  bool m_bSqueeking;
  float m_fHowMuchSqueek;

  std::vector<Entity *> m_entitiesTouching;
  std::vector<Zone *>   m_zonesTouching;

  /* */
  Vector2f m_PrevRearWheelP;          /* Prev. rear wheel position */
  Vector2f m_PrevFrontWheelP;         /* Prev. front wheel position */
  Vector2f m_PrevHeadP;
  Vector2f m_PrevHead2P;
  Vector2f m_PrevActiveHead;
  
  /* Wheels spinning dirt up... muzakka! :D */
  bool m_bWheelSpin;                  /* Do it captain */
  Vector2f m_WheelSpinPoint,m_WheelSpinDir; /* Where and how much */
  
  /* Data - bike bodies, joints and masses */
  dBodyID m_FrameBodyID;              /* Frame of bike */
  dMass m_FrameMass;              
  dBodyID m_RearWheelBodyID;          /* Rear wheel of bike */
  dMass m_RearWheelMass;
  dBodyID m_FrontWheelBodyID;         /* Front wheel of bike */
  dMass m_FrontWheelMass;    
        
  dBodyID m_PlayerTorsoBodyID;
  dMass m_PlayerTorsoMass;
  dBodyID m_PlayerLArmBodyID;
  dMass m_PlayerLArmMass;
  dBodyID m_PlayerUArmBodyID;
  dMass m_PlayerUArmMass;
  dBodyID m_PlayerLLegBodyID;
  dMass m_PlayerLLegMass;
  dBodyID m_PlayerULegBodyID;
  dMass m_PlayerULegMass;
  dBodyID m_PlayerHandAnchorBodyID;
  dMass m_PlayerHandAnchorMass;
  dBodyID m_PlayerFootAnchorBodyID;
  dMass m_PlayerFootAnchorMass;
  
  dJointID m_FootHingeID;
  dJointID m_KneeHingeID;
  dJointID m_LowerBodyHingeID;
  dJointID m_ShoulderHingeID;
  dJointID m_ElbowHingeID;
  dJointID m_HandHingeID;
  
  dBodyID m_PlayerTorsoBodyID2;
  dBodyID m_PlayerLArmBodyID2;
  dBodyID m_PlayerUArmBodyID2;
  dBodyID m_PlayerLLegBodyID2;
  dBodyID m_PlayerULegBodyID2;
  dBodyID m_PlayerHandAnchorBodyID2;
  dBodyID m_PlayerFootAnchorBodyID2;
  
  dJointID m_FootHingeID2;
  dJointID m_KneeHingeID2;
  dJointID m_LowerBodyHingeID2;
  dJointID m_ShoulderHingeID2;
  dJointID m_ElbowHingeID2;
  dJointID m_HandHingeID2;
            
  dJointGroupID m_ContactGroup;       /* Contact joint group */     
  
  bool bFrontWheelTouching;
  bool bRearWheelTouching;
  dWorldID m_WorldID;                 /* World ID */
      
  bool m_clearDynamicTouched;

  /* ***** */

  void initPhysics(Vector2f i_gravity);
  void uninitPhysics();
  void updatePhysics(float i_time, float fTimeStep, vapp::CollisionSystem *v_collisionSystem, Vector2f i_gravity);
  void clearStates();
  void updateGameState();
  void prepareBikePhysics(Vector2f StartPos);
  void prepareRider(Vector2f StartPos);

  bool intersectHeadLevel(Vector2f Cp,float Cr,const Vector2f &LastCp, vapp::CollisionSystem *v_collisionSystem);
  int  intersectWheelLevel(Vector2f Cp,float Cr,dContact *pContacts, vapp::CollisionSystem *v_collisionSystem);
  int  intersectWheelLine(Vector2f Cp,float Cr,int nNumContacts,dContact *pContacts,Vector2f A0,Vector2f A1);
  bool intersectHeadLine(Vector2f Cp,float Cr,Vector2f A0,Vector2f A1);
};


#endif
