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

#include "StateScene.h"
#include "../PhysSettings.h"
#include "../xmscene/Camera.h"
#include "../XMSession.h"
#include "../xmscene/Entity.h"
#include "../xmscene/Scene.h"
#include "StateMessageBox.h"
#include "../drawlib/DrawLib.h"
#include "StatePreplayingGame.h"
#include "../helpers/Log.h"
#include "../helpers/Text.h"
#include "../CameraAnimation.h"
#include "../Renderer.h"
#include "../Universe.h"
#include "../VideoRecorder.h"
#include "../GameText.h"
#include "../Game.h"
#include "StateMainMenu.h"

#define INPLAY_ANIMATION_TIME 1.0
#define INPLAY_ANIMATION_SPEED 10
#define PRESTART_ANIMATION_MARGIN_SIZE 5

/* control the particle generation by ask the particle renders to limit themself if there are too much particles on the screen */
#define NB_PARTICLES_TO_RENDER_LIMITATION 512

#define STATS_LEVELS_NOTES_SIZE 15

void StateScene::init() {
  Sprite *v_sprite;

  m_fLastPhysTime = -1.0;
  // while playing, we want 100 fps for the physic
  m_updateFps     = 100;
  m_showCursor = false;
  m_cameraAnim = NULL;
  m_universe   = NULL;

  m_benchmarkNbFrame   = 0;
  m_benchmarkStartTime = GameApp::getXMTime();

  /* stats */
  m_difficulty = -1.0;
  m_quality    = -1.0;
    
  m_uncheckedTex = m_qualityTex = m_difficultyTex = NULL;
  v_sprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "qsChoiceUnchecked");
  if(v_sprite != NULL) {
    m_uncheckedTex = v_sprite->getTexture();
  }
  
  v_sprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "qsChoiceQuality");
  if(v_sprite != NULL) {
    m_qualityTex = v_sprite->getTexture();
  }
  
  v_sprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "qsChoiceDifficulty");
  if(v_sprite != NULL) {
    m_difficultyTex = v_sprite->getTexture();
  }

  // message registering
  initMessageRegistering();
}

StateScene::StateScene(bool i_doShade, bool i_doShadeAnim)
: GameState(false, false, i_doShade, i_doShadeAnim)
{
  init();
}

StateScene::StateScene(Universe* i_universe, bool i_doShade, bool i_doShadeAnim)
  : GameState(false, false, i_doShade, i_doShadeAnim)
{
  init();
  m_universe   = i_universe;
}

StateScene::~StateScene()
{
  StateManager::instance()->unregisterAsObserver("ERROR", this);
  StateManager::instance()->unregisterAsObserver("FINISH", this);
  StateManager::instance()->unregisterAsObserver("RESTART", this);
  StateManager::instance()->unregisterAsObserver("NEXTLEVEL", this);
  StateManager::instance()->unregisterAsObserver("PREVIOUSLEVEL", this);
  StateManager::instance()->unregisterAsObserver("ABORT", this);
  StateManager::instance()->unregisterAsObserver("INTERPOLATION_CHANGED", this);
  StateManager::instance()->unregisterAsObserver("MIRRORMODE_CHANGED", this);

  if(m_cameraAnim != NULL) {
    delete m_cameraAnim;
  }
}

void StateScene::initMessageRegistering()
{
  StateManager::instance()->registerAsObserver("ERROR", this);
  StateManager::instance()->registerAsObserver("FINISH", this);
  StateManager::instance()->registerAsObserver("RESTART", this);
  StateManager::instance()->registerAsObserver("NEXTLEVEL", this);
  StateManager::instance()->registerAsObserver("PREVIOUSLEVEL", this);
  StateManager::instance()->registerAsObserver("ABORT", this);
  StateManager::instance()->registerAsObserver("INTERPOLATION_CHANGED", this);
  StateManager::instance()->registerAsObserver("MIRRORMODE_CHANGED", this);

  if(XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("FAVORITES_UPDATED");
    StateManager::instance()->registerAsEmitter("BLACKLISTEDLEVELS_UPDATED");
  }
}

