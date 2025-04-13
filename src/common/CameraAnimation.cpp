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

#include "CameraAnimation.h"
#include "helpers/RenderSurface.h"
#include "xmoto/Game.h"
#include "xmoto/Renderer.h"
#include "xmscene/Camera.h"
#include "xmscene/Scene.h"

#define PRESTART_ANIMATION_MARGIN_SIZE 5.0
#define PRESTART_ANIMATION_TIME 2.0
#define PRESTART_ANIMATION_CURVE 3.0
/* logf(PRESTART_ANIMATION_CURVE + 1.0) = 1.386294361*/
#define LOGF_PRE_ANIM_TIME_ADDED_ONE 1.386294361
#define INPLAY_ANIMATION_TIME 1.0
#define INPLAY_ANIMATION_SPEED 10
#define INPLAY_ANIMATION_MAX_OFFSET 0.5

CameraAnimation::CameraAnimation(Camera *i_camera,
                                 RenderSurface *i_screen,
                                 GameRenderer *i_renderer,
                                 Scene *i_motoGame) {
  m_step = 0;
  m_camera = i_camera;
  m_screen = i_screen;
  m_renderer = i_renderer;
  m_motoGame = i_motoGame;
  m_initialCameraZoom = 1.0;
  m_startTime = 0.0;
  m_allowNextStep = false;
}

CameraAnimation::~CameraAnimation() {}

float CameraAnimation::startTime() const {
  return m_startTime;
}

const Vector2f &CameraAnimation::initialPosition() const {
  return m_initialCameraPosition;
}

float CameraAnimation::initialZoom() const {
  return m_initialCameraZoom;
}

void CameraAnimation::init() {
  m_step = 0;
  m_initialCameraZoom = m_camera->getCurrentZoom();
  m_initialCameraPosition =
    Vector2f(m_camera->getCameraPositionX(), m_camera->getCameraPositionY());
  m_startTime = GameApp::getXMTime();
  m_allowNextStep = false;
  m_initialEntitiesToTakeZoom = m_renderer->SizeMultOfEntitiesToTake();
  m_initialEntitiesWhichMakeWin = m_renderer->SizeMultOfEntitiesWhichMakeWin();
}

void CameraAnimation::uninit() {
  m_camera->setAbsoluteZoom(m_initialCameraZoom);
  m_camera->setCameraPosition(m_initialCameraPosition.x,
                              m_initialCameraPosition.y);
  resetSizeMultipliers();
}

void CameraAnimation::resetSizeMultipliers() {
  m_renderer->setSizeMultOfEntitiesToTake(m_initialEntitiesToTakeZoom);
  m_renderer->setSizeMultOfEntitiesWhichMakeWin(m_initialEntitiesWhichMakeWin);
}

float CameraAnimation::initialEntitiesToTakeZoom() {
  return m_initialEntitiesToTakeZoom;
}

float CameraAnimation::initialEntitiesWhichMakeWinZoom() {
  return m_initialEntitiesWhichMakeWin;
}

bool CameraAnimation::step() {
  return true;
}

void CameraAnimation::goNextStep() {
  m_step++;
}

bool CameraAnimation::allowNextStep() {
  return m_allowNextStep;
}

// autozoom
AutoZoomCameraAnimation::AutoZoomCameraAnimation(Camera *i_camera,
                                                 RenderSurface *i_screen,
                                                 GameRenderer *i_renderer,
                                                 Scene *i_motoGame)
  : CameraAnimation(i_camera, i_screen, i_renderer, i_motoGame) {}

AutoZoomCameraAnimation::~AutoZoomCameraAnimation() {}

