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

#include "StateManager.h"
#include "GameState.h"
#include "StateDeadMenu.h"
#include "StateEditProfile.h"
#include "StateEditWebConfig.h"
#include "StateFinished.h"
#include "StateHelp.h"
#include "StateLevelInfoViewer.h"
#include "StateLevelPackViewer.h"
#include "StateMainMenu.h"
#include "StateMessageBox.h"
#include "StateOptions.h"
#include "StatePause.h"
#include "StateRequestKey.h"
#include "StateServerConsole.h"
#include "StateVote.h"
#include "StateWaitServerInstructions.h"
#include "net/NetClient.h"

#include "common/XMSession.h"
#include "drawlib/DrawLib.h"
#include "helpers/Log.h"
#include "thread/DownloadReplaysThread.h"
#include "thread/XMThreadStats.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"
#include "xmoto/GeomsManager.h"
#include "xmoto/SysMessage.h"
#include "xmoto/VideoRecorder.h"
#include "xmscene/Camera.h"
#include <sstream>

#define CURSOR_MOVE_SHOWTIME 1000
#define NETPLAYERBOX_SHOWTIME 2000
#define NETPLAYERBOX_REMOVETIME 1000
#define NETPLAYERBOX_WIDTH 250
#define NETPLAYERBOX_HEIGHT 100
#define NETPLAYERBOX_BORDER 15

StateManager::StateManager() {
  m_currentRenderFps = 0;
  m_currentUpdateFps = 0;

  m_renderFpsNbFrame = 0;
  m_updateFpsNbFrame = 0;

  m_lastFpsTime = 0;
  m_renderPeriod = 1.0;

  m_maxUpdateFps = 50;
  m_maxRenderFps = 50;
  m_maxFps = 50;
  m_renderCounter = 0;
  m_curRenderFps = 50;

  m_cursor = NULL;

  // get the current focus and visibility status
  GameApp *game = GameApp::instance();
  m_isVisible = !game->isIconified();
  m_hasFocus = game->hasKeyboardFocus() || game->hasMouseFocus();

  m_isInvalidated = false;

  m_videoRecorder = NULL;
  // video
  if (XMSession::instance()->enableVideoRecording()) {
    m_videoRecorder =
      new VideoRecorder(XMSession::instance()->videoRecordName(),
                        XMSession::instance()->videoRecordingDivision(),
                        XMSession::instance()->videoRecordingFramerate());
  }

  m_currentUniqueId = 0;

  // the full xmoto windows it the screen of the statemanager
  m_screen =
    RenderSurface(Vector2i(0, 0),
                  Vector2i(GameApp::instance()->getDrawLib()->getDispWidth(),
                           GameApp::instance()->getDrawLib()->getDispHeight()));

  // create the db stats thread
  m_xmstats = new XMThreadStats(XMSession::instance()->sitekey(), this);

  // create the replay downloader thread
  m_drt = new DownloadReplaysThread(this);

  // mouse
  m_isCursorVisible = false;
  SDL_ShowCursor(SDL_DISABLE);
  m_lastMouseMoveTime = GameApp::instance()->getXMTimeInt();
  m_lastMouseMoveTimeInZone = GameApp::instance()->getXMTimeInt();
  m_previousMouseX = 0;
  m_previousMouseY = 0;
  m_previousMouseOverPlayer = -2;
}

StateManager::~StateManager() {
  while (m_statesStack.size() != 0) {
    delete popState();
  }

  /* stats */
  if (m_xmstats != NULL) {
    // do the stats job to confirm there are no more jobs waiting
    m_xmstats->doJob();

    // be sure the thread is finished before closing xmoto
    if (m_xmstats->waitForThreadEnd()) {
      LogError("stats thread failed");
    }
  }

  /* replays */
  if (m_drt != NULL) {
    // be sure the thread is finished before closing xmoto
    if (m_drt->waitForThreadEnd()) {
      LogError("replays downloader thread failed");
    }
    delete m_drt;
  }

  deleteToDeleteState();

  if (m_videoRecorder != NULL) {
    delete m_videoRecorder;
  }

  m_registeredStates.clear();

  // stats thread
  if (m_xmstats != NULL) {
    delete m_xmstats;
  }
}

std::string StateManager::getUniqueId() {
  std::ostringstream v_n;
  v_n << m_currentUniqueId;
  m_currentUniqueId++;
  return v_n.str();
}