void StateScene::enter()
{
  GameState::enter();

  ParticlesSource::setAllowParticleGeneration(true);
  m_isLockedScene = false;
  m_autoZoom      = false;

  m_benchmarkNbFrame   = 0;
  m_benchmarkStartTime = GameApp::getXMTime();

  m_fLastPhysTime = GameApp::getXMTime();
}

void StateScene::enterAfterPop()
{
  GameState::enterAfterPop();
}

bool StateScene::update()
{
  if(doUpdate() == false){
    return false;
  }

  try {
    int nPhysSteps = 0;
    
    if(isLockedScene() == false) {  
      // don't update if that's not required
      // don't do this infinitely, maximum miss 10 frames, then give up
      // in videoRecording mode, don't try to do more to allow to record at a good framerate
      while (( m_fLastPhysTime + (PHYS_STEP_SIZE)/100.0 <= GameApp::getXMTime()) && nPhysSteps < 10 && (XMSession::instance()->enableVideoRecording() == false || nPhysSteps == 0)) {
	if(m_universe != NULL) {
	  for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	    m_universe->getScenes()[i]->updateLevel(PHYS_STEP_SIZE, m_universe->getCurrentReplay());
	  }
	}
	m_fLastPhysTime += PHYS_STEP_SIZE/100.0;
	nPhysSteps++;
      }
      
      // if the delay is too long, reinitialize
      if(m_fLastPhysTime + PHYS_STEP_SIZE/100.0 < GameApp::getXMTime()) {
	m_fLastPhysTime = GameApp::getXMTime();
      }
      
      // update camera scrolling
      if(m_universe != NULL) {
	for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	  for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	    m_universe->getScenes()[j]->Cameras()[i]->setScroll(true, m_universe->getScenes()[j]->getGravity());
	  }
	}
      }
    }
  } catch(Exception &e) {
    StateManager::instance()->replaceState(new StateMessageBox(NULL, splitText(e.getMsg(), 50), UI_MSGBOX_OK));  
  }

  runAutoZoom();

  return true;
}

bool StateScene::render()
{
  GameApp*  pGame = GameApp::instance();

  if (XMSession::instance()->ugly()) {
    pGame->getDrawLib()->clearGraphics();
  }

  try {
    if(autoZoom() == false){
      if(m_universe != NULL) {
	for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	  for(unsigned int i=0; i<m_universe->getScenes()[j]->getNumberCameras(); i++) {
	    m_universe->getScenes()[j]->setCurrentCamera(i);
	    GameRenderer::instance()->render(m_universe->getScenes()[j]);
	  }
	}
      }
    } else {
      if(m_universe != NULL) {
	if(m_universe->getScenes().size() > 0) {
	  m_universe->getScenes()[0]->setAutoZoomCamera();
	  GameRenderer::instance()->render(m_universe->getScenes()[0]);
	}
      }
    }

    ParticlesSource::setAllowParticleGeneration(GameRenderer::instance()->nbParticlesRendered() < NB_PARTICLES_TO_RENDER_LIMITATION);
  } catch(Exception &e) {
    StateManager::instance()->replaceState(new StateMessageBox(NULL, splitText(e.getMsg(), 50), UI_MSGBOX_OK));
  }

  GameState::render();
  m_benchmarkNbFrame++;

  return true;
}

void StateScene::onRenderFlush() {
  // take a screenshot
  if(XMSession::instance()->enableVideoRecording()) {
    if(StateManager::instance()->getVideoRecorder() != NULL) {
      if(m_universe->getScenes().size() > 0) {
	if( (XMSession::instance()->videoRecordingStartTime() < 0 ||
	     XMSession::instance()->videoRecordingStartTime() <= m_universe->getScenes()[0]->getTime()
	     )
	    &&
	    (XMSession::instance()->videoRecordingEndTime() < 0 ||
	     XMSession::instance()->videoRecordingEndTime() >= m_universe->getScenes()[0]->getTime()
	     )
	    ) {
	  StateManager::instance()->getVideoRecorder()->read(m_universe->getScenes()[0]->getTime());
	}
      }
    }
  }
}

