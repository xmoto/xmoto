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

/* 
 *  Game state serialization and magic
 */
#include "PhysSettings.h"
#include "xmscene/Scene.h"
#include "helpers/Log.h"
#include "GameEvents.h"
#include "DBuffer.h"
#include "Game.h"
#include "xmscene/PhysicsSettings.h"

  /*===========================================================================
    Decoding of event stream
    ===========================================================================*/
  void Scene::unserializeGameEvents(DBuffer *Buffer, std::vector<RecordedGameEvent *> *v_ReplayEvents, bool bDisplayInformation) {
    RecordedGameEvent *p;    

    try {
      /* Continue until buffer is empty */
      while((*Buffer).numRemainingBytes() > 0) {
      p = new RecordedGameEvent;
      p->bPassed = false;
      p->Event   = SceneEvent::getUnserialized(*Buffer, bDisplayInformation);
      v_ReplayEvents->push_back(p);
      }
    } catch(Exception &e) {
      LogWarning("unable to unserialize game events !");
      throw e;
    }
  }

  /*===========================================================================
    Serializer
    ===========================================================================*/
void Scene::getSerializedBikeState(BikeState *i_bikeState, int i_time, SerializedBikeState *pState, PhysicsSettings* i_physicsSettings) {
    /* Get. */
    pState->fGameTime = GameApp::timeToFloat(i_time);
    
    if(i_bikeState->Dir == DD_LEFT) pState->cFlags = SER_BIKE_STATE_DIR_LEFT;
    else if(i_bikeState->Dir == DD_RIGHT) pState->cFlags = SER_BIKE_STATE_DIR_RIGHT;
    
    /* Calculate maximum X-axis difference between frame.x and other coords */
    float fMaxX = 0.0f;
    if( fabs(i_bikeState->CenterP.x - i_bikeState->FrontWheelP.x) > fMaxX) fMaxX = fabs(i_bikeState->CenterP.x - i_bikeState->FrontWheelP.x);
    if( fabs(i_bikeState->CenterP.x - i_bikeState->RearWheelP.x) > fMaxX) fMaxX = fabs(i_bikeState->CenterP.x - i_bikeState->RearWheelP.x);
    if(i_bikeState->Dir == DD_RIGHT) {
      if( fabs(i_bikeState->CenterP.x - i_bikeState->ElbowP.x) > fMaxX) fMaxX = fabs(i_bikeState->CenterP.x - i_bikeState->ElbowP.x);
      if( fabs(i_bikeState->CenterP.x - i_bikeState->ShoulderP.x) > fMaxX) fMaxX = fabs(i_bikeState->CenterP.x - i_bikeState->ShoulderP.x);
      if( fabs(i_bikeState->CenterP.x - i_bikeState->LowerBodyP.x) > fMaxX) fMaxX = fabs(i_bikeState->CenterP.x - i_bikeState->LowerBodyP.x);
      if( fabs(i_bikeState->CenterP.x - i_bikeState->KneeP.x) > fMaxX) fMaxX = fabs(i_bikeState->CenterP.x - i_bikeState->KneeP.x);
    }
    else if(i_bikeState->Dir == DD_LEFT) {
      if( fabs(i_bikeState->CenterP.x - i_bikeState->Elbow2P.x) > fMaxX) fMaxX = fabs(i_bikeState->CenterP.x - i_bikeState->Elbow2P.x);
      if( fabs(i_bikeState->CenterP.x - i_bikeState->Shoulder2P.x) > fMaxX) fMaxX = fabs(i_bikeState->CenterP.x - i_bikeState->Shoulder2P.x);
      if( fabs(i_bikeState->CenterP.x - i_bikeState->LowerBody2P.x) > fMaxX) fMaxX = fabs(i_bikeState->CenterP.x - i_bikeState->LowerBody2P.x);
      if( fabs(i_bikeState->CenterP.x - i_bikeState->Knee2P.x) > fMaxX) fMaxX = fabs(i_bikeState->CenterP.x - i_bikeState->Knee2P.x);
    }
    pState->fMaxXDiff = fMaxX;
    
    /* Calculate maximum Y-axis difference between frame.y and other coords */
    float fMaxY = 0.0f;
    if( fabs(i_bikeState->CenterP.y - i_bikeState->FrontWheelP.y) > fMaxY) fMaxY = fabs(i_bikeState->CenterP.y - i_bikeState->FrontWheelP.y);
    if( fabs(i_bikeState->CenterP.y - i_bikeState->RearWheelP.y) > fMaxY) fMaxY = fabs(i_bikeState->CenterP.y - i_bikeState->RearWheelP.y);
    if(i_bikeState->Dir == DD_RIGHT) {
      if( fabs(i_bikeState->CenterP.y - i_bikeState->ElbowP.y) > fMaxY) fMaxY = fabs(i_bikeState->CenterP.y - i_bikeState->ElbowP.y);
      if( fabs(i_bikeState->CenterP.y - i_bikeState->ShoulderP.y) > fMaxY) fMaxY = fabs(i_bikeState->CenterP.y - i_bikeState->ShoulderP.y);
      if( fabs(i_bikeState->CenterP.y - i_bikeState->LowerBodyP.y) > fMaxY) fMaxY = fabs(i_bikeState->CenterP.y - i_bikeState->LowerBodyP.y);
      if( fabs(i_bikeState->CenterP.y - i_bikeState->KneeP.y) > fMaxY) fMaxY = fabs(i_bikeState->CenterP.y - i_bikeState->KneeP.y);
    }
    else if(i_bikeState->Dir == DD_LEFT) {
      if( fabs(i_bikeState->CenterP.y - i_bikeState->Elbow2P.y) > fMaxY) fMaxY = fabs(i_bikeState->CenterP.y - i_bikeState->Elbow2P.y);
      if( fabs(i_bikeState->CenterP.y - i_bikeState->Shoulder2P.y) > fMaxY) fMaxY = fabs(i_bikeState->CenterP.y - i_bikeState->Shoulder2P.y);
      if( fabs(i_bikeState->CenterP.y - i_bikeState->LowerBody2P.y) > fMaxY) fMaxY = fabs(i_bikeState->CenterP.y - i_bikeState->LowerBody2P.y);
      if( fabs(i_bikeState->CenterP.y - i_bikeState->Knee2P.y) > fMaxY) fMaxY = fabs(i_bikeState->CenterP.y - i_bikeState->Knee2P.y);
    }
    pState->fMaxYDiff = fMaxY;
    
    /* Update engine stuff */    
    int n = (int)(((i_bikeState->fBikeEngineRPM-i_physicsSettings->EngineRpmMin())/i_physicsSettings->EngineRpmMax())*255.0f);
    if(n<0) n=0;
    if(n>255) n=255;
    pState->cBikeEngineRPM = n;
        
    /* Calculate serialization */
    pState->fFrameX = i_bikeState->CenterP.x;
    pState->fFrameY = i_bikeState->CenterP.y;
        
    pState->cFrontWheelX = BikeState::_MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,i_bikeState->FrontWheelP.x);
    pState->cFrontWheelY = BikeState::_MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,i_bikeState->FrontWheelP.y);
    pState->cRearWheelX = BikeState::_MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,i_bikeState->RearWheelP.x);
    pState->cRearWheelY = BikeState::_MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,i_bikeState->RearWheelP.y);
    
    //memcpy(pState->fFrontWheelRot,i_bikeState->fFrontWheelRot,sizeof(float)*4);
    //memcpy(pState->fRearWheelRot,i_bikeState->fRearWheelRot,sizeof(float)*4);
    //memcpy(pState->fFrameRot,i_bikeState->fFrameRot,sizeof(float)*4);
    
    pState->nFrontWheelRot = BikeState::_MatrixTo16Bits(i_bikeState->fFrontWheelRot);
    pState->nRearWheelRot = BikeState::_MatrixTo16Bits(i_bikeState->fRearWheelRot);
    pState->nFrameRot = BikeState::_MatrixTo16Bits(i_bikeState->fFrameRot);
    
    //printf("[ %f %f \n"
    //       "  %f %f ]\n",pState->fFrameRot[0],pState->fFrameRot[1],pState->fFrameRot[2],pState->fFrameRot[3]);
           
    //unsigned short test = _MatrixTo16Bits(i_bikeState->fFrameRot);
    //float fTest[4];
    //_16BitsToMatrix(test,fTest);
    //
    //printf(" %f %f      %f %f\n"
    //       " %f %f      %f %f\n\n",
    //       i_bikeState->fFrameRot[0],i_bikeState->fFrameRot[1],      fTest[0],fTest[1],
    //       i_bikeState->fFrameRot[2],i_bikeState->fFrameRot[3],      fTest[2],fTest[3]);             
    
    if(i_bikeState->Dir == DD_RIGHT) {
      pState->cElbowX = BikeState::_MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,i_bikeState->ElbowP.x); 
      pState->cElbowY = BikeState::_MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,i_bikeState->ElbowP.y);
      pState->cShoulderX = BikeState::_MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,i_bikeState->ShoulderP.x); 
      pState->cShoulderY = BikeState::_MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,i_bikeState->ShoulderP.y);
      pState->cLowerBodyX = BikeState::_MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,i_bikeState->LowerBodyP.x); 
      pState->cLowerBodyY = BikeState::_MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,i_bikeState->LowerBodyP.y);
      pState->cKneeX = BikeState::_MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,i_bikeState->KneeP.x); 
      pState->cKneeY = BikeState::_MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,i_bikeState->KneeP.y);
    }
    else if(i_bikeState->Dir == DD_LEFT) {
      pState->cElbowX = BikeState::_MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,i_bikeState->Elbow2P.x); 
      pState->cElbowY = BikeState::_MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,i_bikeState->Elbow2P.y);
      pState->cShoulderX = BikeState::_MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,i_bikeState->Shoulder2P.x); 
      pState->cShoulderY = BikeState::_MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,i_bikeState->Shoulder2P.y);
      pState->cLowerBodyX = BikeState::_MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,i_bikeState->LowerBody2P.x); 
      pState->cLowerBodyY = BikeState::_MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,i_bikeState->LowerBody2P.y);
      pState->cKneeX = BikeState::_MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,i_bikeState->Knee2P.x); 
      pState->cKneeY = BikeState::_MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,i_bikeState->Knee2P.y);
    }
  }      

