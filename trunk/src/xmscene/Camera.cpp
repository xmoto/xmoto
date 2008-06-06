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
#include "Game.h"
#include "Bike.h"

#define ZOOM_DEFAULT 0.24
#define ZOOM_DEFAULT_NO_AUTOZOOM 0.195
#define CAMERA_OFFSETX_DEFAULT 0.6
#define CAMERA_OFFSETY_DEFAULT 0.0

// declared for Active Camera Zooming
#define SPEED_UNTIL_ZOOM_BEGIN 0.24
#define SPEED_UNTIL_ZOOM_END   0.15
#define CAM_ZOOM_NEAR    0.24
#define CAM_ZOOM_FAR     0.18
#define ZOOM_OUT_SPEED  -0.0004
#define ZOOM_IN_SPEED    0.0004
#define TRESHOLD_IN      1.0 * 1000
#define TRESHOLD_OUT     1.0 * 1000
#define DEFAULT_ROTATIONANGLE_SPEED 1.0

Camera::Camera(Vector2i upperleft, Vector2i downright){
  m_fScale         = ZOOM_DEFAULT;
  m_cameraOffsetX  = CAMERA_OFFSETX_DEFAULT;
  m_cameraOffsetY  = CAMERA_OFFSETY_DEFAULT;
  m_playerToFollow = NULL;
  setRenderSurface(upperleft, downright);
  m_mirrored = false;
  m_allowActiveZoom = true;

  prepareForNewLevel();
}

void Camera::prepareForNewLevel() {
  m_fCurrentHorizontalScrollShift = 0.0f;
  m_fCurrentVerticalScrollShift = 0.0f;
  m_previous_driver_dir  = DD_LEFT;    
  m_recenter_camera_fast = true;
  m_rotationAngle = 0.0;
  m_rotationSpeed = DEFAULT_ROTATIONANGLE_SPEED;
  m_desiredRotationAngle = 0.0;
  m_lastSpeedTime = 0.0;
  setScroll(false, Vector2f(0, -9.81));
  resetActiveZoom();
  initActiveZoom();
}

void Camera::resetActiveZoom()
{
  m_timeTresholdIn  = 0;
  m_timeTresholdOut = 0;
  m_currentActiveZoom = none;
}

void Camera::setRelativeZoom(float i_relZoom) {
  m_fScale += i_relZoom;
  if(m_fScale < 0) {
    m_fScale = 0;
  }
}

void Camera::desactiveActionZoom()
{
  if(m_useActiveZoom == true) {
    m_useActiveZoom = false;
    m_fScale        = m_initialZoom;
  }
}

void Camera::initActiveZoom()
{
  m_useActiveZoom = true;
  // for the moment, put it to default value when there wasn't
  // autozoom camera.
  m_initialZoom   = ZOOM_DEFAULT_NO_AUTOZOOM;
}