void StateScene::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  GameApp* pGame = GameApp::instance();  

  if(i_xmkey == XMKey(SDLK_TAB, KMOD_NONE)) {
    if(i_type == INPUT_UP) {
      if(m_cameraAnim != NULL) {
	if(autoZoom() && m_cameraAnim->allowNextStep()) {
	  m_cameraAnim->goNextStep();
	}
      }
    } else {
      if(autoZoom() == false) {
	setAutoZoom(true);
      }
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_RETURN, KMOD_NONE)) {
    restartLevel();
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_F2, KMOD_NONE)) {
    if(m_universe != NULL) {
      m_universe->switchFollowCamera();
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == InputHandler::instance()->getSwitchFavorite()) {
    if(m_universe != NULL) {
      if(m_universe->getScenes().size() > 0) { // just add the first world
	pGame->switchLevelToFavorite(m_universe->getScenes()[0]->getLevelSrc()->Id(), true);
	StateManager::instance()->sendAsynchronousMessage("FAVORITES_UPDATED");
      }
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == InputHandler::instance()->getSwitchBlacklist()) {
    if(m_universe != NULL) {
      if(m_universe->getScenes().size() > 0) { // just blacklist the first world
	pGame->switchLevelToBlacklist(m_universe->getScenes()[0]->getLevelSrc()->Id(), true);
	StateManager::instance()->sendAsynchronousMessage("BLACKLISTEDLEVELS_UPDATED");
      }
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_PAGEUP, KMOD_NONE)) {
    nextLevel();
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_PAGEDOWN, KMOD_NONE)) {
    nextLevel(false);
  }

#if defined(ENABLE_DEV)
  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP7, KMOD_NONE)) {
    /* Zoom in */
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->desactiveActionZoom();
	  m_universe->getScenes()[j]->Cameras()[i]->setRelativeZoom(0.002);
	}
      }
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP9, KMOD_NONE)) {
    /* Zoom out */
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->desactiveActionZoom();
	  m_universe->getScenes()[j]->Cameras()[i]->setRelativeZoom(-0.002);
	}
      }
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_HOME, KMOD_NONE)) {
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->desactiveActionZoom();
	  m_universe->getScenes()[j]->Cameras()[i]->initCamera();
	}
      }
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP6, KMOD_NONE)) {
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->desactiveActionZoom();
	  m_universe->getScenes()[j]->Cameras()[i]->moveCamera(1.0, 0.0);
	}
      }
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP4, KMOD_NONE)) {
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->desactiveActionZoom();
	  m_universe->getScenes()[j]->Cameras()[i]->moveCamera(-1.0, 0.0);
	}
      }
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP8, KMOD_NONE)) {
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->desactiveActionZoom();
	  m_universe->getScenes()[j]->Cameras()[i]->moveCamera(0.0, 1.0);
	}
      }
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP2, KMOD_NONE)) {
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->desactiveActionZoom();
	  m_universe->getScenes()[j]->Cameras()[i]->moveCamera(0.0, -1.0);
	}
      }
    }
  }
#endif

  else {
    GameState::xmKey(i_type, i_xmkey);
  }
}

void StateScene::sendFromMessageBox(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  if(i_id == "ERROR") {
    addCommand("ERROR");
  }
}

