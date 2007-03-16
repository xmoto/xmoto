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

#include "Bike.h"
#include "../Replay.h"
#include "../MotoGame.h"
#include "../GameEvents.h"
#include "../GameText.h"

#define PHYSICAL_ENGINE_REDUCTION 0.05

BikeState::BikeState() {
  reInitializeSpeed();
  reInitializeAnchors();
}

BikeState::~BikeState() {
}


  /*===========================================================================
    Matrix encodings
    ===========================================================================*/
  unsigned short BikeState::_MatrixTo16Bits(const float *pfMatrix) {
    /* The idea is that we only need to store the first column of the matrix,
       as the second on is simply the first one transposed... Each of the
       two components of the first column is given 8 bits of precision */
    int n1 = (int)(pfMatrix[0] * 127.0f + 127.0f);   
    if(n1<0) n1=0;
    if(n1>255) n1=255;    
    unsigned char c1 = n1;
    int n2 = (int)(pfMatrix[2] * 127.0f + 127.0f);   
    if(n2<0) n2=0;
    if(n2>255) n2=255;
    unsigned char c2 = n2;
    return (unsigned short) ((((unsigned short)c1)<<8)|(unsigned short)c2);
  }
  
  void BikeState::_16BitsToMatrix(unsigned short n16,float *pfMatrix) {
    /* Convert it back again */
    int n1 = (int)((n16&0xff00)>>8);
    int n2 = (int)(n16&0xff);
    pfMatrix[0] = (((float)n1) - 127.0f) / 127.0f;
    pfMatrix[2] = (((float)n2) - 127.0f) / 127.0f;
    
    /* Make sure the column is normalized */
    float d = sqrt(pfMatrix[0]*pfMatrix[0] + pfMatrix[2]*pfMatrix[2]);
    if(d == 0.0f) {
      /* It's null... */
      pfMatrix[0] = 1.0f; pfMatrix[1] = 0.0f;
      pfMatrix[2] = 0.0f; pfMatrix[3] = 1.0f;
    }
    else {
      pfMatrix[0] /= d;
      pfMatrix[2] /= d;
      
      /* Transpose second column */
      pfMatrix[1] = -pfMatrix[2];
      pfMatrix[3] = pfMatrix[0];
    }
  }

  /*===========================================================================
    Neat trick for converting floating-point numbers to 8 bits
    ===========================================================================*/
  signed char BikeState::_MapCoordTo8Bits(float fRef,float fMaxDiff,
      float fCoord) {
    int n = (int)((127.0f * (fCoord-fRef))/fMaxDiff);
    if(n<-127) n=-127;
    if(n>127) n=127;
    return (signed char)(n&0xff);
  }
  
  float BikeState::_Map8BitsToCoord(float fRef,float fMaxDiff,signed char c) {
    return fRef + (((float)c)/127.0f) * fMaxDiff;
  }


void BikeState::reInitializeSpeed() {
  m_curBrake  = 0.0;
  m_curEngine = 0.0;
}

float BikeState::CurrentBrake() const {
  return m_curBrake;
}

float BikeState::CurrentEngine() const {
  return m_curEngine;
}

void BikeState::physicalUpdate() {
  if(m_curEngine > 0.0)
    m_curEngine -= m_bikeParameters.MaxEngine() * PHYSICAL_ENGINE_REDUCTION; 
}

void BikeState::reInitializeAnchors() {
  m_bikeAnchors.update(m_bikeParameters);
}

BikeAnchors& BikeState::Anchors() {
  return m_bikeAnchors;
}

BikeParameters& BikeState::Parameters() {
  return m_bikeParameters;
}

BikeState* Biker::getState() {
  return &m_bikeState;
}

Ghost::Ghost(std::string i_replayFile, bool i_isActiv) {
  std::string v_levelId;
  std::string v_playerName;
  float v_framerate;
  SerializedBikeState GhostBikeState;

  m_replay = new vapp::Replay();
  v_levelId = m_replay->openReplay(i_replayFile, &v_framerate, v_playerName);

  m_replay->peekState(GhostBikeState);
  BikeState::updateStateFromReplay(&GhostBikeState, &m_bikeState);

  m_diffToPlayer = 0.0;
  m_nFrame = 0;

  m_info = "";
  m_isActiv = i_isActiv;
}

Ghost::~Ghost() {
}

void Ghost::execReplayEvents(float i_time, vapp::MotoGame *i_motogame) {
  std::vector<vapp::RecordedGameEvent *> *v_replayEvents;
  v_replayEvents = m_replay->getEvents();
  
  /* Start looking for events that should be passed */
  for(int i=0;i<v_replayEvents->size();i++) {
    /* Not passed? And with a time stamp that tells it should have happened
       by now? */
    if(!(*v_replayEvents)[i]->bPassed && (*v_replayEvents)[i]->Event->getEventTime() < i_time) {
      /* Nice. Handle this event, replay style */
      i_motogame->handleEvent((*v_replayEvents)[i]->Event);
      
      /* Pass it */
      (*v_replayEvents)[i]->bPassed = true;
    }
  }
  
  /* Now see if we have moved back in time and whether we should apply some
     REVERSE events */
  for(int i=v_replayEvents->size()-1;i>=0;i--) {
    /* Passed? And with a time stamp larger than current time? */
    if((*v_replayEvents)[i]->bPassed && (*v_replayEvents)[i]->Event->getEventTime() > i_time) {
      /* Nice. Handle this event, replay style BACKWARDS */
      (*v_replayEvents)[i]->Event->revert(i_motogame);
      
      /* Un-pass it */
      (*v_replayEvents)[i]->bPassed = false;
    }
  }
}

void Ghost::setInfo(std::string i_info) {
  m_info = i_info;
}

std::string Ghost::getDescription() const {
  return
    std::string(GAMETEXT_GHOSTOF)   +
    " " + m_replay->getPlayerName() +
    "\n(" + m_info + ")"            +
    "\n(" + vapp::App::formatTime(m_replay->getFinishTime()) + ")";
}

