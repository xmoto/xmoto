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

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "../VCommon.h"
#include "../VApp.h"
#include "Bike.h"

#define ZOOM_DEFAULT 0.195f
#define CAMERA_OFFSETX_DEFAULT 0.0
#define CAMERA_OFFSETY_DEFAULT 0.0

class RenderSurface {
public:
	void update(Vector2d upperleft, Vector2d downright){
		m_upperleft = upperleft;
		m_downright = downright;
		m_size      = downright - upperleft;
		vapp::Log("update::upperleft:: x: %d y: %d", m_upperleft.x, m_upperleft.y);
		vapp::Log("update::downright:: x: %d y: %d", m_downright.x, m_downright.y);
		vapp::Log("update::size:: x: %d y: %d", m_size.x, m_size.y);
	}
	Vector2d& size(){
		return m_size;
	}
	Vector2d& upperleft(){
		return m_upperleft;
	}
	Vector2d& downright(){
		return m_downright;
	}

private:
	Vector2d m_upperleft;
	Vector2d m_downright;
	Vector2d m_size;
};

class Camera {
public:
  Camera(Vector2d upperleft, Vector2d downright){
		m_fSpeedMultiply = 1.0f;
    m_fScale         = ZOOM_DEFAULT;
    m_cameraOffsetX  = CAMERA_OFFSETX_DEFAULT;
    m_cameraOffsetY  = CAMERA_OFFSETY_DEFAULT;
    m_playerToFollow = NULL;
		vapp::Log("Camera::upperleft.x: %d, upperleft.y: %d, downright.x: %d, downright.y: %d",
							upperleft.x, upperleft.y, downright.x, downright.y);
		setRenderSurface(upperleft, downright);
  }
  void zoom(float p_f);
  void setZoom(float p_f);
  void initZoom();
  float getCurrentZoom();
  void moveCamera(float px, float py);
  void setCameraPosition(float px, float py);
  float getCameraPositionX();
  float getCameraPositionY();
  void initCamera();
  void initCameraPosition();

  void prepareForNewLevel();

	void setSpeedMultiplier(float f) {m_fSpeedMultiply = f;}
  void setScroll(bool isSmooth, const Vector2f& gravity);
  void guessDesiredCameraPosition(float &p_fDesiredHorizontalScrollShift,
																	float &p_fDesiredVerticalScrollShift,
																	const Vector2f& gravity);
  void setPlayerToFollow(Biker* i_playerToFollow);
  Biker* getPlayerToFollow();

	void setCamera2d();
	void setCamera3d();
	void render();
	void setRenderSurface(Vector2d upperleft, Vector2d downright);

	int getDispWidth(){
		return m_renderSurf.size().x;
	}
	int getDispHeight(){
		return m_renderSurf.size().y;
	}

private:
  float m_fScale;
  float m_cameraOffsetX;
  float m_cameraOffsetY;
  Biker* m_playerToFollow;

	float m_fSpeedMultiply;
  Vector2f m_Scroll;
  float m_fCurrentHorizontalScrollShift;
  float m_fCurrentVerticalScrollShift;
  DriveDir m_previous_driver_dir; /* to move camera faster if the dir changed */
  bool  m_recenter_camera_fast;

	RenderSurface m_renderSurf;
};

#endif