void StateScene::setScoresTimes() {
  char **v_result;
  unsigned int nrow;
  char *v_res;  
  std::string T1 = "--:--:--", T2 = "--:--:--";

  std::string v_id_level;
  // take the level id of the first world
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() > 0) {
      v_id_level = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  /* get best result */
  v_result = xmDatabase::instance("main")->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
						  "id_level=\"" + 
						  xmDatabase::protectString(v_id_level) + "\";",
						  nrow);
  v_res = xmDatabase::instance("main")->getResult(v_result, 1, 0, 0);
  if(v_res != NULL) {
    T1 = formatTime(atoi(v_res));
  }
  xmDatabase::instance("main")->read_DB_free(v_result);
    
  /* get best player result */
  v_result = xmDatabase::instance("main")->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
						  "id_level=\"" + 
						  xmDatabase::protectString(v_id_level) + "\" " + 
						  "AND id_profile=\"" + xmDatabase::protectString(XMSession::instance()->profile())  + "\";",
						  nrow);
  v_res = xmDatabase::instance("main")->getResult(v_result, 1, 0, 0);
  if(v_res != NULL) {
    T2 = formatTime(atoi(v_res));
  }
  xmDatabase::instance("main")->read_DB_free(v_result);
    
  if(XMSession::instance()->hidePlayingInformation() == false) {
    GameRenderer::instance()->setBestTime(T1 + std::string(" / ") + T2);
  } else {
    GameRenderer::instance()->setBestTime("");
  }

  if(XMSession::instance()->showHighscoreInGame() && XMSession::instance()->hidePlayingInformation() == false) {
    std::string v_strWorldRecord;
    for(unsigned int i=0; i<XMSession::instance()->nbRoomsEnabled(); i++) {
      v_strWorldRecord += GameApp::instance()->getWorldRecord(i, v_id_level) + "\n";
    }
    GameRenderer::instance()->setWorldRecordTime(v_strWorldRecord);
  } else {
    GameRenderer::instance()->setWorldRecordTime("");
  }
}

void StateScene::abortPlaying() {
  closePlaying();
}

void StateScene::closePlaying() {
  if(m_universe != NULL) {
    delete m_universe;
    m_universe = NULL;
  }

  InputHandler::instance()->resetScriptKeyHooks();                     
  GameRenderer::instance()->unprepareForNewLevel();
}

bool StateScene::isLockedScene() const {
  return m_isLockedScene;
}

void StateScene::lockScene(bool i_value) {
  m_isLockedScene = i_value;
  if(m_isLockedScene == false){
    m_fLastPhysTime = GameApp::getXMTime();
  }
}

void StateScene::setAutoZoom(bool i_value) {
  if(m_autoZoom == false && i_value == true) {
    lockScene(true);

    if(m_cameraAnim != NULL) {
      delete m_cameraAnim;
    }
    GameApp*  pGame = GameApp::instance();

    if(m_universe != NULL) {
      if(m_universe->getScenes().size() > 0) { // do only for the first world for the moment
	m_universe->getScenes()[0]->setAutoZoomCamera();
	m_cameraAnim = new AutoZoomCameraAnimation(m_universe->getScenes()[0]->getCamera(),
						   pGame->getDrawLib(),
						   m_universe->getScenes()[0]);
	m_cameraAnim->init();
      }
    }
  }

  m_autoZoom = i_value;
}

bool StateScene::autoZoom() const {
  return m_autoZoom;
}

void StateScene::runAutoZoom() {
  if(autoZoom()) {
    if(m_cameraAnim != NULL) {
      if(m_cameraAnim->step() == false) {
	lockScene(false);
	m_cameraAnim->uninit();
	setAutoZoom(false);
      }
    }
  }
}

void StateScene::executeOneCommand(std::string cmd, std::string args)
{
  LogDebug("cmd [%s [%s]] executed by state [%s].",
	   cmd.c_str(), args.c_str(), getName().c_str());

  if(cmd == "ERROR") {
    closePlaying();

    // there is no other state before
    if(StateManager::instance()->numberOfStates() == 1) {
      // run the mainmenu state
      StateManager::instance()->replaceState(new StateMainMenu());
    } else {
      m_requestForEnd = true;
    }
  }

  else if(cmd == "FINISH") {
    closePlaying();

    // there is no other state before
    if(StateManager::instance()->numberOfStates() == 1) {
      // run the mainmenu state
      StateManager::instance()->replaceState(new StateMainMenu());
    } else {
      m_requestForEnd = true;
    }
  }

  else if(cmd == "RESTART") {
    restartLevel();
  }

  else if(cmd == "NEXTLEVEL") {
    nextLevel();
  }

  else if(cmd == "PREVIOUSLEVEL") {
    nextLevel(false);
  }

  else if(cmd == "ABORT") {
    abortPlaying();

    // there is no other state before
    if(StateManager::instance()->numberOfStates() == 1) {
      // run the mainmenu state
      StateManager::instance()->replaceState(new StateMainMenu());
    } else {
      m_requestForEnd = true;
    }
  }

  else if(cmd == "INTERPOLATION_CHANGED") {
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Players().size(); i++) {
	  m_universe->getScenes()[j]->Players()[i]->setInterpolation(XMSession::instance()->enableReplayInterpolation());
	}
      }
    }
  }

  else if(cmd == "MIRRORMODE_CHANGED") {
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Cameras().size(); i++) {
	  m_universe->getScenes()[j]->Cameras()[i]->setMirrored(XMSession::instance()->mirrorMode());
	}
      }
    }
  }

  else {
    GameState::executeOneCommand(cmd, args);
  }
}