void StateManager::setNewStateScreen(GameState *pNewState) {
  /* to test rendering a state to its own render surface, uncomment these :
  if(pNewState->getType() == "SCENE") {
    int offset = 100;
    pNewState->setScreen(RenderSurface(Vector2i(offset, offset),
                                       Vector2i(m_screen.getDispWidth()-offset,
  m_screen.getDispHeight()-offset)));
  } else {
    pNewState->setScreen(RenderSurface(Vector2i(0, 0),
                                       Vector2i(m_screen.getDispWidth(),
  m_screen.getDispHeight())));
  } */
  pNewState->setScreen(
    RenderSurface(Vector2i(0, 0),
                  Vector2i(m_screen.getDispWidth(), m_screen.getDispHeight())));
}

void StateManager::pushState(GameState *pNewState) {
  if (m_statesStack.size() != 0) {
    (m_statesStack.back())->leaveAfterPush();
  }

  setNewStateScreen(pNewState);
  m_statesStack.push_back(pNewState);
  pNewState->setStateId(getUniqueId());
  (m_statesStack.back())->enter();

  calculateWhichStateIsRendered();
  calculateFps();
}

GameState *StateManager::popState() {
  (m_statesStack.back())->leave();
  if (m_statesStack.back()->getType() != "") {
    (m_statesStack.back())->leaveType();
  }
  GameState *pState = m_statesStack.back();
  m_statesStack.pop_back();

  if (m_statesStack.size() != 0)
    (m_statesStack.back())->enterAfterPop();

  calculateWhichStateIsRendered();
  calculateFps();

  return pState;
}

void StateManager::flush() {
  while (m_statesStack.size() > 0 && m_statesStack.back()->requestForEnd()) {
    LogDebug("Flush %s", m_statesStack.back()->getName().c_str());
    delete popState();
  }

  deleteToDeleteState();

  if (m_statesStack.size() == 0) {
    GameApp::instance()->requestEnd();
  }
}

void StateManager::replaceState(GameState *pNewState,
                                const std::string &i_parentId) {
  for (int i = m_statesStack.size() - 1; i >= 0; i--) {
    if (m_statesStack[i]->getStateId() == i_parentId) {
      m_statesStack[i]->leave();
      if (m_statesStack[i]->getType() != "") {
        if (pNewState->getType() != m_statesStack[i]->getType()) {
          m_statesStack[i]->leaveType();
        }
      }
      // dude::can't use the same screen size, as a scene screen may
      // be smaller than the whole screen, and so the main menu state
      // which replace it will inherit its screen size.
      //      pNewState->setScreen(*(m_statesStack[i]->getScreen()));
      setNewStateScreen(pNewState);
      m_toDeleteStates.push_back(m_statesStack[i]);
      m_statesStack[i] = pNewState;
      pNewState->setStateId(i_parentId);
      pNewState->enter();
      calculateWhichStateIsRendered();
      calculateFps();
      return;
    }
  }

  throw Exception("No state to replace");
}

bool StateManager::needUpdateOrRender() {
  int i;

  if (m_hasFocus) { // m_isVisible is not needed while xmoto is rendered after
    // each event (including expose event)
    return true;
  }

  if (m_isInvalidated) {
    setInvalidated(false);
    return true;
  }

  if (m_statesStack.size() == 0) {
    return false;
  }

  // continue to find the upper state to update when unvisible
  // or if the upper state is not to update when unvisible, perhaps
  // its child is
  i = m_statesStack.size() - 1;
  while (m_statesStack[i]->updateWhenUnvisible() == false &&
         m_statesStack[i]->updateStatesBehind()) {
    // down to the state 0
    if (i == 0) {
      return false; // no such state found
    }
    i--;
  }
  return m_statesStack[i]->updateWhenUnvisible();
}

void StateManager::update() {
  // if there is no focus, don't update if the top state won't be updated
  if (m_hasFocus == false) {
    if (m_statesStack.size() > 0) {
      if (m_statesStack[m_statesStack.size() - 1]->updateWhenUnvisible() ==
          false) {
        return;
      }
    }
  }

  // flush states
  flush();

  bool oneUpdate = false;
  // we need a temporary vector to iterate on, because in their update
  // function, some states can push/replace a new state
  std::vector<GameState *> tmp = m_statesStack;
  std::vector<GameState *>::iterator stateIterator = tmp.begin();

  while (stateIterator != tmp.end()) {
    (*stateIterator)->executeCommands();
    ++stateIterator;
  }

  // flush states
  flush();

  tmp = m_statesStack;
  std::vector<GameState *>::reverse_iterator RstateIterator = tmp.rbegin();
  while (RstateIterator != tmp.rend()) {
    if ((*RstateIterator)->update() == true) {
      oneUpdate = true;
    }

    if ((*RstateIterator)->updateStatesBehind() == false)
      break;

    ++RstateIterator;
  }

  if (oneUpdate == true) {
    m_updateFpsNbFrame++;
  }

  /* update fps */
  if (m_lastFpsTime + 1000 < GameApp::getXMTimeInt()) {
    m_currentRenderFps = m_renderFpsNbFrame;
    m_currentUpdateFps = m_updateFpsNbFrame;
    m_renderFpsNbFrame = 0;
    m_updateFpsNbFrame = 0;
    m_lastFpsTime += 1000;
  }

  /* update mouse */
  int mx, my;
  GameApp::instance()->getMousePos(&mx, &my);
  if (mx != m_previousMouseX || my != m_previousMouseY) {
    m_lastMouseMoveTime = GameApp::instance()->getXMTimeInt();
  }
  m_previousMouseX = mx;
  m_previousMouseY = my;
}

