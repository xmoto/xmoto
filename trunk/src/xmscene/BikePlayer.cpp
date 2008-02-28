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

#include "GameText.h"
#include "BikePlayer.h"
#include "BikeParameters.h"
#include "BikeAnchors.h"
#include "PhysSettings.h"
#include "Collision.h"
#include "Zone.h"
#include "Game.h"
#include "Replay.h"

/* This is the magic depth factor :)  - tweak to obtain max. stability */
#define DEPTH_FACTOR    2


ReplayBiker::ReplayBiker(std::string i_replayFile, Theme *i_theme, BikerTheme* i_bikerTheme)
:Ghost(i_replayFile, true, i_theme, i_bikerTheme,
       TColor(255, 255, 255, 0),
       TColor(GET_RED(i_bikerTheme->getUglyRiderColor()),
	      GET_GREEN(i_bikerTheme->getUglyRiderColor()),
	      GET_BLUE(i_bikerTheme->getUglyRiderColor()),
	      GET_ALPHA(i_bikerTheme->getUglyRiderColor()))) {
}

std::string ReplayBiker::getQuickDescription() const {
  char c_tmp[1024];
  
  snprintf(c_tmp, 1024,
	   GAMETEXT_REPLAYOF,
	   m_replay->getPlayerName().c_str()); 
  
  return std::string(c_tmp);
}

PlayerBiker::PlayerBiker(Vector2f i_position, DriveDir i_direction, Vector2f i_gravity,
			 Theme *i_theme, BikerTheme* i_bikerTheme,
			 const TColor& i_filterColor,
			 const TColor& i_filterUglyColor)
: Biker(i_theme, i_bikerTheme, i_filterColor, i_filterUglyColor) {
  m_somersaultCounter.init();

  bFrontWheelTouching = false;
  bRearWheelTouching  = false;

  bFrontWheelTouching = false;
  bRearWheelTouching  = false;

  m_bSqueeking=false;
  m_nStillFrames = 0;
  m_clearDynamicTouched = false;
  m_lastSqueekTime = 0;

  m_forceToAdd = Vector2f(0.0, 0.0);

  clearStates();
  initPhysics(i_gravity);
  initToPosition(i_position, i_direction, i_gravity);
  m_bikerHooks = NULL;
}

PlayerBiker::~PlayerBiker() {
  uninitPhysics();
}

std::string PlayerBiker::getDescription() const {
  return "";
}

std::string PlayerBiker::getQuickDescription() const {
  return GAMETEXT_PLAYER;
}

void PlayerBiker::updateToTime(int i_time, int i_timeStep,
			       CollisionSystem *i_collisionSystem, Vector2f i_gravity,
			       MotoGame *i_motogame) {
  Biker::updateToTime(i_time, i_timeStep, i_collisionSystem, i_gravity, i_motogame);

  if(isFinished()) {
    return;
  }
  /* DONT UPDATE BELOW IF PLAYER FINISHED THE LEVEL */

  m_bSqueeking = false; /* no squeeking right now */
  updatePhysics(i_time, i_timeStep, i_collisionSystem, i_gravity);
  updateGameState();

  if(isDead()) {
    return;
  }
  /* DONT UPDATE BELOW IF PLAYER IS DEAD */

  /* Squeeking? */
  if(isSqueeking()) {
    if(i_time - m_lastSqueekTime > 1) {
      /* (this is crappy, not enabled) */
      //Sound::playSampleByName("Sounds/Squeek.ogg",m_MotoGame.howMuchSqueek());
      m_lastSqueekTime = i_time;
    }
  }

  /* controler */
  bool bChangeDir = false;
  if(m_BikeC.ChangeDir()) {
    m_BikeC.setChangeDir(false);
    bChangeDir = true;

    m_bikeState.Dir = m_bikeState.Dir==DD_LEFT?DD_RIGHT:DD_LEFT; /* switch */
  }

  /* somersault */
  double fAngle = acos(m_bikeState.fFrameRot[0]);
  bool bCounterclock;
  if(m_bikeState.fFrameRot[2] < 0.0f) fAngle = 2*3.14159f - fAngle;

  if(m_somersaultCounter.update(fAngle, bCounterclock)) {
    if(m_bikerHooks != NULL) {
      m_bikerHooks->onSomersaultDone(bCounterclock);
    }
  }
}

void PlayerBiker::initPhysics(Vector2f i_gravity) {
  m_bFirstPhysicsUpdate = true;

  /* Setup ODE */
  m_WorldID = dWorldCreate();
  dWorldSetERP(m_WorldID,PHYS_WORLD_ERP);
  dWorldSetCFM(m_WorldID,PHYS_WORLD_CFM);

  dWorldSetGravity(m_WorldID, i_gravity.x, i_gravity.y,0);

  m_ContactGroup = dJointGroupCreate(0);

  dWorldSetQuickStepNumIterations(m_WorldID,PHYS_QSTEP_ITERS);

  /* Set default bike parameters */
  m_bikeState.Parameters()->setDefaults();
}

void PlayerBiker::uninitPhysics(void) {
  dJointGroupDestroy(m_ContactGroup);
  dWorldDestroy(m_WorldID);
  dCloseODE();
}