void StateScene::displayStats() {
  FontManager* v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
  FontGlyph* v_fg   = GameApp::instance()->getDrawLib()->getFontSmall()->getGlyph(m_statsStr);
  Vector2f A = Vector2f(GameApp::instance()->getDrawLib()->getDispWidth() - v_fg->realWidth(),
			GameApp::instance()->getDrawLib()->getDispHeight() - v_fg->realHeight());
  Vector2f B= Vector2f(GameApp::instance()->getDrawLib()->getDispWidth(),
		       GameApp::instance()->getDrawLib()->getDispHeight());
  int vborder = 10;

  GameApp::instance()->getDrawLib()->drawBox(A - Vector2f(vborder*2, vborder*2),
					     B,
					     1.0f, 0xFFCCCC77, 0xFFFFFFFF);

  v_fm->printString(v_fg,
		    GameApp::instance()->getDrawLib()->getDispWidth() - v_fg->realWidth()   - vborder,
		    GameApp::instance()->getDrawLib()->getDispHeight() - v_fg->realHeight() - vborder,
		    MAKE_COLOR(220,255,255,255), -1.0, true);

  // quality
  int v_quality_yoffset = 5;

  if(m_quality >= 0.0) {
    v_fg = GameApp::instance()->getDrawLib()->getFontSmall()->getGlyph(GAMETEXT_QUALITY);
    v_fm->printString(v_fg,
		      A.x - vborder*2,
		      A.y - vborder*2 - v_fg->realHeight() - STATS_LEVELS_NOTES_SIZE -v_quality_yoffset,
		      MAKE_COLOR(220,255,255,255), -1.0, true);
    
    if(XMSession::instance()->ugly()) {
      for(int i=0; i<(int)(m_quality); i++) {
	GameApp::instance()->getDrawLib()->drawCircle(Vector2f(A.x - vborder*2 + STATS_LEVELS_NOTES_SIZE/2 + (STATS_LEVELS_NOTES_SIZE*i),
							       A.y - vborder*2 - STATS_LEVELS_NOTES_SIZE/2 -v_quality_yoffset),
						      STATS_LEVELS_NOTES_SIZE/2,
						      1.0, 0, MAKE_COLOR(255, 0, 0, 255));
      }
    } else {
      for(int i=0; i<5; i++) {
	if(i<(int)(m_quality)) {
	  GameApp::instance()->getDrawLib()->drawImage(Vector2f(A.x - vborder*2 + (STATS_LEVELS_NOTES_SIZE*i),
								A.y - vborder*2 - STATS_LEVELS_NOTES_SIZE -v_quality_yoffset),
						       Vector2f(A.x - vborder*2 + STATS_LEVELS_NOTES_SIZE + (STATS_LEVELS_NOTES_SIZE*i),
								A.y - vborder*2 -v_quality_yoffset),
						       m_qualityTex, 0xFFFFFFFF, true);
	} else {
	  GameApp::instance()->getDrawLib()->drawImage(Vector2f(A.x - vborder*2 + (STATS_LEVELS_NOTES_SIZE*i),
								A.y - vborder*2 - STATS_LEVELS_NOTES_SIZE -v_quality_yoffset),
						       Vector2f(A.x - vborder*2 + STATS_LEVELS_NOTES_SIZE + (STATS_LEVELS_NOTES_SIZE*i),
								A.y - vborder*2 -v_quality_yoffset),
						       m_uncheckedTex, 0xFFFFFFFF, true);
	}
      }
    }
  }

  // difficulty
  int v_difficulty_yoffset = v_fg->realHeight() + STATS_LEVELS_NOTES_SIZE + v_quality_yoffset;

  if(m_difficulty >= 0.0) {
    v_fg = GameApp::instance()->getDrawLib()->getFontSmall()->getGlyph(GAMETEXT_DIFFICULTY);
    v_fm->printString(v_fg,
		      A.x - vborder*2,
		      A.y - vborder*2 - v_fg->realHeight() - STATS_LEVELS_NOTES_SIZE -v_difficulty_yoffset,
		      MAKE_COLOR(220,255,255,255), -1.0, true);
    
    if(XMSession::instance()->ugly()) {
      for(int i=0; i<(int)(m_difficulty); i++) {
	GameApp::instance()->getDrawLib()->drawCircle(Vector2f(A.x - vborder*2 + STATS_LEVELS_NOTES_SIZE/2 + (STATS_LEVELS_NOTES_SIZE*i),
							       A.y - vborder*2 - STATS_LEVELS_NOTES_SIZE/2 -v_difficulty_yoffset),
						      STATS_LEVELS_NOTES_SIZE/2,
						      1.0, 0, MAKE_COLOR(255, 0, 0, 255));
      }
    } else {
      for(int i=0; i<5; i++) {
	if(i<(int)(m_difficulty)) {
	  GameApp::instance()->getDrawLib()->drawImage(Vector2f(A.x - vborder*2 + (STATS_LEVELS_NOTES_SIZE*i),
								A.y - vborder*2 - STATS_LEVELS_NOTES_SIZE -v_difficulty_yoffset),
						       Vector2f(A.x - vborder*2 + STATS_LEVELS_NOTES_SIZE + (STATS_LEVELS_NOTES_SIZE*i),
							      A.y - vborder*2 -v_difficulty_yoffset),
						       m_difficultyTex, 0xFFFFFFFF, true);
	} else {
	  GameApp::instance()->getDrawLib()->drawImage(Vector2f(A.x - vborder*2 + (STATS_LEVELS_NOTES_SIZE*i),
								A.y - vborder*2 - STATS_LEVELS_NOTES_SIZE -v_difficulty_yoffset),
						       Vector2f(A.x - vborder*2 + STATS_LEVELS_NOTES_SIZE + (STATS_LEVELS_NOTES_SIZE*i),
								A.y - vborder*2 -v_difficulty_yoffset),
						       m_uncheckedTex, 0xFFFFFFFF, true);
	}
      }
    }
  }

}

