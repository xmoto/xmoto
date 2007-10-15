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

#include "StatePlaying.h"
#include "StatePause.h"
#include "Game.h"

StatePlaying::StatePlaying(GameApp* pGame):
  StateScene(pGame)
{

}

StatePlaying::~StatePlaying()
{

}


void StatePlaying::enter()
{
  StateScene::enter();

  m_pGame->m_State = GS_PLAYING; // to be removed, just the time states are finished


  //  m_pGame->getGameRenderer()->setShowEngineCounter(true);
  m_pGame->getGameRenderer()->setShowTimePanel(true);

//
//		case GS_PLAYING: {
//			m_Renderer->setShowEngineCounter(m_Config.getBool("ShowEngineCounter"));
//			m_Renderer->setShowTimePanel(true);
//			v_newMusicPlaying = "";
//
//			m_bAutoZoomInitialized = false;
//				
//			try {
//				m_MotoGame.playLevel();
//				m_State = GS_PLAYING;        
//				m_nFrame = 0;
//				v_newMusicPlaying = m_MotoGame.getLevelSrc()->Music();
//			} catch(Exception &e) {
//				Logger::Log("** Warning ** : level '%s' cannot be loaded",m_PlaySpecificLevelId.c_str());
//				m_MotoGame.endLevel();
//				char cBuf[256];
//				sprintf(cBuf,GAMETEXT_LEVELCANNOTBELOADED,m_PlaySpecificLevelId.c_str());
//				setState(m_StateAfterPlaying);
//				notifyMsg(cBuf);
//			}
//			break;
//		}
}

void StatePlaying::leave()
{
  m_pGame->getMotoGame()->setInfos("");
}

void StatePlaying::enterAfterPop()
{

}

void StatePlaying::leaveAfterPush()
{

}

bool StatePlaying::update()
{
  if(StateScene::update() == false){
    return false;
  }

//
//
//        /* These states all requires that the actual game graphics are rendered (i.e. inside 
//           the game, not the main menu) */
//        try {
//          int nPhysSteps = 0;
//        
//          /* When did the frame start? */
//          double fStartFrameTime = getXMTime();                    
//	  int numberCam = m_MotoGame.getNumberCameras();
//          if(m_State == GS_PREPLAYING) {
//            /* If "preplaying" / "initial-zoom" is enabled, this is where it's done */
//	    if(numberCam > 1){
//	      m_MotoGame.setCurrentCamera(numberCam);
//	    }
//            statePrestart_step();
//
//	    if(m_xmsession->timedemo() == false) {
//	      /* limit framerate while PREPLAY (100 fps)*/
//	      // TODO::MANU::the sleep is done in only one place now.
//	      // put it there
//	      /*
//	      double timeElapsed = getXMTime() - fStartFrameTime;
//	      if(timeElapsed < 0.01)
//		setFrameDelay(10 - (int)(timeElapsed*1000.0));
//	      */
//	    }
//          } else if(m_State == GS_PLAYING ||
//		    ((m_State == GS_DEADMENU || m_State == GS_DEADJUST) && m_bEnableDeathAnim)
//		    ) {
//            /* When actually playing or when dead and the bike is falling apart, 
//               a physics update is required */
//	    if(isLockedMotoGame()) {
//	      nPhysSteps = 0;
//	    } else {
//	      nPhysSteps = _UpdateGamePlaying();            
//	    }
//          }
//  
//	  if(m_State == GS_PLAYING) {
//	    if(numberCam > 1){
//	      m_MotoGame.setCurrentCamera(numberCam);
//	    }
//	    autoZoom();
//	  }
//
//          /* Render */
//          if(!getDrawLib()->isNoGraphics()) {
//	    try {
//	      if((m_autoZoom || (m_bPrePlayAnim && m_xmsession->ugly() == false)) && numberCam > 1){
//		m_Renderer->render(bIsPaused);
//		ParticlesSource::setAllowParticleGeneration(m_Renderer->nbParticlesRendered() < NB_PARTICLES_TO_RENDER_LIMITATION);
//	      }else{
//		for(int i=0; i<numberCam; i++){
//		  m_MotoGame.setCurrentCamera(i);
//		  m_Renderer->render(bIsPaused);
//		  ParticlesSource::setAllowParticleGeneration(m_Renderer->nbParticlesRendered() < NB_PARTICLES_TO_RENDER_LIMITATION);
//		}
//	      }
//	    } catch(Exception &e) {
//	      m_MotoGame.endLevel();
//	      setState(m_StateAfterPlaying);
//	      notifyMsg(splitText(e.getMsg(), 50));
//	    }
//	    getDrawLib()->getMenuCamera()->setCamera2d();
//	  }
//#if SIMULATE_SLOW_RENDERING
//          SDL_Delay(SIMULATE_SLOW_RENDERING);
//#endif
//  
//          /* When actually playing, check if something happened (like dying or finishing) */
//          if(m_State == GS_PLAYING) {        
//            _PostUpdatePlaying();
//          }
//
//	  /* TODO::MANU::get out !!
//          // When did frame rendering end?
//          double fEndFrameTime = getXMTime();
//          
//          // Calculate how large a delay should be inserted after the frame, to keep the 
//	  // desired frame rate 
//          int nADelay = 0;    
//          
//	  if (m_State == GS_DEADJUST) {
//            setFrameDelay(10);
//          } else {
//            // become idle only if we hadn't to skip any frame, recently, and more globaly (80% of fps)
//            if((nPhysSteps <= 1) && (m_fFPS_Rate > (0.8f / PHYS_STEP_SIZE)))
//              nADelay = ((m_fLastPhysTime + PHYS_STEP_SIZE) - fEndFrameTime) * 1000.0f;
//
//	    if(m_autoZoom){
//	      // limit framerate while zooming (100 fps)
//	      double timeElapsed = getXMTime() - fStartFrameTime;
//	      if(timeElapsed < 0.01)
//		nADelay = 10 - (int)(timeElapsed*1000.0);
//	    }
//          }
//
//          if(nADelay > 0) {
//            if(m_xmsession->timedemo() == false) {
//              setFrameDelay(nADelay);
//            }
//          }
//	  */
//
//          if(m_State == GS_DEADJUST) {
//            /* Hmm, you're dead and you know it. */
//            _PostUpdateJustDead();
//          }
//         
//          /* Context menu? */
//          if(m_State == GS_PREPLAYING || m_State == GS_PLAYING || !m_bEnableContextHelp)
//            m_Renderer->getGUI()->enableContextMenuDrawing(false);
//          else
//            m_Renderer->getGUI()->enableContextMenuDrawing(true);
//          
//          /* Draw GUI */
//	  // only if it's not the autozoom camera
//	  if(m_MotoGame.getCurrentCamera() != m_MotoGame.getNumberCameras()){
//	    m_Renderer->getGUI()->paint();        
//	  }
//        
//          break;
//        }
//        catch(Exception &e) {
//	  Logger::Log("** Warning ** : drawFrame failed ! (%s)", e.getMsg().c_str());
//	  // it doesn't work
//	  m_MotoGame.endLevel();
//	  setState(m_StateAfterPlaying);
//          notifyMsg(splitText(e.getMsg(), 50));
//        }
//      }
//    }

  return false;
}

