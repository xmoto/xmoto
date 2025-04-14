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

#ifndef __MOTOGAMEEVENT_H__
#define __MOTOGAMEEVENT_H__

#include "Sound.h"
#include "xmscene/BasicSceneStructs.h"
#include "xmscene/Entity.h"
#include "xmscene/Zone.h"

/*===========================================================================
  Game event types
  ===========================================================================*/
/* never change the order of these values !
   ALWAYS ADD VALUES at the end : (otherwise, old replays will not work !)
*/
enum GameEventType {
  GAME_EVENT_PLAYERS_DIE =
    0, // to able able to read replay made with xmoto < 0.3.0
  GAME_EVENT_PLAYERS_ENTER_ZONE =
    1, // to able able to read replay made with xmoto < 0.3.0
  GAME_EVENT_PLAYERS_LEAVE_ZONE =
    2, // to able able to read replay made with xmoto < 0.3.0
  GAME_EVENT_PLAYERS_TOUCHE_ENTITY =
    3, // to able able to read replay made with xmoto < 0.3.0
  GAME_EVENT_ENTITY_DESTROYED = 4,
  GAME_EVENT_CLEARMESSAGES = 5,
  GAME_EVENT_PLACEINGAMEARROW = 6,
  GAME_EVENT_PLACESCREENARROW = 7,
  GAME_EVENT_HIDEARROW = 8,
  GAME_EVENT_MESSAGE = 9,
  GAME_EVENT_MOVEBLOCK = 10,
  GAME_EVENT_SETBLOCKPOS = 11,
  GAME_EVENT_SETGRAVITY = 12,
  GAME_EVENT_SETPLAYERSPOSITION =
    13, // to able able to read replay made with xmoto < 0.3.0
  GAME_EVENT_SETENTITYPOS = 14,
  GAME_EVENT_SETBLOCKCENTER = 15,
  GAME_EVENT_SETBLOCKROTATION = 16,
  GAME_EVENT_SETDYNAMICENTITYROTATION = 17,
  GAME_EVENT_SETDYNAMICENTITYTRANSLATION = 18,
  GAME_EVENT_SETDYNAMICENTITYNONE = 19,
  GAME_EVENT_CAMERAZOOM = 20,
  GAME_EVENT_CAMERAMOVE = 21,
  GAME_EVENT_SETDYNAMICBLOCKROTATION = 22,
  GAME_EVENT_SETDYNAMICBLOCKTRANSLATION = 23,
  GAME_EVENT_SETDYNAMICBLOCKNONE = 24,
  GAME_EVENT_PENALTY_TIME = 25,
  GAME_EVENT_PLAYER_DIES = 26,
  GAME_EVENT_PLAYER_ENTERS_ZONE = 27,
  GAME_EVENT_PLAYER_LEAVES_ZONE = 28,
  GAME_EVENT_PLAYER_TOUCHES_ENTITY = 29,
  GAME_EVENT_SETPLAYERPOSITION = 30,
  GAME_EVENT_SETDYNAMICBLOCKSELFROTATION = 31,
  GAME_EVENT_SETDYNAMICENTITYSELFROTATION = 32,
  GAME_EVENT_CAMERAROTATE = 33,
  GAME_EVENT_CAMERAADAPTTOGRAVITY = 34,
  GAME_EVENT_ADDFORCETOPLAYER = 35,
  GAME_EVENT_SETCAMERAROTATIONSPEED = 36,
  GAME_EVENT_PLAYSOUND = 37,
  GAME_EVENT_PLAYMUSIC = 38,
  GAME_EVENT_STOPMUSIC = 39,
  /*           = 40, */
  GAME_EVENT_SETPHYSICSBLOCKSELFROTATION = 41,
  GAME_EVENT_SETPHYSICSBLOCKTRANSLATION = 42,
  GAME_EVENT_CAMERASETPOS = 43,
  GAME_EVENT_PLAYER_WINS = 44
};

class SceneEvent;

struct RecordedGameEvent {
  SceneEvent *Event; /* Event itself */
  bool bPassed; /* Whether we have passed it */
};