void StateScene::makeStatsStr() {
  m_statsStr = "";
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() > 0) {
      
      // stats to display
      char **v_result;
      unsigned int nrow;
      xmDatabase* v_pDb = xmDatabase::instance("main");
      int v_nbPlayed, v_nbDied, v_nbCompleted, v_nbRestart, v_playedTime;
      std::string v_idLevel = m_universe->getScenes()[0]->getLevelSrc()->Id();
      
      v_result = v_pDb->readDB("SELECT IFNULL(SUM(nbPlayed),0), IFNULL(SUM(nbDied),0), IFNULL(SUM(nbCompleted),0), IFNULL(SUM(nbRestarted),0), IFNULL(SUM(playedTime),0) "
			       "FROM stats_profiles_levels "
			       "WHERE id_profile=\"" + xmDatabase::protectString(XMSession::instance()->profile()) + "\" "
			       "AND id_level=\""     + xmDatabase::protectString(v_idLevel) + "\" GROUP BY id_profile;",
			       nrow);
      if(nrow != 1) {
	/* not statistics */
	v_nbPlayed    = 0;
	v_nbDied      = 0;
	v_nbCompleted = 0;
	v_nbRestart   = 0;
	v_playedTime  = 0;
      } else {
	v_nbPlayed    = atoi(v_pDb->getResult(v_result, 5, 0, 0));
	v_nbDied      = atoi(v_pDb->getResult(v_result, 5, 0, 1));
	v_nbCompleted = atoi(v_pDb->getResult(v_result, 5, 0, 2));
	v_nbRestart   = atoi(v_pDb->getResult(v_result, 5, 0, 3));
	v_playedTime  = atoi(v_pDb->getResult(v_result, 5, 0, 4));
      }
      char c_tmp[256];
      snprintf(c_tmp, 256, std::string(GAMETEXT_XMOTOLEVELSTATS_PLAYS(v_nbPlayed)       + std::string("\n") +
				       GAMETEXT_XMOTOLEVELSTATS_FINISHED(v_nbCompleted) + std::string("\n") +
				       GAMETEXT_XMOTOLEVELSTATS_DEATHS(v_nbDied)        + std::string("\n") +
				       GAMETEXT_XMOTOLEVELSTATS_RESTART(v_nbRestart)    + std::string("\n") +
				       GAMETEXT_XMOTOGLOBALSTATS_TIMEPLAYED
				       ).c_str(),
	       v_nbPlayed, v_nbCompleted, v_nbDied, v_nbRestart,
	   formatTime(v_playedTime).c_str());
      m_statsStr = c_tmp;
      v_pDb->read_DB_free(v_result);


      // quality and difficulty
      v_result = v_pDb->readDB("SELECT difficulty, quality "
			       "FROM weblevels "
			       "WHERE id_level=\"" + xmDatabase::protectString(v_idLevel) + "\";",
			       nrow);
      if(nrow != 1) {
	/* not statistics */
	m_difficulty = -1.0;
	m_quality    = -1.0;
      } else {
	m_difficulty = atof(v_pDb->getResult(v_result, 2, 0, 0));
	m_quality    = atof(v_pDb->getResult(v_result, 2, 0, 1));
      }
      v_pDb->read_DB_free(v_result);
    }
  }
}

