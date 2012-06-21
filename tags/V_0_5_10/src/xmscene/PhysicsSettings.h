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

#ifndef __PHYSICSSETTINGS_H__
#define __PHYSICSSETTINGS_H__

#include <string>
#include "../VFileIO_types.h"

/* ODE to Chipmunk Scaling Ratio */
#define CHIP_SCALE_RATIO   10.0
#define CHIP_GRAVITY_RATIO  3.6

class PhysicsSettings {
  public:
  PhysicsSettings(const std::string& i_filename);
  ~PhysicsSettings();

  float WorldErp() 		       const { return m_world_erp;                      }
  float WorldCfm() 		       const { return m_world_cfm;                      }
  float WorldGravity()                 const { return m_world_gravity;                  }
  float SimulationSpeedFactor()        const { return m_simulation_speed_factor;        }
  int   SimulationStepIterations()     const { return m_simulation_step_iterations;     }
  float BikeWheelRadius() 	       const { return m_bike_wheel_radius;              }
  float BikeWheelBase()   	       const { return m_bike_wheel_base;                }
  float BikeWheelMass()   	       const { return m_bike_wheel_mass;                }
  float BikeWheelRoll_resistance()     const { return m_bike_wheel_roll_resistance;     }
  float BikeWheelRoll_resistanceMax()  const { return m_bike_wheel_roll_resistance_max; }
  float BikeWheelRoll_velocityMax()    const { return m_bike_wheel_roll_velocity_max;   }
  float BikeWheelErp()  	       const { return m_bike_wheel_erp;                 }
  float BikeWheelCfm()  	       const { return m_bike_wheel_cfm;                 }
  float BikeWheelBlockGrip() 	       const { return m_bike_wheelblock_grip;           }
  float RiderElbowX()     	       const { return m_rider_elbow_x;                  }
  float RiderElbowY()     	       const { return m_rider_elbow_y;                  }
  float RiderHandX()      	       const { return m_rider_hand_x;                   }
  float RiderHandY()      	       const { return m_rider_hand_y;                   }
  float RiderShoulderX()  	       const { return m_rider_shoulder_x;               }
  float RiderShoulderY()  	       const { return m_rider_shoulder_y;               }
  float RiderLowerbodyX() 	       const { return m_rider_lowerbody_x;              }
  float RiderLowerbodyY() 	       const { return m_rider_lowerbody_y;              }
  float RiderKneeX() 	    	       const { return m_rider_knee_x;                   }
  float RiderKneeY() 	    	       const { return m_rider_knee_y;                   }
  float RiderFootX() 	    	       const { return m_rider_foot_x;                   }
  float RiderFootY() 	    	       const { return m_rider_foot_y;                   }
  float RiderHeadSize()   	       const { return m_rider_head_size;                }
  float RiderNeckLength() 	       const { return m_rider_neck_length;              }
  float BikeRearSuspensionAnchorX()    const { return m_bike_rear_suspension_anchor_x;  }
  float BikeRearSuspensionAnchorY()    const { return m_bike_rear_suspension_anchor_y;  }
  float BikeFrontSuspensionAnchorX()   const { return m_bike_front_suspension_anchor_x; }
  float BikeFrontSuspensionAnchorY()   const { return m_bike_front_suspension_anchor_y; }
  float BikeSuspensionsSpring()        const { return m_bike_suspensions_spring;        }
  float BikeSuspensionsDamp()          const { return m_bike_suspensions_damp;          }
  float RiderSpring()      	       const { return m_rider_spring;                   }
  float RiderDamp()        	       const { return m_rider_damp;                     }
  float InertialLength()   	       const { return m_inertial_length;                }
  float InertialHeight()   	       const { return m_inertial_height;                }
  float RiderAttitudeTorque()          const { return m_rider_attitude_torque;          }
  float RiderAttitudeDefactor()        const { return m_rider_attitude_defactor;        }
  float MassElevation()     	       const { return m_mass_elevation;                 }
  float EngineRpmMin()      	       const { return m_engine_rpm_min;                 }
  float EngineRpmMax()      	       const { return m_engine_rpm_max;                 }
  float EnginePowerMax()    	       const { return m_engine_power_max;               }
  float DepthFactor()                  const { return m_depth_factor;                   }
  float DeadWheelDetachSpeed()         const { return m_dead_wheel_detach_speed;        }
  float BikeBrakeFactor()              const { return m_bike_brake_factor;              }
  float EngineDamp()                   const { return m_engine_damp;                    }
  float BikeFrameMass()                const { return m_bike_frame_mass;                }
  float RiderTorsoMass()               const { return m_rider_torso_mass;               }
  float RiderUpperlegMass()            const { return m_rider_upperleg_mass;            }
  float RiderLowerlegMass()            const { return m_rider_lowerleg_mass;            }
  float RiderUpperarmMass()            const { return m_rider_upperarm_mass;            }
  float RiderLowerarmMass()            const { return m_rider_lowerarm_mass;            }
  float RiderFootMass()                const { return m_rider_foot_mass;                }
  float RiderHandMass()                const { return m_rider_hand_mass;                }
  float RiderAnchorsErp()              const { return m_rider_anchors_erp;              }
  float RiderAnchorsCfm()              const { return m_rider_anchors_cfm;              }

  private:
  void load(FileDataType i_fdt, const std::string& i_filename);

  std::string m_name;
  float m_world_erp;
  float m_world_cfm;
  float m_world_gravity;
  float m_simulation_speed_factor;
  int   m_simulation_step_iterations;
  float m_bike_wheel_radius;
  float m_bike_wheel_base;
  float m_bike_wheel_mass;
  float m_bike_wheel_roll_resistance;
  float m_bike_wheel_roll_resistance_max;
  float m_bike_wheel_roll_velocity_max;
  float m_bike_wheel_erp;
  float m_bike_wheel_cfm;
  float m_bike_wheelblock_grip;  
  float m_rider_elbow_x;
  float m_rider_elbow_y;
  float m_rider_hand_x;
  float m_rider_hand_y;
  float m_rider_shoulder_x;
  float m_rider_shoulder_y;
  float m_rider_lowerbody_x;
  float m_rider_lowerbody_y;
  float m_rider_knee_x;
  float m_rider_knee_y;
  float m_rider_foot_x;
  float m_rider_foot_y;
  float m_rider_head_size;
  float m_rider_neck_length;
  float m_bike_rear_suspension_anchor_x;
  float m_bike_rear_suspension_anchor_y;
  float m_bike_front_suspension_anchor_x;
  float m_bike_front_suspension_anchor_y;
  float m_bike_suspensions_spring;
  float m_bike_suspensions_damp;
  float m_rider_spring;
  float m_rider_damp;
  float m_inertial_length;
  float m_inertial_height;
  float m_rider_attitude_torque;
  float m_rider_attitude_defactor;
  float m_mass_elevation;
  float m_engine_rpm_min;
  float m_engine_rpm_max;
  float m_engine_power_max;
  float m_depth_factor;
  // minimum velocity so that the wheel is detached once the player is dead
  float m_dead_wheel_detach_speed;
  float m_bike_brake_factor;
  float m_engine_damp;
  float m_bike_frame_mass;
  float m_rider_torso_mass;
  float m_rider_upperleg_mass;
  float m_rider_lowerleg_mass;
  float m_rider_upperarm_mass;
  float m_rider_lowerarm_mass;
  float m_rider_foot_mass;
  float m_rider_hand_mass;
  float m_rider_anchors_erp;
  float m_rider_anchors_cfm;
};

#endif
