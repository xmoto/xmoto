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

#include "BikePlayer.h"
#include "BikeAnchors.h"
#include "BikeController.h"
#include "BikeParameters.h"
#include "PhysicsSettings.h"
#include "Scene.h"
#include "Zone.h"
#include "helpers/Log.h"
#include "xmoto/Collision.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"
#include "xmoto/PhysSettings.h"
#include "xmoto/Replay.h"
#include "xmoto/Sound.h"

// autodisabler options
#define PHYS_SLEEP_EPS 0.02 // 2006-04-23: changed from 0.008
#define PHYS_SLEEP_FRAMES 20 // 150
#define XM_MAX_EXTRAPOLATION_T 3.0

#define PHYS_SUSP_SQUEEK_POINT 0.01

PlayerLocalBiker::PlayerLocalBiker(PhysicsSettings *i_physicsSettings,
                                   Vector2f i_position,
                                   DriveDir i_direction,
                                   Vector2f i_gravity,
                                   bool i_engineSound,
                                   Theme *i_theme,
                                   BikerTheme *i_bikerTheme,
                                   const TColor &i_filterColor,
                                   const TColor &i_filterUglyColor)
  : Biker(i_physicsSettings,
          i_engineSound,
          i_theme,
          i_bikerTheme,
          i_filterColor,
          i_filterUglyColor) {
  m_BikeC = new BikeControllerPlayer();

  m_somersaultCounter.init();

  bFrontWheelTouching = false;
  bRearWheelTouching = false;

  bFrontWheelTouching = false;
  bRearWheelTouching = false;

  m_bSqueeking = false;
  m_nStillFrames = 0;
  m_clearDynamicTouched = false;
  m_lastSqueekTime = 0;

  initPhysics(i_gravity);
  initToPosition(i_position, i_direction, i_gravity);
  m_bikerHooks = NULL;
}

PlayerLocalBiker::~PlayerLocalBiker() {
  for (unsigned int i = 0; i < m_externalForces.size(); i++) {
    delete m_externalForces[i];
  }
  uninitPhysics();
  delete m_BikeC;
}

std::string PlayerLocalBiker::getDescription() const {
  return "";
}

std::string PlayerLocalBiker::getVeryQuickDescription() const {
  return GAMETEXT_PLAYER;
}

std::string PlayerLocalBiker::getQuickDescription() const {
  return GAMETEXT_PLAYER;
}

void PlayerLocalBiker::updateToTime(int i_time,
                                    int i_timeStep,
                                    CollisionSystem *i_collisionSystem,
                                    Vector2f i_gravity,
                                    Scene *i_motogame) {
  Biker::updateToTime(
    i_time, i_timeStep, i_collisionSystem, i_gravity, i_motogame);

  if (isFinished()) {
    return;
  }
  /* DONT UPDATE BELOW IF PLAYER FINISHED THE LEVEL */

  m_bSqueeking = false; /* no squeeking right now */
  updatePhysics(i_time, i_timeStep, i_collisionSystem, i_gravity);
  updateGameState();

  if (isDead()) {
    return;
  }
  /* DONT UPDATE BELOW IF PLAYER IS DEAD */

  /* Squeeking? */
  if (isSqueeking()) {
    if (i_time - m_lastSqueekTime > 100) {
      if (m_fHowMuchSqueek > 0.99) { // almost squeeking
        // printf("=>%.2f\n", m_fHowMuchSqueek);
        try {
          // Sound::playSampleByName("Textures/Sounds/Squeek.ogg",
          // m_fHowMuchSqueek);
        } catch (Exception &e) {
        }
        m_lastSqueekTime = i_time;
      }
    }
  }

  /* controler */
  if (m_BikeC->ChangeDir()) {
    m_BikeC->setChangeDir(false);
    m_bikeState->Dir =
      m_bikeState->Dir == DD_LEFT ? DD_RIGHT : DD_LEFT; /* switch */
    m_changeDirPer = 0.0;
  }

  /* somersault */
  bool bCounterclock;

  if (m_somersaultCounter.update(getAngle(), bCounterclock)) {
    if (m_bikerHooks != NULL) {
      m_bikerHooks->onSomersaultDone(bCounterclock);
    }
  }
}

double PlayerLocalBiker::getAngle() {
  double fAngle;

  fAngle = acos(m_bikeState->fFrameRot[0]);
  if (m_bikeState->fFrameRot[2] < 0.0f)
    fAngle = 2 * 3.14159f - fAngle;

  return fAngle;
}

void PlayerLocalBiker::initPhysics(Vector2f i_gravity) {
  m_bFirstPhysicsUpdate = true;

  /* Setup ODE */
  m_WorldID = dWorldCreate();
  dWorldSetERP(m_WorldID, m_physicsSettings->WorldErp());
  dWorldSetCFM(m_WorldID, m_physicsSettings->WorldCfm());

  dWorldSetGravity(m_WorldID, i_gravity.x, i_gravity.y, 0);

  m_ContactGroup = dJointGroupCreate(0);

  dWorldSetQuickStepNumIterations(
    m_WorldID, m_physicsSettings->SimulationStepIterations());

  m_bikeState->Parameters()->setDefaults(m_physicsSettings);
}

void PlayerLocalBiker::uninitPhysics(void) {
  dJointGroupDestroy(m_ContactGroup);
  dWorldDestroy(m_WorldID);
}

float PlayerLocalBiker::getTorsoVelocity() { // basically stolen from
  // getBikeLinearVel()
  Vector2f curpos, lastpos;

  curpos =
    m_bikeState->Dir == DD_RIGHT ? m_bikeState->HeadP : m_bikeState->Head2P;
  lastpos = m_bikeState->Dir == DD_RIGHT ? m_PrevHeadP : m_PrevHead2P;

  Vector2f delta = curpos - lastpos;
  float speed =
    (3.6 * 100 * sqrt(delta.x * delta.x + delta.y * delta.y)) /
    PHYS_STEP_SIZE; /* *100 because PHYS_STEP_SIZE is in hundreaths */

  /* protection against invalid values */
  if (speed > 500)
    return 0;

  return speed;
}

