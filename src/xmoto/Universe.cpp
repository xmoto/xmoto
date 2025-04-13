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

#include "Universe.h"
#include "Game.h"
#include "GameText.h"
#include "Renderer.h"
#include "Replay.h"
#include "Sound.h"
#include "SysMessage.h"
#include "common/Theme.h"
#include "db/xmDatabase.h"
#include "drawlib/DrawLib.h"
#include "helpers/Log.h"
#include "helpers/RenderSurface.h"
#include "states/GameState.h"
#include "xmscene/Camera.h"
#include "xmscene/Level.h"

void XMSceneHooks::OnEntityToTakeDestroyed() {
  /* Play yummy-yummy sound */
  try {
    Sound::playSampleByName(
      Theme::instance()
        ->getSound(m_Scene->getLevelSrc()->SoundForPickUpStrawberry())
        ->FilePath());
  } catch (Exception &e) {
  }
}

void XMSceneHooks::OnTakeCheckpoint(unsigned int i_player) {
  /* Play yummy-yummy sound */
  try {
    Sound::playSampleByName(
      Theme::instance()
        ->getSound(m_Scene->getLevelSrc()->SoundForCheckpoint())
        ->FilePath());
  } catch (Exception &e) {
  }
}

void XMSceneHooks::setGameApps(Scene *i_Scene) {
  m_Scene = i_Scene;
}

XMSceneHooks::XMSceneHooks() {
  m_Scene = NULL;
}

XMSceneHooks::~XMSceneHooks() {}

Universe::Universe() {
  m_pJustPlayReplay = NULL;
  m_waitingForGhosts = false;
}

Universe::~Universe() {
  removeAllWorlds();

  if (m_pJustPlayReplay != NULL) {
    delete m_pJustPlayReplay;
  }
}

void Universe::addScene() {
  Scene *v_motoGame = new Scene();
  XMSceneHooks *v_motoGameHooks = new XMSceneHooks();

  /* Tell collision system whether we want debug-info or not */
  v_motoGame->getCollisionHandler()->setDebug(XMSession::instance()->debug());
  v_motoGame->setHooks(v_motoGameHooks);
  v_motoGameHooks->setGameApps(v_motoGame);

  m_scenes.push_back(v_motoGame);
  m_motoGameHooks.push_back(v_motoGameHooks);
}

void Universe::removeAllWorlds() {
  for (unsigned int i = 0; i < m_scenes.size(); i++) {
    m_scenes[i]->resetFollow();
    m_scenes[i]->endLevel();
    delete m_scenes[i];
  }
  m_scenes.clear();

  for (unsigned int i = 0; i < m_motoGameHooks.size(); i++) {
    delete m_motoGameHooks[i];
  }
  m_motoGameHooks.clear();
}

void Universe::initPlayServer() {
  // just one scene for all players
  addScene();
}

void Universe::initPlay(RenderSurface *i_screen,
                        int i_nbPlayer,
                        bool i_multiScenes) {
  if (i_multiScenes) {
    for (int i = 0; i < i_nbPlayer; i++) {
      addScene();
    }
  } else {
    // only one scene
    addScene();
  }

  initCameras(i_screen, i_nbPlayer);
}

