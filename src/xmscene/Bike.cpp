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

#include "Bike.h"
#include "../Replay.h"
#include "Scene.h"
#include "../GameEvents.h"
#include "../GameText.h"
#include "../PhysSettings.h"
#include "../Game.h"
#include "BikeParameters.h"
#include "BikeAnchors.h"
#include "BikeController.h"
#include "../Sound.h"
#include "../helpers/Log.h"
#include "PhysicsSettings.h"

#define MINIMUM_VELOCITY_TO_GET_MAXIMUM_DEATH_SOUND 80.0
#define MINIMUM_SOUND_VOLUME 0.2

BikeState::BikeState(PhysicsSettings* i_physicsSettings) {
  m_bikeParameters = new BikeParameters(i_physicsSettings);
  m_bikeAnchors    = new BikeAnchors();

  reInitializeAnchors();
  clear();

  GameTime = 0;
}

BikeState& BikeState::operator=(const BikeState& i_copy) {
  *(this->m_bikeParameters) = *(i_copy.m_bikeParameters);
  *(this->m_bikeAnchors)    = *(i_copy.m_bikeAnchors);

  this->Dir = i_copy.Dir;
  this->fBikeEngineRPM = i_copy.fBikeEngineRPM;
  this->RearWheelP = i_copy.RearWheelP;
  this->FrontWheelP = i_copy.FrontWheelP;
  this->SwingAnchorP = i_copy.SwingAnchorP;
  this->FrontAnchorP = i_copy.FrontAnchorP;
  this->SwingAnchor2P = i_copy.SwingAnchor2P;
  this->FrontAnchor2P = i_copy.FrontAnchor2P;
  this->CenterP = i_copy.CenterP;
  this->PlayerTorsoP = i_copy.PlayerTorsoP;
  this->PlayerULegP = i_copy.PlayerULegP;
  this->PlayerLLegP = i_copy.PlayerLLegP;
  this->PlayerUArmP = i_copy.PlayerUArmP;
  this->PlayerLArmP = i_copy.PlayerLArmP;
  this->PlayerTorso2P = i_copy.PlayerTorso2P;
  this->PlayerULeg2P = i_copy.PlayerULeg2P;
  this->PlayerLLeg2P = i_copy.PlayerLLeg2P;
  this->PlayerUArm2P = i_copy.PlayerUArm2P;
  this->PlayerLArm2P = i_copy.PlayerLArm2P;
  this->WantedHandP = i_copy.WantedHandP;
  this->WantedFootP = i_copy.WantedFootP;
  this->WantedHand2P = i_copy.WantedHand2P;
  this->WantedFoot2P = i_copy.WantedFoot2P;    
  this->HandP = i_copy.HandP;
  this->ElbowP = i_copy.ElbowP;
  this->ShoulderP = i_copy.ShoulderP;
  this->LowerBodyP = i_copy.LowerBodyP;
  this->KneeP = i_copy.KneeP;
  this->FootP = i_copy.FootP;
  this->HeadP = i_copy.HeadP;
  this->Hand2P = i_copy.Hand2P;
  this->Elbow2P = i_copy.Elbow2P;
  this->Shoulder2P = i_copy.Shoulder2P;
  this->LowerBody2P = i_copy.LowerBody2P;
  this->Knee2P = i_copy.Knee2P;
  this->Foot2P = i_copy.Foot2P;
  this->Head2P = i_copy.Head2P;
  this->RRearWheelP = i_copy.RRearWheelP;
  this->RFrontWheelP = i_copy.RFrontWheelP;
  this->PrevRq = i_copy.PrevRq;
  this->PrevFq = i_copy.PrevFq;
  this->PrevPFq = i_copy.PrevPFq;
  this->PrevPHq = i_copy.PrevPHq;
  this->PrevPFq2 = i_copy.PrevPFq2;
  this->PrevPHq2 = i_copy.PrevPHq2;
  
  this->GameTime = i_copy.GameTime;

  for(unsigned int i=0; i<4; i++) {
    fFrontWheelRot[i] = i_copy.fFrontWheelRot[i];
    fRearWheelRot[i]  = i_copy.fRearWheelRot[i];
    fFrameRot[i]      = i_copy.fFrameRot[i];
  }

  return *this;
}

