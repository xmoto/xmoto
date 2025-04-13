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

#include "Camera.h"
#include "Bike.h"
#include "BikeGhost.h"
#include "drawlib/DrawLib.h"
#include "helpers/Log.h"
#include "xmoto/Game.h"
#include "xmoto/SysMessage.h"
#include <sstream>

#define ZOOM_DEFAULT 0.24
#define ZOOM_DEFAULT_NO_AUTOZOOM 0.195
#define CAMERA_OFFSETX_DEFAULT 0.6
#define CAMERA_OFFSETY_DEFAULT 0.0
#define CAMERA_SMOOTHNESS 0.5

// declared for Active Camera Zooming
#define SPEED_UNTIL_ZOOM_BEGIN 24.0
#define SPEED_UNTIL_ZOOM_END 15.0
#define CAM_ZOOM_NEAR 0.24
#define CAM_ZOOM_FAR 0.18
#define ZOOM_OUT_SPEED -0.0004
#define ZOOM_IN_SPEED 0.0004
#define TRESHOLD_IN 1.0 * 1000
#define TRESHOLD_OUT 1.0 * 1000
#define DEFAULT_ROTATIONANGLE_SPEED 1.0

// declared for Trail Camera
#define TRAIL_TRACKINGSPEED 0.3
#define TRAILCAM_CATCHPROXIMITY 2.0
#define TRAILCAM_MAX_FORWARDSTEPS 21
#define TRAILCAM_MIN_FORWARDSTEPS 3
#define TRAILCAM_SMOOTHNESS 2
#define TRAILCAM_MAXSPEED 0.03
#define TRAILCAM_MINSPEED 0.005
#define TRAILCAM_SPEEDREACTIVITY 0.0001
#define TRAILCAM_TRACKINGSHOT_SPEED 0.008 // smaller is faster
#define TRAILCAM_DAMPING 0.01 // smaller damps more

Camera::Camera(Vector2i downleft, Vector2i upright) {
  m_fScale = ZOOM_DEFAULT;
  m_cameraOffsetX = CAMERA_OFFSETX_DEFAULT;
  m_cameraOffsetY = CAMERA_OFFSETY_DEFAULT;
  m_playerToFollow = NULL;
  setRenderSurface(downleft, upright);
  m_mirrored = false;
  m_allowActiveZoom = false;
  m_cameraDeathOffset = Vector2f(0.0, 0.0);
  m_ghostTrail = NULL;
  m_trailAvailable = false;
  m_useTrailCam = false;
  m_catchTrail = false;
  m_trackingShotActivated = false;
  prepareForNewLevel();
}

Camera::~Camera() {
  DrawLib *drawLib = GameApp::instance()->getDrawLib();
  drawLib->setRenderSurface(
    new RenderSurface(m_renderSurf.downleft(), m_renderSurf.upright()), true);
}

void Camera::prepareForNewLevel() {
  m_fCurrentHorizontalScrollShift = 0.0f;
  m_fCurrentVerticalScrollShift = 0.0f;
  m_previous_driver_dir = DD_LEFT;
  if (!m_useTrailCam)
    m_recenter_camera_fast = true;
  m_rotationAngle = 0.0;
  m_rotationSpeed = DEFAULT_ROTATIONANGLE_SPEED;
  m_desiredRotationAngle = 0.0;
  m_lastSpeedTime = 0.0;
  setScroll(false, Vector2f(0, -9.81));
  resetActiveZoom();
  initActiveZoom();
  m_nShadeTime = GameApp::getXMTime();
  m_doShade = false;
  m_doShadeAnim = false;
}

void Camera::resetActiveZoom() {
  m_timeTresholdIn = 0;
  m_timeTresholdOut = 0;
  m_currentActiveZoom = none;
}

void Camera::setRelativeZoom(float i_relZoom) {
  m_fScale += i_relZoom;
  if (m_fScale < 0) {
    m_fScale = 0;
  }
}

