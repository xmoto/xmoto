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

#include "xmscene/BasicSceneStructs.h"
#include "xmscene/Zone.h"
#include "xmscene/Entity.h"
#include "Sound.h"

  /*===========================================================================
    Game event types
    ===========================================================================*/
  /* never change the order of these values ! 
     ALWAYS ADD VALUES at the end : (otherwise, old replays will not work !)
  */
  enum GameEventType {
    GAME_EVENT_PLAYERS_DIE                  =  0, // to able able to read replay made with xmoto < 0.3.0
    GAME_EVENT_PLAYERS_ENTER_ZONE           =  1, // to able able to read replay made with xmoto < 0.3.0
    GAME_EVENT_PLAYERS_LEAVE_ZONE           =  2, // to able able to read replay made with xmoto < 0.3.0
    GAME_EVENT_PLAYERS_TOUCHE_ENTITY        =  3, // to able able to read replay made with xmoto < 0.3.0
    GAME_EVENT_ENTITY_DESTROYED             =  4,    
    GAME_EVENT_CLEARMESSAGES                =  5,
    GAME_EVENT_PLACEINGAMEARROW             =  6,
    GAME_EVENT_PLACESCREENARROW             =  7,
    GAME_EVENT_HIDEARROW                    =  8,
    GAME_EVENT_MESSAGE                      =  9,
    GAME_EVENT_MOVEBLOCK                    = 10,
    GAME_EVENT_SETBLOCKPOS                  = 11,
    GAME_EVENT_SETGRAVITY                   = 12,
    GAME_EVENT_SETPLAYERSPOSITION           = 13, // to able able to read replay made with xmoto < 0.3.0
    GAME_EVENT_SETENTITYPOS                 = 14,
    GAME_EVENT_SETBLOCKCENTER               = 15,
    GAME_EVENT_SETBLOCKROTATION             = 16, 
    GAME_EVENT_SETDYNAMICENTITYROTATION     = 17,
    GAME_EVENT_SETDYNAMICENTITYTRANSLATION  = 18,
    GAME_EVENT_SETDYNAMICENTITYNONE         = 19,
    GAME_EVENT_CAMERAZOOM                   = 20,
    GAME_EVENT_CAMERAMOVE                   = 21,
    GAME_EVENT_SETDYNAMICBLOCKROTATION      = 22,
    GAME_EVENT_SETDYNAMICBLOCKTRANSLATION   = 23,
    GAME_EVENT_SETDYNAMICBLOCKNONE          = 24,
    GAME_EVENT_PENALITY_TIME                = 25,
    GAME_EVENT_PLAYER_DIES                  = 26,
    GAME_EVENT_PLAYER_ENTERS_ZONE           = 27,
    GAME_EVENT_PLAYER_LEAVES_ZONE           = 28,
    GAME_EVENT_PLAYER_TOUCHES_ENTITY        = 29,
    GAME_EVENT_SETPLAYERPOSITION            = 30,
    GAME_EVENT_SETDYNAMICBLOCKSELFROTATION  = 31,
    GAME_EVENT_SETDYNAMICENTITYSELFROTATION = 32,
    GAME_EVENT_CAMERAROTATE                 = 33,
    GAME_EVENT_CAMERAADAPTTOGRAVITY         = 34,
    GAME_EVENT_ADDFORCETOPLAYER             = 35,
    GAME_EVENT_SETCAMERAROTATIONSPEED       = 36,
    GAME_EVENT_PLAYSOUND                    = 37,
    GAME_EVENT_PLAYMUSIC                    = 38,
    GAME_EVENT_STOPMUSIC                    = 39,
    GAME_EVENT_SETPHYSICSBLOCKPOS           = 40,
    GAME_EVENT_SETPHYSICSBLOCKSELFROTATION  = 41,
    GAME_EVENT_SETPHYSICSBLOCKTRANSLATION   = 42
  };

class MotoGameEvent;

struct RecordedGameEvent {
  MotoGameEvent *Event;     /* Event itself */
  bool bPassed;             /* Whether we have passed it */
};

// two for a scene:
// -the one for storing current play events.
// -the one for playing events in the main ghost
class MotoGameEventManager : public ISerializer {
public:
  MotoGameEventManager();
  virtual ~MotoGameEventManager();

  void storeFrame(Scene* pScene);

  void unserializeFrames(Scene* pScene);

private:
  std::vector<RecordedGameEvent*> m_replayEvents;
};


