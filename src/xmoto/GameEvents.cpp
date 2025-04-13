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

#include "GameEvents.h"
#include "Game.h"
#include "ScriptDynamicObjects.h"
#include "Sound.h"
#include "common/DBuffer.h"
#include "helpers/Log.h"
#include "helpers/SwapEndian.h"
#include "xmscene/Camera.h"
#include "xmscene/Level.h"
#include <sstream>
#include <string>

SceneEvent::SceneEvent(int p_eventTime) {
  m_eventTime = p_eventTime;
}

SceneEvent::~SceneEvent() {}

void SceneEvent::serialize(DBuffer &Buffer) {
  Buffer << GameApp::timeToFloat(m_eventTime);
  Buffer << (int)(this->getType());
}

SceneEvent *SceneEvent::getUnserialized(DBuffer &Buffer,
                                        bool bDisplayInformation) {
  SceneEvent *v_event;
  float v_fEventTime;
  int v_eventTime;
  GameEventType v_eventType;
  int i_tmp;

  /**** to be able to debug the replays files */
  // int   EType;
  // float ETime;
  // int   SNameL;
  //
  // Buffer >> ETime;
  // Buffer >> EType;
  // Buffer >> SNameL;
  //
  // printf("%i\n", EType);
  //
  // int n = 256;
  // char c[n];
  // Buffer.readBuf(c, n);
  // c[n-1] = '\0';
  // for(unsigned int i=0; i<n-1; i++) {
  //  if(c[i] == '\0') {
  //	c[i] = '0';
  //  } else {
  //	if( ! ( (c[i] >= 'a' && c[i] <= 'z') || (c[i] >= 'A' && c[i] <= 'Z') ) )
  //{
  //	  c[i] = '@';
  //	}
  //  }
  //}
  // printf("+%s+\n", c);
  /*****/

  Buffer >> v_fEventTime;
  v_eventTime = GameApp::floatToTime(v_fEventTime);
  Buffer >> i_tmp;
  v_eventType = (GameEventType)i_tmp;

  if (MGE_PlayersDie::SgetType() == v_eventType) {
    v_event = new MGE_PlayersDie(v_eventTime);
  } else if (MGE_PlayersEnterZone::SgetType() == v_eventType) {
    v_event = new MGE_PlayersEnterZone(v_eventTime);
  } else if (MGE_PlayersLeaveZone::SgetType() == v_eventType) {
    v_event = new MGE_PlayersLeaveZone(v_eventTime);
  } else if (MGE_PlayersToucheEntity::SgetType() == v_eventType) {
    v_event = new MGE_PlayersToucheEntity(v_eventTime);
  } else if (MGE_EntityDestroyed::SgetType() == v_eventType) {
    v_event = new MGE_EntityDestroyed(v_eventTime);
  } else if (MGE_ClearMessages::SgetType() == v_eventType) {
    v_event = new MGE_ClearMessages(v_eventTime);
  } else if (MGE_PlaceInGameArrow::SgetType() == v_eventType) {
    v_event = new MGE_PlaceInGameArrow(v_eventTime);
  } else if (MGE_PlaceScreenarrow::SgetType() == v_eventType) {
    v_event = new MGE_PlaceScreenarrow(v_eventTime);
  } else if (MGE_HideArrow::SgetType() == v_eventType) {
    v_event = new MGE_HideArrow(v_eventTime);
  } else if (MGE_Message::SgetType() == v_eventType) {
    v_event = new MGE_Message(v_eventTime);
  } else if (MGE_MoveBlock::SgetType() == v_eventType) {
    v_event = new MGE_MoveBlock(v_eventTime);
  } else if (MGE_SetBlockPos::SgetType() == v_eventType) {
    v_event = new MGE_SetBlockPos(v_eventTime);
  } else if (MGE_SetGravity::SgetType() == v_eventType) {
    v_event = new MGE_SetGravity(v_eventTime);
  } else if (MGE_SetPlayersPosition::SgetType() == v_eventType) {
    v_event = new MGE_SetPlayersPosition(v_eventTime);
  } else if (MGE_SetEntityPos::SgetType() == v_eventType) {
    v_event = new MGE_SetEntityPos(v_eventTime);
  } else if (MGE_SetBlockCenter::SgetType() == v_eventType) {
    v_event = new MGE_SetBlockCenter(v_eventTime);
  } else if (MGE_SetBlockRotation::SgetType() == v_eventType) {
    v_event = new MGE_SetBlockRotation(v_eventTime);
  } else if (MGE_SetDynamicEntityRotation::SgetType() == v_eventType) {
    v_event = new MGE_SetDynamicEntityRotation(v_eventTime);
  } else if (MGE_SetDynamicEntityTranslation::SgetType() == v_eventType) {
    v_event = new MGE_SetDynamicEntityTranslation(v_eventTime);
  } else if (MGE_SetDynamicEntityNone::SgetType() == v_eventType) {
    v_event = new MGE_SetDynamicEntityNone(v_eventTime);
  } else if (MGE_SetDynamicBlockRotation::SgetType() == v_eventType) {
    v_event = new MGE_SetDynamicBlockRotation(v_eventTime);
  } else if (MGE_SetDynamicBlockTranslation::SgetType() == v_eventType) {
    v_event = new MGE_SetDynamicBlockTranslation(v_eventTime);
  } else if (MGE_SetDynamicBlockNone::SgetType() == v_eventType) {
    v_event = new MGE_SetDynamicBlockNone(v_eventTime);
  } else if (MGE_CameraMove::SgetType() == v_eventType) {
    v_event = new MGE_CameraMove(v_eventTime);
  } else if (MGE_CameraSetPos::SgetType() == v_eventType) {
    v_event = new MGE_CameraSetPos(v_eventTime);
  } else if (MGE_CameraZoom::SgetType() == v_eventType) {
    v_event = new MGE_CameraZoom(v_eventTime);
  } else if (MGE_PenaltyTime::SgetType() == v_eventType) {
    v_event = new MGE_PenaltyTime(v_eventTime);
  } else if (MGE_SetPlayerPosition::SgetType() == v_eventType) {
    v_event = new MGE_SetPlayerPosition(v_eventTime);
  } else if (MGE_PlayerDies::SgetType() == v_eventType) {
    v_event = new MGE_PlayerDies(v_eventTime);
  } else if (MGE_PlayerEntersZone::SgetType() == v_eventType) {
    v_event = new MGE_PlayerEntersZone(v_eventTime);
  } else if (MGE_PlayerLeavesZone::SgetType() == v_eventType) {
    v_event = new MGE_PlayerLeavesZone(v_eventTime);
  } else if (MGE_PlayerTouchesEntity::SgetType() == v_eventType) {
    v_event = new MGE_PlayerTouchesEntity(v_eventTime);
  } else if (MGE_SetDynamicBlockSelfRotation::SgetType() == v_eventType) {
    v_event = new MGE_SetDynamicBlockSelfRotation(v_eventTime);
  } else if (MGE_SetPhysicsBlockSelfRotation::SgetType() == v_eventType) {
    v_event = new MGE_SetPhysicsBlockSelfRotation(v_eventTime);
  } else if (MGE_SetPhysicsBlockTranslation::SgetType() == v_eventType) {
    v_event = new MGE_SetPhysicsBlockTranslation(v_eventTime);
  } else if (MGE_SetDynamicEntitySelfRotation::SgetType() == v_eventType) {
    v_event = new MGE_SetDynamicEntitySelfRotation(v_eventTime);
  } else if (MGE_CameraRotate::SgetType() == v_eventType) {
    v_event = new MGE_CameraRotate(v_eventTime);
  } else if (MGE_CameraAdaptToGravity::SgetType() == v_eventType) {
    v_event = new MGE_CameraAdaptToGravity(v_eventTime);
  } else if (MGE_AddForceToPlayer::SgetType() == v_eventType) {
    v_event = new MGE_AddForceToPlayer(v_eventTime);
  } else if (MGE_SetCameraRotationSpeed::SgetType() == v_eventType) {
    v_event = new MGE_SetCameraRotationSpeed(v_eventTime);
  } else if (MGE_PlaySound::SgetType() == v_eventType) {
    v_event = new MGE_PlaySound(v_eventTime);
  } else if (MGE_PlayMusic::SgetType() == v_eventType) {
    v_event = new MGE_PlayMusic(v_eventTime);
  } else if (MGE_StopMusic::SgetType() == v_eventType) {
    v_event = new MGE_StopMusic(v_eventTime);

  } else {
    std::ostringstream error_type;
    error_type << (int)v_eventType;
    throw Exception("Can't unserialize ! (event of type " + error_type.str() +
                    ")");
  }
  v_event->unserialize(Buffer);
  if (bDisplayInformation) {
    printf(
      "   %6.2f %-27s\n", v_eventTime / 100.0, v_event->toString().c_str());
  }

  return v_event;
}