void PlayerBiker::updatePhysics(int i_time, int i_timeStep, CollisionSystem *v_collisionSystem, Vector2f i_gravity) {
  /* No wheel spin per default */
  m_bWheelSpin = false;

  /* Update gravity vector */
  dWorldSetGravity(m_WorldID, i_gravity.x, i_gravity.y,0);

  /* Should we disable stuff? Ok ODE has an autodisable feature, but i'd rather
     roll my own here. */
  const dReal *pfFront = dBodyGetLinearVel(m_FrontWheelBodyID);
  const dReal *pfRear = dBodyGetLinearVel(m_RearWheelBodyID);
  const dReal *pfFrame = dBodyGetLinearVel(m_FrameBodyID);
#define SLEEP_EPS PHYS_SLEEP_EPS
  if(fabs(pfFront[0]) < SLEEP_EPS && fabs(pfFront[1]) < SLEEP_EPS &&
     fabs(pfRear[0]) < SLEEP_EPS && fabs(pfRear[1]) < SLEEP_EPS &&
     fabs(pfFrame[0]) < SLEEP_EPS && fabs(pfFrame[1]) < SLEEP_EPS) {
    m_nStillFrames++;
  }
  else {
    m_nStillFrames=0;
  }
  bool bSleep = false;

  //printf("{%d}\n",m_Collision.isDynamicTouched());
  //printf("]",m_nStillFrames);
  if(m_nStillFrames > PHYS_SLEEP_FRAMES && !m_clearDynamicTouched) {
    bSleep = true;
    dBodyDisable(m_FrontWheelBodyID);
    dBodyDisable(m_RearWheelBodyID);
    dBodyDisable(m_FrameBodyID);
  }
  else {
    if(!dBodyIsEnabled(m_FrontWheelBodyID)) dBodyEnable(m_FrontWheelBodyID);
    if(!dBodyIsEnabled(m_RearWheelBodyID)) dBodyEnable(m_RearWheelBodyID);
    if(!dBodyIsEnabled(m_FrameBodyID)) dBodyEnable(m_FrameBodyID);
  }

  //printf("%d",bSleep);

  if(!bSleep) {

    /* add external force */
    if(m_forceToAdd != Vector2f(0.0, 0.0)) {
      dBodyEnable(m_FrameBodyID);
      dBodyAddForce(m_FrameBodyID, m_forceToAdd.x, m_forceToAdd.y, 0.0);
      m_forceToAdd = Vector2f(0.0, 0.0);
    }

    /* Update front suspension */
    Vector2f Fq = m_bikeState.RFrontWheelP - m_bikeState.FrontWheelP;
    Vector2f Fqv = Fq - m_bikeState.PrevFq;
    Vector2f FSpring = Fq * PHYS_SUSP_SPRING;
    Vector2f FDamp = Fqv * PHYS_SUSP_DAMP;
    Vector2f FTotal = FSpring + FDamp;
    if(m_bodyDetach == false || m_bikeState.Dir == DD_LEFT) {
      dBodyAddForce(m_FrontWheelBodyID,FTotal.x,FTotal.y,0);
      dBodyAddForceAtPos(m_FrameBodyID,-FTotal.x,-FTotal.y,0,m_bikeState.RFrontWheelP.x,m_bikeState.RFrontWheelP.y,0);
    }
    m_bikeState.PrevFq = Fq;

    /* Update rear suspension */
    Vector2f Rq = m_bikeState.RRearWheelP - m_bikeState.RearWheelP;
    Vector2f Rqv = Rq - m_bikeState.PrevRq;
    Vector2f RSpring = Rq * PHYS_SUSP_SPRING;
    Vector2f RDamp = Rqv * PHYS_SUSP_DAMP;
    Vector2f RTotal = RSpring + RDamp;
    if(m_bodyDetach == false || m_bikeState.Dir == DD_RIGHT) {
      dBodyAddForce(m_RearWheelBodyID,RTotal.x,RTotal.y,0);
      dBodyAddForceAtPos(m_FrameBodyID,-RTotal.x,-RTotal.y,0,m_bikeState.RRearWheelP.x,m_bikeState.RRearWheelP.y,0);
    }
    m_bikeState.PrevRq = Rq;

    /* Have any of the suspensions reached the "squeek-point"? (rate of compression/decompression at
       which they will make squeeky noises) */
    if(Fqv.length() > PHYS_SUSP_SQUEEK_POINT || Rqv.length() > PHYS_SUSP_SQUEEK_POINT) {
      /* Calculate how large a sqeek it should be */
      float fSqueekSize1 = Fqv.length() - PHYS_SUSP_SQUEEK_POINT;
      float fSqueekSize2 = Rqv.length() - PHYS_SUSP_SQUEEK_POINT;
      float fSqueekSize = fSqueekSize1 > fSqueekSize2 ? fSqueekSize1 : fSqueekSize2;
      float fMaxSqueek = 0.11f;
      float fL = fSqueekSize / fMaxSqueek;
      if(fL > 1.0f) fL = 1.0f;
      m_fHowMuchSqueek = fL;
      //printf("%f\n",fL);

      m_bSqueeking = true;
    }
  }

  /* Apply attitude control (SIMPLISTIC!) */
  // when you want to rotate in opposite direction  (m_BikeC.Pull() * m_fLastAttitudeDir < 0) it's true
  //  benetnash: I don't think It will affect highscores in any way
  if(isDead() == false && isFinished() == false) {
    if((m_BikeC.Pull() != 0.0f) && (i_time/100.0 > m_fNextAttitudeCon /*XXX*/ || (m_BikeC.Pull() * m_fLastAttitudeDir < 0) /*XXX*/ )) {
      m_fAttitudeCon = m_BikeC.Pull() * PHYS_RIDER_ATTITUDE_TORQUE;
      m_fLastAttitudeDir = m_fAttitudeCon;
      m_fNextAttitudeCon = (i_time/100.0) + (0.6f * fabsf(m_BikeC.Pull()));
    }
  }

  if(m_fAttitudeCon != 0.0f) {
    m_nStillFrames=0;
    dBodyEnable(m_FrontWheelBodyID);
    dBodyEnable(m_RearWheelBodyID);
    dBodyEnable(m_FrameBodyID);
    dBodyAddTorque(m_FrameBodyID,0,0,m_fAttitudeCon);
    
    //printf("AttitudeCon %f\n",m_fAttitudeCon);
  }
  
  m_fAttitudeCon *= PHYS_ATTITUDE_DEFACTOR;
  if(fabs(m_fAttitudeCon) < 100) { /* make sure we glue to zero */
    m_fAttitudeCon = 0.0f;
  }
  
  float fRearWheelAngVel = dBodyGetAngularVel(m_RearWheelBodyID)[2];
  float fFrontWheelAngVel = dBodyGetAngularVel(m_FrontWheelBodyID)[2];

  /* Misc */
  if(!bSleep) {
    if(fRearWheelAngVel > -PHYS_MAX_ROLL_VELOCITY && fRearWheelAngVel < PHYS_MAX_ROLL_VELOCITY)
      dBodyAddTorque(m_RearWheelBodyID,0,0,-dBodyGetAngularVel(m_RearWheelBodyID)[2]*PHYS_ROLL_RESIST);
    else
      dBodyAddTorque(m_RearWheelBodyID,0,0,-dBodyGetAngularVel(m_RearWheelBodyID)[2]*PHYS_ROLL_RESIST_MAX);

    if(fFrontWheelAngVel > -PHYS_MAX_ROLL_VELOCITY && fFrontWheelAngVel < PHYS_MAX_ROLL_VELOCITY)
      dBodyAddTorque(m_FrontWheelBodyID,0,0,-dBodyGetAngularVel(m_FrontWheelBodyID)[2]*PHYS_ROLL_RESIST);
    else
      dBodyAddTorque(m_FrontWheelBodyID,0,0,-dBodyGetAngularVel(m_FrontWheelBodyID)[2]*PHYS_ROLL_RESIST_MAX);
  }

  m_bikeState.physicalUpdate();

  /* Update RPM */
  if(isDead() == false && isFinished() == false) {
    /* Simply map the wheel ang vel to the RPM (stupid, lame, lots of bad stuff) */
    if(m_bikeState.Dir == DD_RIGHT) {
      float f = -fRearWheelAngVel;
      if(f<0.0f) f=0.0f;
      m_bikeState.fBikeEngineRPM = ENGINE_MIN_RPM + (ENGINE_MAX_RPM - ENGINE_MIN_RPM) * (f / PHYS_MAX_ROLL_VELOCITY) * m_BikeC.Drive();
      if(m_bikeState.fBikeEngineRPM < ENGINE_MIN_RPM) m_bikeState.fBikeEngineRPM = ENGINE_MIN_RPM;
      if(m_bikeState.fBikeEngineRPM > ENGINE_MAX_RPM) m_bikeState.fBikeEngineRPM = ENGINE_MAX_RPM;
    } else if(m_bikeState.Dir == DD_LEFT) {
      float f = fFrontWheelAngVel;
      if(f<0.0f) f=0.0f;
      m_bikeState.fBikeEngineRPM = ENGINE_MIN_RPM + (ENGINE_MAX_RPM - ENGINE_MIN_RPM) * (f / PHYS_MAX_ROLL_VELOCITY) * m_BikeC.Drive();
      if(m_bikeState.fBikeEngineRPM < ENGINE_MIN_RPM) m_bikeState.fBikeEngineRPM = ENGINE_MIN_RPM;
      if(m_bikeState.fBikeEngineRPM > ENGINE_MAX_RPM) m_bikeState.fBikeEngineRPM = ENGINE_MAX_RPM;
    }
  }

  /* Apply motor/brake torques */
  if(isDead() == false && isFinished() == false) {
    if(m_BikeC.Drive() < 0.0f) {
      /* Brake! */
      if(!bSleep) {
	//printf("Brake!\n");

	dBodyAddTorque(m_RearWheelBodyID,0,0,dBodyGetAngularVel(m_RearWheelBodyID)[2]*PHYS_BRAKE_FACTOR*m_BikeC.Drive());
	dBodyAddTorque(m_FrontWheelBodyID,0,0,dBodyGetAngularVel(m_FrontWheelBodyID)[2]*PHYS_BRAKE_FACTOR*m_BikeC.Drive());
      }
    } else {
      /* Throttle? */
      if(m_BikeC.Drive() > 0.0f) {
	if(m_bikeState.Dir == DD_RIGHT) {
	  if(fRearWheelAngVel > -PHYS_MAX_ROLL_VELOCITY) {
	    m_nStillFrames=0;
	    dBodyEnable(m_FrontWheelBodyID);
	    dBodyEnable(m_RearWheelBodyID);
	    dBodyEnable(m_FrameBodyID);
	    
	    dBodyAddTorque(m_RearWheelBodyID,0,0,-m_bikeState.Parameters()->MaxEngine()*PHYS_ENGINE_DAMP*m_BikeC.Drive());
	    
	    //printf("Drive!\n");
	  }
	} else {
	  if(fFrontWheelAngVel < PHYS_MAX_ROLL_VELOCITY) {
	    m_nStillFrames=0;
	    dBodyEnable(m_FrontWheelBodyID);
	    dBodyEnable(m_RearWheelBodyID);
	    dBodyEnable(m_FrameBodyID);
	    dBodyAddTorque(m_FrontWheelBodyID,0,0,m_bikeState.Parameters()->MaxEngine()*PHYS_ENGINE_DAMP*m_BikeC.Drive());
	  }
	}
      }
    }
  }

  /* Perform collision detection between the bike and the level blocks */
  v_collisionSystem->clearDynamicTouched();

  int nNumContacts;
  dContact Contacts[100];

  static int nSlipFrameInterlace =0; /* Okay, lazy approach again. Front wheel can generate particles
					even frames, rear wheel odd frames */
  nSlipFrameInterlace++;

  nNumContacts = intersectWheelLevel( m_bikeState.FrontWheelP,m_bikeState.Parameters()->WheelRadius(),Contacts, v_collisionSystem);
  if(nNumContacts>0) {
    if(bFrontWheelTouching == false) {
      bFrontWheelTouching = true;
      if(m_bikerHooks != NULL) {
	m_bikerHooks->onWheelTouches(1, true);
      }
    }
  } else {
    if(bFrontWheelTouching) {
      bFrontWheelTouching = false;
      if(m_bikerHooks != NULL) {
	m_bikerHooks->onWheelTouches(1, false);
      }
    }
  }
  if(v_collisionSystem->isDynamicTouched()) {
    if(!dBodyIsEnabled(m_FrontWheelBodyID)) dBodyEnable(m_FrontWheelBodyID);
    if(!dBodyIsEnabled(m_RearWheelBodyID)) dBodyEnable(m_RearWheelBodyID);
    if(!dBodyIsEnabled(m_FrameBodyID)) dBodyEnable(m_FrameBodyID);
  }
  if(dBodyIsEnabled(m_FrontWheelBodyID)) {
    Vector2f WSP;
    for(int i=0;i<nNumContacts;i++) {
      dJointAttach(dJointCreateContact(m_WorldID,m_ContactGroup,&Contacts[i]),m_FrontWheelBodyID,0);

      WSP.x = Contacts[i].geom.pos[0];
      WSP.y = Contacts[i].geom.pos[1];
    }

    //if(nNumContacts > 0 && nSlipFrameInterlace&1  && sqrt(pfFrame[0]*pfFrame[0] + pfFrame[1]*pfFrame[1])>1.2f) {
    //  Vector2f WSPvel( (-(m_bikeState.FrontWheelP.y - WSP.y)),
    //                   ((m_bikeState.FrontWheelP.x - WSP.x)) );
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

    //    //Vector2f Zz = (m_WheelSpinDir + Vector2f(m_bikeState.FrontWheelP.x - WSP.x,m_bikeState.FrontWheelP.y - WSP.y)) / 2.0f;
    //    //m_WheelSpinDir = Zz;
    //  }
    //}

    if(m_bikeState.Dir == DD_LEFT) {
      if(isDead() == false && isFinished() == false) {
	if(fabs(fFrontWheelAngVel) > 5 && m_BikeC.Drive()>0.0f && nNumContacts > 0) {
	  m_bWheelSpin = true;
	  m_WheelSpinPoint = WSP;
	  m_WheelSpinDir.x = (((m_bikeState.FrontWheelP.y - WSP.y))*1 + (m_bikeState.FrontWheelP.x - WSP.x)) /2;
	  m_WheelSpinDir.y = ((-(m_bikeState.FrontWheelP.x - WSP.x))*1 + (m_bikeState.FrontWheelP.y - WSP.y)) /2;
	}
      }
    }
  }

  nNumContacts = intersectWheelLevel( m_bikeState.RearWheelP,m_bikeState.Parameters()->WheelRadius(),Contacts, v_collisionSystem);
  if(nNumContacts>0) {
    if(bRearWheelTouching == false) {
      bRearWheelTouching = true;
      if(m_bikerHooks != NULL) {
	m_bikerHooks->onWheelTouches(2, true);
      }
    }
  } else {
    if(bRearWheelTouching) {
      bRearWheelTouching = false;
      if(m_bikerHooks != NULL) {
	m_bikerHooks->onWheelTouches(2, false);
      }
    }
  }

  if(v_collisionSystem->isDynamicTouched()) {
    if(!dBodyIsEnabled(m_FrontWheelBodyID)) dBodyEnable(m_FrontWheelBodyID);
    if(!dBodyIsEnabled(m_RearWheelBodyID)) dBodyEnable(m_RearWheelBodyID);
    if(!dBodyIsEnabled(m_FrameBodyID)) dBodyEnable(m_FrameBodyID);
  }
  if(dBodyIsEnabled(m_RearWheelBodyID)) {
    Vector2f WSP;
    for(int i=0;i<nNumContacts;i++) {
      dJointAttach(dJointCreateContact(m_WorldID,m_ContactGroup,&Contacts[i]),m_RearWheelBodyID,0);
      WSP.x = Contacts[i].geom.pos[0];
      WSP.y = Contacts[i].geom.pos[1];
    }

    /* Calculate wheel linear velocity at slip-point */
    //if(nNumContacts > 0 && !(nSlipFrameInterlace&1) && sqrt(pfFrame[0]*pfFrame[0] + pfFrame[1]*pfFrame[1])>1.2f) {
    //  Vector2f WSPvel( (-(m_bikeState.RearWheelP.y - WSP.y)),
    //                   ((m_bikeState.RearWheelP.x - WSP.x)) );
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
    //    //Vector2f Zz = (m_WheelSpinDir + Vector2f(m_bikeState.RearWheelP.x - WSP.x,m_bikeState.RearWheelP.y - WSP.y)) / 2.0f;
    //    //m_WheelSpinDir = Zz;
    //  }
    //}

    if(m_bikeState.Dir == DD_RIGHT) {
      if(isDead() == false && isFinished() == false) {
	if(fabs(fRearWheelAngVel) > 5 && m_BikeC.Drive()>0 && nNumContacts > 0) {
	  m_bWheelSpin = true;
	  m_WheelSpinPoint = WSP;
	  m_WheelSpinDir.x = ((-(m_bikeState.RearWheelP.y - WSP.y))*1 + (m_bikeState.RearWheelP.x - WSP.x)) /2;
	  m_WheelSpinDir.y = (((m_bikeState.RearWheelP.x - WSP.x))*1 + (m_bikeState.RearWheelP.y - WSP.y)) /2;
	}
      }
    }
  }

  /* body */
  if(m_bodyDetach) {
    /*
    // m_PlayerTorsoBody
    if(m_bikeState.Dir == DD_RIGHT) {
    nNumContacts = m_Collision.collideLine(m_bikeState.ShoulderP.x, m_bikeState.ShoulderP.y,
    m_bikeState.LowerBodyP.x, m_bikeState.LowerBodyP.y,
    Contacts, 100);
    } else {
    nNumContacts = m_Collision.collideLine(m_bikeState.Shoulder2P.x,  m_bikeState.Shoulder2P.y,
    m_bikeState.LowerBody2P.x, m_bikeState.LowerBody2P.y,
    Contacts, 100);
    }
    if(nNumContacts > 0) {
    //usleep(100000);
    }
    for(int i=0;i<nNumContacts;i++) {
    dJointAttach(dJointCreateContact(m_WorldID,
    m_ContactGroup,
    &Contacts[i]),
    m_bikeState.Dir == DD_RIGHT ?
    m_PlayerTorsoBodyID : m_PlayerTorsoBodyID2, 0);
    }
    */
    /*
    // m_PlayerLArmBodyID
    if(m_bikeState.Dir == DD_RIGHT) {
    nNumContacts = m_Collision.collideLine(m_bikeState.ElbowP.x, m_bikeState.ElbowP.y,
    m_bikeState.HandP.x, m_bikeState.HandP.y,
    Contacts, 100);
    } else {
    nNumContacts = m_Collision.collideLine(m_bikeState.Elbow2P.x,  m_bikeState.Elbow2P.y,
    m_bikeState.Hand2P.x, m_bikeState.Hand2P.y,
    Contacts, 100);
    }
    for(int i=0;i<nNumContacts;i++) {
    dJointAttach(dJointCreateContact(m_WorldID,
    m_ContactGroup,
    &Contacts[i]),
    m_bikeState.Dir == DD_RIGHT ?
    m_PlayerLArmBodyID : m_PlayerLArmBodyID2, 0);
    }

    // m_PlayerUArmBodyID
    if(m_bikeState.Dir == DD_RIGHT) {
    nNumContacts = m_Collision.collideLine(m_bikeState.ElbowP.x, m_bikeState.ElbowP.y,
    m_bikeState.ShoulderP.x, m_bikeState.ShoulderP.y,
    Contacts, 100);
    } else {
    nNumContacts = m_Collision.collideLine(m_bikeState.Elbow2P.x,  m_bikeState.Elbow2P.y,
    m_bikeState.Shoulder2P.x, m_bikeState.Shoulder2P.y,
    Contacts, 100);
    }
    for(int i=0;i<nNumContacts;i++) {
    dJointAttach(dJointCreateContact(m_WorldID,
    m_ContactGroup,
    &Contacts[i]),
    m_bikeState.Dir == DD_RIGHT ?
    m_PlayerUArmBodyID : m_PlayerUArmBodyID2, 0);
    }

    // m_PlayerULegBodyID
    if(m_bikeState.Dir == DD_RIGHT) {
    nNumContacts = m_Collision.collideLine(m_bikeState.LowerBodyP.x, m_bikeState.LowerBodyP.y,
    m_bikeState.KneeP.x, m_bikeState.KneeP.y,
    Contacts, 100);
    } else {
    nNumContacts = m_Collision.collideLine(m_bikeState.LowerBody2P.x,  m_bikeState.LowerBody2P.y,
    m_bikeState.Knee2P.x, m_bikeState.Knee2P.y,
    Contacts, 100);
    }
    for(int i=0;i<nNumContacts;i++) {
    dJointAttach(dJointCreateContact(m_WorldID,
    m_ContactGroup,
    &Contacts[i]),
    m_bikeState.Dir == DD_RIGHT ?
    m_PlayerULegBodyID : m_PlayerULegBodyID2, 0);
    }

    // m_PlayerLLegBodyID
    if(m_bikeState.Dir == DD_RIGHT) {
    nNumContacts = m_Collision.collideLine(m_bikeState.KneeP.x, m_bikeState.KneeP.y,
    m_bikeState.FootP.x, m_bikeState.FootP.y,
    Contacts, 100);
    } else {
    nNumContacts = m_Collision.collideLine(m_bikeState.Knee2P.x,  m_bikeState.Knee2P.y,
    m_bikeState.Foot2P.x, m_bikeState.Foot2P.y,
    Contacts, 100);
    }
    for(int i=0;i<nNumContacts;i++) {
    dJointAttach(dJointCreateContact(m_WorldID,
    m_ContactGroup,
    &Contacts[i]),
    m_bikeState.Dir == DD_RIGHT ?
    m_PlayerLLegBodyID : m_PlayerLLegBodyID2, 0);
    }
    */

    /*
    // hand
    if(m_bikeState.Dir == DD_RIGHT) {
    nNumContacts = m_Collision.collideCircle(m_bikeState.HandP.x, m_bikeState.HandP.y, 0.1, Contacts, 100);
    } else {
    nNumContacts = m_Collision.collideCircle(m_bikeState.Hand2P.x, m_bikeState.Hand2P.y, 0.1, Contacts, 100);
    }
    for(int i=0;i<nNumContacts;i++) {
    dJointAttach(dJointCreateContact(m_WorldID,
    m_ContactGroup,
    &Contacts[i]),
    m_bikeState.Dir == DD_RIGHT ?
    m_PlayerHandAnchorBodyID : m_PlayerHandAnchorBodyID2, 0);
    }

    // foot
    if(m_bikeState.Dir == DD_RIGHT) {
    nNumContacts = m_Collision.collideCircle(m_bikeState.FootP.x, m_bikeState.FootP.y, 0.1, Contacts, 100);
    } else {
    nNumContacts = m_Collision.collideCircle(m_bikeState.Foot2P.x, m_bikeState.Foot2P.y, 0.1, Contacts, 100);
    }
    for(int i=0;i<nNumContacts;i++) {
    dJointAttach(dJointCreateContact(m_WorldID,
    m_ContactGroup,
    &Contacts[i]),
    m_bikeState.Dir == DD_RIGHT ?
    m_PlayerFootAnchorBodyID : m_PlayerFootAnchorBodyID2 , 0);
    }
    */

  }

  /* Player head */
  if(m_bikeState.Dir == DD_RIGHT) {
    if(intersectHeadLevel(m_bikeState.HeadP,m_bikeState.Parameters()->HeadSize(),m_PrevActiveHead, v_collisionSystem)) {
      if(m_bikerHooks != NULL) {
	m_bikerHooks->onHeadTouches();
      }
    }

    m_PrevActiveHead = m_bikeState.HeadP;
  }
  else if(m_bikeState.Dir == DD_LEFT) {
    if(intersectHeadLevel(m_bikeState.Head2P,m_bikeState.Parameters()->HeadSize(),m_PrevActiveHead, v_collisionSystem)) {
      if(m_bikerHooks != NULL) {
	m_bikerHooks->onHeadTouches();
      }
    }

    m_PrevActiveHead = m_bikeState.Head2P;
  }

  m_PrevFrontWheelP = m_bikeState.FrontWheelP;
  m_PrevRearWheelP = m_bikeState.RearWheelP;
  m_PrevHeadP = m_bikeState.HeadP;
  m_PrevHead2P = m_bikeState.Head2P;

  /* Move rider along bike -- calculate handlebar and footpeg coords */
  Vector2f FootRP,HandRP;

  FootRP = m_bikeState.WantedFootP;
  HandRP = m_bikeState.WantedHandP;

  Vector2f PFq = FootRP - m_bikeState.FootP;
  Vector2f PFqv = PFq - m_bikeState.PrevPFq;
  Vector2f PFSpring = PFq * PHYS_RIDER_SPRING;
  Vector2f PFDamp = PFqv * PHYS_RIDER_DAMP;
  Vector2f PFTotal = PFSpring + PFDamp;
  if(m_bodyDetach == false) {
    dBodyAddForce(m_PlayerFootAnchorBodyID,PFTotal.x,PFTotal.y,0);
  }

  m_bikeState.PrevPFq = PFq;

  Vector2f PHq = HandRP - m_bikeState.HandP;
  Vector2f PHqv = PHq - m_bikeState.PrevPHq;
  Vector2f PHSpring = PHq * PHYS_RIDER_SPRING;
  Vector2f PHDamp = PHqv * PHYS_RIDER_DAMP;
  Vector2f PHTotal = PHSpring + PHDamp;
  if(m_bodyDetach == false) {
    dBodyAddForce(m_PlayerHandAnchorBodyID,PHTotal.x,PHTotal.y,0);
  }
  m_bikeState.PrevPHq = PHq;

  FootRP = m_bikeState.WantedFoot2P;
  HandRP = m_bikeState.WantedHand2P;

  PFq = FootRP - m_bikeState.Foot2P;
  PFqv = PFq - m_bikeState.PrevPFq2;
  PFSpring = PFq * PHYS_RIDER_SPRING;
  PFDamp = PFqv * PHYS_RIDER_DAMP;
  PFTotal = PFSpring + PFDamp;
  if(m_bodyDetach == false) {
    dBodyAddForce(m_PlayerFootAnchorBodyID2,PFTotal.x,PFTotal.y,0);
  }
  m_bikeState.PrevPFq2 = PFq;

  PHq = HandRP - m_bikeState.Hand2P;
  PHqv = PHq - m_bikeState.PrevPHq2;
  PHSpring = PHq * PHYS_RIDER_SPRING;
  PHDamp = PHqv * PHYS_RIDER_DAMP;
  PHTotal = PHSpring + PHDamp;
  if(m_bodyDetach == false) {
    dBodyAddForce(m_PlayerHandAnchorBodyID2,PHTotal.x,PHTotal.y,0);
  }
  m_bikeState.PrevPHq2 = PHq;

  /* Perform world simulation step */
  dWorldQuickStep(m_WorldID,((float)i_timeStep/100.0)*PHYS_SPEED);
  //dWorldStep(m_WorldID,fTimeStep*PHYS_SPEED);

  /* Empty contact joint group */
  dJointGroupEmpty(m_ContactGroup);

  m_clearDynamicTouched = v_collisionSystem->isDynamicTouched();

  m_bFirstPhysicsUpdate = false;
}