void Universe::initCameras(RenderSurface *i_screen, int nbPlayer) {
  int width =
    i_screen
      ->getDispWidth(); // probably not the best way, but this is over screen
  int height =
    i_screen
      ->getDispHeight(); // probably not the best way, but this is over screen
  bool v_useActiveZoom = XMSession::instance()->enableActiveZoom();

  if (m_scenes.size() <= 0) {
    return;
  }

  /* Cameras for single and multi player:
     single player:
                    have just one camera (scene[0]),
                    which can be autozoom cam at the same time
                    (b/c its as big as screen size)
     multi player:
                    have up to 4 cameras for players, and one autozoom cam:
           single scene (coop mode):
                    all in the same scene (scene[0]->cameras[0..4]) with 4
     players
                    autozoom cam is scene[0]->camera[4]
           multi scenes (normal mode):
                    4 player cams in 4 scenes (scene[0..3]->cameras[0])
                    autozoom cam is scene[0]->camera[1] for 2 and 4 players
                               but  scene[0]->camera[2] for 3 players, because
                               the camera[1] is used for duplicate player 1 cam
                               for 4th player field (otherwise we get ugly
     drawing)  */
  switch (nbPlayer) {
    default:
    case 1:
      m_scenes[0]->addCamera(
        i_screen->downleft(), i_screen->upright(), v_useActiveZoom);
      break;
    case 2:
      m_scenes[0]->addCamera(
        Vector2i(i_screen->downleft().x, i_screen->downleft().y + height / 2),
        i_screen->upright(),
        v_useActiveZoom);
      if (m_scenes.size() == 1) {
        m_scenes[0]->addCamera(
          i_screen->downleft(),
          Vector2i(i_screen->upright().x, i_screen->upright().y - height / 2),
          v_useActiveZoom);
      } else {
        m_scenes[1]->addCamera(
          i_screen->downleft(),
          Vector2i(i_screen->upright().x, i_screen->upright().y - height / 2),
          v_useActiveZoom);
      }
      break;
    case 3:
    case 4:
      if (m_scenes.size() == 1) {
        m_scenes[0]->addCamera(
          Vector2i(i_screen->downleft().x, i_screen->downleft().y + height / 2),
          Vector2i(i_screen->downleft().x + width / 2, i_screen->upright().y),
          v_useActiveZoom);
        m_scenes[0]->addCamera(Vector2i(i_screen->downleft().x + width / 2,
                                        i_screen->downleft().y + height / 2),
                               i_screen->upright(),
                               v_useActiveZoom);
        m_scenes[0]->addCamera(i_screen->downleft(),
                               Vector2i(i_screen->downleft().x + width / 2,
                                        i_screen->downleft().y + height / 2),
                               v_useActiveZoom);
        m_scenes[0]->addCamera(
          Vector2i(i_screen->downleft().x + width / 2, i_screen->downleft().y),
          Vector2i(i_screen->upright().x, i_screen->downleft().y + height / 2),
          v_useActiveZoom);

      } else {
        m_scenes[0]->addCamera(
          Vector2i(i_screen->downleft().x, i_screen->downleft().y + height / 2),
          Vector2i(i_screen->downleft().x + width / 2, i_screen->upright().y),
          v_useActiveZoom);
        m_scenes[1]->addCamera(Vector2i(i_screen->downleft().x + width / 2,
                                        i_screen->downleft().y + height / 2),
                               i_screen->upright(),
                               v_useActiveZoom);
        m_scenes[2]->addCamera(i_screen->downleft(),
                               Vector2i(i_screen->downleft().x + width / 2,
                                        i_screen->downleft().y + height / 2),
                               v_useActiveZoom);

        if (nbPlayer == 4) {
          m_scenes[3]->addCamera(Vector2i(i_screen->downleft().x + width / 2,
                                          i_screen->downleft().y),
                                 Vector2i(i_screen->upright().x,
                                          i_screen->downleft().y + height / 2),
                                 v_useActiveZoom);
        } else { // 3 player then, add another cam for not having an ugly
          // not-drawn rect in the screen
          m_scenes[0]->addCamera(Vector2i(i_screen->downleft().x + width / 2,
                                          i_screen->downleft().y),
                                 Vector2i(i_screen->upright().x,
                                          i_screen->downleft().y + height / 2),
                                 v_useActiveZoom);
        }
      }
      break;
  }

  // the autozoom camera is a special one in multi player
  if (nbPlayer > 1) {
    m_scenes[0]->addCamera(
      i_screen->downleft(), i_screen->upright(), v_useActiveZoom);
  }
  // current cam is autozoom one
  for (unsigned int i = 0; i < m_scenes.size(); i++) {
    m_scenes[i]->setAutoZoomCamera();
  }

  // init trailcam
  for (unsigned int i = 0; i < m_scenes.size(); i++) {
    for (unsigned int j = 0; j < m_scenes[i]->Cameras().size(); j++) {
      m_scenes[i]->Cameras()[j]->setUseTrailCam(
        XMSession::instance()->enableTrailCam());
    }
  }
}