void SceneEvent::revert(Scene *p_pScene) {}

int SceneEvent::getEventTime() {
  return m_eventTime;
}

//////////////////////////////
MGE_PlayersDie::MGE_PlayersDie(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_bKilledByWrecker = false;
}

MGE_PlayersDie::MGE_PlayersDie(int p_eventTime, bool p_bKilledByWrecker)
  : SceneEvent(p_eventTime) {
  m_bKilledByWrecker = p_bKilledByWrecker;
}

MGE_PlayersDie::~MGE_PlayersDie() {}

void MGE_PlayersDie::doAction(Scene *p_pScene) {
  for (unsigned int i = 0; i < p_pScene->Players().size(); i++) {
    p_pScene->killPlayer(i);
  }
}

void MGE_PlayersDie::serialize(DBuffer &Buffer) {}

void MGE_PlayersDie::unserialize(DBuffer &Buffer) {}

GameEventType MGE_PlayersDie::SgetType() {
  return GAME_EVENT_PLAYERS_DIE;
}

GameEventType MGE_PlayersDie::getType() {
  return SgetType();
}

std::string MGE_PlayersDie::toString() {
  return "Players die";
}

//////////////////////////////
MGE_PlayerDies::MGE_PlayerDies(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_bKilledByWrecker = false;
  m_player = 0;
}

MGE_PlayerDies::MGE_PlayerDies(int p_eventTime,
                               bool p_bKilledByWrecker,
                               int i_player)
  : SceneEvent(p_eventTime) {
  m_bKilledByWrecker = p_bKilledByWrecker;
  m_player = i_player;
}

MGE_PlayerDies::~MGE_PlayerDies() {}

void MGE_PlayerDies::doAction(Scene *p_pScene) {
  if (((int)p_pScene->Players().size()) >
      m_player) { // action are from external data (replays, network, so
    // basically, not sure)
    p_pScene->killPlayer(m_player);
  }
}

void MGE_PlayerDies::serialize(DBuffer &Buffer) {}

void MGE_PlayerDies::unserialize(DBuffer &Buffer) {}

GameEventType MGE_PlayerDies::SgetType() {
  return GAME_EVENT_PLAYER_DIES;
}

GameEventType MGE_PlayerDies::getType() {
  return SgetType();
}

std::string MGE_PlayerDies::toString() {
  std::ostringstream v_txt_player;
  v_txt_player << m_player;
  return "Player " + v_txt_player.str() + " dies";
}

//////////////////////////////
MGE_PlayerWins::MGE_PlayerWins(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_player = 0;
}

MGE_PlayerWins::MGE_PlayerWins(int p_eventTime, int i_player)
  : SceneEvent(p_eventTime) {
  m_player = i_player;
}

MGE_PlayerWins::~MGE_PlayerWins() {}

void MGE_PlayerWins::doAction(Scene *p_pScene) {
  if (((int)p_pScene->Players().size()) >
      m_player) { // action are from external data (replays, network, so
    // basically, not sure)
    p_pScene->makePlayerWin(m_player);
  }
}

void MGE_PlayerWins::serialize(DBuffer &Buffer) {}

void MGE_PlayerWins::unserialize(DBuffer &Buffer) {}

GameEventType MGE_PlayerWins::SgetType() {
  return GAME_EVENT_PLAYER_WINS;
}

GameEventType MGE_PlayerWins::getType() {
  return SgetType();
}

std::string MGE_PlayerWins::toString() {
  std::ostringstream v_txt_player;
  v_txt_player << m_player;
  return "Player " + v_txt_player.str() + " wins";
}

//////////////////////////////
MGE_PlayersEnterZone::MGE_PlayersEnterZone(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_zone = NULL;
}

MGE_PlayersEnterZone::MGE_PlayersEnterZone(int p_eventTime, Zone *p_zone)
  : SceneEvent(p_eventTime) {
  m_zone = p_zone;
}

MGE_PlayersEnterZone::~MGE_PlayersEnterZone() {}

void MGE_PlayersEnterZone::doAction(Scene *p_pScene) {
  for (unsigned int i = 0; i < p_pScene->Players().size(); i++) {
    p_pScene->playerEntersZone(i, m_zone);
  }
}

void MGE_PlayersEnterZone::serialize(DBuffer &Buffer) {}

void MGE_PlayersEnterZone::unserialize(DBuffer &Buffer) {}

GameEventType MGE_PlayersEnterZone::SgetType() {
  return GAME_EVENT_PLAYERS_ENTER_ZONE;
}

GameEventType MGE_PlayersEnterZone::getType() {
  return SgetType();
}

std::string MGE_PlayersEnterZone::toString() {
  return "Players enter on zone " + m_zone->Id();
}

//////////////////////////////
MGE_PlayerEntersZone::MGE_PlayerEntersZone(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_zone = NULL;
  m_player = 0;
}

MGE_PlayerEntersZone::MGE_PlayerEntersZone(int p_eventTime,
                                           Zone *p_zone,
                                           int i_player)
  : SceneEvent(p_eventTime) {
  m_zone = p_zone;
  m_player = i_player;
}

MGE_PlayerEntersZone::~MGE_PlayerEntersZone() {}

void MGE_PlayerEntersZone::doAction(Scene *p_pScene) {
  p_pScene->playerEntersZone(m_player, m_zone);
}

void MGE_PlayerEntersZone::serialize(DBuffer &Buffer) {}

void MGE_PlayerEntersZone::unserialize(DBuffer &Buffer) {}

GameEventType MGE_PlayerEntersZone::SgetType() {
  return GAME_EVENT_PLAYER_ENTERS_ZONE;
}

GameEventType MGE_PlayerEntersZone::getType() {
  return SgetType();
}

std::string MGE_PlayerEntersZone::toString() {
  std::ostringstream v_txt_player;
  v_txt_player << m_player;
  return "Player " + v_txt_player.str() + " enter on zone " + m_zone->Id();
}

//////////////////////////////
MGE_PlayersLeaveZone::MGE_PlayersLeaveZone(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_zone = NULL;
}

MGE_PlayersLeaveZone::MGE_PlayersLeaveZone(int p_eventTime, Zone *p_zone)
  : SceneEvent(p_eventTime) {
  m_zone = p_zone;
}

MGE_PlayersLeaveZone::~MGE_PlayersLeaveZone() {}

void MGE_PlayersLeaveZone::doAction(Scene *p_pScene) {
  for (unsigned int i = 0; i < p_pScene->Players().size(); i++) {
    p_pScene->playerLeavesZone(i, m_zone);
  }
}

void MGE_PlayersLeaveZone::serialize(DBuffer &Buffer) {}

void MGE_PlayersLeaveZone::unserialize(DBuffer &Buffer) {}

GameEventType MGE_PlayersLeaveZone::SgetType() {
  return GAME_EVENT_PLAYERS_LEAVE_ZONE;
}

GameEventType MGE_PlayersLeaveZone::getType() {
  return SgetType();
}

std::string MGE_PlayersLeaveZone::toString() {
  return "Players leave the zone " + m_zone->Id();
}

//////////////////////////////
MGE_PlayerLeavesZone::MGE_PlayerLeavesZone(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_zone = NULL;
  m_player = 0;
}

MGE_PlayerLeavesZone::MGE_PlayerLeavesZone(int p_eventTime,
                                           Zone *p_zone,
                                           int i_player)
  : SceneEvent(p_eventTime) {
  m_zone = p_zone;
  m_player = i_player;
}

MGE_PlayerLeavesZone::~MGE_PlayerLeavesZone() {}

void MGE_PlayerLeavesZone::doAction(Scene *p_pScene) {
  if (((int)p_pScene->Players().size()) >
      m_player) { // action are from external data (replays, network, so
    // basically, not sure)
    p_pScene->playerLeavesZone(m_player, m_zone);
  }
}

void MGE_PlayerLeavesZone::serialize(DBuffer &Buffer) {}

void MGE_PlayerLeavesZone::unserialize(DBuffer &Buffer) {}

GameEventType MGE_PlayerLeavesZone::SgetType() {
  return GAME_EVENT_PLAYER_LEAVES_ZONE;
}

GameEventType MGE_PlayerLeavesZone::getType() {
  return SgetType();
}

std::string MGE_PlayerLeavesZone::toString() {
  std::ostringstream v_txt_player;
  v_txt_player << m_player;
  return "Player " + v_txt_player.str() + " leaves the zone " + m_zone->Id();
}

//////////////////////////////
MGE_PlayersToucheEntity::MGE_PlayersToucheEntity(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_entityID = "";
  m_bTouchedWithHead = false;
}

