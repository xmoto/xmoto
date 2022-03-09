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

#include "Scene.h"
#include "BikeController.h"
#include "BikeParameters.h"
#include "BikePlayer.h"
#include "Block.h"
#include "Camera.h"
#include "ChipmunkWorld.h"
#include "Entity.h"
#include "GhostTrail.h"
#include "PhysicsSettings.h"
#include "ScriptTimer.h"
#include "common/VFileIO.h"
#include "helpers/Log.h"
#include "helpers/Random.h"
#include "net/NetActions.h"
#include "net/NetClient.h"
#include "xmoto/BSP.h"
#include "xmoto/Game.h"
#include "xmoto/GameEvents.h"
#include "xmoto/GameText.h"
#include "xmoto/LuaLibGame.h"
#include "xmoto/PhysSettings.h"
#include "xmoto/Replay.h"
#include "xmoto/ScriptDynamicObjects.h"
#include "xmoto/Sound.h"

#define GAMEMESSAGES_PACKTIME 40
#define XM_PHYSICS_MD5 "b6822d58d992fbb0a7ef45eed71141e4"

/*
 *  Game object. Handles all of the gamestate management und so weiter.
 */
Scene::Scene() {
  m_lastCallToEveryHundreath = 0;

  m_showGhostTimeDiff = true;

  m_motoGameHooks = NULL;

  m_speed_factor = 1.00f;
  m_is_paused = false;
  m_playEvents = true;

  m_lastStateSerializationTime = -100; /* loong time ago :) */
  m_lastStateUploadTime = -100;

  m_pLevelSrc = NULL;

  m_luaGame = NULL;

  m_currentCamera = 0;

  m_chipmunkWorld = NULL;

  m_halfUpdate = true;
  m_physicsSettings = NULL;
  m_ghostTrail = NULL;
  m_checkpoint = NULL;
}

Scene::~Scene() {
  cleanPlayers();
  cleanGhosts();
  cleanScriptTimers();
  if (m_ghostTrail != NULL) {
    delete m_ghostTrail;
  }
}

void Scene::loadLevel(xmDatabase *i_db,
                      const std::string &i_id_level,
                      bool i_loadMainLayerOnly) {
  char **v_result;
  unsigned int nrow;

  m_pLevelSrc = new Level();
  try {
    v_result = i_db->readDB("SELECT filepath FROM levels "
                            "WHERE id_level=\"" +
                              xmDatabase::protectString(i_id_level) + "\";",
                            nrow);
    if (nrow != 1) {
      i_db->read_DB_free(v_result);
      throw Exception("Level " + i_id_level + " not found");
    }
    m_pLevelSrc->setFileName(i_db->getResult(v_result, 1, 0, 0));
    i_db->read_DB_free(v_result);

    m_pLevelSrc->loadReducedFromFile(i_loadMainLayerOnly);

  } catch (Exception &e) {
    delete m_pLevelSrc;
    m_pLevelSrc = NULL;
    throw e;
  }
}

void Scene::cleanGhosts() {
  for (unsigned int i = 0; i < m_ghosts.size(); i++) {
    delete m_ghosts[i];
  }
  m_ghosts.clear();
}

void Scene::cleanPlayers() {
  for (unsigned int i = 0; i < m_players.size(); i++) {
    if (m_players[i]->getOnBikerHooks() != NULL) {
      delete m_players[i]->getOnBikerHooks();
    }
    delete m_players[i];
  }
  m_players.clear();
}

void Scene::setHooks(SceneHooks *i_motoGameHooks) {
  m_motoGameHooks = i_motoGameHooks;
}

/*===========================================================================
  Teleporting
  ===========================================================================*/
void Scene::setPlayerPosition(int i_player, float x, float y, bool bFaceRight) {
  /* Going to teleport? Do it now, before we tinker to much with the state */
  if (m_players[i_player]->isDead() == false) {
    m_players[i_player]->initToPosition(
      Vector2f(x, y), bFaceRight ? DD_RIGHT : DD_LEFT, m_PhysGravity);
  }
}

const Vector2f &Scene::getPlayerPosition(int i_player) {
  return m_players[i_player]->getState()->CenterP;
  throw Exception("Invalid player number");
}

DriveDir Scene::getPlayerFaceDir(int i_player) {
  return m_players[i_player]->getState()->Dir;
}

/*===========================================================================
  Add game message
  ===========================================================================*/
void Scene::gameMessage(std::string Text,
                        bool bOnce,
                        int duration,
                        MessageType i_msgType) {
  if (Text == "")
    return; // eliminate empty messages

  /* "unique"? */
  GameMessage *pMsg = NULL;

  if (bOnce) {
    /* Yeah, if there's another "unique", replace it */
    for (unsigned int i = 0; i < m_GameMessages.size(); i++) {
      if (m_GameMessages[i]->bOnce) {
        pMsg = m_GameMessages[i];
        break;
      }
    }
  }

  if (pMsg == NULL) {
    if (bOnce) { /* don't allow \n split on bOnce messages (i'm too lazy) */
      pMsg = new GameMessage;
      pMsg->removeTime = getTime() + duration;
      pMsg->bNew = true;
      pMsg->nAlpha = 255;
      pMsg->bOnce = bOnce;
      pMsg->Text = Text;
      pMsg->msgType = i_msgType;
      m_GameMessages.push_back(pMsg);
    } else {
      /* split the message if \n is encountered */
      std::string v_txtRest = Text;
      int n = v_txtRest.find_first_of("\n");

      while (n >= 0 && (unsigned int)n < v_txtRest.length()) {
        pMsg = new GameMessage;
        pMsg->removeTime = getTime() + duration;
        pMsg->bNew = true;
        pMsg->nAlpha = 255;
        pMsg->bOnce = bOnce;
        pMsg->Text = v_txtRest.substr(0, n);
        pMsg->msgType = i_msgType;
        m_GameMessages.push_back(pMsg);
        v_txtRest = v_txtRest.substr(n + 1, v_txtRest.length() - 1);
        n = v_txtRest.find_first_of("\n");
      }

      pMsg = new GameMessage;
      pMsg->removeTime = getTime() + duration;
      pMsg->bNew = true;
      pMsg->nAlpha = 255;
      pMsg->bOnce = bOnce;
      pMsg->Text = v_txtRest;
      pMsg->msgType = i_msgType;

      m_GameMessages.push_back(pMsg);
    }
  } else {
    /* redefine the text only to replay a bonce message */
    pMsg->Text = Text;
    pMsg->removeTime = getTime() + duration;
    pMsg->nAlpha = 255;
    pMsg->msgType = i_msgType;
  }
  packGameMessages();
  updateGameMessages();
}

void Scene::packGameMessages() { // put multiple GameMessages into one, if they
  // appear shortly one after another

  for (int i = m_GameMessages.size() - 1; i > 0; i--) {
    if ((m_GameMessages[i]->removeTime - m_GameMessages[i - 1]->removeTime <
         GAMEMESSAGES_PACKTIME) &&
        (m_GameMessages[i]->removeTime != 0) &&
        (m_GameMessages[i]->msgType == m_GameMessages[i - 1]->msgType) &&
        (m_GameMessages[i]->bOnce == m_GameMessages[i - 1]->bOnce) &&
        (m_GameMessages[i]->nAlpha ==
         m_GameMessages[i - 1]->nAlpha)) { // of same Type?

      m_GameMessages[i - 1]->Text += "\n" + m_GameMessages[i]->Text;
      m_GameMessages[i - 1]->removeTime =
        m_GameMessages[i]->removeTime; // set removeTime of latest Message
      // remove last one
      delete m_GameMessages[i];
      m_GameMessages.erase(m_GameMessages.begin() + i);
    }
  }

  /* count number of lines */
  for (unsigned i = 0; i < m_GameMessages.size(); i++) {
    int v_numLines = 1;
    for (unsigned j = 0; j < m_GameMessages[i]->Text.length(); j++) {
      if (!m_GameMessages[i]->Text.compare(j, 1, "\n")) {
        v_numLines++;
      }
    }
    m_GameMessages[i]->lines = v_numLines;
  }
}

