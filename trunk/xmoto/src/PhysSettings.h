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

#ifndef __PHYSSETTINGS_H__
#define __PHYSSETTINGS_H__

/*=============================================================================
Physical setup of the bike goes in this file. You should indeed not modify this
if you want to be able to play levels besides your own...
=============================================================================*/

#define ENGINE_MIN_RPM      700.0f
#define ENGINE_MAX_RPM      8000.0f

/* World settings */
#define PHYS_WORLD_ERP                0.3f      /* global error reduction (0.3) */
#define PHYS_WORLD_CFM                0.000001f /* global constant force mix */
#define PHYS_WORLD_GRAV               -9.81f    /* Grav. acceleration */
#define PHYS_SPEED                    0.6f      /* simulation speed factor */
#define PHYS_QSTEP_ITERS              10
#define PHYS_STEP_SIZE                0.01f

/* Bike geometrical settings */
#define PHYS_WHEEL_RADIUS             0.35f
#define PHYS_MASS_ELEVATION           0.9f
#define PHYS_WHEEL_BASE               1.4f

#define PHYS_REAR_SUSP_ANCHOR_X       -0.11f
#define PHYS_REAR_SUSP_ANCHOR_Y       -0.3f
#define PHYS_FRONT_SUSP_ANCHOR_X      0.4f
#define PHYS_FRONT_SUSP_ANCHOR_Y      0.4f

#define PHYS_RIDER_ELBOW_X            -0.1f
#define PHYS_RIDER_ELBOW_Y            0.6f
#define PHYS_RIDER_HAND_X             0.3f
#define PHYS_RIDER_HAND_Y             0.45f
#define PHYS_RIDER_SHOULDER_X         -0.2f
#define PHYS_RIDER_SHOULDER_Y         1.1f
#define PHYS_RIDER_LOWERBODY_X        -0.3f  
#define PHYS_RIDER_LOWERBODY_Y        0.4f
#define PHYS_RIDER_KNEE_X             0.3f
#define PHYS_RIDER_KNEE_Y             0.2f
#define PHYS_RIDER_FOOT_X             0.0f
#define PHYS_RIDER_FOOT_Y             -0.37f
#define PHYS_RIDER_HEAD_SIZE          0.18f
#define PHYS_RIDER_NECK_LENGTH        0.22f

#define PHYS_RIDER_SPRING             100000
#define PHYS_RIDER_DAMP               300000
#define PHYS_RIDER_ATTITUDE_TORQUE    10000
#define PHYS_ATTITUDE_DEFACTOR        0.75f

/* Bike physical properties */
#define PHYS_WHEEL_MASS               10
#define PHYS_RIDER_BODYPART_MASS      5
#define PHYS_FRAME_MASS               90
#define PHYS_INERTIAL_LENGTH          1.2f
#define PHYS_INERTIAL_HEIGHT          1.8f
#define PHYS_MAX_BRAKING              10000
#define PHYS_MAX_ENGINE               1400

/* Bike suspension */
#define PHYS_SUSP_SPRING              21000
#define PHYS_SUSP_DAMP                205000

/* Misc */
#define PHYS_SLEEP_EPS                0.02 // 2006-04-23: changed from 0.008
#define PHYS_SLEEP_FRAMES             20     // 150
#define PHYS_BRAKE_FACTOR             80
#define PHYS_ENGINE_DAMP              0.4
#define PHYS_SUSP_SQUEEK_POINT        0.03

/* Wheel rolling */
#define PHYS_ROLL_RESIST              1
#define PHYS_ROLL_RESIST_MAX          20
#define PHYS_MAX_ROLL_VELOCITY        60

/* Friction */
#define PHYS_WHEEL_GRIP               20
#define PHYS_WHEEL_ERP                0.8
#define PHYS_WHEEL_CFM                0.00001

#endif




