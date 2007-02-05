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

/* 
 *  Game state serialization and magic
 */
#include "PhysSettings.h"
#include "MotoGame.h"

namespace vapp {

  /*===========================================================================
    Game state interpolation for smoother replays 
    ===========================================================================*/
  void MotoGame::interpolateGameState(SerializedBikeState *pA,SerializedBikeState *pB,SerializedBikeState *p,float t) {
    /* First of all inherit everything from A */
    memcpy(p,pA,sizeof(SerializedBikeState));
    
    /* Interpolate away! The frame is the most important... */
    p->fFrameX = pA->fFrameX + (pB->fFrameX - pA->fFrameX)*t;
    p->fFrameY = pA->fFrameY + (pB->fFrameY - pA->fFrameY)*t;
    
    p->fGameTime = pA->fGameTime + (pB->fGameTime - pA->fGameTime)*t;
  }
  
  /*===========================================================================
    Decoding of event stream
    ===========================================================================*/
  void MotoGame::unserializeGameEvents(DBuffer *Buffer, std::vector<RecordedGameEvent *> *v_ReplayEvents) {
    MotoGameEvent *v_event;
    RecordedGameEvent *p;    

    try {
      /* Continue until buffer is empty */
      while((*Buffer).numRemainingBytes() > sizeof(float)) {
    	p = new RecordedGameEvent;
    	p->bPassed = false;
    	p->Event   = MotoGameEvent::getUnserialized(*Buffer);
    	v_ReplayEvents->push_back(p);
      }
    } catch(Exception &e) {
      Log("** Warning ** : unable to unserialize game events !");
    }
  }

  /*===========================================================================
    Encoding of event buffer 
    ===========================================================================*/
  void MotoGame::_SerializeGameEventQueue(DBuffer &Buffer, MotoGameEvent *pEvent) {
    pEvent->serialize(Buffer);
  }