void Scene::clearGameMessages(void) {
  for (unsigned int i = 0; i < m_GameMessages.size(); i++)
    m_GameMessages[i]->removeTime = 0;
}

void Scene::updatePlayers(int timeStep, bool i_updateDiedPlayers) {
  for (unsigned int i = 0; i < m_players.size(); i++) {
    if (i_updateDiedPlayers || m_players[i]->isDead() == false) {
      m_players[i]->updateToTime(
        m_time, timeStep, &m_Collision, m_PhysGravity, this);

      if (m_playEvents && timeStep > 0) {
        /* New wheel-spin particles? */
        if (m_players[i]->isWheelSpinning()) {
          if (NotSoRandom::randomNum(0, 1) < 0.7f) {
            ParticlesSource *v_debris;
            v_debris =
              (ParticlesSource *)getLevelSrc()->getEntityById("BikeDebris");
            v_debris->setDynamicPosition(m_players[i]->getWheelSpinPoint());
            v_debris->addParticle(m_time);
          }
        }
      }
    }
  }
}

/*===========================================================================
  Update game
  ===========================================================================*/
void Scene::updateLevel(int timeStep,
                        Replay *i_frameRecorder,
                        DBuffer *i_eventRecorder,
                        bool i_fast,
                        bool i_allowParticules,
                        bool i_updateDiedPlayers) {
  float v_diff;
  int v_previousTime;
  bool v_recordReplay;
  bool v_uploadFrame;
  SerializedBikeState BikeState;

  if (m_is_paused || (m_useTargetTime &&
                      m_time > m_targetTime)) // do nothing when m_targetTime >
    // m_time because it would mean
    // that the server is faster than
    // the local host which can
    // however be true
    return;

  if (m_halfUpdate == true) {
    getLevelSrc()->updateToTime(*this, m_physicsSettings, i_allowParticules);
    m_halfUpdate = false;
  } else
    m_halfUpdate = true;

  updateGameMessages();

  /* Increase time */
  if (m_speed_factor != 1.00f) {
    // save the diff when timeStep is not an integer to make nice rewind/forward
    v_previousTime = m_time;
    v_diff = ((float)timeStep) * m_speed_factor;
    m_floattantTimeStepDiff += v_diff - ((int)v_diff);
    m_time += (int)(v_diff);
    m_time += ((int)m_floattantTimeStepDiff);
    m_floattantTimeStepDiff -= ((int)m_floattantTimeStepDiff);

    if (m_time < 0) {
      m_time = 0;
    }

    if (v_previousTime > m_time) {
      onRewinding();
    }

  } else {
    m_time += timeStep;
  }

  /* Update misc stuff (only when not playing a replay) */
  if (m_playEvents) {
    getLevelSrc()->updatePhysics(
      m_time, timeStep, &m_Collision, m_chipmunkWorld, i_eventRecorder);
    _UpdateZones();
    _UpdateEntities();
  }

  /* update ScriptTimers */
  try {
    unsigned int i = 0;
    while (i < m_ScriptTimers.size()) {
      m_ScriptTimers[i]->UpdateTimer(getTime());
      if (m_ScriptTimers[i]->isFinished() == true) {
        delete m_ScriptTimers[i];
        m_ScriptTimers.erase(m_ScriptTimers.begin() + i);
      } else { // only grow index if we don't delete anything
        i++;
      }
    }
  } catch (Exception &e) {
    LogWarning(
      std::string("Script Timer Update Failed!!\n" + e.getMsg()).c_str());
    throw e;
  }

  /* Invoke Tick() script function */
  /* and play script dynamic objects */
  int v_nbCents = 0;
  while (getTime() - m_lastCallToEveryHundreath > 1) {
    if (m_playEvents) {
      if (m_luaGame->scriptCallBool("Tick", true) == false) {
        throw Exception("level script Tick() returned false");
      }
    }
    v_nbCents++;
    m_lastCallToEveryHundreath += 1;
  }
  nextStateScriptDynamicObjects(v_nbCents);

  for (unsigned int i = 0; i < m_ghosts.size(); i++) {
    m_ghosts[i]->updateToTime(
      getTime(), timeStep, &m_Collision, m_PhysGravity, this);
  }

  updatePlayers(timeStep, i_updateDiedPlayers);

  if (m_chipmunkWorld != NULL) {
    /* players moves, update their positions */
    m_chipmunkWorld->updateWheelsPosition(m_players);
  }

  // last thing is to execute all collected events. don't create events after
  // here
  executeEvents(i_eventRecorder);

  // record the replay only if
  v_recordReplay =
    i_frameRecorder != NULL &&
    getTime() - m_lastStateSerializationTime >=
      100.0f / i_frameRecorder->getFrameRate() && // limit the framerate
    Players().size() == 1; // supported in one player mode only

  // upload the frame only if
  v_uploadFrame = NetClient::instance()->isConnected() &&
                  NetClient::instance()->mode() == NETCLIENT_GHOST_MODE &&
                  getTime() - m_lastStateUploadTime >=
                    100.0f / XMSession::instance()->clientFramerateUpload();

  if (v_recordReplay) {
    m_lastStateSerializationTime = getTime();
  }

  if (v_uploadFrame) {
    m_lastStateUploadTime = getTime();
  }

  /* save the replay */
  if ((v_recordReplay || v_uploadFrame) && i_fast == false) {
    for (unsigned int i = 0; i < Players().size(); i++) {
      // in both case, don't get when player is dead (in the frame, you don't
      // get the 'died' information, thus via the network, you're not able to
      // draw the player correctly)
      if (Players()[i]->isDead() == false &&
          Players()[i]->isFinished() == false) {
        // get the state only if we need it for replay or frame
        if ((v_uploadFrame && Players()[i]->localNetId() >= 0) ||
            (v_recordReplay &&
             i ==
               0)) { // /* only store the state if 1 player plays for replays */
          getSerializedBikeState(
            Players()[i]->getState(), getTime(), &BikeState, m_physicsSettings);
        }

        if (v_uploadFrame && Players()[i]->localNetId() >= 0) {
          NA_frame na(&BikeState);
          try {
            NetClient::instance()->send(&na, Players()[i]->localNetId());
          } catch (Exception &e) {
          }
        }

        if (v_recordReplay && i == 0) {
          i_frameRecorder->storeState(BikeState);
          if (m_pLevelSrc->isPhysics()) {
            i_frameRecorder->storeBlocks(
              getTime(), m_pLevelSrc->Blocks(), Players());
          }
        }
      }
    }
  }

  /* Entities scheduled for termination? */
  for (unsigned int i = 0; i < m_DelSchedule.size(); i++) {
    _KillEntity(m_DelSchedule[i]);
  }
  m_DelSchedule.clear();
}

void Scene::executeEvents_step(SceneEvent *pEvent, DBuffer *i_recorder) {
  if (i_recorder != NULL) { /* Encode event */
    pEvent->serialize(*i_recorder);
  }
  pEvent->doAction(this);
  // printf("%s => %i\n", pEvent->toString().c_str(), pEvent->getEventTime());
}

void Scene::executeEvents(DBuffer *i_recorder) {
  /* Handle events generated this update */

  // here, we must play GAME_EVENT_PLAYER_WINS after other events
  // this avoid people wining by catching a flower and going throw the wall or
  // taking a wrecker at the same time
  // we must be sure that all events are played before declaring a player
  // winning -- so that death + win forces death
  // note that events can create events, so, it's not just simple loops

  unsigned int i;
  bool v_found_other;

  // 1st : play events until only GAME_EVENT_PLAYER_WINS are remaining
  do {
    v_found_other = false;
    i = 0;
    while (i < m_GameEventQueue.size()) {
      if (m_GameEventQueue[i]->getType() != GAME_EVENT_PLAYER_WINS) {
        v_found_other = true;
        executeEvents_step(m_GameEventQueue[i], i_recorder);
        destroyGameEvent(m_GameEventQueue[i]);
        m_GameEventQueue.erase(m_GameEventQueue.begin() + i);
      } else {
        i++;
      }
    }
  } while (v_found_other);

  // then when only GAME_EVENT_PLAYER_WINS are remaining, play them
  while (m_GameEventQueue.size() > 0) {
    executeEvents_step(m_GameEventQueue[0], i_recorder);
    destroyGameEvent(m_GameEventQueue[0]);
    m_GameEventQueue.erase(m_GameEventQueue.begin());
  }
}

