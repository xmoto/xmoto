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

#ifndef __BIKE_H__
#define __BIKE_H__

#include "../helpers/Color.h"
#include "../helpers/VMath.h"
#include "BasicSceneStructs.h"
#include <string>
#include <vector>

class Level;
class Entity;
class Zone;
class OnBikerHooks;
class Replay;
class CollisionSystem;
class Scene;
class Replay;
class BikerTheme;
class Theme;
class BikeAnchors;
class BikeParameters;
class BikeController;
class EngineSoundSimulator;
class PhysicsSettings;

class BikeState {
public:
  DriveDir Dir; /* Driving left or right? */
  float fBikeEngineRPM;

  Vector2f RearWheelP; /* Rear wheel position */
  Vector2f FrontWheelP; /* Front wheel position */
  Vector2f SwingAnchorP; /* Swing arm anchor position */
  Vector2f FrontAnchorP; /* Front suspension anchor position */
  Vector2f SwingAnchor2P; /* Swing arm anchor position (Alt.) */
  Vector2f FrontAnchor2P; /* Front suspension anchor position (Alt.) */
  Vector2f CenterP; /* Center position */

  Vector2f PlayerTorsoP; /* Position of player's torso */
  Vector2f PlayerULegP; /* Position of player's upper leg */
  Vector2f PlayerLLegP; /* Position of player's lower leg */
  Vector2f PlayerUArmP; /* Position of player's upper arm */
  Vector2f PlayerLArmP; /* Position of player's upper arm */

  Vector2f PlayerTorso2P; /* Position of player's torso (Alt.) */
  Vector2f PlayerULeg2P; /* Position of player's upper leg (Alt.) */
  Vector2f PlayerLLeg2P; /* Position of player's lower leg (Alt.) */
  Vector2f PlayerUArm2P; /* Position of player's upper arm (Alt.) */
  Vector2f PlayerLArm2P; /* Position of player's upper arm (Alt.) */

  /* Internals */
  float fFrontWheelRot[4];
  float fRearWheelRot[4];
  float fFrameRot[4];

  Vector2f WantedHandP, WantedFootP;
  Vector2f WantedHand2P, WantedFoot2P;

  Vector2f HandP;
  Vector2f ElbowP;
  Vector2f ShoulderP;
  Vector2f LowerBodyP;
  Vector2f KneeP;
  Vector2f FootP;
  Vector2f HeadP; /* NB! not a phys. body */

  Vector2f Hand2P;
  Vector2f Elbow2P;
  Vector2f Shoulder2P;
  Vector2f LowerBody2P;
  Vector2f Knee2P;
  Vector2f Foot2P;
  Vector2f Head2P; /* NB! not a phys. body */

  Vector2f RRearWheelP; /* Relaxed rear wheel position */
  Vector2f RFrontWheelP; /* Relaxed front wheel position */
  Vector2f PrevRq; /* Previous error (rear) */
  Vector2f PrevFq; /* Previous error (front) */
  Vector2f PrevPFq; /* Previous error (player foot) */
  Vector2f PrevPHq; /* Previous error (player hand) */
  Vector2f PrevPFq2; /* Previous error (player foot) (Alt.) */
  Vector2f PrevPHq2; /* Previous error (player hand) (Alt.) */

  int GameTime; // time of the state, for replays;

  /* Bonusinfo */
  BikeState(PhysicsSettings *i_physicsSettings);
  virtual ~BikeState();

  BikeState &operator=(const BikeState &i_copy);

  void reInitializeAnchors();
  void clear();

  BikeAnchors *Anchors();
  BikeParameters *Parameters();

  static void interpolateGameState(std::vector<BikeState *> &i_ghostBikeStates,
                                   BikeState *p,
                                   float t);
  static void interpolateGameStateLinear(
    std::vector<BikeState *> &i_ghostBikeStates,
    BikeState *p,
    float t);
  static void interpolateGameStateCubic(
    std::vector<BikeState *> &i_ghostBikeStates,
    BikeState *p,
    float t);

  static void convertStateFromReplay(SerializedBikeState *pReplayState,
                                     BikeState *pBikeS,
                                     PhysicsSettings *i_physicsSettings);

  static signed char _MapCoordTo8Bits(float fRef, float fMaxDiff, float fCoord);
  static float _Map8BitsToCoord(float fRef, float fMaxDiff, signed char c);
  static unsigned short _MatrixTo16Bits(const float *pfMatrix);
  static void _16BitsToMatrix(unsigned short n16, float *pfMatrix);

private:
  /* Driving */
  BikeParameters *m_bikeParameters;
  BikeAnchors *m_bikeAnchors;
};