void StateManager::renderOverAll() {
  int v_mouseOverPlayer; // -2 => on nobody ; -1 => on your profile ; X => on an
  // other player
  v_mouseOverPlayer = -2;

  // net infos
  if (NetClient::instance()->isConnected()) {
    FontManager *v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
    FontGlyph *v_fg;
    int vborder = 10;
    int v_voffset = 0;
    int v_maxwidth;

    int nMX, nMY;
    GameApp::getMousePos(&nMX, &nMY);
    int v_nameWidth;
    int v_nameBorder = 10;

    // header
    v_fg = GameApp::instance()->getDrawLib()->getFontSmall()->getGlyph(
      GAMETEXT_CONNECTED_PLAYERS);
    v_fm->printString(GameApp::instance()->getDrawLib(),
                      v_fg,
                      m_screen.getDispWidth() - v_fg->realWidth() - vborder,
                      vborder,
                      MAKE_COLOR(240, 240, 240, 255),
                      -1.0,
                      true);
    v_voffset += v_fg->realHeight();
    v_maxwidth = v_fg->realWidth();

    // you : name
    v_fg = GameApp::instance()->getDrawLib()->getFontSmall()->getGlyph(
      XMSession::instance()->profile());
    v_fm->printString(GameApp::instance()->getDrawLib(),
                      v_fg,
                      m_screen.getDispWidth() - v_fg->realWidth() - vborder,
                      vborder + v_voffset,
                      MAKE_COLOR(200, 200, 200, 255),
                      -1.0,
                      true);
    v_nameWidth = v_fg->realWidth();

    // you : points
    if (NetClient::instance()->mode() == NETCLIENT_SLAVE_MODE) {
      std::ostringstream v_npoints;
      v_npoints << "[ " << NetClient::instance()->points() << " ]";
      v_fg = GameApp::instance()->getDrawLib()->getFontSmall()->getGlyph(
        v_npoints.str());
      v_fm->printString(GameApp::instance()->getDrawLib(),
                        v_fg,
                        m_screen.getDispWidth() - v_fg->realWidth() - vborder -
                          v_nameWidth - v_nameBorder,
                        vborder + v_voffset,
                        MAKE_COLOR(255, 150, 150, 255),
                        -1.0,
                        true);
    }

    // update v_mouseOverPlayer
    if (nMX > m_screen.getDispWidth() - v_fg->realWidth() - vborder &&
        nMX < m_screen.getDispWidth() - vborder && nMY > vborder + v_voffset &&
        nMY < vborder + v_voffset + v_fg->realHeight()) {
      v_mouseOverPlayer = -1;
    }
    v_voffset += v_fg->realHeight();
    if (v_fg->realWidth() > v_maxwidth) {
      v_maxwidth = v_fg->realWidth();
    }

    // others
    for (unsigned int i = 0; i < NetClient::instance()->otherClients().size();
         i++) {
      v_fg = GameApp::instance()->getDrawLib()->getFontSmall()->getGlyph(
        NetClient::instance()->otherClients()[i]->name());

      // others : name
      v_fm->printString(GameApp::instance()->getDrawLib(),
                        v_fg,
                        m_screen.getDispWidth() - v_fg->realWidth() - vborder,
                        vborder + v_voffset,
                        MAKE_COLOR(200, 200, 200, 255),
                        -1.0,
                        true);
      v_nameWidth = v_fg->realWidth();

      // others : points
      if (NetClient::instance()->otherClients()[i]->mode() ==
          NETCLIENT_SLAVE_MODE) {
        std::ostringstream v_npoints;
        v_npoints << "[ " << NetClient::instance()->otherClients()[i]->points()
                  << " ]";
        v_fg = GameApp::instance()->getDrawLib()->getFontSmall()->getGlyph(
          v_npoints.str());
        v_fm->printString(GameApp::instance()->getDrawLib(),
                          v_fg,
                          m_screen.getDispWidth() - v_fg->realWidth() -
                            vborder - v_nameWidth - v_nameBorder,
                          vborder + v_voffset,
                          MAKE_COLOR(255, 150, 150, 255),
                          -1.0,
                          true);
      }

      // update v_mouseOverPlayer
      if (nMX > m_screen.getDispWidth() - v_fg->realWidth() - vborder &&
          nMX < m_screen.getDispWidth() - vborder &&
          nMY > vborder + v_voffset &&
          nMY < vborder + v_voffset + v_fg->realHeight()) {
        v_mouseOverPlayer = i;
      }
      v_voffset += v_fg->realHeight();
      if (v_fg->realWidth() > v_maxwidth) {
        v_maxwidth = v_fg->realWidth();
      }
    }

    // memorize previous value for transparency effect
    if (v_mouseOverPlayer != -2) {
      m_lastMouseMoveTimeInZone = m_lastMouseMoveTime;
    }

    // render the player information
    int v_displayPlayer = -2;

    // the mouse must have move recently
    if ((GameApp::instance()->getXMTimeInt() - m_lastMouseMoveTimeInZone) <
        NETPLAYERBOX_SHOWTIME + NETPLAYERBOX_REMOVETIME) {
      v_displayPlayer = v_mouseOverPlayer;

      // if the mouse is over nothing, display the last displayed player
      if (v_mouseOverPlayer == -2) {
        v_displayPlayer = m_previousMouseOverPlayer;
      }
    }

    // if there is something to display
    if (v_displayPlayer != -2 &&
        v_displayPlayer != -1 /* don't display yourself */) {
      int v_alpha;

      if ((GameApp::instance()->getXMTimeInt() - m_lastMouseMoveTimeInZone) <
          NETPLAYERBOX_SHOWTIME) {
        v_alpha = 255;
      } else {
        v_alpha = 255 - (int)(((GameApp::instance()->getXMTimeInt() -
                                m_lastMouseMoveTimeInZone) -
                               NETPLAYERBOX_SHOWTIME) *
                              255.0 / NETPLAYERBOX_REMOVETIME);
      }

      // display the name of the player
      std::string v_name, v_level;

      if (v_displayPlayer == -1) {
        v_name = XMSession::instance()->profile();
      } else {
        v_name = NetClient::instance()->otherClients()[v_displayPlayer]->name();
        v_level = NetClient::instance()
                    ->otherClients()[v_displayPlayer]
                    ->playingLevelName();
        if (v_level == "") {
          v_level = "-";
        }
      }

      v_fg =
        GameApp::instance()->getDrawLib()->getFontSmall()->getGlyph(v_name);

      // box
      GameApp::instance()->getDrawLib()->drawBox(
        Vector2f(m_screen.getDispWidth() - vborder - v_maxwidth -
                   NETPLAYERBOX_BORDER,
                 0),
        Vector2f(m_screen.getDispWidth() - vborder - v_maxwidth -
                   NETPLAYERBOX_BORDER - NETPLAYERBOX_WIDTH,
                 NETPLAYERBOX_HEIGHT),
        0.0,
        MAKE_COLOR(41, 41, 95, v_alpha));
      v_voffset = 0;

      // name
      v_fm->printString(GameApp::instance()->getDrawLib(),
                        v_fg,
                        m_screen.getDispWidth() - vborder - v_maxwidth -
                          NETPLAYERBOX_BORDER - NETPLAYERBOX_WIDTH / 2 -
                          v_fg->realWidth() / 2,
                        0,
                        MAKE_COLOR(200, 200, 200, v_alpha),
                        -1.0,
                        true);
      v_voffset += v_fg->realHeight();

      // level
      v_fg = GameApp::instance()->getDrawLib()->getFontSmall()->getGlyph(
        GAMETEXT_LEVEL + std::string(" :"));
      v_fm->printString(GameApp::instance()->getDrawLib(),
                        v_fg,
                        m_screen.getDispWidth() - vborder - v_maxwidth -
                          NETPLAYERBOX_BORDER - NETPLAYERBOX_WIDTH,
                        v_voffset,
                        MAKE_COLOR(200, 200, 200, v_alpha),
                        -1.0,
                        false);
      v_voffset += v_fg->realHeight();
      v_fg =
        GameApp::instance()->getDrawLib()->getFontSmall()->getGlyph(v_level);
      v_fm->printString(GameApp::instance()->getDrawLib(),
                        v_fg,
                        m_screen.getDispWidth() - vborder - v_maxwidth -
                          NETPLAYERBOX_BORDER - NETPLAYERBOX_WIDTH,
                        v_voffset,
                        MAKE_COLOR(200, 200, 200, v_alpha),
                        -1.0,
                        false);
      v_voffset += v_fg->realHeight();
    }

    // memorize previous value
    if (v_mouseOverPlayer != -2 &&
        (GameApp::instance()->getXMTimeInt() - m_lastMouseMoveTimeInZone) <
          NETPLAYERBOX_SHOWTIME + NETPLAYERBOX_REMOVETIME) {
      m_previousMouseOverPlayer = v_mouseOverPlayer;
    }
  }
}