void PlayerLocalBiker::updatePhysics(int i_time,
                                     int i_timeStep,
                                     CollisionSystem *v_collisionSystem,
                                     Vector2f i_gravity) {
  /* No wheel spin per default */
  m_bWheelSpin = false;

  // remove old collision points
  cleanCollisionPoints();

  /* Update gravity vector */
  dWorldSetGravity(m_WorldID, i_gravity.x, i_gravity.y, 0);

  /* Should we disable stuff? Ok ODE has an autodisable feature, but i'd rather
     roll my own here. */
  const dReal *pfFront = dBodyGetLinearVel(m_FrontWheelBodyID);
  const dReal *pfRear = dBodyGetLinearVel(m_RearWheelBodyID);
  const dReal *pfFrame = dBodyGetLinearVel(m_FrameBodyID);

  if (fabs(pfFront[0]) < PHYS_SLEEP_EPS && fabs(pfFront[1]) < PHYS_SLEEP_EPS &&
      fabs(pfRear[0]) < PHYS_SLEEP_EPS && fabs(pfRear[1]) < PHYS_SLEEP_EPS &&
      fabs(pfFrame[0]) < PHYS_SLEEP_EPS && fabs(pfFrame[1]) < PHYS_SLEEP_EPS) {
    m_nStillFrames++;
  } else {
    m_nStillFrames = 0;
  }
  bool bSleep = false;

  // printf("{%d}\n",m_Collision.isDynamicTouched());
  // printf("]",m_nStillFrames);
  if (m_nStillFrames > PHYS_SLEEP_FRAMES && !m_clearDynamicTouched) {
    bSleep = true;
    dBodyDisable(m_FrontWheelBodyID);
    dBodyDisable(m_RearWheelBodyID);
    dBodyDisable(m_FrameBodyID);
  } else {
    if (!dBodyIsEnabled(m_FrontWheelBodyID))
      dBodyEnable(m_FrontWheelBodyID);
    if (!dBodyIsEnabled(m_RearWheelBodyID))
      dBodyEnable(m_RearWheelBodyID);
    if (!dBodyIsEnabled(m_FrameBodyID))
      dBodyEnable(m_FrameBodyID);
  }

  /* add external force */
  Vector2f v_forceToAdd = determineForceToAdd(i_time);
  if (v_forceToAdd != Vector2f(0.0, 0.0)) {
    resetAutoDisabler();
    dBodyEnable(m_FrameBodyID);
    dBodyAddForce(m_FrameBodyID, v_forceToAdd.x, v_forceToAdd.y, 0.0);
  }

  // printf("%d",bSleep);
  if (!bSleep) {
    /* Update front suspension */
    Vector2f Fq = m_bikeState->RFrontWheelP - m_bikeState->FrontWheelP;
    Vector2f Fqv = Fq - m_bikeState->PrevFq;
    Vector2f FSpring = Fq * m_physicsSettings->BikeSuspensionsSpring();
    Vector2f FDamp = Fqv * m_physicsSettings->BikeSuspensionsDamp();
    Vector2f FTotal = FSpring + FDamp;
    if (m_wheelDetach == false || m_bikeState->Dir == DD_LEFT) {
      dBodyAddForce(m_FrontWheelBodyID, FTotal.x, FTotal.y, 0);
      dBodyAddForceAtPos(m_FrameBodyID,
                         -FTotal.x,
                         -FTotal.y,
                         0,
                         m_bikeState->RFrontWheelP.x,
                         m_bikeState->RFrontWheelP.y,
                         0);
    }
    m_bikeState->PrevFq = Fq;

    /* Update rear suspension */
    Vector2f Rq = m_bikeState->RRearWheelP - m_bikeState->RearWheelP;
    Vector2f Rqv = Rq - m_bikeState->PrevRq;
    Vector2f RSpring = Rq * m_physicsSettings->BikeSuspensionsSpring();
    Vector2f RDamp = Rqv * m_physicsSettings->BikeSuspensionsDamp();
    Vector2f RTotal = RSpring + RDamp;
    if (m_wheelDetach == false || m_bikeState->Dir == DD_RIGHT) {
      dBodyAddForce(m_RearWheelBodyID, RTotal.x, RTotal.y, 0);
      dBodyAddForceAtPos(m_FrameBodyID,
                         -RTotal.x,
                         -RTotal.y,
                         0,
                         m_bikeState->RRearWheelP.x,
                         m_bikeState->RRearWheelP.y,
                         0);
    }
    m_bikeState->PrevRq = Rq;

    /* Have any of the suspensions reached the "squeek-point"? (rate of
       compression/decompression at
       which they will make squeeky noises) */
    if (Fqv.length() > PHYS_SUSP_SQUEEK_POINT ||
        Rqv.length() > PHYS_SUSP_SQUEEK_POINT) {
      /* Calculate how large a sqeek it should be */
      float fSqueekSize1 = Fqv.length() - PHYS_SUSP_SQUEEK_POINT;
      float fSqueekSize2 = Rqv.length() - PHYS_SUSP_SQUEEK_POINT;
      float fSqueekSize =
        fSqueekSize1 > fSqueekSize2 ? fSqueekSize1 : fSqueekSize2;
      float fMaxSqueek = 0.11f;
      float fL = fSqueekSize / fMaxSqueek;
      if (fL > 1.0f)
        fL = 1.0f;
      m_fHowMuchSqueek = fL;
      // printf("%f\n",fL);

      m_bSqueeking = true;
    }
  }

  /* Apply attitude control (SIMPLISTIC!) */
  // when you want to rotate in opposite direction  (m_BikeC->Pull() *
  // m_fLastAttitudeDir < 0) it's true
  //  benetnash: I don't think It will affect highscores in any way
  if (isDead() == false && isFinished() == false) {
    if ((m_BikeC->Pull() != 0.0f) &&
        (i_time / 100.0 > m_fNextAttitudeCon /*XXX*/ ||
         (m_BikeC->Pull() * m_fLastAttitudeDir < 0) /*XXX*/)) {
      m_fAttitudeCon =
        m_BikeC->Pull() * m_physicsSettings->RiderAttitudeTorque();
      m_fLastAttitudeDir = m_fAttitudeCon;
      m_fNextAttitudeCon = (i_time / 100.0) + (0.6f * fabsf(m_BikeC->Pull()));
    }
  }

  if (m_fAttitudeCon != 0.0f) {
    m_nStillFrames = 0;
    dBodyEnable(m_FrontWheelBodyID);
    dBodyEnable(m_RearWheelBodyID);
    dBodyEnable(m_FrameBodyID);
    dBodyAddTorque(m_FrameBodyID, 0, 0, m_fAttitudeCon);

    // printf("AttitudeCon %f\n",m_fAttitudeCon);
  }

  m_fAttitudeCon *= m_physicsSettings->RiderAttitudeDefactor();
  if (fabs(m_fAttitudeCon) < 100) { /* make sure we glue to zero */
    m_fAttitudeCon = 0.0f;
  }

  float fRearWheelAngVel = dBodyGetAngularVel(m_RearWheelBodyID)[2];
  float fFrontWheelAngVel = dBodyGetAngularVel(m_FrontWheelBodyID)[2];

  /* Misc */
  if (!bSleep) {
    if (fRearWheelAngVel > -(m_physicsSettings->BikeWheelRoll_velocityMax()) &&
        fRearWheelAngVel < m_physicsSettings->BikeWheelRoll_velocityMax())
      dBodyAddTorque(m_RearWheelBodyID,
                     0,
                     0,
                     -dBodyGetAngularVel(m_RearWheelBodyID)[2] *
                       m_physicsSettings->BikeWheelRoll_resistance());
    else
      dBodyAddTorque(m_RearWheelBodyID,
                     0,
                     0,
                     -dBodyGetAngularVel(m_RearWheelBodyID)[2] *
                       m_physicsSettings->BikeWheelRoll_resistanceMax());

    if (fFrontWheelAngVel > -(m_physicsSettings->BikeWheelRoll_velocityMax()) &&
        fFrontWheelAngVel < m_physicsSettings->BikeWheelRoll_velocityMax())
      dBodyAddTorque(m_FrontWheelBodyID,
                     0,
                     0,
                     -dBodyGetAngularVel(m_FrontWheelBodyID)[2] *
                       m_physicsSettings->BikeWheelRoll_resistance());
    else
      dBodyAddTorque(m_FrontWheelBodyID,
                     0,
                     0,
                     -dBodyGetAngularVel(m_FrontWheelBodyID)[2] *
                       m_physicsSettings->BikeWheelRoll_resistanceMax());
  }

  /* Update RPM */
  if (isDead() == false && isFinished() == false) {
    /* Simply map the wheel ang vel to the RPM (stupid, lame, lots of bad stuff)
     */
    if (m_bikeState->Dir == DD_RIGHT) {
      float f = -fRearWheelAngVel;
      if (f < 0.0f)
        f = 0.0f;
      m_bikeState->fBikeEngineRPM =
        m_physicsSettings->EngineRpmMin() +
        (m_physicsSettings->EngineRpmMax() -
         m_physicsSettings->EngineRpmMin()) *
          (f / m_physicsSettings->BikeWheelRoll_velocityMax()) *
          m_BikeC->Drive();
      if (m_bikeState->fBikeEngineRPM < m_physicsSettings->EngineRpmMin())
        m_bikeState->fBikeEngineRPM = m_physicsSettings->EngineRpmMin();
      if (m_bikeState->fBikeEngineRPM > m_physicsSettings->EngineRpmMax())
        m_bikeState->fBikeEngineRPM = m_physicsSettings->EngineRpmMax();
    } else if (m_bikeState->Dir == DD_LEFT) {
      float f = fFrontWheelAngVel;
      if (f < 0.0f)
        f = 0.0f;
      m_bikeState->fBikeEngineRPM =
        m_physicsSettings->EngineRpmMin() +
        (m_physicsSettings->EngineRpmMax() -
         m_physicsSettings->EngineRpmMin()) *
          (f / m_physicsSettings->BikeWheelRoll_velocityMax()) *
          m_BikeC->Drive();
      if (m_bikeState->fBikeEngineRPM < m_physicsSettings->EngineRpmMin())
        m_bikeState->fBikeEngineRPM = m_physicsSettings->EngineRpmMin();
      if (m_bikeState->fBikeEngineRPM > m_physicsSettings->EngineRpmMax())
        m_bikeState->fBikeEngineRPM = m_physicsSettings->EngineRpmMax();
    }
  }

  /* Apply motor/brake torques */
  if (isDead() == false && isFinished() == false) {
    if (m_BikeC->Drive() < 0.0f) {
      /* Brake! */
      if (!bSleep) {
        // printf("Brake!\n");

        dBodyAddTorque(m_RearWheelBodyID,
                       0,
                       0,
                       dBodyGetAngularVel(m_RearWheelBodyID)[2] *
                         m_physicsSettings->BikeBrakeFactor() *
                         m_BikeC->Drive());
        dBodyAddTorque(m_FrontWheelBodyID,
                       0,
                       0,
                       dBodyGetAngularVel(m_FrontWheelBodyID)[2] *
                         m_physicsSettings->BikeBrakeFactor() *
                         m_BikeC->Drive());
      }
    } else {
      /* Throttle? */
      if (m_BikeC->Drive() > 0.0f) {
        if (m_bikeState->Dir == DD_RIGHT) {
          if (fRearWheelAngVel >
              -(m_physicsSettings->BikeWheelRoll_velocityMax())) {
            m_nStillFrames = 0;
            dBodyEnable(m_FrontWheelBodyID);
            dBodyEnable(m_RearWheelBodyID);
            dBodyEnable(m_FrameBodyID);
            dBodyAddTorque(m_RearWheelBodyID,
                           0,
                           0,
                           -m_bikeState->Parameters()->MaxEngine() *
                             m_physicsSettings->EngineDamp() *
                             m_BikeC->Drive());

            // printf("Drive!\n");
          }
        } else {
          if (fFrontWheelAngVel <
              m_physicsSettings->BikeWheelRoll_velocityMax()) {
            m_nStillFrames = 0;
            dBodyEnable(m_FrontWheelBodyID);
            dBodyEnable(m_RearWheelBodyID);
            dBodyEnable(m_FrameBodyID);
            dBodyAddTorque(m_FrontWheelBodyID,
                           0,
                           0,
                           m_bikeState->Parameters()->MaxEngine() *
                             m_physicsSettings->EngineDamp() *
                             m_BikeC->Drive());
          }
        }
      }
    }
  }

  /* Perform collision detection between the bike and the level blocks */
  v_collisionSystem->clearDynamicTouched();

  int nNumContacts;
  dContact Contacts[100];

  static int nSlipFrameInterlace =
    0; /* Okay, lazy approach again. Front wheel can generate particles
          even frames, rear wheel odd frames */
  nSlipFrameInterlace++;

  nNumContacts = intersectWheelLevel(m_bikeState->FrontWheelP,
                                     m_bikeState->Parameters()->WheelRadius(),
                                     Contacts,
                                     v_collisionSystem);
  if (nNumContacts > 0) {
    if (bFrontWheelTouching == false) {
      bFrontWheelTouching = true;
      if (m_bikerHooks != NULL) {
        m_bikerHooks->onWheelTouches(1, true);
      }
    }
  } else {
    if (bFrontWheelTouching) {
      bFrontWheelTouching = false;
      if (m_bikerHooks != NULL) {
        m_bikerHooks->onWheelTouches(1, false);
      }
    }
  }
  if (v_collisionSystem->isDynamicTouched()) {
    if (!dBodyIsEnabled(m_FrontWheelBodyID))
      dBodyEnable(m_FrontWheelBodyID);
    if (!dBodyIsEnabled(m_RearWheelBodyID))
      dBodyEnable(m_RearWheelBodyID);
    if (!dBodyIsEnabled(m_FrameBodyID))
      dBodyEnable(m_FrameBodyID);
  }
  if (dBodyIsEnabled(m_FrontWheelBodyID)) {
    Vector2f WSP;
    for (int i = 0; i < nNumContacts; i++) {
      dJointAttach(dJointCreateContact(m_WorldID, m_ContactGroup, &Contacts[i]),
                   m_FrontWheelBodyID,
                   0);
      WSP.x = Contacts[i].geom.pos[0];
      WSP.y = Contacts[i].geom.pos[1];
      m_collisionPoints.push_back(
        Vector2f(Contacts[i].geom.pos[0], Contacts[i].geom.pos[1]));
    }

    // if(nNumContacts > 0 && nSlipFrameInterlace&1  &&
    // sqrt(pfFrame[0]*pfFrame[0] + pfFrame[1]*pfFrame[1])>1.2f) {
    //  Vector2f WSPvel( (-(m_bikeState->FrontWheelP.y - WSP.y)),
    //                   ((m_bikeState->FrontWheelP.x - WSP.x)) );
    //  WSPvel.normalize();
    //  WSPvel *= -m_BikeP.WR * fFrontWheelAngVel;
    //
    //  Vector2f Vs;
    //  Vs.x = WSPvel.x + pfRear[0];
    //  Vs.y = WSPvel.y + pfRear[1];
    //
    //  if(Vs.length() > 1.25) {
    //    m_bWheelSpin = true;
    //    m_WheelSpinPoint = WSP;
    //    m_WheelSpinDir = Vs * 0.1f;

    //    //Vector2f Zz = (m_WheelSpinDir + Vector2f(m_bikeState->FrontWheelP.x
    //    - WSP.x,m_bikeState->FrontWheelP.y - WSP.y)) / 2.0f;
    //    //m_WheelSpinDir = Zz;
    //  }
    //}

    if (m_bikeState->Dir == DD_LEFT) {
      if (isDead() == false && isFinished() == false) {
        if (fabs(fFrontWheelAngVel) > 5 && m_BikeC->Drive() > 0.0f &&
            nNumContacts > 0) {
          m_bWheelSpin = true;
          m_WheelSpinPoint = WSP;
          m_WheelSpinDir.x = (((m_bikeState->FrontWheelP.y - WSP.y)) * 1 +
                              (m_bikeState->FrontWheelP.x - WSP.x)) /
                             2;
          m_WheelSpinDir.y = ((-(m_bikeState->FrontWheelP.x - WSP.x)) * 1 +
                              (m_bikeState->FrontWheelP.y - WSP.y)) /
                             2;
        }
      }
    }
  }

  nNumContacts = intersectWheelLevel(m_bikeState->RearWheelP,
                                     m_bikeState->Parameters()->WheelRadius(),
                                     Contacts,
                                     v_collisionSystem);
  if (nNumContacts > 0) {
    if (bRearWheelTouching == false) {
      bRearWheelTouching = true;
      if (m_bikerHooks != NULL) {
        m_bikerHooks->onWheelTouches(2, true);
      }
    }
  } else {
    if (bRearWheelTouching) {
      bRearWheelTouching = false;
      if (m_bikerHooks != NULL) {
        m_bikerHooks->onWheelTouches(2, false);
      }
    }
  }

  if (v_collisionSystem->isDynamicTouched()) {
    if (!dBodyIsEnabled(m_FrontWheelBodyID))
      dBodyEnable(m_FrontWheelBodyID);
    if (!dBodyIsEnabled(m_RearWheelBodyID))
      dBodyEnable(m_RearWheelBodyID);
    if (!dBodyIsEnabled(m_FrameBodyID))
      dBodyEnable(m_FrameBodyID);
  }
  if (dBodyIsEnabled(m_RearWheelBodyID)) {
    Vector2f WSP;
    for (int i = 0; i < nNumContacts; i++) {
      dJointAttach(dJointCreateContact(m_WorldID, m_ContactGroup, &Contacts[i]),
                   m_RearWheelBodyID,
                   0);
      WSP.x = Contacts[i].geom.pos[0];
      WSP.y = Contacts[i].geom.pos[1];
      m_collisionPoints.push_back(
        Vector2f(Contacts[i].geom.pos[0], Contacts[i].geom.pos[1]));
    }

    /* Calculate wheel linear velocity at slip-point */
    // if(nNumContacts > 0 && !(nSlipFrameInterlace&1) &&
    // sqrt(pfFrame[0]*pfFrame[0] + pfFrame[1]*pfFrame[1])>1.2f) {
    //  Vector2f WSPvel( (-(m_bikeState->RearWheelP.y - WSP.y)),
    //                   ((m_bikeState->RearWheelP.x - WSP.x)) );
    //  WSPvel.normalize();
    //  WSPvel *= -m_BikeP.WR * fRearWheelAngVel;
    //
    //  Vector2f Vs;
    //  Vs.x = WSPvel.x + pfRear[0];
    //  Vs.y = WSPvel.y + pfRear[1];
    //
    //  if(Vs.length() > 1.25) {
    //    m_bWheelSpin = true;
    //    m_WheelSpinPoint = WSP;
    //    m_WheelSpinDir = Vs * 0.1f;
    //
    //    //Vector2f Zz = (m_WheelSpinDir + Vector2f(m_bikeState->RearWheelP.x -
    //    WSP.x,m_bikeState->RearWheelP.y - WSP.y)) / 2.0f;
    //    //m_WheelSpinDir = Zz;
    //  }
    //}

    if (m_bikeState->Dir == DD_RIGHT) {
      if (isDead() == false && isFinished() == false) {
        if (fabs(fRearWheelAngVel) > 5 && m_BikeC->Drive() > 0 &&
            nNumContacts > 0) {
          m_bWheelSpin = true;
          m_WheelSpinPoint = WSP;
          m_WheelSpinDir.x = ((-(m_bikeState->RearWheelP.y - WSP.y)) * 1 +
                              (m_bikeState->RearWheelP.x - WSP.x)) /
                             2;
          m_WheelSpinDir.y = (((m_bikeState->RearWheelP.x - WSP.x)) * 1 +
                              (m_bikeState->RearWheelP.y - WSP.y)) /
                             2;
        }
      }
    }
  }

  /* body */
  if (m_bodyDetach) {
    float v_memberRay = 0.1;
    float v_detachGrip = 0.8;

    // ShoulderP
    if (m_bikeState->Dir == DD_RIGHT) {
      nNumContacts = intersectBodyLevel(
        m_bikeState->ShoulderP, v_memberRay, Contacts, v_collisionSystem);
    } else {
      nNumContacts = intersectBodyLevel(
        m_bikeState->Shoulder2P, v_memberRay, Contacts, v_collisionSystem);
    }
    for (int i = 0; i < nNumContacts; i++) {
      Contacts[i].surface.mu = v_detachGrip;
      dJointAttach(dJointCreateContact(m_WorldID, m_ContactGroup, &Contacts[i]),
                   m_bikeState->Dir == DD_RIGHT ? m_PlayerTorsoBodyID
                                                : m_PlayerTorsoBodyID2,
                   0);
      dJointAttach(dJointCreateContact(m_WorldID, m_ContactGroup, &Contacts[i]),
                   m_bikeState->Dir == DD_RIGHT ? m_PlayerUArmBodyID
                                                : m_PlayerUArmBodyID2,
                   0);
      m_collisionPoints.push_back(
        Vector2f(Contacts[i].geom.pos[0], Contacts[i].geom.pos[1]));
    }

    // LowerBodyP
    if (m_bikeState->Dir == DD_RIGHT) {
      nNumContacts = intersectBodyLevel(
        m_bikeState->LowerBodyP, v_memberRay, Contacts, v_collisionSystem);
    } else {
      nNumContacts = intersectBodyLevel(
        m_bikeState->LowerBody2P, v_memberRay, Contacts, v_collisionSystem);
    }
    for (int i = 0; i < nNumContacts; i++) {
      Contacts[i].surface.mu = v_detachGrip;
      dJointAttach(dJointCreateContact(m_WorldID, m_ContactGroup, &Contacts[i]),
                   m_bikeState->Dir == DD_RIGHT ? m_PlayerTorsoBodyID
                                                : m_PlayerTorsoBodyID2,
                   0);
      dJointAttach(dJointCreateContact(m_WorldID, m_ContactGroup, &Contacts[i]),
                   m_bikeState->Dir == DD_RIGHT ? m_PlayerULegBodyID
                                                : m_PlayerULegBodyID2,
                   0);
      m_collisionPoints.push_back(
        Vector2f(Contacts[i].geom.pos[0], Contacts[i].geom.pos[1]));
    }

    // ElbowP
    if (m_bikeState->Dir == DD_RIGHT) {
      nNumContacts = intersectBodyLevel(
        m_bikeState->ElbowP, v_memberRay, Contacts, v_collisionSystem);
    } else {
      nNumContacts = intersectBodyLevel(
        m_bikeState->Elbow2P, v_memberRay, Contacts, v_collisionSystem);
    }
    for (int i = 0; i < nNumContacts; i++) {
      Contacts[i].surface.mu = v_detachGrip;
      dJointAttach(dJointCreateContact(m_WorldID, m_ContactGroup, &Contacts[i]),
                   m_bikeState->Dir == DD_RIGHT ? m_PlayerUArmBodyID
                                                : m_PlayerUArmBodyID2,
                   0);
      dJointAttach(dJointCreateContact(m_WorldID, m_ContactGroup, &Contacts[i]),
                   m_bikeState->Dir == DD_RIGHT ? m_PlayerLArmBodyID
                                                : m_PlayerLArmBodyID2,
                   0);
      m_collisionPoints.push_back(
        Vector2f(Contacts[i].geom.pos[0], Contacts[i].geom.pos[1]));
    }

    // HandP
    if (m_bikeState->Dir == DD_RIGHT) {
      nNumContacts = intersectBodyLevel(
        m_bikeState->HandP, v_memberRay, Contacts, v_collisionSystem);
    } else {
      nNumContacts = intersectBodyLevel(
        m_bikeState->Hand2P, v_memberRay, Contacts, v_collisionSystem);
    }
    for (int i = 0; i < nNumContacts; i++) {
      Contacts[i].surface.mu = v_detachGrip;
      dJointAttach(dJointCreateContact(m_WorldID, m_ContactGroup, &Contacts[i]),
                   m_bikeState->Dir == DD_RIGHT ? m_PlayerLArmBodyID
                                                : m_PlayerLArmBodyID2,
                   0);
      m_collisionPoints.push_back(
        Vector2f(Contacts[i].geom.pos[0], Contacts[i].geom.pos[1]));
    }

    // KneeP
    if (m_bikeState->Dir == DD_RIGHT) {
      nNumContacts = intersectBodyLevel(
        m_bikeState->KneeP, v_memberRay, Contacts, v_collisionSystem);
    } else {
      nNumContacts = intersectBodyLevel(
        m_bikeState->Knee2P, v_memberRay, Contacts, v_collisionSystem);
    }
    for (int i = 0; i < nNumContacts; i++) {
      Contacts[i].surface.mu = v_detachGrip;
      dJointAttach(dJointCreateContact(m_WorldID, m_ContactGroup, &Contacts[i]),
                   m_bikeState->Dir == DD_RIGHT ? m_PlayerULegBodyID
                                                : m_PlayerULegBodyID2,
                   0);
      dJointAttach(dJointCreateContact(m_WorldID, m_ContactGroup, &Contacts[i]),
                   m_bikeState->Dir == DD_RIGHT ? m_PlayerLLegBodyID
                                                : m_PlayerLLegBodyID2,
                   0);
      m_collisionPoints.push_back(
        Vector2f(Contacts[i].geom.pos[0], Contacts[i].geom.pos[1]));
    }

    // FootP
    if (m_bikeState->Dir == DD_RIGHT) {
      nNumContacts = intersectBodyLevel(
        m_bikeState->FootP, v_memberRay, Contacts, v_collisionSystem);
    } else {
      nNumContacts = intersectBodyLevel(
        m_bikeState->Foot2P, v_memberRay, Contacts, v_collisionSystem);
    }
    for (int i = 0; i < nNumContacts; i++) {
      Contacts[i].surface.mu = v_detachGrip;
      dJointAttach(dJointCreateContact(m_WorldID, m_ContactGroup, &Contacts[i]),
                   m_bikeState->Dir == DD_RIGHT ? m_PlayerLLegBodyID
                                                : m_PlayerLLegBodyID2,
                   0);
      m_collisionPoints.push_back(
        Vector2f(Contacts[i].geom.pos[0], Contacts[i].geom.pos[1]));
    }
  }

  /* Player head */
  if (m_bikeState->Dir == DD_RIGHT) {
    if (intersectHeadLevel(m_bikeState->HeadP,
                           m_bikeState->Parameters()->HeadSize(),
                           m_PrevActiveHead,
                           v_collisionSystem)) {
      if (m_bikerHooks != NULL) {
        m_bikerHooks->onHeadTouches();
      }
    }

    m_PrevActiveHead = m_bikeState->HeadP;
  } else if (m_bikeState->Dir == DD_LEFT) {
    if (intersectHeadLevel(m_bikeState->Head2P,
                           m_bikeState->Parameters()->HeadSize(),
                           m_PrevActiveHead,
                           v_collisionSystem)) {
      if (m_bikerHooks != NULL) {
        m_bikerHooks->onHeadTouches();
      }
    }

    m_PrevActiveHead = m_bikeState->Head2P;
  }

  m_PrevFrontWheelP = m_bikeState->FrontWheelP;
  m_PrevRearWheelP = m_bikeState->RearWheelP;
  m_PrevHeadP = m_bikeState->HeadP;
  m_PrevHead2P = m_bikeState->Head2P;

  /* Move rider along bike -- calculate handlebar and footpeg coords */
  Vector2f FootRP, HandRP;

  FootRP = m_bikeState->WantedFootP;
  HandRP = m_bikeState->WantedHandP;

  Vector2f PFq = FootRP - m_bikeState->FootP;
  Vector2f PFqv = PFq - m_bikeState->PrevPFq;
  Vector2f PFSpring = PFq * m_physicsSettings->RiderSpring();
  Vector2f PFDamp = PFqv * m_physicsSettings->RiderDamp();
  Vector2f PFTotal = PFSpring + PFDamp;
  if (m_bodyDetach == false) {
    dBodyAddForce(m_PlayerFootAnchorBodyID, PFTotal.x, PFTotal.y, 0);
  }

  m_bikeState->PrevPFq = PFq;

  Vector2f PHq = HandRP - m_bikeState->HandP;
  Vector2f PHqv = PHq - m_bikeState->PrevPHq;
  Vector2f PHSpring = PHq * m_physicsSettings->RiderSpring();
  Vector2f PHDamp = PHqv * m_physicsSettings->RiderDamp();
  Vector2f PHTotal = PHSpring + PHDamp;
  if (m_bodyDetach == false) {
    dBodyAddForce(m_PlayerHandAnchorBodyID, PHTotal.x, PHTotal.y, 0);
  }
  m_bikeState->PrevPHq = PHq;

  FootRP = m_bikeState->WantedFoot2P;
  HandRP = m_bikeState->WantedHand2P;

  PFq = FootRP - m_bikeState->Foot2P;
  PFqv = PFq - m_bikeState->PrevPFq2;
  PFSpring = PFq * m_physicsSettings->RiderSpring();
  PFDamp = PFqv * m_physicsSettings->RiderDamp();
  PFTotal = PFSpring + PFDamp;
  if (m_bodyDetach == false) {
    dBodyAddForce(m_PlayerFootAnchorBodyID2, PFTotal.x, PFTotal.y, 0);
  }
  m_bikeState->PrevPFq2 = PFq;

  PHq = HandRP - m_bikeState->Hand2P;
  PHqv = PHq - m_bikeState->PrevPHq2;
  PHSpring = PHq * m_physicsSettings->RiderSpring();
  PHDamp = PHqv * m_physicsSettings->RiderDamp();
  PHTotal = PHSpring + PHDamp;
  if (m_bodyDetach == false) {
    dBodyAddForce(m_PlayerHandAnchorBodyID2, PHTotal.x, PHTotal.y, 0);
  }
  m_bikeState->PrevPHq2 = PHq;

  /* Perform world simulation step */
  dWorldQuickStep(m_WorldID,
                  ((float)i_timeStep / 100.0) *
                    m_physicsSettings->SimulationSpeedFactor());
  // dWorldStep(m_WorldID,fTimeStep*PHYS_SPEED);

  /* Empty contact joint group */
  dJointGroupEmpty(m_ContactGroup);

  m_clearDynamicTouched = v_collisionSystem->isDynamicTouched();

  m_bFirstPhysicsUpdate = false;
}