class Biker {
public:
  Biker(PhysicsSettings *i_physicsSettings,
        bool i_engineSound,
        Theme *i_theme,
        BikerTheme *i_bikerTheme,
        const TColor &i_colorFilter,
        const TColor &i_uglyColorFilter);
  virtual ~Biker();
  inline BikeState *getState() { return m_bikeState; }

  virtual BikeState *getStateForUpdate() { return m_bikeState; }

  virtual void stateBeforeExternalUpdated() {}
  virtual void stateAfterExternalUpdated() {}

  virtual bool
  getRenderBikeFront() = 0; /* display the bikefront ? (for detach) */
  virtual float getBikeEngineSpeed() = 0; /* engine speed */
  virtual float getBikeLinearVel() = 0; /* bike linear velocity */
  virtual float getTorsoVelocity() = 0; /* Torso Velocity, to determine corpse
                                           move (e.g. for death menu popup) */
  virtual double getAngle() = 0; /* biker angle */
  virtual std::string getVeryQuickDescription() const = 0;
  virtual std::string getQuickDescription() const = 0;
  virtual std::string getDescription() const = 0;
  virtual float getBikeEngineRPM();
  virtual void updateToTime(int i_time,
                            int i_timeStep,
                            CollisionSystem *i_collisionSystem,
                            Vector2f i_gravity,
                            Scene *i_motogame);
  virtual bool isNetGhost() const { return false; };

  PhysicsSettings *getPhysicsSettings();
  virtual bool isStateInitialized() const;
  int localNetId()
    const; // id of the biker for the network part (must be 0, 1, 2 or 3)
  virtual void setLocalNetId(int i_value);

  void setFinished(bool i_value, int i_finishTime);
  void setDead(bool i_value, int i_deadTime = 0);
  bool isFinished() const;
  int finishTime() const;
  bool isDead() const;
  int deadTime() const;
  BikerTheme *getBikeTheme();
  virtual BikeController *getControler();
  virtual bool isWheelSpinning();
  virtual Vector2f getWheelSpinPoint();
  virtual Vector2f getWheelSpinDir();
  virtual float getRearWheelVelocity();
  virtual float getFrontWheelVelocity();
  virtual void initWheelDetach();
  virtual void initToPosition(Vector2f i_position,
                              DriveDir i_direction,
                              Vector2f i_gravity);
  virtual void
  resetAutoDisabler(); /* a player can have a disabler when nothing append */

  OnBikerHooks *getOnBikerHooks();
  void setOnBikerHooks(OnBikerHooks *i_bikerHooks);

  /* added=added to m_touching */
  /* removed=removed from m_touching */
  typedef enum { none, added, removed } touch;

  std::vector<Entity *> &EntitiesTouching();
  std::vector<Zone *> &ZonesTouching();

  bool isTouching(const Entity *i_entity) const;
  touch setTouching(Entity *i_entity, bool i_touching);
  bool isTouching(const Zone *i_zone) const;
  touch setTouching(Zone *i_zone, bool i_isTouching);

  void clearStates();

  virtual void setBodyDetach(bool state);
  const TColor &getColorFilter() const;
  const TColor &getUglyColorFilter() const;

  virtual void addBodyForce(int i_time,
                            const Vector2f &i_force,
                            int i_startTime,
                            int i_endTime);

  void setInterpolation(bool bValue);

  std::vector<Vector2f> &CollisionPoints();

  float changeDirPer() const;

  unsigned int getNbRenderedFrames() const;
  void addNbRenderedFrames();

protected:
  BikerTheme *m_bikerTheme;
  BikeState *m_bikeState;
  EngineSoundSimulator *m_EngineSound;
  bool m_finished;
  int m_finishTime;
  bool m_dead;
  int m_deadTime;
  OnBikerHooks *m_bikerHooks;
  bool m_bodyDetach;
  bool m_wheelDetach;
  std::vector<Entity *> m_entitiesTouching;
  std::vector<Zone *> m_zonesTouching;

  PhysicsSettings *m_physicsSettings;

  /* Wheels spinning dirt up... muzakka! :D */
  bool m_bWheelSpin; /* Do it captain */
  Vector2f m_WheelSpinPoint, m_WheelSpinDir; /* Where and how much */

  TColor m_colorFilter;
  TColor m_uglyColorFilter;
  bool m_doInterpolation;

  /* collision points */
  void cleanCollisionPoints();
  std::vector<Vector2f> m_collisionPoints;

  /* changedir anim */
  float m_changeDirPer; // between 0.0 and 1.0, give the % of the change dir
  // done (only for graphisms)

  int m_localNetId;
  unsigned int m_nbRenderedFrames;
};

class OnBikerHooks {
public:
  virtual ~OnBikerHooks() {};
  virtual void onSomersaultDone(bool i_counterclock) = 0;
  virtual void onWheelTouches(int i_wheel, bool i_touch) = 0;
  virtual void onHeadTouches() = 0;
};

#endif /* __BIKE_H__ */