bool PlayerBiker::intersectHeadLevel(Vector2f Cp,float Cr,const Vector2f &LastCp, CollisionSystem *v_collisionSystem) {
  if(v_collisionSystem->checkCircle(Cp.x,Cp.y,Cr)) return true;

  if(!m_bFirstPhysicsUpdate) {
    dContact c[100];
    int nNumContacts = v_collisionSystem->collideLine(LastCp.x,LastCp.y,Cp.x,Cp.y,c,100);
    if(nNumContacts > 0) {
      return true;
    }
  }

  return false;
}

int PlayerBiker::intersectWheelLevel(Vector2f Cp,float Cr,dContact *pContacts, CollisionSystem *v_collisionSystem) {
  int nNumContacts = v_collisionSystem->collideCircle(Cp.x,Cp.y,Cr,pContacts,100);
  if(nNumContacts == 0) {
      /* Nothing... but what if we are moving so fast that the circle has moved
         all the way through some geometry? Check it's path. */
      //nNumContacts = m_Collision.collideLine(
  }
  return nNumContacts;
}

void PlayerBiker::initToPosition(Vector2f i_position, DriveDir i_direction, Vector2f i_gravity) {
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
  m_bikeState.reInitializeAnchors();
  Vector2f C(i_position - m_bikeState.Anchors()->GroundPoint());
  prepareBikePhysics(C);

  setBodyDetach(false);

  m_bikeState.Dir = i_direction;

  m_bikeState.reInitializeSpeed();

  updateGameState();
}