void Ghost::initLastToTakeEntities(Level* i_level) {
  std::vector<vapp::RecordedGameEvent *> *v_replayEvents;
  v_replayEvents = m_replay->getEvents();
    
  m_lastToTakeEntities.clear();
  m_diffToPlayer = 0.0;

  /* Start looking for events */
  for(int i=0; i<v_replayEvents->size(); i++) {
    vapp::MotoGameEvent *v_event = (*v_replayEvents)[i]->Event;
      
    if(v_event->getType() == vapp::GAME_EVENT_ENTITY_DESTROYED) {
      if(i_level->getEntityById(((vapp::MGE_EntityDestroyed*)v_event)->EntityId()).IsToTake()) {
	/* new Strawberry for ghost */
	m_lastToTakeEntities.push_back((*v_replayEvents)[i]->Event->getEventTime());
      }
    }
  }
}

float Ghost::diffToPlayer() const {
  return m_diffToPlayer;
}

void Ghost::updateDiffToPlayer(std::vector<float> &i_lastToTakeEntities) {
  int v_n;
    
  /* no strawberry, no update */
  if(i_lastToTakeEntities.size() == 0) {
    return;
  }
    
  /* the ghost did not get this number of strawberries */
  if(m_lastToTakeEntities.size() < i_lastToTakeEntities.size() ) {
    return;
  }
    
  m_diffToPlayer = i_lastToTakeEntities[i_lastToTakeEntities.size()-1]
                 - m_lastToTakeEntities[i_lastToTakeEntities.size()-1];
}

void Ghost::updateToTime(float i_time, vapp::MotoGame *i_motogame) {
  /* Read replay state */
  static SerializedBikeState GhostBikeState;
  static SerializedBikeState previousGhostBikeState;
    
  m_replay->peekState(previousGhostBikeState);

  /* back in the past */
  if(i_time < previousGhostBikeState.fGameTime) {
    m_replay->fastrewind(previousGhostBikeState.fGameTime - i_time);
    m_replay->peekState(previousGhostBikeState);
  }

  if(previousGhostBikeState.fGameTime < i_time && m_replay->endOfFile() == false) {
    do {
      m_replay->loadState(GhostBikeState);
    } while(GhostBikeState.fGameTime < i_time && m_replay->endOfFile() == false);
    
    if(m_nFrame%2 || m_nFrame==1) {
      /* NON-INTERPOLATED FRAME */
      BikeState::updateStateFromReplay(&GhostBikeState, &m_bikeState);
    } 
    else {
      /* INTERPOLATED FRAME */
	SerializedBikeState ibs;
	BikeState::interpolateGameState(&previousGhostBikeState,&GhostBikeState,&ibs,0.5f);
	BikeState::updateStateFromReplay(&ibs, &m_bikeState);
    }
    m_nFrame++;
  }

  if(m_isActiv) {
    execReplayEvents(i_time, i_motogame);
  }
}

float Ghost::getFinishTime() {
  return m_replay->getFinishTime();
}

std::string Ghost::levelId() {
   return m_replay->getLevelId();
}

bool Ghost::getRenderBikeFront() {
  return true;
}

float Ghost::getBikeEngineSpeed() {
  return 0.0; /* unable to know it */
}

PlayerBiker::PlayerBiker(Vector2f i_position, DriveDir i_direction, Vector2f i_gravity, Theme *i_theme) {
  m_somersaultCounter.init();

  bFrontWheelTouching = false;
  bRearWheelTouching  = false;

  bFrontWheelTouching = false;
  bRearWheelTouching  = false;

  m_bWheelSpin = false;
  m_bSqueeking=false;
  m_nStillFrames = 0;
  m_clearDynamicTouched = false;
  m_fLastSqueekTime = 0.0f;

  clearStates();
  initPhysics(i_gravity);
  initToPosition(i_position, i_direction, i_gravity);
  m_bikerHooks = NULL;

  /* sound engine */
  if(vapp::Sound::isEnabled()) {
    try {
      m_EngineSound.addBangSample(vapp::Sound::findSample(i_theme->getSound("Engine00")->FilePath()));
      m_EngineSound.addBangSample(vapp::Sound::findSample(i_theme->getSound("Engine01")->FilePath()));
      m_EngineSound.addBangSample(vapp::Sound::findSample(i_theme->getSound("Engine02")->FilePath()));
      m_EngineSound.addBangSample(vapp::Sound::findSample(i_theme->getSound("Engine03")->FilePath()));
      m_EngineSound.addBangSample(vapp::Sound::findSample(i_theme->getSound("Engine04")->FilePath()));
      m_EngineSound.addBangSample(vapp::Sound::findSample(i_theme->getSound("Engine05")->FilePath()));
      m_EngineSound.addBangSample(vapp::Sound::findSample(i_theme->getSound("Engine06")->FilePath()));
      m_EngineSound.addBangSample(vapp::Sound::findSample(i_theme->getSound("Engine07")->FilePath()));
      m_EngineSound.addBangSample(vapp::Sound::findSample(i_theme->getSound("Engine08")->FilePath()));
      m_EngineSound.addBangSample(vapp::Sound::findSample(i_theme->getSound("Engine09")->FilePath()));
      m_EngineSound.addBangSample(vapp::Sound::findSample(i_theme->getSound("Engine10")->FilePath()));
      m_EngineSound.addBangSample(vapp::Sound::findSample(i_theme->getSound("Engine11")->FilePath()));
      m_EngineSound.addBangSample(vapp::Sound::findSample(i_theme->getSound("Engine12")->FilePath()));
    } catch(Exception &e) {
      /* hum, no nice */
    }
  }
}

PlayerBiker::~PlayerBiker() {
  uninitPhysics();
}