void Universe::deleteCurrentReplay() {
  if (m_pJustPlayReplay != NULL) {
    delete m_pJustPlayReplay;
    m_pJustPlayReplay = NULL;
  }
}

void Universe::TeleportationCheatTo(int i_player, Vector2f i_position) {
  if (m_scenes.size() > 0) {
    m_scenes[0]->setPlayerPosition(i_player, i_position.x, i_position.y, true);
    m_scenes[0]->getCamera()->initCamera();
    // m_scenes[0]->addPenaltyTime(90000); /* 15 min of penalty for that ! */
    /* ... and no replay of course ! */
    deleteCurrentReplay();
  }
}

void Universe::switchFollowCameraScene(Scene *i_scene) {
  Camera *pCamera = i_scene->getCamera();

  if (pCamera->getPlayerToFollow() == NULL) {
    return;
  }

  std::vector<Biker *> &players = i_scene->Players();
  std::vector<Ghost *> &ghosts = i_scene->Ghosts();
  unsigned int sizePlayers = players.size();
  unsigned int sizeGhosts = ghosts.size();

  /* search into the player */
  for (unsigned i = 0; i < sizePlayers; i++) {
    if (players[i] == pCamera->getPlayerToFollow()) {
      if (i < sizePlayers - 1) {
        pCamera->setPlayerToFollow(players[i + 1]);
      } else {
        if (sizeGhosts > 0) {
          pCamera->setPlayerToFollow(ghosts[0]);
        } else {
          pCamera->setPlayerToFollow(players[0]);
        }
      }
      return;
    }
  }

  /* search into the ghost */
  for (unsigned i = 0; i < sizeGhosts; i++) {
    if (ghosts[i] == pCamera->getPlayerToFollow()) {
      if (i < sizeGhosts - 1) {
        pCamera->setPlayerToFollow(ghosts[i + 1]);
      } else {
        if (sizePlayers > 0) {
          pCamera->setPlayerToFollow(players[0]);
        } else {
          pCamera->setPlayerToFollow(ghosts[0]);
        }
      }
      return;
    }
  }
}

void Universe::switchFollowCamera() {
  if (m_scenes.size() > 0) {
    switchFollowCameraScene(m_scenes[0]);

    SysMessage::instance()->displayText(
      m_scenes[0]->getCamera()->getPlayerToFollow()->getQuickDescription());
  }
}

void Universe::isTheCurrentPlayAHighscore(xmDatabase *pDb,
                                          bool &o_personal,
                                          bool &o_room) {
  int v_best_personal_time;
  int v_current_time;
  int v_best_room_time;
  char **v_result;
  unsigned int nrow;
  char *v_res;

  o_personal = o_room = false;

  if (m_scenes.size() != 1) {
    return;
  }

  if (m_scenes[0]->Players().size() != 1) {
    return;
  }

  v_current_time = m_scenes[0]->Players()[0]->finishTime();

  /* get best player result */
  v_result = pDb->readDB(
    "SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
    "id_level=\"" +
      xmDatabase::protectString(m_scenes[0]->getLevelSrc()->Id()) + "\" " +
      "AND id_profile=\"" +
      xmDatabase::protectString(XMSession::instance()->profile()) + "\";",
    nrow);
  v_res = pDb->getResult(v_result, 1, 0, 0);
  if (v_res != NULL) {
    v_best_personal_time = atoi(v_res);
  } else {
    /* should never happend because the score is already stored */
    v_best_personal_time = -1;
  }
  pDb->read_DB_free(v_result);
  o_personal =
    (v_current_time <= v_best_personal_time || v_best_personal_time < 0);

  /* search a better webhighscore */
  o_room = false;
  for (unsigned int i = 0; i < XMSession::instance()->nbRoomsEnabled(); i++) {
    v_best_room_time = pDb->webrooms_getHighscoreTime(
      XMSession::instance()->idRoom(i), m_scenes[0]->getLevelSrc()->Id());
    if (v_current_time < v_best_room_time || v_best_room_time < 0) {
      o_room = true;
    }
  }
}

bool Universe::isAReplayToSave() const {
  return m_pJustPlayReplay != NULL;
}

bool Universe::isAnErrorOnSaving() const {
  // fake an error on physics level -> removed since it works now, but can
  // happend on some new levels types
  return false;
}

