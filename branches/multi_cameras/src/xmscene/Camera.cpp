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

void Camera::prepareForNewLevel() {
  m_fCurrentHorizontalScrollShift = 0.0f;
  m_fCurrentVerticalScrollShift = 0.0f;
  m_previous_driver_dir  = DD_LEFT;    
  m_recenter_camera_fast = true;
	vapp::Log("Camera::prepareForNewLevel");
  setScroll(false, Vector2f(0, -9.81));
}

void Camera::zoom(float p_f) {
  m_fScale += p_f;
  if(m_fScale < 0) {
    m_fScale = 0;
  }
}

void Camera::initZoom() {
  m_fScale = ZOOM_DEFAULT;
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

  float normal_hoffset = 4.0;
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

void Camera::setScroll(bool isSmooth, const Vector2f& gravity) {
  float v_move_camera_max;
  float v_fDesiredHorizontalScrollShift = 0.0;
  float v_fDesiredVerticalScrollShift   = 0.0;

	vapp::Log("Camera::setScroll curScrollShift H:%f V:%f",
						m_fCurrentHorizontalScrollShift,
						m_fCurrentVerticalScrollShift);

  if(m_playerToFollow == NULL) {
		vapp::Log("Camera::setScroll Player to follow is NULL");
		return;
  }

  /* determine if the camera must move fastly */
  /* it must go faster if the player change of sense */
  if(m_previous_driver_dir != m_playerToFollow->getState()->Dir) {
    m_recenter_camera_fast = true;
  }
  m_previous_driver_dir = m_playerToFollow->getState()->Dir;

  if(m_recenter_camera_fast) {
    v_move_camera_max = 0.1;
  } else {
    v_move_camera_max = 0.01;
  }

  /* Determine scroll */
  m_Scroll = -m_playerToFollow->getState()->CenterP;

	vapp::Log("Camera::setScroll playerCenterP: %f,%f -- m_Scroll: %f,%f",
						-m_playerToFollow->getState()->CenterP.x,
						-m_playerToFollow->getState()->CenterP.y,
						m_Scroll.x, m_Scroll.y);

  /* Driving direction? */
  guessDesiredCameraPosition(v_fDesiredHorizontalScrollShift,
														 v_fDesiredVerticalScrollShift,
														 gravity);
    
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
      m_fCurrentHorizontalScrollShift -= v_move_camera_max * m_fSpeedMultiply;
    }
    else if(d > 0.0f) {
      m_fCurrentHorizontalScrollShift += v_move_camera_max * m_fSpeedMultiply;
    }
  }
    
  if(v_fDesiredVerticalScrollShift != m_fCurrentVerticalScrollShift) {
    float d = v_fDesiredVerticalScrollShift - m_fCurrentVerticalScrollShift;
    if(fabs(d)<0.01f || isSmooth == false) {
      m_fCurrentVerticalScrollShift = v_fDesiredVerticalScrollShift;
    }
    else if(d < 0.0f) {
      m_fCurrentVerticalScrollShift -= 0.01f * m_fSpeedMultiply;
    }
    else if(d > 0.0f) {
      m_fCurrentVerticalScrollShift += 0.01f * m_fSpeedMultiply;
    }
  }
    
  m_Scroll += Vector2f(m_fCurrentHorizontalScrollShift,
											 m_fCurrentVerticalScrollShift);

	vapp::Log("Camera::setScroll scale: %f camoffX: %f camoffY: %f speedmul: %f scro.x: %f scro.y: %f curHscro: %f curVscro: %f recfast: %d",
						m_fScale, m_cameraOffsetX, m_cameraOffsetY, m_fSpeedMultiply, m_Scroll.x, m_Scroll.y, m_fCurrentHorizontalScrollShift,
						m_fCurrentVerticalScrollShift, m_recenter_camera_fast);
}

float Camera::getCurrentZoom() {
  return m_fScale;
}

void Camera::setZoom(float p_f) {
  m_fScale = p_f;
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
	vapp::Log("Camera::setCamera2d scale: %f camoffX: %f camoffY: %f speedmul: %f scro.x: %f scro.y: %f curHscro: %f curVscro: %f recfast: %d",
						m_fScale, m_cameraOffsetX, m_cameraOffsetY, m_fSpeedMultiply, m_Scroll.x, m_Scroll.y, m_fCurrentHorizontalScrollShift,
						m_fCurrentVerticalScrollShift, m_recenter_camera_fast);
}

void Camera::setCamera3d(){
#ifdef ENABLE_OPENGL
	glViewport(m_renderSurf.upperleft().x, m_renderSurf.upperleft().y,
						 m_renderSurf.size().x,      m_renderSurf.size().y);						 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
#endif
	vapp::Log("Camera::setCamera3d scale: %f camoffX: %f camoffY: %f speedmul: %f scro.x: %f scro.y: %f curHscro: %f curVscro: %f recfast: %d",
						m_fScale, m_cameraOffsetX, m_cameraOffsetY, m_fSpeedMultiply, m_Scroll.x, m_Scroll.y, m_fCurrentHorizontalScrollShift,
						m_fCurrentVerticalScrollShift, m_recenter_camera_fast);
}

void Camera::setRenderSurface(Vector2d upperleft, Vector2d downright){
	vapp::Log("setRenderSurface::upperleft.x: %d, upperleft.y: %d, downright.x: %d, downright.y: %d",
						upperleft.x, upperleft.y, downright.x, downright.y);
	m_renderSurf.update(upperleft, downright);
}
