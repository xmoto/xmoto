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

#include "BikeGhost.h"
#include "../Replay.h"
#include "../GameText.h"
#include "../Game.h"

Ghost::Ghost(std::string i_replayFile, bool i_isActiv,
	     Theme *i_theme, BikerTheme* i_bikerTheme,
	     const TColor& i_colorFilter,
	     const TColor& i_uglyColorFilter) : Biker(i_theme, i_bikerTheme,
						      i_colorFilter,
						      i_uglyColorFilter) {
  std::string v_levelId;
  std::string v_playerName;

  m_replay = new Replay();
  v_levelId = m_replay->openReplay(i_replayFile, v_playerName);

  m_replay->peekState(m_previous_ghostBikeState);
  m_replay->peekState(m_next_ghostBikeState);
  BikeState::updateStateFromReplay(&m_previous_ghostBikeState, &m_bikeState);

  m_diffToPlayer = 0.0;

  m_info = "";
  m_isActiv = i_isActiv;
}

Ghost::~Ghost() {
}

void Ghost::execReplayEvents(float i_time, MotoGame *i_motogame) {
  std::vector<RecordedGameEvent *> *v_replayEvents;
  v_replayEvents = m_replay->getEvents();
  
  /* Start looking for events that should be passed */
  for(int i=0;i<v_replayEvents->size();i++) {
    /* Not passed? And with a time stamp that tells it should have happened
       by now? */
    if(!(*v_replayEvents)[i]->bPassed && (*v_replayEvents)[i]->Event->getEventTime() < i_time) {
      /* Nice. Handle this event, replay style */
      i_motogame->handleEvent((*v_replayEvents)[i]->Event);
      
      /* Pass it */
      (*v_replayEvents)[i]->bPassed = true;
    }
  }
  
  /* Now see if we have moved back in time and whether we should apply some
     REVERSE events */
  for(int i=v_replayEvents->size()-1;i>=0;i--) {
    /* Passed? And with a time stamp larger than current time? */
    if((*v_replayEvents)[i]->bPassed && (*v_replayEvents)[i]->Event->getEventTime() > i_time) {
      /* Nice. Handle this event, replay style BACKWARDS */
      (*v_replayEvents)[i]->Event->revert(i_motogame);
      
      /* Un-pass it */
      (*v_replayEvents)[i]->bPassed = false;
    }
  }
}

void Ghost::setInfo(std::string i_info) {
  m_info = i_info;
}

std::string Ghost::getDescription() const {
  char c_tmp[1024];

  snprintf(c_tmp, 1024,
	   GAMETEXT_GHOSTOF,
	   m_replay->getPlayerName().c_str());

  return
    std::string(c_tmp)   +
    "\n(" + m_info + ")" +
    "\n(" + GameApp::formatTime(m_replay->getFinishTime()) + ")";
}

std::string Ghost::playerName() {
  return m_replay->getPlayerName();
}

void Ghost::initLastToTakeEntities(Level* i_level) {
  std::vector<RecordedGameEvent *> *v_replayEvents;
  v_replayEvents = m_replay->getEvents();
    
  m_lastToTakeEntities.clear();
  m_diffToPlayer = 0.0;

  /* Start looking for events */
  for(int i=0; i<v_replayEvents->size(); i++) {
    MotoGameEvent *v_event = (*v_replayEvents)[i]->Event;
      
    if(v_event->getType() == GAME_EVENT_ENTITY_DESTROYED) {
      if(i_level->getEntityById(((MGE_EntityDestroyed*)v_event)->EntityId()).IsToTake()) {
	/* new Strawberry for ghost */
	m_lastToTakeEntities.push_back((*v_replayEvents)[i]->Event->getEventTime());
      }
    }
  }
}

float Ghost::diffToPlayer() const {
  return m_diffToPlayer;
}

void Ghost::updateDiffToPlayer(std::vector<float> &i_lastToTakeEntities) {
  int v_n;
    
  /* no strawberry, no update */
  if(i_lastToTakeEntities.size() == 0) {
    return;
  }
    
  /* the ghost did not get this number of strawberries */
  if(m_lastToTakeEntities.size() < i_lastToTakeEntities.size() ) {
    return;
  }
    
  m_diffToPlayer = i_lastToTakeEntities[i_lastToTakeEntities.size()-1]
                 - m_lastToTakeEntities[i_lastToTakeEntities.size()-1];
}

void Ghost::updateToTime(float i_time, float i_timeStep,
			 CollisionSystem *i_collisionSystem, Vector2f i_gravity,
			 MotoGame *i_motogame) {
  Biker::updateToTime(i_time, i_timeStep, i_collisionSystem, i_gravity, i_motogame);

  /* back in the past */
  if(m_previous_ghostBikeState.fGameTime > i_time) {
    m_replay->fastrewind(m_previous_ghostBikeState.fGameTime - i_time);
    m_replay->peekState(m_previous_ghostBikeState);
    m_replay->peekState(m_next_ghostBikeState);
    m_finished = false;
    m_dead     = false;
  }
  
  /* stop the motor at the end */
  if(m_replay->endOfFile()) {
    m_bikeState.fBikeEngineRPM = 0.0;
    if(m_replay->didFinish()) {
      m_finished   = true;
      m_finishTime = m_replay->getFinishTime();
    } else {
      m_dead = true;
    }
  } else {
    if(m_next_ghostBikeState.fGameTime < i_time && m_replay->endOfFile() == false) {
      do {
	m_replay->loadState(m_previous_ghostBikeState);
	m_replay->peekState(m_next_ghostBikeState);
      } while(m_next_ghostBikeState.fGameTime < i_time && m_replay->endOfFile() == false);
      BikeState::updateStateFromReplay(&m_previous_ghostBikeState, &m_bikeState);
    } else { /* interpolation */
      if(m_doInterpolation) {
	if(m_next_ghostBikeState.fGameTime - m_previous_ghostBikeState.fGameTime > 0.0) {
	  /* INTERPOLATED FRAME */
	  SerializedBikeState ibs;
	  float v_interpolation_value =
	    (i_time - m_previous_ghostBikeState.fGameTime)
	    /(m_next_ghostBikeState.fGameTime - m_previous_ghostBikeState.fGameTime);
	  
	  BikeState::interpolateGameState(&m_previous_ghostBikeState,
					  &m_next_ghostBikeState,
					  &ibs,
					  v_interpolation_value);
	  BikeState::updateStateFromReplay(&ibs, &m_bikeState);
	}
      }
    }
  }
  
  if(m_isActiv) {
    execReplayEvents(i_time, i_motogame);
  }
}

float Ghost::getFinishTime() {
  return m_replay->getFinishTime();
}

std::string Ghost::levelId() {
   return m_replay->getLevelId();
}

bool Ghost::getRenderBikeFront() {
  return true;
}

float Ghost::getBikeLinearVel() {
	return 0.0; /* unable to know it */
}

float Ghost::getBikeEngineSpeed() {
  return 0.0; /* unable to know it */
}
