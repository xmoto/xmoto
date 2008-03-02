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

class CameraAnimation;
class Universe;

class StateScene : public GameState {
 public:
 StateScene(bool i_doShade = false, bool i_doShadeAnim = false);
 StateScene(Universe* i_universe, bool i_doShade = false, bool i_doShadeAnim = false); // for state which doesn't create the universe
 virtual ~StateScene();
 
 virtual void enter();
 
 virtual bool update();
 virtual bool render();
 virtual void onRenderFlush();

 /* input */
 virtual void keyUp(int nKey, SDLMod mod);
 virtual void keyDown(int nKey, SDLMod mod,int nChar);

 virtual void send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input);
 virtual void send(const std::string& i_id, const std::string& i_message);

 protected:
 virtual void executeOneCommand(std::string cmd);
 void makeStatsStr();

 double m_fLastPhysTime; /* When the last frama was initiated */
 bool   m_isLockedScene;
 bool   m_autoZoom;      /* true : the key is pressed so that it zooms out to see the level */

 void setScoresTimes();
 virtual void restartLevel(bool i_reloadLevel = false);
 virtual void nextLevel(bool i_positifOrder = true);

 void closePlaying();
 virtual void abortPlaying();

 bool isLockedScene() const;
 void lockScene(bool i_value);
 void setAutoZoom(bool i_value);
 bool autoZoom() const;
 void runAutoZoom();

 void displayStats();

 /* animation */
 CameraAnimation* m_cameraAnim;

 Universe* m_universe;

 int m_benchmarkNbFrame;
 float m_benchmarkStartTime;

 /* stats to display */
 std::string m_statsStr;
 
};

#endif
