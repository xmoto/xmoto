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

#include "GameEvents.h"

namespace vapp {

  MotoGameEvent::MotoGameEvent(float p_fEventTime) {
    m_fEventTime = p_fEventTime;
  }

  MotoGameEvent::~MotoGameEvent() {
  }

  void MotoGameEvent::serialize(DBuffer &Buffer) {
    Buffer << m_fEventTime;
    Buffer << this->getType();
  }

  MotoGameEvent* MotoGameEvent::getUnserialized(DBuffer &Buffer) {
    MotoGameEvent* v_event;
    float v_fEventTime;
    GameEventType v_eventType;

    Buffer >> v_fEventTime;
    Buffer >> ((int)v_eventType);

    if(MGE_PlayerDies::SgetType() == v_eventType) {
      v_event = new MGE_PlayerDies(v_fEventTime);
    } else if(MGE_PlayerEntersZone::SgetType() == v_eventType) {
      v_event = new MGE_PlayerEntersZone(v_fEventTime);
    } else if(MGE_PlayerLeavesZone::SgetType() == v_eventType) {
      v_event = new MGE_PlayerLeavesZone(v_fEventTime);
    } else if(MGE_PlayerTouchesEntity::SgetType() == v_eventType) {
      v_event = new MGE_PlayerTouchesEntity(v_fEventTime);
    } else if(MGE_EntityDestroyed::SgetType() == v_eventType) {
      v_event = new MGE_EntityDestroyed(v_fEventTime);
    } else if(MGE_ClearMessages::SgetType() == v_eventType) {
      v_event = new MGE_ClearMessages(v_fEventTime);
    } else if(MGE_PlaceInGameArrow::SgetType() == v_eventType) {
      v_event = new MGE_PlaceInGameArrow(v_fEventTime);
    } else if(MGE_PlaceScreenarrow::SgetType() == v_eventType) {
      v_event = new MGE_PlaceScreenarrow(v_fEventTime);
    } else if(MGE_HideArrow::SgetType() == v_eventType) {
      v_event = new MGE_HideArrow(v_fEventTime);
    } else if(MGE_Message::SgetType() == v_eventType) {
      v_event = new MGE_Message(v_fEventTime);
    } else if(MGE_MoveBlock::SgetType() == v_eventType) {
      v_event = new MGE_MoveBlock(v_fEventTime);
    } else if(MGE_SetBlockPos::SgetType() == v_eventType) {
      v_event = new MGE_SetBlockPos(v_fEventTime);
    } else if(MGE_SetGravity::SgetType() == v_eventType) {
      v_event = new MGE_SetGravity(v_fEventTime);
    } else if(MGE_SetPlayerPosition::SgetType() == v_eventType) {
      v_event = new MGE_SetPlayerPosition(v_fEventTime);
    } else if(MGE_SetEntityPos::SgetType() == v_eventType) {
      v_event = new MGE_SetEntityPos(v_fEventTime);
    } else if(MGE_SetBlockCenter::SgetType() == v_eventType) {
      v_event = new MGE_SetBlockCenter(v_fEventTime);
    } else if(MGE_SetBlockRotation::SgetType() == v_eventType) {
      v_event = new MGE_SetBlockRotation(v_fEventTime);
    } else if(MGE_SetDynamicEntityRotation::SgetType() == v_eventType) {
      v_event = new MGE_SetDynamicEntityRotation(v_fEventTime);
    } else if(MGE_SetDynamicEntityTranslation::SgetType() == v_eventType) {
      v_event = new MGE_SetDynamicEntityTranslation(v_fEventTime);
    } else if(MGE_SetDynamicEntityNone::SgetType() == v_eventType) {
      v_event = new MGE_SetDynamicEntityNone(v_fEventTime);
    } else if(MGE_SetDynamicBlockRotation::SgetType() == v_eventType) {
      v_event = new MGE_SetDynamicBlockRotation(v_fEventTime);
    } else if(MGE_SetDynamicBlockTranslation::SgetType() == v_eventType) {
      v_event = new MGE_SetDynamicBlockTranslation(v_fEventTime);
    } else if(MGE_SetDynamicBlockNone::SgetType() == v_eventType) {
      v_event = new MGE_SetDynamicBlockNone(v_fEventTime);
    } else if(MGE_CameraMove::SgetType() == v_eventType) {
      v_event = new MGE_CameraMove(v_fEventTime);
    } else if(MGE_CameraZoom::SgetType() == v_eventType) {
      v_event = new MGE_CameraZoom(v_fEventTime);
    } else if(MGE_PenalityTime::SgetType() == v_eventType) {
      v_event = new MGE_PenalityTime(v_fEventTime);
    } else {
      std::ostringstream error_type;
      error_type << (int) v_eventType;
      throw Exception("Can't unserialize ! (event of type " + error_type.str() + ")");
    }

    v_event->unserialize(Buffer);
    return v_event;
  }