MGE_PlayersToucheEntity::MGE_PlayersToucheEntity(int p_eventTime,
                                                 const std::string &p_entityID,
                                                 bool p_bTouchedWithHead)
  : SceneEvent(p_eventTime) {
  m_entityID = p_entityID;
  m_bTouchedWithHead = p_bTouchedWithHead;
}

MGE_PlayersToucheEntity::~MGE_PlayersToucheEntity() {}

void MGE_PlayersToucheEntity::doAction(Scene *p_pScene) {
  for (unsigned int i = 0; i < p_pScene->Players().size(); i++) {
    p_pScene->playerTouchesEntity(i, m_entityID, m_bTouchedWithHead);
  }
}

void MGE_PlayersToucheEntity::serialize(DBuffer &Buffer) {}

void MGE_PlayersToucheEntity::unserialize(DBuffer &Buffer) {}

GameEventType MGE_PlayersToucheEntity::SgetType() {
  return GAME_EVENT_PLAYERS_TOUCHE_ENTITY;
}

GameEventType MGE_PlayersToucheEntity::getType() {
  return SgetType();
}

std::string MGE_PlayersToucheEntity::toString() {
  return "Players touche entity " + m_entityID;
}

//////////////////////////////
MGE_PlayerTouchesEntity::MGE_PlayerTouchesEntity(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_entityID = "";
  m_bTouchedWithHead = false;
  m_player = 0;
}

MGE_PlayerTouchesEntity::MGE_PlayerTouchesEntity(int p_eventTime,
                                                 const std::string &p_entityID,
                                                 bool p_bTouchedWithHead,
                                                 int i_player)
  : SceneEvent(p_eventTime) {
  m_entityID = p_entityID;
  m_bTouchedWithHead = p_bTouchedWithHead;
  m_player = i_player;
}

MGE_PlayerTouchesEntity::~MGE_PlayerTouchesEntity() {}

void MGE_PlayerTouchesEntity::doAction(Scene *p_pScene) {
  if (((int)p_pScene->Players().size()) >
      m_player) { // action are from external data (replays, network, so
    // basically, not sure)
    p_pScene->playerTouchesEntity(m_player, m_entityID, m_bTouchedWithHead);
  }
}

void MGE_PlayerTouchesEntity::serialize(DBuffer &Buffer) {}

void MGE_PlayerTouchesEntity::unserialize(DBuffer &Buffer) {}

GameEventType MGE_PlayerTouchesEntity::SgetType() {
  return GAME_EVENT_PLAYER_TOUCHES_ENTITY;
}

GameEventType MGE_PlayerTouchesEntity::getType() {
  return SgetType();
}

std::string MGE_PlayerTouchesEntity::toString() {
  std::ostringstream v_txt_player;
  v_txt_player << m_player;

  return "Player " + v_txt_player.str() + " touches entity " + m_entityID;
}

//////////////////////////////
MGE_EntityDestroyed::MGE_EntityDestroyed(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_entitySize = 0.0;
  m_takenByPlayer = -1; // unknown
}

MGE_EntityDestroyed::MGE_EntityDestroyed(int p_eventTime,
                                         const std::string &i_entityId,
                                         EntitySpeciality i_entityType,
                                         Vector2f i_entityPosition,
                                         float i_entitySize,
                                         int i_takenByPlayer)
  : SceneEvent(p_eventTime) {
  m_entityId = i_entityId;
  m_entityType = i_entityType;
  m_entityPosition = i_entityPosition;
  m_entitySize = i_entitySize;
  m_takenByPlayer = i_takenByPlayer;
}

MGE_EntityDestroyed::~MGE_EntityDestroyed() {}

void MGE_EntityDestroyed::doAction(Scene *p_pScene) {
  p_pScene->entityDestroyed(m_entityId, m_eventTime, m_takenByPlayer);
}

void MGE_EntityDestroyed::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_entityId;
  Buffer << (int)(m_entityType);
  Buffer << m_entitySize;
  Buffer << m_entityPosition.x;
  Buffer << m_entityPosition.y;
}

void MGE_EntityDestroyed::unserialize(DBuffer &Buffer) {
  Vector2f v_position;
  std::string v_entityId;
  int i_tmp;

  Buffer >> m_entityId;

  /* no more used, kept just for compatibiliy */
  Buffer >> i_tmp;
  m_entityType = (EntitySpeciality)i_tmp;
  switch (m_entityType) {
    case ET_NONE:
    case ET_ISSTART:
    case ET_MAKEWIN:
    case ET_KILL:
    case ET_ISTOTAKE:
    case ET_PARTICLES_SOURCE:
      break;
    default:
      std::ostringstream error_type;
      error_type << (int)m_entityType;
      throw Exception("Invalid entity type (" + error_type.str() +
                      ")"); // with some compilator, an invalid value causes a
      // segfault (on my linux box)
  }
  Buffer >> m_entitySize;
  Buffer >> m_entityPosition.x;
  Buffer >> m_entityPosition.y;
}

void MGE_EntityDestroyed::revert(Scene *p_pScene) {
  p_pScene->getLevelSrc()->revertEntityDestroyed(m_entityId);
}

GameEventType MGE_EntityDestroyed::SgetType() {
  return GAME_EVENT_ENTITY_DESTROYED;
}

GameEventType MGE_EntityDestroyed::getType() {
  return SgetType();
}

std::string MGE_EntityDestroyed::toString() {
  return "Player destroys entity " + m_entityId;
}

std::string MGE_EntityDestroyed::EntityId() {
  return m_entityId;
}

int MGE_EntityDestroyed::takenByPlayer() {
  return m_takenByPlayer;
}

//////////////////////////////
MGE_ClearMessages::MGE_ClearMessages(int p_eventTime)
  : SceneEvent(p_eventTime) {}

MGE_ClearMessages::~MGE_ClearMessages() {}

void MGE_ClearMessages::doAction(Scene *p_pScene) {
  p_pScene->clearGameMessages();
}

void MGE_ClearMessages::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
}

void MGE_ClearMessages::unserialize(DBuffer &Buffer) {}

GameEventType MGE_ClearMessages::SgetType() {
  return GAME_EVENT_CLEARMESSAGES;
}

GameEventType MGE_ClearMessages::getType() {
  return SgetType();
}

std::string MGE_ClearMessages::toString() {
  return "Messages are cleared";
}

//////////////////////////////
MGE_PlaceInGameArrow::MGE_PlaceInGameArrow(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_x = 0.0;
  m_y = 0.0;
  m_angle = 0.0;
}

MGE_PlaceInGameArrow::MGE_PlaceInGameArrow(int p_eventTime,
                                           float p_x,
                                           float p_y,
                                           float p_angle)
  : SceneEvent(p_eventTime) {
  m_x = p_x;
  m_y = p_y;
  m_angle = p_angle;
}

MGE_PlaceInGameArrow::~MGE_PlaceInGameArrow() {}

void MGE_PlaceInGameArrow::doAction(Scene *p_pScene) {
  p_pScene->PlaceInGameArrow(m_x, m_y, m_angle);
}

void MGE_PlaceInGameArrow::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_x;
  Buffer << m_y;
  Buffer << m_angle;
}

void MGE_PlaceInGameArrow::unserialize(DBuffer &Buffer) {
  Buffer >> m_x;
  Buffer >> m_y;
  Buffer >> m_angle;
}

GameEventType MGE_PlaceInGameArrow::SgetType() {
  return GAME_EVENT_PLACEINGAMEARROW;
}

GameEventType MGE_PlaceInGameArrow::getType() {
  return SgetType();
}

std::string MGE_PlaceInGameArrow::toString() {
  return "Place in game arrow";
}

//////////////////////////////
MGE_PlaceScreenarrow::MGE_PlaceScreenarrow(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_x = 0.0;
  m_y = 0.0;
  m_angle = 0.0;
}

MGE_PlaceScreenarrow::MGE_PlaceScreenarrow(int p_eventTime,
                                           float p_x,
                                           float p_y,
                                           float p_angle)
  : SceneEvent(p_eventTime) {
  m_x = p_x;
  m_y = p_y;
  m_angle = p_angle;
}

MGE_PlaceScreenarrow::~MGE_PlaceScreenarrow() {}

void MGE_PlaceScreenarrow::doAction(Scene *p_pScene) {
  p_pScene->PlaceScreenArrow(m_x, m_y, m_angle);
}

void MGE_PlaceScreenarrow::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_x;
  Buffer << m_y;
  Buffer << m_angle;
}

void MGE_PlaceScreenarrow::unserialize(DBuffer &Buffer) {
  Buffer >> m_x;
  Buffer >> m_y;
  Buffer >> m_angle;
}

GameEventType MGE_PlaceScreenarrow::SgetType() {
  return GAME_EVENT_PLACESCREENARROW;
}

GameEventType MGE_PlaceScreenarrow::getType() {
  return SgetType();
}

