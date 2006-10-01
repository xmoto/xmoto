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

#ifndef __MOTOGAMEEVENT_H__
#define __MOTOGAMEEVENT_H__

namespace vapp {
  /*===========================================================================
    Game event types
    ===========================================================================*/
  /* never change the order of these values ! 
     ALWAYS ADD VALUES at the end : (otherwise, old replays will not work !)
  */
  enum GameEventType {
    GAME_EVENT_PLAYER_DIES                          =  0,
    GAME_EVENT_PLAYER_ENTERS_ZONE                   =  1,
    GAME_EVENT_PLAYER_LEAVES_ZONE                   =  2,
    GAME_EVENT_PLAYER_TOUCHES_ENTITY                =  3,
    GAME_EVENT_ENTITY_DESTROYED                     =  4,    
    GAME_EVENT_LUA_CALL_CLEARMESSAGES               =  5,
    GAME_EVENT_LUA_CALL_PLACEINGAMEARROW            =  6,
    GAME_EVENT_LUA_CALL_PLACESCREENARROW            =  7,
    GAME_EVENT_LUA_CALL_HIDEARROW                   =  8,
    GAME_EVENT_LUA_CALL_MESSAGE                     =  9,
    GAME_EVENT_LUA_CALL_MOVEBLOCK                   = 10,
    GAME_EVENT_LUA_CALL_SETBLOCKPOS                 = 11,
    GAME_EVENT_LUA_CALL_SETGRAVITY                  = 12,
    GAME_EVENT_LUA_CALL_SETPLAYERPOSITION           = 13,
    GAME_EVENT_LUA_CALL_SETENTITYPOS                = 14,
    GAME_EVENT_LUA_CALL_SETBLOCKCENTER              = 15,
    GAME_EVENT_LUA_CALL_SETBLOCKROTATION            = 16, 
    GAME_EVENT_LUA_CALL_SETDYNAMICENTITYROTATION    = 17,
    GAME_EVENT_LUA_CALL_SETDYNAMICENTITYTRANSLATION = 18,
    GAME_EVENT_LUA_CALL_SETDYNAMICENTITYNONE        = 19,
    GAME_EVENT_LUA_CALL_CAMERAZOOM                  = 20,
    GAME_EVENT_LUA_CALL_CAMERAMOVE                  = 21,
    GAME_EVENT_LUA_CALL_SETDYNAMICBLOCKROTATION     = 22,
    GAME_EVENT_LUA_CALL_SETDYNAMICBLOCKTRANSLATION  = 23,
    GAME_EVENT_LUA_CALL_SETDYNAMICBLOCKNONE         = 24,
    GAME_EVENT_LUA_CALL_PENALITY_TIME               = 25
  };
}

#include "MotoGame.h"

namespace vapp {

class MotoGameEvent {
  public:
  MotoGameEvent(float p_fEventTime);
  virtual ~MotoGameEvent();

  virtual void doAction(MotoGame *p_pMotoGame)          = 0;
  virtual void serialize(DBuffer &Buffer)               = 0;
  virtual void unserialize(DBuffer &Buffer)             = 0;
  virtual void revert(MotoGame *p_pMotoGame);
  virtual GameEventType getType()                       = 0;

  static MotoGameEvent* getUnserialized(DBuffer &Buffer);
  float getEventTime();

  protected:
  float m_fEventTime;
};

class MGE_PlayerDies : public MotoGameEvent {
 public:
  MGE_PlayerDies(float p_fEventTime);
  MGE_PlayerDies(float p_fEventTime, bool p_bKilledByWrecker);
  ~MGE_PlayerDies();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  bool m_bKilledByWrecker;
};

class MGE_PlayerEntersZone : public MotoGameEvent {
 public:
  MGE_PlayerEntersZone(float p_fEventTime);
  MGE_PlayerEntersZone(float p_fEventTime, LevelZone *p_zone);
  ~MGE_PlayerEntersZone();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  LevelZone *m_zone;
};

class MGE_PlayerLeavesZone : public MotoGameEvent {
 public:
  MGE_PlayerLeavesZone(float p_fEventTime);
  MGE_PlayerLeavesZone(float p_fEventTime, LevelZone *p_zone);
  ~MGE_PlayerLeavesZone();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  LevelZone *m_zone;
};

class MGE_PlayerTouchesEntity : public MotoGameEvent {
 public:
  MGE_PlayerTouchesEntity(float p_fEventTime);
  MGE_PlayerTouchesEntity(float p_fEventTime, std::string p_entityID, bool p_bTouchedWithHead);
  ~MGE_PlayerTouchesEntity();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
    std::string m_entityID;
    bool m_bTouchedWithHead;
};

class MGE_EntityDestroyed : public MotoGameEvent {
 public:
  MGE_EntityDestroyed(float p_fEventTime);
  MGE_EntityDestroyed(float p_fEventTime, std::string p_entityID, EntityType p_type,
                      float p_fSize, float p_fPosX, float p_fPosY);
  ~MGE_EntityDestroyed();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  void revert(MotoGame *p_pMotoGame);
  static GameEventType SgetType();
  GameEventType getType();