void StateManager::render() {
  std::vector<GameState *>::iterator stateIterator;

  if (m_isVisible == false) {
    return;
  }

  if (doRender() == true) {
    DrawLib *drawLib = GameApp::instance()->getDrawLib();
    drawLib->resetGraphics();

    // erase screen if the first state allow somebody to write before (it means
    // that it has potentially some transparent parts)
    if (m_statesStack.size() > 0) {
      if (m_statesStack[0]->drawStatesBehind()) {
        drawLib->clearGraphics();
      }
    }

    /* we have to draw states from the bottom of the stack to the top */
    stateIterator = m_statesStack.begin();
    while (stateIterator != m_statesStack.end()) {
      if ((*stateIterator)->isHide() == false) {
        (*stateIterator)->render();
      }

      ++stateIterator;
    }

    renderOverAll();

    // FPS
    if (XMSession::instance()->fps()) {
      drawFps();
    }

    // STACK
    if (XMSession::instance()->debug()) {
      drawStack();
      drawTexturesLoading();
      drawGeomsLoading();
    }

    // scraps
    if (XMSession::instance()->debug()) {
      // render scraps
      FontManager *v_fm =
        drawLib->getFontSmall(); // any in fact while it's shared
      v_fm->displayScrap(drawLib);
    }

    // SYSMESSAGE
    SysMessage::instance()->render();

    // CURSOR
    if (m_statesStack.size() > 0) {
      bool m_mustCursorBeDisplayed =
        m_statesStack.back()->showCursor() ||
        (GameApp::instance()->getXMTimeInt() - m_lastMouseMoveTime) <
          CURSOR_MOVE_SHOWTIME;

      if (XMSession::instance()->ugly()) {
        setCursorVisible(m_mustCursorBeDisplayed);
      } else {
        if (XMSession::instance()->useThemeCursor()) {
          if (m_mustCursorBeDisplayed) {
            drawCursor();
          }
          setCursorVisible(false); // hide the cursor to show the texture
        } else {
          setCursorVisible(m_mustCursorBeDisplayed);
        }
      }
    }

    drawLib->flushGraphics();
    m_renderFpsNbFrame++;

    stateIterator = m_statesStack.begin();
    while (stateIterator != m_statesStack.end()) {
      if ((*stateIterator)->isHide() == false) {
        (*stateIterator)->onRenderFlush();
      }

      ++stateIterator;
    }
  }
}