void AutoZoomCameraAnimation::init() {
  CameraAnimation::init();

  m_zoomX = (2.0 * ((float)m_screen->getDispWidth() /
                    (float)m_screen->getDispHeight())) /
            (m_motoGame->getLevelSrc()->RightLimit() -
             m_motoGame->getLevelSrc()->LeftLimit() +
             2 * PRESTART_ANIMATION_MARGIN_SIZE);
  m_zoomY = 2.0 / (m_motoGame->getLevelSrc()->TopLimit() -
                   m_motoGame->getLevelSrc()->BottomLimit() +
                   2 * PRESTART_ANIMATION_MARGIN_SIZE);

  if (m_zoomX > m_zoomY) {
    float visibleHeight, cameraStartHeight;

    m_zoomU = m_zoomX;

    visibleHeight = 2.0 / m_zoomU;
    cameraStartHeight = visibleHeight / 2.0;

    m_fPreCameraStart.x = (m_motoGame->getLevelSrc()->RightLimit() +
                           m_motoGame->getLevelSrc()->LeftLimit()) /
                          2;
    m_fPreCameraStart.y = m_motoGame->getLevelSrc()->TopLimit() -
                          cameraStartHeight + PRESTART_ANIMATION_MARGIN_SIZE;
    m_fPreCameraFinal.x = (m_motoGame->getLevelSrc()->RightLimit() +
                           m_motoGame->getLevelSrc()->LeftLimit()) /
                          2;
    m_fPreCameraFinal.y = m_motoGame->getLevelSrc()->BottomLimit() +
                          cameraStartHeight - PRESTART_ANIMATION_MARGIN_SIZE;

    if (fabs(m_fPreCameraStart.y - m_fPrePlayStartCamera.y) >
        fabs(m_fPreCameraFinal.y - m_fPrePlayStartCamera.y)) {
      float f;
      f = m_fPreCameraFinal.y;
      m_fPreCameraFinal.y = m_fPreCameraStart.y;
      m_fPreCameraStart.y = f;
    }

  } else {
    float visibleWidth, cameraStartLeft;

    m_zoomU = m_zoomY;

    visibleWidth = (2.0 * ((float)m_screen->getDispWidth() /
                           (float)m_screen->getDispHeight())) /
                   m_zoomU;
    cameraStartLeft = visibleWidth / 2.0;

    m_fPreCameraStart.x = m_motoGame->getLevelSrc()->RightLimit() -
                          cameraStartLeft + PRESTART_ANIMATION_MARGIN_SIZE;
    m_fPreCameraStart.y = (m_motoGame->getLevelSrc()->BottomLimit() +
                           m_motoGame->getLevelSrc()->TopLimit()) /
                          2;
    m_fPreCameraFinal.x = m_motoGame->getLevelSrc()->LeftLimit() +
                          cameraStartLeft - PRESTART_ANIMATION_MARGIN_SIZE;
    m_fPreCameraFinal.y = (m_motoGame->getLevelSrc()->BottomLimit() +
                           m_motoGame->getLevelSrc()->TopLimit()) /
                          2;

    if (fabs(m_fPreCameraStart.x - m_fPrePlayStartCamera.x) >
        fabs(m_fPreCameraFinal.x - m_fPrePlayStartCamera.x)) {
      float f;
      f = m_fPreCameraFinal.x;
      m_fPreCameraFinal.x = m_fPreCameraStart.x;
      m_fPreCameraStart.x = f;
    }
  }

  m_fAnimPlayStartZoom = m_camera->getCurrentZoom();
  m_fAnimPlayFinalZoom = m_zoomU;
  m_fAnimPlayFinalCamera1 = m_fPreCameraStart;
  m_fAnimPlayFinalCamera2 = m_fPreCameraFinal;
  m_fAnimPlayStartCamera.x = m_camera->getCameraPositionX();
  m_fAnimPlayStartCamera.y = m_camera->getCameraPositionY();
  m_step = 1;
  m_allowNextStep = true;
  m_entitiesToTakeZoom = m_renderer->SizeMultOfEntitiesToTake();
  m_entitiesWhichMakeWinZoom = m_renderer->SizeMultOfEntitiesWhichMakeWin();
  m_entitiesGrowing = true;
  m_previousZoomTime = -1.0;
}

void AutoZoomCameraAnimation::uninit() {
  CameraAnimation::uninit();
}

