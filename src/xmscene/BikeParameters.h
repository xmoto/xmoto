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

#ifndef __BIKEPARAMETERS_H__
#define __BIKEPARAMETERS_H__

#include "../helpers/VMath.h"

class PhysicsSettings;

class BikeParameters {
public:
  BikeParameters(PhysicsSettings *i_physicsSettings);
  ~BikeParameters();

  void setDefaults(PhysicsSettings *i_physicsSettings);

  float WheelRadius() const;
  float HeadSize() const;
  float MaxEngine() const;

  //  private:

  /* Geometrical */
  float WR; /* Wheel radius */
  float Ch; /* Center of mass height */
  float Wb; /* Wheel base */
  float RVx, RVy; /* Position of rear susp. anchor */
  float FVx, FVy; /* Position of front susp. anchor */

  float PSVx, PSVy; /* Position of player shoulder */
  float PEVx, PEVy; /* Position of player elbow */
  float PHVx, PHVy; /* Position of player hand */
  float PLVx, PLVy; /* Position of player lower body */
  float PKVx, PKVy; /* Position of player knee */
  float PFVx, PFVy; /* Position of player foot */

  float fHeadSize; /* Radius */
  float fNeckLength; /* Length of neck */

  /* Physical */
  float Wm; /* Wheel mass [kg] */
  float BPm_torso; /* Player body part mass [kg] */
  float BPm_uleg;
  float BPm_lleg;
  float BPm_uarm;
  float BPm_larm;
  float BPm_foot;
  float BPm_hand;
  float Fm; /* Frame mass [kg] */
  float IL; /* Frame "inertia" length [m] */
  float IH; /* Frame "inertia" height [m] */

  float RErp;
  float RCfm;

  /* Braking/engine performance */
  float fMaxEngine;
};

#endif /* BIKEPARAMETERS */