class MotoGameEvent {
  public:
  MotoGameEvent(int p_eventTime);
  virtual ~MotoGameEvent();

  virtual void doAction(MotoGame *p_pMotoGame)          = 0;
  virtual void serialize(DBuffer &Buffer)               = 0;
  virtual void unserialize(DBuffer &Buffer) = 0;
  virtual void revert(MotoGame *p_pMotoGame);
  virtual GameEventType getType()                       = 0;

  virtual std::string toString() = 0;

  static MotoGameEvent* getUnserialized(DBuffer &Buffer, bool bDisplayInformation = false);
  int getTime();

  protected:
  int m_eventTime;
};

class MGE_PlayersDie : public MotoGameEvent {
 public:
  MGE_PlayersDie(int p_eventTime);
  MGE_PlayersDie(int p_eventTime, bool p_bKilledByWrecker);
  ~MGE_PlayersDie();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  bool m_bKilledByWrecker;
};

class MGE_PlayerDies : public MotoGameEvent {
 public:
  MGE_PlayerDies(int p_eventTime);
  MGE_PlayerDies(int p_eventTime, bool p_bKilledByWrecker, int i_player);
  ~MGE_PlayerDies();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  bool m_bKilledByWrecker;
  int  m_player;
};

class MGE_PlayersEnterZone : public MotoGameEvent {
 public:
  MGE_PlayersEnterZone(int p_eventTime);
  MGE_PlayersEnterZone(int p_eventTime, Zone *p_zone);
  ~MGE_PlayersEnterZone();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  Zone *m_zone;
};

class MGE_PlayerEntersZone : public MotoGameEvent {
 public:
  MGE_PlayerEntersZone(int p_eventTime);
  MGE_PlayerEntersZone(int p_eventTime, Zone *p_zone, int i_player);
  ~MGE_PlayerEntersZone();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  Zone *m_zone;
  int   m_player;
};

class MGE_PlayersLeaveZone : public MotoGameEvent {
 public:
  MGE_PlayersLeaveZone(int p_eventTime);
  MGE_PlayersLeaveZone(int p_eventTime, Zone *p_zone);
  ~MGE_PlayersLeaveZone();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  Zone *m_zone;
};

class MGE_PlayerLeavesZone : public MotoGameEvent {
 public:
  MGE_PlayerLeavesZone(int p_eventTime);
  MGE_PlayerLeavesZone(int p_eventTime, Zone *p_zone, int i_player);
  ~MGE_PlayerLeavesZone();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  Zone *m_zone;
  int   m_player;
};

class MGE_PlayersToucheEntity : public MotoGameEvent {
 public:
  MGE_PlayersToucheEntity(int p_eventTime);
  MGE_PlayersToucheEntity(int p_eventTime, std::string p_entityID, bool p_bTouchedWithHead);
  ~MGE_PlayersToucheEntity();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
    std::string m_entityID;
    bool m_bTouchedWithHead;
};

class MGE_PlayerTouchesEntity : public MotoGameEvent {
 public:
  MGE_PlayerTouchesEntity(int p_eventTime);
  MGE_PlayerTouchesEntity(int p_eventTime, std::string p_entityID, bool p_bTouchedWithHead, int i_player);
  ~MGE_PlayerTouchesEntity();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
    std::string m_entityID;
    bool m_bTouchedWithHead;
    int  m_player;
};

class MGE_EntityDestroyed : public MotoGameEvent {
 public:
  MGE_EntityDestroyed(int p_eventTime);
  MGE_EntityDestroyed(int p_eventTime, std::string i_entityId, EntitySpeciality i_entityType, Vector2f i_entityPosition, float i_entitySize);
  ~MGE_EntityDestroyed();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  void revert(MotoGame *p_pMotoGame);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

  std::string EntityId();

 private:
  std::string m_entityId;
  EntitySpeciality m_entityType;
  Vector2f    m_entityPosition;
  float       m_entitySize;
};

class MGE_ClearMessages : public MotoGameEvent {
 public:
  MGE_ClearMessages(int p_eventTime);
  ~MGE_ClearMessages();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
};

class MGE_PlaceInGameArrow : public MotoGameEvent {
 public:
  MGE_PlaceInGameArrow(int p_eventTime);
  MGE_PlaceInGameArrow(int p_eventTime, float p_x, float p_y, float p_angle);
  ~MGE_PlaceInGameArrow();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  float m_x, m_y;
  float m_angle;
};

