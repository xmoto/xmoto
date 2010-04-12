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
#include "Sound.h"
#include "Theme.h"
#include "xmscene/Level.h"
#include "xmscene/Camera.h"
#include "Replay.h"
#include "Game.h"
#include "SysMessage.h"
#include "helpers/Log.h"
#include "db/xmDatabase.h"
#include "drawlib/DrawLib.h"
#include "Renderer.h"

void XMSceneHooks::OnTakeEntity() {
    /* Play yummy-yummy sound */
    try {
      Sound::playSampleByName(Theme::instance()->getSound(m_Scene->getLevelSrc()->SoundForPickUpStrawberry())->FilePath());
    } catch(Exception &e) {
    }
}


void XMSceneHooks::setGameApps(Scene *i_Scene) {
  m_Scene = i_Scene;
}

XMSceneHooks::XMSceneHooks() {
  m_Scene = NULL;
}
  
XMSceneHooks::~XMSceneHooks() {
}

Universe::Universe() {
  m_pJustPlayReplay = NULL;
}

Universe::~Universe() {
  removeAllWorlds();

  if(m_pJustPlayReplay != NULL) {
    delete m_pJustPlayReplay;
  }    
}

void Universe::addScene() {
  Scene*        v_motoGame      = new Scene();
  XMSceneHooks* v_motoGameHooks = new XMSceneHooks();

  /* Tell collision system whether we want debug-info or not */
  v_motoGame->getCollisionHandler()->setDebug(XMSession::instance()->debug());
  v_motoGame->setHooks(v_motoGameHooks);
  v_motoGameHooks->setGameApps(v_motoGame);

  m_scenes.push_back(v_motoGame);
  m_motoGameHooks.push_back(v_motoGameHooks);
}

void Universe::removeAllWorlds() {
  for(unsigned int i=0; i<m_scenes.size(); i++) {
    m_scenes[i]->resetFollow();
    m_scenes[i]->endLevel();
    delete m_scenes[i];
  }
  m_scenes.clear();
}

void Universe::initPlayServer() {
  // just one scene for all players
  addScene();
}

void Universe::initPlay(int i_nbPlayer, bool i_multiScenes) {
  if(i_multiScenes) {
    for(int i=0; i<i_nbPlayer; i++) {
      addScene();
    }
  } else {
    // only one scene
    addScene();
  }

  initCameras(i_nbPlayer);
}

void Universe::initCameras(int nbPlayer) {
  int width  = GameApp::instance()->getDrawLib()->getDispWidth();
  int height = GameApp::instance()->getDrawLib()->getDispHeight();
  bool v_useActiveZoom = XMSession::instance()->enableActiveZoom();
  bool v_useTrailCam = XMSession::instance()->enableTrailCam();
  
  if(m_scenes.size() <= 0) {
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
                    all in the same scene (scene[0]->cameras[0..4]) with 4 players
                    autozoom cam is scene[0]->camera[4]
           multi scenes (normal mode):
                    4 player cams in 4 scenes (scene[0..3]->cameras[0])
                    autozoom cam is scene[0]->camera[1] for 2 and 4 players
                               but  scene[0]->camera[2] for 3 players, because
                               the camera[1] is used for duplicate player 1 cam 
                               for 4th player field (otherwise we get ugly drawing)  */
  switch(nbPlayer){
  default:
  case 1:
    m_scenes[0]->addCamera(Vector2i(0,0),
			   Vector2i(width, height), v_useActiveZoom, v_useTrailCam);
    break;
  case 2:
    m_scenes[0]->addCamera(Vector2i(0,height/2),
			   Vector2i(width, height), v_useActiveZoom, v_useTrailCam);
    if(m_scenes.size() == 1) {
      m_scenes[0]->addCamera(Vector2i(0,0),
			     Vector2i(width, height/2), v_useActiveZoom, v_useTrailCam);
    } else {
      m_scenes[1]->addCamera(Vector2i(0,0),
			     Vector2i(width, height/2), v_useActiveZoom, v_useTrailCam);
    }
    break;
  case 3:
  case 4:
    if(m_scenes.size() == 1) {
      m_scenes[0]->addCamera(Vector2i(0,height/2),
			     Vector2i(width/2, height), v_useActiveZoom, v_useTrailCam);
      m_scenes[0]->addCamera(Vector2i(width/2,height/2),
			     Vector2i(width, height), v_useActiveZoom, v_useTrailCam);
      m_scenes[0]->addCamera(Vector2i(0,0),
			     Vector2i(width/2, height/2), v_useActiveZoom, v_useTrailCam);
      m_scenes[0]->addCamera(Vector2i(width/2,0),
			     Vector2i(width, height/2), v_useActiveZoom, v_useTrailCam);
    } else {
      m_scenes[0]->addCamera(Vector2i(0,height/2),
			     Vector2i(width/2, height), v_useActiveZoom, v_useTrailCam);
      m_scenes[1]->addCamera(Vector2i(width/2,height/2),
			     Vector2i(width, height), v_useActiveZoom, v_useTrailCam);
      m_scenes[2]->addCamera(Vector2i(0,0),
			     Vector2i(width/2, height/2), v_useActiveZoom, v_useTrailCam);

      if(nbPlayer == 4) {
	m_scenes[3]->addCamera(Vector2i(width/2,0),
			       Vector2i(width, height/2), v_useActiveZoom, v_useTrailCam);
      }
      else {  //3 player then, add another cam for not having an ugly not-drawn rect in the screen
        m_scenes[0]->addCamera(Vector2i(width/2,0),
			       Vector2i(width, height/2), v_useActiveZoom, v_useTrailCam);
      }
    }
    break;
  }
  
  // the autozoom camera is a special one in multi player
  if(nbPlayer > 1){
    m_scenes[0]->addCamera(Vector2i(0,0),
			   Vector2i(width, height), v_useActiveZoom, v_useTrailCam);
  }
  // current cam is autozoom one
  for(unsigned int i=0; i<m_scenes.size(); i++) {
    m_scenes[i]->setAutoZoomCamera();
  }
}

void Universe::deleteCurrentReplay() {
  if(m_pJustPlayReplay != NULL) {   
      delete m_pJustPlayReplay;
      m_pJustPlayReplay = NULL;
  }          
}

void Universe::TeleportationCheatTo(int i_player, Vector2f i_position) {
  if(m_scenes.size() > 0) {
    m_scenes[0]->setPlayerPosition(i_player, i_position.x, i_position.y, true);
    m_scenes[0]->getCamera()->initCamera();
    //m_scenes[0]->addPenalityTime(90000); /* 15 min of penality for that ! */
    /* ... and no replay of course ! */
    deleteCurrentReplay();
  }
}

void Universe::switchFollowCamera() {
  if(m_scenes.size() > 0) {
    GameRenderer::instance()->switchFollow(m_scenes[0]);

    SysMessage::instance()->displayText(m_scenes[0]->getCamera()->
					getPlayerToFollow()->
					getQuickDescription());
  }
}

void Universe::isTheCurrentPlayAHighscore(xmDatabase *pDb, bool& o_personal, bool& o_room) {
  int v_best_personal_time;
  int v_current_time;
  int v_best_room_time;
  char **v_result;
  unsigned int nrow;
  char *v_res;

  o_personal = o_room = false;

  if(m_scenes.size() != 1) {
    return;
  }

  if(m_scenes[0]->Players().size() != 1) {
    return;
  }

  v_current_time = m_scenes[0]->Players()[0]->finishTime();

  /* get best player result */
  v_result = pDb->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
			  "id_level=\"" + 
			  xmDatabase::protectString(m_scenes[0]->getLevelSrc()->Id()) + "\" " + 
			  "AND id_profile=\"" + xmDatabase::protectString(XMSession::instance()->profile())  + "\";",
			  nrow);
  v_res = pDb->getResult(v_result, 1, 0, 0);
  if(v_res != NULL) {
    v_best_personal_time = atoi(v_res);
  } else {
    /* should never happend because the score is already stored */
    v_best_personal_time = -1;
  }
  pDb->read_DB_free(v_result);
  o_personal = (v_current_time <= v_best_personal_time
		|| v_best_personal_time < 0);

  /* search a better webhighscore */
  o_room = false;
  for(unsigned int i=0; i<XMSession::instance()->nbRoomsEnabled(); i++) {
    v_best_room_time = pDb->webrooms_getHighscoreTime(XMSession::instance()->idRoom(i), m_scenes[0]->getLevelSrc()->Id());    
    if(v_current_time < v_best_room_time || v_best_room_time < 0) {
      o_room = true;
    }
  }
}