void Universe::finalizeReplay(bool i_finished) {
  if (m_scenes.size() != 1) {
    return;
  }

  if (m_scenes[0]->Players().size() != 1) {
    return;
  }

  /* save the last state because scene don't record each frame */
  SerializedBikeState BikeState;
  Scene::getSerializedBikeState(m_scenes[0]->Players()[0]->getState(),
                                m_scenes[0]->getTime(),
                                &BikeState,
                                m_scenes[0]->getPhysicsSettings());
  m_pJustPlayReplay->storeState(BikeState);
  if (m_scenes[0]->getLevelSrc()->isPhysics()) {
    m_pJustPlayReplay->storeBlocks(m_scenes[0]->getTime(),
                                   m_scenes[0]->getLevelSrc()->Blocks(),
                                   m_scenes[0]->Players(),
                                   true);
  }
  m_pJustPlayReplay->finishReplay(
    i_finished, i_finished ? m_scenes[0]->Players()[0]->finishTime() : 0);
}

Replay *Universe::getCurrentReplay() {
  return m_pJustPlayReplay;
}

void Universe::initReplay() {
  if (m_pJustPlayReplay != NULL)
    delete m_pJustPlayReplay;
  m_pJustPlayReplay = NULL;

  if (m_scenes.size() != 1) {
    return;
  }

  if (XMSession::instance()->storeReplays() &&
      XMSession::instance()->multiNbPlayers() == 1) {
    m_pJustPlayReplay = new Replay;
    m_pJustPlayReplay->createReplay("Latest.rpl",
                                    m_scenes[0]->getLevelSrc()->Id(),
                                    XMSession::instance()->profile(),
                                    XMSession::instance()->replayFrameRate(),
                                    sizeof(SerializedBikeState));
  }
}

std::string Universe::getTemporaryReplayName() const {
  return "Replays/Latest.rpl";
}

void Universe::saveReplayTemporary(xmDatabase *pDb) {
  /*
    use old format if level is not physics to allow people having an old version
    to read replays
    in the future (today is 28/06/2009), once everybody can read 3 version, use
    always 3 version
  */
  m_pJustPlayReplay->saveReplayIfNot(
    m_scenes[0]->getLevelSrc()->isPhysics() ? 3 : 1);
}

void Universe::saveReplay(xmDatabase *pDb, const std::string &Name) {
  /* This is simply a job of copying the Replays/Latest.rpl file into
     Replays/Name.rpl */

  saveReplayTemporary(pDb);

  /* Try saving */
  std::string v_outputfile;

  if (!XMFS::copyFile(FDT_DATA,
                      getTemporaryReplayName(),
                      std::string("Replays/") + Name + std::string(".rpl"),
                      v_outputfile)) {
    throw Exception("Failed to save replay " + Name);
  } else {
    /* Update replay list to reflect changes */
    GameApp::instance()->addReplay(v_outputfile, pDb);
  }
}

std::vector<Scene *> &Universe::getScenes() {
  return m_scenes;
}

void Universe::updateWaitingGhosts() {
  m_waitingForGhosts = false;

  for (unsigned int i = 0; i < m_scenes.size(); i++) {
    if (m_scenes[i]->RequestedGhosts().size() > 0) {
      m_waitingForGhosts = true;
      return;
    }
  }
}

void Universe::markDownloadedGhost(const std::string &i_replay,
                                   bool i_downloadSuccess) {
  for (unsigned int i = 0; i < m_scenes.size(); i++) {
    for (unsigned int j = 0; j < m_scenes[i]->RequestedGhosts().size(); j++) {
      if (m_scenes[i]->RequestedGhosts()[j].name == i_replay) {
        if (i_downloadSuccess) {
          try {
            addGhost(m_scenes[i], m_scenes[i]->RequestedGhosts()[j]);
          } catch (Exception &e) {
            // hum, ok
          }
        } else {
          m_scenes[i]->removeRequestedGhost(
            m_scenes[i]->RequestedGhosts()[j].name);
        }
      }
    }
  }

  // if case of mistake, addGhost will not update waiting ghosts
  if (i_downloadSuccess == false) {
    updateWaitingGhosts();
  }
}