bool PlayerLocalBiker::intersectHeadLevel(Vector2f Cp,
                                          float Cr,
                                          const Vector2f &LastCp,
                                          CollisionSystem *v_collisionSystem) {
  // check the circle
  if (v_collisionSystem->checkCircle(Cp.x, Cp.y, Cr))
    return true;

  // check if the line between the line of 2 head moves. (including when you
  // change directory to not use this to go throw walls)
  if (!m_bFirstPhysicsUpdate) {
    dContact c[100];
    int nNumContacts = v_collisionSystem->collideLine(
      LastCp.x, LastCp.y, Cp.x, Cp.y, c, 100, m_physicsSettings);
    if (nNumContacts > 0) {
      return true;
    }
  }

  return false;
}

int PlayerLocalBiker::intersectBodyLevel(Vector2f Cp,
                                         float Cr,
                                         dContact *pContacts,
                                         CollisionSystem *v_collisionSystem) {
  int nNumContacts = v_collisionSystem->collideCircle(
    Cp.x, Cp.y, Cr, pContacts, 100, m_physicsSettings);
  if (nNumContacts == 0) {
    /* Nothing... but what if we are moving so fast that the circle has moved
       all the way through some geometry? Check it's path. */
    // nNumContacts = m_Collision.collideLine(
  }
  return nNumContacts;
}