VideoRecorder *StateManager::getVideoRecorder() {
  return m_videoRecorder;
}

void StateManager::drawFps() {
  char cTemp[128];

  if (NetClient::instance()->isConnected()) {
    snprintf(cTemp,
             128,
             "u(%i) d(%i) n(%i)",
             getCurrentUpdateFPS(),
             getCurrentRenderFPS(),
             NetClient::instance()->getOwnFrameFPS());
  } else {
    snprintf(
      cTemp, 128, "u(%i) d(%i)", getCurrentUpdateFPS(), getCurrentRenderFPS());
  }

  FontManager *v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
  FontGlyph *v_fg = v_fm->getGlyph(cTemp);
  v_fm->printString(GameApp::instance()->getDrawLib(),
                    v_fg,
                    0,
                    130,
                    MAKE_COLOR(255, 255, 255, 255),
                    -1.0,
                    true);
}

void StateManager::drawTexturesLoading() {
  std::ostringstream v_n;
  v_n << "Textures: "
      << Theme::instance()->getTextureManager()->getTextures().size();

  FontManager *v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
  FontGlyph *v_fg = v_fm->getGlyph(v_n.str());
  v_fm->printString(GameApp::instance()->getDrawLib(),
                    v_fg,
                    0,
                    100,
                    MAKE_COLOR(255, 255, 255, 255),
                    -1.0,
                    true);
}

void StateManager::drawGeomsLoading() {
  std::ostringstream v_n;
  v_n << "Geoms (Blocks/Edges): "
      << GeomsManager::instance()->getNumberOfBlockGeoms() << "/"
      << GeomsManager::instance()->getNumberOfEdgeGeoms();

  FontManager *v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
  FontGlyph *v_fg = v_fm->getGlyph(v_n.str());
  v_fm->printString(GameApp::instance()->getDrawLib(),
                    v_fg,
                    0,
                    120,
                    MAKE_COLOR(255, 255, 255, 255),
                    -1.0,
                    true);
}