void Scene::updateGameMessages() {
  /* Handle game messages (keep them in place) */
  int i = 0;
  while (1) {
    if (i < 0 || (unsigned int)i >= m_GameMessages.size())
      break;
    if (getTime() > m_GameMessages[i]->removeTime) {
      m_GameMessages[i]->nAlpha -= 2;

      if (m_GameMessages[i]->nAlpha <= 0) {
        delete m_GameMessages[i];
        m_GameMessages.erase(m_GameMessages.begin() + i);
        i--;
        continue;
      }
    }

    /* detect correct target position, considering number of lines of previously
     * displayed message */
    int v_row = 0;
    for (int j = 0; j < i; j++) {
      v_row += m_GameMessages[j]->lines;
    }
    Vector2f TargetPos = Vector2f(
      0.2f, (0.5f - (m_GameMessages.size() * 0.05f) / (2.0f) + 0.049f * v_row));

    if (m_GameMessages[i]->bNew) {
      m_GameMessages[i]->Vel = Vector2f(0, 0);
      m_GameMessages[i]->Pos = TargetPos;
      m_GameMessages[i]->bNew = false;
    } else {
      if (!TargetPos.almostEqual(m_GameMessages[i]->Pos)) {
        Vector2f F = TargetPos - m_GameMessages[i]->Pos;
        m_GameMessages[i]->Vel += F * 0.0005f;
        m_GameMessages[i]->Pos += m_GameMessages[i]->Vel;
      }
    }
    i++;
  }
}

/*===========================================================================
  Prepare the specified level for playing through this game object
  ===========================================================================*/
void Scene::prePlayLevel(DBuffer *i_recorder,
                         bool i_playEvents,
                         bool i_loadMainLayerOnly,
                         bool i_loadBSP) {
  m_playInitLevel_done = false;

  m_playEvents = i_playEvents;
  /* load the level if not */
  if (m_pLevelSrc->isFullyLoaded() == false) {
    m_pLevelSrc->loadFullyFromFile(i_loadMainLayerOnly);
  }

  /* Create Lua state */
  m_luaGame = new LuaLibGame(this);

  /* physics */
  m_physicsSettings = new PhysicsSettings("Physics/original.xml");
  // force people to compile to change settings
  if (XMFS::md5sum(FDT_DATA, "Physics/original.xml") !=
      std::string(XM_PHYSICS_MD5)) {
    throw Exception("Invalid physics settings");
  }

  /* Clear collision system */
  m_Collision.reset();
  m_pLevelSrc->setCollisionSystem(&m_Collision);

  /* Set default gravity */
  m_PhysGravity.x = 0;
  m_PhysGravity.y = -(m_physicsSettings->WorldGravity());

  m_time = 0;
  m_targetTime = 0;
  m_useTargetTime = false;
  m_checkpointStartTime = 0;
  m_floattantTimeStepDiff = 0.0;
  m_speed_factor = 1.00f;
  m_is_paused = false;

  m_nLastEventSeq = 0;

  m_Arrow.nArrowPointerMode = 0;

  m_lastCallToEveryHundreath = 0;

  /* Load and parse level script */

  for (unsigned int i = 0; i < m_pLevelSrc->scriptLibraryFileNames().size();
       i++) {
    try {
      m_luaGame->loadScriptFile("LevelsLibraries/" +
                                m_pLevelSrc->scriptLibraryFileNames()[i]);
    } catch (Exception &e) {
      throw e;
    }
  }

  if (m_pLevelSrc->scriptFileName() != "") {
    try {
      m_luaGame->loadScriptFile("./Levels/" + m_pLevelSrc->scriptFileName());
    } catch (Exception &e) {
      throw e;
    }
  }

  if (m_pLevelSrc->scriptSource() != "") {
    /* Use the Lua aux lib to load the buffer */

    try {
      m_luaGame->loadScript(m_pLevelSrc->scriptSource(),
                            m_pLevelSrc->scriptFileName());
    } catch (Exception &e) {
      std::string error_msg = m_luaGame->getErrorMsg();
      throw Exception("failed to load level encapsulated script :\n" +
                      error_msg);
    }
  }

  // load chimunk
  if (m_playEvents) {
    if (m_pLevelSrc->isPhysics()) {
      m_chipmunkWorld = new ChipmunkWorld(m_physicsSettings, m_pLevelSrc);
    }
  }

  /* Generate extended level data to be used by the game */
  try {
    _GenerateLevel(i_loadBSP);
  } catch (Exception &e) {
    LogWarning(std::string("Level generation failed !\n" + e.getMsg()).c_str());
    throw Exception(e);
  }

  m_myLastStrawberries.clear();

  /* add the debris particlesSource */
  ParticlesSource *v_debris = new ParticlesSourceDebris("BikeDebris");
  v_debris->loadToPlay();
  v_debris->setZ(1.0);
  getLevelSrc()->spawnEntity(v_debris);

  /* execute events */
  m_lastStateSerializationTime = -100; // reset the last serialization time
  m_lastStateUploadTime = -100;

  if (m_playEvents) {
    executeEvents(i_recorder);
  }
}

ReplayBiker *Scene::addReplayFromFile(std::string i_ghostFile,
                                      Theme *i_theme,
                                      BikerTheme *i_bikerTheme,
                                      bool i_enableEngineSound) {
  ReplayBiker *v_biker = NULL;
  v_biker = new ReplayBiker(
    i_ghostFile, m_physicsSettings, i_enableEngineSound, i_theme, i_bikerTheme);
  m_players.push_back(v_biker);
  return v_biker;
}

FileGhost *Scene::addGhostFromFile(std::string i_ghostFile,
                                   const std::string &i_info,
                                   bool i_isReference,
                                   Theme *i_theme,
                                   BikerTheme *i_bikerTheme,
                                   const TColor &i_filterColor,
                                   const TColor &i_filterUglyColor) {
  FileGhost *v_ghost = NULL;

  /* the level must be set to add a ghost */
  if (m_pLevelSrc == NULL) {
    throw Exception("No level defined");
  }

  v_ghost = new FileGhost(i_ghostFile,
                          m_physicsSettings,
                          false,
                          false,
                          i_theme,
                          i_bikerTheme,
                          i_filterColor,
                          i_filterUglyColor);
  v_ghost->setInfo(i_info);
  v_ghost->setReference(i_isReference);
  v_ghost->initLastToTakeEntities(m_pLevelSrc);

  // synchronize it to the good position (can be later if the ghost is
  // downloaded while playing)
  v_ghost->updateToTime(getTime(),
                        1 /* 1, not very important */,
                        &m_Collision,
                        m_PhysGravity,
                        this);

  m_ghosts.push_back(v_ghost);

  return v_ghost;
}

void Scene::initGhostTrail(FileGhost *i_ghost) {
  if (m_ghostTrail != NULL) {
    delete m_ghostTrail;
  }
  m_ghostTrail = new GhostTrail(i_ghost);
}

NetGhost *Scene::addNetGhost(const std::string &i_info,
                             Theme *i_theme,
                             BikerTheme *i_bikerTheme,
                             const TColor &i_filterColor,
                             const TColor &i_filterUglyColor) {
  NetGhost *v_ghost = NULL;
  LogInfo("New NetGhost");

  v_ghost = new NetGhost(m_physicsSettings,
                         false,
                         i_theme,
                         i_bikerTheme,
                         i_filterColor,
                         i_filterUglyColor);
  v_ghost->setInfo(i_info);
  m_ghosts.push_back(v_ghost);
  return v_ghost;
}

std::vector<Ghost *> &Scene::Ghosts() {
  return m_ghosts;
}

std::vector<Biker *> &Scene::Players() {
  return m_players;
}

bool Scene::playInitLevelDone() const {
  return m_playInitLevel_done;
}