void Camera::desactiveActionZoom() {
  if (m_useActiveZoom == true) {
    m_useActiveZoom = false;
    m_fScale = m_initialZoom;
  }
}

void Camera::initActiveZoom() {
  m_useActiveZoom = true;
  // for the moment, put it to default value when there wasn't
  // autozoom camera.
  m_initialZoom = ZOOM_DEFAULT_NO_AUTOZOOM;
}

void Camera::initZoom() {
  if (m_allowActiveZoom == true && m_useActiveZoom == true)
    m_fScale = ZOOM_DEFAULT;
  else
    m_fScale = ZOOM_DEFAULT_NO_AUTOZOOM;
}

void Camera::setCameraPosition(float px, float py) {
  m_cameraOffsetX = m_Scroll.x + px;
  m_cameraOffsetY = m_Scroll.y + py;
}

void Camera::moveCamera(float px, float py) {
  m_cameraOffsetX += px;
  m_cameraOffsetY += py;
}

void Camera::initCameraPosition() {
  m_cameraOffsetX = CAMERA_OFFSETX_DEFAULT;
  m_cameraOffsetY = CAMERA_OFFSETY_DEFAULT;
}

void Camera::initCamera() {
  initCameraPosition();
  initZoom();
}

void Camera::setUseTrailCam(bool i_value) {
  m_useTrailCam = i_value;

  if (m_useTrailCam == false && m_catchTrail && m_trailAvailable) {
    m_catchTrail = false;
    initCameraPosition();
  }
}

void Camera::initTrailCam(GhostTrail *i_ghostTrail) {
  if (i_ghostTrail == NULL) {
    m_trailAvailable = false;
    m_ghostTrail = NULL;
    return;
  }

  m_trailAvailable = true;
  m_ghostTrail = i_ghostTrail;
  m_trailAvailable = i_ghostTrail->getGhostTrailAvailable();
  m_currentNearestTrailDataPosition = 0;
  m_trackShotIndex = 0;
  m_trackShotStartIndex = 0;
  m_dampVectorMax = Vector2f(m_cameraOffsetX, m_cameraOffsetY);
  m_dampCurrent = Vector2f(m_cameraOffsetX, m_cameraOffsetY);

  m_trackingShotActivated = false;
  m_catchTrail = false;
  m_trailCamForwardSteps = 0;
}

Vector2f Camera::updateTrailCam() {
  if (m_useTrailCam && m_catchTrail && m_trailAvailable) {
    // calculate how many forward steps depending from player speed
    m_trailCamForwardSteps = int(m_playerToFollow->getBikeLinearVel() * 0.4) +
                             TRAILCAM_MIN_FORWARDSTEPS;
    if (m_trailCamForwardSteps > TRAILCAM_MAX_FORWARDSTEPS)
      m_trailCamForwardSteps = TRAILCAM_MAX_FORWARDSTEPS;

    if (((*m_ghostTrail->getSimplifiedGhostTrailData()).size() >
         (m_currentNearestTrailDataPosition + m_trailCamForwardSteps))) {
      // we want to use the trail to predict the camera track
      return (
        *m_ghostTrail
           ->getSimplifiedGhostTrailData())[m_currentNearestTrailDataPosition +
                                            m_trailCamForwardSteps];
    }
  }
  return Vector2f(0, 0);
}