class MGE_PlaceScreenarrow : public MotoGameEvent {
 public:
  MGE_PlaceScreenarrow(int p_eventTime);
  MGE_PlaceScreenarrow(int p_eventTime, float p_x, float p_y, float p_angle);
  ~MGE_PlaceScreenarrow();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  float m_x, m_y;
  float m_angle;
};

class MGE_HideArrow : public MotoGameEvent {
 public:
  MGE_HideArrow(int p_eventTime);
  ~MGE_HideArrow();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
};

class MGE_Message : public MotoGameEvent {
 public:
  MGE_Message(int p_eventTime);
  MGE_Message(int p_eventTime, std::string p_message);
  ~MGE_Message();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_message;
};

class MGE_MoveBlock : public MotoGameEvent {
 public:
  MGE_MoveBlock(int p_eventTime);
  MGE_MoveBlock(int p_eventTime, std::string p_blockID, float p_x, float p_y);
  ~MGE_MoveBlock();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_blockID;
  float m_x, m_y;
};

class MGE_SetPhysicsBlockPos : public MotoGameEvent {
 public:
  MGE_SetPhysicsBlockPos(int p_eventTime);
  MGE_SetPhysicsBlockPos(int p_eventTime, std::string p_blockID, float p_x, float p_y);
  ~MGE_SetPhysicsBlockPos();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_blockID;
  float m_x, m_y;
};

class MGE_SetBlockPos : public MotoGameEvent {
 public:
  MGE_SetBlockPos(int p_eventTime);
  MGE_SetBlockPos(int p_eventTime, std::string p_blockID, float p_x, float p_y);
  ~MGE_SetBlockPos();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_blockID;
  float m_x, m_y;
};

class MGE_SetGravity : public MotoGameEvent {
 public:
  MGE_SetGravity(int p_eventTime);
  MGE_SetGravity(int p_eventTime, float p_x, float p_y);
  ~MGE_SetGravity();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  float m_x, m_y;
};

class MGE_SetPlayersPosition : public MotoGameEvent {
 public:
  MGE_SetPlayersPosition(int p_eventTime);
  MGE_SetPlayersPosition(int p_eventTime, float p_x, float p_y, bool p_bRight);
  ~MGE_SetPlayersPosition();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  float m_x, m_y;
  bool  m_bRight;
};

class MGE_SetPlayerPosition : public MotoGameEvent {
 public:
  MGE_SetPlayerPosition(int p_eventTime);
  MGE_SetPlayerPosition(int p_eventTime, float p_x, float p_y, bool p_bRight, int i_player);
  ~MGE_SetPlayerPosition();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  float m_x, m_y;
  bool  m_bRight;
  int   m_player;
};

class MGE_AddForceToPlayer : public MotoGameEvent {
 public:
  MGE_AddForceToPlayer(int p_eventTime);
  MGE_AddForceToPlayer(int p_eventTime, const Vector2f& i_force, int i_startTime, int i_endTime, int i_player);
  ~MGE_AddForceToPlayer();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  Vector2f m_force;
  int      m_startTime, m_endTime;
  int      m_player;
};

class MGE_SetEntityPos : public MotoGameEvent {
 public:
  MGE_SetEntityPos(int p_eventTime);
  MGE_SetEntityPos(int p_eventTime, std::string p_entityID, float p_x, float p_y);
  ~MGE_SetEntityPos();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_entityID;
  float m_x, m_y;
};

class MGE_SetBlockCenter : public MotoGameEvent {
 public:
  MGE_SetBlockCenter(int p_eventTime);
  MGE_SetBlockCenter(int p_eventTime, std::string p_blockID, float p_x, float p_y);
  ~MGE_SetBlockCenter();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_blockID;
  float m_x, m_y;
};

class MGE_SetBlockRotation : public MotoGameEvent {
 public:
  MGE_SetBlockRotation(int p_eventTime);
  MGE_SetBlockRotation(int p_eventTime, std::string p_blockID, float p_angle);
  ~MGE_SetBlockRotation();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_blockID;
  float m_angle;
};

class MGE_SetDynamicBlockSelfRotation : public MotoGameEvent {
 public:
  MGE_SetDynamicBlockSelfRotation(int p_eventTime);
  MGE_SetDynamicBlockSelfRotation(int p_eventTime,
				  std::string p_blockID,
				  int   p_period,
				  int   p_startTime,
				  int   p_endTime);
  ~MGE_SetDynamicBlockSelfRotation();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_blockID;
  float m_angle;
  int m_period;
  int   m_startTime;
  int   m_endTime;
};