  void MotoGameEvent::revert(MotoGame *p_pMotoGame) {
  }

  float MotoGameEvent::getEventTime() {
    return m_fEventTime;
  }

  //////////////////////////////
  MGE_PlayerDies::MGE_PlayerDies(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_bKilledByWrecker = false;
  }

  MGE_PlayerDies::MGE_PlayerDies(float p_fEventTime, bool p_bKilledByWrecker) 
    : MotoGameEvent(p_fEventTime) {
      m_bKilledByWrecker = p_bKilledByWrecker;
  }

  MGE_PlayerDies::~MGE_PlayerDies() {
  } 
  
  void MGE_PlayerDies::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->killPlayer();
  }

  void MGE_PlayerDies::serialize(DBuffer &Buffer) {
  }
  
  void MGE_PlayerDies::unserialize(DBuffer &Buffer) {
  }

  GameEventType MGE_PlayerDies::SgetType() {
    return GAME_EVENT_PLAYER_DIES;
  }

  GameEventType MGE_PlayerDies::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_PlayerEntersZone::MGE_PlayerEntersZone(float p_fEventTime) 
    : MotoGameEvent(p_fEventTime) {
      m_zone = NULL;
    }

  MGE_PlayerEntersZone::MGE_PlayerEntersZone(float p_fEventTime, LevelZone *p_zone) 
    : MotoGameEvent(p_fEventTime) {
      m_zone = p_zone;
    }


  MGE_PlayerEntersZone::~MGE_PlayerEntersZone() {
  } 
  
  void MGE_PlayerEntersZone::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->playerEntersZone(m_zone);
  }

  void MGE_PlayerEntersZone::serialize(DBuffer &Buffer) {
  }
  
  void MGE_PlayerEntersZone::unserialize(DBuffer &Buffer) {
  }

  GameEventType MGE_PlayerEntersZone::SgetType() {
     return GAME_EVENT_PLAYER_ENTERS_ZONE;
  }

  GameEventType MGE_PlayerEntersZone::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_PlayerLeavesZone::MGE_PlayerLeavesZone(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_zone = NULL;
  }

  MGE_PlayerLeavesZone::MGE_PlayerLeavesZone(float p_fEventTime, LevelZone *p_zone) 
    : MotoGameEvent(p_fEventTime) {
      m_zone = p_zone;
  }

  MGE_PlayerLeavesZone::~MGE_PlayerLeavesZone() {
  } 
  
  void MGE_PlayerLeavesZone::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->playerLeavesZone(m_zone);
  }

  void MGE_PlayerLeavesZone::serialize(DBuffer &Buffer) {
  }
  
  void MGE_PlayerLeavesZone::unserialize(DBuffer &Buffer) {
  }

  GameEventType MGE_PlayerLeavesZone::SgetType() {
    return GAME_EVENT_PLAYER_LEAVES_ZONE;
  }

  GameEventType MGE_PlayerLeavesZone::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_PlayerTouchesEntity::MGE_PlayerTouchesEntity(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_entityID = "";
    m_bTouchedWithHead = false;
  }

  MGE_PlayerTouchesEntity::MGE_PlayerTouchesEntity(float p_fEventTime, std::string p_entityID, bool p_bTouchedWithHead) 
    : MotoGameEvent(p_fEventTime) {
      m_entityID = p_entityID;
      m_bTouchedWithHead = p_bTouchedWithHead;
    }

  MGE_PlayerTouchesEntity::~MGE_PlayerTouchesEntity() {
  } 
  
  void MGE_PlayerTouchesEntity::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->playerTouchesEntity(m_entityID, m_bTouchedWithHead);
  }

  void MGE_PlayerTouchesEntity::serialize(DBuffer &Buffer) {
  }
  
  void MGE_PlayerTouchesEntity::unserialize(DBuffer &Buffer) {
  }

  GameEventType MGE_PlayerTouchesEntity::SgetType() {
    return GAME_EVENT_PLAYER_TOUCHES_ENTITY;
  }

  GameEventType MGE_PlayerTouchesEntity::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_EntityDestroyed::MGE_EntityDestroyed(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_entityID = "";
    m_type = ET_UNASSIGNED;
    m_fSize = 0.0;
    m_fPosX = 0.0;
    m_fPosY = 0.0;
  }

  MGE_EntityDestroyed::MGE_EntityDestroyed(float p_fEventTime,
                                           std::string p_entityID, EntityType p_type,
                                           float p_fSize, float p_fPosX, float p_fPosY) 
    : MotoGameEvent(p_fEventTime) {
      m_entityID = p_entityID;
      m_type = p_type;
      m_fSize = p_fSize;
      m_fPosX = p_fPosX;
      m_fPosY = p_fPosY;
    }

  MGE_EntityDestroyed::~MGE_EntityDestroyed() {
  } 
  
  void MGE_EntityDestroyed::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->entityDestroyed(m_entityID, m_type, m_fSize, m_fPosX, m_fPosY);
  }

  void MGE_EntityDestroyed::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer.write(m_entityID);
    Buffer << m_type;
    Buffer << m_fSize;
    Buffer << m_fPosX;
    Buffer << m_fPosY;
  }
  
  void MGE_EntityDestroyed::unserialize(DBuffer &Buffer) {
    Buffer.read(m_entityID);
    Buffer >> m_type;
    switch(m_type) {
    case ET_UNASSIGNED:
    case ET_SPRITE:
    case ET_PLAYERSTART:
    case ET_ENDOFLEVEL:
    case ET_WRECKER:
    case ET_STRAWBERRY:
    case ET_PARTICLESOURCE:
    case ET_DUMMY:
      break;
    default:
      throw Exception("Invalid entity type"); // with some compilator, an invalid value causes a segfault (on my linux box)
    }
    Buffer >> m_fSize;
    Buffer >> m_fPosX;
    Buffer >> m_fPosY;   
  }

  void MGE_EntityDestroyed::revert(MotoGame *p_pMotoGame) {
    p_pMotoGame->revertEntityDestroyed(m_entityID);
  }

  GameEventType MGE_EntityDestroyed::SgetType() {
    return GAME_EVENT_ENTITY_DESTROYED;
  }

  GameEventType MGE_EntityDestroyed::getType() {
    return SgetType();
  }

  EntityType MGE_EntityDestroyed::getEntityType() {
    return m_type;
  }

  //////////////////////////////
  MGE_ClearMessages::MGE_ClearMessages(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
  }

  MGE_ClearMessages::~MGE_ClearMessages() {
  } 
  
  void MGE_ClearMessages::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->clearGameMessages();
  }

  void MGE_ClearMessages::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
  }
  
  void MGE_ClearMessages::unserialize(DBuffer &Buffer) {    
  }

  GameEventType MGE_ClearMessages::SgetType() {
    return GAME_EVENT_LUA_CALL_CLEARMESSAGES;
  }

  GameEventType MGE_ClearMessages::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_PlaceInGameArrow::MGE_PlaceInGameArrow(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_x = 0.0;
    m_y = 0.0;
    m_angle = 0.0;
  }

  MGE_PlaceInGameArrow::MGE_PlaceInGameArrow(float p_fEventTime,
                                             float p_x, float p_y,
                                             float p_angle) 
    : MotoGameEvent(p_fEventTime){
      m_x = p_x;
      m_y = p_y;
      m_angle = p_angle;
    }

  MGE_PlaceInGameArrow::~MGE_PlaceInGameArrow() {
  } 
  
  void MGE_PlaceInGameArrow::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->PlaceInGameArrow(m_x, m_y, m_angle);
  }

  void MGE_PlaceInGameArrow::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
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
    return GAME_EVENT_LUA_CALL_PLACEINGAMEARROW;
  }

  GameEventType MGE_PlaceInGameArrow::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_PlaceScreenarrow::MGE_PlaceScreenarrow(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_x = 0.0;
    m_y = 0.0;
    m_angle = 0.0;
  }

  MGE_PlaceScreenarrow::MGE_PlaceScreenarrow(float p_fEventTime,
                                             float p_x, float p_y,
                                             float p_angle) 
    : MotoGameEvent(p_fEventTime) {
      m_x = p_x;
      m_y = p_y;
      m_angle = p_angle;
    }
  
  MGE_PlaceScreenarrow::~MGE_PlaceScreenarrow() {
  } 
  
  void MGE_PlaceScreenarrow::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->PlaceScreenArrow(m_x, m_y, m_angle); 
  }

  void MGE_PlaceScreenarrow::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
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
    return GAME_EVENT_LUA_CALL_PLACESCREENARROW;
  }

  GameEventType MGE_PlaceScreenarrow::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_HideArrow::MGE_HideArrow(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
  }

  MGE_HideArrow::~MGE_HideArrow() {
  } 
  
  void MGE_HideArrow::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->HideArrow();
  }

  void MGE_HideArrow::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
  }
  
  void MGE_HideArrow::unserialize(DBuffer &Buffer) {
  }

  GameEventType MGE_HideArrow::SgetType() {
    return GAME_EVENT_LUA_CALL_HIDEARROW;
  }

  GameEventType MGE_HideArrow::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_Message::MGE_Message(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_message = "";
  }

  MGE_Message::MGE_Message(float p_fEventTime, std::string p_message) 
    : MotoGameEvent(p_fEventTime) {
      m_message = p_message;
    }

  MGE_Message::~MGE_Message() {
  } 
  
  void MGE_Message::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->gameMessage(m_message);
  }

  void MGE_Message::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer.write(m_message);
  }
  
  void MGE_Message::unserialize(DBuffer &Buffer) {
    Buffer.read(m_message);
  }

  GameEventType MGE_Message::SgetType() {
    return GAME_EVENT_LUA_CALL_MESSAGE;
  }

  GameEventType MGE_Message::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_MoveBlock::MGE_MoveBlock(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_blockID = "";
    m_x = 0.0;
    m_y = 0.0;
  }

  MGE_MoveBlock::MGE_MoveBlock(float p_fEventTime,
                               std::string p_blockID,
                               float p_x, float p_y)  
    : MotoGameEvent(p_fEventTime){
      m_blockID = p_blockID;
      m_x = p_x;
      m_y = p_y;
  }
  
  MGE_MoveBlock::~MGE_MoveBlock() {
  } 
  
  void MGE_MoveBlock::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->MoveBlock(m_blockID, m_x, m_y);
  }

  void MGE_MoveBlock::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer.write(m_blockID);
    Buffer << m_x;
    Buffer << m_y;
  }
  
  void MGE_MoveBlock::unserialize(DBuffer &Buffer) {
    Buffer.read(m_blockID);
    Buffer >> m_x;
    Buffer >> m_y;
  }

  GameEventType MGE_MoveBlock::SgetType() {
    return GAME_EVENT_LUA_CALL_MOVEBLOCK;
  }

  GameEventType MGE_MoveBlock::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_SetBlockPos::MGE_SetBlockPos(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_blockID = "";
    m_x = 0.0;
    m_y = 0.0;
  }

  MGE_SetBlockPos::MGE_SetBlockPos(float p_fEventTime,
                                   std::string p_blockID,
                                   float p_x, float p_y) 
  : MotoGameEvent(p_fEventTime) {
    m_blockID = p_blockID;
    m_x = p_x;
    m_y = p_y;
  }

  MGE_SetBlockPos::~MGE_SetBlockPos() {
  } 
  
  void MGE_SetBlockPos::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->SetBlockPos(m_blockID, m_x, m_y);
  }

  void MGE_SetBlockPos::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer.write(m_blockID);
    Buffer << m_x;
    Buffer << m_y;
  }
  
  void MGE_SetBlockPos::unserialize(DBuffer &Buffer) {
    Buffer.read(m_blockID);
    Buffer >> m_x;
    Buffer >> m_y;
  }

  GameEventType MGE_SetBlockPos::SgetType() {
    return GAME_EVENT_LUA_CALL_SETBLOCKPOS;
  }

  GameEventType MGE_SetBlockPos::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_SetGravity::MGE_SetGravity(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_x = 0.0;
    m_y = 0.0;
  }

  MGE_SetGravity::MGE_SetGravity(float p_fEventTime, float p_x, float p_y) 
    : MotoGameEvent(p_fEventTime) {
      m_x = p_x;
      m_y = p_y;
  }

  MGE_SetGravity::~MGE_SetGravity() {
  } 
  
  void MGE_SetGravity::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->setGravity(m_x, m_y);
  }

  void MGE_SetGravity::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer << m_x;
    Buffer << m_y;
  }
  
  void MGE_SetGravity::unserialize(DBuffer &Buffer) {
    Buffer >> m_x;
    Buffer >> m_y;
  }

  GameEventType MGE_SetGravity::SgetType() {
    return GAME_EVENT_LUA_CALL_SETGRAVITY;
  }

  GameEventType MGE_SetGravity::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_SetPlayerPosition::MGE_SetPlayerPosition(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_x = 0.0;
    m_y = 0.0;
    m_bRight = 0.0;
  }

  MGE_SetPlayerPosition::MGE_SetPlayerPosition(float p_fEventTime,
                                               float p_x, float p_y,
                                               bool p_bRight) 
    : MotoGameEvent(p_fEventTime) {
      m_x = p_x;
      m_y = p_y;
      m_bRight = p_bRight;
    }

  MGE_SetPlayerPosition::~MGE_SetPlayerPosition() {
  } 
  
  void MGE_SetPlayerPosition::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->setPlayerPosition(m_x, m_y, m_bRight);
  }

  void MGE_SetPlayerPosition::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer << m_x;
    Buffer << m_y;
    Buffer << m_bRight;
  }
  
  void MGE_SetPlayerPosition::unserialize(DBuffer &Buffer) {
    Buffer >> m_x;
    Buffer >> m_y;
    Buffer >> m_bRight;
  }

  GameEventType MGE_SetPlayerPosition::SgetType() {
    return GAME_EVENT_LUA_CALL_SETPLAYERPOSITION;
  }

  GameEventType MGE_SetPlayerPosition::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_SetEntityPos::MGE_SetEntityPos(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_entityID = "";
    m_x = 0.0;
    m_y = 0.0;
  }

  MGE_SetEntityPos::MGE_SetEntityPos(float p_fEventTime,
                                     std::string p_entityID,
                                     float p_x, float p_y)  
  : MotoGameEvent(p_fEventTime){
    m_entityID = p_entityID;
    m_x = p_x;
    m_y = p_y;
  }

  MGE_SetEntityPos::~MGE_SetEntityPos() {
  } 
  
  void MGE_SetEntityPos::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->SetEntityPos(m_entityID, m_x, m_y);
  }

  void MGE_SetEntityPos::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer.write(m_entityID);
    Buffer << m_x;
    Buffer << m_y;
  }
  
  void MGE_SetEntityPos::unserialize(DBuffer &Buffer) {
    Buffer.read(m_entityID);
    Buffer >> m_x;
    Buffer >> m_y;
  }

  GameEventType MGE_SetEntityPos::SgetType() {
    return GAME_EVENT_LUA_CALL_SETENTITYPOS;
  }

  GameEventType MGE_SetEntityPos::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_SetBlockCenter::MGE_SetBlockCenter(float p_fEventTime) 
    : MotoGameEvent(p_fEventTime) {
      m_blockID = "";
      m_x = 0.0;
      m_y = 0.0;
    }

  MGE_SetBlockCenter::MGE_SetBlockCenter(float p_fEventTime,
                                         std::string p_blockID,
                                         float p_x, float p_y) 
    : MotoGameEvent(p_fEventTime) {
      m_blockID = p_blockID;
      m_x = p_x;
      m_y = p_y;
    }

  MGE_SetBlockCenter::~MGE_SetBlockCenter() {
  } 
  
  void MGE_SetBlockCenter::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->SetBlockCenter(m_blockID, m_x, m_y);
  }

  void MGE_SetBlockCenter::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer.write(m_blockID);
    Buffer << m_x;
    Buffer << m_y;
  }
  
  void MGE_SetBlockCenter::unserialize(DBuffer &Buffer) {
    Buffer.read(m_blockID);
    Buffer >> m_x;
    Buffer >> m_y;
  }

  GameEventType MGE_SetBlockCenter::SgetType() {
    return GAME_EVENT_LUA_CALL_SETBLOCKCENTER;
  }

  GameEventType MGE_SetBlockCenter::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_SetBlockRotation::MGE_SetBlockRotation(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_blockID = "";
    m_angle = 0.0;
  }

  MGE_SetBlockRotation::MGE_SetBlockRotation(float p_fEventTime,
                                             std::string p_blockID,
                                             float p_angle) 
    : MotoGameEvent(p_fEventTime) {
      m_blockID = p_blockID;
      m_angle = p_angle;
  }

  MGE_SetBlockRotation::~MGE_SetBlockRotation() {
  } 
  
  void MGE_SetBlockRotation::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->SetBlockRotation(m_blockID, m_angle);
  }

  void MGE_SetBlockRotation::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer.write(m_blockID);
    Buffer << m_angle;
  }
  
  void MGE_SetBlockRotation::unserialize(DBuffer &Buffer) {
    Buffer.read(m_blockID);
    Buffer >> m_angle;
  }

  GameEventType MGE_SetBlockRotation::SgetType() {
    return GAME_EVENT_LUA_CALL_SETBLOCKROTATION ;
  }

  GameEventType MGE_SetBlockRotation::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_SetDynamicEntityRotation::MGE_SetDynamicEntityRotation(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_entityID   = "";
    m_fInitAngle = 0.0;
    m_fRadius    = 0.0;
    m_fPeriod    = 0.0;
    m_startTime  = 0;
    m_endTime    = 0;
  }

  MGE_SetDynamicEntityRotation::MGE_SetDynamicEntityRotation(float p_fEventTime,
                                                             std::string p_entityID,
                                                             float p_fInitAngle,
                                                             float p_fRadius,
                                                             float p_fPeriod,
                                                             int   p_startTime,
                                                             int   p_endTime) 
    : MotoGameEvent(p_fEventTime) {
      m_entityID   = p_entityID;
      m_fInitAngle = p_fInitAngle;
      m_fRadius    = p_fRadius;
      m_fPeriod    = p_fPeriod;
      m_startTime  = p_startTime;
      m_endTime    = p_endTime;
    }

  MGE_SetDynamicEntityRotation::~MGE_SetDynamicEntityRotation() {
  } 
  
  void MGE_SetDynamicEntityRotation::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->addDynamicObject(new SDynamicEntityRotation(m_entityID,
							     m_fInitAngle, m_fRadius,
							     m_fPeriod,
							     m_startTime, m_endTime));
  }

  void MGE_SetDynamicEntityRotation::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer.write(m_entityID);
    Buffer << m_fInitAngle;
    Buffer << m_fRadius;
    Buffer << m_fPeriod;
    Buffer << m_startTime;
    Buffer << m_endTime;
  }
  
  void MGE_SetDynamicEntityRotation::unserialize(DBuffer &Buffer) {
    Buffer.read(m_entityID);
    Buffer >> m_fInitAngle;
    Buffer >> m_fRadius;
    Buffer >> m_fPeriod;
    Buffer >> m_startTime;
    Buffer >> m_endTime;
  }

  GameEventType MGE_SetDynamicEntityRotation::SgetType() {
    return GAME_EVENT_LUA_CALL_SETDYNAMICENTITYROTATION ;
  }

  GameEventType MGE_SetDynamicEntityRotation::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_SetDynamicEntityTranslation::MGE_SetDynamicEntityTranslation(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
      m_entityID   = "";
      m_x = 0.0;
      m_y = 0.0;
      m_fPeriod    = 0.0;
      m_startTime  = 0;
      m_endTime    = 0;
  }

  MGE_SetDynamicEntityTranslation::MGE_SetDynamicEntityTranslation(float p_fEventTime,  
                                                                   std::string p_entityID,
                                                                   float p_x,
                                                                   float p_y,
                                                                   float p_fPeriod,
                                                                   int   p_startTime,
                                                                   int   p_endTime) 
    : MotoGameEvent(p_fEventTime) {
      m_entityID   = p_entityID;
      m_x = p_x;
      m_y = p_y;
      m_fPeriod    = p_fPeriod;
      m_startTime  = p_startTime;
      m_endTime    = p_endTime;
    }

  MGE_SetDynamicEntityTranslation::~MGE_SetDynamicEntityTranslation() {
  } 
  
  void MGE_SetDynamicEntityTranslation::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->addDynamicObject(new SDynamicEntityTranslation(m_entityID,
								m_x, m_y,
								m_fPeriod,
								m_startTime, m_endTime));
  }

  void MGE_SetDynamicEntityTranslation::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer.write(m_entityID);
    Buffer << m_x;
    Buffer << m_y;
    Buffer << m_fPeriod;
    Buffer << m_startTime;
    Buffer << m_endTime;
  }
  
  void MGE_SetDynamicEntityTranslation::unserialize(DBuffer &Buffer) {
    Buffer.read(m_entityID);
    Buffer >> m_x;
    Buffer >> m_y;
    Buffer >> m_fPeriod;
    Buffer >> m_startTime;
    Buffer >> m_endTime;
  }

  GameEventType MGE_SetDynamicEntityTranslation::SgetType() {
    return GAME_EVENT_LUA_CALL_SETDYNAMICENTITYTRANSLATION;
  }

  GameEventType MGE_SetDynamicEntityTranslation::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_SetDynamicEntityNone::MGE_SetDynamicEntityNone(float p_fEventTime) 
    : MotoGameEvent(p_fEventTime) {
      m_entityID = "";
    }

  MGE_SetDynamicEntityNone::MGE_SetDynamicEntityNone(float p_fEventTime, std::string p_entityID)
    : MotoGameEvent(p_fEventTime) {
      m_entityID = p_entityID;
    }

  MGE_SetDynamicEntityNone::~MGE_SetDynamicEntityNone() {
  } 
  
  void MGE_SetDynamicEntityNone::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->removeSDynamicOfObject(m_entityID);
  }

  void MGE_SetDynamicEntityNone::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer.write(m_entityID);
  }
  
  void MGE_SetDynamicEntityNone::unserialize(DBuffer &Buffer) {
    Buffer.read(m_entityID);
  }

  GameEventType MGE_SetDynamicEntityNone::SgetType() {
    return GAME_EVENT_LUA_CALL_SETDYNAMICENTITYNONE;
  }

  GameEventType MGE_SetDynamicEntityNone::getType() {
    return SgetType();
  }

  //////////////////////////////
  MGE_SetDynamicBlockRotation::MGE_SetDynamicBlockRotation(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_blockID   = "";
    m_fInitAngle = 0.0;
    m_fRadius    = 0.0;
    m_fPeriod    = 0.0;
    m_startTime  = 0;
    m_endTime    = 0;
  }

  MGE_SetDynamicBlockRotation::MGE_SetDynamicBlockRotation(float p_fEventTime,
                                                           std::string p_blockID,
                                                           float p_fInitAngle,
                                                           float p_fRadius,
                                                           float p_fPeriod,
                                                           int   p_startTime,
                                                           int   p_endTime) 
    : MotoGameEvent(p_fEventTime) {
      m_blockID   = p_blockID;
      m_fInitAngle = p_fInitAngle;
      m_fRadius    = p_fRadius;
      m_fPeriod    = p_fPeriod;
      m_startTime  = p_startTime;
      m_endTime    = p_endTime;
    }

  MGE_SetDynamicBlockRotation::~MGE_SetDynamicBlockRotation() {
  } 
  
  void MGE_SetDynamicBlockRotation::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->addDynamicObject(new SDynamicBlockRotation(m_blockID,
							    m_fInitAngle, m_fRadius,
							    m_fPeriod,
							    m_startTime, m_endTime));
  }

  void MGE_SetDynamicBlockRotation::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer.write(m_blockID);
    Buffer << m_fInitAngle;
    Buffer << m_fRadius;
    Buffer << m_fPeriod;
    Buffer << m_startTime;
    Buffer << m_endTime;
  }
  
  void MGE_SetDynamicBlockRotation::unserialize(DBuffer &Buffer) {
    Buffer.read(m_blockID);
    Buffer >> m_fInitAngle;
    Buffer >> m_fRadius;
    Buffer >> m_fPeriod;
    Buffer >> m_startTime;
    Buffer >> m_endTime;
  }

  GameEventType MGE_SetDynamicBlockRotation::SgetType() {
    return GAME_EVENT_LUA_CALL_SETDYNAMICBLOCKROTATION;
  }

  GameEventType MGE_SetDynamicBlockRotation::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_SetDynamicBlockTranslation::MGE_SetDynamicBlockTranslation(float p_fEventTime) 
    : MotoGameEvent(p_fEventTime) {
      m_blockID   = "";
      m_x = 0.0;
      m_y = 0.0;
      m_fPeriod    = 0.0;
      m_startTime  = 0;
      m_endTime    = 0;
    }

  MGE_SetDynamicBlockTranslation::MGE_SetDynamicBlockTranslation(float p_fEventTime,  
                                                                 std::string p_blockID,
                                                                 float p_x,
                                                                 float p_y,
                                                                 float p_fPeriod,
                                                                 int   p_startTime,
                                                                 int   p_endTime) 
    : MotoGameEvent(p_fEventTime) {
      m_blockID   = p_blockID;
      m_x = p_x;
      m_y = p_y;
      m_fPeriod    = p_fPeriod;
      m_startTime  = p_startTime;
      m_endTime    = p_endTime;
    }

  MGE_SetDynamicBlockTranslation::~MGE_SetDynamicBlockTranslation() {
  } 
  
  void MGE_SetDynamicBlockTranslation::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->addDynamicObject(new SDynamicBlockTranslation(m_blockID,
							       m_x, m_y,
							       m_fPeriod,
							       m_startTime, m_endTime));
  }

  void MGE_SetDynamicBlockTranslation::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer.write(m_blockID);
    Buffer << m_x;
    Buffer << m_y;
    Buffer << m_fPeriod;
    Buffer << m_startTime;
    Buffer << m_endTime;
  }
  
  void MGE_SetDynamicBlockTranslation::unserialize(DBuffer &Buffer) {
    Buffer.read(m_blockID);
    Buffer >> m_x;
    Buffer >> m_y;
    Buffer >> m_fPeriod;
    Buffer >> m_startTime;
    Buffer >> m_endTime;
  }

  GameEventType MGE_SetDynamicBlockTranslation::SgetType() {
    return GAME_EVENT_LUA_CALL_SETDYNAMICBLOCKTRANSLATION;
  }

  GameEventType MGE_SetDynamicBlockTranslation::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_SetDynamicBlockNone::MGE_SetDynamicBlockNone(float p_fEventTime) 
    : MotoGameEvent(p_fEventTime) {
      m_blockID = "";
    }

  MGE_SetDynamicBlockNone::MGE_SetDynamicBlockNone(float p_fEventTime, std::string p_blockID) 
    : MotoGameEvent(p_fEventTime) {
      m_blockID = p_blockID;
    }

  MGE_SetDynamicBlockNone::~MGE_SetDynamicBlockNone() {
  } 
  
  void MGE_SetDynamicBlockNone::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->removeSDynamicOfObject(m_blockID);
  }

  void MGE_SetDynamicBlockNone::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer.write(m_blockID);
  }
  
  void MGE_SetDynamicBlockNone::unserialize(DBuffer &Buffer) {
    Buffer.read(m_blockID);
  }

  GameEventType MGE_SetDynamicBlockNone::SgetType() {
    return GAME_EVENT_LUA_CALL_SETDYNAMICBLOCKNONE;
  }

  GameEventType MGE_SetDynamicBlockNone::getType() {
    return SgetType();
  }

  //////////////////////////////
  MGE_CameraMove::MGE_CameraMove(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_moveX = 0.0;
    m_moveY = 0.0;
  }

  MGE_CameraMove::MGE_CameraMove(float p_fEventTime, float p_moveX, float p_moveY)
  : MotoGameEvent(p_fEventTime) {
    m_moveX = p_moveX;
    m_moveY = p_moveY;
  }

  MGE_CameraMove::~MGE_CameraMove() {
  } 
  
  void MGE_CameraMove::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->CameraMove(m_moveX, m_moveY);
  }

  void MGE_CameraMove::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer << m_moveX;
    Buffer << m_moveY;
  }
  
  void MGE_CameraMove::unserialize(DBuffer &Buffer) {
    Buffer >> m_moveX;
    Buffer >> m_moveY;
  }

  GameEventType MGE_CameraMove::SgetType() {
    return GAME_EVENT_LUA_CALL_CAMERAMOVE;
  }

  GameEventType MGE_CameraMove::getType() {
    return SgetType();
  }


  //////////////////////////////
  MGE_CameraZoom::MGE_CameraZoom(float p_fEventTime) 
  : MotoGameEvent(p_fEventTime) {
    m_zoom = 0.0;
  }

  MGE_CameraZoom::MGE_CameraZoom(float p_fEventTime, float p_zoom) 
    : MotoGameEvent(p_fEventTime) {
      m_zoom = p_zoom;
    }

  MGE_CameraZoom::~MGE_CameraZoom() {
  } 
  
  void MGE_CameraZoom::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->CameraZoom(m_zoom);
  }

  void MGE_CameraZoom::serialize(DBuffer &Buffer) {
    MotoGameEvent::serialize(Buffer);
    Buffer << m_zoom;
  }
  
  void MGE_CameraZoom::unserialize(DBuffer &Buffer) {
    Buffer >> m_zoom;
  }

  GameEventType MGE_CameraZoom::SgetType() {
    return GAME_EVENT_LUA_CALL_CAMERAZOOM;
  }

  GameEventType MGE_CameraZoom::getType() {
    return SgetType();
  }

  //////////////////////////////

  MGE_PenalityTime::MGE_PenalityTime(float p_fEventTime)
  : MotoGameEvent(p_fEventTime) {
    m_penalityTime = 0.0;
  }

  MGE_PenalityTime::MGE_PenalityTime(float p_fEventTime, float p_penatityTime) 
    : MotoGameEvent(p_fEventTime) {
      m_penalityTime = p_penatityTime;
  }

  MGE_PenalityTime::~MGE_PenalityTime() {
  }

  void MGE_PenalityTime::doAction(MotoGame *p_pMotoGame) {
    p_pMotoGame->addPenalityTime(m_penalityTime);
  }

  void MGE_PenalityTime::serialize(DBuffer &Buffer) {
    Buffer << m_penalityTime;
  }

  void MGE_PenalityTime::unserialize(DBuffer &Buffer) {
    Buffer >> m_penalityTime;
  }
 
  GameEventType MGE_PenalityTime::SgetType() {
    return GAME_EVENT_LUA_CALL_PENALITY_TIME;
  }

  GameEventType MGE_PenalityTime::getType() {
    return SgetType();
  }

}