Vector2f Camera::dampCam(Vector2f v_inputVec) {
#define SPEEDSTEP \
  0.002 // just to adjust the player speed to the necessary value range
  float dampValue = TRAILCAM_DAMPING +
                    m_playerToFollow->getBikeLinearVel() *
                      SPEEDSTEP; // damping dependent also from player velocity
  if (m_dampVectorMax.x >= 0)
    m_dampVectorMax.x -= dampValue;
  else
    m_dampVectorMax.x += dampValue;
  if (m_dampVectorMax.y >= 0)
    m_dampVectorMax.y -= dampValue;
  else
    m_dampVectorMax.y += dampValue;

  if ((v_inputVec.x > m_dampVectorMax.x) &&
      v_inputVec.x >= 0) { // set new X Peak
    m_dampVectorMax.x = v_inputVec.x;
  }
  if ((v_inputVec.x < m_dampVectorMax.x) && v_inputVec.x < 0) {
    m_dampVectorMax.x = v_inputVec.x;
  }
  if ((v_inputVec.y > m_dampVectorMax.y) &&
      v_inputVec.y >= 0) { // set new Y peak
    m_dampVectorMax.y = v_inputVec.y;
  }
  if ((v_inputVec.y < m_dampVectorMax.y) && v_inputVec.y < 0) {
    m_dampVectorMax.y = v_inputVec.y;
  }

  m_dampCurrent.x = SimpleInterpolate(m_dampCurrent.x, m_dampVectorMax.x, 1.1);
  m_dampCurrent.y = SimpleInterpolate(m_dampCurrent.y, m_dampVectorMax.y, 1.1);

  if (XMSession::instance()->debug()) {
    std::stringstream outX;
    outX << m_dampCurrent.x;
    std::stringstream outY;
    outY << m_dampCurrent.y;
    std::stringstream outMX;
    outMX << m_dampVectorMax.x;
    std::stringstream outMY;
    outMY << m_dampVectorMax.y;
    std::stringstream inX, inY;
    inX << v_inputVec.x;
    inY << v_inputVec.y;
    std::stringstream deltaDamp;
    deltaDamp << dampValue;
    SysMessage::instance()->displayText(
      "Cam Damp XY: " + outX.str() + "  |  " + outY.str() + "\nMAX XY: " +
      outMX.str() + "  |  " + outMY.str() + "\nIN  XY: " + inX.str() + "  |  " +
      inY.str() + "\nDampVal: " + deltaDamp.str());
  }

  return m_dampCurrent;
}

Vector2f Camera::getTrailCamAimPos() {
  return (
    *m_ghostTrail
       ->getSimplifiedGhostTrailData())[m_currentNearestTrailDataPosition +
                                        m_trailCamForwardSteps];
}

Vector2f Camera::getNearestPointOnTrailPos() {
  return (
    *m_ghostTrail
       ->getSimplifiedGhostTrailData())[m_currentNearestTrailDataPosition];
}

void Camera::trailCamTrackingShot() {
  // calculate camera offset for trail cam here
  if (m_previousTSStepTime < 0.0) {
    m_previousTSStepTime = GameApp::getXMTime();
  } else {
    if (GameApp::getXMTime() - m_previousTSStepTime >
        TRAILCAM_TRACKINGSHOT_SPEED) { /* do it regularly */
      m_previousTSStepTime = GameApp::getXMTime();

      if (m_trackingShotActivated && m_trailAvailable) {
        if ((*m_ghostTrail->getSimplifiedGhostTrailData()).size() >
            unsigned(m_trackShotIndex)) {
          Vector2f v_step;
          v_step.x = SimpleInterpolate(
            (*m_ghostTrail
                ->getSimplifiedGhostTrailData())[int(m_trackShotIndex)]
              .x,
            getCameraPositionX(),
            1.1); // SMOOTH :')
          v_step.y = SimpleInterpolate(
            (*m_ghostTrail
                ->getSimplifiedGhostTrailData())[int(m_trackShotIndex)]
              .y,
            getCameraPositionY(),
            1.1); // SMOOTH :')
          setCameraPosition(v_step.x, v_step.y);
          m_trackShotIndex += TRAIL_TRACKINGSPEED;
        } else
          m_trackShotIndex = m_trackShotStartIndex; // restart trail tracking on
        // current player position
      }
    }
  }
}

bool Camera::useTrackingShot() const {
  return m_trackingShotActivated;
}