int PlayerLocalBiker::intersectWheelLevel(Vector2f Cp,
                                          float Cr,
                                          dContact *pContacts,
                                          CollisionSystem *v_collisionSystem) {
  int nNumContacts = v_collisionSystem->collideCircle(
    Cp.x, Cp.y, Cr, pContacts, 100, m_physicsSettings);
  if (nNumContacts == 0) {
    /* Nothing... but what if we are moving so fast that the circle has moved
       all the way through some geometry? Check it's path. */
    // nNumContacts = m_Collision.collideLine(
  } else {
    // detach the wheel if the player is dead and the velocity is too much
    if (isDead() &&
        getBikeLinearVel() > m_physicsSettings->DeadWheelDetachSpeed()) {
      m_wheelDetach = true;
    }
  }
  return nNumContacts;
}

void PlayerLocalBiker::initToPosition(Vector2f i_position,
                                      DriveDir i_direction,
                                      Vector2f i_gravity) {
  /* Clear stuff */
  clearStates();

  m_fNextAttitudeCon = -1000.0f;
  m_fAttitudeCon = 0.0f;

  m_PlayerFootAnchorBodyID = NULL;
  m_PlayerHandAnchorBodyID = NULL;
  m_PlayerTorsoBodyID = NULL;
  m_PlayerUArmBodyID = NULL;
  m_PlayerLArmBodyID = NULL;
  m_PlayerULegBodyID = NULL;
  m_PlayerLLegBodyID = NULL;
  m_PlayerFootAnchorBodyID2 = NULL;
  m_PlayerHandAnchorBodyID2 = NULL;
  m_PlayerTorsoBodyID2 = NULL;
  m_PlayerUArmBodyID2 = NULL;
  m_PlayerLArmBodyID2 = NULL;
  m_PlayerULegBodyID2 = NULL;
  m_PlayerLLegBodyID2 = NULL;

  /* Restart physics */
  uninitPhysics();
  initPhysics(i_gravity);

  resetAutoDisabler();

  /* Calculate bike stuff */
  m_bikeState->reInitializeAnchors();
  Vector2f C(i_position - m_bikeState->Anchors()->GroundPoint());
  prepareBikePhysics(C);

  setBodyDetach(false);

  m_bikeState->Dir = i_direction;

  updateGameState();
}

float PlayerLocalBiker::getBikeEngineSpeed() {
  float fWheelAngVel;
  float speed;

  if (m_bikeState->Dir == DD_RIGHT) {
    fWheelAngVel = dBodyGetAngularVel(m_RearWheelBodyID)[2];
  } else {
    fWheelAngVel = dBodyGetAngularVel(m_FrontWheelBodyID)[2];
  }

  speed = (fWheelAngVel * m_physicsSettings->BikeWheelRadius() * 3.6);
  return speed >= 0.0 ? speed : -speed;
}

float PlayerLocalBiker::getBikeLinearVel() {
  Vector2f curpos =
    (m_bikeState->RearWheelP +
     m_bikeState->FrontWheelP); // adding both rear and front to manage the side
  Vector2f lastpos = (m_PrevRearWheelP + m_PrevFrontWheelP);
  Vector2f delta = curpos - lastpos;
  float speed =
    (3.6 * 100 * sqrt(delta.x * delta.x + delta.y * delta.y)) /
    PHYS_STEP_SIZE; /* *100 because PHYS_STEP_SIZE is in hundreaths */

  /* protection against invalid values */
  if (speed > 500)
    return 0;

  return speed;
}