void PlayerBiker::clearStates() {
  /* BIKE_S */
  m_bikeState.CenterP = Vector2f(0,0);
  m_bikeState.Dir = DD_RIGHT;
  m_bikeState.fBikeEngineRPM = 0.0f;
  m_bikeState.Elbow2P = Vector2f(0,0);
  m_bikeState.ElbowP = Vector2f(0,0);
  m_bikeState.reInitializeSpeed();
  m_bikeState.Foot2P = Vector2f(0,0);
  m_bikeState.FootP = Vector2f(0,0);
  m_bikeState.FrontAnchor2P = Vector2f(0,0);
  m_bikeState.FrontAnchorP = Vector2f(0,0);
  m_bikeState.FrontWheelP = Vector2f(0,0);
  m_bikeState.Hand2P = Vector2f(0,0);
  m_bikeState.HandP = Vector2f(0,0);
  m_bikeState.Head2P = Vector2f(0,0);
  m_bikeState.HeadP = Vector2f(0,0);
  m_bikeState.Knee2P = Vector2f(0,0);
  m_bikeState.KneeP = Vector2f(0,0);
  m_bikeState.LowerBody2P = Vector2f(0,0);
  m_bikeState.LowerBodyP = Vector2f(0,0);
  //m_bikeState.pfFramePos = NULL;
  //m_bikeState.pfFrameRot = NULL;
  //m_bikeState.pfFrontWheelPos = NULL;
  //m_bikeState.pfFrontWheelRot = NULL;
  //m_bikeState.pfRearWheelPos = NULL;
  //m_bikeState.pfRearWheelRot = NULL;
  //m_bikeState.pfPlayerLArmPos = NULL;
  //m_bikeState.pfPlayerUArmPos = NULL;
  //m_bikeState.pfPlayerLLegPos = NULL;
  //m_bikeState.pfPlayerULegPos = NULL;
  //m_bikeState.pfPlayerTorsoPos = NULL;
  //m_bikeState.pfPlayerTorsoRot = NULL;
  m_bikeState.PlayerLArmP = Vector2f(0,0);
  m_bikeState.PlayerLLegP = Vector2f(0,0);
  m_bikeState.PlayerTorsoP = Vector2f(0,0);
  m_bikeState.PlayerUArmP = Vector2f(0,0);
  m_bikeState.PlayerULegP = Vector2f(0,0);
  //m_bikeState.pfPlayerLArm2Pos = NULL;
  //m_bikeState.pfPlayerUArm2Pos = NULL;
  //m_bikeState.pfPlayerLLeg2Pos = NULL;
  //m_bikeState.pfPlayerULeg2Pos = NULL;
  //m_bikeState.pfPlayerTorso2Pos = NULL;
  //m_bikeState.pfPlayerTorso2Rot = NULL;
  m_bikeState.PlayerLArm2P = Vector2f(0,0);
  m_bikeState.PlayerLLeg2P = Vector2f(0,0);
  m_bikeState.PlayerTorso2P = Vector2f(0,0);
  m_bikeState.PlayerUArm2P = Vector2f(0,0);
  m_bikeState.PlayerULeg2P = Vector2f(0,0);
  m_bikeState.PrevFq = Vector2f(0,0);
  m_bikeState.PrevRq = Vector2f(0,0);
  m_bikeState.PrevPFq = Vector2f(0,0);
  m_bikeState.PrevPHq = Vector2f(0,0);
  m_bikeState.PrevPFq2 = Vector2f(0,0);
  m_bikeState.PrevPHq2 = Vector2f(0,0);
  m_bikeState.RearWheelP = Vector2f(0,0);
  m_bikeState.RFrontWheelP = Vector2f(0,0);
  m_bikeState.RRearWheelP = Vector2f(0,0);
  m_bikeState.Shoulder2P = Vector2f(0,0);
  m_bikeState.ShoulderP = Vector2f(0,0);
  m_bikeState.SwingAnchor2P = Vector2f(0,0);
  m_bikeState.SwingAnchorP = Vector2f(0,0);

  if(isDead() == false && isFinished() == false) {
    /* BIKE_C */
    memset(&m_BikeC,0,sizeof(m_BikeC));

    m_forceToAdd = Vector2f(0.0, 0.0);
  }
}

