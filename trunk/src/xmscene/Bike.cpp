/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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

#include "Bike.h"
#include "../Replay.h"
#include "../MotoGame.h"
#include "../GameEvents.h"
#include "../GameText.h"

#define PHYSICAL_ENGINE_REDUCTION 0.05

BikeState::BikeState() {
  reInitializeSpeed();
  reInitializeAnchors();
}

BikeState::~BikeState() {
}

void BikeState::reInitializeSpeed() {
  m_curBrake  = 0.0;
  m_curEngine = 0.0;
}

float BikeState::CurrentBrake() const {
  return m_curBrake;
}

float BikeState::CurrentEngine() const {
  return m_curEngine;
}

void BikeState::physicalUpdate() {
  if(m_curEngine > 0.0)
    m_curEngine -= m_bikeParameters.MaxEngine() * PHYSICAL_ENGINE_REDUCTION; 
}

void BikeState::reInitializeAnchors() {
  m_bikeAnchors.update(m_bikeParameters);
}

BikeAnchors& BikeState::Anchors() {
  return m_bikeAnchors;
}

BikeParameters& BikeState::Parameters() {
  return m_bikeParameters;
}

Ghost::Ghost(std::string i_replayFile) {
  std::string v_levelId;
  std::string v_playerName;
  float v_framerate;
  SerializedBikeState GhostBikeState;

  m_replay = new vapp::Replay();
  v_levelId = m_replay->openReplay(i_replayFile, &v_framerate, v_playerName);

  m_replay->peekState(GhostBikeState);
  vapp::MotoGame::updateStateFromReplay(&GhostBikeState, &m_bikeState);

  m_diffToPlayer = 0.0;
  m_nFrame = 0;

  m_info = "";
}

Ghost::~Ghost() {
}

void Ghost::setInfo(std::string i_info) {
  m_info = i_info;
}

std::string Ghost::getDescription() const {
  return
    std::string(GAMETEXT_GHOSTOF)   +
    " " + m_replay->getPlayerName() +
    "\n(" + m_info + ")"            +
    "\n(" + vapp::App::formatTime(m_replay->getFinishTime()) + ")";
}

void Ghost::initLastToTakeEntities(Level* i_level) {
  std::vector<vapp::RecordedGameEvent *> *v_replayEvents;
  v_replayEvents = m_replay->getEvents();
    
  m_lastToTakeEntities.clear();
  m_diffToPlayer = 0.0;

  /* Start looking for events */
  for(int i=0; i<v_replayEvents->size(); i++) {
    vapp::MotoGameEvent *v_event = (*v_replayEvents)[i]->Event;
      
    if(v_event->getType() == vapp::GAME_EVENT_ENTITY_DESTROYED) {
      if(i_level->getEntityById(((vapp::MGE_EntityDestroyed*)v_event)->EntityId()).IsToTake()) {
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

BikeState* Ghost::getState() {
  return &m_bikeState;
}

void Ghost::updateToTime(float i_time) {
  /* Read replay state */
  static SerializedBikeState GhostBikeState;
  static SerializedBikeState previousGhostBikeState;
    
  m_replay->peekState(previousGhostBikeState);
  if(previousGhostBikeState.fGameTime < i_time && m_replay->endOfFile() == false) {
    do {
      m_replay->loadState(GhostBikeState);
    } while(GhostBikeState.fGameTime < i_time && m_replay->endOfFile() == false);
    
    if(m_nFrame%2 || m_nFrame==1) {
      /* NON-INTERPOLATED FRAME */
      vapp::MotoGame::updateStateFromReplay(&GhostBikeState, &m_bikeState);
    } 
    else {
      /* INTERPOLATED FRAME */
	SerializedBikeState ibs;
	vapp::MotoGame::interpolateGameState(&previousGhostBikeState,&GhostBikeState,&ibs,0.5f);
	vapp::MotoGame::updateStateFromReplay(&ibs, &m_bikeState);
    }
    m_nFrame++;
  }
}
