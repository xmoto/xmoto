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
#include "GhostTrail.h"

struct CameraGhostVisibility {
  bool isIn;
  float lastIn;
};

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
  Camera(Vector2i upperleft, Vector2i downright);
  ~Camera();
  void  setAbsoluteZoom(float i_absZoom);
  void  setRelativeZoom(float i_relZoom);
  void  desactiveActionZoom();
  void  initActiveZoom();
  void  initZoom();
  float getCurrentZoom();
  void  moveCamera(float px, float py);
  void  setCameraPosition(float px, float py);
  float getCameraPositionX();
  float getCameraPositionY();
  void  initCamera();
  void  initCameraPosition();

  void  prepareForNewLevel();

  void  setScroll(bool isSmooth, const Vector2f& gravity);
  void  guessDesiredCameraPosition(float &p_fDesiredHorizontalScrollShift,
				   float &p_fDesiredVerticalScrollShift,
				   const Vector2f& gravity);
  void  guessDesiredCameraZoom();
  float guessDesiredAngleRotation();
  void  setPlayerToFollow(Biker* i_playerToFollow);
  Biker* getPlayerToFollow();

  void  setCamera2d();
  void  setCamera3d();
  void  render();
  void  setRenderSurface(Vector2i upperleft, Vector2i downright);

  int   getDispWidth(){
    return m_renderSurf.size().x;
  }
  int getDispHeight(){
    return m_renderSurf.size().y;
  }
  Vector2i getDispBottomLeft();
  Vector2i getDispTopLeft();

  bool isMirrored();
  void setMirrored(bool i_value);

  void setPlayerDead(); // inform that the player dies

  float rotationAngle();
  void setRotationAngle(float i_value);
  void setRotationSpeed(float i_value);
  void setDesiredRotationAngle(float i_value);
  void adaptRotationAngleToGravity(Vector2f& gravity);

  void allowActiveZoom(bool i_value);
  void allowTrailCam(bool i_value);

  // assume that a ghost added is never removed and do not change of id (which is the case)
  void clearGhostsVisibility();
  void setGhostIn(unsigned int i);
  void setGhostOut(unsigned int i);
  bool isGhostIn(unsigned int i);
  float getGhostLastIn(unsigned int i);

  // trail cam stuff
  void initTrailCam(Scene* i_scene);
  Vector2f updateTrailCam();
  void trailCamTrackingShot();
  void setActivateTrackingShot(bool i_shot) {m_trackingShotActivated = i_shot;};
  void toggleTrackingShot(Scene* i_scene);
  void toggleTrailCam();
  void setUseTrailCam(bool i_value) { m_useTrailCam = i_value; };
  unsigned int locateNearestPointOnInterpolatedTrail(bool i_optimize);
  void checkIfNearTrail();                           //updates m_catchTrail
  bool getTrailAvailable() { return m_trailAvailable; };
  Vector2f getTrailCamAimPos();
  float m_previousTSStepTime;
  int m_trailCamForwardSteps;
  
private:
  bool  m_mirrored;
  float m_rotationAngle;
  float m_rotationSpeed;
  float m_desiredRotationAngle;

  std::vector<CameraGhostVisibility> m_ghostVisibility;
  void checkGhostVisibilityExists(unsigned int i);

  float m_fScale;
  // for not zooming in and out all the time
  unsigned int m_timeTresholdIn;
  unsigned int m_timeTresholdOut;
  typedef enum {none, zoomIn, zoomOut} activeZoom;
  activeZoom m_currentActiveZoom;
  bool  m_useActiveZoom; // enable/disable activ zoom for this camera ; some actions disable it (like user zooming, script zooming)
  float m_initialZoom;
  bool m_allowActiveZoom; // globally enable active zoom feature for this camera

  // trail cam stuff  
  GhostTrail* m_ghostTrail;
  unsigned int m_currentNearestTrailDataPosition;
  bool m_useTrailCam;
  bool m_trackingShotActivated;
  bool m_catchTrail; // set true if cam is near the ghost trail
  bool m_trailAvailable;
  float m_trackShotIndex; // current step on ghost trail
  unsigned int m_trackShotStartIndex;
  
  // basic cam control
  float m_cameraOffsetX;
  float m_cameraOffsetY;
  Biker* m_playerToFollow;

  Vector2f m_Scroll;
  float m_fCurrentHorizontalScrollShift;
  float m_fCurrentVerticalScrollShift;
  /* to move camera faster if the dir changed */
  DriveDir m_previous_driver_dir;
  bool  m_recenter_camera_fast;

  RenderSurface m_renderSurf;

  float m_lastSpeedTime;
  Vector2f m_cameraDeathOffset;

  void resetActiveZoom();
};

#endif