void PlayerLocalBiker::updateGameState() {
  /* Nope... Get current bike state */
  m_bikeState->RearWheelP.x = ((dReal *)dBodyGetPosition(m_RearWheelBodyID))[0];
  m_bikeState->RearWheelP.y = ((dReal *)dBodyGetPosition(m_RearWheelBodyID))[1];
  m_bikeState->FrontWheelP.x =
    ((dReal *)dBodyGetPosition(m_FrontWheelBodyID))[0];
  m_bikeState->FrontWheelP.y =
    ((dReal *)dBodyGetPosition(m_FrontWheelBodyID))[1];
  m_bikeState->CenterP.x = ((dReal *)dBodyGetPosition(m_FrameBodyID))[0];
  m_bikeState->CenterP.y = ((dReal *)dBodyGetPosition(m_FrameBodyID))[1];

  m_bikeState->fFrameRot[0] = ((dReal *)dBodyGetRotation(m_FrameBodyID))[0];
  m_bikeState->fFrameRot[1] = ((dReal *)dBodyGetRotation(m_FrameBodyID))[1];
  m_bikeState->fFrameRot[2] = ((dReal *)dBodyGetRotation(m_FrameBodyID))[4];
  m_bikeState->fFrameRot[3] = ((dReal *)dBodyGetRotation(m_FrameBodyID))[5];

  m_bikeState->fFrontWheelRot[0] =
    ((dReal *)dBodyGetRotation(m_FrontWheelBodyID))[0];
  m_bikeState->fFrontWheelRot[1] =
    ((dReal *)dBodyGetRotation(m_FrontWheelBodyID))[1];
  m_bikeState->fFrontWheelRot[2] =
    ((dReal *)dBodyGetRotation(m_FrontWheelBodyID))[4];
  m_bikeState->fFrontWheelRot[3] =
    ((dReal *)dBodyGetRotation(m_FrontWheelBodyID))[5];

  m_bikeState->fRearWheelRot[0] =
    ((dReal *)dBodyGetRotation(m_RearWheelBodyID))[0];
  m_bikeState->fRearWheelRot[1] =
    ((dReal *)dBodyGetRotation(m_RearWheelBodyID))[1];
  m_bikeState->fRearWheelRot[2] =
    ((dReal *)dBodyGetRotation(m_RearWheelBodyID))[4];
  m_bikeState->fRearWheelRot[3] =
    ((dReal *)dBodyGetRotation(m_RearWheelBodyID))[5];

  m_bikeState->SwingAnchorP.x =
    m_bikeState->Anchors()->AR.x * m_bikeState->fFrameRot[0] +
    m_bikeState->Anchors()->AR.y * m_bikeState->fFrameRot[1] +
    m_bikeState->CenterP.x;
  m_bikeState->SwingAnchorP.y =
    m_bikeState->Anchors()->AR.x * m_bikeState->fFrameRot[2] +
    m_bikeState->Anchors()->AR.y * m_bikeState->fFrameRot[3] +
    m_bikeState->CenterP.y;

  m_bikeState->SwingAnchor2P.x =
    m_bikeState->Anchors()->AR2.x * m_bikeState->fFrameRot[0] +
    m_bikeState->Anchors()->AR2.y * m_bikeState->fFrameRot[1] +
    m_bikeState->CenterP.x;
  m_bikeState->SwingAnchor2P.y =
    m_bikeState->Anchors()->AR2.x * m_bikeState->fFrameRot[2] +
    m_bikeState->Anchors()->AR2.y * m_bikeState->fFrameRot[3] +
    m_bikeState->CenterP.y;

  m_bikeState->FrontAnchorP.x =
    m_bikeState->Anchors()->AF.x * m_bikeState->fFrameRot[0] +
    m_bikeState->Anchors()->AF.y * m_bikeState->fFrameRot[1] +
    m_bikeState->CenterP.x;
  m_bikeState->FrontAnchor2P.x =
    m_bikeState->Anchors()->AF2.x * m_bikeState->fFrameRot[0] +
    m_bikeState->Anchors()->AF2.y * m_bikeState->fFrameRot[1] +
    m_bikeState->CenterP.x;

  m_bikeState->FrontAnchorP.y =
    m_bikeState->Anchors()->AF.x * m_bikeState->fFrameRot[2] +
    m_bikeState->Anchors()->AF.y * m_bikeState->fFrameRot[3] +
    m_bikeState->CenterP.y;
  m_bikeState->FrontAnchor2P.y =
    m_bikeState->Anchors()->AF2.x * m_bikeState->fFrameRot[2] +
    m_bikeState->Anchors()->AF2.y * m_bikeState->fFrameRot[3] +
    m_bikeState->CenterP.y;

  /* Calculate desired hand/foot positions */
  m_bikeState->WantedFootP.x =
    m_bikeState->Anchors()->PFp.x * m_bikeState->fFrameRot[0] +
    m_bikeState->Anchors()->PFp.y * m_bikeState->fFrameRot[1] +
    m_bikeState->CenterP.x;
  m_bikeState->WantedFootP.y =
    m_bikeState->Anchors()->PFp.x * m_bikeState->fFrameRot[2] +
    m_bikeState->Anchors()->PFp.y * m_bikeState->fFrameRot[3] +
    m_bikeState->CenterP.y;
  m_bikeState->WantedHandP.x =
    m_bikeState->Anchors()->PHp.x * m_bikeState->fFrameRot[0] +
    m_bikeState->Anchors()->PHp.y * m_bikeState->fFrameRot[1] +
    m_bikeState->CenterP.x;
  m_bikeState->WantedHandP.y =
    m_bikeState->Anchors()->PHp.x * m_bikeState->fFrameRot[2] +
    m_bikeState->Anchors()->PHp.y * m_bikeState->fFrameRot[3] +
    m_bikeState->CenterP.y;

  m_bikeState->WantedFoot2P.x =
    m_bikeState->Anchors()->PFp2.x * m_bikeState->fFrameRot[0] +
    m_bikeState->Anchors()->PFp2.y * m_bikeState->fFrameRot[1] +
    m_bikeState->CenterP.x;
  m_bikeState->WantedFoot2P.y =
    m_bikeState->Anchors()->PFp2.x * m_bikeState->fFrameRot[2] +
    m_bikeState->Anchors()->PFp2.y * m_bikeState->fFrameRot[3] +
    m_bikeState->CenterP.y;
  m_bikeState->WantedHand2P.x =
    m_bikeState->Anchors()->PHp2.x * m_bikeState->fFrameRot[0] +
    m_bikeState->Anchors()->PHp2.y * m_bikeState->fFrameRot[1] +
    m_bikeState->CenterP.x;
  m_bikeState->WantedHand2P.y =
    m_bikeState->Anchors()->PHp2.x * m_bikeState->fFrameRot[2] +
    m_bikeState->Anchors()->PHp2.y * m_bikeState->fFrameRot[3] +
    m_bikeState->CenterP.y;

  dVector3 T;
  dJointGetHingeAnchor(m_HandHingeID, T);
  m_bikeState->HandP.x = T[0];
  m_bikeState->HandP.y = T[1];

  dJointGetHingeAnchor(m_ElbowHingeID, T);
  m_bikeState->ElbowP.x = T[0];
  m_bikeState->ElbowP.y = T[1];

  dJointGetHingeAnchor(m_ShoulderHingeID, T);
  m_bikeState->ShoulderP.x = T[0];
  m_bikeState->ShoulderP.y = T[1];

  dJointGetHingeAnchor(m_LowerBodyHingeID, T);
  m_bikeState->LowerBodyP.x = T[0];
  m_bikeState->LowerBodyP.y = T[1];

  dJointGetHingeAnchor(m_KneeHingeID, T);
  m_bikeState->KneeP.x = T[0];
  m_bikeState->KneeP.y = T[1];

  dJointGetHingeAnchor(m_FootHingeID, T);
  m_bikeState->FootP.x = T[0];
  m_bikeState->FootP.y = T[1];

  dJointGetHingeAnchor(m_HandHingeID2, T);
  m_bikeState->Hand2P.x = T[0];
  m_bikeState->Hand2P.y = T[1];

  dJointGetHingeAnchor(m_ElbowHingeID2, T);
  m_bikeState->Elbow2P.x = T[0];
  m_bikeState->Elbow2P.y = T[1];

  dJointGetHingeAnchor(m_ShoulderHingeID2, T);
  m_bikeState->Shoulder2P.x = T[0];
  m_bikeState->Shoulder2P.y = T[1];

  dJointGetHingeAnchor(m_LowerBodyHingeID2, T);
  m_bikeState->LowerBody2P.x = T[0];
  m_bikeState->LowerBody2P.y = T[1];

  dJointGetHingeAnchor(m_KneeHingeID2, T);
  m_bikeState->Knee2P.x = T[0];
  m_bikeState->Knee2P.y = T[1];

  dJointGetHingeAnchor(m_FootHingeID2, T);
  m_bikeState->Foot2P.x = T[0];
  m_bikeState->Foot2P.y = T[1];

  Vector2f V;
  /* Calculate head position */
  V = (m_bikeState->ShoulderP - m_bikeState->LowerBodyP);
  V.normalize();
  m_bikeState->HeadP =
    m_bikeState->ShoulderP + V * m_bikeState->Parameters()->fNeckLength;

  /* Calculate head position (Alt.) */
  V = (m_bikeState->Shoulder2P - m_bikeState->LowerBody2P);
  V.normalize();
  m_bikeState->Head2P =
    m_bikeState->Shoulder2P + V * m_bikeState->Parameters()->fNeckLength;

  /* Internally we'd like to know the abs. relaxed position of the wheels */
  m_bikeState->RFrontWheelP.x =
    m_bikeState->Anchors()->Fp.x * m_bikeState->fFrameRot[0] +
    m_bikeState->Anchors()->Fp.y * m_bikeState->fFrameRot[1] +
    m_bikeState->CenterP.x;
  m_bikeState->RFrontWheelP.y =
    m_bikeState->Anchors()->Fp.x * m_bikeState->fFrameRot[2] +
    m_bikeState->Anchors()->Fp.y * m_bikeState->fFrameRot[3] +
    m_bikeState->CenterP.y;
  m_bikeState->RRearWheelP.x =
    m_bikeState->Anchors()->Rp.x * m_bikeState->fFrameRot[0] +
    m_bikeState->Anchors()->Rp.y * m_bikeState->fFrameRot[1] +
    m_bikeState->CenterP.x;
  m_bikeState->RRearWheelP.y =
    m_bikeState->Anchors()->Rp.x * m_bikeState->fFrameRot[2] +
    m_bikeState->Anchors()->Rp.y * m_bikeState->fFrameRot[3] +
    m_bikeState->CenterP.y;
}

