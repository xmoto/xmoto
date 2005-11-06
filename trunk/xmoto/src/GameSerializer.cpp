/*=============================================================================
XMOTO
Copyright (C) 2005 Rasmus Neckelmann (neckelmann@gmail.com)

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
 *  Game state serialization and unserialization.
 */
#include "MotoGame.h"

namespace vapp {

  /*===========================================================================
  Neat trick for converting floating-point numbers to 8 bits
  ===========================================================================*/
  char MotoGame::_MapCoordTo8Bits(float fRef,float fMaxDiff,float fCoord) {
    int n = (int)((127.0f * (fCoord-fRef))/fMaxDiff);
    if(n<-127) n=-127;
    if(n>127) n=127;
    return (char)(n&0xff);
  }
  
  float MotoGame::_Map8BitsToCoord(float fRef,float fMaxDiff,char c) {
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
      if( fabs(m_BikeS.CenterP.x - m_BikeS.HandP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.HandP.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.ElbowP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.ElbowP.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.ShoulderP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.ShoulderP.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.LowerBodyP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.LowerBodyP.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.KneeP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.KneeP.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.FootP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.FootP.x);
    }
    else if(m_BikeS.Dir == DD_LEFT) {
      if( fabs(m_BikeS.CenterP.x - m_BikeS.Hand2P.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.Hand2P.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.Elbow2P.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.Elbow2P.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.Shoulder2P.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.Shoulder2P.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.LowerBody2P.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.LowerBody2P.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.Knee2P.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.Knee2P.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.Foot2P.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.Foot2P.x);
    }
    pState->fMaxXDiff = fMaxX;
    
    /* Calculate maximum Y-axis difference between frame.y and other coords */
    float fMaxY = 0.0f;
    if( fabs(m_BikeS.CenterP.y - m_BikeS.FrontWheelP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.FrontWheelP.y);
    if( fabs(m_BikeS.CenterP.y - m_BikeS.RearWheelP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.RearWheelP.y);
    if(m_BikeS.Dir == DD_RIGHT) {
      if( fabs(m_BikeS.CenterP.y - m_BikeS.HandP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.HandP.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.ElbowP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.ElbowP.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.ShoulderP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.ShoulderP.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.LowerBodyP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.LowerBodyP.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.KneeP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.KneeP.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.FootP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.FootP.y);
    }
    else if(m_BikeS.Dir == DD_LEFT) {
      if( fabs(m_BikeS.CenterP.y - m_BikeS.Hand2P.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.Hand2P.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.Elbow2P.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.Elbow2P.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.Shoulder2P.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.Shoulder2P.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.LowerBody2P.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.LowerBody2P.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.Knee2P.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.Knee2P.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.Foot2P.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.Foot2P.y);
    }
    pState->fMaxYDiff = fMaxY;
        
    /* Calculate serialization */
    pState->fFrameX = m_BikeS.CenterP.x;
    pState->fFrameY = m_BikeS.CenterP.y;
        
    pState->cFrontWheelX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.FrontWheelP.x);
    pState->cFrontWheelY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.FrontWheelP.y);
    pState->cRearWheelX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.RearWheelP.x);
    pState->cRearWheelY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.RearWheelP.y);
    
    memcpy(pState->fFrontWheelRot,m_BikeS.fFrontWheelRot,sizeof(float)*4);
    memcpy(pState->fRearWheelRot,m_BikeS.fRearWheelRot,sizeof(float)*4);
    memcpy(pState->fFrameRot,m_BikeS.fFrameRot,sizeof(float)*4);
    
    if(m_BikeS.Dir == DD_RIGHT) {
      pState->cHandX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.HandP.x); 
      pState->cHandY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.HandP.y);
      pState->cElbowX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.ElbowP.x); 
      pState->cElbowY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.ElbowP.y);
      pState->cShoulderX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.ShoulderP.x); 
      pState->cShoulderY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.ShoulderP.y);
      pState->cLowerBodyX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.LowerBodyP.x); 
      pState->cLowerBodyY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.LowerBodyP.y);
      pState->cKneeX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.KneeP.x); 
      pState->cKneeY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.KneeP.y);
      pState->cFootX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.FootP.x); 
      pState->cFootY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.FootP.y);
    }
    else if(m_BikeS.Dir == DD_LEFT) {
      pState->cHandX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.Hand2P.x); 
      pState->cHandY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.Hand2P.y);
      pState->cElbowX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.Elbow2P.x); 
      pState->cElbowY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.Elbow2P.y);
      pState->cShoulderX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.Shoulder2P.x); 
      pState->cShoulderY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.Shoulder2P.y);
      pState->cLowerBodyX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.LowerBody2P.x); 
      pState->cLowerBodyY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.LowerBody2P.y);
      pState->cKneeX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.Knee2P.x); 
      pState->cKneeY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.Knee2P.y);
      pState->cFootX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.Foot2P.x); 
      pState->cFootY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.Foot2P.y);
    }
  }      

};