bool AutoZoomCameraAnimation::step() {
  if (CameraAnimation::step() == false) {
    return false;
  }

  float zx, zy, zz, coeff;

  switch (m_step) {
    case 1:
      // zooming
      if (GameApp::getXMTime() > startTime() + INPLAY_ANIMATION_TIME) {
        float zx, zy;
        zx = (m_fAnimPlayFinalCamera1.x - m_fAnimPlayFinalCamera2.x) *
             (sin((GameApp::getXMTime() - startTime() - INPLAY_ANIMATION_TIME) *
                    2 * 3.1415927 / INPLAY_ANIMATION_SPEED -
                  3.1415927 / 2) +
              1) /
             2;
        zy = (m_fAnimPlayFinalCamera1.y - m_fAnimPlayFinalCamera2.y) *
             (sin((GameApp::getXMTime() - startTime() - INPLAY_ANIMATION_TIME) *
                    2 * 3.1415927 / INPLAY_ANIMATION_SPEED -
                  3.1415927 / 2) +
              1) /
             2;

        // maximize speed
        // INPLAY_ANIMATION_MAX_OFFSET
        if ((m_fAnimPlayFinalCamera1.x - zx) - m_camera->getCameraPositionX() >
            INPLAY_ANIMATION_MAX_OFFSET) {
          zx = m_fAnimPlayFinalCamera1.x - m_camera->getCameraPositionX() -
               (INPLAY_ANIMATION_MAX_OFFSET);
        } else if ((m_fAnimPlayFinalCamera1.x - zx) -
                     m_camera->getCameraPositionX() <
                   -(INPLAY_ANIMATION_MAX_OFFSET)) {
          zx = m_fAnimPlayFinalCamera1.x - m_camera->getCameraPositionX() +
               INPLAY_ANIMATION_MAX_OFFSET;
        }
        if ((m_fAnimPlayFinalCamera1.y - zy) - m_camera->getCameraPositionY() >
            INPLAY_ANIMATION_MAX_OFFSET) {
          zy = m_fAnimPlayFinalCamera1.y - m_camera->getCameraPositionY() -
               (INPLAY_ANIMATION_MAX_OFFSET);
        } else if ((m_fAnimPlayFinalCamera1.y - zy) -
                     m_camera->getCameraPositionY() <
                   -(INPLAY_ANIMATION_MAX_OFFSET)) {
          zy = m_fAnimPlayFinalCamera1.y - m_camera->getCameraPositionY() +
               INPLAY_ANIMATION_MAX_OFFSET;
        }

        m_camera->setCameraPosition(m_fAnimPlayFinalCamera1.x - zx,
                                    m_fAnimPlayFinalCamera1.y - zy);

        if (m_previousZoomTime < 0.0) {
          m_previousZoomTime = GameApp::getXMTime();
        } else {
          if (GameApp::getXMTime() - m_previousZoomTime >
              0.03) { /* do it regularly */
            m_previousZoomTime = GameApp::getXMTime();

            if (m_entitiesGrowing) {
              if (m_entitiesToTakeZoom < initialZoom() / m_fAnimPlayFinalZoom) {
                m_entitiesToTakeZoom += initialZoom() - m_fAnimPlayFinalZoom;
                m_entitiesWhichMakeWinZoom +=
                  initialZoom() - m_fAnimPlayFinalZoom;
              } else {
                m_entitiesGrowing = false;
              }
            } else {
              if (m_entitiesToTakeZoom > initialEntitiesToTakeZoom()) {
                m_entitiesToTakeZoom -= initialZoom() - m_fAnimPlayFinalZoom;
                m_entitiesWhichMakeWinZoom -=
                  initialZoom() - m_fAnimPlayFinalZoom;
              } else {
                m_entitiesGrowing = true;
              }
            }
            m_renderer->setSizeMultOfEntitiesToTake(m_entitiesToTakeZoom);
            m_renderer->setSizeMultOfEntitiesWhichMakeWin(
              m_entitiesWhichMakeWinZoom);
          }
        }

        return true;
      }

      coeff = (GameApp::getXMTime() - startTime()) / (INPLAY_ANIMATION_TIME);
      zx = coeff * (m_fAnimPlayStartCamera.x - m_fAnimPlayFinalCamera1.x);
      zy = coeff * (m_fAnimPlayStartCamera.y - m_fAnimPlayFinalCamera1.y);
      zz = coeff * (m_fAnimPlayStartZoom - m_fAnimPlayFinalZoom);

      m_camera->setAbsoluteZoom(m_fAnimPlayStartZoom - zz);
      m_camera->setCameraPosition(m_fAnimPlayStartCamera.x - zx,
                                  m_fAnimPlayStartCamera.y - zy);

      return true;
      break;

    case 2:
      // initialize unzoom
      m_startTimeUnzooming = GameApp::getXMTime();
      m_fAnimPlayStartZoom = m_camera->getCurrentZoom();
      m_fAnimPlayFinalZoom = initialZoom();
      m_fAnimPlayStartCamera.x = m_camera->getCameraPositionX();
      m_fAnimPlayStartCamera.y = m_camera->getCameraPositionY();
      m_fAnimPlayFinalCamera1 = initialPosition();
      m_step = 3;
      break;

    case 3:
      // unzoom
      if (GameApp::getXMTime() > m_startTimeUnzooming + INPLAY_ANIMATION_TIME) {
        return false;
      }

      coeff =
        (GameApp::getXMTime() - m_startTimeUnzooming) / (INPLAY_ANIMATION_TIME);
      zx = coeff * (m_fAnimPlayStartCamera.x - m_fAnimPlayFinalCamera1.x);
      zy = coeff * (m_fAnimPlayStartCamera.y - m_fAnimPlayFinalCamera1.y);
      zz = coeff * (m_fAnimPlayStartZoom - m_fAnimPlayFinalZoom);

      m_camera->setAbsoluteZoom(m_fAnimPlayStartZoom - zz);
      m_camera->setCameraPosition(m_fAnimPlayStartCamera.x - zx,
                                  m_fAnimPlayStartCamera.y - zy);

      if (m_entitiesWhichMakeWinZoom > initialEntitiesWhichMakeWinZoom()) {
        m_entitiesWhichMakeWinZoom -= 0.2;
        m_renderer->setSizeMultOfEntitiesWhichMakeWin(
          m_entitiesWhichMakeWinZoom);
      }

      if (m_entitiesToTakeZoom > initialEntitiesToTakeZoom()) {
        m_entitiesToTakeZoom -= 0.2;
        m_renderer->setSizeMultOfEntitiesToTake(m_entitiesToTakeZoom);
      }

      return true;

      break;
  }

  return true;
}