/*===========================================================================
Prepare rider
===========================================================================*/
void PlayerLocalBiker::prepareRider(Vector2f StartPos) {
  /* Allocate bodies */
  m_PlayerTorsoBodyID = dBodyCreate(m_WorldID);
  m_PlayerLLegBodyID = dBodyCreate(m_WorldID);
  m_PlayerULegBodyID = dBodyCreate(m_WorldID);
  m_PlayerLArmBodyID = dBodyCreate(m_WorldID);
  m_PlayerUArmBodyID = dBodyCreate(m_WorldID);
  m_PlayerFootAnchorBodyID = dBodyCreate(m_WorldID);
  m_PlayerHandAnchorBodyID = dBodyCreate(m_WorldID);

  m_PlayerTorsoBodyID2 = dBodyCreate(m_WorldID);
  m_PlayerLLegBodyID2 = dBodyCreate(m_WorldID);
  m_PlayerULegBodyID2 = dBodyCreate(m_WorldID);
  m_PlayerLArmBodyID2 = dBodyCreate(m_WorldID);
  m_PlayerUArmBodyID2 = dBodyCreate(m_WorldID);
  m_PlayerFootAnchorBodyID2 = dBodyCreate(m_WorldID);
  m_PlayerHandAnchorBodyID2 = dBodyCreate(m_WorldID);

  /* Place and define the player bodies */
  dBodySetPosition(m_PlayerTorsoBodyID,
                   StartPos.x + m_bikeState->Anchors()->PTp.x,
                   StartPos.y + m_bikeState->Anchors()->PTp.y,
                   0.0f);
  dMassSetSphereTotal(
    &m_PlayerTorsoMass, m_bikeState->Parameters()->BPm_torso, 0.4);
  dBodySetMass(m_PlayerTorsoBodyID, &m_PlayerTorsoMass);
  dBodySetFiniteRotationMode(m_PlayerTorsoBodyID, 1);
  dBodySetFiniteRotationAxis(m_PlayerTorsoBodyID, 0, 0, 1);

  dBodySetPosition(m_PlayerLLegBodyID,
                   StartPos.x + m_bikeState->Anchors()->PLLp.x,
                   StartPos.y + m_bikeState->Anchors()->PLLp.y,
                   0.0f);
  dMassSetSphereTotal(
    &m_PlayerLLegMass, m_bikeState->Parameters()->BPm_lleg, 0.4);
  dBodySetMass(m_PlayerLLegBodyID, &m_PlayerLLegMass);
  dBodySetFiniteRotationMode(m_PlayerLLegBodyID, 1);
  dBodySetFiniteRotationAxis(m_PlayerLLegBodyID, 0, 0, 1);

  dBodySetPosition(m_PlayerULegBodyID,
                   StartPos.x + m_bikeState->Anchors()->PULp.x,
                   StartPos.y + m_bikeState->Anchors()->PULp.y,
                   0.0f);
  dMassSetSphereTotal(
    &m_PlayerULegMass, m_bikeState->Parameters()->BPm_uleg, 0.4);
  dBodySetMass(m_PlayerULegBodyID, &m_PlayerULegMass);
  dBodySetFiniteRotationMode(m_PlayerULegBodyID, 1);
  dBodySetFiniteRotationAxis(m_PlayerULegBodyID, 0, 0, 1);

  dBodySetPosition(m_PlayerLArmBodyID,
                   StartPos.x + m_bikeState->Anchors()->PLAp.x,
                   StartPos.y + m_bikeState->Anchors()->PLAp.y,
                   0.0f);
  dMassSetSphereTotal(
    &m_PlayerLArmMass, m_bikeState->Parameters()->BPm_larm, 0.4);
  dBodySetMass(m_PlayerLArmBodyID, &m_PlayerLArmMass);
  dBodySetFiniteRotationMode(m_PlayerLArmBodyID, 1);
  dBodySetFiniteRotationAxis(m_PlayerLArmBodyID, 0, 0, 1);

  dBodySetPosition(m_PlayerUArmBodyID,
                   StartPos.x + m_bikeState->Anchors()->PUAp.x,
                   StartPos.y + m_bikeState->Anchors()->PUAp.y,
                   0.0f);
  dMassSetSphereTotal(
    &m_PlayerUArmMass, m_bikeState->Parameters()->BPm_uarm, 0.4);
  dBodySetMass(m_PlayerUArmBodyID, &m_PlayerUArmMass);
  dBodySetFiniteRotationMode(m_PlayerUArmBodyID, 1);
  dBodySetFiniteRotationAxis(m_PlayerUArmBodyID, 0, 0, 1);

  dBodySetPosition(m_PlayerFootAnchorBodyID,
                   StartPos.x + m_bikeState->Anchors()->PFp.x,
                   StartPos.y + m_bikeState->Anchors()->PFp.y,
                   0.0f);
  dMassSetSphereTotal(
    &m_PlayerFootAnchorMass, m_bikeState->Parameters()->BPm_foot, 0.4);
  dBodySetMass(m_PlayerFootAnchorBodyID, &m_PlayerFootAnchorMass);
  dBodySetFiniteRotationMode(m_PlayerFootAnchorBodyID, 1);
  dBodySetFiniteRotationAxis(m_PlayerFootAnchorBodyID, 0, 0, 1);

  dBodySetPosition(m_PlayerHandAnchorBodyID,
                   StartPos.x + m_bikeState->Anchors()->PHp.x,
                   StartPos.y + m_bikeState->Anchors()->PHp.y,
                   0.0f);
  dMassSetSphereTotal(
    &m_PlayerHandAnchorMass, m_bikeState->Parameters()->BPm_hand, 0.4);
  dBodySetMass(m_PlayerHandAnchorBodyID, &m_PlayerHandAnchorMass);
  dBodySetFiniteRotationMode(m_PlayerHandAnchorBodyID, 1);
  dBodySetFiniteRotationAxis(m_PlayerHandAnchorBodyID, 0, 0, 1);

  /* Place and define the player bodies (Alt.) */
  dBodySetPosition(m_PlayerTorsoBodyID2,
                   StartPos.x + m_bikeState->Anchors()->PTp2.x,
                   StartPos.y + m_bikeState->Anchors()->PTp2.y,
                   0.0f);
  dBodySetMass(m_PlayerTorsoBodyID2, &m_PlayerTorsoMass);
  dBodySetFiniteRotationMode(m_PlayerTorsoBodyID2, 1);
  dBodySetFiniteRotationAxis(m_PlayerTorsoBodyID2, 0, 0, 1);

  dBodySetPosition(m_PlayerLLegBodyID2,
                   StartPos.x + m_bikeState->Anchors()->PLLp2.x,
                   StartPos.y + m_bikeState->Anchors()->PLLp2.y,
                   0.0f);
  dBodySetMass(m_PlayerLLegBodyID2, &m_PlayerLLegMass);
  dBodySetFiniteRotationMode(m_PlayerLLegBodyID2, 1);
  dBodySetFiniteRotationAxis(m_PlayerLLegBodyID2, 0, 0, 1);

  dBodySetPosition(m_PlayerULegBodyID2,
                   StartPos.x + m_bikeState->Anchors()->PULp2.x,
                   StartPos.y + m_bikeState->Anchors()->PULp2.y,
                   0.0f);
  dBodySetMass(m_PlayerULegBodyID2, &m_PlayerULegMass);
  dBodySetFiniteRotationMode(m_PlayerULegBodyID2, 1);
  dBodySetFiniteRotationAxis(m_PlayerULegBodyID2, 0, 0, 1);

  dBodySetPosition(m_PlayerLArmBodyID2,
                   StartPos.x + m_bikeState->Anchors()->PLAp2.x,
                   StartPos.y + m_bikeState->Anchors()->PLAp2.y,
                   0.0f);
  dBodySetMass(m_PlayerLArmBodyID2, &m_PlayerLArmMass);
  dBodySetFiniteRotationMode(m_PlayerLArmBodyID2, 1);
  dBodySetFiniteRotationAxis(m_PlayerLArmBodyID2, 0, 0, 1);

  dBodySetPosition(m_PlayerUArmBodyID2,
                   StartPos.x + m_bikeState->Anchors()->PUAp2.x,
                   StartPos.y + m_bikeState->Anchors()->PUAp2.y,
                   0.0f);
  dBodySetMass(m_PlayerUArmBodyID2, &m_PlayerUArmMass);
  dBodySetFiniteRotationMode(m_PlayerUArmBodyID2, 1);
  dBodySetFiniteRotationAxis(m_PlayerUArmBodyID2, 0, 0, 1);

  dBodySetPosition(m_PlayerFootAnchorBodyID2,
                   StartPos.x + m_bikeState->Anchors()->PFp2.x,
                   StartPos.y + m_bikeState->Anchors()->PFp2.y,
                   0.0f);
  dBodySetMass(m_PlayerFootAnchorBodyID2, &m_PlayerFootAnchorMass);
  dBodySetFiniteRotationMode(m_PlayerFootAnchorBodyID2, 1);
  dBodySetFiniteRotationAxis(m_PlayerFootAnchorBodyID2, 0, 0, 1);

  dBodySetPosition(m_PlayerHandAnchorBodyID2,
                   StartPos.x + m_bikeState->Anchors()->PHp2.x,
                   StartPos.y + m_bikeState->Anchors()->PHp2.y,
                   0.0f);
  dBodySetMass(m_PlayerHandAnchorBodyID2, &m_PlayerHandAnchorMass);
  dBodySetFiniteRotationMode(m_PlayerHandAnchorBodyID2, 1);
  dBodySetFiniteRotationAxis(m_PlayerHandAnchorBodyID2, 0, 0, 1);

  float fERP = m_bikeState->Parameters()->RErp;
  float fCFM = m_bikeState->Parameters()->RCfm;

  /* Connect em */
  m_KneeHingeID = dJointCreateHinge(m_WorldID, 0);
  dJointAttach(m_KneeHingeID, m_PlayerLLegBodyID, m_PlayerULegBodyID);
  dJointSetHingeAnchor(m_KneeHingeID,
                       StartPos.x + m_bikeState->Parameters()->PKVx,
                       StartPos.y + m_bikeState->Parameters()->PKVy,
                       0.0f);
  dJointSetHingeAxis(m_KneeHingeID, 0, 0, 1);
  dJointSetHingeParam(m_KneeHingeID, dParamLoStop, 0);
  dJointSetHingeParam(m_KneeHingeID, dParamHiStop, 0);
  dJointSetHingeParam(m_KneeHingeID, dParamStopERP, fERP);
  dJointSetHingeParam(m_KneeHingeID, dParamStopCFM, fCFM);

  m_LowerBodyHingeID = dJointCreateHinge(m_WorldID, 0);
  dJointAttach(m_LowerBodyHingeID, m_PlayerULegBodyID, m_PlayerTorsoBodyID);
  dJointSetHingeAnchor(m_LowerBodyHingeID,
                       StartPos.x + m_bikeState->Parameters()->PLVx,
                       StartPos.y + m_bikeState->Parameters()->PLVy,
                       0.0f);
  dJointSetHingeAxis(m_LowerBodyHingeID, 0, 0, 1);
  dJointSetHingeParam(m_LowerBodyHingeID, dParamLoStop, 0);
  dJointSetHingeParam(m_LowerBodyHingeID, dParamHiStop, 0);
  dJointSetHingeParam(m_LowerBodyHingeID, dParamStopERP, fERP);
  dJointSetHingeParam(m_LowerBodyHingeID, dParamStopCFM, fCFM);

  m_ShoulderHingeID = dJointCreateHinge(m_WorldID, 0);
  dJointAttach(m_ShoulderHingeID, m_PlayerTorsoBodyID, m_PlayerUArmBodyID);
  dJointSetHingeAnchor(m_ShoulderHingeID,
                       StartPos.x + m_bikeState->Parameters()->PSVx,
                       StartPos.y + m_bikeState->Parameters()->PSVy,
                       0.0f);
  dJointSetHingeAxis(m_ShoulderHingeID, 0, 0, 1);
  dJointSetHingeParam(m_ShoulderHingeID, dParamLoStop, 0);
  dJointSetHingeParam(m_ShoulderHingeID, dParamHiStop, 0);
  dJointSetHingeParam(m_ShoulderHingeID, dParamStopERP, fERP);
  dJointSetHingeParam(m_ShoulderHingeID, dParamStopCFM, fCFM);

  m_ElbowHingeID = dJointCreateHinge(m_WorldID, 0);
  dJointAttach(m_ElbowHingeID, m_PlayerUArmBodyID, m_PlayerLArmBodyID);
  dJointSetHingeAnchor(m_ElbowHingeID,
                       StartPos.x + m_bikeState->Parameters()->PEVx,
                       StartPos.y + m_bikeState->Parameters()->PEVy,
                       0.0f);
  dJointSetHingeAxis(m_ElbowHingeID, 0, 0, 1);
  dJointSetHingeParam(m_ElbowHingeID, dParamLoStop, 0);
  dJointSetHingeParam(m_ElbowHingeID, dParamHiStop, 0);
  dJointSetHingeParam(m_ElbowHingeID, dParamStopERP, fERP);
  dJointSetHingeParam(m_ElbowHingeID, dParamStopCFM, fCFM);

  m_FootHingeID = dJointCreateHinge(m_WorldID, 0);
  dJointAttach(m_FootHingeID, m_PlayerFootAnchorBodyID, m_PlayerLLegBodyID);
  dJointSetHingeAnchor(m_FootHingeID,
                       StartPos.x + m_bikeState->Parameters()->PFVx,
                       StartPos.y + m_bikeState->Parameters()->PFVy,
                       0.0f);
  dJointSetHingeAxis(m_FootHingeID, 0, 0, 1);
  dJointSetHingeParam(m_FootHingeID, dParamLoStop, 0);
  dJointSetHingeParam(m_FootHingeID, dParamHiStop, 0);
  dJointSetHingeParam(m_FootHingeID, dParamStopERP, fERP);
  dJointSetHingeParam(m_FootHingeID, dParamStopCFM, fCFM);

  m_HandHingeID = dJointCreateHinge(m_WorldID, 0);
  dJointAttach(m_HandHingeID, m_PlayerLArmBodyID, m_PlayerHandAnchorBodyID);
  dJointSetHingeAnchor(m_HandHingeID,
                       StartPos.x + m_bikeState->Parameters()->PHVx,
                       StartPos.y + m_bikeState->Parameters()->PHVy,
                       0.0f);
  dJointSetHingeAxis(m_HandHingeID, 0, 0, 1);
  dJointSetHingeParam(m_HandHingeID, dParamLoStop, 0);
  dJointSetHingeParam(m_HandHingeID, dParamHiStop, 0);
  dJointSetHingeParam(m_HandHingeID, dParamStopERP, fERP);
  dJointSetHingeParam(m_HandHingeID, dParamStopCFM, fCFM);

  /* Connect em (Alt.) */
  m_KneeHingeID2 = dJointCreateHinge(m_WorldID, 0);
  dJointAttach(m_KneeHingeID2, m_PlayerLLegBodyID2, m_PlayerULegBodyID2);
  dJointSetHingeAnchor(m_KneeHingeID2,
                       StartPos.x - m_bikeState->Parameters()->PKVx,
                       StartPos.y + m_bikeState->Parameters()->PKVy,
                       0.0f);
  dJointSetHingeAxis(m_KneeHingeID2, 0, 0, 1);
  dJointSetHingeParam(m_KneeHingeID2, dParamLoStop, 0);
  dJointSetHingeParam(m_KneeHingeID2, dParamHiStop, 0);
  dJointSetHingeParam(m_KneeHingeID2, dParamStopERP, fERP);
  dJointSetHingeParam(m_KneeHingeID2, dParamStopCFM, fCFM);

  m_LowerBodyHingeID2 = dJointCreateHinge(m_WorldID, 0);
  dJointAttach(m_LowerBodyHingeID2, m_PlayerULegBodyID2, m_PlayerTorsoBodyID2);
  dJointSetHingeAnchor(m_LowerBodyHingeID2,
                       StartPos.x - m_bikeState->Parameters()->PLVx,
                       StartPos.y + m_bikeState->Parameters()->PLVy,
                       0.0f);
  dJointSetHingeAxis(m_LowerBodyHingeID2, 0, 0, 1);
  dJointSetHingeParam(m_LowerBodyHingeID2, dParamLoStop, 0);
  dJointSetHingeParam(m_LowerBodyHingeID2, dParamHiStop, 0);
  dJointSetHingeParam(m_LowerBodyHingeID2, dParamStopERP, fERP);
  dJointSetHingeParam(m_LowerBodyHingeID2, dParamStopCFM, fCFM);

  m_ShoulderHingeID2 = dJointCreateHinge(m_WorldID, 0);
  dJointAttach(m_ShoulderHingeID2, m_PlayerTorsoBodyID2, m_PlayerUArmBodyID2);
  dJointSetHingeAnchor(m_ShoulderHingeID2,
                       StartPos.x - m_bikeState->Parameters()->PSVx,
                       StartPos.y + m_bikeState->Parameters()->PSVy,
                       0.0f);
  dJointSetHingeAxis(m_ShoulderHingeID2, 0, 0, 1);
  dJointSetHingeParam(m_ShoulderHingeID2, dParamLoStop, 0);
  dJointSetHingeParam(m_ShoulderHingeID2, dParamHiStop, 0);
  dJointSetHingeParam(m_ShoulderHingeID2, dParamStopERP, fERP);
  dJointSetHingeParam(m_ShoulderHingeID2, dParamStopCFM, fCFM);

  m_ElbowHingeID2 = dJointCreateHinge(m_WorldID, 0);
  dJointAttach(m_ElbowHingeID2, m_PlayerUArmBodyID2, m_PlayerLArmBodyID2);
  dJointSetHingeAnchor(m_ElbowHingeID2,
                       StartPos.x - m_bikeState->Parameters()->PEVx,
                       StartPos.y + m_bikeState->Parameters()->PEVy,
                       0.0f);
  dJointSetHingeAxis(m_ElbowHingeID2, 0, 0, 1);
  dJointSetHingeParam(m_ElbowHingeID2, dParamLoStop, 0);
  dJointSetHingeParam(m_ElbowHingeID2, dParamHiStop, 0);
  dJointSetHingeParam(m_ElbowHingeID2, dParamStopERP, fERP);
  dJointSetHingeParam(m_ElbowHingeID2, dParamStopCFM, fCFM);

  m_FootHingeID2 = dJointCreateHinge(m_WorldID, 0);
  dJointAttach(m_FootHingeID2, m_PlayerFootAnchorBodyID2, m_PlayerLLegBodyID2);
  dJointSetHingeAnchor(m_FootHingeID2,
                       StartPos.x - m_bikeState->Parameters()->PFVx,
                       StartPos.y + m_bikeState->Parameters()->PFVy,
                       0.0f);
  dJointSetHingeAxis(m_FootHingeID2, 0, 0, 1);
  dJointSetHingeParam(m_FootHingeID2, dParamLoStop, 0);
  dJointSetHingeParam(m_FootHingeID2, dParamHiStop, 0);
  dJointSetHingeParam(m_FootHingeID2, dParamStopERP, fERP);
  dJointSetHingeParam(m_FootHingeID2, dParamStopCFM, fCFM);

  m_HandHingeID2 = dJointCreateHinge(m_WorldID, 0);
  dJointAttach(m_HandHingeID2, m_PlayerLArmBodyID2, m_PlayerHandAnchorBodyID2);
  dJointSetHingeAnchor(m_HandHingeID2,
                       StartPos.x - m_bikeState->Parameters()->PHVx,
                       StartPos.y + m_bikeState->Parameters()->PHVy,
                       0.0f);
  dJointSetHingeAxis(m_HandHingeID2, 0, 0, 1);
  dJointSetHingeParam(m_HandHingeID2, dParamLoStop, 0);
  dJointSetHingeParam(m_HandHingeID2, dParamHiStop, 0);
  dJointSetHingeParam(m_HandHingeID2, dParamStopERP, fERP);
  dJointSetHingeParam(m_HandHingeID2, dParamStopCFM, fCFM);
}