void Camera::setUseTrackingShot(bool i_value) {
  m_trackingShotActivated = i_value;
  initCameraPosition();

  if (m_trackingShotActivated == false) {
    m_trackShotStartIndex =
      locateNearestPointOnInterpolatedTrail(false); // test true here
    m_trackShotIndex = m_trackShotStartIndex;
  }
}

unsigned int Camera::locateNearestPointOnInterpolatedTrail(bool i_optimize) {
  // delivers the position index for m_trailData for the nearest trailpoint to
  // camera

  if (m_trailAvailable && m_ghostTrail != NULL &&
      (getPlayerToFollow() != NULL)) {
    if ((*m_ghostTrail->getSimplifiedGhostTrailData()).size() != 0) {
      unsigned int v_returnPos = 0;
      if (!i_optimize) { // check whole TrailData

        Vector2f v_Vtmp;
        Vector2f v_Vtmp_nearestPos =
          (*m_ghostTrail->getSimplifiedGhostTrailData())[0] -
          getPlayerToFollow()->getState()->CenterP;

        for (unsigned int i =
               (*m_ghostTrail->getSimplifiedGhostTrailData()).size() - 1;
             i > 0;
             i--) {
          v_Vtmp = (*m_ghostTrail->getSimplifiedGhostTrailData())[i] -
                   m_playerToFollow->getState()->CenterP;

          if (v_Vtmp.length() < v_Vtmp_nearestPos.length()) {
            v_Vtmp_nearestPos = v_Vtmp;
            v_returnPos = i;
          }
        }
        return v_returnPos;
      } else { // optimized trail means check just a little section of the whole
        // trail, from bike position 20 forward (also for catchtrail)

        Vector2f v_Vtmp;
        Vector2f v_Vtmp_nearestPos =
          (*m_ghostTrail->getSimplifiedGhostTrailData())[0] -
          getPlayerToFollow()->getState()->CenterP;

        for (unsigned int i = m_currentNearestTrailDataPosition;
             (i < (*m_ghostTrail->getSimplifiedGhostTrailData()).size() - 20)
               ? (i < m_currentNearestTrailDataPosition + 20)
               : (i < (*m_ghostTrail->getSimplifiedGhostTrailData()).size());
             i++) {
          v_Vtmp = (*m_ghostTrail->getSimplifiedGhostTrailData())[i] -
                   m_playerToFollow->getState()->CenterP;

          if (v_Vtmp.length() < v_Vtmp_nearestPos.length()) {
            v_Vtmp_nearestPos = v_Vtmp;
            v_returnPos = i;
          }
        }
        return v_returnPos;
      }
    }
  }
  return 0;
}

void Camera::checkIfNearTrail() {
  if (m_useTrailCam && m_trailAvailable && getPlayerToFollow() != NULL) {
    m_currentNearestTrailDataPosition =
      locateNearestPointOnInterpolatedTrail(true);
    if (m_currentNearestTrailDataPosition <
        (*m_ghostTrail->getSimplifiedGhostTrailData()).size()) {
      if (fabs((*m_ghostTrail->getSimplifiedGhostTrailData())
                 [m_currentNearestTrailDataPosition]
                   .length() -
               getPlayerToFollow()->getState()->CenterP.length()) <
          TRAILCAM_CATCHPROXIMITY) {
        m_catchTrail = true;
      } else
        m_catchTrail = false;
    }
  }
}

float Camera::getCameraPositionX() {
  return -m_Scroll.x + m_cameraOffsetX;
}

float Camera::getCameraPositionY() {
  return -m_Scroll.y + m_cameraOffsetY;
}