std::string MGE_PlaceScreenarrow::toString() {
  return "Place screen arrow";
}

//////////////////////////////
MGE_HideArrow::MGE_HideArrow(int p_eventTime)
  : SceneEvent(p_eventTime) {}

MGE_HideArrow::~MGE_HideArrow() {}

void MGE_HideArrow::doAction(Scene *p_pScene) {
  p_pScene->HideArrow();
}

void MGE_HideArrow::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
}

void MGE_HideArrow::unserialize(DBuffer &Buffer) {}

GameEventType MGE_HideArrow::SgetType() {
  return GAME_EVENT_HIDEARROW;
}

GameEventType MGE_HideArrow::getType() {
  return SgetType();
}

std::string MGE_HideArrow::toString() {
  return "Hide arrow";
}

//////////////////////////////
MGE_Message::MGE_Message(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_message = "";
}

MGE_Message::MGE_Message(int p_eventTime, const std::string &p_message)
  : SceneEvent(p_eventTime) {
  m_message = p_message;
}

MGE_Message::~MGE_Message() {}

void MGE_Message::doAction(Scene *p_pScene) {
  p_pScene->gameMessage(
    m_message, false, MOTOGAME_DEFAULT_GAME_MESSAGE_DURATION, scripted);
}

void MGE_Message::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_message;
}

void MGE_Message::unserialize(DBuffer &Buffer) {
  Buffer >> m_message;
}

GameEventType MGE_Message::SgetType() {
  return GAME_EVENT_MESSAGE;
}

GameEventType MGE_Message::getType() {
  return SgetType();
}

std::string MGE_Message::toString() {
  return "Display message '" + m_message + "'";
}

//////////////////////////////
MGE_MoveBlock::MGE_MoveBlock(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_blockID = "";
  m_x = 0.0;
  m_y = 0.0;
}

MGE_MoveBlock::MGE_MoveBlock(int p_eventTime,
                             const std::string &p_blockID,
                             float p_x,
                             float p_y)
  : SceneEvent(p_eventTime) {
  m_blockID = p_blockID;
  m_x = p_x;
  m_y = p_y;
}

MGE_MoveBlock::~MGE_MoveBlock() {}

void MGE_MoveBlock::doAction(Scene *p_pScene) {
  p_pScene->MoveBlock(m_blockID, m_x, m_y);
}

void MGE_MoveBlock::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_blockID;
  Buffer << m_x;
  Buffer << m_y;
}

void MGE_MoveBlock::unserialize(DBuffer &Buffer) {
  Buffer >> m_blockID;
  Buffer >> m_x;
  Buffer >> m_y;
}

GameEventType MGE_MoveBlock::SgetType() {
  return GAME_EVENT_MOVEBLOCK;
}

GameEventType MGE_MoveBlock::getType() {
  return SgetType();
}

std::string MGE_MoveBlock::toString() {
  return "Block " + m_blockID + " is moved";
}

//////////////////////////////
MGE_SetBlockPos::MGE_SetBlockPos(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_blockID = "";
  m_x = 0.0;
  m_y = 0.0;
}

MGE_SetBlockPos::MGE_SetBlockPos(int p_eventTime,
                                 const std::string &p_blockID,
                                 float p_x,
                                 float p_y)
  : SceneEvent(p_eventTime) {
  m_blockID = p_blockID;
  m_x = p_x;
  m_y = p_y;
}

MGE_SetBlockPos::~MGE_SetBlockPos() {}

void MGE_SetBlockPos::doAction(Scene *p_pScene) {
  p_pScene->SetBlockPos(m_blockID, m_x, m_y);
}

void MGE_SetBlockPos::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_blockID;
  Buffer << m_x;
  Buffer << m_y;
}

void MGE_SetBlockPos::unserialize(DBuffer &Buffer) {
  Buffer >> m_blockID;
  Buffer >> m_x;
  Buffer >> m_y;
}

GameEventType MGE_SetBlockPos::SgetType() {
  return GAME_EVENT_SETBLOCKPOS;
}

GameEventType MGE_SetBlockPos::getType() {
  return SgetType();
}

std::string MGE_SetBlockPos::toString() {
  return "Block " + m_blockID + "'s position is changed";
}

//////////////////////////////
MGE_SetGravity::MGE_SetGravity(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_x = 0.0;
  m_y = 0.0;
}

MGE_SetGravity::MGE_SetGravity(int p_eventTime, float p_x, float p_y)
  : SceneEvent(p_eventTime) {
  m_x = p_x;
  m_y = p_y;
}

MGE_SetGravity::~MGE_SetGravity() {}

void MGE_SetGravity::doAction(Scene *p_pScene) {
  p_pScene->setGravity(m_x, m_y);
}

void MGE_SetGravity::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_x;
  Buffer << m_y;
}

void MGE_SetGravity::unserialize(DBuffer &Buffer) {
  Buffer >> m_x;
  Buffer >> m_y;
}

GameEventType MGE_SetGravity::SgetType() {
  return GAME_EVENT_SETGRAVITY;
}

GameEventType MGE_SetGravity::getType() {
  return SgetType();
}

std::string MGE_SetGravity::toString() {
  return "Gravity is changed";
}

//////////////////////////////
MGE_SetPlayersPosition::MGE_SetPlayersPosition(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_x = 0.0;
  m_y = 0.0;
  m_bRight = 0.0;
}

MGE_SetPlayersPosition::MGE_SetPlayersPosition(int p_eventTime,
                                               float p_x,
                                               float p_y,
                                               bool p_bRight)
  : SceneEvent(p_eventTime) {
  m_x = p_x;
  m_y = p_y;
  m_bRight = p_bRight;
}

MGE_SetPlayersPosition::~MGE_SetPlayersPosition() {}

void MGE_SetPlayersPosition::doAction(Scene *p_pScene) {
  for (unsigned int i = 0; i < p_pScene->Players().size(); i++) {
    p_pScene->setPlayerPosition(i, m_x, m_y, m_bRight);
  }
}

void MGE_SetPlayersPosition::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_x;
  Buffer << m_y;
  Buffer << m_bRight;
}

void MGE_SetPlayersPosition::unserialize(DBuffer &Buffer) {
  Buffer >> m_x;
  Buffer >> m_y;
  Buffer >> m_bRight;
}

GameEventType MGE_SetPlayersPosition::SgetType() {
  return GAME_EVENT_SETPLAYERSPOSITION;
}

GameEventType MGE_SetPlayersPosition::getType() {
  return SgetType();
}

std::string MGE_SetPlayersPosition::toString() {
  return "Teleportation of the players";
}

//////////////////////////////
MGE_SetPlayerPosition::MGE_SetPlayerPosition(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_x = 0.0;
  m_y = 0.0;
  m_bRight = 0.0;
  m_player = 0;
}

MGE_SetPlayerPosition::MGE_SetPlayerPosition(int p_eventTime,
                                             float p_x,
                                             float p_y,
                                             bool p_bRight,
                                             int i_player)
  : SceneEvent(p_eventTime) {
  m_x = p_x;
  m_y = p_y;
  m_bRight = p_bRight;
  m_player = i_player;
}

MGE_SetPlayerPosition::~MGE_SetPlayerPosition() {}

void MGE_SetPlayerPosition::doAction(Scene *p_pScene) {
  if (((int)p_pScene->Players().size()) >
      m_player) { // action are from external data (replays, network, so
    // basically, not sure)
    p_pScene->setPlayerPosition(m_player, m_x, m_y, m_bRight);
  }
}

void MGE_SetPlayerPosition::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_x;
  Buffer << m_y;
  Buffer << m_bRight;
  Buffer << m_player;
}

void MGE_SetPlayerPosition::unserialize(DBuffer &Buffer) {
  Buffer >> m_x;
  Buffer >> m_y;
  Buffer >> m_bRight;
  Buffer >> m_player;
}

GameEventType MGE_SetPlayerPosition::SgetType() {
  return GAME_EVENT_SETPLAYERPOSITION;
}

GameEventType MGE_SetPlayerPosition::getType() {
  return SgetType();
}

std::string MGE_SetPlayerPosition::toString() {
  return "Teleportation of the player " + std::to_string(m_player);
}

//////////////////////////////
MGE_SetEntityPos::MGE_SetEntityPos(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_entityID = "";
  m_x = 0.0;
  m_y = 0.0;
}

MGE_SetEntityPos::MGE_SetEntityPos(int p_eventTime,
                                   const std::string &p_entityID,
                                   float p_x,
                                   float p_y)
  : SceneEvent(p_eventTime) {
  m_entityID = p_entityID;
  m_x = p_x;
  m_y = p_y;
}

MGE_SetEntityPos::~MGE_SetEntityPos() {}

