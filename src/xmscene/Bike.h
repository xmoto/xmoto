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

#ifndef __BIKE_H__
#define __BIKE_H__

#include <string>
#include "BasicSceneStructs.h"
#include "BikeAnchors.h"
#include "BikeController.h"
#include "../SomersaultCounter.h"
#include "../Sound.h"

class Level;
class Entity;
class Zone;

class OnBikerHooks;

namespace vapp {
  class Replay;
  class CollisionSystem;
  class MotoGame;
}

class BikeState {
  public:
    DriveDir Dir;         /* Driving left or right? */
  
    float fBikeEngineRPM;
  
    Vector2f RearWheelP;  /* Rear wheel position */
    Vector2f FrontWheelP; /* Front wheel position */
    Vector2f SwingAnchorP;/* Swing arm anchor position */
    Vector2f FrontAnchorP;/* Front suspension anchor position */    
    Vector2f SwingAnchor2P;/* Swing arm anchor position (Alt.) */
    Vector2f FrontAnchor2P;/* Front suspension anchor position (Alt.) */    
    Vector2f CenterP;     /* Center position */

    Vector2f PlayerTorsoP;/* Position of player's torso */
    Vector2f PlayerULegP; /* Position of player's upper leg */
    Vector2f PlayerLLegP; /* Position of player's lower leg */
    Vector2f PlayerUArmP; /* Position of player's upper arm */
    Vector2f PlayerLArmP; /* Position of player's upper arm */

    Vector2f PlayerTorso2P;/* Position of player's torso (Alt.) */
    Vector2f PlayerULeg2P; /* Position of player's upper leg (Alt.) */
    Vector2f PlayerLLeg2P; /* Position of player's lower leg (Alt.) */
    Vector2f PlayerUArm2P; /* Position of player's upper arm (Alt.) */ 
    Vector2f PlayerLArm2P; /* Position of player's upper arm (Alt.) */
        
    /* Internals */
    float fFrontWheelRot[4];
    float fRearWheelRot[4];
    float fFrameRot[4];
    
    Vector2f WantedHandP,WantedFootP;
    Vector2f WantedHand2P,WantedFoot2P;    
    
    Vector2f HandP;
    Vector2f ElbowP;
    Vector2f ShoulderP;
    Vector2f LowerBodyP;
    Vector2f KneeP;
    Vector2f FootP;
    Vector2f HeadP;        /* NB! not a phys. body */

    Vector2f Hand2P;
    Vector2f Elbow2P;
    Vector2f Shoulder2P;
    Vector2f LowerBody2P;
    Vector2f Knee2P;
    Vector2f Foot2P;
    Vector2f Head2P;        /* NB! not a phys. body */

    Vector2f RRearWheelP; /* Relaxed rear wheel position */
    Vector2f RFrontWheelP;/* Relaxed front wheel position */
    Vector2f PrevRq;      /* Previous error (rear) */
    Vector2f PrevFq;      /* Previous error (front) */
    Vector2f PrevPFq;     /* Previous error (player foot) */
    Vector2f PrevPHq;     /* Previous error (player hand) */
    Vector2f PrevPFq2;    /* Previous error (player foot) (Alt.) */
    Vector2f PrevPHq2;    /* Previous error (player hand) (Alt.) */
    
    /* Bonusinfo */    
  BikeState();
  ~BikeState();
  
  void reInitializeSpeed();
  void physicalUpdate();
  void reInitializeAnchors();

  float CurrentBrake() const;
  float CurrentEngine() const;
    
  BikeAnchors& Anchors();
  BikeParameters& Parameters();

  static void interpolateGameState(SerializedBikeState *pA,SerializedBikeState *pB,SerializedBikeState *p,float t);
  static void updateStateFromReplay(SerializedBikeState *pReplayState,BikeState *pBikeS);

  static signed char _MapCoordTo8Bits(float fRef,float fMaxDiff,float fCoord);
  static float _Map8BitsToCoord(float fRef,float fMaxDiff,signed char c);
  static unsigned short _MatrixTo16Bits(const float *pfMatrix);
  static void _16BitsToMatrix(unsigned short n16,float *pfMatrix);

  private:
  /* Driving */
  float m_curBrake, m_curEngine;    
  BikeParameters m_bikeParameters;
  BikeAnchors    m_bikeAnchors;
};

class Biker {
 public:
  Biker(Theme *i_theme);
  virtual BikeState* getState();
  virtual bool getRenderBikeFront() = 0; /* display the bikefront ? (for detach) */
  virtual float getBikeEngineSpeed() = 0; /* engine speed */
  virtual float getBikeEngineRPM();
  virtual void  updateToTime(float i_time);

 protected:
  BikeState m_bikeState;
  vapp::EngineSoundSimulator m_EngineSound;
};

class PlayerBiker : public Biker {
 public:
  PlayerBiker(Vector2f i_position, DriveDir i_direction, Vector2f i_gravity, Theme *i_theme);
  ~PlayerBiker();

  OnBikerHooks* getOnBikerHooks();
  void setOnBikerHooks(OnBikerHooks* i_bikerHooks);

  void updateToTime(float i_time, float i_timeStep, vapp::CollisionSystem *v_collisionSystem, Vector2f i_gravity);
  void initToPosition(Vector2f i_position, DriveDir i_direction, Vector2f i_gravity);
  BikeController* getControler();

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

class Ghost : public Biker {
 public:
  Ghost(std::string i_replayFile, bool i_isActiv, Theme *i_theme);
  ~Ghost();

  std::string playerName();
  std::string levelId();
  float getFinishTime();
  void initLastToTakeEntities(Level* i_level);
  void updateDiffToPlayer(std::vector<float> &i_lastToTakeEntities);
  float diffToPlayer() const;
  void updateToTime(float i_time, vapp::MotoGame *i_motogame);
  void setInfo(std::string i_info);
  std::string getDescription() const;
  bool getRenderBikeFront();
  float getBikeEngineSpeed();

 private:
  vapp::Replay* m_replay;
  std::vector<float> m_lastToTakeEntities;
  float m_diffToPlayer; /* time diff between the ghost and the player */
  int m_nFrame;
  std::string m_info;
  bool m_isActiv;

  void execReplayEvents(float i_time, vapp::MotoGame *i_motogame);
};

class OnBikerHooks {
public:
  virtual void onSomersaultDone(bool i_counterclock) = 0;
  virtual void onWheelTouches(int i_wheel, bool i_touch) = 0;
  virtual void onHeadTouches() = 0;
};

#endif /* __BIKE_H__ */