void Camera::guessDesiredCameraPosition(float &p_fDesiredHorizontalScrollShift,
                                        float &p_fDesiredVerticalScrollShift,
                                        const Vector2f &gravity) {
  float normal_hoffset = 1.7;
  float normal_voffset = 2.0;

  p_fDesiredHorizontalScrollShift = 0.0;
  p_fDesiredVerticalScrollShift = 0.0;

  if (m_playerToFollow == NULL) {
    return;
  }

  if (m_useTrailCam && m_catchTrail && m_trailAvailable &&
      !m_trackingShotActivated) { // trail cam! cowabungaaa!

    normal_hoffset = 3.0;
    normal_voffset = 2.0;

    Vector2f v_shiftVector = -dampCam(updateTrailCam()) +
                             m_playerToFollow->getState()->CenterP; // get aim

    p_fDesiredVerticalScrollShift = v_shiftVector.y;
    p_fDesiredHorizontalScrollShift = v_shiftVector.x;
  } else if (!m_trackingShotActivated) { // default, trail cam inactive

    p_fDesiredHorizontalScrollShift = gravity.y * normal_hoffset / 9.81;
    if (m_playerToFollow->getState()->Dir == DD_LEFT) {
      p_fDesiredHorizontalScrollShift *= -1;
    }

    p_fDesiredVerticalScrollShift = gravity.x * normal_voffset / 9.81;
    if (m_playerToFollow->getState()->Dir == DD_RIGHT) {
      p_fDesiredVerticalScrollShift *= -1;
    }
  }

  /* allow maximum and maximum, means dont let the camera go out of the hoffset
   * and voffset borders */
  if (p_fDesiredHorizontalScrollShift > normal_hoffset) {
    p_fDesiredHorizontalScrollShift = normal_hoffset;
  }
  if (p_fDesiredHorizontalScrollShift < -normal_hoffset) {
    p_fDesiredHorizontalScrollShift = -normal_hoffset;
  }
  if (p_fDesiredVerticalScrollShift > normal_voffset) {
    p_fDesiredVerticalScrollShift = normal_voffset;
  }
  if (p_fDesiredVerticalScrollShift < -normal_voffset) {
    p_fDesiredVerticalScrollShift = -normal_voffset;
  }
}

void Camera::guessDesiredCameraZoom() {
  float v_bike_speed = getPlayerToFollow()->getBikeLinearVel();

  if (getPlayerToFollow() != NULL) {
    // ZOOM out
    if (v_bike_speed > SPEED_UNTIL_ZOOM_BEGIN) {
      m_timeTresholdOut += (int)(0.01 * 1000.0);
      m_timeTresholdIn = 0;
      if (m_timeTresholdOut > TRESHOLD_OUT) {
        m_currentActiveZoom = zoomOut;
      }
    }
    // ZOOM in
    else if (v_bike_speed < SPEED_UNTIL_ZOOM_END) {
      m_timeTresholdIn += (int)(0.01 * 1000.0);
      // reduces flickering
      m_timeTresholdOut = 0;
      if (m_timeTresholdIn > TRESHOLD_IN) {
        m_currentActiveZoom = zoomIn;
      }
    } else {
      resetActiveZoom();
    }

    switch (m_currentActiveZoom) {
      case none:
        break;
      case zoomIn:
        if (m_fScale <= CAM_ZOOM_NEAR)
          setRelativeZoom(ZOOM_IN_SPEED);
        else if (m_fScale >= CAM_ZOOM_NEAR)
          resetActiveZoom();
        break;
      case zoomOut:
        if (m_fScale >= CAM_ZOOM_FAR)
          setRelativeZoom(ZOOM_OUT_SPEED);
        else if (m_fScale <= CAM_ZOOM_FAR)
          resetActiveZoom();
        break;
    }
  }
}

void Camera::setPlayerResussite() { // opposite of setPlayerDead
  m_cameraDeathOffset = Vector2f(0.0, 0.0);

  // remove shadow
  setScreenShade(false, false);
}