void MGE_SetEntityPos::doAction(Scene *p_pScene) {
  p_pScene->SetEntityPos(m_entityID, m_x, m_y);
}

void MGE_SetEntityPos::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_entityID;
  Buffer << m_x;
  Buffer << m_y;
}

void MGE_SetEntityPos::unserialize(DBuffer &Buffer) {
  Buffer >> m_entityID;
  Buffer >> m_x;
  Buffer >> m_y;
}

GameEventType MGE_SetEntityPos::SgetType() {
  return GAME_EVENT_SETENTITYPOS;
}

GameEventType MGE_SetEntityPos::getType() {
  return SgetType();
}

std::string MGE_SetEntityPos::toString() {
  return "Entity " + m_entityID + "'s position is changed";
}

//////////////////////////////
MGE_SetBlockCenter::MGE_SetBlockCenter(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_blockID = "";
  m_x = 0.0;
  m_y = 0.0;
}

MGE_SetBlockCenter::MGE_SetBlockCenter(int p_eventTime,
                                       const std::string &p_blockID,
                                       float p_x,
                                       float p_y)
  : SceneEvent(p_eventTime) {
  m_blockID = p_blockID;
  m_x = p_x;
  m_y = p_y;
}

MGE_SetBlockCenter::~MGE_SetBlockCenter() {}

void MGE_SetBlockCenter::doAction(Scene *p_pScene) {
  p_pScene->SetBlockCenter(m_blockID, m_x, m_y);
}

void MGE_SetBlockCenter::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_blockID;
  Buffer << m_x;
  Buffer << m_y;
}

void MGE_SetBlockCenter::unserialize(DBuffer &Buffer) {
  Buffer >> m_blockID;
  Buffer >> m_x;
  Buffer >> m_y;
}

GameEventType MGE_SetBlockCenter::SgetType() {
  return GAME_EVENT_SETBLOCKCENTER;
}

GameEventType MGE_SetBlockCenter::getType() {
  return SgetType();
}

std::string MGE_SetBlockCenter::toString() {
  return "Block " + m_blockID + "'s center is changed";
}

//////////////////////////////
MGE_SetBlockRotation::MGE_SetBlockRotation(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_blockID = "";
  m_angle = 0.0;
}

MGE_SetBlockRotation::MGE_SetBlockRotation(int p_eventTime,
                                           const std::string &p_blockID,
                                           float p_angle)
  : SceneEvent(p_eventTime) {
  m_blockID = p_blockID;
  m_angle = p_angle;
}

MGE_SetBlockRotation::~MGE_SetBlockRotation() {}

void MGE_SetBlockRotation::doAction(Scene *p_pScene) {
  p_pScene->SetBlockRotation(m_blockID, m_angle);
}

void MGE_SetBlockRotation::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_blockID;
  Buffer << m_angle;
}

void MGE_SetBlockRotation::unserialize(DBuffer &Buffer) {
  Buffer >> m_blockID;
  Buffer >> m_angle;
}

GameEventType MGE_SetBlockRotation::SgetType() {
  return GAME_EVENT_SETBLOCKROTATION;
}

GameEventType MGE_SetBlockRotation::getType() {
  return SgetType();
}

std::string MGE_SetBlockRotation::toString() {
  return "Block " + m_blockID + "'s rotation is changed";
}

//////////////////////////////
MGE_SetDynamicEntityRotation::MGE_SetDynamicEntityRotation(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_entityID = "";
  m_fInitAngle = 0.0;
  m_fRadius = 0.0;
  m_period = 0;
  m_startTime = 0;
  m_endTime = 0;
}

MGE_SetDynamicEntityRotation::MGE_SetDynamicEntityRotation(
  int p_eventTime,
  const std::string &p_entityID,
  float p_fInitAngle,
  float p_fRadius,
  int p_period,
  int p_startTime,
  int p_endTime)
  : SceneEvent(p_eventTime) {
  m_entityID = p_entityID;
  m_fInitAngle = p_fInitAngle;
  m_fRadius = p_fRadius;
  m_period = p_period;
  m_startTime = p_startTime;
  m_endTime = p_endTime;
}

MGE_SetDynamicEntityRotation::~MGE_SetDynamicEntityRotation() {}

void MGE_SetDynamicEntityRotation::doAction(Scene *p_pScene) {
  p_pScene->addDynamicObject(new SDynamicEntityRotation(
    m_entityID, m_fInitAngle, m_fRadius, m_period, m_startTime, m_endTime));
}

void MGE_SetDynamicEntityRotation::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_entityID;
  Buffer << m_fInitAngle;
  Buffer << m_fRadius;
  Buffer << GameApp::timeToFloat(m_period * 100);
  Buffer << m_startTime;
  Buffer << m_endTime;
}

void MGE_SetDynamicEntityRotation::unserialize(DBuffer &Buffer) {
  float v_fperiod;

  Buffer >> m_entityID;
  Buffer >> m_fInitAngle;
  Buffer >> m_fRadius;
  Buffer >> v_fperiod;
  Buffer >> m_startTime;
  Buffer >> m_endTime;

  m_period = GameApp::floatToTime(v_fperiod) / 100;
}

GameEventType MGE_SetDynamicEntityRotation::SgetType() {
  return GAME_EVENT_SETDYNAMICENTITYROTATION;
}

GameEventType MGE_SetDynamicEntityRotation::getType() {
  return SgetType();
}

std::string MGE_SetDynamicEntityRotation::toString() {
  return "Dynamic rotation is set for entity " + m_entityID;
}

//////////////////////////////
MGE_SetDynamicEntityTranslation::MGE_SetDynamicEntityTranslation(
  int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_entityID = "";
  m_x = 0.0;
  m_y = 0.0;
  m_period = 0;
  m_startTime = 0;
  m_endTime = 0;
}

MGE_SetDynamicEntityTranslation::MGE_SetDynamicEntityTranslation(
  int p_eventTime,
  const std::string &p_entityID,
  float p_x,
  float p_y,
  int p_period,
  int p_startTime,
  int p_endTime)
  : SceneEvent(p_eventTime) {
  m_entityID = p_entityID;
  m_x = p_x;
  m_y = p_y;
  m_period = p_period;
  m_startTime = p_startTime;
  m_endTime = p_endTime;
}

MGE_SetDynamicEntityTranslation::~MGE_SetDynamicEntityTranslation() {}

void MGE_SetDynamicEntityTranslation::doAction(Scene *p_pScene) {
  p_pScene->addDynamicObject(new SDynamicEntityTranslation(
    m_entityID, m_x, m_y, m_period, m_startTime, m_endTime));
}

void MGE_SetDynamicEntityTranslation::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_entityID;
  Buffer << m_x;
  Buffer << m_y;
  Buffer << GameApp::timeToFloat(m_period * 100);
  Buffer << m_startTime;
  Buffer << m_endTime;
}

void MGE_SetDynamicEntityTranslation::unserialize(DBuffer &Buffer) {
  float v_fperiod;

  Buffer >> m_entityID;
  Buffer >> m_x;
  Buffer >> m_y;
  Buffer >> v_fperiod;
  Buffer >> m_startTime;
  Buffer >> m_endTime;

  m_period = GameApp::floatToTime(v_fperiod) / 100;
}

GameEventType MGE_SetDynamicEntityTranslation::SgetType() {
  return GAME_EVENT_SETDYNAMICENTITYTRANSLATION;
}

GameEventType MGE_SetDynamicEntityTranslation::getType() {
  return SgetType();
}

std::string MGE_SetDynamicEntityTranslation::toString() {
  return "Dynamic translation is set for entity " + m_entityID;
}

//////////////////////////////
MGE_SetDynamicEntityNone::MGE_SetDynamicEntityNone(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_entityID = "";
}

MGE_SetDynamicEntityNone::MGE_SetDynamicEntityNone(
  int p_eventTime,
  const std::string &p_entityID)
  : SceneEvent(p_eventTime) {
  m_entityID = p_entityID;
}

MGE_SetDynamicEntityNone::~MGE_SetDynamicEntityNone() {}

void MGE_SetDynamicEntityNone::doAction(Scene *p_pScene) {
  p_pScene->removeSDynamicOfObject(m_entityID);
}

void MGE_SetDynamicEntityNone::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_entityID;
}

void MGE_SetDynamicEntityNone::unserialize(DBuffer &Buffer) {
  Buffer >> m_entityID;
}

GameEventType MGE_SetDynamicEntityNone::SgetType() {
  return GAME_EVENT_SETDYNAMICENTITYNONE;
}

GameEventType MGE_SetDynamicEntityNone::getType() {
  return SgetType();
}

std::string MGE_SetDynamicEntityNone::toString() {
  return "Remove dynamic for entity " + m_entityID;
}