/*===========================================================================
Set up bike physics
===========================================================================*/
void PlayerLocalBiker::prepareBikePhysics(Vector2f StartPos) {
  /* Create bodies */
  m_FrontWheelBodyID = dBodyCreate(m_WorldID);
  m_RearWheelBodyID = dBodyCreate(m_WorldID);
  m_FrameBodyID = dBodyCreate(m_WorldID);

  /* Place and define the rear wheel */
  dBodySetPosition(m_RearWheelBodyID,
                   StartPos.x + m_bikeState->Anchors()->Rp.x,
                   StartPos.y + m_bikeState->Anchors()->Rp.y,
                   0.0f);
  /*    const dReal *pf;
      pf = dBodyGetAngularVel(m_RearWheelBodyID);
      printf("[%f %f %f]\n",pf[0],pf[1],pf[2]);*/
  dMassSetSphereTotal(&m_RearWheelMass,
                      m_bikeState->Parameters()->Wm,
                      m_bikeState->Parameters()->WheelRadius());
  dBodySetMass(m_RearWheelBodyID, &m_RearWheelMass);
  dBodySetFiniteRotationMode(m_RearWheelBodyID, 1);
  dBodySetFiniteRotationAxis(m_RearWheelBodyID, 0, 0, 1);

  /* Place and define the front wheel */
  dBodySetPosition(m_FrontWheelBodyID,
                   StartPos.x + m_bikeState->Anchors()->Fp.x,
                   StartPos.y + m_bikeState->Anchors()->Fp.y,
                   0.0f);
  dMassSetSphereTotal(&m_FrontWheelMass,
                      m_bikeState->Parameters()->Wm,
                      m_bikeState->Parameters()->WheelRadius());
  dBodySetMass(m_FrontWheelBodyID, &m_FrontWheelMass);
  dBodySetFiniteRotationMode(m_FrontWheelBodyID, 1);
  dBodySetFiniteRotationAxis(m_FrontWheelBodyID, 0, 0, 1);

  /* Place and define the frame */
  dBodySetPosition(m_FrameBodyID, StartPos.x, StartPos.y, 0.0f);
  dMassSetBoxTotal(&m_FrameMass,
                   m_bikeState->Parameters()->Fm,
                   m_bikeState->Parameters()->IL,
                   m_bikeState->Parameters()->IH,
                   m_physicsSettings->DepthFactor());
  dBodySetMass(m_FrameBodyID, &m_FrameMass);
  dBodySetFiniteRotationMode(m_FrameBodyID, 1);
  dBodySetFiniteRotationAxis(m_FrameBodyID, 0, 0, 1);

  /* Prepare rider */
  prepareRider(StartPos);
}

void PlayerLocalBiker::setBodyDetach(bool state) {
  Biker::setBodyDetach(state);

  if (m_bodyDetach) {
    // angle
    dJointSetHingeParam(m_KneeHingeID, dParamLoStop, 0.0);
    dJointSetHingeParam(m_KneeHingeID, dParamHiStop, 3.14159 / 2.0);
    dJointSetHingeParam(m_KneeHingeID2, dParamLoStop, 3.14159 / 2.0 * -1.0);
    dJointSetHingeParam(m_KneeHingeID2, dParamHiStop, 0.0 * -1.0);
    dJointSetHingeParam(m_KneeHingeID, dParamStopCFM, 0.2);
    dJointSetHingeParam(m_KneeHingeID2, dParamStopCFM, 0.2);

    dJointSetHingeParam(m_LowerBodyHingeID, dParamLoStop, -1.4);
    dJointSetHingeParam(m_LowerBodyHingeID, dParamHiStop, -1.2);
    dJointSetHingeParam(m_LowerBodyHingeID2, dParamLoStop, -1.2 * -1.0);
    dJointSetHingeParam(m_LowerBodyHingeID2, dParamHiStop, -1.4 * -1.0);
    dJointSetHingeParam(m_LowerBodyHingeID, dParamStopCFM, 0.2);
    dJointSetHingeParam(m_LowerBodyHingeID2, dParamStopCFM, 0.2);

    dJointSetHingeParam(m_ShoulderHingeID, dParamLoStop, -1.9);
    dJointSetHingeParam(m_ShoulderHingeID, dParamHiStop, 0.8);
    dJointSetHingeParam(m_ShoulderHingeID2, dParamLoStop, 0.8 * -1.0);
    dJointSetHingeParam(m_ShoulderHingeID2, dParamHiStop, -1.9 * -1.0);
    dJointSetHingeParam(m_ShoulderHingeID, dParamStopCFM, 0.2);
    dJointSetHingeParam(m_ShoulderHingeID2, dParamStopCFM, 0.2);

    dJointSetHingeParam(m_ElbowHingeID, dParamLoStop, -1.5);
    dJointSetHingeParam(m_ElbowHingeID, dParamHiStop, 1.0);
    dJointSetHingeParam(m_ElbowHingeID2, dParamLoStop, 1.0 * -1.0);
    dJointSetHingeParam(m_ElbowHingeID2, dParamHiStop, -1.5 * -1.0);
    dJointSetHingeParam(m_ElbowHingeID, dParamStopCFM, 0.2);
    dJointSetHingeParam(m_ElbowHingeID2, dParamStopCFM, 0.2);
  }
}

