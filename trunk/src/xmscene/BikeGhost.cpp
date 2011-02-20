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
#include "../helpers/Text.h"
#include "../GameEvents.h"
#include "../Theme.h"
#include "Level.h"

#define INTERPOLATION_MAXIMUM_TIME  300
#define INTERPOLATION_MAXIMUM_SPACE 5.0

Ghost::Ghost(PhysicsSettings* i_physicsSettings,
	     bool i_engineSound,
	     Theme *i_theme, BikerTheme* i_bikerTheme,
	     const TColor& i_colorFilter,
	     const TColor& i_uglyColorFilter)
: Biker(i_physicsSettings, i_engineSound, i_theme, i_bikerTheme, i_colorFilter, i_uglyColorFilter) {
  m_diffToPlayer = 0.0;
  m_reference = false;
}

Ghost::~Ghost() {
}

float Ghost::diffToPlayer() const {
  return m_diffToPlayer;
}

bool Ghost::getRenderBikeFront() {
  return true;
}

void Ghost::setReference(bool i_value) {
  m_reference = i_value;
}

bool Ghost::isReference() const {
  return m_reference;
}

void Ghost::setInfo(const std::string& i_info) {
  m_info = i_info;
}

FileGhost::FileGhost(std::string i_replayFile, PhysicsSettings* i_physicsSettings,
		     bool i_isActiv,
		     bool i_engineSound,
		     Theme *i_theme, BikerTheme* i_bikerTheme,
		     const TColor& i_colorFilter,
		     const TColor& i_uglyColorFilter)
: Ghost (i_physicsSettings, i_engineSound, i_theme, i_bikerTheme, i_colorFilter, i_uglyColorFilter) {
  std::string v_levelId;
  std::string v_playerName;

  m_replay = new Replay();
  v_levelId = m_replay->openReplay(i_replayFile, v_playerName);

  // 4 states for cubical interpolation

  // 50% of states are before the time T, 50% are after
  m_ghostBikeStates.push_back(new BikeState(m_physicsSettings));
  m_ghostBikeStates.push_back(new BikeState(m_physicsSettings));
  m_ghostBikeStates.push_back(new BikeState(m_physicsSettings));
  m_ghostBikeStates.push_back(new BikeState(m_physicsSettings));


  for(unsigned int i=0; i<m_ghostBikeStates.size(); i++) {
    m_replay->peekState(m_ghostBikeStates[i], m_physicsSettings);
  }
  *m_bikeState = *(m_ghostBikeStates[0]); // copy

  m_isActiv = i_isActiv;
  m_linearVelocity = 0.0;
  m_teleportationOccured = false;
}

FileGhost::~FileGhost() {
  for(unsigned int i=0; i<m_ghostBikeStates.size(); i++) {
    delete m_ghostBikeStates[i];
  }
}

