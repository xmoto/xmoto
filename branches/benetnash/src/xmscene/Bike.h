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
#include "../helpers/Color.h"

class Level;
class Entity;
class Zone;

class OnBikerHooks;

namespace vapp {
  class Replay;
  class CollisionSystem;
  class MotoGame;
  class Replay;
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
  Biker(Theme *i_theme, BikerTheme* i_bikerTheme,
	const TColor& i_colorFilter,
	const TColor& i_uglyColorFilter);
  virtual BikeState* getState();
  virtual bool getRenderBikeFront() = 0; /* display the bikefront ? (for detach) */
  virtual float getBikeEngineSpeed() = 0; /* engine speed */
  virtual std::string getDescription() const = 0;
  virtual float getBikeEngineRPM();
  virtual void  updateToTime(float i_time, float i_timeStep,
			     vapp::CollisionSystem *i_collisionSystem, Vector2f i_gravity,
			     vapp::MotoGame *i_motogame);
  void setPlaySound(bool i_value);

  void setFinished(bool i_value, float i_finishTime);
  void setDead(bool i_value);
  bool isFinished() const;
  float finishTime() const;
  bool isDead() const;
  BikerTheme* getBikeTheme();
  virtual BikeController* getControler();
  virtual bool  isWheelSpinning();
  virtual Vector2f getWheelSpinPoint();
  virtual Vector2f getWheelSpinDir();
  virtual void initToPosition(Vector2f i_position, DriveDir i_direction, Vector2f i_gravity);
  virtual void resetAutoDisabler(); /* a player can have a disabler when nothing append */

  OnBikerHooks* getOnBikerHooks();
  void setOnBikerHooks(OnBikerHooks* i_bikerHooks);

  /* added=added to m_touching */
  /* removed=removed from m_touching */
  typedef enum {none, added, removed} touch;

  std::vector<Entity *> &EntitiesTouching();
  std::vector<Zone *> &ZonesTouching();

  bool isTouching(const Entity& i_entity) const;
  touch setTouching(Entity& i_entity, bool i_touching);     
  bool isTouching(const Zone& i_zone) const;
  touch setTouching(Zone& i_zone, bool i_isTouching);

  virtual void setBodyDetach(bool state);
  const TColor& getColorFilter() const;
  const TColor& getUglyColorFilter() const;

  void setInterpolation(bool bValue);

 protected:
  BikerTheme* m_bikerTheme;
  bool m_playSound;
  BikeState m_bikeState;
  vapp::EngineSoundSimulator m_EngineSound;
  bool m_finished;
  float m_finishTime;
  bool m_dead;
  OnBikerHooks* m_bikerHooks;
  bool m_bodyDetach;
  std::vector<Entity *> m_entitiesTouching;
  std::vector<Zone *>   m_zonesTouching;

  /* Wheels spinning dirt up... muzakka! :D */
  bool m_bWheelSpin;                  /* Do it captain */
  Vector2f m_WheelSpinPoint,m_WheelSpinDir; /* Where and how much */

  TColor m_colorFilter;
  TColor m_uglyColorFilter;
  bool m_doInterpolation;
};

class OnBikerHooks {
public:
  virtual void onSomersaultDone(bool i_counterclock) = 0;
  virtual void onWheelTouches(int i_wheel, bool i_touch) = 0;
  virtual void onHeadTouches() = 0;
};

#endif /* __BIKE_H__ */