void StateManager::drawStack() {
  int i = 0;
  FontGlyph *v_fg;
  FontManager *v_fm;
  DrawLib *drawLib = GameApp::instance()->getDrawLib();

  int xoff = 0;
  int yoff = m_screen.getDispHeight();
  int w = 180;
  int h = 30;
  Color bg_none = MAKE_COLOR(0, 0, 0, 200);
  Color bg_updated = MAKE_COLOR(255, 0, 0, 200);
  Color bg_rendered = MAKE_COLOR(0, 255, 0, 200);
  Color font_color = MAKE_COLOR(255, 255, 255, 255);

  std::vector<GameState *>::iterator stateIterator = m_statesStack.begin();
  v_fm = drawLib->getFontSmall();

  while (stateIterator != m_statesStack.end()) {
    Color bg_render = bg_none;
    Color bg_update = bg_none;

    if (m_statesStack[i]->updateStatesBehind() == true) {
      bg_update = bg_updated;
    }
    if (m_statesStack[i]->isHide() == false) {
      bg_render = bg_rendered;
    }

    drawLib->drawBox(Vector2f(xoff, yoff - (i * h)),
                     Vector2f(xoff + w / 2, yoff - ((i + 1) * h)),
                     1.0,
                     bg_update);
    drawLib->drawBox(Vector2f(xoff + w / 2, yoff - (i * h)),
                     Vector2f(xoff + w, yoff - ((i + 1) * h)),
                     1.0,
                     bg_render);

    if (m_statesStack[i]->getStateId() == "") {
      v_fg = v_fm->getGlyph(m_statesStack[i]->getName());
    } else {
      v_fg = v_fm->getGlyph(m_statesStack[i]->getName() + " (" +
                            m_statesStack[i]->getStateId() + ")");
    }
    v_fm->printString(drawLib,
                      v_fg,
                      (w - v_fg->realWidth()) / 2 + xoff,
                      yoff - ((i + 1) * h - v_fg->realHeight() / 2),
                      font_color,
                      0.0,
                      true);

    i++;
    ++stateIterator;
  }
}

void StateManager::drawCursor() {
  Sprite *pSprite;

  if (!XMSession::instance()->useThemeCursor())
    return;

  if (m_cursor == NULL) {
    /* load cursor */
    pSprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "Cursor");
    if (pSprite != NULL) {
      m_cursor = pSprite->getTexture(false, WrapMode::Clamp, FM_LINEAR);
    }
  }

  if (m_cursor != NULL && XMSession::instance()->ugly() == false) {
    int nMX, nMY;
    GameApp::getMousePos(&nMX, &nMY);
    GameApp::instance()->getDrawLib()->drawImage(Vector2f(nMX - 2, nMY - 2),
                                                 Vector2f(nMX + 30, nMY + 30),
                                                 m_cursor,
                                                 0xFFFFFFFF,
                                                 true);
  }
}

void StateManager::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (m_statesStack.size() == 0)
    return;
  (m_statesStack.back())->xmKey(i_type, i_xmkey);
}

void StateManager::fileDrop(const std::string &path) {
  if (m_statesStack.size() == 0)
    return;
  (m_statesStack.back())->fileDrop(path);
}

void StateManager::changeFocus(bool i_hasFocus) {
  m_hasFocus = i_hasFocus;
}

void StateManager::changeVisibility(bool i_visible) {
  m_isVisible = i_visible;
}

void StateManager::calculateWhichStateIsRendered() {
  /* calculate which state will be rendered */
  std::vector<GameState *>::reverse_iterator stateIterator =
    m_statesStack.rbegin();
  bool hideState = false;

  while (stateIterator != m_statesStack.rend()) {
    (*stateIterator)->setHide(hideState);

    if (hideState == false && (*stateIterator)->drawStatesBehind() == false) {
      hideState = true;
    }

    ++stateIterator;
  }
}

