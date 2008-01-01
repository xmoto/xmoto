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

void XMMotoGameHooks::OnTakeEntity() {
    /* Play yummy-yummy sound */
    try {
      Sound::playSampleByName(Theme::instance()->getSound(m_MotoGame->getLevelSrc()->SoundForPickUpStrawberry())->FilePath());
    } catch(Exception &e) {
    }
}


void XMMotoGameHooks::setGameApps(MotoGame *i_MotoGame) {
  m_MotoGame = i_MotoGame;
}

XMMotoGameHooks::XMMotoGameHooks() {
  m_MotoGame = NULL;
}
  
XMMotoGameHooks::~XMMotoGameHooks() {
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
  MotoGame*        v_motoGame      = new MotoGame();
  XMMotoGameHooks* v_motoGameHooks = new XMMotoGameHooks();

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
  
  if(m_scenes.size() <= 0) {
    return;
  }

  switch(nbPlayer){
  default:
  case 1:
    m_scenes[0]->addCamera(Vector2i(0,0),
			   Vector2i(width, height));
    break;
  case 2:
    m_scenes[0]->addCamera(Vector2i(0,height/2),
			   Vector2i(width, height));
    if(m_scenes.size() == 1) {
      m_scenes[0]->addCamera(Vector2i(0,0),
			     Vector2i(width, height/2));
    } else {
      m_scenes[1]->addCamera(Vector2i(0,0),
			     Vector2i(width, height/2));
    }
    break;
  case 3:
  case 4:
    if(m_scenes.size() == 1) {
      m_scenes[0]->addCamera(Vector2i(0,height/2),
			     Vector2i(width/2, height));
      m_scenes[0]->addCamera(Vector2i(width/2,height/2),
			     Vector2i(width, height));
      m_scenes[0]->addCamera(Vector2i(0,0),
			     Vector2i(width/2, height/2));
      m_scenes[0]->addCamera(Vector2i(width/2,0),
			     Vector2i(width, height/2));
    } else {
      m_scenes[0]->addCamera(Vector2i(0,height/2),
			     Vector2i(width/2, height));
      m_scenes[1]->addCamera(Vector2i(width/2,height/2),
			     Vector2i(width, height));
      m_scenes[2]->addCamera(Vector2i(0,0),
			     Vector2i(width/2, height/2));

      if(nbPlayer == 4) {
	m_scenes[3]->addCamera(Vector2i(width/2,0),
			       Vector2i(width, height/2));
      }
    }
    break;
  }
  
  // the autozoom camera is a special one in multi player
  if(nbPlayer > 1){
    m_scenes[0]->addCamera(Vector2i(0,0),
			   Vector2i(width, height));
  }
  // current cam is autozoom one
  for(unsigned int i=0; i<m_scenes.size(); i++) {
    m_scenes[i]->setAutoZoomCamera();
  }
}


void Universe::TeleportationCheatTo(int i_player, Vector2f i_position) {
  if(m_scenes.size() > 0) {
    m_scenes[0]->setPlayerPosition(i_player, i_position.x, i_position.y, true);
    m_scenes[0]->getCamera()->initCamera();
    m_scenes[0]->addPenalityTime(900); /* 15 min of penality for that ! */
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

void Universe::isTheCurrentPlayAHighscore(bool& o_personal, bool& o_room) {
  int v_best_personal_time;
  int v_current_time;
  int v_best_room_time;
  char **v_result;
  unsigned int nrow;
  char *v_res;
  xmDatabase *pDb = xmDatabase::instance("main");

  o_personal = o_room = false;

  if(m_scenes.size() != 1) {
    return;
  }

  if(m_scenes[0]->Players().size() != 1) {
    return;
  }

  v_current_time = (int)(100.0 * m_scenes[0]->Players()[0]->finishTime());

  /* get best player result */
  v_result = pDb->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
			  "id_level=\"" + 
			  xmDatabase::protectString(m_scenes[0]->getLevelSrc()->Id()) + "\" " + 
			  "AND id_profile=\"" + xmDatabase::protectString(XMSession::instance()->profile())  + "\";",
			  nrow);
  v_res = pDb->getResult(v_result, 1, 0, 0);
  if(v_res != NULL) {
    v_best_personal_time = (int)(100.0 * (atof(v_res) + 0.001)); /* + 0.001 because it is converted into a float */
  } else {
    /* should never happend because the score is already stored */
    v_best_personal_time = -1;
  }
  pDb->read_DB_free(v_result);
  o_personal = (v_current_time <= v_best_personal_time
		|| v_best_personal_time < 0);

  /* search a better webhighscore */
  v_best_room_time = (int)(100.0 * pDb->webrooms_getHighscoreTime(XMSession::instance()->idRoom(), m_scenes[0]->getLevelSrc()->Id()));
  o_room = (v_current_time < v_best_room_time
	    || v_best_room_time < 0);
}

bool Universe::isAReplayToSave() const {
  return m_pJustPlayReplay != NULL;
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
  MotoGame::getSerializedBikeState(m_scenes[0]->Players()[0]->getState(), m_scenes[0]->getTime(), &BikeState);
  m_pJustPlayReplay->storeState(BikeState);
  m_pJustPlayReplay->finishReplay(i_finished, i_finished ? m_scenes[0]->Players()[0]->finishTime() : 0.0);
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

  if(XMSession::instance()->storeReplays() && XMSession::instance()->multiNbPlayers() == 1) {
    m_pJustPlayReplay = new Replay;
    m_pJustPlayReplay->createReplay("Latest.rpl",
				    m_scenes[0]->getLevelSrc()->Id(),
				    XMSession::instance()->profile(),
				    XMSession::instance()->replayFrameRate(),
				    sizeof(SerializedBikeState));
  }
}

void Universe::saveReplay(const std::string &Name) {
  /* This is simply a job of copying the Replays/Latest.rpl file into 
     Replays/Name.rpl */
  std::string RealName = Name;
  
  /* Strip illegal characters from name */
  unsigned int i=0;
  while(1) {
    if(i >= RealName.length())
      break;
    
    if((RealName[i] >= 'a' && RealName[i] <= 'z') ||
       (RealName[i] >= 'A' && RealName[i] <= 'Z') ||
       (RealName[i] >= '0' && RealName[i] <= '9') ||
       RealName[i]=='!' || RealName[i]=='@' || RealName[i]=='#' || RealName[i]=='&' ||
       RealName[i]=='(' || RealName[i]==')' || RealName[i]=='-' || RealName[i]=='_' ||
       RealName[i]==' ' || RealName[i]=='.' || RealName[i]==',' || RealName[i]=='*') {
      /* This is ok */
      i++;
    }
    else {
      /* Not ok */
      RealName.erase(RealName.begin() + i);
    }            
  }

  /* Try saving */
  std::string v_outputfile;
  if(!FS::copyFile("Replays/Latest.rpl",
		   std::string("Replays/") + RealName + std::string(".rpl"),
		   v_outputfile)) {
    Logger::Log("** Warning ** : Failed to save replay: %s",Name.c_str());
    //notifyMsg(GAMETEXT_FAILEDTOSAVEREPLAY);
  } else {
    /* Update replay list to reflect changes */
     GameApp::instance()->addReplay(v_outputfile);
  }
}

std::vector<MotoGame*>& Universe::getScenes() {
  return m_scenes;
}