void AutoZoomCameraAnimation::goNextStep() {
  CameraAnimation::goNextStep();
  m_allowNextStep = m_step == 1;
}

// zooming
ZoomingCameraAnimation::ZoomingCameraAnimation(Camera *i_camera,
                                               RenderSurface *i_screen,
                                               GameRenderer *i_renderer,
                                               Scene *i_motoGame)
  : CameraAnimation(i_camera, i_screen, i_renderer, i_motoGame) {}

ZoomingCameraAnimation::~ZoomingCameraAnimation() {}

void ZoomingCameraAnimation::init() {
  CameraAnimation::init();

  m_zoomX = (2.0 * ((float)m_screen->getDispWidth() /
                    (float)m_screen->getDispHeight())) /
            (m_motoGame->getLevelSrc()->RightLimit() -
             m_motoGame->getLevelSrc()->LeftLimit() +
             2 * PRESTART_ANIMATION_MARGIN_SIZE);
  m_zoomY = 2.0 / (m_motoGame->getLevelSrc()->TopLimit() -
                   m_motoGame->getLevelSrc()->BottomLimit() +
                   2 * PRESTART_ANIMATION_MARGIN_SIZE);

  if (m_zoomX > m_zoomY) {
    float visibleHeight, cameraStartHeight;

    m_zoomU = m_zoomX;
    m_static_time = (m_motoGame->getLevelSrc()->TopLimit() -
                     m_motoGame->getLevelSrc()->BottomLimit()) /
                    (2.0 / m_zoomU);

    visibleHeight = 2.0 / m_zoomU;
    cameraStartHeight = visibleHeight / 2.0;

    m_fPreCameraStart.x = (m_motoGame->getLevelSrc()->RightLimit() +
                           m_motoGame->getLevelSrc()->LeftLimit()) /
                          2;
    m_fPreCameraStart.y = m_motoGame->getLevelSrc()->TopLimit() -
                          cameraStartHeight + PRESTART_ANIMATION_MARGIN_SIZE;
    m_fPreCameraFinal.x = (m_motoGame->getLevelSrc()->RightLimit() +
                           m_motoGame->getLevelSrc()->LeftLimit()) /
                          2;
    m_fPreCameraFinal.y = m_motoGame->getLevelSrc()->BottomLimit() +
                          cameraStartHeight - PRESTART_ANIMATION_MARGIN_SIZE;

    if (fabs(m_fPreCameraStart.y - initialPosition().y) >
        fabs(m_fPreCameraFinal.y - initialPosition().y)) {
      float f;
      f = m_fPreCameraFinal.y;
      m_fPreCameraFinal.y = m_fPreCameraStart.y;
      m_fPreCameraStart.y = f;
    }

  } else {
    float visibleWidth, cameraStartLeft;

    m_zoomU = m_zoomY;
    m_static_time = (m_motoGame->getLevelSrc()->RightLimit() -
                     m_motoGame->getLevelSrc()->LeftLimit()) /
                    ((2.0 * ((float)m_screen->getDispWidth() /
                             (float)m_screen->getDispHeight())) /
                     m_zoomU);

    visibleWidth = (2.0 * ((float)m_screen->getDispWidth() /
                           (float)m_screen->getDispHeight())) /
                   m_zoomU;
    cameraStartLeft = visibleWidth / 2.0;

    m_fPreCameraStart.x = m_motoGame->getLevelSrc()->RightLimit() -
                          cameraStartLeft + PRESTART_ANIMATION_MARGIN_SIZE;
    m_fPreCameraStart.y = (m_motoGame->getLevelSrc()->BottomLimit() +
                           m_motoGame->getLevelSrc()->TopLimit()) /
                          2;
    m_fPreCameraFinal.x = m_motoGame->getLevelSrc()->LeftLimit() +
                          cameraStartLeft - PRESTART_ANIMATION_MARGIN_SIZE;
    m_fPreCameraFinal.y = (m_motoGame->getLevelSrc()->BottomLimit() +
                           m_motoGame->getLevelSrc()->TopLimit()) /
                          2;

    if (fabs(m_fPreCameraStart.x - initialPosition().x) >
        fabs(m_fPreCameraFinal.x - initialPosition().x)) {
      float f;
      f = m_fPreCameraFinal.x;
      m_fPreCameraFinal.x = m_fPreCameraStart.x;
      m_fPreCameraStart.x = f;
    }
  }
}