void StateScene::restartLevel(bool i_reloadLevel) {
  /* do nothing, it's depends of the scene ; often empty for animation steps */
}

void StateScene::nextLevel(bool i_positifOrder) {
  /* do nothing, it's depends of the scene ; often empty for animation steps */
}

void StateScene::playingNextLevel(bool i_positifOrder) {
  GameApp*  pGame  = GameApp::instance();
  std::string v_nextLevel;
  std::string v_currentLevel;
  
  // take the level id of the first world
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() > 0) {
      v_currentLevel = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  if(i_positifOrder) {
    v_nextLevel = pGame->determineNextLevel(v_currentLevel);
  } else {
    v_nextLevel = pGame->determinePreviousLevel(v_currentLevel);
  }

  /* update stats */
  if(v_nextLevel != "") {
    if(m_universe != NULL) {
      if(m_universe->getScenes().size() == 1) {
	if(m_universe->getScenes()[0]->Players().size() == 1) {
	  if(m_universe->getScenes()[0]->Players()[0]->isDead()     == false &&
	     m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
	    xmDatabase::instance("main")->stats_abortedLevel(XMSession::instance()->sitekey(),
							     XMSession::instance()->profile(),
							     v_currentLevel,
							     m_universe->getScenes()[0]->getTime());
	    StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
	  }
	}
      }
    }
  }

  nextLevelToPlay(i_positifOrder);
}

void StateScene::restartLevelToPlay(bool i_reloadLevel) {
  std::string v_level;
  
  // take the level id of the first world
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() > 0) {
      v_level = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  closePlaying();

  GameRenderer::instance()->unprepareForNewLevel();
  
  if(i_reloadLevel) {
    try {
      Level::removeFromCache(xmDatabase::instance("main"), v_level);
    } catch(Exception &e) {
      // hum, not nice
    }
  }

  StateManager::instance()->replaceState(new StatePreplayingGame(v_level, true));

}

void StateScene::nextLevelToPlay(bool i_positifOrder) {
  GameApp*  pGame  = GameApp::instance();
  std::string v_nextLevel;
  std::string v_currentLevel;
  
  // take the level id of the first world
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() > 0) {
      v_currentLevel = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  if(i_positifOrder) {
    v_nextLevel = pGame->determineNextLevel(v_currentLevel);
  } else {
    v_nextLevel = pGame->determinePreviousLevel(v_currentLevel);
  }

  if(v_nextLevel != "") {
    closePlaying();
    StateManager::instance()->replaceState(new StatePreplayingGame(v_nextLevel, v_currentLevel == v_nextLevel));
  }
}