void PlayerBiker::updateToTime(float i_time, float i_timeStep, vapp::CollisionSystem *v_collisionSystem, Vector2f i_gravity) {
  m_bSqueeking = false; /* no squeeking right now */

  updatePhysics(i_time, i_timeStep, v_collisionSystem, i_gravity);
  updateGameState();

  /* Squeeking? */
  if(isSqueeking()) {
    if(i_time - m_fLastSqueekTime > 0.1f) {
      /* (this is crappy, not enabled) */
      //Sound::playSampleByName("Sounds/Squeek.ogg",m_MotoGame.howMuchSqueek());
      m_fLastSqueekTime = i_time;
    }
  }

  /* controler */
  bool bChangeDir = false;
  if(m_BikeC.ChangeDir()) {
    m_BikeC.setChangeDir(false);
    bChangeDir = true;
    
    m_bikeState.Dir = m_bikeState.Dir==DD_LEFT?DD_RIGHT:DD_LEFT; /* switch */
  }

  /* sound */
  if(vapp::Sound::isEnabled()) {
    m_EngineSound.setRPM(getBikeEngineRPM()); 
    m_EngineSound.update(i_time);
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
  m_bikeState.Parameters().setDefaults();
}

void PlayerBiker::uninitPhysics(void) {
  dJointGroupDestroy(m_ContactGroup);
  dWorldDestroy(m_WorldID);
  dCloseODE();
}

void PlayerBiker::updatePhysics(float i_time, float fTimeStep, vapp::CollisionSystem *v_collisionSystem, Vector2f i_gravity) {
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
  if((m_BikeC.Pull() != 0.0f) && (i_time > m_fNextAttitudeCon)) {
    m_fAttitudeCon = m_BikeC.Pull() * PHYS_RIDER_ATTITUDE_TORQUE;
    m_fNextAttitudeCon = i_time + (0.6f * fabsf(m_BikeC.Pull()));
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
  /* Simply map the wheel ang vel to the RPM (stupid, lame, lots of bad stuff) */
  if(m_bikeState.Dir == DD_RIGHT) {
    float f = -fRearWheelAngVel;
    if(f<0.0f) f=0.0f;      
    m_bikeState.fBikeEngineRPM = ENGINE_MIN_RPM + (ENGINE_MAX_RPM - ENGINE_MIN_RPM) * (f / PHYS_MAX_ROLL_VELOCITY) * m_BikeC.Drive();
    if(m_bikeState.fBikeEngineRPM < ENGINE_MIN_RPM) m_bikeState.fBikeEngineRPM = ENGINE_MIN_RPM;
    if(m_bikeState.fBikeEngineRPM > ENGINE_MAX_RPM) m_bikeState.fBikeEngineRPM = ENGINE_MAX_RPM;
  }
  else if(m_bikeState.Dir == DD_LEFT) {
    float f = fFrontWheelAngVel;
    if(f<0.0f) f=0.0f;      
    m_bikeState.fBikeEngineRPM = ENGINE_MIN_RPM + (ENGINE_MAX_RPM - ENGINE_MIN_RPM) * (f / PHYS_MAX_ROLL_VELOCITY) * m_BikeC.Drive();
    if(m_bikeState.fBikeEngineRPM < ENGINE_MIN_RPM) m_bikeState.fBikeEngineRPM = ENGINE_MIN_RPM;
    if(m_bikeState.fBikeEngineRPM > ENGINE_MAX_RPM) m_bikeState.fBikeEngineRPM = ENGINE_MAX_RPM;
  }

  /* Apply motor/brake torques */
  if(m_BikeC.Drive() < 0.0f) {
    /* Brake! */        
    if(!bSleep) {
      //printf("Brake!\n");
      
      dBodyAddTorque(m_RearWheelBodyID,0,0,dBodyGetAngularVel(m_RearWheelBodyID)[2]*PHYS_BRAKE_FACTOR*m_BikeC.Drive());
      dBodyAddTorque(m_FrontWheelBodyID,0,0,dBodyGetAngularVel(m_FrontWheelBodyID)[2]*PHYS_BRAKE_FACTOR*m_BikeC.Drive());
    }
  }
  else {
    /* Throttle? */
    if(m_BikeC.Drive() > 0.0f) {
      if(m_bikeState.Dir == DD_RIGHT) {
	if(fRearWheelAngVel > -PHYS_MAX_ROLL_VELOCITY) {
	  m_nStillFrames=0;
	  dBodyEnable(m_FrontWheelBodyID);
	  dBodyEnable(m_RearWheelBodyID);
	  dBodyEnable(m_FrameBodyID);
            
	  dBodyAddTorque(m_RearWheelBodyID,0,0,-m_bikeState.Parameters().MaxEngine()*PHYS_ENGINE_DAMP*m_BikeC.Drive());    
            
	  //printf("Drive!\n");
	}
      }
      else {
	if(fFrontWheelAngVel < PHYS_MAX_ROLL_VELOCITY) {
	  m_nStillFrames=0;
	  dBodyEnable(m_FrontWheelBodyID);
	  dBodyEnable(m_RearWheelBodyID);
	  dBodyEnable(m_FrameBodyID);

	  dBodyAddTorque(m_FrontWheelBodyID,0,0,m_bikeState.Parameters().MaxEngine()*PHYS_ENGINE_DAMP*m_BikeC.Drive());    
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
        
  nNumContacts = intersectWheelLevel( m_bikeState.FrontWheelP,m_bikeState.Parameters().WheelRadius(),Contacts, v_collisionSystem);
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
      if(fabs(fFrontWheelAngVel) > 5 && m_BikeC.Drive()>0.0f && nNumContacts > 0) {
	m_bWheelSpin = true;
	m_WheelSpinPoint = WSP;
	m_WheelSpinDir.x = (((m_bikeState.FrontWheelP.y - WSP.y))*1 + (m_bikeState.FrontWheelP.x - WSP.x)) /2;
	m_WheelSpinDir.y = ((-(m_bikeState.FrontWheelP.x - WSP.x))*1 + (m_bikeState.FrontWheelP.y - WSP.y)) /2;
      }
    }
  }

  nNumContacts = intersectWheelLevel( m_bikeState.RearWheelP,m_bikeState.Parameters().WheelRadius(),Contacts, v_collisionSystem);
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
      if(fabs(fRearWheelAngVel) > 5 && m_BikeC.Drive()>0 && nNumContacts > 0) {
	m_bWheelSpin = true;
	m_WheelSpinPoint = WSP;
	m_WheelSpinDir.x = ((-(m_bikeState.RearWheelP.y - WSP.y))*1 + (m_bikeState.RearWheelP.x - WSP.x)) /2;
	m_WheelSpinDir.y = (((m_bikeState.RearWheelP.x - WSP.x))*1 + (m_bikeState.RearWheelP.y - WSP.y)) /2;
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
    if(intersectHeadLevel(m_bikeState.HeadP,m_bikeState.Parameters().HeadSize(),m_PrevActiveHead, v_collisionSystem)) {
      if(m_bikerHooks != NULL) {
	m_bikerHooks->onHeadTouches();
      }
    }
      
    m_PrevActiveHead = m_bikeState.HeadP;
  }
  else if(m_bikeState.Dir == DD_LEFT) {
    if(intersectHeadLevel(m_bikeState.Head2P,m_bikeState.Parameters().HeadSize(),m_PrevActiveHead, v_collisionSystem)) {
      if(m_bikerHooks != NULL) {
	m_bikerHooks->onHeadTouches();
      }
    }

    m_PrevActiveHead = m_bikeState.Head2P;
  }
        
  //m_PrevFrontWheelP = m_bikeState.FrontWheelP;
  //m_PrevRearWheelP = m_bikeState.RearWheelP;
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
  dWorldQuickStep(m_WorldID,fTimeStep*PHYS_SPEED);
  //dWorldStep(m_WorldID,fTimeStep*PHYS_SPEED);
    
  /* Empty contact joint group */
  dJointGroupEmpty(m_ContactGroup);
    
  m_clearDynamicTouched = v_collisionSystem->isDynamicTouched();

  m_bFirstPhysicsUpdate = false;
}

bool PlayerBiker::intersectHeadLevel(Vector2f Cp,float Cr,const Vector2f &LastCp, vapp::CollisionSystem *v_collisionSystem) {
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

int PlayerBiker::intersectWheelLevel(Vector2f Cp,float Cr,dContact *pContacts, vapp::CollisionSystem *v_collisionSystem) {
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

      /* Calculate bike stuff */
      m_bikeState.reInitializeAnchors();
      Vector2f C(i_position - m_bikeState.Anchors().GroundPoint());
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
    
  /* BIKE_C */
  memset(&m_BikeC,0,sizeof(m_BikeC)); 
}

BikeController* PlayerBiker::getControler() {
  return &m_BikeC;
}

bool PlayerBiker::isTouching(const Entity& i_entity) const {
  for(int i=0; i<m_entitiesTouching.size(); i++) {
    if(m_entitiesTouching[i] == &i_entity) {
      return true;
    }
  }
  return false;
}

PlayerBiker::touch PlayerBiker::setTouching(Entity& i_entity, bool i_touching) {
  bool v_wasTouching = isTouching(i_entity);
  if(v_wasTouching == i_touching) {
    return none;
  }
  
  if(i_touching) {
    m_entitiesTouching.push_back(&i_entity);
    return added;
  } else {
    for(int i=0; i<m_entitiesTouching.size(); i++) {
      if(m_entitiesTouching[i] == &i_entity) {
	m_entitiesTouching.erase(m_entitiesTouching.begin() + i);
	return removed;
      }
    }
  }
  return none;
}

bool PlayerBiker::isTouching(const Zone& i_zone) const {
  for(unsigned int i=0; i<m_zonesTouching.size(); i++) {
    if(m_zonesTouching[i]->Id() == i_zone.Id()) {
      return true;
    }
  }
  return false;
}

PlayerBiker::touch PlayerBiker::setTouching(Zone& i_zone, bool i_isTouching) {
  bool v_wasTouching = isTouching(i_zone);
  if(v_wasTouching == i_isTouching) {
    return none;
  }
    
  if(i_isTouching) {
    m_zonesTouching.push_back(&i_zone);
    return added;
  } else {
    for(int i=0; i<m_zonesTouching.size(); i++) {
      if(m_zonesTouching[i] == &i_zone) {
	m_zonesTouching.erase(m_zonesTouching.begin() + i);
	return removed;
      }
    }
  }
  return none;
}

std::vector<Entity *>& PlayerBiker::EntitiesTouching() {
  return m_entitiesTouching;
}

std::vector<Zone *>& PlayerBiker::ZonesTouching() {
  return m_zonesTouching;
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

float PlayerBiker::getBikeEngineRPM() {
  return m_bikeState.fBikeEngineRPM;
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
    
    m_bikeState.SwingAnchorP.x = m_bikeState.Anchors().AR.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors().AR.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.SwingAnchorP.y = m_bikeState.Anchors().AR.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors().AR.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;

    m_bikeState.SwingAnchor2P.x = m_bikeState.Anchors().AR2.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors().AR2.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.SwingAnchor2P.y = m_bikeState.Anchors().AR2.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors().AR2.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;

    m_bikeState.FrontAnchorP.x = m_bikeState.Anchors().AF.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors().AF.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.FrontAnchor2P.x = m_bikeState.Anchors().AF2.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors().AF2.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;

    m_bikeState.FrontAnchorP.y = m_bikeState.Anchors().AF.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors().AF.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;
    m_bikeState.FrontAnchor2P.y = m_bikeState.Anchors().AF2.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors().AF2.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;

    /* Calculate desired hand/foot positions */
    m_bikeState.WantedFootP.x = m_bikeState.Anchors().PFp.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors().PFp.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.WantedFootP.y = m_bikeState.Anchors().PFp.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors().PFp.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;
    m_bikeState.WantedHandP.x = m_bikeState.Anchors().PHp.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors().PHp.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.WantedHandP.y = m_bikeState.Anchors().PHp.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors().PHp.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;    
        
    m_bikeState.WantedFoot2P.x = m_bikeState.Anchors().PFp2.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors().PFp2.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.WantedFoot2P.y = m_bikeState.Anchors().PFp2.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors().PFp2.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;
    m_bikeState.WantedHand2P.x = m_bikeState.Anchors().PHp2.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors().PHp2.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.WantedHand2P.y = m_bikeState.Anchors().PHp2.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors().PHp2.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;    

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
      m_bikeState.HeadP = m_bikeState.ShoulderP + V*m_bikeState.Parameters().fNeckLength;
    }

    if(bUpdateAltRider) {
      /* Calculate head position (Alt.) */
      V = (m_bikeState.Shoulder2P - m_bikeState.LowerBody2P);
      V.normalize();
      m_bikeState.Head2P = m_bikeState.Shoulder2P + V*m_bikeState.Parameters().fNeckLength;
    }

    /* Internally we'd like to know the abs. relaxed position of the wheels */
    m_bikeState.RFrontWheelP.x = m_bikeState.Anchors().Fp.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors().Fp.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.RFrontWheelP.y = m_bikeState.Anchors().Fp.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors().Fp.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;
    m_bikeState.RRearWheelP.x = m_bikeState.Anchors().Rp.x*m_bikeState.fFrameRot[0] + m_bikeState.Anchors().Rp.y*m_bikeState.fFrameRot[1] + m_bikeState.CenterP.x;
    m_bikeState.RRearWheelP.y = m_bikeState.Anchors().Rp.x*m_bikeState.fFrameRot[2] + m_bikeState.Anchors().Rp.y*m_bikeState.fFrameRot[3] + m_bikeState.CenterP.y;       
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
    dBodySetPosition(m_PlayerTorsoBodyID,StartPos.x + m_bikeState.Anchors().PTp.x,StartPos.y + m_bikeState.Anchors().PTp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerTorsoMass,m_bikeState.Parameters().BPm,0.4);
    dBodySetMass(m_PlayerTorsoBodyID,&m_PlayerTorsoMass);
    dBodySetFiniteRotationMode(m_PlayerTorsoBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerTorsoBodyID,0,0,1);

    dBodySetPosition(m_PlayerLLegBodyID,StartPos.x + m_bikeState.Anchors().PLLp.x,StartPos.y + m_bikeState.Anchors().PLLp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerLLegMass,m_bikeState.Parameters().BPm,0.4);
    dBodySetMass(m_PlayerLLegBodyID,&m_PlayerLLegMass);
    dBodySetFiniteRotationMode(m_PlayerLLegBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerLLegBodyID,0,0,1);

    dBodySetPosition(m_PlayerULegBodyID,StartPos.x + m_bikeState.Anchors().PULp.x,StartPos.y + m_bikeState.Anchors().PULp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerULegMass,m_bikeState.Parameters().BPm,0.4);
    dBodySetMass(m_PlayerULegBodyID,&m_PlayerULegMass);
    dBodySetFiniteRotationMode(m_PlayerULegBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerULegBodyID,0,0,1);

    dBodySetPosition(m_PlayerLArmBodyID,StartPos.x + m_bikeState.Anchors().PLAp.x,StartPos.y + m_bikeState.Anchors().PLAp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerLArmMass,m_bikeState.Parameters().BPm,0.4);
    dBodySetMass(m_PlayerLArmBodyID,&m_PlayerLArmMass);
    dBodySetFiniteRotationMode(m_PlayerLArmBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerLArmBodyID,0,0,1);

    dBodySetPosition(m_PlayerUArmBodyID,StartPos.x + m_bikeState.Anchors().PUAp.x,StartPos.y + m_bikeState.Anchors().PUAp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerUArmMass,m_bikeState.Parameters().BPm,0.4);
    dBodySetMass(m_PlayerUArmBodyID,&m_PlayerUArmMass);
    dBodySetFiniteRotationMode(m_PlayerUArmBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerUArmBodyID,0,0,1);

    dBodySetPosition(m_PlayerFootAnchorBodyID,StartPos.x + m_bikeState.Anchors().PFp.x,StartPos.y + m_bikeState.Anchors().PFp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerFootAnchorMass,m_bikeState.Parameters().BPm,0.4);
    dBodySetMass(m_PlayerFootAnchorBodyID,&m_PlayerFootAnchorMass);
    dBodySetFiniteRotationMode(m_PlayerFootAnchorBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerFootAnchorBodyID,0,0,1);
    
    dBodySetPosition(m_PlayerHandAnchorBodyID,StartPos.x + m_bikeState.Anchors().PHp.x,StartPos.y + m_bikeState.Anchors().PHp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerHandAnchorMass,m_bikeState.Parameters().BPm,0.4);
    dBodySetMass(m_PlayerHandAnchorBodyID,&m_PlayerHandAnchorMass);
    dBodySetFiniteRotationMode(m_PlayerHandAnchorBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerHandAnchorBodyID,0,0,1);
    
    /* Place and define the player bodies (Alt.) */                
    dBodySetPosition(m_PlayerTorsoBodyID2,StartPos.x + m_bikeState.Anchors().PTp2.x,StartPos.y + m_bikeState.Anchors().PTp2.y,0.0f);
    dBodySetMass(m_PlayerTorsoBodyID2,&m_PlayerTorsoMass);
    dBodySetFiniteRotationMode(m_PlayerTorsoBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerTorsoBodyID2,0,0,1);

    dBodySetPosition(m_PlayerLLegBodyID2,StartPos.x + m_bikeState.Anchors().PLLp2.x,StartPos.y + m_bikeState.Anchors().PLLp2.y,0.0f);
    dBodySetMass(m_PlayerLLegBodyID2,&m_PlayerLLegMass);
    dBodySetFiniteRotationMode(m_PlayerLLegBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerLLegBodyID2,0,0,1);

    dBodySetPosition(m_PlayerULegBodyID2,StartPos.x + m_bikeState.Anchors().PULp2.x,StartPos.y + m_bikeState.Anchors().PULp2.y,0.0f);
    dBodySetMass(m_PlayerULegBodyID2,&m_PlayerULegMass);
    dBodySetFiniteRotationMode(m_PlayerULegBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerULegBodyID2,0,0,1);

    dBodySetPosition(m_PlayerLArmBodyID2,StartPos.x + m_bikeState.Anchors().PLAp2.x,StartPos.y + m_bikeState.Anchors().PLAp2.y,0.0f);
    dBodySetMass(m_PlayerLArmBodyID2,&m_PlayerLArmMass);
    dBodySetFiniteRotationMode(m_PlayerLArmBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerLArmBodyID2,0,0,1);

    dBodySetPosition(m_PlayerUArmBodyID2,StartPos.x + m_bikeState.Anchors().PUAp2.x,StartPos.y + m_bikeState.Anchors().PUAp2.y,0.0f);
    dBodySetMass(m_PlayerUArmBodyID2,&m_PlayerUArmMass);
    dBodySetFiniteRotationMode(m_PlayerUArmBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerUArmBodyID2,0,0,1);

    dBodySetPosition(m_PlayerFootAnchorBodyID2,StartPos.x + m_bikeState.Anchors().PFp2.x,StartPos.y + m_bikeState.Anchors().PFp2.y,0.0f);
    dBodySetMass(m_PlayerFootAnchorBodyID2,&m_PlayerFootAnchorMass);
    dBodySetFiniteRotationMode(m_PlayerFootAnchorBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerFootAnchorBodyID2,0,0,1);
    
    dBodySetPosition(m_PlayerHandAnchorBodyID2,StartPos.x + m_bikeState.Anchors().PHp2.x,StartPos.y + m_bikeState.Anchors().PHp2.y,0.0f);
    dBodySetMass(m_PlayerHandAnchorBodyID2,&m_PlayerHandAnchorMass);
    dBodySetFiniteRotationMode(m_PlayerHandAnchorBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerHandAnchorBodyID2,0,0,1);
    
    float fERP = 0.3; float fCFM = 0.03f;

    /* Connect em */
    m_KneeHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_KneeHingeID,m_PlayerLLegBodyID,m_PlayerULegBodyID);
    dJointSetHingeAnchor(m_KneeHingeID,StartPos.x + m_bikeState.Parameters().PKVx,StartPos.y + m_bikeState.Parameters().PKVy,0.0f);
    dJointSetHingeAxis(m_KneeHingeID,0,0,1);
    dJointSetHingeParam(m_KneeHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_KneeHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_KneeHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_KneeHingeID,dParamStopCFM,fCFM);

    m_LowerBodyHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_LowerBodyHingeID,m_PlayerULegBodyID,m_PlayerTorsoBodyID);
    dJointSetHingeAnchor(m_LowerBodyHingeID,StartPos.x + m_bikeState.Parameters().PLVx,StartPos.y + m_bikeState.Parameters().PLVy,0.0f);
    dJointSetHingeAxis(m_LowerBodyHingeID,0,0,1);       
    dJointSetHingeParam(m_LowerBodyHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_LowerBodyHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_LowerBodyHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_LowerBodyHingeID,dParamStopCFM,fCFM);

    m_ShoulderHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_ShoulderHingeID,m_PlayerTorsoBodyID,m_PlayerUArmBodyID);
    dJointSetHingeAnchor(m_ShoulderHingeID,StartPos.x + m_bikeState.Parameters().PSVx,StartPos.y + m_bikeState.Parameters().PSVy,0.0f);
    dJointSetHingeAxis(m_ShoulderHingeID,0,0,1);       
    dJointSetHingeParam(m_ShoulderHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_ShoulderHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_ShoulderHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_ShoulderHingeID,dParamStopCFM,fCFM);

    m_ElbowHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_ElbowHingeID,m_PlayerUArmBodyID,m_PlayerLArmBodyID);
    dJointSetHingeAnchor(m_ElbowHingeID,StartPos.x + m_bikeState.Parameters().PEVx,StartPos.y + m_bikeState.Parameters().PEVy,0.0f);
    dJointSetHingeAxis(m_ElbowHingeID,0,0,1);       
    dJointSetHingeParam(m_ElbowHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_ElbowHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_ElbowHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_ElbowHingeID,dParamStopCFM,fCFM);
    
    m_FootHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_FootHingeID,m_PlayerFootAnchorBodyID,m_PlayerLLegBodyID);
    dJointSetHingeAnchor(m_FootHingeID,StartPos.x + m_bikeState.Parameters().PFVx,StartPos.y + m_bikeState.Parameters().PFVy,0.0f);
    dJointSetHingeAxis(m_FootHingeID,0,0,1);       
    dJointSetHingeParam(m_FootHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_FootHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_FootHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_FootHingeID,dParamStopCFM,fCFM);
    
    m_HandHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_HandHingeID,m_PlayerLArmBodyID,m_PlayerHandAnchorBodyID);
    dJointSetHingeAnchor(m_HandHingeID,StartPos.x + m_bikeState.Parameters().PHVx,StartPos.y + m_bikeState.Parameters().PHVy,0.0f);
    dJointSetHingeAxis(m_HandHingeID,0,0,1);       
    dJointSetHingeParam(m_HandHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_HandHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_HandHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_HandHingeID,dParamStopCFM,fCFM);                

    /* Connect em (Alt.) */
    m_KneeHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_KneeHingeID2,m_PlayerLLegBodyID2,m_PlayerULegBodyID2);
    dJointSetHingeAnchor(m_KneeHingeID2,StartPos.x - m_bikeState.Parameters().PKVx,StartPos.y + m_bikeState.Parameters().PKVy,0.0f);
    dJointSetHingeAxis(m_KneeHingeID2,0,0,1);       
    dJointSetHingeParam(m_KneeHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_KneeHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_KneeHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_KneeHingeID2,dParamStopCFM,fCFM);

    m_LowerBodyHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_LowerBodyHingeID2,m_PlayerULegBodyID2,m_PlayerTorsoBodyID2);
    dJointSetHingeAnchor(m_LowerBodyHingeID2,StartPos.x - m_bikeState.Parameters().PLVx,StartPos.y + m_bikeState.Parameters().PLVy,0.0f);
    dJointSetHingeAxis(m_LowerBodyHingeID2,0,0,1);       
    dJointSetHingeParam(m_LowerBodyHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_LowerBodyHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_LowerBodyHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_LowerBodyHingeID2,dParamStopCFM,fCFM);

    m_ShoulderHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_ShoulderHingeID2,m_PlayerTorsoBodyID2,m_PlayerUArmBodyID2);
    dJointSetHingeAnchor(m_ShoulderHingeID2,StartPos.x - m_bikeState.Parameters().PSVx,StartPos.y + m_bikeState.Parameters().PSVy,0.0f);
    dJointSetHingeAxis(m_ShoulderHingeID2,0,0,1);       
    dJointSetHingeParam(m_ShoulderHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_ShoulderHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_ShoulderHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_ShoulderHingeID2,dParamStopCFM,fCFM);

    m_ElbowHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_ElbowHingeID2,m_PlayerUArmBodyID2,m_PlayerLArmBodyID2);
    dJointSetHingeAnchor(m_ElbowHingeID2,StartPos.x - m_bikeState.Parameters().PEVx,StartPos.y + m_bikeState.Parameters().PEVy,0.0f);
    dJointSetHingeAxis(m_ElbowHingeID2,0,0,1);       
    dJointSetHingeParam(m_ElbowHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_ElbowHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_ElbowHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_ElbowHingeID2,dParamStopCFM,fCFM);
    
    m_FootHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_FootHingeID2,m_PlayerFootAnchorBodyID2,m_PlayerLLegBodyID2);
    dJointSetHingeAnchor(m_FootHingeID2,StartPos.x - m_bikeState.Parameters().PFVx,StartPos.y + m_bikeState.Parameters().PFVy,0.0f);
    dJointSetHingeAxis(m_FootHingeID2,0,0,1);       
    dJointSetHingeParam(m_FootHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_FootHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_FootHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_FootHingeID2,dParamStopCFM,fCFM);
    
    m_HandHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_HandHingeID2,m_PlayerLArmBodyID2,m_PlayerHandAnchorBodyID2);
    dJointSetHingeAnchor(m_HandHingeID2,StartPos.x - m_bikeState.Parameters().PHVx,StartPos.y + m_bikeState.Parameters().PHVy,0.0f);
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
    dBodySetPosition(m_RearWheelBodyID,StartPos.x + m_bikeState.Anchors().Rp.x,StartPos.y + m_bikeState.Anchors().Rp.y,0.0f);