void PlayerLocalBiker::addBodyForce(int i_time,
                                    const Vector2f &i_force,
                                    int i_startTime,
                                    int i_endTime) {
  // endtime is 0 => infinite, else, the time is relativ to i_time
  m_externalForces.push_back(new ExternalForce(
    i_time + i_startTime, i_endTime == 0 ? 0 : i_time + i_endTime, i_force));
}

void PlayerLocalBiker::resetAutoDisabler() {
  m_nStillFrames = 0;
}

bool PlayerLocalBiker::isSqueeking() {
  return m_bSqueeking;
}

float PlayerLocalBiker::howMuchSqueek() {
  return m_fHowMuchSqueek;
}

bool PlayerLocalBiker::getRenderBikeFront() {
  return m_wheelDetach == false;
}

float PlayerLocalBiker::getRearWheelVelocity() {
  return dBodyGetAngularVel(m_RearWheelBodyID)[2];
}

float PlayerLocalBiker::getFrontWheelVelocity() {
  return dBodyGetAngularVel(m_FrontWheelBodyID)[2];
}

Vector2f PlayerLocalBiker::determineForceToAdd(int i_time) {
  unsigned int i;
  Vector2f v_res = Vector2f(0.0, 0.0);

  // compute force
  for (unsigned int i = 0; i < m_externalForces.size(); i++) {
    if (m_externalForces[i]->startTime() <= i_time &&
        (m_externalForces[i]->endTime() >= i_time ||
         m_externalForces[i]->endTime() == 0)) {
      v_res += m_externalForces[i]->force();
    }
  }

  // clean forces
  i = 0;
  while (i < m_externalForces.size()) {
    // if duration if <= 0, keep for ever
    if (m_externalForces[i]->endTime() < i_time &&
        m_externalForces[i]->endTime() != 0) {
      delete m_externalForces[i];
      m_externalForces.erase(m_externalForces.begin() + i);
    }
    i++;
  }

  return v_res;
}

BikeController *PlayerLocalBiker::getControler() {
  return m_BikeC;
}

ExternalForce::ExternalForce(int i_startTime, int i_endTime, Vector2f i_force) {
  m_startTime = i_startTime;
  m_endTime = i_endTime;
  m_force = i_force;
}

int ExternalForce::startTime() {
  return m_startTime;
}

int ExternalForce::endTime() {
  return m_endTime;
}

Vector2f ExternalForce::force() {
  return m_force;
}

PlayerNetClient::PlayerNetClient(PhysicsSettings *i_physicsSettings,
                                 Vector2f i_position,
                                 DriveDir i_direction,
                                 Vector2f i_gravity,
                                 bool i_engineSound,
                                 Theme *i_theme,
                                 BikerTheme *i_bikerTheme,
                                 const TColor &i_colorFilter,
                                 const TColor &i_uglyColorFilter)
  : Biker(i_physicsSettings,
          i_engineSound,
          i_theme,
          i_bikerTheme,
          i_colorFilter,
          i_uglyColorFilter) {
  m_BikeC = new BikeControllerNet(0);
  initToPosition(i_position, i_direction, i_gravity);

  m_previousBikeStatesInitialized = false;
  m_previousBikeStates.push_back(new BikeState(i_physicsSettings));
  m_previousBikeStates.push_back(new BikeState(i_physicsSettings));

  m_bikeStateForUpdate = new BikeState(m_physicsSettings);
  m_stateExternallyUpdated = false;
  m_isStateInitialized = false;
  m_lastExtrapolateBikeState = new BikeState(i_physicsSettings);
  m_lastFrameTimeUpdate = 0;
}

PlayerNetClient::~PlayerNetClient() {
  delete m_BikeC;

  for (unsigned int i = 0; i < m_previousBikeStates.size(); i++) {
    delete m_previousBikeStates[i];
  }

  delete m_bikeStateForUpdate;
  delete m_lastExtrapolateBikeState;
}

bool PlayerNetClient::getRenderBikeFront() {
  return true;
}

float PlayerNetClient::getBikeEngineSpeed() {
  return 0.0;
}

float PlayerNetClient::getBikeLinearVel() {
  return 0.0;
}

float PlayerNetClient::getTorsoVelocity() {
  return 0.0;
}

double PlayerNetClient::getAngle() {
  return 0.0;
}

std::string PlayerNetClient::getVeryQuickDescription() const {
  return "Player Net Client";
}

std::string PlayerNetClient::getQuickDescription() const {
  return "Player Net Client";
}

std::string PlayerNetClient::getDescription() const {
  return "Player Net Client";
}

BikeController *PlayerNetClient::getControler() {
  return m_BikeC;
}

void PlayerNetClient::setLocalNetId(int i_value) {
  Biker::setLocalNetId(i_value);
  m_BikeC->setLocalNetId(localNetId());
}

void PlayerNetClient::initToPosition(Vector2f i_position,
                                     DriveDir i_direction,
                                     Vector2f i_gravity) {
  m_bikeState->Dir = i_direction;
  setBodyDetach(false);
}

void PlayerNetClient::updateToTime(int i_time,
                                   int i_timeStep,
                                   CollisionSystem *i_collisionSystem,
                                   Vector2f i_gravity,
                                   Scene *i_motogame) {
  float v_xpolation_value;
  BikeState *v_tmp;
  Biker::updateToTime(
    i_time, i_timeStep, i_collisionSystem, i_gravity, i_motogame);
  bool v_hasBeenExternallyUpdated = m_stateExternallyUpdated;

  if (m_stateExternallyUpdated) {
    m_stateExternallyUpdated = false;

    // update states
    *m_lastExtrapolateBikeState = *m_bikeState;
    m_lastFrameTimeUpdate = GameApp::getXMTimeInt();

    /* update previous states */
    if (m_previousBikeStatesInitialized == false) {
      *(m_previousBikeStates[0]) = *m_bikeStateForUpdate;
      *(m_previousBikeStates[1]) = *m_bikeStateForUpdate;
      *m_lastExtrapolateBikeState = *m_bikeStateForUpdate;
      m_previousBikeStatesInitialized = true;
    } else {
      v_tmp = m_previousBikeStates[0];
      m_previousBikeStates[0] = m_previousBikeStates[1];
      m_previousBikeStates[1] = v_tmp;
      *(m_previousBikeStates[1]) = *m_bikeStateForUpdate;
    }
  }

  /* extrapolate frames */
  if (m_previousBikeStatesInitialized == false) {
    return;
  }

  /* second version
     objectiv : remove brutal frameX to frameX+1 change (at the end of
  extrapolation in version 1)

               F1 received               F2 received                  F3
  received
                    |                         |                           |
                    AAABBBBBBBBBBBBBBBBBBBBBBBAAABBBBBBBBBBBBBBBBBBBBBBBBAAABBBBBB
                                                 -
                                             first B = F2

  A: desextrapolate from last B extrapolation to Fx
  B: extrapolate

     In case of F3 happend while F2(AAA) :

     F2        F3
      |         |
      AAAAAAAAAAAAAABBBB
                 -> start to desinterpolate for last A to F3
                    -
                  first B = F3
  */

  /* it seems to give good result to desinterpolate 50% of the time of the two
     last frames :
     if fps = 1 => desinterpolate for 0,5 seconds (very bad case)
     if fps = 10 => desinterpolate for 0,05 seconds (ok)
  */

  int v_deextrapolation_time =
    (m_previousBikeStates[1]->GameTime -
     m_previousBikeStates[0]
       ->GameTime) /* / 2 -- without /2 it seems to give better results */;

  if (m_previousBikeStatesInitialized &&
      m_lastFrameTimeUpdate + (v_deextrapolation_time * 10) >
        GameApp::getXMTimeInt()) {
    // A
    if (m_lastExtrapolateBikeState->Dir != m_previousBikeStates[1]->Dir ||
        v_hasBeenExternallyUpdated ||
        true // the inter/extra polation seems not to work correctly -- disabled
    ) {
      *m_bikeState = *(m_previousBikeStates[1]);
    } else {
      // interpolate from m_lastExtrapolateBikeState to m_previousBikeStates[1]
      v_xpolation_value = (GameApp::getXMTimeInt() - m_lastFrameTimeUpdate) /
                          ((float)(v_deextrapolation_time * 10));
      v_tmp = m_previousBikeStates[0];
      m_previousBikeStates[0] = m_lastExtrapolateBikeState;
      BikeState::interpolateGameStateLinear(
        m_previousBikeStates, m_bikeState, v_xpolation_value);
      m_previousBikeStates[0] = v_tmp;
    }
  } else {
    /// B
    if (m_previousBikeStates[0]->Dir != m_previousBikeStates[1]->Dir ||
        v_hasBeenExternallyUpdated ||
        true // the inter/extra polation seems not to work correctly -- disabled
    ) {
      *m_bikeState = *(m_previousBikeStates[1]);
    } else {
      if (m_previousBikeStates[1]->GameTime >
          m_previousBikeStates[0]->GameTime) {
        v_xpolation_value = (((i_time - v_deextrapolation_time) -
                              m_previousBikeStates[1]->GameTime) /
                             ((float)(m_previousBikeStates[1]->GameTime -
                                      m_previousBikeStates[0]->GameTime))) +
                            1.0;

        if (v_xpolation_value < XM_MAX_EXTRAPOLATION_T) {
          BikeState::interpolateGameStateLinear(
            m_previousBikeStates, m_bikeState, v_xpolation_value);
        }
      } else {
        // cannot extrapolate
        *m_bikeState = *(m_previousBikeStates[1]);
      }
    }
  }

  m_isStateInitialized = true;
}

BikeState *PlayerNetClient::getStateForUpdate() {
  m_stateExternallyUpdated = true;
  return m_bikeStateForUpdate;
}

bool PlayerNetClient::isStateInitialized() const {
  return m_isStateInitialized;
}
