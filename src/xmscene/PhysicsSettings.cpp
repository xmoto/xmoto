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

#include "PhysicsSettings.h"
#include "common/VXml.h"
#include "helpers/VExcept.h"

PhysicsSettings::PhysicsSettings(const std::string &i_filename) {
  load(FDT_DATA, i_filename);
}

PhysicsSettings::~PhysicsSettings() {}

void PhysicsSettings::load(FileDataType i_fdt, const std::string &i_filename) {
  XMLDocument v_xml;
  xmlNodePtr v_xmlElt;
  std::string v_name, v_value;

  bool v_world_erp_done = false;
  bool v_world_cfm_done = false;
  bool v_world_gravity_done = false;
  bool v_simulation_speed_factor_done = false;
  bool v_simulation_step_iterations_done = false;
  bool v_bike_wheel_radius_done = false;
  bool v_bike_wheel_base_done = false;
  bool v_bike_wheel_mass_done = false;
  bool v_bike_wheel_roll_resistance_done = false;
  bool v_bike_wheel_roll_resistance_max_done = false;
  bool v_bike_wheel_roll_velocity_max_done = false;
  bool v_bike_wheel_erp_done = false;
  bool v_bike_wheel_cfm_done = false;
  bool v_bike_wheelblock_grip_done = false;
  bool v_rider_elbow_x_done = false;
  bool v_rider_elbow_y_done = false;
  bool v_rider_hand_x_done = false;
  bool v_rider_hand_y_done = false;
  bool v_rider_shoulder_x_done = false;
  bool v_rider_shoulder_y_done = false;
  bool v_rider_lowerbody_x_done = false;
  bool v_rider_lowerbody_y_done = false;
  bool v_rider_knee_x_done = false;
  bool v_rider_knee_y_done = false;
  bool v_rider_foot_x_done = false;
  bool v_rider_foot_y_done = false;
  bool v_rider_head_size_done = false;
  bool v_rider_neck_length_done = false;
  bool v_bike_rear_suspension_anchor_x_done = false;
  bool v_bike_rear_suspension_anchor_y_done = false;
  bool v_bike_front_suspension_anchor_x_done = false;
  bool v_bike_front_suspension_anchor_y_done = false;
  bool v_bike_suspensions_spring_done = false;
  bool v_bike_suspensions_damp_done = false;
  bool v_rider_spring_done = false;
  bool v_rider_damp_done = false;
  bool v_inertial_length_done = false;
  bool v_inertial_height_done = false;
  bool v_rider_attitude_torque_done = false;
  bool v_rider_attitude_defactor_done = false;
  bool v_mass_elevation_done = false;
  bool v_engine_rpm_min_done = false;
  bool v_engine_rpm_max_done = false;
  bool v_engine_power_max_done = false;
  bool v_depth_factor_done = false;
  bool v_dead_wheel_detach_speed_done = false;
  bool v_bike_brake_factor_done = false;
  bool v_engine_damp_done = false;
  bool v_bike_frame_mass_done = false;
  bool v_rider_torso_mass_done = false;
  bool v_rider_upperleg_mass_done = false;
  bool v_rider_lowerleg_mass_done = false;
  bool v_rider_upperarm_mass_done = false;
  bool v_rider_lowerarm_mass_done = false;
  bool v_rider_foot_mass_done = false;
  bool v_rider_hand_mass_done = false;
  bool v_rider_anchors_erp_done = false;
  bool v_rider_anchors_cfm_done = false;

  try {
    v_xml.readFromFile(i_fdt, i_filename);

    v_xmlElt = v_xml.getRootNode("xmoto_physics");
    if (v_xmlElt == NULL) {
      throw Exception("unable to analyze xml file");
    }

    m_name = XMLDocument::getOption(v_xmlElt, "name");
    if (m_name == "") {
      throw Exception("unnamed physics");
    }

    /* get parameters */
    for (xmlNodePtr pSubElem = XMLDocument::subElement(v_xmlElt, "parameter");
         pSubElem != NULL;
         pSubElem = XMLDocument::nextElement(pSubElem)) {
      v_name = XMLDocument::getOption(pSubElem, "name");
      if (v_name == "") {
        continue;
      }

      v_value = XMLDocument::getOption(pSubElem, "value");
      if (v_value == "") {
        continue;
      }

      if (v_name == "world_erp") {
        m_world_erp = atof(v_value.c_str());
        v_world_erp_done = true;
      }

      else if (v_name == "world_cfm") {
        m_world_cfm = atof(v_value.c_str());
        v_world_cfm_done = true;
      }

      else if (v_name == "world_gravity") {
        m_world_gravity = atof(v_value.c_str());
        v_world_gravity_done = true;
      }

      else if (v_name == "simulation_speed_factor") {
        m_simulation_speed_factor = atof(v_value.c_str());
        v_simulation_speed_factor_done = true;
      }

      else if (v_name == "simulation_step_iterations") {
        m_simulation_step_iterations = atoi(v_value.c_str());
        v_simulation_step_iterations_done = true;
      }

      else if (v_name == "bike_wheel_radius") {
        m_bike_wheel_radius = atof(v_value.c_str());
        v_bike_wheel_radius_done = true;
      }

      else if (v_name == "bike_wheel_base") {
        m_bike_wheel_base = atof(v_value.c_str());
        v_bike_wheel_base_done = true;
      }

      else if (v_name == "bike_wheel_mass") {
        m_bike_wheel_mass = atof(v_value.c_str());
        v_bike_wheel_mass_done = true;
      }

      else if (v_name == "bike_wheel_roll_resistance") {
        m_bike_wheel_roll_resistance = atof(v_value.c_str());
        v_bike_wheel_roll_resistance_done = true;
      }

      else if (v_name == "bike_wheel_roll_resistance_max") {
        m_bike_wheel_roll_resistance_max = atof(v_value.c_str());
        v_bike_wheel_roll_resistance_max_done = true;
      }

      else if (v_name == "bike_wheel_roll_velocity_max") {
        m_bike_wheel_roll_velocity_max = atof(v_value.c_str());
        v_bike_wheel_roll_velocity_max_done = true;
      }

      else if (v_name == "bike_wheel_erp") {
        m_bike_wheel_erp = atof(v_value.c_str());
        v_bike_wheel_erp_done = true;
      }

      else if (v_name == "bike_wheel_cfm") {
        m_bike_wheel_cfm = atof(v_value.c_str());
        v_bike_wheel_cfm_done = true;
      }

      else if (v_name == "bike_wheelblock_grip") {
        m_bike_wheelblock_grip = atof(v_value.c_str());
        v_bike_wheelblock_grip_done = true;
      }

      else if (v_name == "rider_elbow_x") {
        m_rider_elbow_x = atof(v_value.c_str());
        v_rider_elbow_x_done = true;
      }

      else if (v_name == "rider_elbow_y") {
        m_rider_elbow_y = atof(v_value.c_str());
        v_rider_elbow_y_done = true;
      }

      else if (v_name == "rider_hand_x") {
        m_rider_hand_x = atof(v_value.c_str());
        v_rider_hand_x_done = true;
      }

      else if (v_name == "rider_hand_y") {
        m_rider_hand_y = atof(v_value.c_str());
        v_rider_hand_y_done = true;
      }

      else if (v_name == "rider_shoulder_x") {
        m_rider_shoulder_x = atof(v_value.c_str());
        v_rider_shoulder_x_done = true;
      }

      else if (v_name == "rider_shoulder_y") {
        m_rider_shoulder_y = atof(v_value.c_str());
        v_rider_shoulder_y_done = true;
      }

      else if (v_name == "rider_lowerbody_x") {
        m_rider_lowerbody_x = atof(v_value.c_str());
        v_rider_lowerbody_x_done = true;
      }

      else if (v_name == "rider_lowerbody_y") {
        m_rider_lowerbody_y = atof(v_value.c_str());
        v_rider_lowerbody_y_done = true;
      }

      else if (v_name == "rider_knee_x") {
        m_rider_knee_x = atof(v_value.c_str());
        v_rider_knee_x_done = true;
      }

      else if (v_name == "rider_knee_y") {
        m_rider_knee_y = atof(v_value.c_str());
        v_rider_knee_y_done = true;
      }

      else if (v_name == "rider_foot_x") {
        m_rider_foot_x = atof(v_value.c_str());
        v_rider_foot_x_done = true;
      }

      else if (v_name == "rider_foot_y") {
        m_rider_foot_y = atof(v_value.c_str());
        v_rider_foot_y_done = true;
      }

      else if (v_name == "rider_head_size") {
        m_rider_head_size = atof(v_value.c_str());
        v_rider_head_size_done = true;
      }

      else if (v_name == "rider_neck_length") {
        m_rider_neck_length = atof(v_value.c_str());
        v_rider_neck_length_done = true;
      }

      else if (v_name == "bike_rear_suspension_anchor_x") {
        m_bike_rear_suspension_anchor_x = atof(v_value.c_str());
        v_bike_rear_suspension_anchor_x_done = true;
      }

      else if (v_name == "bike_rear_suspension_anchor_y") {
        m_bike_rear_suspension_anchor_y = atof(v_value.c_str());
        v_bike_rear_suspension_anchor_y_done = true;
      }

      else if (v_name == "bike_front_suspension_anchor_x") {
        m_bike_front_suspension_anchor_x = atof(v_value.c_str());
        v_bike_front_suspension_anchor_x_done = true;
      }

      else if (v_name == "bike_front_suspension_anchor_y") {
        m_bike_front_suspension_anchor_y = atof(v_value.c_str());
        v_bike_front_suspension_anchor_y_done = true;
      }

      else if (v_name == "bike_suspensions_spring") {
        m_bike_suspensions_spring = atof(v_value.c_str());
        v_bike_suspensions_spring_done = true;
      }

      else if (v_name == "bike_suspensions_damp") {
        m_bike_suspensions_damp = atof(v_value.c_str());
        v_bike_suspensions_damp_done = true;
      }

      else if (v_name == "rider_spring") {
        m_rider_spring = atof(v_value.c_str());
        v_rider_spring_done = true;
      }

      else if (v_name == "rider_damp") {
        m_rider_damp = atof(v_value.c_str());
        v_rider_damp_done = true;
      }

      else if (v_name == "inertial_length") {
        m_inertial_length = atof(v_value.c_str());
        v_inertial_length_done = true;
      }

      else if (v_name == "inertial_height") {
        m_inertial_height = atof(v_value.c_str());
        v_inertial_height_done = true;
      }

      else if (v_name == "rider_attitude_torque") {
        m_rider_attitude_torque = atof(v_value.c_str());
        v_rider_attitude_torque_done = true;
      }

      else if (v_name == "rider_attitude_defactor") {
        m_rider_attitude_defactor = atof(v_value.c_str());
        v_rider_attitude_defactor_done = true;
      }

      else if (v_name == "mass_elevation") {
        m_mass_elevation = atof(v_value.c_str());
        v_mass_elevation_done = true;
      }

      else if (v_name == "engine_rpm_min") {
        m_engine_rpm_min = atof(v_value.c_str());
        v_engine_rpm_min_done = true;
      }

      else if (v_name == "engine_rpm_max") {
        m_engine_rpm_max = atof(v_value.c_str());
        v_engine_rpm_max_done = true;
      }

      else if (v_name == "engine_power_max") {
        m_engine_power_max = atof(v_value.c_str());
        v_engine_power_max_done = true;
      }

      else if (v_name == "depth_factor") {
        m_depth_factor = atof(v_value.c_str());
        v_depth_factor_done = true;
      }

      else if (v_name == "dead_wheel_detach_speed") {
        m_dead_wheel_detach_speed = atof(v_value.c_str());
        v_dead_wheel_detach_speed_done = true;
      }

      else if (v_name == "bike_brake_factor") {
        m_bike_brake_factor = atof(v_value.c_str());
        v_bike_brake_factor_done = true;
      }

      else if (v_name == "engine_damp") {
        m_engine_damp = atof(v_value.c_str());
        v_engine_damp_done = true;
      }

      else if (v_name == "bike_frame_mass") {
        m_bike_frame_mass = atof(v_value.c_str());
        v_bike_frame_mass_done = true;
      }

      else if (v_name == "rider_torso_mass") {
        m_rider_torso_mass = atof(v_value.c_str());
        v_rider_torso_mass_done = true;
      }

      else if (v_name == "rider_upperleg_mass") {
        m_rider_upperleg_mass = atof(v_value.c_str());
        v_rider_upperleg_mass_done = true;
      }

      else if (v_name == "rider_lowerleg_mass") {
        m_rider_lowerleg_mass = atof(v_value.c_str());
        v_rider_lowerleg_mass_done = true;
      }

      else if (v_name == "rider_upperarm_mass") {
        m_rider_upperarm_mass = atof(v_value.c_str());
        v_rider_upperarm_mass_done = true;
      }

      else if (v_name == "rider_lowerarm_mass") {
        m_rider_lowerarm_mass = atof(v_value.c_str());
        v_rider_lowerarm_mass_done = true;
      }

      else if (v_name == "rider_foot_mass") {
        m_rider_foot_mass = atof(v_value.c_str());
        v_rider_foot_mass_done = true;
      }

      else if (v_name == "rider_hand_mass") {
        m_rider_hand_mass = atof(v_value.c_str());
        v_rider_hand_mass_done = true;
      }

      else if (v_name == "rider_anchors_erp") {
        m_rider_anchors_erp = atof(v_value.c_str());
        v_rider_anchors_erp_done = true;
      }

      else if (v_name == "rider_anchors_cfm") {
        m_rider_anchors_cfm = atof(v_value.c_str());
        v_rider_anchors_cfm_done = true;
      }
    }

    if (v_world_erp_done == false || v_world_cfm_done == false ||
        v_world_gravity_done == false ||
        v_simulation_speed_factor_done == false ||
        v_simulation_step_iterations_done == false ||
        v_bike_wheel_radius_done == false || v_bike_wheel_base_done == false ||
        v_bike_wheel_mass_done == false ||
        v_bike_wheel_roll_resistance_done == false ||
        v_bike_wheel_roll_resistance_max_done == false ||
        v_bike_wheel_roll_velocity_max_done == false ||
        v_bike_wheel_erp_done == false || v_bike_wheel_cfm_done == false ||
        v_bike_wheelblock_grip_done == false || v_rider_elbow_x_done == false ||
        v_rider_elbow_y_done == false || v_rider_hand_x_done == false ||
        v_rider_hand_y_done == false || v_rider_shoulder_x_done == false ||
        v_rider_shoulder_y_done == false || v_rider_lowerbody_x_done == false ||
        v_rider_lowerbody_y_done == false || v_rider_knee_x_done == false ||
        v_rider_knee_y_done == false || v_rider_foot_x_done == false ||
        v_rider_foot_y_done == false || v_rider_head_size_done == false ||
        v_rider_neck_length_done == false ||
        v_bike_rear_suspension_anchor_x_done == false ||
        v_bike_rear_suspension_anchor_y_done == false ||
        v_bike_front_suspension_anchor_x_done == false ||
        v_bike_front_suspension_anchor_y_done == false ||
        v_bike_suspensions_spring_done == false ||
        v_bike_suspensions_damp_done == false || v_rider_spring_done == false ||
        v_rider_damp_done == false || v_inertial_length_done == false ||
        v_inertial_height_done == false ||
        v_rider_attitude_torque_done == false ||
        v_rider_attitude_defactor_done == false ||
        v_mass_elevation_done == false || v_engine_rpm_min_done == false ||
        v_engine_rpm_max_done == false || v_engine_power_max_done == false ||
        v_depth_factor_done == false ||
        v_dead_wheel_detach_speed_done == false ||
        v_bike_brake_factor_done == false || v_engine_damp_done == false ||
        v_bike_frame_mass_done == false || v_rider_torso_mass_done == false ||
        v_rider_upperleg_mass_done == false ||
        v_rider_lowerleg_mass_done == false ||
        v_rider_upperarm_mass_done == false ||
        v_rider_lowerarm_mass_done == false ||
        v_rider_foot_mass_done == false || v_rider_hand_mass_done == false ||
        v_rider_anchors_erp_done == false || v_rider_anchors_cfm_done == false

    ) {
      throw Exception("Incomplete xml file");
    }

  } catch (Exception &e) {
    throw Exception("unable to analyze xml file:\n" + e.getMsg());
  }
}