BikeController* PlayerBiker::getControler() {
  return &m_BikeC;
}

float PlayerBiker::getBikeEngineSpeed() {
	float fWheelAngVel;
	float speed;

	if(m_bikeState.Dir == DD_RIGHT) {
		fWheelAngVel = dBodyGetAngularVel(m_RearWheelBodyID)[2];
	} else {
		fWheelAngVel = dBodyGetAngularVel(m_FrontWheelBodyID)[2];
	}

	speed = (fWheelAngVel * PHYS_WHEEL_RADIUS * 3.6);
	return speed >= 0.0 ? speed : -speed;
}

float PlayerBiker::getBikeLinearVel() {

  Vector2f curpos = (m_bikeState.RearWheelP + m_bikeState.FrontWheelP) / 2;
  Vector2f lastpos = (m_PrevRearWheelP + m_PrevFrontWheelP) / 2;
  Vector2f delta = curpos - lastpos;
  float speed = 10 * sqrt(delta.x * delta.x + delta.y * delta.y) / PHYS_STEP_SIZE;

  /* protection against invalid values */
  if (speed > 400)
    return 0;

  return speed;  
  
}

  void PlayerBiker::updateGameState() {
    bool bUpdateRider=true,bUpdateAltRider=true;

    /* Replaying? */
      /* Nope... Get current bike state */
      m_bikeState.RearWheelP.x = ((dReal *)dBodyGetPosition( m_RearWheelBodyID ))[0];     /* 4 bytes */
      m_bikeState.RearWheelP.y = ((dReal *)dBodyGetPosition( m_RearWheelBodyID ))[1];     /* 4 bytes */
      m_bikeState.FrontWheelP.x = ((dReal *)dBodyGetPosition( m_FrontWheelBodyID ))[0];   /* 4 bytes */
      m_bikeState.FrontWheelP.y = ((dReal *)dBodyGetPosition( m_FrontWheelBodyID ))[1];   /* 4 bytes */
      m_bikeState.CenterP.x = ((dReal *)dBodyGetPosition( m_FrameBodyID ))[0];            /* 4 bytes */
      m_bikeState.CenterP.y = ((dReal *)dBodyGetPosition( m_FrameBodyID ))[1];            /* 4 bytes */
                                                                                      /* ------- */
                                                                                      /* 24 bytes total */

      m_bikeState.fFrameRot[0] = ((dReal *)dBodyGetRotation( m_FrameBodyID ))[0];
      m_bikeState.fFrameRot[1] = ((dReal *)dBodyGetRotation( m_FrameBodyID ))[1];
      m_bikeState.fFrameRot[2] = ((dReal *)dBodyGetRotation( m_FrameBodyID ))[4];
      m_bikeState.fFrameRot[3] = ((dReal *)dBodyGetRotation( m_FrameBodyID ))[5];           /* 16 bytes */

      m_bikeState.fFrontWheelRot[0] = ((dReal *)dBodyGetRotation( m_FrontWheelBodyID ))[0];
      m_bikeState.fFrontWheelRot[1] = ((dReal *)dBodyGetRotation( m_FrontWheelBodyID ))[1];
      m_bikeState.fFrontWheelRot[2] = ((dReal *)dBodyGetRotation( m_FrontWheelBodyID ))[4];
      m_bikeState.fFrontWheelRot[3] = ((dReal *)dBodyGetRotation( m_FrontWheelBodyID ))[5]; /* 16 bytes */

      m_bikeState.fRearWheelRot[0] = ((dReal *)dBodyGetRotation( m_RearWheelBodyID ))[0];
      m_bikeState.fRearWheelRot[1] = ((dReal *)dBodyGetRotation( m_RearWheelBodyID ))[1];
      m_bikeState.fRearWheelRot[2] = ((dReal *)dBodyGetRotation( m_RearWheelBodyID ))[4];
      m_bikeState.fRearWheelRot[3] = ((dReal *)dBodyGetRotation( m_RearWheelBodyID ))[5];   /* 16 bytes */
                                                                                        /* -------- */
                                                                                        /* 48 bytes total */

    m_bikeState.SwingAnchorP.x = m_bikeState.Anchors()->AR.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors()->AR.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.SwingAnchorP.y = m_bikeState.Anchors()->AR.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors()->AR.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;

    m_bikeState.SwingAnchor2P.x = m_bikeState.Anchors()->AR2.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors()->AR2.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.SwingAnchor2P.y = m_bikeState.Anchors()->AR2.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors()->AR2.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;

    m_bikeState.FrontAnchorP.x = m_bikeState.Anchors()->AF.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors()->AF.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.FrontAnchor2P.x = m_bikeState.Anchors()->AF2.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors()->AF2.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;

    m_bikeState.FrontAnchorP.y = m_bikeState.Anchors()->AF.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors()->AF.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;
    m_bikeState.FrontAnchor2P.y = m_bikeState.Anchors()->AF2.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors()->AF2.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;

    /* Calculate desired hand/foot positions */
    m_bikeState.WantedFootP.x = m_bikeState.Anchors()->PFp.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors()->PFp.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.WantedFootP.y = m_bikeState.Anchors()->PFp.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors()->PFp.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;
    m_bikeState.WantedHandP.x = m_bikeState.Anchors()->PHp.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors()->PHp.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.WantedHandP.y = m_bikeState.Anchors()->PHp.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors()->PHp.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;

    m_bikeState.WantedFoot2P.x = m_bikeState.Anchors()->PFp2.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors()->PFp2.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.WantedFoot2P.y = m_bikeState.Anchors()->PFp2.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors()->PFp2.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;
    m_bikeState.WantedHand2P.x = m_bikeState.Anchors()->PHp2.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors()->PHp2.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.WantedHand2P.y = m_bikeState.Anchors()->PHp2.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors()->PHp2.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;

    /* Still a replay question... */
      dVector3 T;

      dJointGetHingeAnchor(m_HandHingeID,T);
      m_bikeState.HandP.x = T[0]; m_bikeState.HandP.y = T[1];            /* 8 bytes */

      dJointGetHingeAnchor(m_ElbowHingeID,T);
      m_bikeState.ElbowP.x = T[0]; m_bikeState.ElbowP.y = T[1];          /* 8 bytes */

      dJointGetHingeAnchor(m_ShoulderHingeID,T);
      m_bikeState.ShoulderP.x = T[0]; m_bikeState.ShoulderP.y = T[1];    /* 8 bytes */

      dJointGetHingeAnchor(m_LowerBodyHingeID,T);
      m_bikeState.LowerBodyP.x = T[0]; m_bikeState.LowerBodyP.y = T[1];  /* 8 bytes */

      dJointGetHingeAnchor(m_KneeHingeID,T);
      m_bikeState.KneeP.x = T[0]; m_bikeState.KneeP.y = T[1];            /* 8 bytes */

      dJointGetHingeAnchor(m_FootHingeID,T);
      m_bikeState.FootP.x = T[0]; m_bikeState.FootP.y = T[1];            /* 8 bytes */
                                                                /* ------- */
                                                                /* 48 bytes total */

      dJointGetHingeAnchor(m_HandHingeID2,T);
      m_bikeState.Hand2P.x = T[0]; m_bikeState.Hand2P.y = T[1];

      dJointGetHingeAnchor(m_ElbowHingeID2,T);
      m_bikeState.Elbow2P.x = T[0]; m_bikeState.Elbow2P.y = T[1];

      dJointGetHingeAnchor(m_ShoulderHingeID2,T);
      m_bikeState.Shoulder2P.x = T[0]; m_bikeState.Shoulder2P.y = T[1];

      dJointGetHingeAnchor(m_LowerBodyHingeID2,T);
      m_bikeState.LowerBody2P.x = T[0]; m_bikeState.LowerBody2P.y = T[1];

      dJointGetHingeAnchor(m_KneeHingeID2,T);
      m_bikeState.Knee2P.x = T[0]; m_bikeState.Knee2P.y = T[1];

      dJointGetHingeAnchor(m_FootHingeID2,T);
      m_bikeState.Foot2P.x = T[0]; m_bikeState.Foot2P.y = T[1];

    Vector2f V;
    if(bUpdateRider) {
      /* Calculate head position */
      V = (m_bikeState.ShoulderP - m_bikeState.LowerBodyP);
      V.normalize();
      m_bikeState.HeadP = m_bikeState.ShoulderP + V*m_bikeState.Parameters()->fNeckLength;
    }

    if(bUpdateAltRider) {
      /* Calculate head position (Alt.) */
      V = (m_bikeState.Shoulder2P - m_bikeState.LowerBody2P);
      V.normalize();
      m_bikeState.Head2P = m_bikeState.Shoulder2P + V*m_bikeState.Parameters()->fNeckLength;
    }

    /* Internally we'd like to know the abs. relaxed position of the wheels */
    m_bikeState.RFrontWheelP.x = m_bikeState.Anchors()->Fp.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors()->Fp.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.RFrontWheelP.y = m_bikeState.Anchors()->Fp.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors()->Fp.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;
    m_bikeState.RRearWheelP.x = m_bikeState.Anchors()->Rp.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors()->Rp.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.RRearWheelP.y = m_bikeState.Anchors()->Rp.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors()->Rp.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;
  }

  /*===========================================================================
  Prepare rider
  ===========================================================================*/
  void PlayerBiker::prepareRider(Vector2f StartPos) {
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
    dBodySetPosition(m_PlayerTorsoBodyID,StartPos.x + m_bikeState.Anchors()->PTp.x,StartPos.y + m_bikeState.Anchors()->PTp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerTorsoMass,m_bikeState.Parameters()->BPm,0.4);
    dBodySetMass(m_PlayerTorsoBodyID,&m_PlayerTorsoMass);
    dBodySetFiniteRotationMode(m_PlayerTorsoBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerTorsoBodyID,0,0,1);

    dBodySetPosition(m_PlayerLLegBodyID,StartPos.x + m_bikeState.Anchors()->PLLp.x,StartPos.y + m_bikeState.Anchors()->PLLp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerLLegMass,m_bikeState.Parameters()->BPm,0.4);
    dBodySetMass(m_PlayerLLegBodyID,&m_PlayerLLegMass);
    dBodySetFiniteRotationMode(m_PlayerLLegBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerLLegBodyID,0,0,1);

    dBodySetPosition(m_PlayerULegBodyID,StartPos.x + m_bikeState.Anchors()->PULp.x,StartPos.y + m_bikeState.Anchors()->PULp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerULegMass,m_bikeState.Parameters()->BPm,0.4);
    dBodySetMass(m_PlayerULegBodyID,&m_PlayerULegMass);
    dBodySetFiniteRotationMode(m_PlayerULegBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerULegBodyID,0,0,1);

    dBodySetPosition(m_PlayerLArmBodyID,StartPos.x + m_bikeState.Anchors()->PLAp.x,StartPos.y + m_bikeState.Anchors()->PLAp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerLArmMass,m_bikeState.Parameters()->BPm,0.4);
    dBodySetMass(m_PlayerLArmBodyID,&m_PlayerLArmMass);
    dBodySetFiniteRotationMode(m_PlayerLArmBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerLArmBodyID,0,0,1);

    dBodySetPosition(m_PlayerUArmBodyID,StartPos.x + m_bikeState.Anchors()->PUAp.x,StartPos.y + m_bikeState.Anchors()->PUAp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerUArmMass,m_bikeState.Parameters()->BPm,0.4);
    dBodySetMass(m_PlayerUArmBodyID,&m_PlayerUArmMass);
    dBodySetFiniteRotationMode(m_PlayerUArmBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerUArmBodyID,0,0,1);

    dBodySetPosition(m_PlayerFootAnchorBodyID,StartPos.x + m_bikeState.Anchors()->PFp.x,StartPos.y + m_bikeState.Anchors()->PFp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerFootAnchorMass,m_bikeState.Parameters()->BPm,0.4);
    dBodySetMass(m_PlayerFootAnchorBodyID,&m_PlayerFootAnchorMass);
    dBodySetFiniteRotationMode(m_PlayerFootAnchorBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerFootAnchorBodyID,0,0,1);

    dBodySetPosition(m_PlayerHandAnchorBodyID,StartPos.x + m_bikeState.Anchors()->PHp.x,StartPos.y + m_bikeState.Anchors()->PHp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerHandAnchorMass,m_bikeState.Parameters()->BPm,0.4);
    dBodySetMass(m_PlayerHandAnchorBodyID,&m_PlayerHandAnchorMass);
    dBodySetFiniteRotationMode(m_PlayerHandAnchorBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerHandAnchorBodyID,0,0,1);

    /* Place and define the player bodies (Alt.) */
    dBodySetPosition(m_PlayerTorsoBodyID2,StartPos.x + m_bikeState.Anchors()->PTp2.x,StartPos.y + m_bikeState.Anchors()->PTp2.y,0.0f);
    dBodySetMass(m_PlayerTorsoBodyID2,&m_PlayerTorsoMass);
    dBodySetFiniteRotationMode(m_PlayerTorsoBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerTorsoBodyID2,0,0,1);

    dBodySetPosition(m_PlayerLLegBodyID2,StartPos.x + m_bikeState.Anchors()->PLLp2.x,StartPos.y + m_bikeState.Anchors()->PLLp2.y,0.0f);
    dBodySetMass(m_PlayerLLegBodyID2,&m_PlayerLLegMass);
    dBodySetFiniteRotationMode(m_PlayerLLegBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerLLegBodyID2,0,0,1);

    dBodySetPosition(m_PlayerULegBodyID2,StartPos.x + m_bikeState.Anchors()->PULp2.x,StartPos.y + m_bikeState.Anchors()->PULp2.y,0.0f);
    dBodySetMass(m_PlayerULegBodyID2,&m_PlayerULegMass);
    dBodySetFiniteRotationMode(m_PlayerULegBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerULegBodyID2,0,0,1);

    dBodySetPosition(m_PlayerLArmBodyID2,StartPos.x + m_bikeState.Anchors()->PLAp2.x,StartPos.y + m_bikeState.Anchors()->PLAp2.y,0.0f);
    dBodySetMass(m_PlayerLArmBodyID2,&m_PlayerLArmMass);
    dBodySetFiniteRotationMode(m_PlayerLArmBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerLArmBodyID2,0,0,1);

    dBodySetPosition(m_PlayerUArmBodyID2,StartPos.x + m_bikeState.Anchors()->PUAp2.x,StartPos.y + m_bikeState.Anchors()->PUAp2.y,0.0f);
    dBodySetMass(m_PlayerUArmBodyID2,&m_PlayerUArmMass);
    dBodySetFiniteRotationMode(m_PlayerUArmBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerUArmBodyID2,0,0,1);

    dBodySetPosition(m_PlayerFootAnchorBodyID2,StartPos.x + m_bikeState.Anchors()->PFp2.x,StartPos.y + m_bikeState.Anchors()->PFp2.y,0.0f);
    dBodySetMass(m_PlayerFootAnchorBodyID2,&m_PlayerFootAnchorMass);
    dBodySetFiniteRotationMode(m_PlayerFootAnchorBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerFootAnchorBodyID2,0,0,1);

    dBodySetPosition(m_PlayerHandAnchorBodyID2,StartPos.x + m_bikeState.Anchors()->PHp2.x,StartPos.y + m_bikeState.Anchors()->PHp2.y,0.0f);
    dBodySetMass(m_PlayerHandAnchorBodyID2,&m_PlayerHandAnchorMass);
    dBodySetFiniteRotationMode(m_PlayerHandAnchorBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerHandAnchorBodyID2,0,0,1);

    float fERP = 0.3; float fCFM = 0.03f;

    /* Connect em */
    m_KneeHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_KneeHingeID,m_PlayerLLegBodyID,m_PlayerULegBodyID);
    dJointSetHingeAnchor(m_KneeHingeID,StartPos.x + m_bikeState.Parameters()->PKVx,StartPos.y + m_bikeState.Parameters()->PKVy,0.0f);
    dJointSetHingeAxis(m_KneeHingeID,0,0,1);
    dJointSetHingeParam(m_KneeHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_KneeHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_KneeHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_KneeHingeID,dParamStopCFM,fCFM);

    m_LowerBodyHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_LowerBodyHingeID,m_PlayerULegBodyID,m_PlayerTorsoBodyID);
    dJointSetHingeAnchor(m_LowerBodyHingeID,StartPos.x + m_bikeState.Parameters()->PLVx,StartPos.y + m_bikeState.Parameters()->PLVy,0.0f);
    dJointSetHingeAxis(m_LowerBodyHingeID,0,0,1);
    dJointSetHingeParam(m_LowerBodyHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_LowerBodyHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_LowerBodyHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_LowerBodyHingeID,dParamStopCFM,fCFM);

    m_ShoulderHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_ShoulderHingeID,m_PlayerTorsoBodyID,m_PlayerUArmBodyID);
    dJointSetHingeAnchor(m_ShoulderHingeID,StartPos.x + m_bikeState.Parameters()->PSVx,StartPos.y + m_bikeState.Parameters()->PSVy,0.0f);
    dJointSetHingeAxis(m_ShoulderHingeID,0,0,1);
    dJointSetHingeParam(m_ShoulderHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_ShoulderHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_ShoulderHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_ShoulderHingeID,dParamStopCFM,fCFM);

    m_ElbowHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_ElbowHingeID,m_PlayerUArmBodyID,m_PlayerLArmBodyID);
    dJointSetHingeAnchor(m_ElbowHingeID,StartPos.x + m_bikeState.Parameters()->PEVx,StartPos.y + m_bikeState.Parameters()->PEVy,0.0f);
    dJointSetHingeAxis(m_ElbowHingeID,0,0,1);
    dJointSetHingeParam(m_ElbowHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_ElbowHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_ElbowHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_ElbowHingeID,dParamStopCFM,fCFM);

    m_FootHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_FootHingeID,m_PlayerFootAnchorBodyID,m_PlayerLLegBodyID);
    dJointSetHingeAnchor(m_FootHingeID,StartPos.x + m_bikeState.Parameters()->PFVx,StartPos.y + m_bikeState.Parameters()->PFVy,0.0f);
    dJointSetHingeAxis(m_FootHingeID,0,0,1);
    dJointSetHingeParam(m_FootHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_FootHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_FootHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_FootHingeID,dParamStopCFM,fCFM);

    m_HandHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_HandHingeID,m_PlayerLArmBodyID,m_PlayerHandAnchorBodyID);
    dJointSetHingeAnchor(m_HandHingeID,StartPos.x + m_bikeState.Parameters()->PHVx,StartPos.y + m_bikeState.Parameters()->PHVy,0.0f);
    dJointSetHingeAxis(m_HandHingeID,0,0,1);
    dJointSetHingeParam(m_HandHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_HandHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_HandHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_HandHingeID,dParamStopCFM,fCFM);

    /* Connect em (Alt.) */
    m_KneeHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_KneeHingeID2,m_PlayerLLegBodyID2,m_PlayerULegBodyID2);
    dJointSetHingeAnchor(m_KneeHingeID2,StartPos.x - m_bikeState.Parameters()->PKVx,StartPos.y + m_bikeState.Parameters()->PKVy,0.0f);
    dJointSetHingeAxis(m_KneeHingeID2,0,0,1);
    dJointSetHingeParam(m_KneeHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_KneeHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_KneeHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_KneeHingeID2,dParamStopCFM,fCFM);

    m_LowerBodyHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_LowerBodyHingeID2,m_PlayerULegBodyID2,m_PlayerTorsoBodyID2);
    dJointSetHingeAnchor(m_LowerBodyHingeID2,StartPos.x - m_bikeState.Parameters()->PLVx,StartPos.y + m_bikeState.Parameters()->PLVy,0.0f);
    dJointSetHingeAxis(m_LowerBodyHingeID2,0,0,1);
    dJointSetHingeParam(m_LowerBodyHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_LowerBodyHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_LowerBodyHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_LowerBodyHingeID2,dParamStopCFM,fCFM);

    m_ShoulderHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_ShoulderHingeID2,m_PlayerTorsoBodyID2,m_PlayerUArmBodyID2);
    dJointSetHingeAnchor(m_ShoulderHingeID2,StartPos.x - m_bikeState.Parameters()->PSVx,StartPos.y + m_bikeState.Parameters()->PSVy,0.0f);
    dJointSetHingeAxis(m_ShoulderHingeID2,0,0,1);
    dJointSetHingeParam(m_ShoulderHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_ShoulderHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_ShoulderHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_ShoulderHingeID2,dParamStopCFM,fCFM);

    m_ElbowHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_ElbowHingeID2,m_PlayerUArmBodyID2,m_PlayerLArmBodyID2);
    dJointSetHingeAnchor(m_ElbowHingeID2,StartPos.x - m_bikeState.Parameters()->PEVx,StartPos.y + m_bikeState.Parameters()->PEVy,0.0f);
    dJointSetHingeAxis(m_ElbowHingeID2,0,0,1);
    dJointSetHingeParam(m_ElbowHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_ElbowHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_ElbowHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_ElbowHingeID2,dParamStopCFM,fCFM);

    m_FootHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_FootHingeID2,m_PlayerFootAnchorBodyID2,m_PlayerLLegBodyID2);
    dJointSetHingeAnchor(m_FootHingeID2,StartPos.x - m_bikeState.Parameters()->PFVx,StartPos.y + m_bikeState.Parameters()->PFVy,0.0f);
    dJointSetHingeAxis(m_FootHingeID2,0,0,1);
    dJointSetHingeParam(m_FootHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_FootHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_FootHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_FootHingeID2,dParamStopCFM,fCFM);

    m_HandHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_HandHingeID2,m_PlayerLArmBodyID2,m_PlayerHandAnchorBodyID2);
    dJointSetHingeAnchor(m_HandHingeID2,StartPos.x - m_bikeState.Parameters()->PHVx,StartPos.y + m_bikeState.Parameters()->PHVy,0.0f);
    dJointSetHingeAxis(m_HandHingeID2,0,0,1);
    dJointSetHingeParam(m_HandHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_HandHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_HandHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_HandHingeID2,dParamStopCFM,fCFM);
  }

  /*===========================================================================
  Set up bike physics
  ===========================================================================*/
  void PlayerBiker::prepareBikePhysics(Vector2f StartPos) {
    /* Create bodies */
    m_FrontWheelBodyID = dBodyCreate(m_WorldID);
    m_RearWheelBodyID = dBodyCreate(m_WorldID);
    m_FrameBodyID = dBodyCreate(m_WorldID);

    /* Place and define the rear wheel */
    dBodySetPosition(m_RearWheelBodyID,StartPos.x + m_bikeState.Anchors()->Rp.x,StartPos.y + m_bikeState.Anchors()->Rp.y,0.0f);
/*    const dReal *pf;
    pf = dBodyGetAngularVel(m_RearWheelBodyID);
    printf("[%f %f %f]\n",pf[0],pf[1],pf[2]);*/
    dMassSetSphereTotal(&m_RearWheelMass,m_bikeState.Parameters()->Wm,m_bikeState.Parameters()->WheelRadius());
    dBodySetMass(m_RearWheelBodyID,&m_RearWheelMass);
    dBodySetFiniteRotationMode(m_RearWheelBodyID,1);
    dBodySetFiniteRotationAxis(m_RearWheelBodyID,0,0,1);

    /* Place and define the front wheel */
    dBodySetPosition(m_FrontWheelBodyID,StartPos.x + m_bikeState.Anchors()->Fp.x,StartPos.y + m_bikeState.Anchors()->Fp.y,0.0f);
    dMassSetSphereTotal(&m_FrontWheelMass,m_bikeState.Parameters()->Wm,m_bikeState.Parameters()->WheelRadius());
    dBodySetMass(m_FrontWheelBodyID,&m_FrontWheelMass);
    dBodySetFiniteRotationMode(m_FrontWheelBodyID,1);
    dBodySetFiniteRotationAxis(m_FrontWheelBodyID,0,0,1);

    /* Place and define the frame */
    dBodySetPosition(m_FrameBodyID,StartPos.x,StartPos.y,0.0f);
    dMassSetBoxTotal(&m_FrameMass,m_bikeState.Parameters()->Fm,m_bikeState.Parameters()->IL,m_bikeState.Parameters()->IH,DEPTH_FACTOR);
    dBodySetMass(m_FrameBodyID,&m_FrameMass);
    dBodySetFiniteRotationMode(m_FrameBodyID,1);
    dBodySetFiniteRotationAxis(m_FrameBodyID,0,0,1);

    /* Prepare rider */
    prepareRider(StartPos);
  }

