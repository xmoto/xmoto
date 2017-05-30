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

#include "GameState.h"

class CameraAnimation;
class Universe;
class GameRenderer;

class StateScene : public GameState {
public:
  StateScene(bool i_doShade = false, bool i_doShadeAnim = false);
  StateScene(
    Universe *i_universe,
    GameRenderer *i_renderer,
    bool i_doShade = false,
    bool i_doShadeAnim = false); // for state which doesn't create the universe
  virtual ~StateScene();

  virtual void enter();
  virtual void enterAfterPop();
  virtual void leaveAfterPush();
  virtual void leaveType();

  virtual bool update();
  virtual bool render();
  virtual void onRenderFlush();

  /* input */
  virtual void xmKey(InputEventType i_type, const XMKey &i_xmkey);

  virtual void sendFromMessageBox(const std::string &i_id,
                                  UIMsgBoxButton i_button,
                                  const std::string &i_input);

  virtual void restartLevel(bool i_reloadLevel = false);
  virtual void nextLevel(bool i_positifOrder = true);

protected:
  virtual void executeOneCommand(std::string cmd, std::string args);
  void makeStatsStr();

  double m_fLastPhysTime; /* When the last frama was initiated */
  bool m_isLockedScene;
  bool m_autoZoom; /* true : the key is pressed so that it zooms out to see the
                      level */

  void setScoresTimes();

  void restartLevelToPlay(bool i_reloadLevel = false);
  void nextLevelToPlay(bool i_positifOrder = true);

  void closePlaying();
  virtual void abortPlaying();

  bool isLockedScene() const;
  void lockScene(bool i_value);
  void setAutoZoom(bool i_value);
  bool autoZoom() const;
  void runAutoZoom();

  void displayStats();

  // next level when you play levels (it can be different, replaying for
  // exemple)
  void playingNextLevel(bool i_positifOrder);

  /* animation */
  CameraAnimation *m_cameraAnim;

  Universe *m_universe;
  GameRenderer *m_renderer;

  int m_benchmarkNbFrame;
  float m_benchmarkStartTime;

  /* stats to display */
  std::string m_statsStr;
  float m_difficulty, m_quality;
  Texture *m_uncheckedTex, *m_qualityTex, *m_difficultyTex;

  void playLevelMusic();
  void playToCheckpoint();

private:
  void init(bool i_doShade, bool i_doShadeAnim);
  void initMessageRegistering();
  bool m_trackingShotMode;

  // shade
  bool m_doShade;
  bool m_doShadeAnim;
};

#endif