BikeState::~BikeState() {
  delete m_bikeParameters;
  delete m_bikeAnchors;
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


void BikeState::reInitializeAnchors() {
  m_bikeAnchors->update(m_bikeParameters);
}

void BikeState::clear() {
  CenterP = Vector2f(0,0);
  Dir = DD_RIGHT;
  fBikeEngineRPM = 0.0f;
  Elbow2P = Vector2f(0,0);
  ElbowP = Vector2f(0,0);
  Foot2P = Vector2f(0,0);
  FootP = Vector2f(0,0);
  FrontAnchor2P = Vector2f(0,0);
  FrontAnchorP = Vector2f(0,0);
  FrontWheelP = Vector2f(0,0);
  Hand2P = Vector2f(0,0);
  HandP = Vector2f(0,0);
  Head2P = Vector2f(0,0);
  HeadP = Vector2f(0,0);
  Knee2P = Vector2f(0,0);
  KneeP = Vector2f(0,0);
  LowerBody2P = Vector2f(0,0);
  LowerBodyP = Vector2f(0,0);
  PlayerLArmP = Vector2f(0,0);
  PlayerLLegP = Vector2f(0,0);
  PlayerTorsoP = Vector2f(0,0);
  PlayerUArmP = Vector2f(0,0);
  PlayerULegP = Vector2f(0,0);
  PlayerLArm2P = Vector2f(0,0);
  PlayerLLeg2P = Vector2f(0,0);
  PlayerTorso2P = Vector2f(0,0);
  PlayerUArm2P = Vector2f(0,0);
  PlayerULeg2P = Vector2f(0,0);
  PrevFq = Vector2f(0,0);
  PrevRq = Vector2f(0,0);
  PrevPFq = Vector2f(0,0);
  PrevPHq = Vector2f(0,0);
  PrevPFq2 = Vector2f(0,0);
  PrevPHq2 = Vector2f(0,0);
  RearWheelP = Vector2f(0,0);
  RFrontWheelP = Vector2f(0,0);
  RRearWheelP = Vector2f(0,0);
  Shoulder2P = Vector2f(0,0);
  ShoulderP = Vector2f(0,0);
  SwingAnchor2P = Vector2f(0,0);
  SwingAnchorP = Vector2f(0,0);
}

BikeAnchors* BikeState::Anchors() {
  return m_bikeAnchors;
}

BikeParameters* BikeState::Parameters() {
  return m_bikeParameters;
}

Biker::Biker(PhysicsSettings* i_physicsSettings,
	     bool i_engineSound,
	     Theme *i_theme, BikerTheme* i_bikerTheme,
	     const TColor& i_colorFilter,
	     const TColor& i_uglyColorFilter) {

  m_EngineSound = NULL;
  if(i_engineSound) {
    m_EngineSound = new EngineSoundSimulator();
  }

  m_physicsSettings = i_physicsSettings;
  m_bikeState   = new BikeState(m_physicsSettings);
  m_localNetId = -1;
  m_nbRenderedFrames = 0;

  /* sound engine */
  if(m_EngineSound != NULL) {
    try {
      m_EngineSound->addBangSample(Sound::findSample(i_theme->getSound("Engine00")->FilePath()));
      m_EngineSound->addBangSample(Sound::findSample(i_theme->getSound("Engine01")->FilePath()));
      m_EngineSound->addBangSample(Sound::findSample(i_theme->getSound("Engine02")->FilePath()));
      m_EngineSound->addBangSample(Sound::findSample(i_theme->getSound("Engine03")->FilePath()));
      m_EngineSound->addBangSample(Sound::findSample(i_theme->getSound("Engine04")->FilePath()));
      m_EngineSound->addBangSample(Sound::findSample(i_theme->getSound("Engine05")->FilePath()));
      m_EngineSound->addBangSample(Sound::findSample(i_theme->getSound("Engine06")->FilePath()));
      m_EngineSound->addBangSample(Sound::findSample(i_theme->getSound("Engine07")->FilePath()));
      m_EngineSound->addBangSample(Sound::findSample(i_theme->getSound("Engine08")->FilePath()));
      m_EngineSound->addBangSample(Sound::findSample(i_theme->getSound("Engine09")->FilePath()));
      m_EngineSound->addBangSample(Sound::findSample(i_theme->getSound("Engine10")->FilePath()));
      m_EngineSound->addBangSample(Sound::findSample(i_theme->getSound("Engine11")->FilePath()));
      m_EngineSound->addBangSample(Sound::findSample(i_theme->getSound("Engine12")->FilePath()));
    } catch(Exception &e) {
      /* hum, no nice */
    }
  }

  m_dead      = false;
  m_deadTime  = 0;
  m_finished  = false;
  m_finishTime = 0;
  m_bikerTheme = i_bikerTheme;
  m_bWheelSpin = false;
  m_bikerHooks = NULL;
  m_colorFilter = i_colorFilter;
  m_uglyColorFilter = i_uglyColorFilter;
  m_doInterpolation = true;
  m_bodyDetach      = false;
  m_wheelDetach     =  false;
  m_changeDirPer      = 1.0;
}

Biker::~Biker() { 
  cleanCollisionPoints();
  if(m_EngineSound != NULL) {
    delete m_EngineSound;
  }
  delete m_bikeState;
}

void Biker::setLocalNetId(int i_value) {
  m_localNetId = i_value;
}

const TColor& Biker::getColorFilter() const {
  return m_colorFilter;
}

const TColor& Biker::getUglyColorFilter() const {
  return m_uglyColorFilter;
}

int Biker::finishTime() const {
  return m_finishTime;
}

int Biker::deadTime() const {
  return m_deadTime;
}

int Biker::localNetId() const {
  return m_localNetId;
}

float Biker::getBikeEngineRPM() {
  return m_bikeState->fBikeEngineRPM;
}

PhysicsSettings* Biker::getPhysicsSettings() {
  return m_physicsSettings;
}

void Biker::updateToTime(int i_time, int i_timeStep,
			     CollisionSystem *v_collisionSystem, Vector2f i_gravity,
			     Scene *i_motogame) {
  /* update direction - even if the biker is dead of finished */
  if(m_changeDirPer < 1.0) {
    m_changeDirPer += ((float)i_timeStep)*3.0 / 100.0;
    if(m_changeDirPer > 1.0) {
      m_changeDirPer = 1.0;
    }
  }

  if(isFinished() || isDead()) return;

  /* sound */
  if(m_EngineSound != NULL) {
    m_EngineSound->setRPM(getBikeEngineRPM());
    m_EngineSound->update(i_time);
  }

}

bool Biker::isTouching(const Entity* i_entity) const {
  for(unsigned int i=0; i<m_entitiesTouching.size(); i++) {
    if(m_entitiesTouching[i] == i_entity) {
      return true;
    }
  }
  return false;
}

Biker::touch Biker::setTouching(Entity* i_entity, bool i_touching) {
  bool v_wasTouching = isTouching(i_entity);
  if(v_wasTouching == i_touching) {
    return none;
  }
  
  if(i_touching) {
    m_entitiesTouching.push_back(i_entity);
    return added;
  } else {
    for(unsigned int i=0; i<m_entitiesTouching.size(); i++) {
      if(m_entitiesTouching[i] == i_entity) {
	m_entitiesTouching.erase(m_entitiesTouching.begin() + i);
	return removed;
      }
    }
  }
  return none;
}

bool Biker::isTouching(const Zone* i_zone) const {
  for(unsigned int i=0; i<m_zonesTouching.size(); i++) {
    if(m_zonesTouching[i]->Id() == i_zone->Id()) {
      return true;
    }
  }
  return false;
}

Biker::touch Biker::setTouching(Zone* i_zone, bool i_isTouching) {
  bool v_wasTouching = isTouching(i_zone);
  if(v_wasTouching == i_isTouching) {
    return none;
  }
    
  if(i_isTouching) {
    m_zonesTouching.push_back(i_zone);
    return added;
  } else {
    for(unsigned int i=0; i<m_zonesTouching.size(); i++) {
      if(m_zonesTouching[i] == i_zone) {
	m_zonesTouching.erase(m_zonesTouching.begin() + i);
	return removed;
      }
    }
  }
  return none;
}

std::vector<Entity *>& Biker::EntitiesTouching() {
  return m_entitiesTouching;
}

std::vector<Zone *>& Biker::ZonesTouching() {
  return m_zonesTouching;
}

void Biker::setBodyDetach(bool state) {
  resetAutoDisabler();
  m_bodyDetach = state;
}

void Biker::setOnBikerHooks(OnBikerHooks* i_bikerHooks) {
  m_bikerHooks = i_bikerHooks;
}

OnBikerHooks* Biker::getOnBikerHooks() {
  return m_bikerHooks;
}

bool Biker::isWheelSpinning() {
  return m_bWheelSpin;
}

Vector2f Biker::getWheelSpinPoint() {
  return m_WheelSpinPoint;
}

Vector2f Biker::getWheelSpinDir() {
  return m_WheelSpinDir;
}

void Biker::initWheelDetach() {
  m_wheelDetach = false;
}

void Biker::initToPosition(Vector2f i_position, DriveDir i_direction, Vector2f i_gravity) {
}

void Biker::resetAutoDisabler() {
}

void Biker::setFinished(bool i_value, int i_finishTime) {
  m_finished = i_value;
  m_finishTime = i_finishTime;
}

void Biker::setDead(bool i_value, int i_deadTime) {
  m_dead = i_value;
  m_deadTime = i_deadTime;
 
  if(m_dead == false) {
    return;
  }
 
  /* Play the DIE!!! sound */
  try {
    float v_deathVolume;
    float v_maxVelocity = 0.0;

    if(this->getBikeLinearVel() > v_maxVelocity) {
      v_maxVelocity = this->getBikeLinearVel();
    }
    // make deathVolume dependant of the velocity of the fastest of the players
    v_deathVolume = v_maxVelocity / MINIMUM_VELOCITY_TO_GET_MAXIMUM_DEATH_SOUND + MINIMUM_SOUND_VOLUME;
    if(v_deathVolume > 1.0) {
      v_deathVolume = 1.0;
    }
    Sound::playSampleByName(Theme::instance()->getSound("Headcrash")->FilePath(), v_deathVolume);
  } 
  catch(Exception &e) {
  }

}

bool Biker::isFinished() const {
  return m_finished;
}

bool Biker::isDead() const {
  return m_dead;
}

void Biker::setInterpolation(bool bValue) {
  m_doInterpolation = bValue;
}

BikerTheme* Biker::getBikeTheme() {
  return m_bikerTheme;
}

  /*===========================================================================
    Game state interpolation for smoother replays 
    ===========================================================================*/
void BikeState::interpolateGameStateLinear(std::vector<BikeState*> &i_ghostBikeStates, BikeState *p,float t) {
  BikeState *pA, *pB;
  pA = i_ghostBikeStates[i_ghostBikeStates.size()/2-1];
  pB = i_ghostBikeStates[i_ghostBikeStates.size()/2];
  
  /* First of all inherit everything from A */
  *p = *pA;
  
  if(pA->Dir != pB->Dir) {
    return;
  }
  
  p->CenterP = pA->CenterP + (pB->CenterP - pA->CenterP)*t;
  p->RearWheelP = pA->RearWheelP + (pB->RearWheelP - pA->RearWheelP)*t;
  p->FrontWheelP = pA->FrontWheelP + (pB->FrontWheelP - pA->FrontWheelP)*t;

  if(t <= 1.0) { // don't extrapolate the sound
    p->fBikeEngineRPM = pA->fBikeEngineRPM + (pB->fBikeEngineRPM - pA->fBikeEngineRPM)*t;
  }  

  for(unsigned int i=0; i<4; i++) {
    p->fFrameRot[i] = pA->fFrameRot[i] + (pB->fFrameRot[i] - pA->fFrameRot[i])*t;

    // base on last frame : on netClient, if based on pA, it won't rotate, don't really know why
    p->fFrontWheelRot[i] = pB->fFrontWheelRot[i];
    p->fRearWheelRot[i]  = pB->fRearWheelRot[i];
  }
  
  if(pA->Dir == DD_RIGHT) {
    p->HeadP = pA->HeadP + (pB->HeadP - pA->HeadP)*t;
    p->HandP = pA->HandP + (pB->HandP - pA->HandP)*t;
    p->ElbowP = pA->ElbowP + (pB->ElbowP - pA->ElbowP)*t;
    p->ShoulderP = pA->ShoulderP + (pB->ShoulderP - pA->ShoulderP)*t;
    p->LowerBodyP = pA->LowerBodyP + (pB->LowerBodyP - pA->LowerBodyP)*t;
    p->KneeP = pA->KneeP + (pB->KneeP - pA->KneeP)*t;
    p->FootP = pA->FootP + (pB->FootP - pA->FootP)*t;
    p->SwingAnchorP = pA->SwingAnchorP + (pB->SwingAnchorP - pA->SwingAnchorP)*t;
    p->FrontAnchorP = pA->FrontAnchorP + (pB->FrontAnchorP - pA->FrontAnchorP)*t;
    
  } else {
    p->Head2P = pA->Head2P + (pB->Head2P - pA->Head2P)*t;
    p->Hand2P = pA->Hand2P + (pB->Hand2P - pA->Hand2P)*t;
    p->Elbow2P = pA->Elbow2P + (pB->Elbow2P - pA->Elbow2P)*t;
    p->Shoulder2P = pA->Shoulder2P + (pB->Shoulder2P - pA->Shoulder2P)*t;
    p->LowerBody2P = pA->LowerBody2P + (pB->LowerBody2P - pA->LowerBody2P)*t;
    p->Knee2P = pA->Knee2P + (pB->Knee2P - pA->Knee2P)*t;
    p->Foot2P = pA->Foot2P + (pB->Foot2P - pA->Foot2P)*t;
    p->SwingAnchor2P = pA->SwingAnchor2P + (pB->SwingAnchor2P - pA->SwingAnchor2P)*t;
    p->FrontAnchor2P = pA->FrontAnchor2P + (pB->FrontAnchor2P - pA->FrontAnchor2P)*t;
  }
  
  p->GameTime = pA->GameTime + (int)(((float)(pB->GameTime - pA->GameTime))*t);
}

void BikeState::interpolateGameStateCubic(std::vector<BikeState*> &i_ghostBikeStates, BikeState *p,float t) {
  BikeState *pA, *pB, *pC, *pD;

  pA = i_ghostBikeStates[i_ghostBikeStates.size()/2-1-1];
  pB = i_ghostBikeStates[i_ghostBikeStates.size()/2-1];
  pC = i_ghostBikeStates[i_ghostBikeStates.size()/2];
  pD = i_ghostBikeStates[i_ghostBikeStates.size()/2+1];

  /* First of all inherit everything from A */
  *p = *pB;

  if(pA->Dir != pB->Dir || pB->Dir != pC->Dir || pC->Dir != pD->Dir) {
    return;
  }

  p->CenterP = interpolation_cubic(pA->CenterP, pB->CenterP, pC->CenterP, pD->CenterP, t);
  p->RearWheelP = interpolation_cubic(pA->RearWheelP, pB->RearWheelP, pC->RearWheelP, pD->RearWheelP, t);
  p->FrontWheelP = interpolation_cubic(pA->FrontWheelP, pB->FrontWheelP, pC->FrontWheelP, pD->FrontWheelP, t);
  p->FrontWheelP = interpolation_cubic(pA->FrontWheelP, pB->FrontWheelP, pC->FrontWheelP, pD->FrontWheelP, t);
  p->fBikeEngineRPM = interpolation_cubic(pA->fBikeEngineRPM, pB->fBikeEngineRPM, pC->fBikeEngineRPM, pD->fBikeEngineRPM, t);

  for(unsigned int i=0; i<4; i++) {
    //p->fFrontWheelRot[i]
    //p->fRearWheelRot[i]
    p->fFrameRot[i] = interpolation_cubic(pA->fFrameRot[i], pB->fFrameRot[i], pC->fFrameRot[i], pD->fFrameRot[i], t);
  }
  
  if(pB->Dir == DD_RIGHT) {
    p->HeadP = interpolation_cubic(pA->HeadP, pB->HeadP, pC->HeadP, pD->HeadP, t);
    p->HandP = interpolation_cubic(pA->HandP, pB->HandP, pC->HandP, pD->HandP, t);
    p->ElbowP = interpolation_cubic(pA->ElbowP, pB->ElbowP, pC->ElbowP, pD->ElbowP, t);
    p->ShoulderP = interpolation_cubic(pA->ShoulderP, pB->ShoulderP, pC->ShoulderP, pD->ShoulderP, t);
    p->LowerBodyP = interpolation_cubic(pA->LowerBodyP, pB->LowerBodyP, pC->LowerBodyP, pD->LowerBodyP, t);
    p->KneeP = interpolation_cubic(pA->KneeP, pB->KneeP, pC->KneeP, pD->KneeP, t);
    p->FootP = interpolation_cubic(pA->FootP, pB->FootP, pC->FootP, pD->FootP, t);
    p->SwingAnchorP = interpolation_cubic(pA->SwingAnchorP, pB->SwingAnchorP, pC->SwingAnchorP, pD->SwingAnchorP, t);
    p->FrontAnchorP = interpolation_cubic(pA->FrontAnchorP, pB->FrontAnchorP, pC->FrontAnchorP, pD->FrontAnchorP, t);
  } else {
    p->Head2P = interpolation_cubic(pA->Head2P, pB->Head2P, pC->Head2P, pD->Head2P, t);
    p->Hand2P = interpolation_cubic(pA->Hand2P, pB->Hand2P, pC->Hand2P, pD->Hand2P, t);
    p->Elbow2P = interpolation_cubic(pA->Elbow2P, pB->Elbow2P, pC->Elbow2P, pD->Elbow2P, t);
    p->Shoulder2P = interpolation_cubic(pA->Shoulder2P, pB->Shoulder2P, pC->Shoulder2P, pD->Shoulder2P, t);
    p->LowerBody2P = interpolation_cubic(pA->LowerBody2P, pB->LowerBody2P, pC->LowerBody2P, pD->LowerBody2P, t);
    p->Knee2P = interpolation_cubic(pA->Knee2P, pB->Knee2P, pC->Knee2P, pD->Knee2P, t);
    p->Foot2P = interpolation_cubic(pA->Foot2P, pB->Foot2P, pC->Foot2P, pD->Foot2P, t);
    p->SwingAnchor2P = interpolation_cubic(pA->SwingAnchor2P, pB->SwingAnchor2P, pC->SwingAnchor2P, pD->SwingAnchor2P, t);
    p->FrontAnchor2P = interpolation_cubic(pA->FrontAnchor2P, pB->FrontAnchor2P, pC->FrontAnchor2P, pD->FrontAnchor2P, t);
  }

  p->GameTime = pB->GameTime + (int)(((float)(pC->GameTime - pB->GameTime))*t);
}

void BikeState::interpolateGameState(std::vector<BikeState*> &i_ghostBikeStates, BikeState *p, float t) {
  BikeState *pA, *pB, *pC, *pD;

  pA = i_ghostBikeStates[i_ghostBikeStates.size()/2-1-1];
  pB = i_ghostBikeStates[i_ghostBikeStates.size()/2-1];
  pC = i_ghostBikeStates[i_ghostBikeStates.size()/2];
  pD = i_ghostBikeStates[i_ghostBikeStates.size()/2+1];

  if(pA->Dir == pB->Dir && pB->Dir == pC->Dir && pC->Dir == pD->Dir) {
    interpolateGameStateCubic(i_ghostBikeStates, p, t);
  } else {
    interpolateGameStateLinear(i_ghostBikeStates, p, t);
  }
}

  void BikeState::convertStateFromReplay(SerializedBikeState *pReplayState,
					 BikeState *pBikeS, PhysicsSettings* i_physicsSettings) 
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
    pBikeS->fBikeEngineRPM = i_physicsSettings->EngineRpmMin() + (i_physicsSettings->EngineRpmMax()-i_physicsSettings->EngineRpmMin()) * ((float)pReplayState->cBikeEngineRPM) / 255.0f;
    
    pBikeS->SwingAnchorP.x = pBikeS->Anchors()->AR.x*pBikeS->fFrameRot[0] + pBikeS->Anchors()->AR.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->SwingAnchorP.y = pBikeS->Anchors()->AR.x*pBikeS->fFrameRot[2] + pBikeS->Anchors()->AR.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->FrontAnchorP.x = pBikeS->Anchors()->AF.x*pBikeS->fFrameRot[0] + pBikeS->Anchors()->AF.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->FrontAnchorP.y = pBikeS->Anchors()->AF.x*pBikeS->fFrameRot[2] + pBikeS->Anchors()->AF.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    
    pBikeS->SwingAnchor2P.x = pBikeS->Anchors()->AR2.x*pBikeS->fFrameRot[0] + pBikeS->Anchors()->AR2.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->SwingAnchor2P.y = pBikeS->Anchors()->AR2.x*pBikeS->fFrameRot[2] + pBikeS->Anchors()->AR2.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->FrontAnchor2P.x = pBikeS->Anchors()->AF2.x*pBikeS->fFrameRot[0] + pBikeS->Anchors()->AF2.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->FrontAnchor2P.y = pBikeS->Anchors()->AF2.x*pBikeS->fFrameRot[2] + pBikeS->Anchors()->AF2.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    
    /* Calculate desired hand/foot positions */
    pBikeS->WantedFootP.x = pBikeS->Anchors()->PFp.x*pBikeS->fFrameRot[0] + pBikeS->Anchors()->PFp.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->WantedFootP.y = pBikeS->Anchors()->PFp.x*pBikeS->fFrameRot[2] + pBikeS->Anchors()->PFp.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->WantedHandP.x = pBikeS->Anchors()->PHp.x*pBikeS->fFrameRot[0] + pBikeS->Anchors()->PHp.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->WantedHandP.y = pBikeS->Anchors()->PHp.x*pBikeS->fFrameRot[2] + pBikeS->Anchors()->PHp.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;    
    
    pBikeS->WantedFoot2P.x = pBikeS->Anchors()->PFp2.x*pBikeS->fFrameRot[0] + pBikeS->Anchors()->PFp2.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->WantedFoot2P.y = pBikeS->Anchors()->PFp2.x*pBikeS->fFrameRot[2] + pBikeS->Anchors()->PFp2.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->WantedHand2P.x = pBikeS->Anchors()->PHp2.x*pBikeS->fFrameRot[0] + pBikeS->Anchors()->PHp2.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->WantedHand2P.y = pBikeS->Anchors()->PHp2.x*pBikeS->fFrameRot[2] + pBikeS->Anchors()->PHp2.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;    
    
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

      try {
	V.normalize();
	pBikeS->HeadP = pBikeS->ShoulderP + V*pBikeS->Parameters()->fNeckLength;
      } catch(Exception &e) {
	pBikeS->HeadP = pBikeS->ShoulderP;
      }
    }
    
    if(bUpdateAltRider) {
      /* Calculate head position (Alt.) */
      V = (pBikeS->Shoulder2P - pBikeS->LowerBody2P);

      try {
	V.normalize();
	pBikeS->Head2P = pBikeS->Shoulder2P + V*pBikeS->Parameters()->fNeckLength;
      } catch(Exception &e) {
	pBikeS->Head2P = pBikeS->Shoulder2P;
      }
    }
    
    /* Internally we'd like to know the abs. relaxed position of the wheels */
    pBikeS->RFrontWheelP.x = pBikeS->Anchors()->Fp.x*pBikeS->fFrameRot[0] + pBikeS->Anchors()->Fp.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->RFrontWheelP.y = pBikeS->Anchors()->Fp.x*pBikeS->fFrameRot[2] + pBikeS->Anchors()->Fp.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->RRearWheelP.x = pBikeS->Anchors()->Rp.x*pBikeS->fFrameRot[0] + pBikeS->Anchors()->Rp.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->RRearWheelP.y = pBikeS->Anchors()->Rp.x*pBikeS->fFrameRot[2] + pBikeS->Anchors()->Rp.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;

    // time
    pBikeS->GameTime = GameApp::floatToTime(pReplayState->fGameTime);
  }

void Biker::addBodyForce(int i_time, const Vector2f& i_force, int i_startTime, int i_endTime) {
}

void Biker::cleanCollisionPoints() {
	m_collisionPoints.clear();
}

std::vector<Vector2f> &Biker::CollisionPoints() {
	return m_collisionPoints;
}

float Biker::getRearWheelVelocity() {
  return 0.0;
}

float Biker::getFrontWheelVelocity() {
  return 0.0;
}

float Biker::changeDirPer() const {
  return m_changeDirPer;
}

unsigned int Biker::getNbRenderedFrames() const {
  return m_nbRenderedFrames;
}

void Biker::addNbRenderedFrames() {
  m_nbRenderedFrames++;
}

bool Biker::isStateInitialized() const {
  return true; // by default, consider that the biker is iniialized
}

void Biker::clearStates() {
  m_bikeState->clear();

  if(isDead() == false && isFinished() == false) {
    /* BIKE_C */
    if(getControler() != NULL) {
      getControler()->stopControls();
    }
  }
}

BikeController* Biker::getControler() {
  return NULL;
}