void PlayerBiker::setBodyDetach(bool state) {
  Biker::setBodyDetach(state);

  if(m_bodyDetach) {
    dJointSetHingeParam(m_KneeHingeID,  dParamLoStop, 0.0);
    dJointSetHingeParam(m_KneeHingeID,  dParamHiStop, 3.14159/8.0);
    dJointSetHingeParam(m_KneeHingeID2, dParamLoStop, 3.14159/8.0 * -1.0);
    dJointSetHingeParam(m_KneeHingeID2, dParamHiStop, 0.0         * -1.0);

    dJointSetHingeParam(m_LowerBodyHingeID,  dParamLoStop,  -1.2);
    dJointSetHingeParam(m_LowerBodyHingeID,  dParamHiStop,  0.0);
    dJointSetHingeParam(m_LowerBodyHingeID2, dParamLoStop, 0.0  * -1.0);
    dJointSetHingeParam(m_LowerBodyHingeID2, dParamHiStop, -1.2 * -1.0);

    dJointSetHingeParam(m_ShoulderHingeID,  dParamLoStop, -2.0);
    dJointSetHingeParam(m_ShoulderHingeID,  dParamHiStop,  0.0);
    dJointSetHingeParam(m_ShoulderHingeID2, dParamLoStop,  0.0  * -1.0);
    dJointSetHingeParam(m_ShoulderHingeID2, dParamHiStop, -2.0  * -1.0);

    dJointSetHingeParam(m_ElbowHingeID,  dParamLoStop, -1.5);
    dJointSetHingeParam(m_ElbowHingeID,  dParamHiStop, 1.0);
    dJointSetHingeParam(m_ElbowHingeID2, dParamLoStop, 1.0   * -1.0);
    dJointSetHingeParam(m_ElbowHingeID2, dParamHiStop, -1.5  * -1.0);
  }
}

void PlayerBiker::addBodyForce(const Vector2f& i_force) {
  m_forceToAdd += i_force;
  resetAutoDisabler();
}

void PlayerBiker::resetAutoDisabler() {
  m_nStillFrames = 0;
}

bool PlayerBiker::isSqueeking() {
  return m_bSqueeking;
}

float PlayerBiker::howMuchSqueek() {
  return m_fHowMuchSqueek;
}

bool PlayerBiker::getRenderBikeFront() {
  return m_bodyDetach == false;
}