void Scene::playInitLevel() {
  /* Invoke the OnLoad() script function */
  if (m_playEvents) {
    bool bOnLoadSuccess;
    std::string v_error_msg = "OnLoad script function failed !";

    try {
      bOnLoadSuccess = m_luaGame->scriptCallBool("OnLoad", true);
    } catch (Exception &e) {
      bOnLoadSuccess = false;
      v_error_msg += "\n" + e.getMsg();
    }
    /* if no OnLoad(), assume success */
    /* Success? */
    if (bOnLoadSuccess == false) {
      LogError(v_error_msg.c_str());
      throw Exception(v_error_msg);
    }
  }

  m_playInitLevel_done = true;
}

/*===========================================================================
  Free this game object
  ===========================================================================*/
void Scene::endLevel(void) {
  /* If not already freed */
  if (m_pLevelSrc != NULL) {
    /* Clean up */
    if (m_luaGame != NULL) {
      delete m_luaGame;
      m_luaGame = NULL;
    }
    m_pLevelSrc->unloadToPlay();
    delete m_pLevelSrc;

    /* Release reference to level source */
    m_pLevelSrc = NULL;
  }

  /* Get rid of game messages */
  for (unsigned int i = 0; i < m_GameMessages.size(); i++)
    delete m_GameMessages[i];
  m_GameMessages.clear();

  /* clean Sdynamic objects for scripts */
  cleanScriptDynamicObjects();

  /* clean event queue */
  cleanEventsQueue();

  cleanPlayers();
  cleanGhosts();
  cleanScriptTimers();

  removeCameras();

  if (m_chipmunkWorld != NULL) {
    delete m_chipmunkWorld;
    m_chipmunkWorld = NULL;
  }

  if (m_physicsSettings != NULL) {
    delete m_physicsSettings;
  }
}

/*===========================================================================
  Level generation (i.e. parsing of level source)
  ===========================================================================*/
void Scene::_GenerateLevel(bool i_loadBSP) {
  if (m_pLevelSrc == NULL) {
    LogWarning("Can't generate level when no source is assigned!");
    return;
  }

  /* Ok, our primary job is to convert the set of input blocks, which CAN
     contain concave polygons, into a final set of (possibly) smaller
     blocks that are guaranteed to be convex. */
  std::vector<Block *> &InBlocks = m_pLevelSrc->Blocks();

  /* Start by determining the bounding box of the level */
  Vector2f LevelBoundsMin, LevelBoundsMax;
  LevelBoundsMin.x = m_pLevelSrc->LeftLimit();
  LevelBoundsMax.x = m_pLevelSrc->RightLimit();
  LevelBoundsMin.y = m_pLevelSrc->BottomLimit();
  LevelBoundsMax.y = m_pLevelSrc->TopLimit();
  for (unsigned int i = 0; i < InBlocks.size(); i++) {
    for (unsigned int j = 0; j < InBlocks[i]->Vertices().size(); j++) {
      LevelBoundsMin.x = InBlocks[i]->InitialPosition().x +
                               InBlocks[i]->Vertices()[j]->Position().x <
                             LevelBoundsMin.x
                           ? InBlocks[i]->InitialPosition().x +
                               InBlocks[i]->Vertices()[j]->Position().x
                           : LevelBoundsMin.x;
      LevelBoundsMin.y = InBlocks[i]->InitialPosition().y +
                               InBlocks[i]->Vertices()[j]->Position().y <
                             LevelBoundsMin.y
                           ? InBlocks[i]->InitialPosition().y +
                               InBlocks[i]->Vertices()[j]->Position().y
                           : LevelBoundsMin.y;
      LevelBoundsMax.x = InBlocks[i]->InitialPosition().x +
                               InBlocks[i]->Vertices()[j]->Position().x >
                             LevelBoundsMax.x
                           ? InBlocks[i]->InitialPosition().x +
                               InBlocks[i]->Vertices()[j]->Position().x
                           : LevelBoundsMax.x;
      LevelBoundsMax.y = InBlocks[i]->InitialPosition().y +
                               InBlocks[i]->Vertices()[j]->Position().y >
                             LevelBoundsMax.y
                           ? InBlocks[i]->InitialPosition().y +
                               InBlocks[i]->Vertices()[j]->Position().y
                           : LevelBoundsMax.y;
    }
  }

  m_Collision.setDims(LevelBoundsMin.x,
                      LevelBoundsMin.y,
                      LevelBoundsMax.x,
                      LevelBoundsMax.y,
                      m_pLevelSrc->getNumberLayer(),
                      m_pLevelSrc->getLayerOffsets());

  LogDebug("Generating level from %d block%s...",
           InBlocks.size(),
           InBlocks.size() == 1 ? "" : "s");

  /* For each input block */
  int nTotalBSPErrors = 0;

  nTotalBSPErrors =
    m_pLevelSrc->loadToPlay(m_chipmunkWorld, m_physicsSettings, i_loadBSP);

  if (nTotalBSPErrors > 0) {
    LogWarning(" %d BSP error%s in total",
               nTotalBSPErrors,
               nTotalBSPErrors == 1 ? "" : "s");
    gameMessage(std::string(GAMETEXT_WARNING) + ":");
    gameMessage(GAMETEXT_ERRORSINLEVEL);
  }

  if (m_playEvents) {
    /* Give limits to collision system */
    m_Collision.defineLine(m_pLevelSrc->LeftLimit(),
                           m_pLevelSrc->TopLimit(),
                           m_pLevelSrc->LeftLimit(),
                           m_pLevelSrc->BottomLimit(),
                           m_physicsSettings->BikeWheelBlockGrip());
    m_Collision.defineLine(m_pLevelSrc->LeftLimit(),
                           m_pLevelSrc->BottomLimit(),
                           m_pLevelSrc->RightLimit(),
                           m_pLevelSrc->BottomLimit(),
                           m_physicsSettings->BikeWheelBlockGrip());
    m_Collision.defineLine(m_pLevelSrc->RightLimit(),
                           m_pLevelSrc->BottomLimit(),
                           m_pLevelSrc->RightLimit(),
                           m_pLevelSrc->TopLimit(),
                           m_physicsSettings->BikeWheelBlockGrip());
    m_Collision.defineLine(m_pLevelSrc->RightLimit(),
                           m_pLevelSrc->TopLimit(),
                           m_pLevelSrc->LeftLimit(),
                           m_pLevelSrc->TopLimit(),
                           m_physicsSettings->BikeWheelBlockGrip());

    /* Show stats about the collision system */
    CollisionSystemStats CStats;
    m_Collision.getStats(&CStats);
    LogDebug(" %dx%d grid with %.1fx%.1f cells (%.0f%% empty)",
             CStats.nGridWidth,
             CStats.nGridHeight,
             CStats.fCellWidth,
             CStats.fCellHeight,
             CStats.fPercentageOfEmptyCells);
    LogDebug(" %d total blocking lines", CStats.nTotalLines);
  }
}

