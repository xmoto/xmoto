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

#ifndef __CAMERAANIMATION_H__
#define __CAMERAANIMATION_H__

#include "helpers/VMath.h"

class Camera;
class RenderSurface;
class Scene;
class GameRenderer;

class CameraAnimation {
  public:
  CameraAnimation(Camera* i_camera, RenderSurface* i_screen, GameRenderer* i_renderer, Scene* i_motoGame);
  virtual ~CameraAnimation();

  virtual void init();
  virtual void uninit();

  virtual bool step();
  bool allowNextStep();
  virtual void goNextStep();

  float startTime() const;
  const Vector2f& initialPosition() const;
  float initialZoom() const;

  float initialEntitiesToTakeZoom();
  float initialEntitiesWhichMakeWinZoom();

 protected:
  Camera*   m_camera;
  RenderSurface*  m_screen;
  GameRenderer* m_renderer;
  Scene* m_motoGame;
  int       m_step;
  bool      m_allowNextStep;

 private:
  // initial values
  float    m_I_cameraZoom;
  Vector2f m_I_cameraPosition;
  float    m_I_entitiesToTakeZoom;
  float    m_I_entitiesWhichMakeWin;

  // usable values
  float m_startTime;
};

class AutoZoomCameraAnimation : public CameraAnimation {
 public:
  AutoZoomCameraAnimation(Camera* i_camera, RenderSurface* i_screen, GameRenderer* i_renderer, Scene* i_motoGame);
  ~AutoZoomCameraAnimation();

  virtual void init();
  virtual void uninit();

  virtual bool step();
  virtual void goNextStep();

 private:
  float    m_zoomX, m_zoomY, m_zoomU;
  float    m_fAnimPlayStartZoom;
  float    m_fAnimPlayFinalZoom;
  Vector2f m_fPreCameraStart;
  Vector2f m_fPreCameraFinal;
  Vector2f m_fAnimPlayFinalCamera1;
  Vector2f m_fAnimPlayFinalCamera2;
  Vector2f m_fAnimPlayStartCamera;
  Vector2f m_fPrePlayStartCamera;
  float    m_startTimeUnzooming;
  float    m_entitiesToTakeZoom;
  float    m_entitiesWhichMakeWinZoom;
  bool     m_entitiesGrowing;
  float    m_previousZoomTime;
};

class ZoomingCameraAnimation : public CameraAnimation {
 public:
  ZoomingCameraAnimation(Camera* i_camera, RenderSurface* i_screen, GameRenderer* i_renderer, Scene* i_motoGame);
  ~ZoomingCameraAnimation();

  virtual void init();
  virtual bool step();

 private:
  float    m_zoomX, m_zoomY, m_zoomU;
  float    m_static_time;
  Vector2f m_fPreCameraStart;
  Vector2f m_fPreCameraFinal;
  Vector2f m_fPrePlayCameraLast;
};

#endif