#include "xmscene/Scene.h"

class SceneEvent {
public:
  SceneEvent(int p_eventTime);
  virtual ~SceneEvent();

  virtual void doAction(Scene *p_pScene) = 0;
  virtual void serialize(DBuffer &Buffer) = 0;
  virtual void unserialize(DBuffer &Buffer) = 0;
  virtual void revert(Scene *p_pScene);
  virtual GameEventType getType() = 0;

  virtual std::string toString() = 0;

  static SceneEvent *getUnserialized(DBuffer &Buffer,
                                     bool bDisplayInformation = false);
  int getEventTime();

protected:
  int m_eventTime;
};

class MGE_PlayersDie : public SceneEvent {
public:
  MGE_PlayersDie(int p_eventTime);
  MGE_PlayersDie(int p_eventTime, bool p_bKilledByWrecker);
  ~MGE_PlayersDie();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  bool m_bKilledByWrecker;
};

class MGE_PlayerDies : public SceneEvent {
public:
  MGE_PlayerDies(int p_eventTime);
  MGE_PlayerDies(int p_eventTime, bool p_bKilledByWrecker, int i_player);
  ~MGE_PlayerDies();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  bool m_bKilledByWrecker;
  int m_player;
};

class MGE_PlayerWins : public SceneEvent {
public:
  MGE_PlayerWins(int p_eventTime);
  MGE_PlayerWins(int p_eventTime, int i_player);
  ~MGE_PlayerWins();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  int m_player;
};

class MGE_PlayersEnterZone : public SceneEvent {
public:
  MGE_PlayersEnterZone(int p_eventTime);
  MGE_PlayersEnterZone(int p_eventTime, Zone *p_zone);
  ~MGE_PlayersEnterZone();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  Zone *m_zone;
};

class MGE_PlayerEntersZone : public SceneEvent {
public:
  MGE_PlayerEntersZone(int p_eventTime);
  MGE_PlayerEntersZone(int p_eventTime, Zone *p_zone, int i_player);
  ~MGE_PlayerEntersZone();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  Zone *m_zone;
  int m_player;
};

class MGE_PlayersLeaveZone : public SceneEvent {
public:
  MGE_PlayersLeaveZone(int p_eventTime);
  MGE_PlayersLeaveZone(int p_eventTime, Zone *p_zone);
  ~MGE_PlayersLeaveZone();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  Zone *m_zone;
};

class MGE_PlayerLeavesZone : public SceneEvent {
public:
  MGE_PlayerLeavesZone(int p_eventTime);
  MGE_PlayerLeavesZone(int p_eventTime, Zone *p_zone, int i_player);
  ~MGE_PlayerLeavesZone();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  Zone *m_zone;
  int m_player;
};

class MGE_PlayersToucheEntity : public SceneEvent {
public:
  MGE_PlayersToucheEntity(int p_eventTime);
  MGE_PlayersToucheEntity(int p_eventTime,
                          const std::string &p_entityID,
                          bool p_bTouchedWithHead);
  ~MGE_PlayersToucheEntity();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_entityID;
  bool m_bTouchedWithHead;
};

class MGE_PlayerTouchesEntity : public SceneEvent {
public:
  MGE_PlayerTouchesEntity(int p_eventTime);
  MGE_PlayerTouchesEntity(int p_eventTime,
                          const std::string &p_entityID,
                          bool p_bTouchedWithHead,
                          int i_player);
  ~MGE_PlayerTouchesEntity();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_entityID;
  bool m_bTouchedWithHead;
  int m_player;
};

class MGE_EntityDestroyed : public SceneEvent {
public:
  MGE_EntityDestroyed(int p_eventTime);
  MGE_EntityDestroyed(
    int p_eventTime,
    const std::string &i_entityId,
    EntitySpeciality i_entityType,
    Vector2f i_entityPosition,
    float i_entitySize,
    int i_takenByPlayer /* -1 if taken by an external event */);
  ~MGE_EntityDestroyed();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  void revert(Scene *p_pScene);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