bool Scene::touchEntityBodyExceptHead(const BikeState &pBike,
                                      const Entity &p_entity) {
  Vector2f res1, res2;

  if (pBike.Dir == DD_RIGHT) {
    if (intersectLineCircle2f(p_entity.DynamicPosition(),
                              p_entity.Size(),
                              pBike.FootP,
                              pBike.KneeP,
                              res1,
                              res2) > 0) {
      return true;
    }

    if (intersectLineCircle2f(p_entity.DynamicPosition(),
                              p_entity.Size(),
                              pBike.KneeP,
                              pBike.LowerBodyP,
                              res1,
                              res2) > 0) {
      return true;
    }

    if (intersectLineCircle2f(p_entity.DynamicPosition(),
                              p_entity.Size(),
                              pBike.LowerBodyP,
                              pBike.ShoulderP,
                              res1,
                              res2) > 0) {
      return true;
    }

    if (intersectLineCircle2f(p_entity.DynamicPosition(),
                              p_entity.Size(),
                              pBike.ShoulderP,
                              pBike.ElbowP,
                              res1,
                              res2) > 0) {
      return true;
    }

    if (intersectLineCircle2f(p_entity.DynamicPosition(),
                              p_entity.Size(),
                              pBike.ElbowP,
                              pBike.HandP,
                              res1,
                              res2) > 0) {
      return true;
    }
    return false;
  }

  if (pBike.Dir == DD_LEFT) {
    if (intersectLineCircle2f(p_entity.DynamicPosition(),
                              p_entity.Size(),
                              pBike.Foot2P,
                              pBike.Knee2P,
                              res1,
                              res2) > 0) {
      return true;
    }

    if (intersectLineCircle2f(p_entity.DynamicPosition(),
                              p_entity.Size(),
                              pBike.Knee2P,
                              pBike.LowerBody2P,
                              res1,
                              res2) > 0) {
      return true;
    }

    if (intersectLineCircle2f(p_entity.DynamicPosition(),
                              p_entity.Size(),
                              pBike.LowerBody2P,
                              pBike.Shoulder2P,
                              res1,
                              res2) > 0) {
      return true;
    }

    if (intersectLineCircle2f(p_entity.DynamicPosition(),
                              p_entity.Size(),
                              pBike.Shoulder2P,
                              pBike.Elbow2P,
                              res1,
                              res2) > 0) {
      return true;
    }

    if (intersectLineCircle2f(p_entity.DynamicPosition(),
                              p_entity.Size(),
                              pBike.Elbow2P,
                              pBike.Hand2P,
                              res1,
                              res2) > 0) {
      return true;
    }
    return false;
  }

  return false;
}

/*===========================================================================
  Update zone specific stuff -- call scripts where needed
  ===========================================================================*/
void Scene::_UpdateZones(void) {
  /* Check player touching for each zone */
  for (unsigned int i = 0; i < m_pLevelSrc->Zones().size(); i++) {
    Zone *pZone = m_pLevelSrc->Zones()[i];

    for (unsigned int j = 0; j < m_players.size(); j++) {
      Biker *v_player = m_players[j];

      if (m_players[j]->isDead() == false) {
        /* Check it against the wheels and the head */
        if (pZone->doesCircleTouch(
              v_player->getState()->FrontWheelP,
              v_player->getState()->Parameters()->WheelRadius()) ||
            pZone->doesCircleTouch(
              v_player->getState()->RearWheelP,
              v_player->getState()->Parameters()->WheelRadius()) ||
            pZone->doesCircleTouch(
              v_player->getState()->HeadP,
              v_player->getState()->Parameters()->HeadSize())) {
          /* In the zone -- did he just enter it? */
          if (v_player->setTouching(pZone, true) == PlayerLocalBiker::added) {
            createGameEvent(new MGE_PlayerEntersZone(getTime(), pZone, j));
          }
        } else {
          /* Not in the zone... but was he during last update? - i.e. has
             he just left it? */
          if (v_player->setTouching(pZone, false) ==
              PlayerLocalBiker::removed) {
            createGameEvent(new MGE_PlayerLeavesZone(getTime(), pZone, j));
          }
        }
      }
    }
  }
}

void Scene::_UpdateEntities(void) {
  for (unsigned int j = 0; j < m_players.size(); j++) {
    Biker *v_player = m_players[j];

    if (m_players[j]->isDead() == false) {
      Vector2f HeadPos = v_player->getState()->Dir == DD_RIGHT
                           ? v_player->getState()->HeadP
                           : v_player->getState()->Head2P;

      /* Get biker bounding box */
      AABB BBox;
      float headSize = v_player->getState()->Parameters()->HeadSize();
      float wheelRadius = v_player->getState()->Parameters()->WheelRadius();
      /* in case the body is outside of the aabb */
      float securityMargin = 0.5;

      BBox.addPointToAABB2f(HeadPos[0] - headSize - securityMargin,
                            HeadPos[1] - headSize - securityMargin);
      BBox.addPointToAABB2f(
        v_player->getState()->FrontWheelP[0] - wheelRadius - securityMargin,
        v_player->getState()->FrontWheelP[1] - wheelRadius - securityMargin);
      BBox.addPointToAABB2f(
        v_player->getState()->RearWheelP[0] - wheelRadius - securityMargin,
        v_player->getState()->RearWheelP[1] - wheelRadius - securityMargin);

      BBox.addPointToAABB2f(HeadPos[0] + headSize + securityMargin,
                            HeadPos[1] + headSize + securityMargin);
      BBox.addPointToAABB2f(
        v_player->getState()->FrontWheelP[0] + wheelRadius + securityMargin,
        v_player->getState()->FrontWheelP[1] + wheelRadius + securityMargin);
      BBox.addPointToAABB2f(
        v_player->getState()->RearWheelP[0] + wheelRadius + securityMargin,
        v_player->getState()->RearWheelP[1] + wheelRadius + securityMargin);

      std::vector<Entity *> entities =
        m_Collision.getEntitiesNearPosition(BBox);

      /* Do player touch anything? */
      for (unsigned int i = 0; i < entities.size(); i++) {
        /* Test against the biker aabb first */
        if (true) {
          /* Head? */
          if (circleTouchCircle2f(
                entities[i]->DynamicPosition(),
                entities[i]->Size(),
                HeadPos,
                v_player->getState()->Parameters()->HeadSize())) {
            if (v_player->setTouching(entities[i], true) ==
                PlayerLocalBiker::added) {
              createGameEvent(new MGE_PlayerTouchesEntity(
                getTime(), entities[i]->Id(), true, j));
            }

            /* Wheel then ? */
          } else if (circleTouchCircle2f(
                       entities[i]->DynamicPosition(),
                       entities[i]->Size(),
                       v_player->getState()->FrontWheelP,
                       v_player->getState()->Parameters()->WheelRadius()) ||
                     circleTouchCircle2f(
                       entities[i]->DynamicPosition(),
                       entities[i]->Size(),
                       v_player->getState()->RearWheelP,
                       v_player->getState()->Parameters()->WheelRadius())) {
            if (v_player->setTouching(entities[i], true) ==
                PlayerLocalBiker::added) {
              createGameEvent(new MGE_PlayerTouchesEntity(
                getTime(), entities[i]->Id(), false, j));
            }

            /* body then ?*/
          } else if (touchEntityBodyExceptHead(*(v_player->getState()),
                                               *(entities[i]))) {
            if (v_player->setTouching(entities[i], true) ==
                PlayerLocalBiker::added) {
              createGameEvent(new MGE_PlayerTouchesEntity(
                getTime(), entities[i]->Id(), false, j));
            }
          } else {
            /* TODO::generate an event "leaves entity" if needed */
            v_player->setTouching(entities[i], false);
          }
        } else {
          /* TODO::generate an event "leaves entity" if needed */
          v_player->setTouching(entities[i], false);
        }
      }
    }
  }
}

/*===========================================================================
  Entity stuff (public)
  ===========================================================================*/
void Scene::deleteEntity(Entity *pEntity) {
  /* Already scheduled for deletion? */
  for (unsigned int i = 0; i < m_DelSchedule.size(); i++)
    if (m_DelSchedule[i] == pEntity)
      return;
  m_DelSchedule.push_back(pEntity);
}

void Scene::touchEntity(int i_player, Entity *pEntity, bool bHead) {
  /* Start by invoking scripts if any */
  if (m_playEvents) {
    m_luaGame->scriptCallTblVoid(pEntity->Id(), "Touch");
    m_luaGame->scriptCallTblVoid(pEntity->Id(), "TouchBy", i_player);

    if (pEntity->DoesMakeWin()) {
      if (getNbRemainingStrawberries() == 0) {
        createGameEvent(new MGE_PlayerWins(getTime(), i_player));
      }
    }

    if (pEntity->DoesKill()) {
      createGameEvent(new MGE_PlayerDies(getTime(), true, i_player));
    }

    if (pEntity->IsToTake()) {
      /* OH... nice */
      createGameEvent(new MGE_EntityDestroyed(getTime(),
                                              pEntity->Id(),
                                              pEntity->Speciality(),
                                              pEntity->DynamicPosition(),
                                              pEntity->Size(),
                                              i_player));
    }

    if ((Checkpoint *)pEntity->IsCheckpoint()) {
      if (m_checkpoint != NULL) {
        m_checkpoint->deactivate();
      }
      Checkpoint *v_checkpoint = (Checkpoint *)pEntity;
      v_checkpoint->activate(m_pLevelSrc->EntitiesDestroyed(),
                             m_players[i_player]->getState()->Dir);
      if (m_motoGameHooks != NULL) {
        m_motoGameHooks->OnTakeCheckpoint(i_player);
      }
      m_checkpoint = v_checkpoint;
    }
  }
}

