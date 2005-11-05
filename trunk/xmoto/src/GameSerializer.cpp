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
  Serializer
  ===========================================================================*/
  void MotoGame::getSerializedBikeState(SerializedBikeState *pState) {
    /* Get. */
    pState->fGameTime = m_fTime;
    
    if(m_BikeS.Dir == DD_LEFT) pState->cFlags = SER_BIKE_STATE_DIR_LEFT;
    else if(m_BikeS.Dir == DD_RIGHT) pState->cFlags = SER_BIKE_STATE_DIR_RIGHT;
    
    pState->fFrontWheelX = m_BikeS.FrontWheelP.x;
    pState->fFrontWheelY = m_BikeS.FrontWheelP.y;
    pState->fRearWheelX = m_BikeS.RearWheelP.x;
    pState->fRearWheelY = m_BikeS.RearWheelP.y;
    pState->fFrameX = m_BikeS.CenterP.x;
    pState->fFrameY = m_BikeS.CenterP.y;
    
    memcpy(pState->fFrontWheelRot,m_BikeS.fFrontWheelRot,sizeof(float)*4);
    memcpy(pState->fRearWheelRot,m_BikeS.fRearWheelRot,sizeof(float)*4);
    memcpy(pState->fFrameRot,m_BikeS.fFrameRot,sizeof(float)*4);
    
    if(m_BikeS.Dir == DD_RIGHT) {
      pState->fHandX = m_BikeS.HandP.x; pState->fHandY = m_BikeS.HandP.y;
      pState->fElbowX = m_BikeS.ElbowP.x; pState->fElbowY = m_BikeS.ElbowP.y;
      pState->fShoulderX = m_BikeS.ShoulderP.x; pState->fShoulderY = m_BikeS.ShoulderP.y;
      pState->fLowerBodyX = m_BikeS.LowerBodyP.x; pState->fLowerBodyY = m_BikeS.LowerBodyP.y;
      pState->fKneeX = m_BikeS.KneeP.x; pState->fKneeY = m_BikeS.KneeP.y;
      pState->fFootX = m_BikeS.FootP.x; pState->fFootY = m_BikeS.FootP.y;
    }
    else if(m_BikeS.Dir == DD_LEFT) {
      pState->fHandX = m_BikeS.Hand2P.x; pState->fHandY = m_BikeS.Hand2P.y;
      pState->fElbowX = m_BikeS.Elbow2P.x; pState->fElbowY = m_BikeS.Elbow2P.y;
      pState->fShoulderX = m_BikeS.Shoulder2P.x; pState->fShoulderY = m_BikeS.Shoulder2P.y;
      pState->fLowerBodyX = m_BikeS.LowerBody2P.x; pState->fLowerBodyY = m_BikeS.LowerBody2P.y;
      pState->fKneeX = m_BikeS.Knee2P.x; pState->fKneeY = m_BikeS.Knee2P.y;
      pState->fFootX = m_BikeS.Foot2P.x; pState->fFootY = m_BikeS.Foot2P.y;
    }
  }      

};


