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

#include "StatePreplaying.h"
#include "Game.h"
#include "StatePlaying.h"
#include "helpers/Log.h"
#include "GameText.h"
#include "xmscene/Camera.h"
#include "XMSession.h"
#include "xmscene/BikePlayer.h"

StatePreplaying::StatePreplaying(GameApp* pGame, const std::string i_idlevel):
  StateScene(pGame)
{
  m_name  = "StatePreplaying";
  m_idlevel = i_idlevel;
}

StatePreplaying::~StatePreplaying()
{

}


void StatePreplaying::enter()
{
  StateScene::enter();

  m_pGame->m_State = GS_PREPLAYING; // to be removed, just the time states are finished

//		  m_Renderer->setShowTimePanel(false);
//			/* because statePrestart_init() can call setState */
//			if(m_bEnableMenuMusic && Sound::isEnabled()) {
//				Sound::stopMusic();
//				m_playingMusic = "";
//			}
//
//
//    char **v_result;
//    unsigned int nrow;
//    char *v_res;
//
//    /* Initialize controls */
    m_pGame->getInputHandler()->configure(m_pGame->getUserConfig());
//      
//    /* Default playing state */
//    m_fLastPerfStateTime = 0.0f;
//      
//    /* We need a profile */
//    if(m_xmsession->profile() == "") {
//      Logger::Log("** Warning ** : no player profile selected, use -profile option");
//      throw Exception("no player");
//    }
//      

  try {
    m_pGame->getMotoGame()->loadLevel(m_pGame->getDb(), m_idlevel);
  } catch(Exception &e) {
    Logger::Log("** Warning ** : level '%s' cannot be loaded", m_idlevel.c_str());
    char cBuf[256];
    sprintf(cBuf,GAMETEXT_LEVELCANNOTBELOADED, m_idlevel.c_str());
//    setState(m_StateAfterPlaying);
//    notifyMsg(cBuf);
    return;
  }
//
//    if(m_MotoGame.getLevelSrc()->isXMotoTooOld()) {
//      Logger::Log("** Warning ** : level '%s' requires newer X-Moto",
//	  m_MotoGame.getLevelSrc()->Name().c_str());
//  
//      char cBuf[256];
//      sprintf(cBuf,GAMETEXT_NEWERXMOTOREQUIRED,
//	      m_MotoGame.getLevelSrc()->getRequiredVersion().c_str());
//      m_MotoGame.endLevel();
//
//      setState(m_StateAfterPlaying);
//      notifyMsg(cBuf);     
//      return;
//    }
//
//    /* Start playing right away */     
  m_pGame->initReplay();
//      
      try {
	m_pGame->getInputHandler()->reset();
	//m_InputHandler.setMirrored(m_MotoGame.getCamera()->isMirrored());
	m_pGame->getMotoGame()->prePlayLevel(m_pGame->getInputHandler(), m_pGame->getCurrentReplay(), true);
	m_pGame->getMotoGame()->setInfos("");
	
//	/* add the players */
	int v_nbPlayer = m_pGame->getNumberOfPlayersToPlay();
//	Logger::Log("Preplay level for %i player(s)", v_nbPlayer);
//
	m_pGame->initCameras(v_nbPlayer);
//
	for(int i=0; i<v_nbPlayer; i++) {
	  m_pGame->getMotoGame()->setCurrentCamera(i);
	  m_pGame->getMotoGame()->getCamera()->setPlayerToFollow(m_pGame->getMotoGame()->addPlayerBiker(m_pGame->getMotoGame()->getLevelSrc()->PlayerStart(),
													DD_RIGHT,
													m_pGame->getTheme(), m_pGame->getTheme()->getPlayerTheme(),
													m_pGame->getColorFromPlayerNumber(i),
													m_pGame->getUglyColorFromPlayerNumber(i),
													m_pGame->getSession()->enableEngineSound()));
	}
	// if there's more camera than player (ex: 3 players and 4 cameras),
	// then, make the remaining cameras follow the first player
	if(v_nbPlayer < m_pGame->getMotoGame()->getNumberCameras()){
	  for(int i=v_nbPlayer; i<m_pGame->getMotoGame()->getNumberCameras(); i++){
	    m_pGame->getMotoGame()->setCurrentCamera(i);
	    m_pGame->getMotoGame()->getCamera()->setPlayerToFollow(m_pGame->getMotoGame()->Players()[0]);
	  }
	}

	if(m_pGame->getMotoGame()->getNumberCameras() > 1){
	  // make the zoom camera follow the first player
	  m_pGame->getMotoGame()->setCurrentCamera(m_pGame->getMotoGame()->getNumberCameras());
	  m_pGame->getMotoGame()->getCamera()->setPlayerToFollow(m_pGame->getMotoGame()->Players()[0]);
	}

	/* add the ghosts */
	if(m_pGame->getSession()->enableGhosts()) {
	  try {
	    m_pGame->addGhosts(m_pGame->getMotoGame(), m_pGame->getTheme());
	  } catch(Exception &e) {
	    /* anyway */
	  }
	}
      } catch(Exception &e) {
	Logger::Log(std::string("** Warning ** : failed to initialize level\n" + e.getMsg()).c_str());
	m_pGame->getMotoGame()->endLevel();
//	setState(m_StateAfterPlaying);
//	notifyMsg(splitText(e.getMsg(), 50));
	return;
      }
//
//    m_State = GS_PREPLAYING;
//
//    std::string T1 = "--:--:--", T2 = "--:--:--";
//
//    /* get best result */
//    v_result = m_db->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
//    			    "id_level=\"" + 
//    			    xmDatabase::protectString(m_MotoGame.getLevelSrc()->Id()) + "\";",
//    			    nrow);
//    v_res = m_db->getResult(v_result, 1, 0, 0);
//    if(v_res != NULL) {
//      T1 = formatTime(atof(v_res));
//    }
//    m_db->read_DB_free(v_result);
//    
//    /* get best player result */
//    v_result = m_db->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
//    			    "id_level=\"" + 
//    			    xmDatabase::protectString(m_MotoGame.getLevelSrc()->Id()) + "\" " + 
//    			    "AND id_profile=\"" + xmDatabase::protectString(m_xmsession->profile())  + "\";",
//    			    nrow);
//    v_res = m_db->getResult(v_result, 1, 0, 0);
//    if(v_res != NULL) {
//      T2 = formatTime(atof(v_res));
//    }
//    m_db->read_DB_free(v_result);
//    
//    m_Renderer->setBestTime(T1 + std::string(" / ") + T2);
//    m_Renderer->hideReplayHelp();
//    
//    /* World-record stuff */
//    m_Renderer->setWorldRecordTime(getWorldRecord(m_PlaySpecificLevelId));
//
//    /* Prepare level */
    m_pGame->getGameRenderer()->prepareForNewLevel();
//    prestartAnimation_init();

      m_pGame->getStateManager()->replaceState(new StatePlaying(m_pGame));
}

void StatePreplaying::leave()
{

}

void StatePreplaying::enterAfterPop()
{

}

void StatePreplaying::leaveAfterPush()
{

}

bool StatePreplaying::update()
{
  if(doUpdate() == false){
    return false;
  }

  //StateScene::update(); // don't update the scene in preplaying

//  double fStartFrameTime = getXMTime();                    
//  int numberCam = m_MotoGame.getNumberCameras();
//  
//  /* If "preplaying" / "initial-zoom" is enabled, this is where it's done */
//  if(numberCam > 1){
//    m_MotoGame.setCurrentCamera(numberCam);
//  }
//  statePrestart_step();

  return true;
}

bool StatePreplaying::render()
{
  StateScene::render();
  return true;
}

void StatePreplaying::keyDown(int nKey, SDLMod mod,int nChar)
{
  //    m_bPrePlayAnim = false;
}

void StatePreplaying::keyUp(int nKey,   SDLMod mod)
{

}

void StatePreplaying::mouseDown(int nButton)
{

}

void StatePreplaying::mouseDoubleClick(int nButton)
{

}

void StatePreplaying::mouseUp(int nButton)
{

}