bool StatePlaying::render()
{
  return StateScene::render();
}

void StatePlaying::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {
  case SDLK_ESCAPE:
    //if(LockedMotoGame() == false) {
      /* Escape pauses */
      m_pGame->getStateManager()->pushState(new StatePause(m_pGame));
      //}
    break;
//  case SDLK_F2:
//    switchFollowCamera();
//      break;
//  case SDLK_F3:
//    switchLevelToFavorite(m_MotoGame.getLevelSrc()->Id(), true);
//    break;
//  case SDLK_PAGEUP:
//    if(isThereANextLevel(m_PlaySpecificLevelId)) {
//      m_db->stats_abortedLevel(m_xmsession->profile(), m_MotoGame.getLevelSrc()->Id(), m_MotoGame.getTime());
//      m_MotoGame.endLevel();
//      m_Renderer->unprepareForNewLevel();
//      m_PlaySpecificLevelId = _DetermineNextLevel(m_PlaySpecificLevelId);
//      m_bPrePlayAnim = true;
//      setState(GS_PREPLAYING);
//    }
//    break;
//  case SDLK_PAGEDOWN:
//    if(isThereAPreviousLevel(m_PlaySpecificLevelId)) {
//      m_db-> stats_abortedLevel(m_xmsession->profile(), m_MotoGame.getLevelSrc()->Id(), m_MotoGame.getTime());
//	m_MotoGame.endLevel();
//	m_Renderer->unprepareForNewLevel();
//	m_PlaySpecificLevelId = _DeterminePreviousLevel(m_PlaySpecificLevelId);
//	m_bPrePlayAnim = true;
//	setState(GS_PREPLAYING);
//    }
//    break;
  case SDLK_RETURN:
    /* retart immediatly the level */
    m_pGame->restartLevel();
    break;
//  case SDLK_F5:
//    restartLevel(true);
//    break;
//    
  default:
    /* Notify the controller */
    m_pGame->getInputHandler()->handleInput(INPUT_KEY_DOWN, nKey, mod,
					    m_pGame->getMotoGame()->Players(),
					    m_pGame->getMotoGame()->Cameras(),
					    m_pGame);
  }
  
  StateScene::keyDown(nKey, mod, nChar);
}

void StatePlaying::keyUp(int nKey, SDLMod mod)
{
  m_pGame->getInputHandler()->handleInput(INPUT_KEY_UP,nKey,mod,
			     m_pGame->getMotoGame()->Players(),
			     m_pGame->getMotoGame()->Cameras(),
			     m_pGame);

  StateScene::keyUp(nKey, mod);
}

void StatePlaying::mouseDown(int nButton)
{
  m_pGame->getInputHandler()->handleInput(INPUT_KEY_DOWN,nButton,KMOD_NONE,
			     m_pGame->getMotoGame()->Players(),
			     m_pGame->getMotoGame()->Cameras(),
			     m_pGame);

  StateScene::mouseDown(nButton);
}

void StatePlaying::mouseDoubleClick(int nButton)
{
  StateScene::mouseDoubleClick(nButton);
}

void StatePlaying::mouseUp(int nButton)
{
  m_pGame->getInputHandler()->handleInput(INPUT_KEY_UP,nButton,KMOD_NONE,
			     m_pGame->getMotoGame()->Players(),
			     m_pGame->getMotoGame()->Cameras(),
			     m_pGame);

  StateScene::mouseUp(nButton);
}