void Universe::addGhost(Scene *i_scene, GhostsAddInfos i_gai) {
  Theme *v_theme = Theme::instance();
  FileGhost *v_ghost = NULL;

  switch (i_gai.strategyType) {
    case GAI_BESTOFROOM:
      v_ghost = i_scene->addGhostFromFile(
        "Replays/" + i_gai.name + ".rpl",
        i_gai.description,
        i_gai.isReference,
        v_theme,
        v_theme->getGhostTheme(),
        TColor(255, 255, 255, 0),
        TColor(GET_RED(v_theme->getGhostTheme()->getUglyRiderColor()),
               GET_GREEN(v_theme->getGhostTheme()->getUglyRiderColor()),
               GET_BLUE(v_theme->getGhostTheme()->getUglyRiderColor()),
               0));
      break;

    case GAI_MYBEST:
      v_ghost = i_scene->addGhostFromFile(
        "Replays/" + i_gai.name + ".rpl",
        i_gai.description,
        i_gai.isReference,
        v_theme,
        v_theme->getGhostTheme(),
        TColor(82, 255, 255, 0),
        TColor(GET_RED(v_theme->getGhostTheme()->getUglyRiderColor()),
               GET_GREEN(v_theme->getGhostTheme()->getUglyRiderColor()),
               GET_BLUE(v_theme->getGhostTheme()->getUglyRiderColor()),
               0));
      break;

    case GAI_THEBEST:
      v_ghost = i_scene->addGhostFromFile(
        "Replays/" + i_gai.name + ".rpl",
        i_gai.description,
        i_gai.isReference,
        v_theme,
        v_theme->getGhostTheme(),
        TColor(255, 200, 140, 0),
        TColor(GET_RED(v_theme->getGhostTheme()->getUglyRiderColor()),
               GET_GREEN(v_theme->getGhostTheme()->getUglyRiderColor()),
               GET_BLUE(v_theme->getGhostTheme()->getUglyRiderColor()),
               0));
      break;
  }

  // remove from requested ghosts
  i_scene->removeRequestedGhost(i_gai.name);
  updateWaitingGhosts();

  // init the trailer
  // best of the 1st room
  if (v_ghost != NULL) {
    if (i_gai.strategyType == GAI_BESTOFROOM && i_gai.isReference) {
      i_scene->initGhostTrail(v_ghost);

      for (unsigned int i = 0; i < i_scene->getNumberCameras(); i++) {
        i_scene->setCurrentCamera(i);
        i_scene->getCamera()->initTrailCam(i_scene->getGhostTrail());
      }
    }
  }
}

void Universe::addAvailableGhosts(xmDatabase *pDb) {
  for (unsigned int i = 0; i < getScenes().size(); i++) {
    int v_alreayAdded = -1;

    /* check for the previous scenes */
    for (unsigned int j = 0; j < i; j++) {
      if (getScenes()[j]->getLevelSrc()->Id() ==
          getScenes()[i]->getLevelSrc()->Id()) {
        v_alreayAdded = j;
      }
    }
    if (v_alreayAdded == -1) {
      addAvailableGhostsToScene(pDb, getScenes()[i]);
    } else {
      // copy levels with the same ghosts
      for (unsigned int j = 0;
           j < getScenes()[v_alreayAdded]->RequestedGhosts().size();
           j++) {
        getScenes()[i]->addRequestedGhost(
          getScenes()[v_alreayAdded]->RequestedGhosts()[j]);
      }
    }

    if (getScenes()[i]->RequestedGhosts().size() != 0) {
      m_waitingForGhosts = true;
    }
  }
}

bool Universe::waitingForGhosts() const {
  return m_waitingForGhosts;
}