class MGE_SetPhysicsBlockSelfRotation : public MotoGameEvent {
 public:
  MGE_SetPhysicsBlockSelfRotation(int eventTime);
  MGE_SetPhysicsBlockSelfRotation(int eventTime,
                                  std::string blockID,
                                  int   torque,
                                  int   startTime,
                                  int   endTime);
  ~MGE_SetPhysicsBlockSelfRotation();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_blockID;
  float m_angle;
  int   m_torque;
  int   m_startTime;
  int   m_endTime;
};


class MGE_SetPhysicsBlockTranslation : public MotoGameEvent {
 public:
  MGE_SetPhysicsBlockTranslation(int eventTime);
  MGE_SetPhysicsBlockTranslation(int eventTime, std::string blockID,
				 float x, float y,
				 int period, int startTime, int endTime);
  ~MGE_SetPhysicsBlockTranslation();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_blockID;
  float m_x, m_y;
  int   m_period;
  int   m_startTime;
  int   m_endTime;
};


class MGE_SetDynamicEntityRotation : public MotoGameEvent {
 public:
  MGE_SetDynamicEntityRotation(int p_eventTime);
  MGE_SetDynamicEntityRotation(int p_eventTime,
                               std::string p_entityID,
                               float p_fInitAngle,
                               float p_fRadius,
                               int   p_period,
                               int   p_startTime,
                               int   p_endTime);
  ~MGE_SetDynamicEntityRotation();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_entityID;
  float m_fInitAngle;
  float m_fRadius;
  int   m_period;
  int   m_startTime;
  int   m_endTime;
};

class MGE_SetDynamicEntitySelfRotation : public MotoGameEvent {
 public:
  MGE_SetDynamicEntitySelfRotation(int p_eventTime);
  MGE_SetDynamicEntitySelfRotation(int p_eventTime,
				  std::string p_entityID,
				  int   p_period,
				  int   p_startTime,
				  int   p_endTime);
  ~MGE_SetDynamicEntitySelfRotation();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_entityID;
  float m_angle;
  int m_period;
  int   m_startTime;
  int   m_endTime;
};

class MGE_SetDynamicEntityTranslation : public MotoGameEvent {
 public:
  MGE_SetDynamicEntityTranslation(int p_eventTime);
  MGE_SetDynamicEntityTranslation(int p_eventTime,  
                                  std::string p_entityID,
                                  float p_x,
                                  float p_y,
                                  int   p_period,
                                  int   p_startTime,
                                  int   p_endTime);
  ~MGE_SetDynamicEntityTranslation();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_entityID;
  float m_x, m_y;
  int   m_period;
  int   m_startTime;
  int   m_endTime;
};

class MGE_SetDynamicEntityNone : public MotoGameEvent {
 public:
  MGE_SetDynamicEntityNone(int p_eventTime);
  MGE_SetDynamicEntityNone(int p_eventTime, std::string p_entityID);
  ~MGE_SetDynamicEntityNone();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_entityID;
};

class MGE_SetDynamicBlockRotation : public MotoGameEvent {
 public:
  MGE_SetDynamicBlockRotation(int p_eventTime);
  MGE_SetDynamicBlockRotation(int p_eventTime,
                              std::string p_blockID,
                              float p_fInitAngle,
                              float p_fRadius,
                              int   p_period,
                              int   p_startTime,
                              int   p_endTime);
  ~MGE_SetDynamicBlockRotation();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_blockID;
  float m_fInitAngle;
  float m_fRadius;
  int   m_period;
  int   m_startTime;
  int   m_endTime;
};

// TODO::make it herit from MGE_SetDynamicBlockTranslation
class MGE_SetDynamicBlockTranslation : public MotoGameEvent {
 public:
  MGE_SetDynamicBlockTranslation(int p_eventTime);
  MGE_SetDynamicBlockTranslation(int p_eventTime,  
                                 std::string p_blockID,
                                 float p_x,
                                 float p_y,
                                 int   p_period,
                                 int   p_startTime,
                                 int   p_endTime);
  ~MGE_SetDynamicBlockTranslation();

