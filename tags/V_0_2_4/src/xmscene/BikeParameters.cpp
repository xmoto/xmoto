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

#include "BikeParameters.h"
#include "../PhysSettings.h"

BikeParameters::BikeParameters() {
  setDefaults();
}

BikeParameters::~BikeParameters() {
}

void BikeParameters::setDefaults() {
  WR = PHYS_WHEEL_RADIUS;                     
  Ch = PHYS_MASS_ELEVATION;    
  Wb = PHYS_WHEEL_BASE;

  RVx = PHYS_REAR_SUSP_ANCHOR_X;
  RVy = PHYS_REAR_SUSP_ANCHOR_Y;
  FVx = PHYS_FRONT_SUSP_ANCHOR_X;
  FVy = PHYS_FRONT_SUSP_ANCHOR_Y;
    
  PEVx = PHYS_RIDER_ELBOW_X;
  PEVy = PHYS_RIDER_ELBOW_Y;
  PHVx = PHYS_RIDER_HAND_X;
  PHVy = PHYS_RIDER_HAND_Y;
  PSVx = PHYS_RIDER_SHOULDER_X;
  PSVy = PHYS_RIDER_SHOULDER_Y;
  PLVx = PHYS_RIDER_LOWERBODY_X;
  PLVy = PHYS_RIDER_LOWERBODY_Y;
  PKVx = PHYS_RIDER_KNEE_X; 
  PKVy = PHYS_RIDER_KNEE_Y;
  PFVx = PHYS_RIDER_FOOT_X;
  PFVy = PHYS_RIDER_FOOT_Y;
  
  Wm = PHYS_WHEEL_MASS;
  BPm = PHYS_RIDER_BODYPART_MASS;                 
  Fm = PHYS_FRAME_MASS;
  IL = PHYS_INERTIAL_LENGTH;
  IH = PHYS_INERTIAL_HEIGHT;
  
  fMaxBrake =  PHYS_MAX_BRAKING;
  fMaxEngine = PHYS_MAX_ENGINE;
  
  fHeadSize = PHYS_RIDER_HEAD_SIZE;
  fNeckLength = PHYS_RIDER_NECK_LENGTH;
}

float BikeParameters::WheelRadius() const {
  return WR;
}

float BikeParameters::HeadSize()    const {
  return fHeadSize;
}

float BikeParameters::MaxEngine()   const {
  return fMaxEngine;
}

float BikeParameters::MaxBrake()    const {
  return fMaxBrake;
}