void Scene::createGameEvent(SceneEvent *p_event) {
  m_GameEventQueue.push_back(p_event);
}

void Scene::destroyGameEvent(SceneEvent *p_event) {
  delete p_event;
}

void Scene::cleanEventsQueue() {
  for (unsigned int i = 0; i < m_GameEventQueue.size(); i++) {
    destroyGameEvent(m_GameEventQueue[i]);
  }
  m_GameEventQueue.clear();
}

void Scene::handleEvent(SceneEvent *pEvent) {
  switch (pEvent->getType()) {
    case GAME_EVENT_PLAYER_TOUCHES_ENTITY:
    case GAME_EVENT_PLAYERS_TOUCHE_ENTITY:
      /* touching an entity creates events, so, don't call it */
      break;
    default:
      pEvent->doAction(this);
      break;
  }
}

void Scene::SetEntityPos(std::string pEntityID, float pX, float pY) {
  SetEntityPos(m_pLevelSrc->getEntityById(pEntityID), pX, pY);
}

void Scene::SetEntityPos(Entity *pEntity, float pX, float pY) {
  pEntity->setDynamicPosition(Vector2f(pX, pY));
  // move it in the collision system only if it's not dead
  if (pEntity->isAlive() == true) {
    m_Collision.moveEntity(pEntity);
  }
}

void Scene::translateEntity(std::string pEntityID, float x, float y) {
  translateEntity(m_pLevelSrc->getEntityById(pEntityID), x, y);
}

void Scene::translateEntity(Entity *pEntity, float x, float y) {
  pEntity->translate(x, y);
  if (pEntity->isAlive() == true) {
    m_Collision.moveEntity(pEntity);
  }
}

void Scene::PlaceInGameArrow(float pX, float pY, float pAngle) {
  getArrowPointer().nArrowPointerMode = 1;
  getArrowPointer().ArrowPointerPos = Vector2f(pX, pY);
  getArrowPointer().fArrowPointerAngle = pAngle;
}

void Scene::PlaceScreenArrow(float pX, float pY, float pAngle) {
  getArrowPointer().nArrowPointerMode = 2;
  getArrowPointer().ArrowPointerPos = Vector2f(pX, pY);
  getArrowPointer().fArrowPointerAngle = pAngle;
}

void Scene::HideArrow() {
  getArrowPointer().nArrowPointerMode = 0;
}

void Scene::MoveBlock(std::string pBlockID, float pX, float pY) {
  Block *v_block = m_pLevelSrc->getBlockById(pBlockID);
  MoveBlock(v_block, pX, pY);
}

void Scene::MoveBlock(Block *pBlock, float pX, float pY) {
  if (pBlock->isDynamic() == true) {
    pBlock->translate(pX, pY);
    m_Collision.moveDynBlock(pBlock);
  }
}

void Scene::SetBlockPos(std::string pBlockID, float pX, float pY) {
  Block *v_block = m_pLevelSrc->getBlockById(pBlockID);
  SetBlockPos(v_block, pX, pY);
}

void Scene::SetBlockPos(Block *pBlock, float pX, float pY) {
  if (pBlock->isDynamic() == true) {
    pBlock->setDynamicPositionAccordingToCenter(Vector2f(pX, pY));

    if (m_chipmunkWorld != NULL) {
      if (pBlock->isPhysics()) {
        pBlock->setPhysicsPosition(pX, pY);
      }
    }
    m_Collision.moveDynBlock(pBlock);
  }
}

void Scene::SetBlockCenter(std::string pBlockID, float pX, float pY) {
  Block *v_block = m_pLevelSrc->getBlockById(pBlockID);
  if (v_block->isDynamic() == true) {
    v_block->setCenter(Vector2f(pX, pY));
    m_Collision.moveDynBlock(v_block);
  }
}

void Scene::SetBlockRotation(std::string pBlockID, float pAngle) {
  Block *v_block = m_pLevelSrc->getBlockById(pBlockID);
  SetBlockRotation(v_block, pAngle);
}

void Scene::SetBlockRotation(Block *pBlock, float pAngle) {
  if (pBlock->isDynamic() == true) {
    if (pBlock->setDynamicRotation(pAngle) == true)
      m_Collision.moveDynBlock(pBlock);
  }
}

void Scene::SetEntityDrawAngle(std::string pEntityID, float pAngle) {
  Entity *v_entity;
  v_entity = getLevelSrc()->getEntityById(pEntityID);
  v_entity->setDrawAngle(pAngle);
}

void Scene::DisplayDiffFromGhost() {
  bool v_diffAvailable = false;

  if (m_ghosts.size() > 0) {
    float v_diffToGhost = 0.0;

    /* take the more */
    for (unsigned int i = 0; i < m_ghosts.size(); i++) {
      if (m_ghosts[i]->diffToPlayerAvailable() && m_ghosts[i]->isReference()) {
        if (v_diffAvailable == false ||
            m_ghosts[i]->diffToPlayer() > v_diffToGhost) {
          v_diffToGhost = m_ghosts[i]->diffToPlayer();
          v_diffAvailable = true;
        }
      }
    }

    if (v_diffAvailable) {
      char msg[256];
      snprintf(msg, 256, "%+.2f", v_diffToGhost / 100.0);
      gameMessage(
        msg, true, MOTOGAME_DEFAULT_GAME_MESSAGE_DURATION / 2, gameTime);
    }
  }
}

void Scene::cleanScriptDynamicObjects() {
  for (unsigned int i = 0; i < m_SDynamicObjects.size(); i++) {
    delete m_SDynamicObjects[i];
  }
  m_SDynamicObjects.clear();
}

void Scene::nextStateScriptDynamicObjects(int i_nbCents) {
  unsigned int i = 0;

  while (i < m_SDynamicObjects.size()) {
    if (m_SDynamicObjects[i]->nextState(this, i_nbCents) == false) {
      delete m_SDynamicObjects[i];
      m_SDynamicObjects.erase(m_SDynamicObjects.begin() + i);
    } else {
      i++;
    }
  }
}

void Scene::removeSDynamicOfObject(std::string pObject) {
  unsigned int i = 0;

  while (i < m_SDynamicObjects.size()) {
    if (m_SDynamicObjects[i]->getObjectId() == pObject) {
      delete m_SDynamicObjects[i];
      m_SDynamicObjects.erase(m_SDynamicObjects.begin() + i);
    } else {
      i++;
    }
  }
}

void Scene::CameraZoom(float pZoom) {
  getCamera()->desactiveActionZoom();
  getCamera()->setRelativeZoom(pZoom);
}

void Scene::CameraMove(float p_x, float p_y) {
  getCamera()->desactiveActionZoom();
  getCamera()->setUseTrailCam(false);
  getCamera()->moveCamera(p_x, p_y);
}

void Scene::CameraSetPos(float p_x, float p_y) {
  getCamera()->desactiveActionZoom();
  getCamera()->setUseTrailCam(false);
  getCamera()->setCameraPosition(p_x, p_y);
}

void Scene::CameraRotate(float i_angle) {
  getCamera()->setDesiredRotationAngle(i_angle);
}

void Scene::CameraAdaptToGravity() {
  getCamera()->adaptRotationAngleToGravity(m_PhysGravity);
}

void Scene::resussitePlayer(int i_player) {
  m_players[i_player]->setDead(false);

  m_players[i_player]->initWheelDetach();

  // inform camera that the player is not dead
  for (unsigned int i = 0; i < m_cameras.size(); i++) {
    if (m_cameras[i]->getPlayerToFollow() == m_players[i_player]) {
      m_cameras[i]->setPlayerResussite();
    }
  }
}