  std::string EntityId();
  int takenByPlayer(); // -1 if taken by an external event

private:
  std::string m_entityId;
  EntitySpeciality m_entityType;
  Vector2f m_entityPosition;
  float m_entitySize;
  int m_takenByPlayer; // -1 if taken by an external event
};

class MGE_ClearMessages : public SceneEvent {
public:
  MGE_ClearMessages(int p_eventTime);
  ~MGE_ClearMessages();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
};

class MGE_PlaceInGameArrow : public SceneEvent {
public:
  MGE_PlaceInGameArrow(int p_eventTime);
  MGE_PlaceInGameArrow(int p_eventTime, float p_x, float p_y, float p_angle);
  ~MGE_PlaceInGameArrow();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  float m_x, m_y;
  float m_angle;
};

class MGE_PlaceScreenarrow : public SceneEvent {
public:
  MGE_PlaceScreenarrow(int p_eventTime);
  MGE_PlaceScreenarrow(int p_eventTime, float p_x, float p_y, float p_angle);
  ~MGE_PlaceScreenarrow();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  float m_x, m_y;
  float m_angle;
};

class MGE_HideArrow : public SceneEvent {
public:
  MGE_HideArrow(int p_eventTime);
  ~MGE_HideArrow();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
};

class MGE_Message : public SceneEvent {
public:
  MGE_Message(int p_eventTime);
  MGE_Message(int p_eventTime, const std::string &p_message);
  ~MGE_Message();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_message;
};

class MGE_MoveBlock : public SceneEvent {
public:
  MGE_MoveBlock(int p_eventTime);
  MGE_MoveBlock(int p_eventTime,
                const std::string &p_blockID,
                float p_x,
                float p_y);
  ~MGE_MoveBlock();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_blockID;
  float m_x, m_y;
};

class MGE_SetBlockPos : public SceneEvent {
public:
  MGE_SetBlockPos(int p_eventTime);
  MGE_SetBlockPos(int p_eventTime,
                  const std::string &p_blockID,
                  float p_x,
                  float p_y);
  ~MGE_SetBlockPos();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_blockID;
  float m_x, m_y;
};

class MGE_SetGravity : public SceneEvent {
public:
  MGE_SetGravity(int p_eventTime);
  MGE_SetGravity(int p_eventTime, float p_x, float p_y);
  ~MGE_SetGravity();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  float m_x, m_y;
};

class MGE_SetPlayersPosition : public SceneEvent {
public:
  MGE_SetPlayersPosition(int p_eventTime);
  MGE_SetPlayersPosition(int p_eventTime, float p_x, float p_y, bool p_bRight);
  ~MGE_SetPlayersPosition();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  float m_x, m_y;
  bool m_bRight;
};

class MGE_SetPlayerPosition : public SceneEvent {
public:
  MGE_SetPlayerPosition(int p_eventTime);
  MGE_SetPlayerPosition(int p_eventTime,
                        float p_x,
                        float p_y,
                        bool p_bRight,
                        int i_player);
  ~MGE_SetPlayerPosition();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  float m_x, m_y;
  bool m_bRight;
  int m_player;
};

class MGE_AddForceToPlayer : public SceneEvent {
public:
  MGE_AddForceToPlayer(int p_eventTime);
  MGE_AddForceToPlayer(int p_eventTime,
                       const Vector2f &i_force,
                       int i_startTime,
                       int i_endTime,
                       int i_player);
  ~MGE_AddForceToPlayer();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  Vector2f m_force;
  int m_startTime, m_endTime;
  int m_player;
};

class MGE_SetEntityPos : public SceneEvent {
public:
  MGE_SetEntityPos(int p_eventTime);
  MGE_SetEntityPos(int p_eventTime,
                   const std::string &p_entityID,
                   float p_x,
                   float p_y);
  ~MGE_SetEntityPos();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_entityID;
  float m_x, m_y;
};

class MGE_SetBlockCenter : public SceneEvent {
public:
  MGE_SetBlockCenter(int p_eventTime);
  MGE_SetBlockCenter(int p_eventTime,
                     const std::string &p_blockID,
                     float p_x,
                     float p_y);
  ~MGE_SetBlockCenter();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_blockID;
  float m_x, m_y;
};

