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

#ifndef __BIKEPLAYER_H__
#define __BIKEPLAYER_H__

#include "Bike.h"
#include "BikeGhost.h"
#include "xmoto/SomersaultCounter.h"
#include <ode/ode.h>

class BikeController;
class BikeControllerNet;
class BikeControllerPlayer;

class ExternalForce {
public:
  ExternalForce(int i_startTime, int i_endTime, Vector2f i_force);
  int startTime();
  int endTime();
  Vector2f force();

private:
  int m_startTime;
  int m_endTime;
  Vector2f m_force;
};

class PlayerLocalBiker : public Biker {
public:
  PlayerLocalBiker(PhysicsSettings *i_physicsSettings,
                   Vector2f i_position,
                   DriveDir i_direction,
                   Vector2f i_gravity,
                   bool i_engineSound,
                   Theme *i_theme,
                   BikerTheme *i_bikerTheme,
                   const TColor &i_filterColor,
                   const TColor &i_filterUglyColor);
  virtual ~PlayerLocalBiker();

  void updateToTime(int i_time,
                    int i_timeStep,
                    CollisionSystem *i_collisionSystem,
                    Vector2f i_gravity,
                    Scene *i_motogame);
  void initToPosition(Vector2f i_position,
                      DriveDir i_direction,
                      Vector2f i_gravity);

  std::string getVeryQuickDescription() const;
  std::string getQuickDescription() const;
  std::string getDescription() const;
  void setBodyDetach(bool state);

  virtual void addBodyForce(int i_time,
                            const Vector2f &i_force,
                            int i_startTime,
                            int i_endTime);

  float getBikeEngineSpeed();
  float getBikeLinearVel();

  bool getRenderBikeFront();
  void resetAutoDisabler();
  bool isSqueeking();
  float howMuchSqueek();

  virtual float getRearWheelVelocity();
  virtual float getFrontWheelVelocity();
  virtual float getTorsoVelocity();

  virtual double getAngle();

  virtual BikeController *getControler();

private:
  PlayerLocalBiker();

  BikeControllerPlayer *m_BikeC;

  SomersaultCounter m_somersaultCounter;
  bool m_bFirstPhysicsUpdate;

  float m_fAttitudeCon;
  float m_fNextAttitudeCon;
  float m_fLastAttitudeDir;

  int m_nStillFrames;
  int m_lastSqueekTime;
  bool m_bSqueeking;
  float m_fHowMuchSqueek;

  std::vector<ExternalForce *> m_externalForces;
  Vector2f determineForceToAdd(int i_time);

  /* */
  Vector2f m_PrevRearWheelP; /* Prev. rear wheel position */
  Vector2f m_PrevFrontWheelP; /* Prev. front wheel position */
  Vector2f m_PrevHeadP;
  Vector2f m_PrevHead2P;
  Vector2f m_PrevActiveHead;

  /* Data - bike bodies, joints and masses */
  dBodyID m_FrameBodyID; /* Frame of bike */
  dMass m_FrameMass;
  dBodyID m_RearWheelBodyID; /* Rear wheel of bike */
  dMass m_RearWheelMass;
  dBodyID m_FrontWheelBodyID; /* Front wheel of bike */
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

  dJointGroupID m_ContactGroup; /* Contact joint group */

  bool bFrontWheelTouching;
  bool bRearWheelTouching;
  dWorldID m_WorldID; /* World ID */

  bool m_clearDynamicTouched;

  /* ***** */

  void initPhysics(Vector2f i_gravity);
  void uninitPhysics();
  void updatePhysics(int i_time,
                     int i_timeStep,
                     CollisionSystem *v_collisionSystem,
                     Vector2f i_gravity);
  void updateGameState();
  void prepareBikePhysics(Vector2f StartPos);
  void prepareRider(Vector2f StartPos);

  bool intersectHeadLevel(Vector2f Cp,
                          float Cr,
                          const Vector2f &LastCp,
                          CollisionSystem *v_collisionSystem);
  int intersectWheelLevel(Vector2f Cp,
                          float Cr,
                          dContact *pContacts,
                          CollisionSystem *v_collisionSystem);
  int intersectBodyLevel(Vector2f Cp,
                         float Cr,
                         dContact *pContacts,
                         CollisionSystem *v_collisionSystem);
  int intersectWheelLine(Vector2f Cp,
                         float Cr,
                         int nNumContacts,
                         dContact *pContacts,
                         Vector2f A0,
                         Vector2f A1);
  bool intersectHeadLine(Vector2f Cp, float Cr, Vector2f A0, Vector2f A1);
};

class PlayerNetClient : public Biker {
public:
  PlayerNetClient(PhysicsSettings *i_physicsSettings,
                  Vector2f i_position,
                  DriveDir i_direction,
                  Vector2f i_gravity,
                  bool i_engineSound,
                  Theme *i_theme,
                  BikerTheme *i_bikerTheme,
                  const TColor &i_colorFilter,
                  const TColor &i_uglyColorFilter);
  ~PlayerNetClient();

  virtual bool getRenderBikeFront();
  virtual float getBikeEngineSpeed();
  virtual float getBikeLinearVel();
  virtual float getTorsoVelocity();

  virtual double getAngle();
  virtual std::string getVeryQuickDescription() const;
  virtual std::string getQuickDescription() const;
  virtual std::string getDescription() const;

  virtual void updateToTime(int i_time,
                            int i_timeStep,
                            CollisionSystem *i_collisionSystem,
                            Vector2f i_gravity,
                            Scene *i_motogame);
  virtual BikeState *getStateForUpdate();
  virtual bool isStateInitialized() const;

  virtual void setLocalNetId(int i_value);
  virtual BikeController *getControler();
  void initToPosition(Vector2f i_position,
                      DriveDir i_direction,
                      Vector2f i_gravity);

private:
  BikeControllerNet *m_BikeC;

  /* previous states */
  BikeState *m_bikeStateForUpdate;
  bool m_stateExternallyUpdated;
  bool m_previousBikeStatesInitialized;
  std::vector<BikeState *> m_previousBikeStates;
  BikeState *m_lastExtrapolateBikeState;
  int m_lastFrameTimeUpdate;

  bool m_isStateInitialized;
};

#endif
