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

#ifndef __PHYSSETTINGS_H__
#define __PHYSSETTINGS_H__

/*=============================================================================
Physical setup of the bike goes in this file. You should indeed not modify this
if you want to be able to play levels besides your own...
=============================================================================*/

#define ENGINE_MIN_RPM      700.0f
#define ENGINE_MAX_RPM      8000.0f

#if 0 /* Set to 1 to enable EXPERIMENTAL physics! */

  /* This is an attempt to lower the system requirements drastically by 
     changing the world update rate from 100 times/second to 33 times/second,
     effectively dividing the system requirements by 3. It would be EXTREMELY
     cool if this got to work, but the problem is that the larger steps causes
     the game to go unstable. 
     Note that if one wants to get these settings into a stable state, it's
     probably also necesary to change things elsewhere. 
     
     The HUGE problem with these physics is, that there's some manually 
     updated constraints, i.e. for the location of the wheels. If one could
     create an actual ODE joint type that did exactly the same, everything 
     would probably work in a wonderful state of unagi. Sigh. */  
      
  /* World settings */
  #define PHYS_WORLD_ERP                0.3f      /* global error reduction */
  #define PHYS_WORLD_CFM                0.000001f /* global constant force mix */
  #define PHYS_WORLD_GRAV               -9.81f    /* Grav. acceleration */
  #define PHYS_SPEED                    0.6f      /* simulation speed factor */
  #define PHYS_QSTEP_ITERS              10
  #define PHYS_STEP_SIZE                0.03f

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

  #define PHYS_RIDER_SPRING             800
  #define PHYS_RIDER_DAMP               500
  #define PHYS_RIDER_ATTITUDE_TORQUE    1000
  #define PHYS_ATTITUDE_DEFACTOR        0.50f

  /* Bike physical properties */
  #define PHYS_WHEEL_MASS               5
  #define PHYS_RIDER_BODYPART_MASS      5
  #define PHYS_FRAME_MASS               90
  #define PHYS_INERTIAL_LENGTH          1.2f
  #define PHYS_INERTIAL_HEIGHT          1.8f
  #define PHYS_MAX_BRAKING              100
  #define PHYS_MAX_ENGINE               300

  /* Bike suspension */
  #define PHYS_SUSP_SPRING              2000
  #define PHYS_SUSP_DAMP                20000

  /* Misc */
  #define PHYS_SLEEP_EPS                0.008
  #define PHYS_SLEEP_FRAMES             150
  #define PHYS_BRAKE_FACTOR             10
  #define PHYS_ENGINE_DAMP              0.4

  /* Wheel rolling */
  #define PHYS_ROLL_RESIST              1
  #define PHYS_ROLL_RESIST_MAX          20
  #define PHYS_MAX_ROLL_VELOCITY        60

  /* Friction */
  #define PHYS_WHEEL_GRIP               20
  #define PHYS_WHEEL_ERP                0.8
  #define PHYS_WHEEL_CFM                0.00001

#else

  /* World settings */
  #define PHYS_WORLD_ERP                0.3f      /* global error reduction */
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
  #define PHYS_SLEEP_EPS                0.008
  #define PHYS_SLEEP_FRAMES             150
  #define PHYS_BRAKE_FACTOR             80
  #define PHYS_ENGINE_DAMP              0.4

  /* Wheel rolling */
  #define PHYS_ROLL_RESIST              1
  #define PHYS_ROLL_RESIST_MAX          20
  #define PHYS_MAX_ROLL_VELOCITY        60

  /* Friction */
  #define PHYS_WHEEL_GRIP               20 // 20
  #define PHYS_WHEEL_ERP                0.8
  #define PHYS_WHEEL_CFM                0.00001

#endif

#endif




