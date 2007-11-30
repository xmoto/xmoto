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
#include "Bike.h"

#define ZOOM_DEFAULT 0.195f
#define CAMERA_OFFSETX_DEFAULT 0.0
#define CAMERA_OFFSETY_DEFAULT 0.0

class RenderSurface {
public:
  void update(Vector2i upperleft, Vector2i downright){
    m_upperleft = upperleft;
    m_downright = downright;
    m_size      = downright - upperleft;
  }
  Vector2i& size(){
    return m_size;
  }
  Vector2i& upperleft(){
    return m_upperleft;
  }
  Vector2i& downright(){
    return m_downright;
  }

private:
  Vector2i m_upperleft;
  Vector2i m_downright;
  Vector2i m_size;
};

class Camera {
public:
  Camera(Vector2i upperleft, Vector2i downright){
    m_fScale         = ZOOM_DEFAULT;
    m_cameraOffsetX  = CAMERA_OFFSETX_DEFAULT;
    m_cameraOffsetY  = CAMERA_OFFSETY_DEFAULT;
    m_playerToFollow = NULL;
    setRenderSurface(upperleft, downright);
    m_mirrored = false;
    m_rotationAngle = 0.0;
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

  void setScroll(bool isSmooth, const Vector2f& gravity);
  void guessDesiredCameraPosition(float &p_fDesiredHorizontalScrollShift,
				  float &p_fDesiredVerticalScrollShift,
				  const Vector2f& gravity);
  float guessDesiredAngleRotation();
  void setPlayerToFollow(Biker* i_playerToFollow);
  Biker* getPlayerToFollow();

  void setCamera2d();
  void setCamera3d();
  void render();
  void setRenderSurface(Vector2i upperleft, Vector2i downright);

  int getDispWidth(){
    return m_renderSurf.size().x;
  }
  int getDispHeight(){
    return m_renderSurf.size().y;
  }
  Vector2i getDispBottomLeft();

  bool isMirrored();
  void setMirrored(bool i_value);

  float rotationAngle();
  void setRotationAngle(float i_value);
  void setDesiredRotationAngle(float i_value);
  void adaptRotationAngleToGravity(Vector2f& gravity);

private:
  bool m_mirrored;
  float m_rotationAngle;
  float m_desiredRotationAngle;

  float m_fScale;
  float m_cameraOffsetX;
  float m_cameraOffsetY;
  Biker* m_playerToFollow;

  Vector2f m_Scroll;
  float m_fCurrentHorizontalScrollShift;
  float m_fCurrentVerticalScrollShift;
  DriveDir m_previous_driver_dir; /* to move camera faster if the dir changed */
  bool  m_recenter_camera_fast;

  RenderSurface m_renderSurf;
};

#endif
