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

#ifndef __STATEMANAGER_H__
#define __STATEMANAGER_H__

#include "common/VCommon.h"
#include "helpers/RenderSurface.h"
#include "helpers/Singleton.h"
#include "include/xm_SDL.h"
#include "xmoto/input/XMKey.h"
#include <map>
#include <string>
#include <vector>

class VideoRecorder;
class GameState;
class Texture;
class RenderSurface;
class XMThreadStats;
class DownloadReplaysThread;

class StateManager : public Singleton<StateManager> {
  friend class Singleton<StateManager>;

private:
  StateManager();
  ~StateManager();

public:
  std::string getUniqueId();

  void pushState(GameState *pNewState);
  /* replace only the last state with id = i_parentId */
  void replaceState(GameState *pNewState, const std::string &i_parentId);

  // after some events, a state can requestForEnd
  // -> return NULL or the state which requested to be ended
  void flush();

  void update();
  void render();
  // input
  void xmKey(InputEventType i_type, const XMKey &i_xmkey);

  void fileDrop(const std::string &path);

  void changeFocus(bool i_hasFocus);
  void changeVisibility(bool i_visible);
  void setInvalidated(bool i_isInvalidated) {
    m_isInvalidated = i_isInvalidated;
  }

  bool hasFocus() const { return m_hasFocus; }
  bool isInvalidated() const { return m_isInvalidated; }

  bool needUpdateOrRender();

  // to display on the screen
  int getCurrentUpdateFPS();
  int getCurrentRenderFPS();

  // in order to receive a message, you have to first register
  // yourself as an observer of this message
  void registerAsObserver(const std::string &message, GameState *self);
  void unregisterAsObserver(const std::string &message, GameState *self);
  // register as emitter only for debug information
  void registerAsEmitter(const std::string &message);

  // send the message to registered states
  void sendSynchronousMessage(const std::string &message,
                              const std::string &args = "",
                              const std::string &i_parentId = "");
  void sendAsynchronousMessage(const std::string &message,
                               const std::string &args = "",
                               const std::string &i_parentId = "");

  GameState *getTopState();
  bool isTopOfTheStates(GameState *i_state);
  int numberOfStates();

  int getMaxFps() { return m_maxFps; }

  // video recorder
  VideoRecorder *getVideoRecorder();

  // ask to states to clean themself
  static void cleanStates();
  static void refreshStaticCaptions();

  bool isThereASuchState(const std::string &i_name);
  bool isThereASuchStateType(const std::string &i_type);

  // thread to externalize db update in other thread to reduce freezes
  XMThreadStats *getDbStatsThread();

  DownloadReplaysThread *getReplayDownloaderThread();

  void setCursorVisible(bool visible);

  void connectOrDisconnect();

private:
  GameState *popState();

  void calculateWhichStateIsRendered();
  void calculateFps();
  bool doRender();
  void drawFps();
  void drawStack();
  void drawTexturesLoading();
  void drawGeomsLoading();
  void drawCursor();

  void renderOverAll();

  void deleteToDeleteState();

  void setNewStateScreen(GameState *pNewState);

  bool m_isVisible;
  bool m_hasFocus;
  bool m_isInvalidated;

  std::vector<GameState *> m_statesStack;
  std::vector<GameState *> m_toDeleteStates;

  // last time the m_current*Fps have been filled
  int m_lastFpsTime;

  // contains the number of frame during the last second
  int m_currentRenderFps;
  int m_currentUpdateFps;

  // counting the number of frame for the current second
  int m_renderFpsNbFrame;
  int m_updateFpsNbFrame;

  // the hz at which the state manager runs
  // (the highest from the states)
  int m_maxUpdateFps;
  int m_maxRenderFps;
  int m_maxFps;

  // to render depending on the max Hz
  float m_renderCounter;
  // the desired rendering fps
  int m_curRenderFps;
  // how many max fps beat for one render
  float m_renderPeriod;

  // cursor
  Texture *m_cursor;
  bool m_isCursorVisible;
  int m_lastMouseMoveTime;
  int m_lastMouseMoveTimeInZone;
  int m_previousMouseOverPlayer;
  int m_previousMouseX;
  int m_previousMouseY;

  // video
  VideoRecorder *m_videoRecorder;

  // messages and associate observer states
  std::map<std::string, std::vector<GameState *>> m_registeredStates;

  // db stats thread
  XMThreadStats *m_xmstats;

  // replays downloader
  DownloadReplaysThread *m_drt;

  int m_currentUniqueId;

  RenderSurface m_screen;
};

#endif