//////////////////////////////
MGE_SetDynamicBlockRotation::MGE_SetDynamicBlockRotation(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_blockID = "";
  m_fInitAngle = 0.0;
  m_fRadius = 0.0;
  m_period = 0;
  m_startTime = 0;
  m_endTime = 0;
}

MGE_SetDynamicBlockRotation::MGE_SetDynamicBlockRotation(
  int p_eventTime,
  const std::string &p_blockID,
  float p_fInitAngle,
  float p_fRadius,
  int p_period,
  int p_startTime,
  int p_endTime)
  : SceneEvent(p_eventTime) {
  m_blockID = p_blockID;
  m_fInitAngle = p_fInitAngle;
  m_fRadius = p_fRadius;
  m_period = p_period;
  m_startTime = p_startTime;
  m_endTime = p_endTime;
}

MGE_SetDynamicBlockRotation::~MGE_SetDynamicBlockRotation() {}

void MGE_SetDynamicBlockRotation::doAction(Scene *p_pScene) {
  p_pScene->addDynamicObject(new SDynamicBlockRotation(
    m_blockID, m_fInitAngle, m_fRadius, m_period, m_startTime, m_endTime));
}

void MGE_SetDynamicBlockRotation::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_blockID;
  Buffer << m_fInitAngle;
  Buffer << m_fRadius;
  Buffer << GameApp::timeToFloat(m_period * 100);
  Buffer << m_startTime;
  Buffer << m_endTime;
}

void MGE_SetDynamicBlockRotation::unserialize(DBuffer &Buffer) {
  float v_fperiod;

  Buffer >> m_blockID;
  Buffer >> m_fInitAngle;
  Buffer >> m_fRadius;
  Buffer >> v_fperiod;
  Buffer >> m_startTime;
  Buffer >> m_endTime;

  m_period = GameApp::floatToTime(v_fperiod) / 100;
}

GameEventType MGE_SetDynamicBlockRotation::SgetType() {
  return GAME_EVENT_SETDYNAMICBLOCKROTATION;
}

GameEventType MGE_SetDynamicBlockRotation::getType() {
  return SgetType();
}

std::string MGE_SetDynamicBlockRotation::toString() {
  return "Dynamic rotation is set for block " + m_blockID;
}

//////////////////////////////
MGE_SetDynamicBlockTranslation::MGE_SetDynamicBlockTranslation(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_blockID = "";
  m_x = 0.0;
  m_y = 0.0;
  m_period = 0;
  m_startTime = 0;
  m_endTime = 0;
}

MGE_SetDynamicBlockTranslation::MGE_SetDynamicBlockTranslation(
  int p_eventTime,
  const std::string &p_blockID,
  float p_x,
  float p_y,
  int p_period,
  int p_startTime,
  int p_endTime)
  : SceneEvent(p_eventTime) {
  m_blockID = p_blockID;
  m_x = p_x;
  m_y = p_y;
  m_period = p_period;
  m_startTime = p_startTime;
  m_endTime = p_endTime;
}

MGE_SetDynamicBlockTranslation::~MGE_SetDynamicBlockTranslation() {}

void MGE_SetDynamicBlockTranslation::doAction(Scene *p_pScene) {
  p_pScene->addDynamicObject(new SDynamicBlockTranslation(
    m_blockID, m_x, m_y, m_period, m_startTime, m_endTime));
}

void MGE_SetDynamicBlockTranslation::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_blockID;
  Buffer << m_x;
  Buffer << m_y;
  Buffer << GameApp::timeToFloat(m_period * 100);
  Buffer << m_startTime;
  Buffer << m_endTime;
}

void MGE_SetDynamicBlockTranslation::unserialize(DBuffer &Buffer) {
  float v_fperiod;

  Buffer >> m_blockID;
  Buffer >> m_x;
  Buffer >> m_y;
  Buffer >> v_fperiod;
  Buffer >> m_startTime;
  Buffer >> m_endTime;

  m_period = GameApp::floatToTime(v_fperiod) / 100;
}

GameEventType MGE_SetDynamicBlockTranslation::SgetType() {
  return GAME_EVENT_SETDYNAMICBLOCKTRANSLATION;
}

GameEventType MGE_SetDynamicBlockTranslation::getType() {
  return SgetType();
}

std::string MGE_SetDynamicBlockTranslation::toString() {
  printf("%.2f %.2f %i %i %i\n", m_x, m_y, m_period, m_startTime, m_endTime);
  return "Dynamic translation is set for block " + m_blockID;
}

//////////////////////////////
MGE_SetDynamicBlockNone::MGE_SetDynamicBlockNone(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_blockID = "";
}

MGE_SetDynamicBlockNone::MGE_SetDynamicBlockNone(int p_eventTime,
                                                 const std::string &p_blockID)
  : SceneEvent(p_eventTime) {
  m_blockID = p_blockID;
}

MGE_SetDynamicBlockNone::~MGE_SetDynamicBlockNone() {}

void MGE_SetDynamicBlockNone::doAction(Scene *p_pScene) {
  p_pScene->removeSDynamicOfObject(m_blockID);
}

void MGE_SetDynamicBlockNone::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_blockID;
}

void MGE_SetDynamicBlockNone::unserialize(DBuffer &Buffer) {
  Buffer >> m_blockID;
}

GameEventType MGE_SetDynamicBlockNone::SgetType() {
  return GAME_EVENT_SETDYNAMICBLOCKNONE;
}

GameEventType MGE_SetDynamicBlockNone::getType() {
  return SgetType();
}

std::string MGE_SetDynamicBlockNone::toString() {
  return "Remove dynamic for block " + m_blockID;
}

//////////////////////////////
MGE_CameraMove::MGE_CameraMove(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_moveX = 0.0;
  m_moveY = 0.0;
}

MGE_CameraMove::MGE_CameraMove(int p_eventTime, float p_moveX, float p_moveY)
  : SceneEvent(p_eventTime) {
  m_moveX = p_moveX;
  m_moveY = p_moveY;
}

MGE_CameraMove::~MGE_CameraMove() {}

void MGE_CameraMove::doAction(Scene *p_pScene) {
  for (unsigned int i = 0; i < p_pScene->getNumberCameras(); i++) {
    p_pScene->setCurrentCamera(i);
    p_pScene->CameraMove(m_moveX, m_moveY);
  }
}

void MGE_CameraMove::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_moveX;
  Buffer << m_moveY;
}

void MGE_CameraMove::unserialize(DBuffer &Buffer) {
  Buffer >> m_moveX;
  Buffer >> m_moveY;
}

GameEventType MGE_CameraMove::SgetType() {
  return GAME_EVENT_CAMERAMOVE;
}

GameEventType MGE_CameraMove::getType() {
  return SgetType();
}

std::string MGE_CameraMove::toString() {
  return "Camera moves";
}

//////////////////////////////
MGE_CameraSetPos::MGE_CameraSetPos(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_X = 0.0;
  m_Y = 0.0;
}

MGE_CameraSetPos::MGE_CameraSetPos(int p_eventTime, float p_X, float p_Y)
  : SceneEvent(p_eventTime) {
  m_X = p_X;
  m_Y = p_Y;
}

MGE_CameraSetPos::~MGE_CameraSetPos() {}

void MGE_CameraSetPos::doAction(Scene *p_pScene) {
  for (unsigned int i = 0; i < p_pScene->getNumberCameras(); i++) {
    p_pScene->setCurrentCamera(i);
    p_pScene->CameraSetPos(m_X, m_Y);
  }
}

void MGE_CameraSetPos::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_X;
  Buffer << m_Y;
}

void MGE_CameraSetPos::unserialize(DBuffer &Buffer) {
  Buffer >> m_X;
  Buffer >> m_Y;
}

GameEventType MGE_CameraSetPos::SgetType() {
  return GAME_EVENT_CAMERASETPOS;
}

GameEventType MGE_CameraSetPos::getType() {
  return SgetType();
}

std::string MGE_CameraSetPos::toString() {
  return "Camera position is set";
}

//////////////////////////////
MGE_CameraZoom::MGE_CameraZoom(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_zoom = 0.0;
}

MGE_CameraZoom::MGE_CameraZoom(int p_eventTime, float p_zoom)
  : SceneEvent(p_eventTime) {
  m_zoom = p_zoom;
}

MGE_CameraZoom::~MGE_CameraZoom() {}

void MGE_CameraZoom::doAction(Scene *p_pScene) {
  for (unsigned int i = 0; i < p_pScene->getNumberCameras(); i++) {
    p_pScene->setCurrentCamera(i);
    p_pScene->CameraZoom(m_zoom);
  }
}

void MGE_CameraZoom::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_zoom;
}

void MGE_CameraZoom::unserialize(DBuffer &Buffer) {
  Buffer >> m_zoom;
}

GameEventType MGE_CameraZoom::SgetType() {
  return GAME_EVENT_CAMERAZOOM;
}