class MGE_SetBlockRotation : public SceneEvent {
public:
  MGE_SetBlockRotation(int p_eventTime);
  MGE_SetBlockRotation(int p_eventTime,
                       const std::string &p_blockID,
                       float p_angle);
  ~MGE_SetBlockRotation();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_blockID;
  float m_angle;
};

class MGE_SetDynamicBlockSelfRotation : public SceneEvent {
public:
  MGE_SetDynamicBlockSelfRotation(int p_eventTime);
  MGE_SetDynamicBlockSelfRotation(int p_eventTime,
                                  const std::string &p_blockID,
                                  int p_period,
                                  int p_startTime,
                                  int p_endTime);
  ~MGE_SetDynamicBlockSelfRotation();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_blockID;
  float m_angle;
  int m_period;
  int m_startTime;
  int m_endTime;
};

class MGE_SetPhysicsBlockSelfRotation : public SceneEvent {
public:
  MGE_SetPhysicsBlockSelfRotation(int eventTime);
  MGE_SetPhysicsBlockSelfRotation(int eventTime,
                                  const std::string &blockID,
                                  int torque,
                                  int startTime,
                                  int endTime);
  ~MGE_SetPhysicsBlockSelfRotation();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_blockID;
  float m_angle;
  int m_torque;
  int m_startTime;
  int m_endTime;
};

class MGE_SetPhysicsBlockTranslation : public SceneEvent {
public:
  MGE_SetPhysicsBlockTranslation(int eventTime);
  MGE_SetPhysicsBlockTranslation(int eventTime,
                                 const std::string &blockID,
                                 float x,
                                 float y,
                                 int period,
                                 int startTime,
                                 int endTime);
  ~MGE_SetPhysicsBlockTranslation();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_blockID;
  float m_x, m_y;
  int m_period;
  int m_startTime;
  int m_endTime;
};

class MGE_SetDynamicEntityRotation : public SceneEvent {
public:
  MGE_SetDynamicEntityRotation(int p_eventTime);
  MGE_SetDynamicEntityRotation(int p_eventTime,
                               const std::string &p_entityID,
                               float p_fInitAngle,
                               float p_fRadius,
                               int p_period,
                               int p_startTime,
                               int p_endTime);
  ~MGE_SetDynamicEntityRotation();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_entityID;
  float m_fInitAngle;
  float m_fRadius;
  int m_period;
  int m_startTime;
  int m_endTime;
};

class MGE_SetDynamicEntitySelfRotation : public SceneEvent {
public:
  MGE_SetDynamicEntitySelfRotation(int p_eventTime);
  MGE_SetDynamicEntitySelfRotation(int p_eventTime,
                                   const std::string &p_entityID,
                                   int p_period,
                                   int p_startTime,
                                   int p_endTime);
  ~MGE_SetDynamicEntitySelfRotation();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_entityID;
  float m_angle;
  int m_period;
  int m_startTime;
  int m_endTime;
};

class MGE_SetDynamicEntityTranslation : public SceneEvent {
public:
  MGE_SetDynamicEntityTranslation(int p_eventTime);
  MGE_SetDynamicEntityTranslation(int p_eventTime,
                                  const std::string &p_entityID,
                                  float p_x,
                                  float p_y,
                                  int p_period,
                                  int p_startTime,
                                  int p_endTime);
  ~MGE_SetDynamicEntityTranslation();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_entityID;
  float m_x, m_y;
  int m_period;
  int m_startTime;
  int m_endTime;
};

class MGE_SetDynamicEntityNone : public SceneEvent {
public:
  MGE_SetDynamicEntityNone(int p_eventTime);
  MGE_SetDynamicEntityNone(int p_eventTime, const std::string &p_entityID);
  ~MGE_SetDynamicEntityNone();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_entityID;
};