void Universe::addAvailableGhostsToScene(xmDatabase *pDb, Scene *i_scene) {
  std::string v_replay_MYBEST;
  std::string v_replay_THEBEST;
  std::string v_replay_BESTOFROOM[ROOMS_NB_MAX];
  std::string v_replay_MYBEST_tmp;
  int v_finishTime;
  int v_player_finishTime;
  bool v_exists;
  GhostsAddInfos v_gi;
  std::string v_replayUrl;

  LogDebug("addGhosts stategy:");

  v_replay_MYBEST_tmp = _getGhostReplayPath_bestOfThePlayer(
    pDb, i_scene->getLevelSrc()->Id(), v_player_finishTime);

  /* first, add the best of the room -- because if mybest or thebest =
   * bestofroom, i prefer to see writen bestofroom */
  for (unsigned int i = 0; i < XMSession::instance()->nbRoomsEnabled(); i++) {
    if ((XMSession::instance()->ghostStrategy_BESTOFREFROOM() && i == 0) ||
        (XMSession::instance()->ghostStrategy_BESTOFOTHERROOMS() && i != 0)) {
      LogDebug("Choosing ghost for room %i", i);

      v_replayUrl = _getGhostReplayPath_bestOfTheRoom(
        pDb, i, i_scene->getLevelSrc()->Id(), v_finishTime);
      v_replay_BESTOFROOM[i] = XMFS::getFileBaseName(v_replayUrl);
      LogDebug("the room ghost: %s", v_replay_BESTOFROOM[i].c_str());

      /* add MYBEST if MYBEST if better the BESTOF the other ROOM */
      if (v_player_finishTime > 0 &&
          (v_finishTime < 0 || v_player_finishTime < v_finishTime)) {
        v_replay_BESTOFROOM[i] = v_replay_MYBEST_tmp;
        LogDebug("my best time is %i ; room one is %i",
                 v_player_finishTime,
                 v_finishTime);
        LogDebug("my best is better than the one of the room => choose it");
      }

      v_exists = false;
      for (unsigned int j = 0; j < i; j++) {
        if (v_replay_BESTOFROOM[i] == v_replay_BESTOFROOM[j]) {
          v_exists = true;
          LogDebug("the ghost is already set by room %i", j);
        }
      }

      if (v_replay_BESTOFROOM[i] != "" && v_exists == false) {
        LogInfo("add ghost %s", v_replay_BESTOFROOM[i].c_str());

        v_gi.name = v_replay_BESTOFROOM[i];
        v_gi.description =
          pDb->webrooms_getName(XMSession::instance()->idRoom(i));
        v_gi.isReference = i == 0;
        v_gi.external = true;
        v_gi.url = v_replayUrl;
        v_gi.strategyType = GAI_BESTOFROOM;

        if (isGhostToExclude(v_gi.name) == false) {
          i_scene->addRequestedGhost(v_gi);
        }
      }
    }
  }

  /* second, add your best */
  if (XMSession::instance()->ghostStrategy_MYBEST()) {
    LogDebug("Choosing ghost MYBEST");
    v_replay_MYBEST = _getGhostReplayPath_bestOfThePlayer(
      pDb, i_scene->getLevelSrc()->Id(), v_finishTime);
    LogDebug("MYBEST ghost is %s", v_replay_MYBEST.c_str());
    if (v_replay_MYBEST != "") {
      v_exists = false;
      for (unsigned int i = 0; i < XMSession::instance()->nbRoomsEnabled();
           i++) {
        if (v_replay_MYBEST == v_replay_BESTOFROOM[i]) {
          LogDebug("the ghost is already set by room %i", i);
          v_exists = true;
        }
      }

      if (v_exists == false) {
        LogDebug("add ghost %s", v_replay_MYBEST.c_str());

        v_gi.name = v_replay_MYBEST;
        v_gi.description = GAMETEXT_GHOST_BEST;
        v_gi.isReference = true;
        v_gi.external = false;
        v_gi.url = "";
        v_gi.strategyType = GAI_MYBEST;

        if (isGhostToExclude(v_gi.name) == false) {
          i_scene->addRequestedGhost(v_gi);
        }
      }
    }
  }

  /* third, the best locally */
  if (XMSession::instance()->ghostStrategy_THEBEST()) {
    LogDebug("Choosing ghost LOCAL BEST");
    v_replay_THEBEST = _getGhostReplayPath_bestOfLocal(
      pDb, i_scene->getLevelSrc()->Id(), v_finishTime);
    if (v_replay_THEBEST != "") {
      v_exists = false;
      for (unsigned int i = 0; i < XMSession::instance()->nbRoomsEnabled();
           i++) {
        if (v_replay_THEBEST == v_replay_BESTOFROOM[i]) {
          LogDebug("the ghost is already set by room %i", i);
          v_exists = true;
        }
      }

      if (v_replay_THEBEST != v_replay_MYBEST &&
          v_exists == false) { /* don't add two times the same ghost */
        LogDebug("add ghost %s", v_replay_THEBEST.c_str());

        v_gi.name = v_replay_THEBEST;
        v_gi.description = GAMETEXT_GHOST_LOCAL;
        v_gi.isReference = false;
        v_gi.external = false;
        v_gi.url = "";
        v_gi.strategyType = GAI_THEBEST;

        if (isGhostToExclude(v_gi.name) == false) {
          i_scene->addRequestedGhost(v_gi);
        }
      }
    }
  }

  LogDebug("addGhosts stategy finished");
}