  void doAction(MotoGame* pMotoGame);
  void serialize(DBuffer& Buffer);
  void unserialize(DBuffer& Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_blockID;
  float m_x, m_y;
  int   m_period;
  int   m_startTime;
  int   m_endTime;
};

class MGE_SetDynamicBlockNone : public MotoGameEvent {
 public:
  MGE_SetDynamicBlockNone(int p_eventTime);
  MGE_SetDynamicBlockNone(int p_eventTime, std::string p_blockID);
  ~MGE_SetDynamicBlockNone();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  std::string m_blockID;
};

class MGE_CameraMove : public MotoGameEvent {
 public:
  MGE_CameraMove(int p_eventTime);
  MGE_CameraMove(int p_eventTime, float p_moveX, float p_moveY);
  ~MGE_CameraMove();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  float m_moveX, m_moveY;
};

class MGE_CameraZoom : public MotoGameEvent {
 public:
  MGE_CameraZoom(int p_eventTime);
  MGE_CameraZoom(int p_eventTime, float p_zoom);
  ~MGE_CameraZoom();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  float m_zoom;
};

class MGE_PenalityTime : public MotoGameEvent {
 public:
  MGE_PenalityTime(int p_eventTime);
  MGE_PenalityTime(int p_eventTime, int p_penatityTime);
  ~MGE_PenalityTime();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  int m_penalityTime;
};

class MGE_CameraRotate : public MotoGameEvent {
 public:
  MGE_CameraRotate(int p_eventTime);
  MGE_CameraRotate(int p_eventTime, float p_angle);
  ~MGE_CameraRotate();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
  float m_angle;
};

class MGE_CameraAdaptToGravity : public MotoGameEvent {
 public:
  MGE_CameraAdaptToGravity(int p_eventTime);
  ~MGE_CameraAdaptToGravity();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

  std::string toString();

 private:
};

class MGE_SetCameraRotationSpeed : public MotoGameEvent {                                         
 public:                                                                                        
   MGE_SetCameraRotationSpeed(int p_eventTime);                                                    
   MGE_SetCameraRotationSpeed(int p_eventTime, float p_speed);
   ~MGE_SetCameraRotationSpeed();                                                                  
                                                                                                     
   void doAction(MotoGame *p_pMotoGame);                                                         
   void serialize(DBuffer &Buffer);                                                              
   void unserialize(DBuffer &Buffer);                                                            
   static GameEventType SgetType();                                                              
   GameEventType getType();                                                                      
	                                                                                                       
   std::string toString();                                                                       
		                                                                                                 
   private:     
   float m_speed;                                                                                  
};
                

class MGE_PlaySound : public MotoGameEvent {
 public:
  MGE_PlaySound(int p_eventTime);
  MGE_PlaySound(int p_eventTime, std::string p_soundName, float p_volume = DEFAULT_SAMPLE_VOLUME);
  ~MGE_PlaySound();
  
  void doAction(MotoGame *p_pMotoGame);                                                    
  void serialize(DBuffer &Buffer);                                                         
  void unserialize(DBuffer &Buffer);                                                       
  static GameEventType SgetType();                                                         
  GameEventType getType();                                                                 
	                                                                                                  
  std::string toString();        
  
  private:
  
  std::string m_soundName;
  float m_volume;
  
};

class MGE_PlayMusic : public MotoGameEvent {                                                                    
 public:                                                                                                        
   MGE_PlayMusic(int p_eventTime);                                                                               
   MGE_PlayMusic(int p_eventTime, std::string p_musicName);                                                      
   ~MGE_PlayMusic();                                                                                             
                                                                                                                       
   void doAction(MotoGame *p_pMotoGame);                                                                         
   void serialize(DBuffer &Buffer);                                                                              
   void unserialize(DBuffer &Buffer);                                                                            
   static GameEventType SgetType();                                                                              
   GameEventType getType();                                                                                      
		                                                                                                                 
   std::string toString();                                                                                       
		                                                                                                                   
   private:                                                                                                      
		                                                                                                                     
   std::string m_musicName;                                                                                      
		                                                                                                                       
};

class MGE_StopMusic : public MotoGameEvent {                                                                    
 public:                                                                                                        
   MGE_StopMusic(int p_eventTime);                                                                                                                               
   ~MGE_StopMusic();                                                                                             
                                                                                                                       
   void doAction(MotoGame *p_pMotoGame);                                                                         
   void serialize(DBuffer &Buffer);                                                                              
   void unserialize(DBuffer &Buffer);                                                                            
   static GameEventType SgetType();                                                                              
   GameEventType getType();                                                                                      
		                                                                                                                 
   std::string toString();                                                                                       
		                                                                                                                       
};                                
#endif /* __MOTOGAMEEVENT_H__ */