/*    const dReal *pf;
    pf = dBodyGetAngularVel(m_RearWheelBodyID);
    printf("[%f %f %f]\n",pf[0],pf[1],pf[2]);*/
    dMassSetSphereTotal(&m_RearWheelMass,m_bikeState.Parameters().Wm,m_bikeState.Parameters().WheelRadius());
    dBodySetMass(m_RearWheelBodyID,&m_RearWheelMass);
    dBodySetFiniteRotationMode(m_RearWheelBodyID,1);
    dBodySetFiniteRotationAxis(m_RearWheelBodyID,0,0,1);

    /* Place and define the front wheel */
    dBodySetPosition(m_FrontWheelBodyID,StartPos.x + m_bikeState.Anchors().Fp.x,StartPos.y + m_bikeState.Anchors().Fp.y,0.0f);
    dMassSetSphereTotal(&m_FrontWheelMass,m_bikeState.Parameters().Wm,m_bikeState.Parameters().WheelRadius());
    dBodySetMass(m_FrontWheelBodyID,&m_FrontWheelMass);
    dBodySetFiniteRotationMode(m_FrontWheelBodyID,1);
    dBodySetFiniteRotationAxis(m_FrontWheelBodyID,0,0,1);
    
    /* Place and define the frame */
    dBodySetPosition(m_FrameBodyID,StartPos.x,StartPos.y,0.0f);    
    dMassSetBoxTotal(&m_FrameMass,m_bikeState.Parameters().Fm,m_bikeState.Parameters().IL,m_bikeState.Parameters().IH,DEPTH_FACTOR);
    dBodySetMass(m_FrameBodyID,&m_FrameMass);      
    dBodySetFiniteRotationMode(m_FrameBodyID,1);
    dBodySetFiniteRotationAxis(m_FrameBodyID,0,0,1);
    
    /* Prepare rider */
    prepareRider(StartPos);
  }
  