bool Universe::isAReplayToSave() const {
  return m_pJustPlayReplay != NULL;
}

bool Universe::isAnErrorOnSaving() const {

  // fake an error on physics level
  for(unsigned int i=0; i<m_scenes.size(); i++) {
    if(m_scenes[i]->getLevelSrc()->isPhysics()) {
      return true;
    }
  }
  return false;
}

void Universe::finalizeReplay(bool i_finished) {
  if(m_scenes.size() != 1) {
    return;
  }

  if(m_scenes[0]->Players().size() != 1) {
    return;
  }

  /* save the last state because scene don't record each frame */
  SerializedBikeState BikeState;
  Scene::getSerializedBikeState(m_scenes[0]->Players()[0]->getState(), m_scenes[0]->getTime(), &BikeState, m_scenes[0]->getPhysicsSettings());
  m_pJustPlayReplay->storeState(BikeState);
  m_pJustPlayReplay->finishReplay(i_finished, i_finished ? m_scenes[0]->Players()[0]->finishTime() : 0,
				  1); // still always use the format 1
				  /*
				    use old format if level is not physics to allow people having an old version to read replays
				    in the future (today is 28/06/2009), once everybody can read 2 version, use always 2 version
				    
				    m_scenes[0]->getLevelSrc()->isPhysics() ? 2 : 1);
				  */
}

Replay* Universe::getCurrentReplay() {
  return m_pJustPlayReplay;
}

void Universe::initReplay() {
  if(m_pJustPlayReplay != NULL) delete m_pJustPlayReplay;
  m_pJustPlayReplay = NULL;

  if(m_scenes.size() != 1) {
    return;
  }

  if(XMSession::instance()->storeReplays() && XMSession::instance()->multiNbPlayers() == 1 &&
     m_scenes[0]->getLevelSrc()->isPhysics() == false) {
    m_pJustPlayReplay = new Replay;
    m_pJustPlayReplay->createReplay("Latest.rpl",
				    m_scenes[0]->getLevelSrc()->Id(),
				    XMSession::instance()->profile(),
				    XMSession::instance()->replayFrameRate(),
				    sizeof(SerializedBikeState));
  }
}

void Universe::saveReplay(xmDatabase *pDb, const std::string &Name) {
  /* This is simply a job of copying the Replays/Latest.rpl file into 
     Replays/Name.rpl */

  /* Try saving */
  std::string v_outputfile;

  if(!XMFS::copyFile(FDT_DATA, "Replays/Latest.rpl",
		   std::string("Replays/") + Name + std::string(".rpl"),
		   v_outputfile)) {
    throw Exception("Failed to save replay " + Name);
  } else {
    /* Update replay list to reflect changes */
    GameApp::instance()->addReplay(v_outputfile, pDb);
  }
}

std::vector<Scene*>& Universe::getScenes() {
  return m_scenes;
}