class MGE_SetDynamicBlockRotation : public SceneEvent {
public:
  MGE_SetDynamicBlockRotation(int p_eventTime);
  MGE_SetDynamicBlockRotation(int p_eventTime,
                              const std::string &p_blockID,
                              float p_fInitAngle,
                              float p_fRadius,
                              int p_period,
                              int p_startTime,
                              int p_endTime);
  ~MGE_SetDynamicBlockRotation();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_blockID;
  float m_fInitAngle;
  float m_fRadius;
  int m_period;
  int m_startTime;
  int m_endTime;
};

// TODO::make it herit from MGE_SetDynamicBlockTranslation
class MGE_SetDynamicBlockTranslation : public SceneEvent {
public:
  MGE_SetDynamicBlockTranslation(int p_eventTime);
  MGE_SetDynamicBlockTranslation(int p_eventTime,
                                 const std::string &p_blockID,
                                 float p_x,
                                 float p_y,
                                 int p_period,
                                 int p_startTime,
                                 int p_endTime);
  ~MGE_SetDynamicBlockTranslation();

  void doAction(Scene *pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_blockID;
  float m_x, m_y;
  int m_period;
  int m_startTime;
  int m_endTime;
};

class MGE_SetDynamicBlockNone : public SceneEvent {
public:
  MGE_SetDynamicBlockNone(int p_eventTime);
  MGE_SetDynamicBlockNone(int p_eventTime, const std::string &p_blockID);
  ~MGE_SetDynamicBlockNone();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_blockID;
};

class MGE_CameraMove : public SceneEvent {
public:
  MGE_CameraMove(int p_eventTime);
  MGE_CameraMove(int p_eventTime, float p_moveX, float p_moveY);
  ~MGE_CameraMove();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  float m_moveX, m_moveY;
};

class MGE_CameraSetPos : public SceneEvent {
public:
  MGE_CameraSetPos(int p_eventTime);
  MGE_CameraSetPos(int p_eventTime, float p_X, float p_Y);
  ~MGE_CameraSetPos();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  float m_X, m_Y;
};

class MGE_CameraZoom : public SceneEvent {
public:
  MGE_CameraZoom(int p_eventTime);
  MGE_CameraZoom(int p_eventTime, float p_zoom);
  ~MGE_CameraZoom();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  float m_zoom;
};

class MGE_PenaltyTime : public SceneEvent {
public:
  MGE_PenaltyTime(int p_eventTime);
  MGE_PenaltyTime(int p_eventTime, int p_penaltyTime);
  ~MGE_PenaltyTime();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  int m_penaltyTime;
};

class MGE_CameraRotate : public SceneEvent {
public:
  MGE_CameraRotate(int p_eventTime);
  MGE_CameraRotate(int p_eventTime, float p_angle);
  ~MGE_CameraRotate();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  float m_angle;
};

class MGE_CameraAdaptToGravity : public SceneEvent {
public:
  MGE_CameraAdaptToGravity(int p_eventTime);
  ~MGE_CameraAdaptToGravity();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
};

class MGE_SetCameraRotationSpeed : public SceneEvent {
public:
  MGE_SetCameraRotationSpeed(int p_eventTime);
  MGE_SetCameraRotationSpeed(int p_eventTime, float p_speed);
  ~MGE_SetCameraRotationSpeed();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  float m_speed;
};

class MGE_PlaySound : public SceneEvent {
public:
  MGE_PlaySound(int p_eventTime);
  MGE_PlaySound(int p_eventTime,
                const std::string &p_soundName,
                float p_volume = DEFAULT_SAMPLE_VOLUME);
  ~MGE_PlaySound();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_soundName;
  float m_volume;
};

class MGE_PlayMusic : public SceneEvent {
public:
  MGE_PlayMusic(int p_eventTime);
  MGE_PlayMusic(int p_eventTime, const std::string &p_musicName);
  ~MGE_PlayMusic();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

private:
  std::string m_musicName;
};

class MGE_StopMusic : public SceneEvent {
public:
  MGE_StopMusic(int p_eventTime);
  ~MGE_StopMusic();

  void doAction(Scene *p_pScene);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();
};
#endif /* __MOTOGAMEEVENT_H__ */