void Scene::killPlayer(int i_player) {
  if (m_players[i_player]->isDead() == false &&
      m_players[i_player]->isFinished() == false) {
    m_players[i_player]->setDead(true, getTime());
    if (m_motoGameHooks != NULL) {
      m_motoGameHooks->OnPlayerDies(i_player);
    }
    m_players[i_player]->setBodyDetach(true);
    m_players[i_player]->getControler()->stopControls();

    // inform camera that the player dies (for the following point)
    for (unsigned int i = 0; i < m_cameras.size(); i++) {
      if (m_cameras[i]->getPlayerToFollow() == m_players[i_player]) {
        m_cameras[i]->setPlayerDead();
      }
    }
  }
}

void Scene::addForceToPlayer(int i_player,
                             const Vector2f &i_force,
                             int i_startTime,
                             int i_endTime) {
  m_players[i_player]->addBodyForce(m_time, i_force, i_startTime, i_endTime);
}

void Scene::playerEntersZone(int i_player, Zone *pZone) {
  if (m_playEvents) {
    m_luaGame->scriptCallTblVoid(pZone->Id(), "OnEnter");
    m_luaGame->scriptCallTblVoid(pZone->Id(), "OnEnterBy", i_player);
  }
}

void Scene::playerLeavesZone(int i_player, Zone *pZone) {
  if (m_playEvents) {
    m_luaGame->scriptCallTblVoid(pZone->Id(), "OnLeave");
    m_luaGame->scriptCallTblVoid(pZone->Id(), "OnLeaveBy", i_player);
  }
}

void Scene::playerTouchesEntity(int i_player,
                                std::string p_entityID,
                                bool p_bTouchedWithHead) {
  touchEntity(
    i_player, getLevelSrc()->getEntityById(p_entityID), p_bTouchedWithHead);
}

void Scene::entityDestroyed(
  const std::string &i_entityId,
  int i_time,
  int i_takenByPlayer /* -1 if taken by an external event */) {
  Entity *v_entity;
  v_entity = getLevelSrc()->getEntityById(i_entityId);

  if (v_entity->IsToTake()) {
    ParticlesSource *v_stars = new ParticlesSourceStar("");
    v_stars->setInitialPosition(v_entity->DynamicPosition());
    v_stars->loadToPlay();
    v_stars->setZ(1.0);
    for (int i = 0; i < 3; i++) {
      v_stars->addParticle(m_time);
    }
    getLevelSrc()->spawnEntity(v_stars);

    if (m_motoGameHooks != NULL) {
      m_motoGameHooks->OnEntityToTakeDestroyed();

      if (i_takenByPlayer == -1) {
        m_motoGameHooks->OnEntityToTakeTakenExternal();
      } else {
        m_motoGameHooks->OnEntityToTakeTakenByPlayer(i_takenByPlayer);
      }
    }

    /* update timediff */
    m_myLastStrawberries.push_back(i_time);

    for (unsigned int i = 0; i < m_ghosts.size(); i++) {
      m_ghosts[i]->updateDiffToPlayer(m_myLastStrawberries);
    }

    if (m_showGhostTimeDiff) {
      DisplayDiffFromGhost();
    }
  }

  /* Destroy entity */
  deleteEntity(v_entity);
}

void Scene::addDynamicObject(SDynamicObject *p_obj) {
  m_SDynamicObjects.push_back(p_obj);
}

void Scene::createExternalKillEntityEvent(std::string p_entityID) {
  Entity *v_entity;
  v_entity = m_pLevelSrc->getEntityById(p_entityID);
  createGameEvent(new MGE_EntityDestroyed(getTime(),
                                          v_entity->Id(),
                                          v_entity->Speciality(),
                                          v_entity->DynamicPosition(),
                                          v_entity->Size(),
                                          -1));
}

unsigned int Scene::getNbRemainingStrawberries() {
  return m_pLevelSrc->countToTakeEntities();
}

void Scene::makePlayerWin(int i_player) {
  if (m_players[i_player]->isDead() == false &&
      m_players[i_player]->isFinished() == false) {
    m_players[i_player]->setFinished(true, getTime());
    if (m_motoGameHooks != NULL) {
      m_motoGameHooks->OnPlayerWins(i_player);
    }
  }
}

void Scene::addPenaltyTime(int i_time) {
  if (i_time > 0) {
    m_time += i_time;
  }
}

void Scene::_KillEntity(Entity *pEnt) {
  getLevelSrc()->killEntity(pEnt->Id());
  /* now that rendering use the space partionnement,
     we have to remove entity from the collision system */
  m_Collision.removeEntity(pEnt);

  /* special case for the last strawberry : if we are touching the end, finish
   * the level */
  if (getNbRemainingStrawberries() == 0) {
    for (unsigned int j = 0; j < m_players.size(); j++) {
      if (m_players[j]->isDead() == false) {
        for (unsigned int i = 0; i < m_players[j]->EntitiesTouching().size();
             i++) {
          if (m_players[j]->EntitiesTouching()[i]->DoesMakeWin()) {
            createGameEvent(new MGE_PlayerWins(getTime(), j));
          }
        }
      }
    }
  }
}

PlayerLocalBiker *Scene::addPlayerLocalBiker(int i_localNetId,
                                             Vector2f i_position,
                                             DriveDir i_direction,
                                             Theme *i_theme,
                                             BikerTheme *i_bikerTheme,
                                             const TColor &i_filterColor,
                                             const TColor &i_filterUglyColor,
                                             bool i_enableEngineSound) {
  PlayerLocalBiker *v_playerBiker = new PlayerLocalBiker(m_physicsSettings,
                                                         i_position,
                                                         i_direction,
                                                         m_PhysGravity,
                                                         i_enableEngineSound,
                                                         i_theme,
                                                         i_bikerTheme,
                                                         i_filterColor,
                                                         i_filterUglyColor);
  v_playerBiker->setOnBikerHooks(new SceneOnBikerHooks(this, m_players.size()));
  v_playerBiker->setLocalNetId(i_localNetId);
  m_players.push_back(v_playerBiker);

  if (m_chipmunkWorld != NULL) {
    m_chipmunkWorld->addPlayer(v_playerBiker);
  }

  return v_playerBiker;
}

PlayerNetClient *Scene::addPlayerNetClient(Vector2f i_position,
                                           DriveDir i_direction,
                                           Theme *i_theme,
                                           BikerTheme *i_bikerTheme,
                                           const TColor &i_filterColor,
                                           const TColor &i_filterUglyColor,
                                           bool i_enableEngineSound) {
  PlayerNetClient *v_playerNetClient = new PlayerNetClient(m_physicsSettings,
                                                           i_position,
                                                           i_direction,
                                                           m_PhysGravity,
                                                           i_enableEngineSound,
                                                           i_theme,
                                                           i_bikerTheme,
                                                           i_filterColor,
                                                           i_filterUglyColor);
  m_players.push_back(v_playerNetClient);

  return v_playerNetClient;
}

void Scene::setGravity(float x, float y) {
  m_PhysGravity.x = x;
  m_PhysGravity.y = y;

  for (unsigned int i = 0; i < m_players.size(); i++) {
    if (m_players[i]->isDead() == false) {
      m_players[i]->resetAutoDisabler();
    }
  }

  // change gravity for chipmunk
  if (m_chipmunkWorld != NULL) {
    m_chipmunkWorld->setGravity(x, y);
  }

  // m_renderer->adaptRotationAngleToGravity();
}

const Vector2f &Scene::getGravity() {
  return m_PhysGravity;
}

void Scene::pause() {
  m_is_paused = !m_is_paused;
}

bool Scene::isPaused() {
  return m_is_paused;
}

float Scene::getSpeed() const {
  if (m_is_paused) {
    return 0.0;
  }
  return m_speed_factor;
}

void Scene::slower(float i_increment) {
  if (m_is_paused == false) {
    m_speed_factor -= i_increment;
  }
  if (getLevelSrc() != NULL) {
    if (getLevelSrc()->isScripted() || getLevelSrc()->isPhysics()) {
      if (m_speed_factor < 0.0)
        m_speed_factor = 0.0;
    }
  }
}

void Scene::faster(float i_increment) {
  if (m_is_paused == false) {
    m_speed_factor += i_increment;
  }
}

void Scene::fastforward(int i_time) {
  m_time += i_time;
}

