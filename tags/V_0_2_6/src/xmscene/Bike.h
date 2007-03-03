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

#ifndef __BIKE_H__
#define __BIKE_H__

#include <string>
#include "BasicSceneStructs.h"
#include "BikeAnchors.h"

class BikeState {
  public:
    DriveDir Dir;         /* Driving left or right? */
  
    float fBikeEngineRPM;
  
    Vector2f RearWheelP;  /* Rear wheel position */
    Vector2f FrontWheelP; /* Front wheel position */
    Vector2f SwingAnchorP;/* Swing arm anchor position */
    Vector2f FrontAnchorP;/* Front suspension anchor position */    
    Vector2f SwingAnchor2P;/* Swing arm anchor position (Alt.) */
    Vector2f FrontAnchor2P;/* Front suspension anchor position (Alt.) */    
    Vector2f CenterP;     /* Center position */

    Vector2f PlayerTorsoP;/* Position of player's torso */
    Vector2f PlayerULegP; /* Position of player's upper leg */
    Vector2f PlayerLLegP; /* Position of player's lower leg */
    Vector2f PlayerUArmP; /* Position of player's upper arm */
    Vector2f PlayerLArmP; /* Position of player's upper arm */

    Vector2f PlayerTorso2P;/* Position of player's torso (Alt.) */
    Vector2f PlayerULeg2P; /* Position of player's upper leg (Alt.) */
    Vector2f PlayerLLeg2P; /* Position of player's lower leg (Alt.) */
    Vector2f PlayerUArm2P; /* Position of player's upper arm (Alt.) */ 
    Vector2f PlayerLArm2P; /* Position of player's upper arm (Alt.) */
        
    /* Internals */
    float fFrontWheelRot[4];
    float fRearWheelRot[4];
    float fFrameRot[4];
    
    Vector2f WantedHandP,WantedFootP;
    Vector2f WantedHand2P,WantedFoot2P;    
    
    Vector2f HandP;
    Vector2f ElbowP;
    Vector2f ShoulderP;
    Vector2f LowerBodyP;
    Vector2f KneeP;
    Vector2f FootP;
    Vector2f HeadP;        /* NB! not a phys. body */

    Vector2f Hand2P;
    Vector2f Elbow2P;
    Vector2f Shoulder2P;
    Vector2f LowerBody2P;
    Vector2f Knee2P;
    Vector2f Foot2P;
    Vector2f Head2P;        /* NB! not a phys. body */

    Vector2f RRearWheelP; /* Relaxed rear wheel position */
    Vector2f RFrontWheelP;/* Relaxed front wheel position */
    Vector2f PrevRq;      /* Previous error (rear) */
    Vector2f PrevFq;      /* Previous error (front) */
    Vector2f PrevPFq;     /* Previous error (player foot) */
    Vector2f PrevPHq;     /* Previous error (player hand) */
    Vector2f PrevPFq2;    /* Previous error (player foot) (Alt.) */
    Vector2f PrevPHq2;    /* Previous error (player hand) (Alt.) */
    
    /* Bonusinfo */    
  BikeState();
  ~BikeState();
  
  void reInitializeSpeed();
  void physicalUpdate();
  void reInitializeAnchors();

  float CurrentBrake() const;
  float CurrentEngine() const;
    
  BikeAnchors& Anchors();
  BikeParameters& Parameters();

  private:
  /* Driving */
  float m_curBrake, m_curEngine;    
  BikeParameters m_bikeParameters;
  BikeAnchors    m_bikeAnchors;
};

#endif /* __BIKE_H__ */