void FileGhost::execReplayEvents(int i_time, Scene *i_motogame) {
  std::vector<RecordedGameEvent *> *v_replayEvents;
  v_replayEvents = m_replay->getEvents();
  
  /* Start looking for events that should be passed */
  for(unsigned int i=0;i<v_replayEvents->size();i++) {
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

  // apply moving blocks
  std::vector<rmtime>* v_mb = m_replay->getMovingBlocks();
  for(unsigned int i=0; i<v_mb->size(); i++) {
    // reference the block
    if((*v_mb)[i].block == NULL) {
      (*v_mb)[i].block = i_motogame->getLevelSrc()->getBlockById((*v_mb)[i].name);
    }

    //
    if((*v_mb)[i].states.size() > 0) {

      // continue to read the time
      while((*v_mb)[i].states[(*v_mb)[i].readPos].time < i_time) {
	(*v_mb)[i].readPos++;
      }

      // not at the end
      if((*v_mb)[i].readPos < (*v_mb)[i].states.size()) {
	// and time is found
	if((*v_mb)[i].states[(*v_mb)[i].readPos].time == i_time) {
	  // apply the state
	  i_motogame->SetBlockPos((*v_mb)[i].block,
				  (*v_mb)[i].states[(*v_mb)[i].readPos].position.x,
				  (*v_mb)[i].states[(*v_mb)[i].readPos].position.y);
	  i_motogame->SetBlockRotation((*v_mb)[i].block, (*v_mb)[i].states[(*v_mb)[i].readPos].rotation);
	} else {
	  // interpolation
	  if(m_doInterpolation) {
	    // ok between frame 2 and n-1
	    if((*v_mb)[i].readPos > 0) {
	      float pertime = 
		((float)(i_time - (*v_mb)[i].states[(*v_mb)[i].readPos-1].time)) /
		((float)((*v_mb)[i].states[(*v_mb)[i].readPos].time - (*v_mb)[i].states[(*v_mb)[i].readPos-1].time));
	      
	      float new_pos_x = (*v_mb)[i].states[(*v_mb)[i].readPos-1].position.x +
		((*v_mb)[i].states[(*v_mb)[i].readPos].position.x - (*v_mb)[i].states[(*v_mb)[i].readPos-1].position.x)*pertime;
	      
	      float new_pos_y = (*v_mb)[i].states[(*v_mb)[i].readPos-1].position.y +
		((*v_mb)[i].states[(*v_mb)[i].readPos].position.y - (*v_mb)[i].states[(*v_mb)[i].readPos-1].position.y)*pertime;

	      float new_pos_r = interpolateAngle((*v_mb)[i].states[(*v_mb)[i].readPos-1].rotation, (*v_mb)[i].states[(*v_mb)[i].readPos].rotation, pertime);

	      i_motogame->SetBlockPos((*v_mb)[i].block, new_pos_x, new_pos_y);
	      i_motogame->SetBlockRotation((*v_mb)[i].block, new_pos_r);
	    }
	  }
	}
      }
    }
  }
}

std::string FileGhost::getDescription() const {
  char c_tmp[1024];

  snprintf(c_tmp, 1024,
	   GAMETEXT_GHOSTOF,
	   m_replay->getPlayerName().c_str());

  return
    std::string(c_tmp)   +
    "\n(" + m_info + ")" +
    "\n(" + formatTime(m_replay->getFinishTime()) + ")";
}

std::string FileGhost::getVeryQuickDescription() const {
  return m_replay->getPlayerName();
}

std::string FileGhost::getQuickDescription() const {
  char c_tmp[1024];

  snprintf(c_tmp, 1024,
	   GAMETEXT_GHOSTOF,
	   m_replay->getPlayerName().c_str()); 

  return std::string(c_tmp);
}

std::string FileGhost::playerName() {
  return m_replay->getPlayerName();
}

void FileGhost::initLastToTakeEntities(Level* i_level) {
  std::vector<RecordedGameEvent *> *v_replayEvents;
  v_replayEvents = m_replay->getEvents();
    
  m_lastToTakeEntities.clear();
  m_diffToPlayer = 0.0;

  /* Start looking for events */
  for(unsigned int i=0; i<v_replayEvents->size(); i++) {
    SceneEvent *v_event = (*v_replayEvents)[i]->Event;
      
    if(v_event->getType() == GAME_EVENT_ENTITY_DESTROYED) {
      if(i_level->getEntityById(((MGE_EntityDestroyed*)v_event)->EntityId())->IsToTake()) {
	/* new Strawberry for ghost */
	m_lastToTakeEntities.push_back((*v_replayEvents)[i]->Event->getEventTime());
      }
    }
  }
}

void FileGhost::updateDiffToPlayer(std::vector<float> &i_lastToTakeEntities) {
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

void FileGhost::initToPosition(Vector2f i_position, DriveDir i_direction, Vector2f i_gravity) {
  m_teleportationOccured = true;
  m_linearVelocity = 0.0;
}

void FileGhost::updateToTime(int i_time, int i_timeStep,
			 CollisionSystem *i_collisionSystem, Vector2f i_gravity,
			 Scene *i_motogame) {
  Biker::updateToTime(i_time, i_timeStep, i_collisionSystem, i_gravity, i_motogame);
  DriveDir v_previousDir = m_bikeState->Dir;

  /* back in the past */
  // m_ghostBikeStates.size()/2-1 : it's the more recent frame in the past
  if(m_ghostBikeStates[m_ghostBikeStates.size()/2-1]->GameTime > i_time) {
    m_replay->fastrewind(m_ghostBikeStates[m_ghostBikeStates.size()/2-1]->GameTime - i_time, 1);

    for(unsigned int i=0; i<m_ghostBikeStates.size(); i++) {
      m_replay->peekState(m_ghostBikeStates[i], m_physicsSettings);
    }

    m_finished = false;
    m_dead     = false;
  }

  /* stop the motor at the end */
  if(m_replay->endOfFile() && m_ghostBikeStates[m_ghostBikeStates.size()-1]->GameTime <= i_time) { // end of file, and interpolation is finished (before last future frame is in the feature)
    m_bikeState->fBikeEngineRPM = 0.0;
    m_replay->peekState(m_ghostBikeStates[m_ghostBikeStates.size()-1], m_physicsSettings); // take the last frame
    *m_bikeState = *(m_ghostBikeStates[m_ghostBikeStates.size()-1]); // copy last frame
    m_linearVelocity = 0.0;

    if(m_replay->didFinish()) {
      m_finished   = true;
      m_finishTime = m_replay->getFinishTime();
    } else {
      m_dead     = true;
      m_deadTime = m_bikeState->GameTime;
    }
  } else {
    BikeState* v_state;
    // to compute the velocity
    Vector2f v_beforePos;
    Vector2f v_afterPos;
    float v_beforeTime;
    float v_afterTime;

    // m_ghostBikeStates.size()/2 : first state in the feature
    if(m_ghostBikeStates[m_ghostBikeStates.size()/2]->GameTime < i_time) {
      
      bool v_didRead = false;
      do {
	// move the bikestates
	v_state = m_ghostBikeStates[0];
	for(unsigned int i=0; i<m_ghostBikeStates.size()-1; i++) {
	  m_ghostBikeStates[i] = m_ghostBikeStates[i+1];
	}
	m_ghostBikeStates[m_ghostBikeStates.size()-1] = v_state;

	if(m_replay->endOfFile()) {
	  // copy n-1 to n
	  *(m_ghostBikeStates[m_ghostBikeStates.size()-1]) = *(m_ghostBikeStates[m_ghostBikeStates.size()-1-1]);
	  v_didRead = false;
	} else {
	  // read the replay
	  m_replay->loadState(m_ghostBikeStates[m_ghostBikeStates.size()-1], m_physicsSettings);
	  m_replay->loadState(m_bikeState, m_physicsSettings);
	  v_didRead = true;
	}
      } while(m_ghostBikeStates[m_ghostBikeStates.size()/2]->GameTime < i_time && v_didRead);


      *m_bikeState = *(m_ghostBikeStates[m_ghostBikeStates.size()/2-1]); // copy

      v_beforePos  = m_ghostBikeStates[m_ghostBikeStates.size()/2-1]->CenterP;
      v_beforeTime = m_ghostBikeStates[m_ghostBikeStates.size()/2-1]->GameTime;
      v_afterPos   = m_ghostBikeStates[m_ghostBikeStates.size()/2]->CenterP;
      v_afterTime  = m_ghostBikeStates[m_ghostBikeStates.size()/2]->GameTime;
      
      // warning, absolutly no idea why * 2.0, but it is needed to work... (*100 because hundreadths, *3.6 for km/h)
      if(v_afterTime - v_beforeTime > 0) {
	m_linearVelocity = (Vector2f(v_afterPos - v_beforePos)).length()*100.0*3.6*2.0 / ((float)(v_afterTime - v_beforeTime));
      }

    } else { /* interpolation */
      /* do interpolation if it doesn't seems to be a teleportation or something like that */
      /* in fact, this is not nice ; the best way would be to test if there is a teleport event between the two frames */
      float v_distance = Vector2f(Vector2f(m_ghostBikeStates[m_ghostBikeStates.size()/2]->CenterP.x,
					   m_ghostBikeStates[m_ghostBikeStates.size()/2]->CenterP.y) -
				  Vector2f(m_ghostBikeStates[m_ghostBikeStates.size()/2-1]->CenterP.x,
					   m_ghostBikeStates[m_ghostBikeStates.size()/2-1]->CenterP.y)
				  ).length();
      bool v_can_interpolate =
	// interpolate only if the frame are near in the time
	m_ghostBikeStates[m_ghostBikeStates.size()/2]->GameTime - m_ghostBikeStates[m_ghostBikeStates.size()/2-1]->GameTime < INTERPOLATION_MAXIMUM_TIME &&
	// interpolate only if the state are near in the space
	v_distance < INTERPOLATION_MAXIMUM_SPACE;
      if(m_teleportationOccured) {
	v_can_interpolate = false; // in this case, we sure that interpolation occured; (but in case of a ghost, we don't read the events)
      }

      if(m_doInterpolation && v_can_interpolate) {
	if(m_ghostBikeStates[m_ghostBikeStates.size()/2]->GameTime - m_ghostBikeStates[m_ghostBikeStates.size()/2-1]->GameTime > 0) {
	  /* INTERPOLATED FRAME */
	  float v_interpolation_value;

	  v_interpolation_value = (i_time - m_ghostBikeStates[m_ghostBikeStates.size()/2-1]->GameTime) 
	    /((float)(m_ghostBikeStates[m_ghostBikeStates.size()/2]->GameTime - m_ghostBikeStates[m_ghostBikeStates.size()/2-1]->GameTime));

	  BikeState::interpolateGameState(m_ghostBikeStates, m_bikeState, v_interpolation_value);
	}
      }
    }
  }

  /* update change position */
  if(v_previousDir != m_bikeState->Dir) {
    m_changeDirPer = 0.0;
  }

  if(m_isActiv) {
    m_teleportationOccured = false;
    execReplayEvents(i_time, i_motogame);
  }
}

int FileGhost::getFinishTime() {
  return m_replay->getFinishTime();
}

std::string FileGhost::levelId() {
   return m_replay->getLevelId();
}

float FileGhost::getBikeLinearVel() {
  return m_linearVelocity;
}

float FileGhost::getTorsoVelocity() {
  return 0.0;
}

float FileGhost::getBikeEngineSpeed() {
  return 0.0; /* unable to know it */
}

double FileGhost::getAngle() {
  return 0.0; /* to do */
}

NetGhost::NetGhost(PhysicsSettings* i_physicsSettings,
		   bool i_engineSound,
		   Theme *i_theme, BikerTheme* i_bikerTheme,
		   const TColor& i_colorFilter,
		   const TColor& i_uglyColorFilter) 
: Ghost(i_physicsSettings, i_engineSound, i_theme, i_bikerTheme, i_colorFilter, i_uglyColorFilter) {
}

NetGhost::~NetGhost() {
}

void NetGhost::updateDiffToPlayer(std::vector<float> &i_lastToTakeEntities) {
}

std::string NetGhost::getVeryQuickDescription() const {
  return m_info;
}

std::string NetGhost::getQuickDescription() const {
  char c_tmp[256];
  snprintf(c_tmp, 256, GAMETEXT_WEBGHOSTOF, m_info.c_str()); 
  return std::string(c_tmp);
}

std::string NetGhost::getDescription() const {
  char c_tmp[256];
  snprintf(c_tmp, 256, GAMETEXT_WEBGHOSTOF, m_info.c_str()); 
  return std::string(c_tmp);
}

float NetGhost::getBikeEngineSpeed() {
  return 0.0; /* unable to know it */
}

float NetGhost::getBikeLinearVel() {
  return 0.0; /* to do */
}

float NetGhost::getTorsoVelocity() {
 return 0.0;
}

double NetGhost::getAngle() {
  return 0.0; /* to do */
}

ReplayBiker::ReplayBiker(std::string i_replayFile, PhysicsSettings* i_physicsSettings,
			 bool i_engineSound,
			 Theme *i_theme, BikerTheme* i_bikerTheme)
:FileGhost(i_replayFile, i_physicsSettings, true, i_engineSound, i_theme, i_bikerTheme,
       TColor(255, 255, 255, 0),
       TColor(GET_RED(i_bikerTheme->getUglyRiderColor()),
	      GET_GREEN(i_bikerTheme->getUglyRiderColor()),
	      GET_BLUE(i_bikerTheme->getUglyRiderColor()),
	      GET_ALPHA(i_bikerTheme->getUglyRiderColor()))) {
}

std::string ReplayBiker::getQuickDescription() const {
  char c_tmp[1024];
  
  snprintf(c_tmp, 1024,
	   GAMETEXT_REPLAYOF,
	   m_replay->getPlayerName().c_str()); 
  
  return std::string(c_tmp);
}

std::string ReplayBiker::getVeryQuickDescription() const {
  return m_replay->getPlayerName();
}