void Camera::initZoom() {
  if(m_allowActiveZoom == true && m_useActiveZoom == true)
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

float Camera::getCameraPositionX() {
  return -m_Scroll.x + m_cameraOffsetX;
}
  
float Camera::getCameraPositionY() {
  return -m_Scroll.y + m_cameraOffsetY;
}

void Camera::guessDesiredCameraPosition(float &p_fDesiredHorizontalScrollShift,
					float &p_fDesiredVerticalScrollShift,
					const Vector2f& gravity) {

  float normal_hoffset = 1.7;
  float normal_voffset = 2.0;             
  p_fDesiredHorizontalScrollShift = 0.0;
  p_fDesiredVerticalScrollShift   = 0.0;

  if(m_playerToFollow == NULL) {
    return;
  }

  p_fDesiredHorizontalScrollShift = gravity.y * normal_hoffset / 9.81;
  if(m_playerToFollow->getState()->Dir == DD_LEFT) {
    p_fDesiredHorizontalScrollShift *= -1;
  }

  p_fDesiredVerticalScrollShift = gravity.x * normal_voffset / 9.81;
  if(m_playerToFollow->getState()->Dir == DD_RIGHT) {
    p_fDesiredVerticalScrollShift *= -1;
  }

  /* allow maximum and maximum */
  if(p_fDesiredHorizontalScrollShift > normal_hoffset) {
    p_fDesiredHorizontalScrollShift = normal_hoffset;
  }
  if(p_fDesiredHorizontalScrollShift < -normal_hoffset) {
    p_fDesiredHorizontalScrollShift = -normal_hoffset;
  }
  if(p_fDesiredVerticalScrollShift > normal_voffset) {
    p_fDesiredVerticalScrollShift = normal_voffset;
  }
  if(p_fDesiredVerticalScrollShift < -normal_voffset) {
    p_fDesiredVerticalScrollShift = -normal_voffset;
  }
}

void Camera::guessDesiredCameraZoom() {
  float bike_speed = getPlayerToFollow()->getBikeLinearVel();

  if(getPlayerToFollow() != NULL) {
    // ZOOM out
    if(bike_speed > SPEED_UNTIL_ZOOM_BEGIN) { 
      m_timeTresholdOut += (int)(0.01 * 1000.0);
      m_timeTresholdIn = 0;
      if(m_timeTresholdOut > TRESHOLD_OUT) {
	m_currentActiveZoom = zoomOut;
      }
    }
    // ZOOM in
    else if (bike_speed < SPEED_UNTIL_ZOOM_END) {
      m_timeTresholdIn += (int)(0.01 * 1000.0);
      //reduces flickering
      m_timeTresholdOut = 0;
      if(m_timeTresholdIn > TRESHOLD_IN) {
	m_currentActiveZoom = zoomIn;
      }
    }
    else {
      resetActiveZoom();
    }

    switch (m_currentActiveZoom) {
    case none:
      break;
    case zoomIn: 
      if(m_fScale <= CAM_ZOOM_NEAR) 
	setRelativeZoom(ZOOM_IN_SPEED);
      else if(m_fScale >= CAM_ZOOM_NEAR)
	resetActiveZoom();
      break;
    case zoomOut:
      if(m_fScale >= CAM_ZOOM_FAR) 
	setRelativeZoom(ZOOM_OUT_SPEED); 
      else if(m_fScale <= CAM_ZOOM_FAR)
	resetActiveZoom();
      break;
    }    
  }
}

void Camera::setScroll(bool isSmooth, const Vector2f& gravity) {
  float v_move_camera_max;
  float v_fDesiredHorizontalScrollShift = 0.0;
  float v_fDesiredVerticalScrollShift   = 0.0;
  float v_newSpeedTime;

  if(m_playerToFollow == NULL) {
    return;
  }

  v_newSpeedTime = GameApp::getXMTime();
  if( (v_newSpeedTime - m_lastSpeedTime) < 0.15) {
    return;
  }

  /* determine if the camera must move fastly */
  /* it must go faster if the player change of sense */
  if(m_previous_driver_dir != m_playerToFollow->getState()->Dir) {
    m_recenter_camera_fast = true;
  }
  m_previous_driver_dir = m_playerToFollow->getState()->Dir;

  if(m_recenter_camera_fast) {
    v_move_camera_max = 0.05;
  } else {
    v_move_camera_max = 0.01;
  }
    
  /* Determine scroll */
  if(m_playerToFollow->isDead()) {
    if(m_playerToFollow->getState()->Dir == DD_RIGHT) {
      m_Scroll = -m_playerToFollow->getState()->KneeP;
    } else {
      m_Scroll = -m_playerToFollow->getState()->Knee2P;
    }
  } else {
    m_Scroll = -m_playerToFollow->getState()->CenterP;
  }

  /* Driving direction? */
  guessDesiredCameraPosition(v_fDesiredHorizontalScrollShift,
			     v_fDesiredVerticalScrollShift,
			     gravity);

  // Switch Automatic Camera in Dependency to altered Zoom Value
  if(m_useActiveZoom == true && m_allowActiveZoom) {
    guessDesiredCameraZoom();
  }
    
  if(fabs(v_fDesiredHorizontalScrollShift - m_fCurrentHorizontalScrollShift)
     < v_move_camera_max) {
    /* remove fast move once the camera is set correctly */
    m_recenter_camera_fast = false;
  }

  if(v_fDesiredHorizontalScrollShift != m_fCurrentHorizontalScrollShift) {
    float d = v_fDesiredHorizontalScrollShift - m_fCurrentHorizontalScrollShift;
    if(fabs(d)<v_move_camera_max || isSmooth == false) {
      m_fCurrentHorizontalScrollShift = v_fDesiredHorizontalScrollShift;
    }
    else if(d < 0.0f) {
      m_fCurrentHorizontalScrollShift -= v_move_camera_max;
    }
    else if(d > 0.0f) {
      m_fCurrentHorizontalScrollShift += v_move_camera_max;
    }
  }
    
  if(v_fDesiredVerticalScrollShift != m_fCurrentVerticalScrollShift) {
    float d = v_fDesiredVerticalScrollShift - m_fCurrentVerticalScrollShift;
    if(fabs(d)<0.01f || isSmooth == false) {
      m_fCurrentVerticalScrollShift = v_fDesiredVerticalScrollShift;
    }
    else if(d < 0.0f) {
      m_fCurrentVerticalScrollShift -= v_move_camera_max;
    }
    else if(d > 0.0f) {
      m_fCurrentVerticalScrollShift += v_move_camera_max;
    }
  }

  m_Scroll += Vector2f(m_fCurrentHorizontalScrollShift,
		       m_fCurrentVerticalScrollShift);
}

float Camera::getCurrentZoom() {
  return m_fScale;
}

void Camera::setAbsoluteZoom(float i_absZoom) {
  m_fScale = i_absZoom;
}

void Camera::setPlayerToFollow(Biker* i_playerToFollow) {
  m_playerToFollow = i_playerToFollow;
}

Biker* Camera::getPlayerToFollow() {
  return m_playerToFollow;
}

void Camera::setCamera2d() {
#ifdef ENABLE_OPENGL
  glViewport(m_renderSurf.upperleft().x, m_renderSurf.upperleft().y,
	     m_renderSurf.size().x,      m_renderSurf.size().y);						 
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, m_renderSurf.size().x,
	  0, m_renderSurf.size().y,
	  -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
#endif
}

void Camera::setCamera3d(){
#ifdef ENABLE_OPENGL
  glViewport(m_renderSurf.upperleft().x, m_renderSurf.upperleft().y,
	     m_renderSurf.size().x,      m_renderSurf.size().y);						 
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
#endif
}

void Camera::setRenderSurface(Vector2i upperleft, Vector2i downright){
  m_renderSurf.update(upperleft, downright);
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

void Camera::adaptRotationAngleToGravity(Vector2f& gravity) {
  float v_x, v_y;

  if(gravity.x == 0.0) {
    if(gravity.y > 0) {
      m_desiredRotationAngle = 180.0;
    } else {
      m_desiredRotationAngle = 0.0;
    }
    return;
  }

  v_x = gravity.x;
  v_y = -gravity.y; // because gravity y is reversed

  if(v_x > 0.0) {
    if(v_y > 0.0) {
      m_desiredRotationAngle = atan(v_y/v_x);
    } else {
      m_desiredRotationAngle = -atan(-v_y/v_x);
    }
  } else {
    if(v_y > 0.0) {
      m_desiredRotationAngle = M_PI - atan(v_y/-v_x);
    } else {
      m_desiredRotationAngle = M_PI + atan(-v_y/-v_x);
    }
  }

  m_desiredRotationAngle = ((m_desiredRotationAngle * 180.0) / M_PI) - 90.0;
  m_desiredRotationAngle = (float)((int)((m_desiredRotationAngle) + 360.0) % 360);
  //printf("Gravity (%.2f, %.2f)\n", gravity.x, gravity.y);
  //printf("Desired Angle = %.2f\n", m_desiredRotationAngle < 0.0 ? m_desiredRotationAngle+360.0 : m_desiredRotationAngle);
}

float Camera::guessDesiredAngleRotation() {
  if(fabs(m_desiredRotationAngle - m_rotationAngle) <= m_rotationSpeed * 3.0) {
    m_rotationAngle = m_desiredRotationAngle;
    return m_rotationAngle;
  }

  if(m_desiredRotationAngle < 0.0) {
    m_desiredRotationAngle = ((int)m_desiredRotationAngle)%360;
    m_desiredRotationAngle += 360;
  } else {
    if(m_desiredRotationAngle >= 360.0) {
      m_desiredRotationAngle = ((int)m_desiredRotationAngle)%360;
    }
  }

  float v_diff;
  if(m_rotationAngle > m_desiredRotationAngle) {
    v_diff = m_rotationAngle - m_desiredRotationAngle;
  } else {
    v_diff = m_rotationAngle - m_desiredRotationAngle + 360.0;
  }
  
  /* rotate in the fastest way */
  if(v_diff <= 180.0) {
    m_rotationAngle = m_rotationAngle - m_rotationSpeed + 360.0;
    int n = ((int)m_rotationAngle) /360;
    m_rotationAngle -= (((float)n)*360.0);
  } else {
    m_rotationAngle = m_rotationAngle + m_rotationSpeed + 360.0;
    int n = ((int)m_rotationAngle)/360;
    m_rotationAngle -= (((float)n)*360.0);
  }
  return m_rotationAngle;
}

Vector2i Camera::getDispBottomLeft() {
  return Vector2i(m_renderSurf.upperleft().x, m_renderSurf.downright().y);
}

Vector2i Camera::getDispTopLeft() {
  return m_renderSurf.upperleft();
}

void Camera::allowActiveZoom(bool i_value) {
  m_allowActiveZoom = i_value;

  initZoom();
}