void StateManager::calculateFps() {
  // we can't put zero. because there's divisions by that number, and
  // when there's no states, there are used for frame duration
  // calculation
  int maxUpdateFps = 1;
  int maxRenderFps = 1;

  int topStateRenderFps = 0;

  std::vector<GameState *>::iterator stateIterator = m_statesStack.begin();

  while (stateIterator != m_statesStack.end()) {
    int updateFps = (*stateIterator)->getUpdateFps();
    int renderFps = 0;
    if ((*stateIterator)->isHide() == false) {
      renderFps = (*stateIterator)->getRenderFps();
      topStateRenderFps = renderFps;
    }

    if (updateFps > maxUpdateFps) {
      maxUpdateFps = updateFps;
    }
    if (renderFps > maxRenderFps) {
      maxRenderFps = renderFps;
    }

    ++stateIterator;
  }

  m_maxUpdateFps = maxUpdateFps;
  m_maxRenderFps = maxRenderFps;
  m_maxFps =
    (m_maxUpdateFps > m_maxRenderFps) ? m_maxUpdateFps : m_maxRenderFps;

  stateIterator = m_statesStack.begin();
  while (stateIterator != m_statesStack.end()) {
    (*stateIterator)->setCurrentRenderFps(topStateRenderFps);
    (*stateIterator)->setMaxFps(m_maxFps);
    ++stateIterator;
  }

  m_curRenderFps = topStateRenderFps;
  m_renderPeriod = (float)m_maxFps / (float)m_curRenderFps;
}

void StateManager::cleanStates() {
  StateDeadMenu::clean();
  StateEditProfile::clean();
  StateEditWebConfig::clean();
  StateFinished::clean();
  StateLevelInfoViewer::clean();
  StateLevelPackViewer::clean();
  StateMainMenu::clean();
  StatePause::clean();
  StateRequestKey::clean();
  StateOptions::clean();
  StateVote::clean();
  StateServerConsole::clean();
}

void StateManager::refreshStaticCaptions() {
  //  StateDeadMenu::refreshStaticCaptions();
  //  StateEditProfile::refreshStaticCaptions();
  //  StateEditWebConfig::refreshStaticCaptions();
  //  StateFinished::refreshStaticCaptions();
  //  StateLevelInfoViewer::refreshStaticCaptions();
  //  StateLevelPackViewer::refreshStaticCaptions();
  StateMainMenu::refreshStaticCaptions();
  //  StatePause::refreshStaticCaptions();
  //  StateRequestKey::refreshStaticCaptions();
}

GameState *StateManager::getTopState() {
  if (m_statesStack.size() == 0) {
    return NULL;
  }

  return m_statesStack[m_statesStack.size() - 1];
}

bool StateManager::isTopOfTheStates(GameState *i_state) {
  return i_state && getTopState() == i_state;
}

int StateManager::numberOfStates() {
  return m_statesStack.size();
}

int StateManager::getCurrentUpdateFPS() {
  return m_currentUpdateFps;
}

int StateManager::getCurrentRenderFPS() {
  return m_currentRenderFps;
}

bool StateManager::doRender() {
  m_renderCounter += 1.0;

  if (XMSession::instance()->timedemo()) {
    return true;
  }

  if (m_renderCounter >= m_renderPeriod) {
    m_renderCounter -= m_renderPeriod;
    return true;
  }
  return false;
}

void StateManager::registerAsObserver(const std::string &message,
                                      GameState *self) {
  std::map<std::string, std::vector<GameState *>>::iterator itFind;

  itFind = m_registeredStates.find(message);
  if (itFind != m_registeredStates.end()) {
    if (self != NULL) {
      // check if the state is already registered
      if (XMSession::instance()->debug() == true) {
        std::vector<GameState *> &states = (*itFind).second;
        std::vector<GameState *>::iterator it = states.begin();
        std::string name = self->getName();

        while (it != states.end()) {
          if ((*it)->getName() == name) {
            LogWarning("state [%s] already registered as observer for this "
                       "message [%s].",
                       name.c_str(),
                       message.c_str());
            break;
          }
          ++it;
        }
      }

      (*itFind).second.push_back(self);
    }
  } else {
    m_registeredStates[message] = std::vector<GameState *>();
    if (self != NULL) {
      itFind = m_registeredStates.find(message);
      (*itFind).second.push_back(self);
    }
  }
}

void StateManager::unregisterAsObserver(const std::string &message,
                                        GameState *self) {
  std::map<std::string, std::vector<GameState *>>::iterator itFind;

  itFind = m_registeredStates.find(message);
  if (itFind != m_registeredStates.end()) {
    std::vector<GameState *> &states = (*itFind).second;
    std::vector<GameState *>::iterator stateIterator = states.begin();

    while (stateIterator != states.end()) {
      if (self == (*stateIterator)) {
        states.erase(stateIterator);
        return;
      }

      ++stateIterator;
    }
  } else {
    LogWarning(
      "unregisterAsObserver message [%s] state [%], but not registered.",
      message.c_str(),
      self->getName().c_str());
  }
}

void StateManager::registerAsEmitter(const std::string &message) {
  // TODO::show debug information
  return registerAsObserver(message, NULL);
}