bool ZoomingCameraAnimation::step() {
  if (CameraAnimation::step() == false) {
    return false;
  }

  if (GameApp::getXMTime() >
      startTime() + m_static_time + PRESTART_ANIMATION_TIME) {
    return false;
  }
  if (GameApp::getXMTime() > startTime() + m_static_time) {
    float zx, zy, zz;

    zz = logf(PRESTART_ANIMATION_CURVE *
                ((PRESTART_ANIMATION_TIME + m_static_time -
                  GameApp::getXMTime() + startTime()) /
                 (PRESTART_ANIMATION_TIME)) +
              1.0) /
         LOGF_PRE_ANIM_TIME_ADDED_ONE * (initialZoom() - m_zoomU);

    m_camera->setAbsoluteZoom(initialZoom() - zz);

    zx = (PRESTART_ANIMATION_TIME + m_static_time - GameApp::getXMTime() +
          startTime()) /
         (PRESTART_ANIMATION_TIME) *
         (initialPosition().x - m_fPrePlayCameraLast.x);
    zy = (PRESTART_ANIMATION_TIME + m_static_time - GameApp::getXMTime() +
          startTime()) /
         (PRESTART_ANIMATION_TIME) *
         (initialPosition().y - m_fPrePlayCameraLast.y);

    m_camera->setCameraPosition(initialPosition().x - zx,
                                initialPosition().y - zy);
  } else {
    float zx, zy;

    m_camera->setAbsoluteZoom(m_zoomU);

    zx = (m_static_time - GameApp::getXMTime() + startTime()) /
         (m_static_time) * (m_fPreCameraStart.x - m_fPreCameraFinal.x);

    zy = (m_static_time - GameApp::getXMTime() + startTime()) /
         (m_static_time) * (m_fPreCameraStart.y - m_fPreCameraFinal.y);

    m_camera->setCameraPosition(m_fPreCameraStart.x - zx,
                                m_fPreCameraStart.y - zy);

    m_fPrePlayCameraLast.x = m_fPreCameraStart.x - zx;
    m_fPrePlayCameraLast.y = m_fPreCameraStart.y - zy;
  }
  return true;
}