void Scene::fastrewind(int i_time) {
  if (getLevelSrc() != NULL) {
    if (getLevelSrc()->isScripted() == false &&
        getLevelSrc()->isPhysics() == false) {
      m_time -= i_time;
      if (m_time < 0)
        m_time = 0;
      onRewinding();
    }
  }
}

void Scene::onRewinding() {
  bool v_continue = true;
  int i = m_myLastStrawberries.size() - 1;

  // remove strawberries untaken
  while (v_continue && i >= 0) { // must be at least one strawberry
    // > because updateToTime is done after m_time increase
    if (m_myLastStrawberries[i] > m_time) {
      m_myLastStrawberries.erase(m_myLastStrawberries.begin() + i);
      i--;
    } else {
      v_continue = false;
    }
  }

  // update diffs
  for (unsigned int i = 0; i < m_ghosts.size(); i++) {
    m_ghosts[i]->updateDiffToPlayer(m_myLastStrawberries);
  }

  // update times of messages
  clearGameMessages();
}

bool Scene::doesPlayEvents() const {
  return m_playEvents;
}

void Scene::setInfos(const std::string &i_infos) {
  m_infos = i_infos;
}

std::string Scene::getInfos() const {
  return m_infos;
}

LuaLibGame *Scene::getLuaLibGame() {
  return m_luaGame;
}

Camera *Scene::getCamera() {
  return m_cameras[m_currentCamera];
}
unsigned int Scene::getNumberCameras() {
  // the last camera is the autozoom one
  if (m_cameras.size() <= 1) {
    return m_cameras.size();
  }
  return m_cameras.size() - 1;
}
void Scene::setCurrentCamera(unsigned int currentCamera) {
  m_currentCamera = currentCamera;
}
unsigned int Scene::getCurrentCamera() {
  return m_currentCamera;
}
void Scene::addCamera(Vector2i upperleft,
                      Vector2i downright,
                      bool i_useActiveZoom) {
  Camera *i_cam = new Camera(upperleft, downright);
  i_cam->allowActiveZoom(i_useActiveZoom);
  m_cameras.push_back(i_cam);
  m_cameras.back()->initCamera();
}
void Scene::resetFollow() {
  for (unsigned int i = 0; i < m_cameras.size(); i++) {
    m_cameras[i]->setPlayerToFollow(NULL);
  }
}
void Scene::removeCameras() {
  for (unsigned int i = 0; i < m_cameras.size(); i++) {
    delete m_cameras[i];
  }
  m_cameras.clear();
}
void Scene::setAutoZoomCamera() {
  if (m_cameras.size() == 1) {
    setCurrentCamera(0);
  } else {
    // the last camera is the autozoom one
    setCurrentCamera(m_cameras.size() - 1);
  }
}

bool Scene::isAutoZoomCamera() {
  return (getCurrentCamera() == getNumberCameras());
}

std::vector<Camera *> &Scene::Cameras() {
  return m_cameras;
}

PhysicsSettings *Scene::getPhysicsSettings() {
  return m_physicsSettings;
}

void Scene::onSomersaultDone(unsigned int i_player, bool i_counterclock) {
  if (doesPlayEvents() == false) {
    return;
  }
  getLuaLibGame()->scriptCallVoidNumberArg("OnSomersault",
                                           i_counterclock ? 1 : 0);
  getLuaLibGame()->scriptCallVoidNumberArg(
    "OnSomersaultBy", i_counterclock ? 1 : 0, i_player);
  m_motoGameHooks->OnPlayerSomersault(i_player, i_counterclock);
}

SceneOnBikerHooks::SceneOnBikerHooks(Scene *i_motoGame, int i_playerNumber) {
  m_motoGame = i_motoGame;
  m_playerNumber = i_playerNumber;
}

SceneOnBikerHooks::~SceneOnBikerHooks() {}

void SceneOnBikerHooks::onSomersaultDone(bool i_counterclock) {
  m_motoGame->onSomersaultDone(m_playerNumber, i_counterclock);
}

void SceneOnBikerHooks::onWheelTouches(int i_wheel, bool i_touch) {
  if (m_motoGame->doesPlayEvents() == false)
    return;

  if (i_wheel == 1) {
    if (i_touch) {
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg("OnWheel1Touchs", 1);
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg(
        "OnWheel1TouchsBy", 1, m_playerNumber);
    } else {
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg("OnWheel1Touchs", 0);
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg(
        "OnWheel1TouchsBy", 0, m_playerNumber);
    }
  } else {
    if (i_touch) {
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg("OnWheel2Touchs", 1);
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg(
        "OnWheel2TouchsBy", 1, m_playerNumber);
    } else {
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg("OnWheel2Touchs", 0);
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg(
        "OnWheel2TouchsBy", 0, m_playerNumber);
    }
  }
}

void SceneOnBikerHooks::onHeadTouches() {
  if (m_motoGame->Players()[m_playerNumber]->isDead() == false) {
    m_motoGame->createGameEvent(
      new MGE_PlayerDies(m_motoGame->getTime(), false, m_playerNumber));
  }
}

/*===========================================================================*/
/* ScriptTimer methods (public)                                              */
/*===========================================================================*/
ScriptTimer *Scene::getScriptTimerByName(std::string TimerName) {
  for (unsigned int i = 0; i < m_ScriptTimers.size(); i++) {
    if (m_ScriptTimers[i]->GetName() == TimerName) {
      return m_ScriptTimers[i];
    }
  }
  return NULL;
}

void Scene::createScriptTimer(std::string TimerName, float delay, int loops) {
  m_ScriptTimers.push_back(
    new ScriptTimer(TimerName, delay, loops, m_luaGame, getTime()));
}

void Scene::cleanScriptTimers() {
  for (unsigned int i = 0; i < m_ScriptTimers.size(); i++) {
    delete m_ScriptTimers[i];
  }
  m_ScriptTimers.clear();
}

Checkpoint *Scene::getCheckpoint() {
  return m_checkpoint;
}

void Scene::playToCheckpoint() {
  if (m_checkpoint == NULL) {
    return;
  }

  // get rid of former messages and the level restart messages
  clearGameMessages();
  gameMessage(" ", true, 0);

  for (unsigned int i = 0; i < Players().size(); i++) {
    setPlayerPosition(i,
                      m_checkpoint->DynamicPosition().x,
                      m_checkpoint->DynamicPosition().y,
                      m_checkpoint->getDirection() == DD_RIGHT);
    getCamera()->initCamera();
  }

  // put strawberries
  std::vector<std::string> v_entityToRevert;
  for (unsigned int i = 0; i < getLevelSrc()->EntitiesDestroyed().size(); i++) {
    bool v_found = false;
    for (unsigned int j = 0; j < m_checkpoint->getDestroyedEntities().size();
         j++) {
      if (getLevelSrc()->EntitiesDestroyed()[i] ==
          m_checkpoint->getDestroyedEntities()[j]) {
        v_found = true;
      }
    }

    if (v_found == false) {
      v_entityToRevert.push_back(getLevelSrc()->EntitiesDestroyed()[i]->Id());
    }
  }

  // revert the entity -- done in 2 times to not remove entity while beeing
  // looping them
  for (unsigned int i = 0; i < v_entityToRevert.size(); i++) {
    getLevelSrc()->revertEntityDestroyed(v_entityToRevert[i]);
  }

  // call the checkpoint.OnUse
  m_luaGame->scriptCallTblVoid(m_checkpoint->Id(), "OnUse");

  m_checkpointStartTime = m_time;
}

void Scene::addRequestedGhost(GhostsAddInfos i_ghostInfo) {
  m_requestedGhosts.push_back(i_ghostInfo);
}
std::vector<GhostsAddInfos> &Scene::RequestedGhosts() {
  return m_requestedGhosts;
}

void Scene::removeRequestedGhost(const std::string &i_ghostName) {
  unsigned int i = 0;

  while (i < m_requestedGhosts.size()) {
    if (m_requestedGhosts[i].name == i_ghostName) {
      m_requestedGhosts.erase(m_requestedGhosts.begin() + i);
    } else {
      i++;
    }
  }
}