GameEventType MGE_CameraZoom::getType() {
  return SgetType();
}

std::string MGE_CameraZoom::toString() {
  return "Camera zoom is changed";
}

//////////////////////////////

MGE_PenaltyTime::MGE_PenaltyTime(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_penaltyTime = 0;
}

MGE_PenaltyTime::MGE_PenaltyTime(int p_eventTime, int p_penaltyTime)
  : SceneEvent(p_eventTime) {
  m_penaltyTime = p_penaltyTime;
}

MGE_PenaltyTime::~MGE_PenaltyTime() {}

void MGE_PenaltyTime::doAction(Scene *p_pScene) {
  p_pScene->addPenaltyTime(m_penaltyTime);
}

void MGE_PenaltyTime::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << GameApp::timeToFloat(m_penaltyTime);
}

void MGE_PenaltyTime::unserialize(DBuffer &Buffer) {
  float v_fpenaltyTime;
  Buffer >> v_fpenaltyTime;
  m_penaltyTime = GameApp::floatToTime(v_fpenaltyTime);
}

GameEventType MGE_PenaltyTime::SgetType() {
  return GAME_EVENT_PENALTY_TIME;
}

GameEventType MGE_PenaltyTime::getType() {
  return SgetType();
}

std::string MGE_PenaltyTime::toString() {
  return "Time penalty";
}

//////////////////////////////

MGE_SetDynamicBlockSelfRotation::MGE_SetDynamicBlockSelfRotation(
  int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_blockID = "";
  m_period = 0;
  m_startTime = 0;
  m_endTime = 0;
}

MGE_SetDynamicBlockSelfRotation::MGE_SetDynamicBlockSelfRotation(
  int p_eventTime,
  const std::string &p_blockID,
  int p_period,
  int p_startTime,
  int p_endTime)
  : SceneEvent(p_eventTime) {
  m_blockID = p_blockID;
  m_period = p_period;
  m_startTime = p_startTime;
  m_endTime = p_endTime;
}

MGE_SetDynamicBlockSelfRotation::~MGE_SetDynamicBlockSelfRotation() {}

void MGE_SetDynamicBlockSelfRotation::doAction(Scene *p_pScene) {
  p_pScene->addDynamicObject(
    new SDynamicBlockSelfRotation(m_blockID, m_period, m_startTime, m_endTime));
}

void MGE_SetDynamicBlockSelfRotation::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_blockID;
  Buffer << GameApp::timeToFloat(m_period * 100);
  Buffer << m_startTime;
  Buffer << m_endTime;
}

void MGE_SetDynamicBlockSelfRotation::unserialize(DBuffer &Buffer) {
  float v_fperiod;

  Buffer >> m_blockID;
  Buffer >> v_fperiod;
  Buffer >> m_startTime;
  Buffer >> m_endTime;

  m_period = GameApp::floatToTime(v_fperiod / 100);
}

GameEventType MGE_SetDynamicBlockSelfRotation::SgetType() {
  return GAME_EVENT_SETDYNAMICBLOCKSELFROTATION;
}

GameEventType MGE_SetDynamicBlockSelfRotation::getType() {
  return SgetType();
}

std::string MGE_SetDynamicBlockSelfRotation::toString() {
  return "Dynamic self rotation is set for block " + m_blockID;
}

//////////////////////////////

MGE_SetPhysicsBlockSelfRotation::MGE_SetPhysicsBlockSelfRotation(int eventTime)
  : SceneEvent(eventTime) {
  m_blockID = "";
  m_torque = 0;
  m_startTime = 0;
  m_endTime = 0;
}

MGE_SetPhysicsBlockSelfRotation::MGE_SetPhysicsBlockSelfRotation(
  int eventTime,
  const std::string &blockID,
  int torque,
  int startTime,
  int endTime)
  : SceneEvent(eventTime) {
  m_blockID = blockID;
  m_torque = torque;
  m_startTime = startTime;
  m_endTime = endTime;
}

MGE_SetPhysicsBlockSelfRotation::~MGE_SetPhysicsBlockSelfRotation() {}

void MGE_SetPhysicsBlockSelfRotation::doAction(Scene *p_pScene) {
  p_pScene->addDynamicObject(
    new SPhysicBlockSelfRotation(m_blockID, m_startTime, m_endTime, m_torque));
}

void MGE_SetPhysicsBlockSelfRotation::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_blockID;
  Buffer << m_torque;
  Buffer << m_startTime;
  Buffer << m_endTime;
}

void MGE_SetPhysicsBlockSelfRotation::unserialize(DBuffer &Buffer) {
  Buffer >> m_blockID;
  Buffer >> m_torque;
  Buffer >> m_startTime;
  Buffer >> m_endTime;
}

GameEventType MGE_SetPhysicsBlockSelfRotation::SgetType() {
  return GAME_EVENT_SETPHYSICSBLOCKSELFROTATION;
}

GameEventType MGE_SetPhysicsBlockSelfRotation::getType() {
  return SgetType();
}

std::string MGE_SetPhysicsBlockSelfRotation::toString() {
  return "Physic self rotation is set for block " + m_blockID;
}

MGE_SetPhysicsBlockTranslation::MGE_SetPhysicsBlockTranslation(int eventTime)
  : SceneEvent(eventTime) {
  m_blockID = "";
  m_x = m_y = 0.0f;
  m_period = 0;
  m_startTime = 0;
  m_endTime = 0;
}

MGE_SetPhysicsBlockTranslation::MGE_SetPhysicsBlockTranslation(
  int eventTime,
  const std::string &blockID,
  float x,
  float y,
  int period,
  int startTime,
  int endTime)
  : SceneEvent(eventTime) {
  m_blockID = blockID;
  m_x = x;
  m_y = y;
  m_period = period;
  m_startTime = startTime;
  m_endTime = endTime;
}

MGE_SetPhysicsBlockTranslation::~MGE_SetPhysicsBlockTranslation() {}

void MGE_SetPhysicsBlockTranslation::doAction(Scene *pScene) {
  pScene->addDynamicObject(new SPhysicBlockTranslation(
    m_blockID, m_x, m_y, m_period, m_startTime, m_endTime));
}

void MGE_SetPhysicsBlockTranslation::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_blockID;
  Buffer << m_x;
  Buffer << m_y;
  Buffer << GameApp::timeToFloat(m_period * 100);
  Buffer << m_startTime;
  Buffer << m_endTime;
}

void MGE_SetPhysicsBlockTranslation::unserialize(DBuffer &Buffer) {
  float v_fperiod;

  Buffer >> m_blockID;
  Buffer >> m_x;
  Buffer >> m_y;
  Buffer >> v_fperiod;
  Buffer >> m_startTime;
  Buffer >> m_endTime;

  m_period = GameApp::floatToTime(v_fperiod) / 100;
}

GameEventType MGE_SetPhysicsBlockTranslation::SgetType() {
  return GAME_EVENT_SETPHYSICSBLOCKTRANSLATION;
}

GameEventType MGE_SetPhysicsBlockTranslation::getType() {
  return SgetType();
}

std::string MGE_SetPhysicsBlockTranslation::toString() {
  return "Physic translation is set for block " + m_blockID;
}

//////////////////////////////

MGE_SetDynamicEntitySelfRotation::MGE_SetDynamicEntitySelfRotation(
  int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_entityID = "";
  m_period = 0;
  m_startTime = 0;
  m_endTime = 0;
}

MGE_SetDynamicEntitySelfRotation::MGE_SetDynamicEntitySelfRotation(
  int p_eventTime,
  const std::string &p_entityID,
  int p_period,
  int p_startTime,
  int p_endTime)
  : SceneEvent(p_eventTime) {
  m_entityID = p_entityID;
  m_period = p_period;
  m_startTime = p_startTime;
  m_endTime = p_endTime;
}

MGE_SetDynamicEntitySelfRotation::~MGE_SetDynamicEntitySelfRotation() {}

void MGE_SetDynamicEntitySelfRotation::doAction(Scene *p_pScene) {
  p_pScene->addDynamicObject(new SDynamicEntitySelfRotation(
    m_entityID, m_period, m_startTime, m_endTime));
}

void MGE_SetDynamicEntitySelfRotation::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_entityID;
  Buffer << GameApp::timeToFloat(m_period * 100);
  Buffer << m_startTime;
  Buffer << m_endTime;
}

void MGE_SetDynamicEntitySelfRotation::unserialize(DBuffer &Buffer) {
  float v_fperiod;

  Buffer >> m_entityID;
  Buffer >> v_fperiod;
  Buffer >> m_startTime;
  Buffer >> m_endTime;

  m_period = GameApp::floatToTime(v_fperiod / 100);
}

GameEventType MGE_SetDynamicEntitySelfRotation::SgetType() {
  return GAME_EVENT_SETDYNAMICENTITYSELFROTATION;
}