void Camera::setPlayerDead() {
  if (m_playerToFollow->getState()->Dir == DD_RIGHT) {
    m_cameraDeathOffset = m_playerToFollow->getState()->CenterP -
                          m_playerToFollow->getState()->KneeP;
  } else {
    m_cameraDeathOffset = m_playerToFollow->getState()->CenterP -
                          m_playerToFollow->getState()->Knee2P;
  }
  // apply shadow
  setShadeTime(GameApp::getXMTime());
  setScreenShade(true, true);
}

void Camera::setScroll(bool isSmooth, const Vector2f &gravity) {
  float v_move_camera_max;
  float v_fDesiredHorizontalScrollShift = 0.0;
  float v_fDesiredVerticalScrollShift = 0.0;
  float v_newSpeedTime;

  if (m_playerToFollow == NULL) {
    return;
  }

  v_newSpeedTime = GameApp::getXMTime();
  if ((v_newSpeedTime - m_lastSpeedTime) < 0.15) {
    return;
  }

  /* determine if the camera must move fastly */
  /* it must go faster if the player change of sense */
  if (m_previous_driver_dir != m_playerToFollow->getState()->Dir) {
    m_recenter_camera_fast = true;
  }
  m_previous_driver_dir = m_playerToFollow->getState()->Dir;

  if (!m_catchTrail && m_recenter_camera_fast) {
    v_move_camera_max = 0.05;
  } else if (m_catchTrail) {
    // make v_move_camera_max dependent from vector player_pos -
    // getTrailCamAimPos
    // v_move_camera_max =
    // TRAIL_SPEEDREACTIVITY*m_playerToFollow->getBikeLinearVel() +
    // TRAILCAM_MINSPEED;
    // v_move_camera_max = fabs((m_playerToFollow->getState()->CenterP -
    // getTrailCamAimPos()).length()) * TRAILCAM_SPEEDREACTIVITY;
    // if(v_move_camera_max < 0.01)
    v_move_camera_max = 0.01;
    if (v_move_camera_max >= TRAILCAM_MAXSPEED)
      v_move_camera_max = TRAILCAM_MAXSPEED;
  } else {
    v_move_camera_max = 0.01;
  }

  /* Determine scroll */
  if (m_playerToFollow->isDead()) {
    if (m_playerToFollow->getState()->Dir == DD_RIGHT) {
      m_Scroll = -m_playerToFollow->getState()->KneeP - m_cameraDeathOffset;
    } else {
      m_Scroll = -m_playerToFollow->getState()->Knee2P - m_cameraDeathOffset;
    }

    // update X
    if (m_cameraDeathOffset.x != 0.0 && m_cameraDeathOffset.y != 0.0) {
      if (fabs(m_cameraDeathOffset.x) > 0.015) {
        if (m_cameraDeathOffset.x > 0.0) {
          m_cameraDeathOffset.x -= 0.01;
        } else {
          m_cameraDeathOffset.x += 0.01;
        }
      } else {
        m_cameraDeathOffset.x = 0.0;
      }
    }

    // update Y
    if (m_cameraDeathOffset.y != 0.0 && m_cameraDeathOffset.y != 0.0) {
      if (fabs(m_cameraDeathOffset.x) > 0.015) {
        if (m_cameraDeathOffset.y > 0.0) {
          m_cameraDeathOffset.y -= 0.01;
        } else {
          m_cameraDeathOffset.y += 0.01;
        }
      } else {
        m_cameraDeathOffset.y = 0.0;
      }
    }

  } else {
    m_Scroll = -m_playerToFollow->getState()->CenterP;
  }

  checkIfNearTrail();
  /* Driving direction? */
  guessDesiredCameraPosition(
    v_fDesiredHorizontalScrollShift, v_fDesiredVerticalScrollShift, gravity);

  // Switch Automatic Camera in Dependency to altered Zoom Value
  if (m_useActiveZoom == true && m_allowActiveZoom) {
    guessDesiredCameraZoom();
  }

  if (fabs(v_fDesiredHorizontalScrollShift - m_fCurrentHorizontalScrollShift) <
      v_move_camera_max) {
    /* remove fast move once the camera is set correctly */
    m_recenter_camera_fast = false;
  }

  if (v_fDesiredHorizontalScrollShift != m_fCurrentHorizontalScrollShift) {
    float d = v_fDesiredHorizontalScrollShift - m_fCurrentHorizontalScrollShift;
    if (fabs(d) < 0.01f || isSmooth == false) {
      m_fCurrentHorizontalScrollShift = v_fDesiredHorizontalScrollShift;
    } else {
      m_fCurrentHorizontalScrollShift +=
        v_move_camera_max * d * CAMERA_SMOOTHNESS;
    }
  }

  if (v_fDesiredVerticalScrollShift != m_fCurrentVerticalScrollShift) {
    float d = v_fDesiredVerticalScrollShift - m_fCurrentVerticalScrollShift;
    if (fabs(d) < 0.01f || isSmooth == false) {
      m_fCurrentVerticalScrollShift = v_fDesiredVerticalScrollShift;
    } else {
      m_fCurrentVerticalScrollShift +=
        v_move_camera_max * d * CAMERA_SMOOTHNESS;
    }
  }

  if (!m_trackingShotActivated)
    m_Scroll +=
      Vector2f(m_fCurrentHorizontalScrollShift, m_fCurrentVerticalScrollShift);
  else
    trailCamTrackingShot();
}