  /*===========================================================================
    Matrix encodings
    ===========================================================================*/
  unsigned short MotoGame::_MatrixTo16Bits(const float *pfMatrix) {
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
  
  void MotoGame::_16BitsToMatrix(unsigned short n16,float *pfMatrix) {
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
  signed char MotoGame::_MapCoordTo8Bits(float fRef,float fMaxDiff,
      float fCoord) {
    int n = (int)((127.0f * (fCoord-fRef))/fMaxDiff);
    if(n<-127) n=-127;
    if(n>127) n=127;
    return (signed char)(n&0xff);
  }
  
  float MotoGame::_Map8BitsToCoord(float fRef,float fMaxDiff,signed char c) {
    return fRef + (((float)c)/127.0f) * fMaxDiff;
  }

  /*===========================================================================
    Serializer
    ===========================================================================*/
  void MotoGame::getSerializedBikeState(SerializedBikeState *pState) {
    /* Get. */
    pState->fGameTime = m_fTime;
    
    if(m_BikeS.Dir == DD_LEFT) pState->cFlags = SER_BIKE_STATE_DIR_LEFT;
    else if(m_BikeS.Dir == DD_RIGHT) pState->cFlags = SER_BIKE_STATE_DIR_RIGHT;
    
    /* Calculate maximum X-axis difference between frame.x and other coords */
    float fMaxX = 0.0f;
    if( fabs(m_BikeS.CenterP.x - m_BikeS.FrontWheelP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.FrontWheelP.x);
    if( fabs(m_BikeS.CenterP.x - m_BikeS.RearWheelP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.RearWheelP.x);
    if(m_BikeS.Dir == DD_RIGHT) {
      if( fabs(m_BikeS.CenterP.x - m_BikeS.ElbowP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.ElbowP.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.ShoulderP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.ShoulderP.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.LowerBodyP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.LowerBodyP.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.KneeP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.KneeP.x);
    }
    else if(m_BikeS.Dir == DD_LEFT) {
      if( fabs(m_BikeS.CenterP.x - m_BikeS.Elbow2P.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.Elbow2P.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.Shoulder2P.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.Shoulder2P.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.LowerBody2P.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.LowerBody2P.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.Knee2P.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.Knee2P.x);
    }
    pState->fMaxXDiff = fMaxX;
    
    /* Calculate maximum Y-axis difference between frame.y and other coords */
    float fMaxY = 0.0f;
    if( fabs(m_BikeS.CenterP.y - m_BikeS.FrontWheelP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.FrontWheelP.y);
    if( fabs(m_BikeS.CenterP.y - m_BikeS.RearWheelP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.RearWheelP.y);
    if(m_BikeS.Dir == DD_RIGHT) {
      if( fabs(m_BikeS.CenterP.y - m_BikeS.ElbowP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.ElbowP.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.ShoulderP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.ShoulderP.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.LowerBodyP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.LowerBodyP.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.KneeP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.KneeP.y);
    }
    else if(m_BikeS.Dir == DD_LEFT) {
      if( fabs(m_BikeS.CenterP.y - m_BikeS.Elbow2P.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.Elbow2P.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.Shoulder2P.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.Shoulder2P.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.LowerBody2P.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.LowerBody2P.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.Knee2P.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.Knee2P.y);
    }
    pState->fMaxYDiff = fMaxY;
    
    /* Update engine stuff */    
    int n = (int)(((m_BikeS.fBikeEngineRPM-ENGINE_MIN_RPM)/ENGINE_MAX_RPM)*255.0f);
    if(n<0) n=0;
    if(n>255) n=255;
    pState->cBikeEngineRPM = n;
        
    /* Calculate serialization */
    pState->fFrameX = m_BikeS.CenterP.x;
    pState->fFrameY = m_BikeS.CenterP.y;
        
    pState->cFrontWheelX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.FrontWheelP.x);
    pState->cFrontWheelY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.FrontWheelP.y);
    pState->cRearWheelX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.RearWheelP.x);
    pState->cRearWheelY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.RearWheelP.y);
    
    //memcpy(pState->fFrontWheelRot,m_BikeS.fFrontWheelRot,sizeof(float)*4);
    //memcpy(pState->fRearWheelRot,m_BikeS.fRearWheelRot,sizeof(float)*4);
    //memcpy(pState->fFrameRot,m_BikeS.fFrameRot,sizeof(float)*4);
    
    pState->nFrontWheelRot = _MatrixTo16Bits(m_BikeS.fFrontWheelRot);
    pState->nRearWheelRot = _MatrixTo16Bits(m_BikeS.fRearWheelRot);
    pState->nFrameRot = _MatrixTo16Bits(m_BikeS.fFrameRot);
    
    //printf("[ %f %f \n"
    //       "  %f %f ]\n",pState->fFrameRot[0],pState->fFrameRot[1],pState->fFrameRot[2],pState->fFrameRot[3]);
           
    //unsigned short test = _MatrixTo16Bits(m_BikeS.fFrameRot);
    //float fTest[4];
    //_16BitsToMatrix(test,fTest);
    //
    //printf(" %f %f      %f %f\n"
    //       " %f %f      %f %f\n\n",
    //       m_BikeS.fFrameRot[0],m_BikeS.fFrameRot[1],      fTest[0],fTest[1],
    //       m_BikeS.fFrameRot[2],m_BikeS.fFrameRot[3],      fTest[2],fTest[3]);             
    
    if(m_BikeS.Dir == DD_RIGHT) {
      pState->cElbowX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.ElbowP.x); 
      pState->cElbowY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.ElbowP.y);
      pState->cShoulderX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.ShoulderP.x); 
      pState->cShoulderY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.ShoulderP.y);
      pState->cLowerBodyX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.LowerBodyP.x); 
      pState->cLowerBodyY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.LowerBodyP.y);
      pState->cKneeX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.KneeP.x); 
      pState->cKneeY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.KneeP.y);
    }
    else if(m_BikeS.Dir == DD_LEFT) {
      pState->cElbowX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.Elbow2P.x); 
      pState->cElbowY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.Elbow2P.y);
      pState->cShoulderX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.Shoulder2P.x); 
      pState->cShoulderY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.Shoulder2P.y);
      pState->cLowerBodyX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.LowerBody2P.x); 
      pState->cLowerBodyY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.LowerBody2P.y);
      pState->cKneeX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.Knee2P.x); 
      pState->cKneeY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.Knee2P.y);
    }
  }      

}