GameEventType MGE_SetDynamicEntitySelfRotation::getType() {
  return SgetType();
}

std::string MGE_SetDynamicEntitySelfRotation::toString() {
  return "Dynamic self rotation is set for entity " + m_entityID;
}

//////////////////////////////
MGE_CameraRotate::MGE_CameraRotate(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_angle = 0.0;
}

MGE_CameraRotate::MGE_CameraRotate(int p_eventTime, float p_angle)
  : SceneEvent(p_eventTime) {
  m_angle = p_angle;
}

MGE_CameraRotate::~MGE_CameraRotate() {}

void MGE_CameraRotate::doAction(Scene *p_pScene) {
  for (unsigned int i = 0; i < p_pScene->getNumberCameras(); i++) {
    p_pScene->setCurrentCamera(i);
    p_pScene->CameraRotate(m_angle);
  }
}

void MGE_CameraRotate::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_angle;
}

void MGE_CameraRotate::unserialize(DBuffer &Buffer) {
  Buffer >> m_angle;
}

GameEventType MGE_CameraRotate::SgetType() {
  return GAME_EVENT_CAMERAROTATE;
}

GameEventType MGE_CameraRotate::getType() {
  return SgetType();
}

std::string MGE_CameraRotate::toString() {
  return "Camera rotates";
}

//////////////////////////////
MGE_CameraAdaptToGravity::MGE_CameraAdaptToGravity(int p_eventTime)
  : SceneEvent(p_eventTime) {}

MGE_CameraAdaptToGravity::~MGE_CameraAdaptToGravity() {}

void MGE_CameraAdaptToGravity::doAction(Scene *p_pScene) {
  for (unsigned int i = 0; i < p_pScene->getNumberCameras(); i++) {
    p_pScene->setCurrentCamera(i);
    p_pScene->CameraAdaptToGravity();
  }
}

void MGE_CameraAdaptToGravity::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
}

void MGE_CameraAdaptToGravity::unserialize(DBuffer &Buffer) {}

GameEventType MGE_CameraAdaptToGravity::SgetType() {
  return GAME_EVENT_CAMERAADAPTTOGRAVITY;
}

GameEventType MGE_CameraAdaptToGravity::getType() {
  return SgetType();
}

std::string MGE_CameraAdaptToGravity::toString() {
  return "Camera is adapted to the gravity";
}

//////////////////////////////
MGE_AddForceToPlayer::MGE_AddForceToPlayer(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_force = Vector2f(0.0, 0.0);
  m_player = 0;
  m_startTime = m_endTime = 0;
}

MGE_AddForceToPlayer::MGE_AddForceToPlayer(int p_eventTime,
                                           const Vector2f &i_force,
                                           int i_startTime,
                                           int i_endTime,
                                           int i_player)
  : SceneEvent(p_eventTime) {
  m_force = i_force;
  m_startTime = i_startTime;
  m_endTime = i_endTime;
  m_player = i_player;
}

MGE_AddForceToPlayer::~MGE_AddForceToPlayer() {}

void MGE_AddForceToPlayer::doAction(Scene *p_pScene) {
  if (((int)p_pScene->Players().size()) >
      m_player) { // action are from external data (replays, network, so
    // basically, not sure)
    p_pScene->addForceToPlayer(m_player, m_force, m_startTime, m_endTime);
  }
}

void MGE_AddForceToPlayer::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_force.x;
  Buffer << m_force.y;
  Buffer << m_startTime;
  Buffer << m_endTime;
  Buffer << m_player;
}

void MGE_AddForceToPlayer::unserialize(DBuffer &Buffer) {
  Buffer >> m_force.x;
  Buffer >> m_force.y;
  Buffer >> m_startTime;
  Buffer >> m_endTime;
  Buffer >> m_player;
}

GameEventType MGE_AddForceToPlayer::SgetType() {
  return GAME_EVENT_ADDFORCETOPLAYER;
}

GameEventType MGE_AddForceToPlayer::getType() {
  return SgetType();
}

std::string MGE_AddForceToPlayer::toString() {
  return "Add force to the player " + std::to_string(m_player);
}

//////////////////////////////
MGE_SetCameraRotationSpeed::MGE_SetCameraRotationSpeed(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_speed = 0.0;
}

MGE_SetCameraRotationSpeed::MGE_SetCameraRotationSpeed(int p_eventTime,
                                                       float p_speed)
  : SceneEvent(p_eventTime) {
  m_speed = p_speed;
}

MGE_SetCameraRotationSpeed::~MGE_SetCameraRotationSpeed() {}

void MGE_SetCameraRotationSpeed::doAction(Scene *p_pScene) {
  for (unsigned int i = 0; i < p_pScene->getNumberCameras(); i++) {
    p_pScene->Cameras()[i]->setRotationSpeed(m_speed);
  }
}

void MGE_SetCameraRotationSpeed::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_speed;
}

void MGE_SetCameraRotationSpeed::unserialize(DBuffer &Buffer) {
  Buffer >> m_speed;
}

GameEventType MGE_SetCameraRotationSpeed::SgetType() {
  return GAME_EVENT_SETCAMERAROTATIONSPEED;
}

GameEventType MGE_SetCameraRotationSpeed::getType() {
  return SgetType();
}

std::string MGE_SetCameraRotationSpeed::toString() {
  return "Camera Rotate set to desired Speed";
}

///////////////////////////////////////
MGE_PlaySound::MGE_PlaySound(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_soundName = "";
}

MGE_PlaySound::MGE_PlaySound(int p_eventTime,
                             const std::string &p_name,
                             float p_volume)
  : SceneEvent(p_eventTime) {
  m_soundName = p_name;
  m_volume = p_volume;
}

MGE_PlaySound::~MGE_PlaySound() {}

void MGE_PlaySound::doAction(Scene *p_pScene) {
  if (XMSession::instance()->enableAudio() == false) {
    return;
  }

  try {
    Sound::playSampleByName(
      Theme::instance()->getSound(m_soundName)->FilePath(), m_volume);
  } catch (Exception &e) {
    LogWarning(
      "PlaySound(\"%s\") failed: %s", m_soundName.c_str(), e.getMsg().c_str());
  }
}

void MGE_PlaySound::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_soundName;
  Buffer << m_volume;
}

void MGE_PlaySound::unserialize(DBuffer &Buffer) {
  Buffer >> m_soundName;
  Buffer >> m_volume;
}

GameEventType MGE_PlaySound::SgetType() {
  return GAME_EVENT_PLAYSOUND;
}

GameEventType MGE_PlaySound::getType() {
  return SgetType();
}

std::string MGE_PlaySound::toString() {
  return "Audio played";
}

///////////////////////////////////////
MGE_PlayMusic::MGE_PlayMusic(int p_eventTime)
  : SceneEvent(p_eventTime) {
  m_musicName = "";
}

MGE_PlayMusic::MGE_PlayMusic(int p_eventTime, const std::string &p_name)
  : SceneEvent(p_eventTime) {
  m_musicName = p_name;
}

MGE_PlayMusic::~MGE_PlayMusic() {}

void MGE_PlayMusic::doAction(Scene *p_pScene) {
  try {
    GameApp::instance()->playGameMusic(m_musicName);
  } catch (Exception &e) {
    LogWarning(
      "PlayMusic(\"%s\") failed: %s", m_musicName.c_str(), e.getMsg().c_str());
  }
}

void MGE_PlayMusic::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
  Buffer << m_musicName;
}

void MGE_PlayMusic::unserialize(DBuffer &Buffer) {
  Buffer >> m_musicName;
}

GameEventType MGE_PlayMusic::SgetType() {
  return GAME_EVENT_PLAYMUSIC;
}

GameEventType MGE_PlayMusic::getType() {
  return SgetType();
}

std::string MGE_PlayMusic::toString() {
  return "Music played";
}

///////////////////////////////////////
MGE_StopMusic::MGE_StopMusic(int p_eventTime)
  : SceneEvent(p_eventTime) {}

MGE_StopMusic::~MGE_StopMusic() {}

void MGE_StopMusic::doAction(Scene *p_pScene) {
  try {
    GameApp::instance()->playGameMusic("");
  } catch (Exception &e) {
    LogWarning("StopMusic failed: %s", e.getMsg().c_str());
  }
}

void MGE_StopMusic::serialize(DBuffer &Buffer) {
  SceneEvent::serialize(Buffer);
}

void MGE_StopMusic::unserialize(DBuffer &Buffer) {}

GameEventType MGE_StopMusic::SgetType() {
  return GAME_EVENT_STOPMUSIC;
}

GameEventType MGE_StopMusic::getType() {
  return SgetType();
}

std::string MGE_StopMusic::toString() {
  return "Music stopped";
};