float Camera::getCurrentZoom() {
  return m_fScale;
}

void Camera::setAbsoluteZoom(float i_absZoom) {
  m_fScale = i_absZoom;
}

void Camera::setPlayerToFollow(Biker *i_playerToFollow) {
  m_playerToFollow = i_playerToFollow;
}

Biker *Camera::getPlayerToFollow() {
  return m_playerToFollow;
}

void Camera::activate() {
  DrawLib *drawLib = GameApp::instance()->getDrawLib();
  drawLib->setRenderSurface(&m_renderSurf, false);
}

void Camera::setCamera2d() {
  activate();

  DrawLib *drawLib = GameApp::instance()->getDrawLib();
  drawLib->setCameraDimensionality(CAMERA_2D);
}

void Camera::setCamera3d() {
  activate();

  DrawLib *drawLib = GameApp::instance()->getDrawLib();
  drawLib->setCameraDimensionality(CAMERA_3D);
}

void Camera::setRenderSurface(Vector2i downleft, Vector2i upright) {
  m_renderSurf.update(downleft, upright);
}

bool Camera::isMirrored() {
  return m_mirrored;
}

void Camera::setMirrored(bool i_value) {
  m_mirrored = i_value;
}

float Camera::rotationAngle() {
  return (m_rotationAngle * M_PI) / 180.0;
}

void Camera::setRotationAngle(float i_value) {
  m_rotationAngle = (i_value * 180.0) / M_PI;
}

void Camera::setRotationSpeed(float i_value) {
  m_rotationSpeed = (i_value * 180.0) / M_PI;
}

void Camera::setDesiredRotationAngle(float i_value) {
  m_desiredRotationAngle = (i_value * 180.0) / M_PI;
}

