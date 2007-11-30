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

#ifndef __STATESCENE_H__
#define __STATESCENE_H__

#include "StateManager.h"

class StateScene : public GameState {
 public:
 StateScene(GameApp* pGame, bool i_doShade = false, bool i_doShadeAnim = false);
 virtual ~StateScene();
 
 virtual void enter();
 virtual void leave();
 /* called when a new state is pushed or poped on top of the
    current one*/
 virtual void enterAfterPop();
 virtual void leaveAfterPush();
 
 virtual bool update();
 virtual bool render();
 /* input */
 virtual void keyDown(int nKey, SDLMod mod,int nChar);
 virtual void keyUp(int nKey,   SDLMod mod);
 virtual void mouseDown(int nButton);
 virtual void mouseDoubleClick(int nButton);
 virtual void mouseUp(int nButton);

 virtual void send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input);
 virtual void send(const std::string& i_id, const std::string& i_message);

 protected:
 virtual void executeOneCommand(std::string cmd);

 double m_fLastPhysTime; /* When the last frama was initiated */
 bool   m_isLockedScene;
 bool   m_autoZoom;      /* true : the key is pressed so that it zooms out to see the level */
 int    m_autoZoomStep;

 void setScoresTimes();
 void restartLevel(bool i_reloadLevel = false);
 void nextLevel(bool i_positifOrder = true);

 void closePlaying();
 void abortPlaying();

 bool isLockedScene() const;
 void lockScene(bool i_value);
 void setAutoZoom(bool i_value);
 bool autoZoom() const;
 void runAutoZoom();
 int autoZoomStep() const;
 void setAutoZoomStep(int n);

 float m_fPrePlayStartTime;
 float m_fPrePlayStartInitZoom;
 float m_fPrePlayStartCameraX;
 float m_fPrePlayStartCameraY;
 float m_fPreCameraStartX;
 float m_fPreCameraStartY;
 float m_fPreCameraFinalX;
 float m_fPreCameraFinalY;
 float m_zoomX;
 float m_zoomY;
 float m_zoomU;
 float static_time;
 float fAnimPlayStartZoom;
 float fAnimPlayStartCameraX; 
 float fAnimPlayStartCameraY;
 float fAnimPlayFinalZoom;
 float fAnimPlayFinalCameraX1;
 float fAnimPlayFinalCameraY1;
 float fAnimPlayFinalCameraX2;
 float fAnimPlayFinalCameraY2;

 void zoomAnimation1_init();
 void zoomAnimation2_init();
 bool zoomAnimation2_step();
 void zoomAnimation2_init_unzoom();
 bool zoomAnimation2_unstep();
};

#endif
