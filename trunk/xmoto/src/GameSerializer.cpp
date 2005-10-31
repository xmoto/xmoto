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
  Serialize game state into buffer
  ===========================================================================*/
  int MotoGame::serializeGameState(char *pcBuf,int nBufSize) {  
    SerializedState *pState = (SerializedState *)pcBuf;
    if(nBufSize < sizeof(SerializedState)) return 0;
  
    /* Save front wheel pos/rot */
    pState->fFrontWheelX = m_BikeS.pfFrontWheelPos[0];
    pState->fFrontWheelY = m_BikeS.pfFrontWheelPos[1];
    pState->fFrontWheelRot[0] = m_BikeS.pfFrontWheelRot[0];
    pState->fFrontWheelRot[1] = m_BikeS.pfFrontWheelRot[1];
    pState->fFrontWheelRot[2] = m_BikeS.pfFrontWheelRot[4];
    pState->fFrontWheelRot[3] = m_BikeS.pfFrontWheelRot[5];
  
    /* Save rear wheel pos/rot */
    pState->fRearWheelX = m_BikeS.pfRearWheelPos[0];
    pState->fRearWheelY = m_BikeS.pfRearWheelPos[1];
    pState->fRearWheelRot[0] = m_BikeS.pfRearWheelRot[0];
    pState->fRearWheelRot[1] = m_BikeS.pfRearWheelRot[1];
    pState->fRearWheelRot[2] = m_BikeS.pfRearWheelRot[4];
    pState->fRearWheelRot[3] = m_BikeS.pfRearWheelRot[5];
  
    /* Save frame pos/rot */
    pState->fFrameX = m_BikeS.pfFramePos[0];
    pState->fFrameY = m_BikeS.pfFramePos[1];
    pState->fFrameRot[0] = m_BikeS.pfFrameRot[0];
    pState->fFrameRot[1] = m_BikeS.pfFrameRot[1];
    pState->fFrameRot[2] = m_BikeS.pfFrameRot[4];
    pState->fFrameRot[3] = m_BikeS.pfFrameRot[5];
    
    return sizeof(SerializedState);
  }

  /*===========================================================================
  Load serialized game state from buffer
  ===========================================================================*/
  void MotoGame::unserializeGameState(const char *pcBuf,int nBufSize) {
    SerializedState *pState = (SerializedState *)pcBuf;
    if(nBufSize < sizeof(SerializedState)) return;
    
    /* ... */
  }

};