void Camera::adaptRotationAngleToGravity(Vector2f &gravity) {
  float v_x, v_y;

  if (gravity.x == 0.0) {
    if (gravity.y > 0) {
      m_desiredRotationAngle = 180.0;
    } else {
      m_desiredRotationAngle = 0.0;
    }
    return;
  }

  v_x = gravity.x;
  v_y = -gravity.y; // because gravity y is reversed

  if (v_x > 0.0) {
    if (v_y > 0.0) {
      m_desiredRotationAngle = atan(v_y / v_x);
    } else {
      m_desiredRotationAngle = -atan(-v_y / v_x);
    }
  } else {
    if (v_y > 0.0) {
      m_desiredRotationAngle = M_PI - atan(v_y / -v_x);
    } else {
      m_desiredRotationAngle = M_PI + atan(-v_y / -v_x);
    }
  }

  m_desiredRotationAngle = ((m_desiredRotationAngle * 180.0) / M_PI) - 90.0;
  m_desiredRotationAngle =
    (float)((int)((m_desiredRotationAngle) + 360.0) % 360);
  // printf("Gravity (%.2f, %.2f)\n", gravity.x, gravity.y);
  // printf("Desired Angle = %.2f\n", m_desiredRotationAngle < 0.0 ?
  // m_desiredRotationAngle+360.0 : m_desiredRotationAngle);
}

float Camera::guessDesiredAngleRotation() {
  if (fabs(m_desiredRotationAngle - m_rotationAngle) <= m_rotationSpeed * 3.0) {
    m_rotationAngle = m_desiredRotationAngle;
    return m_rotationAngle;
  }

  if (m_desiredRotationAngle < 0.0) {
    m_desiredRotationAngle = ((int)m_desiredRotationAngle) % 360;
    m_desiredRotationAngle += 360;
  } else {
    if (m_desiredRotationAngle >= 360.0) {
      m_desiredRotationAngle = ((int)m_desiredRotationAngle) % 360;
    }
  }

  float v_diff;
  if (m_rotationAngle > m_desiredRotationAngle) {
    v_diff = m_rotationAngle - m_desiredRotationAngle;
  } else {
    v_diff = m_rotationAngle - m_desiredRotationAngle + 360.0;
  }

  /* rotate in the fastest way */
  if (v_diff <= 180.0) {
    m_rotationAngle = m_rotationAngle - m_rotationSpeed + 360.0;
    int n = ((int)m_rotationAngle) / 360;
    m_rotationAngle -= (((float)n) * 360.0);
  } else {
    m_rotationAngle = m_rotationAngle + m_rotationSpeed + 360.0;
    int n = ((int)m_rotationAngle) / 360;
    m_rotationAngle -= (((float)n) * 360.0);
  }
  return m_rotationAngle;
}

Vector2i Camera::getDispBottomLeft() {
  return m_renderSurf.downleft();
}

Vector2i Camera::getDispUpRight() {
  return m_renderSurf.upright();
}

void Camera::allowActiveZoom(bool i_value) {
  m_allowActiveZoom = i_value;

  initZoom();
}

void Camera::clearGhostsVisibility() {
  m_ghostVisibility.clear();
}

void Camera::checkGhostVisibilityExists(unsigned int i) {
  CameraGhostVisibility cgv;

  for (unsigned int j = m_ghostVisibility.size(); j <= i; j++) {
    cgv.isIn = false;
    m_ghostVisibility.push_back(cgv);
  }
}

void Camera::setGhostIn(unsigned int i) {
  checkGhostVisibilityExists(i);

  if (m_ghostVisibility[i].isIn == false) {
    m_ghostVisibility[i].lastIn = GameApp::getXMTime();
  }
  m_ghostVisibility[i].isIn = true;
}

void Camera::setGhostOut(unsigned int i) {
  checkGhostVisibilityExists(i);
  m_ghostVisibility[i].isIn = false;
}

float Camera::getGhostLastIn(unsigned int i) {
  checkGhostVisibilityExists(i);
  return m_ghostVisibility[i].lastIn;
}

bool Camera::isGhostIn(unsigned int i) {
  checkGhostVisibilityExists(i);
  return m_ghostVisibility[i].isIn;
}

float Camera::getShadeTime() {
  return m_nShadeTime;
}

void Camera::setShadeTime(float i_time) {
  m_nShadeTime = i_time;
}

void Camera::setScreenShade(bool i_doShade, bool i_doShadeAnim) {
  m_doShade = i_doShade;
  m_doShadeAnim = i_doShadeAnim;
}
