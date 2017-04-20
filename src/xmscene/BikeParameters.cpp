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

#include "BikeParameters.h"
#include "xmoto/PhysSettings.h"
#include "PhysicsSettings.h"

BikeParameters::BikeParameters(PhysicsSettings* i_physicsSettings) {
  setDefaults(i_physicsSettings);
}

BikeParameters::~BikeParameters() {
}

void BikeParameters::setDefaults(PhysicsSettings* i_physicsSettings) {
  WR = i_physicsSettings->BikeWheelRadius();
  Wb = i_physicsSettings->BikeWheelBase();
  Wm = i_physicsSettings->BikeWheelMass();

  Ch = i_physicsSettings->MassElevation();

  RVx = i_physicsSettings->BikeRearSuspensionAnchorX();
  RVy = i_physicsSettings->BikeRearSuspensionAnchorY();
  FVx = i_physicsSettings->BikeFrontSuspensionAnchorX();
  FVy = i_physicsSettings->BikeFrontSuspensionAnchorY();
    
  PEVx = i_physicsSettings->RiderElbowX();
  PEVy = i_physicsSettings->RiderElbowY();
  PHVx = i_physicsSettings->RiderHandX();
  PHVy = i_physicsSettings->RiderHandY();
  PSVx = i_physicsSettings->RiderShoulderX();
  PSVy = i_physicsSettings->RiderShoulderY();
  PLVx = i_physicsSettings->RiderLowerbodyX();
  PLVy = i_physicsSettings->RiderLowerbodyY();
  PKVx = i_physicsSettings->RiderKneeX(); 
  PKVy = i_physicsSettings->RiderKneeY();
  PFVx = i_physicsSettings->RiderFootX();
  PFVy = i_physicsSettings->RiderFootY();
  
  BPm_torso = i_physicsSettings->RiderTorsoMass();
  BPm_uleg  = i_physicsSettings->RiderUpperlegMass();
  BPm_lleg  = i_physicsSettings->RiderLowerlegMass();
  BPm_uarm  = i_physicsSettings->RiderUpperarmMass();
  BPm_larm  = i_physicsSettings->RiderLowerarmMass();
  BPm_foot  = i_physicsSettings->RiderFootMass();
  BPm_hand  = i_physicsSettings->RiderHandMass();

  Fm = i_physicsSettings->BikeFrameMass();
  IL = i_physicsSettings->InertialLength();
  IH = i_physicsSettings->InertialHeight();

  RErp = i_physicsSettings->RiderAnchorsErp();
  RCfm = i_physicsSettings->RiderAnchorsCfm();
  
  fMaxEngine = i_physicsSettings->EnginePowerMax();
  
  fHeadSize = i_physicsSettings->RiderHeadSize();
  fNeckLength = i_physicsSettings->RiderNeckLength();
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