void StateManager::sendSynchronousMessage(const std::string &message,
                                          const std::string &args,
                                          const std::string &i_parentId) {
  std::map<std::string, std::vector<GameState *>>::iterator itFind;

  itFind = m_registeredStates.find(message);
  if (itFind != m_registeredStates.end()) {
    std::vector<GameState *> &states = (*itFind).second;
    std::vector<GameState *>::iterator stateIterator = states.begin();

    if (states.size() == 0) {
      if (XMSession::instance()->debug()) {
        LogWarning("sendSynchronousMessage message [%s [%s]] sent and there's "
                   "no state to receive it.",
                   message.c_str(),
                   args.c_str());
      }
    }

    while (stateIterator != states.end()) {
      if (i_parentId == "" || (*stateIterator)->getStateId() == i_parentId) {
        (*stateIterator)->executeOneCommand(message, args);

        LogDebug(
          "sendSynchronousMessage [%s [%s]] to [%s] /* parent_id = '%s' */",
          message.c_str(),
          args.c_str(),
          (*stateIterator)->getName().c_str(),
          i_parentId.c_str());
      }

      ++stateIterator;
    }
  } else {
    if (XMSession::instance()->debug()) {
      LogWarning("sendSynchronousMessage message [%s [%s]] sent and there's no "
                 "state to receive it.",
                 message.c_str(),
                 args.c_str());
    }
  }
}

void StateManager::sendAsynchronousMessage(const std::string &message,
                                           const std::string &args,
                                           const std::string &i_parentId) {
  std::map<std::string, std::vector<GameState *>>::iterator itFind;

  itFind = m_registeredStates.find(message);
  if (itFind != m_registeredStates.end()) {
    std::vector<GameState *> &states = (*itFind).second;
    std::vector<GameState *>::iterator stateIterator = states.begin();

    if (states.size() == 0) {
      if (XMSession::instance()->debug()) {
        LogWarning("sendAsynchronousMessage message [%s [%s]] sent and there's "
                   "no state to receive it.",
                   message.c_str(),
                   args.c_str());
      }
    }

    while (stateIterator != states.end()) {
      if (i_parentId == "" || (*stateIterator)->getStateId() == i_parentId) {
        LogDebug(
          "sendAsynchronousMessage [%s [%s]] to [%s] /* parent_id = '%s' */",
          message.c_str(),
          args.c_str(),
          (*stateIterator)->getName().c_str(),
          i_parentId.c_str());

        (*stateIterator)->send(message, args);
      }
      ++stateIterator;
    }
  } else {
    if (XMSession::instance()->debug()) {
      LogWarning("sendAsynchronousMessage message [% [%s]s] sent and there's "
                 "no state to receive it.",
                 message.c_str(),
                 args.c_str());
    }
  }
}

void StateManager::deleteToDeleteState() {
  while (m_toDeleteStates.size() != 0) {
    delete m_toDeleteStates.back();
    m_toDeleteStates.pop_back();
  }
}

bool StateManager::isThereASuchState(const std::string &i_name) {
  for (unsigned int i = 0; i < m_statesStack.size(); i++) {
    if (m_statesStack[i]->getName() == i_name) {
      return true;
    }
  }

  return false;
}

bool StateManager::isThereASuchStateType(const std::string &i_type) {
  for (unsigned int i = 0; i < m_statesStack.size(); i++) {
    if (m_statesStack[i]->getStateType() == i_type) {
      return true;
    }
  }

  return false;
}

XMThreadStats *StateManager::getDbStatsThread() {
  return m_xmstats;
}

DownloadReplaysThread *StateManager::getReplayDownloaderThread() {
  return m_drt;
}

void StateManager::setCursorVisible(bool visible) {
  if (m_isCursorVisible == visible)
    return;

  m_isCursorVisible = visible;
  SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}

void StateManager::connectOrDisconnect() {
  if (NetClient::instance()->isConnected()) {
    NetClient::instance()->disconnect();
  } else {
    try {
      NetClient::instance()->connect(XMSession::instance()->clientServerName(),
                                     XMSession::instance()->clientServerPort());

      NetClient::instance()->changeMode(XMSession::instance()->clientGhostMode()
                                          ? NETCLIENT_GHOST_MODE
                                          : NETCLIENT_SLAVE_MODE);

      if (!XMSession::instance()->clientGhostMode())
        StateManager::instance()->pushState(new StateWaitServerInstructions());
    } catch (Exception &e) {
      SysMessage::instance()->displayError(GAMETEXT_UNABLETOCONNECTONTHESERVER);
      LogError("Unable to connect to the server");
    }
  }
}