  EntityType getEntityType();

 private:
  /* Have enough information so that we can recreate the entity */
  std::string m_entityID;
  EntityType  m_type;
  float m_fSize;
  float m_fPosX, m_fPosY;
};

class MGE_ClearMessages : public MotoGameEvent {
 public:
  MGE_ClearMessages(float p_fEventTime);
  ~MGE_ClearMessages();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
};

class MGE_PlaceInGameArrow : public MotoGameEvent {
 public:
  MGE_PlaceInGameArrow(float p_fEventTime);
  MGE_PlaceInGameArrow(float p_fEventTime, float p_x, float p_y, float p_angle);
  ~MGE_PlaceInGameArrow();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  float m_x, m_y;
  float m_angle;
};

class MGE_PlaceScreenarrow : public MotoGameEvent {
 public:
  MGE_PlaceScreenarrow(float p_fEventTime);
  MGE_PlaceScreenarrow(float p_fEventTime, float p_x, float p_y, float p_angle);
  ~MGE_PlaceScreenarrow();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  float m_x, m_y;
  float m_angle;
};

class MGE_HideArrow : public MotoGameEvent {
 public:
  MGE_HideArrow(float p_fEventTime);
  ~MGE_HideArrow();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
};

class MGE_Message : public MotoGameEvent {
 public:
  MGE_Message(float p_fEventTime);
  MGE_Message(float p_fEventTime, std::string p_message);
  ~MGE_Message();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  std::string m_message;
};

class MGE_MoveBlock : public MotoGameEvent {
 public:
  MGE_MoveBlock(float p_fEventTime);
  MGE_MoveBlock(float p_fEventTime, std::string p_blockID, float p_x, float p_y);
  ~MGE_MoveBlock();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  std::string m_blockID;
  float m_x, m_y;
};

class MGE_SetBlockPos : public MotoGameEvent {
 public:
  MGE_SetBlockPos(float p_fEventTime);
  MGE_SetBlockPos(float p_fEventTime, std::string p_blockID, float p_x, float p_y);
  ~MGE_SetBlockPos();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  std::string m_blockID;
  float m_x, m_y;
};

class MGE_SetGravity : public MotoGameEvent {
 public:
  MGE_SetGravity(float p_fEventTime);
  MGE_SetGravity(float p_fEventTime, float p_x, float p_y);
  ~MGE_SetGravity();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  float m_x, m_y;
};

class MGE_SetPlayerPosition : public MotoGameEvent {
 public:
  MGE_SetPlayerPosition(float p_fEventTime);
  MGE_SetPlayerPosition(float p_fEventTime, float p_x, float p_y, bool p_bRight);
  ~MGE_SetPlayerPosition();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  float m_x, m_y;
  bool  m_bRight;
};

class MGE_SetEntityPos : public MotoGameEvent {
 public:
  MGE_SetEntityPos(float p_fEventTime);
  MGE_SetEntityPos(float p_fEventTime, std::string p_entityID, float p_x, float p_y);
  ~MGE_SetEntityPos();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  std::string m_entityID;
  float m_x, m_y;
};

class MGE_SetBlockCenter : public MotoGameEvent {
 public:
  MGE_SetBlockCenter(float p_fEventTime);
  MGE_SetBlockCenter(float p_fEventTime, std::string p_blockID, float p_x, float p_y);
  ~MGE_SetBlockCenter();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  std::string m_blockID;
  float m_x, m_y;
};

class MGE_SetBlockRotation : public MotoGameEvent {
 public:
  MGE_SetBlockRotation(float p_fEventTime);
  MGE_SetBlockRotation(float p_fEventTime, std::string p_blockID, float p_angle);
  ~MGE_SetBlockRotation();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  std::string m_blockID;
  float m_angle;
};

class MGE_SetDynamicEntityRotation : public MotoGameEvent {
 public:
  MGE_SetDynamicEntityRotation(float p_fEventTime);
  MGE_SetDynamicEntityRotation(float p_fEventTime,
                               std::string p_entityID,
                               float p_fInitAngle,
                               float p_fRadius,
                               float p_fPeriod,
                               int   p_startTime,
                               int   p_endTime);
  ~MGE_SetDynamicEntityRotation();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  std::string m_entityID;
  float m_fInitAngle;
  float m_fRadius;
  float m_fPeriod;
  int   m_startTime;
  int   m_endTime;
};

class MGE_SetDynamicEntityTranslation : public MotoGameEvent {
 public:
  MGE_SetDynamicEntityTranslation(float p_fEventTime);
  MGE_SetDynamicEntityTranslation(float p_fEventTime,  
                                  std::string p_entityID,
                                  float p_x,
                                  float p_y,
                                  float p_fPeriod,
                                  int   p_startTime,
                                  int   p_endTime);
  ~MGE_SetDynamicEntityTranslation();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  std::string m_entityID;
  float m_x, m_y;
  float m_fPeriod;
  int   m_startTime;
  int   m_endTime;
};

class MGE_SetDynamicEntityNone : public MotoGameEvent {
 public:
  MGE_SetDynamicEntityNone(float p_fEventTime);
  MGE_SetDynamicEntityNone(float p_fEventTime, std::string p_entityID);
  ~MGE_SetDynamicEntityNone();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  std::string m_entityID;
};

class MGE_SetDynamicBlockRotation : public MotoGameEvent {
 public:
  MGE_SetDynamicBlockRotation(float p_fEventTime);
  MGE_SetDynamicBlockRotation(float p_fEventTime,
                              std::string p_blockID,
                              float p_fInitAngle,
                              float p_fRadius,
                              float p_fPeriod,
                              int   p_startTime,
                              int   p_endTime);
  ~MGE_SetDynamicBlockRotation();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  std::string m_blockID;
  float m_fInitAngle;
  float m_fRadius;
  float m_fPeriod;
  int   m_startTime;
  int   m_endTime;
};

class MGE_SetDynamicBlockTranslation : public MotoGameEvent {
 public:
  MGE_SetDynamicBlockTranslation(float p_fEventTime);
  MGE_SetDynamicBlockTranslation(float p_fEventTime,  
                                 std::string p_blockID,
                                 float p_x,
                                 float p_y,
                                 float p_fPeriod,
                                 int   p_startTime,
                                 int   p_endTime);
  ~MGE_SetDynamicBlockTranslation();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  std::string m_blockID;
  float m_x, m_y;
  float m_fPeriod;
  int   m_startTime;
  int   m_endTime;
};

class MGE_SetDynamicBlockNone : public MotoGameEvent {
 public:
  MGE_SetDynamicBlockNone(float p_fEventTime);
  MGE_SetDynamicBlockNone(float p_fEventTime, std::string p_blockID);
  ~MGE_SetDynamicBlockNone();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  std::string m_blockID;
};

class MGE_CameraMove : public MotoGameEvent {
 public:
  MGE_CameraMove(float p_fEventTime);
  MGE_CameraMove(float p_fEventTime, float p_moveX, float p_moveY);
  ~MGE_CameraMove();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  float m_moveX, m_moveY;
};

class MGE_CameraZoom : public MotoGameEvent {
 public:
  MGE_CameraZoom(float p_fEventTime);
  MGE_CameraZoom(float p_fEventTime, float p_zoom);
  ~MGE_CameraZoom();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  float m_zoom;
};

class MGE_PenalityTime : public MotoGameEvent {
 public:
  MGE_PenalityTime(float p_fEventTime);
  MGE_PenalityTime(float p_fEventTime, float p_penatityTime);
  ~MGE_PenalityTime();

  void doAction(MotoGame *p_pMotoGame);
  void serialize(DBuffer &Buffer);
  void unserialize(DBuffer &Buffer);
  static GameEventType SgetType();
  GameEventType getType();

 private:
  float m_penalityTime;
};

}

#endif /* __MOTOGAMEEVENT_H__ */