std::string Universe::_getGhostReplayPath_bestOfThePlayer(xmDatabase *pDb,
                                                          std::string p_levelId,
                                                          int &p_time) {
  char **v_result;
  unsigned int nrow;
  std::string res;

  p_time = -1;

  v_result =
    pDb->readDB("SELECT name, finishTime FROM replays "
                "WHERE id_profile=\"" +
                  xmDatabase::protectString(XMSession::instance()->profile()) +
                  "\" "
                  "AND   id_level=\"" +
                  xmDatabase::protectString(p_levelId) +
                  "\" "
                  "AND   isFinished=1 "
                  "ORDER BY finishTime LIMIT 1;",
                nrow);
  if (nrow == 0) {
    pDb->read_DB_free(v_result);
    return "";
  }

  res = pDb->getResult(v_result, 2, 0, 0);
  p_time = atoi(pDb->getResult(v_result, 2, 0, 1));

  pDb->read_DB_free(v_result);
  return res;
}

std::string Universe::_getGhostReplayPath_bestOfTheRoom(xmDatabase *pDb,
                                                        unsigned int i_number,
                                                        std::string p_levelId,
                                                        int &p_time) {
  char **v_result;
  unsigned int nrow;
  std::string res;
  std::string v_fileUrl;

  v_result = pDb->readDB("SELECT fileUrl, finishTime FROM webhighscores "
                         "WHERE id_room=" +
                           XMSession::instance()->idRoom(i_number) +
                           " "
                           "AND id_level=\"" +
                           xmDatabase::protectString(p_levelId) + "\";",
                         nrow);
  if (nrow == 0) {
    p_time = -1;
    pDb->read_DB_free(v_result);
    return "";
  }

  v_fileUrl = pDb->getResult(v_result, 2, 0, 0);
  p_time = atoi(pDb->getResult(v_result, 2, 0, 1));
  pDb->read_DB_free(v_result);

  return v_fileUrl;
}

std::string Universe::_getGhostReplayPath_bestOfLocal(xmDatabase *pDb,
                                                      std::string p_levelId,
                                                      int &p_time) {
  char **v_result;
  unsigned int nrow;
  std::string res;

  v_result = pDb->readDB("SELECT a.name, a.finishTime FROM replays AS a INNER "
                         "JOIN stats_profiles AS b "
                         "ON a.id_profile = b.id_profile "
                         "WHERE a.id_level=\"" +
                           xmDatabase::protectString(p_levelId) +
                           "\" "
                           "AND   a.isFinished=1 "
                           "ORDER BY a.finishTime+0 LIMIT 1;",
                         nrow);
  if (nrow == 0) {
    pDb->read_DB_free(v_result);
    return "";
  }

  res = pDb->getResult(v_result, 2, 0, 0);
  p_time = atoi(pDb->getResult(v_result, 2, 0, 1));
  pDb->read_DB_free(v_result);
  return res;
}

void Universe::addGhostToExclude(const std::string &i_ghostname) {
  m_ghostToExclude.push_back(i_ghostname);
}

bool Universe::isGhostToExclude(const std::string &i_ghostname) {
  for (unsigned int i = 0; i < m_ghostToExclude.size(); i++) {
    if (m_ghostToExclude[i] == i_ghostname) {
      return true;
    }
  }

  return false;
}
