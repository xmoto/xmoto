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
#include "Sound.h"

#define PHYSICAL_ENGINE_REDUCTION 0.05

BikeState::BikeState() {
  m_bikeParameters = new BikeParameters();
  m_bikeAnchors    = new BikeAnchors();

  reInitializeSpeed();
  reInitializeAnchors();
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
    m_curEngine -= m_bikeParameters->MaxEngine() * PHYSICAL_ENGINE_REDUCTION; 
}

void BikeState::reInitializeAnchors() {
  m_bikeAnchors->update(m_bikeParameters);
}

BikeAnchors* BikeState::Anchors() {
  return m_bikeAnchors;
}

BikeParameters* BikeState::Parameters() {
  return m_bikeParameters;
}

BikeState* Biker::getState() {
  return &m_bikeState;
}

Biker::Biker(Theme *i_theme, BikerTheme* i_bikerTheme,
	     const TColor& i_colorFilter,
	     const TColor& i_uglyColorFilter) {
  m_EngineSound = new EngineSoundSimulator();

  /* sound engine */
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

  m_playSound = true;
  m_dead      = false;
  m_deadTime  = 0.0;
  m_finished  = false;
  m_finishTime = 0.0;
  m_bikerTheme = i_bikerTheme;
  m_bWheelSpin = false;
  m_bikerHooks = NULL;
  m_colorFilter = i_colorFilter;
  m_uglyColorFilter = i_uglyColorFilter;
  m_doInterpolation = true;
}

Biker::~Biker() { 
  delete m_EngineSound;
}

const TColor& Biker::getColorFilter() const {
  return m_colorFilter;
}

const TColor& Biker::getUglyColorFilter() const {
  return m_uglyColorFilter;
}

float Biker::finishTime() const {
  return m_finishTime;
}

float Biker::deadTime() const {
  return m_deadTime;
}


float Biker::getBikeEngineRPM() {
  return m_bikeState.fBikeEngineRPM;
}

void Biker::updateToTime(float i_time, float i_timeStep,
			     CollisionSystem *v_collisionSystem, Vector2f i_gravity,
			     MotoGame *i_motogame) {
  if(isFinished() || isDead()) return;

  /* sound */
  m_EngineSound->setRPM(getBikeEngineRPM());
  if(m_playSound) {
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

void Biker::initToPosition(Vector2f i_position, DriveDir i_direction, Vector2f i_gravity) {
}

void Biker::resetAutoDisabler() {
}

BikeController* Biker::getControler() {
  return NULL;
}

void Biker::setPlaySound(bool i_value) {
  m_playSound = i_value;
}

void Biker::setFinished(bool i_value, float i_finishTime) {
  m_finished = i_value;
  m_finishTime = i_finishTime;
}

void Biker::setDead(bool i_value, float i_deadTime) {
  m_dead = i_value;
  m_deadTime = i_deadTime;
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
void BikeState::interpolateGameState(std::vector<SerializedBikeState*> &i_ghostBikeStates ,SerializedBikeState *p,float t) {

  // linear interpolation
  SerializedBikeState *pA, *pB;
  pA = i_ghostBikeStates[i_ghostBikeStates.size()/2-1];
  pB = i_ghostBikeStates[i_ghostBikeStates.size()/2];

  /* First of all inherit everything from A */
  memcpy(p,pA,sizeof(SerializedBikeState));
  
  /* Interpolate away! The frame is the most important... */
  p->fFrameX = pA->fFrameX + (pB->fFrameX - pA->fFrameX)*t;
  p->fFrameY = pA->fFrameY + (pB->fFrameY - pA->fFrameY)*t;
  
  p->fGameTime = pA->fGameTime + (pB->fGameTime - pA->fGameTime)*t;
  //


//  // cubique
//  SerializedBikeState *pA, *pB, *pC, *pD;
//  double t2 = t*t;
//  double x0,x1,x2,x3;
//  double y0,y1,y2,y3;
//
//  pA = i_ghostBikeStates[i_ghostBikeStates.size()/2-1-1];
//  pB = i_ghostBikeStates[i_ghostBikeStates.size()/2-1];
//  pC = i_ghostBikeStates[i_ghostBikeStates.size()/2];
//  pD = i_ghostBikeStates[i_ghostBikeStates.size()/2+1];
//
//  /* First of all inherit everything from A */
//  memcpy(p,pA,sizeof(SerializedBikeState));
//
//  x0 = pD->fFrameX - pC->fFrameX - pA->fFrameX + pB->fFrameX;
//  x1 = pA->fFrameX - pB->fFrameX - x0;
//  x2 = pC->fFrameX - pA->fFrameX;
//  x3 = pB->fFrameX;
//  p->fFrameX = x0*t*t2 + x1*t2 + x2*t + x3;
//
//  y0 = pD->fFrameY - pC->fFrameY - pA->fFrameY + pB->fFrameY;
//  y1 = pA->fFrameY - pB->fFrameY - y0;
//  y2 = pC->fFrameY - pA->fFrameY;
//  y3 = pB->fFrameY;
//  p->fFrameY = y0*t*t2 + y1*t2 + y2*t + y3;
//
//  p->fGameTime = pB->fGameTime + (pC->fGameTime - pB->fGameTime)*t;
//  //

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
      V.normalize();
      pBikeS->HeadP = pBikeS->ShoulderP + V*pBikeS->Parameters()->fNeckLength;
    }
    
    if(bUpdateAltRider) {
      /* Calculate head position (Alt.) */
      V = (pBikeS->Shoulder2P - pBikeS->LowerBody2P);
      V.normalize();
      pBikeS->Head2P = pBikeS->Shoulder2P + V*pBikeS->Parameters()->fNeckLength;
    }
    
    /* Internally we'd like to know the abs. relaxed position of the wheels */
    pBikeS->RFrontWheelP.x = pBikeS->Anchors()->Fp.x*pBikeS->fFrameRot[0] + pBikeS->Anchors()->Fp.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->RFrontWheelP.y = pBikeS->Anchors()->Fp.x*pBikeS->fFrameRot[2] + pBikeS->Anchors()->Fp.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->RRearWheelP.x = pBikeS->Anchors()->Rp.x*pBikeS->fFrameRot[0] + pBikeS->Anchors()->Rp.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->RRearWheelP.y = pBikeS->Anchors()->Rp.x*pBikeS->fFrameRot[2] + pBikeS->Anchors()->Rp.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;     
  }

void Biker::addBodyForce(const Vector2f& i_force) {
  
}