bool PlayerBiker::isWheelSpinning() {
  return m_bWheelSpin;
}

Vector2f PlayerBiker::getWheelSpinPoint() {
  return m_WheelSpinPoint;
}

Vector2f PlayerBiker::getWheelSpinDir() {
  return m_WheelSpinDir;
}

void PlayerBiker::setBodyDetach(bool state) {
  m_bodyDetach = state;
  
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

void PlayerBiker::resetAutoDisabler() {
  m_nStillFrames = 0;
}

bool PlayerBiker::isSqueeking() {
  return m_bSqueeking;
}

float PlayerBiker::howMuchSqueek() {
  return m_fHowMuchSqueek;
}

void PlayerBiker::setOnBikerHooks(OnBikerHooks* i_bikerHooks) {
  m_bikerHooks = i_bikerHooks;
}

OnBikerHooks* PlayerBiker::getOnBikerHooks() {
  return m_bikerHooks;
}

bool PlayerBiker::getRenderBikeFront() {
  return m_bodyDetach == false;
}

  /*===========================================================================
    Game state interpolation for smoother replays 
    ===========================================================================*/
  void BikeState::interpolateGameState(SerializedBikeState *pA,SerializedBikeState *pB,SerializedBikeState *p,float t) {
    /* First of all inherit everything from A */
    memcpy(p,pA,sizeof(SerializedBikeState));
    
    /* Interpolate away! The frame is the most important... */
    p->fFrameX = pA->fFrameX + (pB->fFrameX - pA->fFrameX)*t;
    p->fFrameY = pA->fFrameY + (pB->fFrameY - pA->fFrameY)*t;
    
    p->fGameTime = pA->fGameTime + (pB->fGameTime - pA->fGameTime)*t;
  }

  void BikeState::updateStateFromReplay(SerializedBikeState *pReplayState,
          BikeState *pBikeS) 
  {
    bool bUpdateRider=true,bUpdateAltRider=true;
    
    /* Replaying... fetch serialized state */
    pBikeS->RearWheelP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cRearWheelX); 
    pBikeS->RearWheelP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cRearWheelY);
    pBikeS->FrontWheelP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cFrontWheelX); 
    pBikeS->FrontWheelP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cFrontWheelY);
    pBikeS->CenterP.x = pReplayState->fFrameX; 
    pBikeS->CenterP.y = pReplayState->fFrameY;
    
    _16BitsToMatrix(pReplayState->nFrameRot,pBikeS->fFrameRot);
    _16BitsToMatrix(pReplayState->nFrontWheelRot,pBikeS->fFrontWheelRot);
    _16BitsToMatrix(pReplayState->nRearWheelRot,pBikeS->fRearWheelRot);
    
    /* Update engine stuff */
    pBikeS->fBikeEngineRPM = ENGINE_MIN_RPM + (ENGINE_MAX_RPM - ENGINE_MIN_RPM) * ((float)pReplayState->cBikeEngineRPM) / 255.0f;
    
    pBikeS->SwingAnchorP.x = pBikeS->Anchors().AR.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().AR.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->SwingAnchorP.y = pBikeS->Anchors().AR.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().AR.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->FrontAnchorP.x = pBikeS->Anchors().AF.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().AF.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->FrontAnchorP.y = pBikeS->Anchors().AF.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().AF.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    
    pBikeS->SwingAnchor2P.x = pBikeS->Anchors().AR2.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().AR2.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->SwingAnchor2P.y = pBikeS->Anchors().AR2.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().AR2.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->FrontAnchor2P.x = pBikeS->Anchors().AF2.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().AF2.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->FrontAnchor2P.y = pBikeS->Anchors().AF2.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().AF2.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    
    /* Calculate desired hand/foot positions */
    pBikeS->WantedFootP.x = pBikeS->Anchors().PFp.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().PFp.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->WantedFootP.y = pBikeS->Anchors().PFp.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().PFp.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->WantedHandP.x = pBikeS->Anchors().PHp.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().PHp.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->WantedHandP.y = pBikeS->Anchors().PHp.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().PHp.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;    
    
    pBikeS->WantedFoot2P.x = pBikeS->Anchors().PFp2.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().PFp2.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->WantedFoot2P.y = pBikeS->Anchors().PFp2.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().PFp2.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->WantedHand2P.x = pBikeS->Anchors().PHp2.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().PHp2.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->WantedHand2P.y = pBikeS->Anchors().PHp2.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().PHp2.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;    
    
    /* Get hinges from serialized state */
    if(pReplayState->cFlags & SER_BIKE_STATE_DIR_RIGHT) {
      pBikeS->HandP = pBikeS->WantedHandP;
      pBikeS->ElbowP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cElbowX);         
      pBikeS->ElbowP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cElbowY);
      pBikeS->ShoulderP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cShoulderX); 
      pBikeS->ShoulderP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cShoulderY);
      pBikeS->LowerBodyP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cLowerBodyX); 
      pBikeS->LowerBodyP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cLowerBodyY);
      pBikeS->KneeP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cKneeX); 
      pBikeS->KneeP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cKneeY);
      pBikeS->FootP = pBikeS->WantedFootP;
      
      pBikeS->Dir = DD_RIGHT;
      
      bUpdateAltRider = false;
    }
    else if(pReplayState->cFlags & SER_BIKE_STATE_DIR_LEFT) {
      pBikeS->Hand2P = pBikeS->WantedHand2P;
      pBikeS->Elbow2P.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cElbowX); 
      pBikeS->Elbow2P.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cElbowY);
      pBikeS->Shoulder2P.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cShoulderX); 
      pBikeS->Shoulder2P.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cShoulderY);
      pBikeS->LowerBody2P.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cLowerBodyX); 
      pBikeS->LowerBody2P.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cLowerBodyY);
      pBikeS->Knee2P.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cKneeX); 
      pBikeS->Knee2P.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cKneeY);
      pBikeS->Foot2P = pBikeS->WantedFoot2P;
      
      pBikeS->Dir = DD_LEFT;
      
      bUpdateRider = false;
    }
    
    Vector2f V;      
    
    if(bUpdateRider) {        
      /* Calculate head position */
      V = (pBikeS->ShoulderP - pBikeS->LowerBodyP);
      V.normalize();
      pBikeS->HeadP = pBikeS->ShoulderP + V*pBikeS->Parameters().fNeckLength;
    }
    
    if(bUpdateAltRider) {
      /* Calculate head position (Alt.) */
      V = (pBikeS->Shoulder2P - pBikeS->LowerBody2P);
      V.normalize();
      pBikeS->Head2P = pBikeS->Shoulder2P + V*pBikeS->Parameters().fNeckLength;
    }
    
    /* Internally we'd like to know the abs. relaxed position of the wheels */
    pBikeS->RFrontWheelP.x = pBikeS->Anchors().Fp.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().Fp.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->RFrontWheelP.y = pBikeS->Anchors().Fp.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().Fp.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->RRearWheelP.x = pBikeS->Anchors().Rp.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().Rp.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->RRearWheelP.y = pBikeS->Anchors().Rp.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().Rp.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;     
  }
