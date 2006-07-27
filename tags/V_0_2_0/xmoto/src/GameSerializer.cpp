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

/* 
 *  Game state serialization and magic
 */
#include "PhysSettings.h"
#include "MotoGame.h"

namespace vapp {

  /*===========================================================================
    Game state interpolation for smoother replays 
    ===========================================================================*/
  void MotoGame::interpolateGameState(SerializedBikeState *pA,SerializedBikeState *pB,SerializedBikeState *p,float t) {
    /* First of all inherit everything from A */
    memcpy(p,pA,sizeof(SerializedBikeState));
    
    /* Interpolate away! The frame is the most important... */
    p->fFrameX = pA->fFrameX + (pB->fFrameX - pA->fFrameX)*t;
    p->fFrameY = pA->fFrameY + (pB->fFrameY - pA->fFrameY)*t;
    
    p->fGameTime = pA->fGameTime + (pB->fGameTime - pA->fGameTime)*t;
  }
  
  /*===========================================================================
    Decoding of event stream
    ===========================================================================*/
  void MotoGame::unserializeGameEvents(DBuffer *Buffer, std::vector<RecordedGameEvent *> *v_ReplayEvents) {
    /* Continue until buffer is empty */
    bool bError = false;
    while((*Buffer).numRemainingBytes() > sizeof(float) && !bError) {
      /* Get event time */
      float fEventTime;
      GameEvent Event;

      (*Buffer) >> fEventTime;
      
      /* Get event type */
      GameEventType EventType;
      (*Buffer) >> EventType;
      
      bool bIsOk = false;

      /* What now depends on event type */
      switch(EventType) {
      case GAME_EVENT_ENTITY_DESTROYED:       
	/* Read entity name */
	int n;
	(*Buffer) >> n;
	if(n >= sizeof(Event.u.EntityDestroyed.cEntityID)) {
	  Log("** Warning ** : Entity name in replay too long, ignoring all events!");
	  bError = true;
	}
	else {
	  (*Buffer).readBuf(Event.u.EntityDestroyed.cEntityID,n);
	  Event.u.EntityDestroyed.cEntityID[n] = '\0';
            
	  /* Read entity type */
	  (*Buffer) >> Event.u.EntityDestroyed.Type;
            
	  /* Read size and pos */
	  (*Buffer) >> Event.u.EntityDestroyed.fSize;
	  (*Buffer) >> Event.u.EntityDestroyed.fPosX;
	  (*Buffer) >> Event.u.EntityDestroyed.fPosY;            
            
	  bIsOk = true;
	}
	break;

      case GAME_EVENT_LUA_CALL_SETENTITYPOS:
	{
	  int n;
	  (*Buffer) >> n;
	  if(n >= sizeof(Event.u.LuaCallSetentitypos.cEntityID)) {
	    Log("** Warning ** : Entity name in replay too long, ignoring all events!");
	    bError = true;
	  }
	  else {
	    (*Buffer).readBuf(Event.u.LuaCallSetentitypos.cEntityID,n);
	    Event.u.LuaCallSetentitypos.cEntityID[n] = '\0';

	    (*Buffer) >> Event.u.LuaCallSetentitypos.x;
	    (*Buffer) >> Event.u.LuaCallSetentitypos.y;

	    bIsOk = true;
	  }
	}
	break;

      case GAME_EVENT_LUA_CALL_CLEARMESSAGES:
	{
	  bIsOk = true;
	}
	break;
	
      case GAME_EVENT_LUA_CALL_PLACEINGAMEARROW:
	{
	  (*Buffer) >> Event.u.LuaCallPlaceingamearrow.x;
	  (*Buffer) >> Event.u.LuaCallPlaceingamearrow.y;
	  (*Buffer) >> Event.u.LuaCallPlaceingamearrow.angle;
	  bIsOk = true;
	}
	break;
	
      case GAME_EVENT_LUA_CALL_PLACESCREENARROW:
	{
	  (*Buffer) >> Event.u.LuaCallPlacescreenarrow.x;
	  (*Buffer) >> Event.u.LuaCallPlacescreenarrow.y;
	  (*Buffer) >> Event.u.LuaCallPlacescreenarrow.angle;
	  bIsOk = true;
	}
	break;
	
      case GAME_EVENT_LUA_CALL_HIDEARROW:
	{
	  bIsOk = true;
	}
	break;
	
      case GAME_EVENT_LUA_CALL_MESSAGE:
	{
	  int n;
	  (*Buffer) >> n;
	  if(n >= sizeof(Event.u.LuaCallMessage.cMessage)) {
	    Log("** Warning ** : Message in replay too long, ignoring all events!");
	    bError = true;
	  }
	  else {
	    (*Buffer).readBuf(Event.u.LuaCallMessage.cMessage, n);
	    Event.u.LuaCallMessage.cMessage[n] = '\0';
	    bIsOk = true;
	  }
	}
	break;
	
      case GAME_EVENT_LUA_CALL_MOVEBLOCK:
	{
	  int n;
	  (*Buffer) >> n;
	  if(n >= sizeof(Event.u.LuaCallMoveblock.cBlockID)) {
	    Log("** Warning ** : Block name in replay too long, ignoring all events!");
	    bError = true;
	  }
	  else {
	    (*Buffer).readBuf(Event.u.LuaCallMoveblock.cBlockID,n);
	    Event.u.LuaCallMoveblock.cBlockID[n] = '\0';

	    (*Buffer) >> Event.u.LuaCallMoveblock.x;
	    (*Buffer) >> Event.u.LuaCallMoveblock.y;

	    bIsOk = true;
	  }
	}
	break;
	
      case GAME_EVENT_LUA_CALL_SETBLOCKPOS:
	{
	  int n;
	  (*Buffer) >> n;
	  if(n >= sizeof(Event.u.LuaCallSetblockpos.cBlockID)) {
	    Log("** Warning ** : Block name in replay too long, ignoring all events!");
	    bError = true;
	  }
	  else {
	    (*Buffer).readBuf(Event.u.LuaCallSetblockpos.cBlockID,n);
	    Event.u.LuaCallSetblockpos.cBlockID[n] = '\0';

	    (*Buffer) >> Event.u.LuaCallSetblockpos.x;
	    (*Buffer) >> Event.u.LuaCallSetblockpos.y;

	    bIsOk = true;
	  }
	}
	break;
	
      case GAME_EVENT_LUA_CALL_SETBLOCKCENTER:
	{
	  int n;
	  (*Buffer) >> n;
	  if(n >= sizeof(Event.u.LuaCallSetBlockCenter.cBlockID)) {
	    Log("** Warning ** : Block name in replay too long, ignoring all events!");
	    bError = true;
	  }
	  else {
	    (*Buffer).readBuf(Event.u.LuaCallSetBlockCenter.cBlockID,n);
	    Event.u.LuaCallSetBlockCenter.cBlockID[n] = '\0';

	    (*Buffer) >> Event.u.LuaCallSetBlockCenter.x;
	    (*Buffer) >> Event.u.LuaCallSetBlockCenter.y;

	    bIsOk = true;
	  }
	}
	break;
	
      case GAME_EVENT_LUA_CALL_SETGRAVITY:
	{
	  (*Buffer) >> Event.u.LuaCallSetgravity.x;
	  (*Buffer) >> Event.u.LuaCallSetgravity.y;
	  bIsOk = true;
	}
	break;
	
      case GAME_EVENT_LUA_CALL_SETPLAYERPOSITION:
	{
	  (*Buffer) >> Event.u.LuaCallSetplayerposition.x;
	  (*Buffer) >> Event.u.LuaCallSetplayerposition.y;
	  (*Buffer) >> Event.u.LuaCallSetplayerposition.bRight;
	  bIsOk = true;
	}
	break;
	
    case GAME_EVENT_LUA_CALL_SETDYNAMICENTITYROTATION:
      {
	  int n;
	  (*Buffer) >> n;
	  if(n >= sizeof(Event.u.LuaCallSetDynamicEntityRotation.cEntityID)) {
	    Log("** Warning ** : Entity name in replay too long, ignoring all events!");
	    bError = true;
	  }
	  else {
	    (*Buffer).readBuf(Event.u.LuaCallSetDynamicEntityRotation.cEntityID,n);
	    Event.u.LuaCallSetDynamicEntityRotation.cEntityID[n] = '\0';

	    (*Buffer) >> Event.u.LuaCallSetDynamicEntityRotation.fInitAngle;
	    (*Buffer) >> Event.u.LuaCallSetDynamicEntityRotation.fRadius;
	    (*Buffer) >> Event.u.LuaCallSetDynamicEntityRotation.fPeriod;
	    (*Buffer) >> Event.u.LuaCallSetDynamicEntityRotation.startTime;
	    (*Buffer) >> Event.u.LuaCallSetDynamicEntityRotation.endTime;

	    bIsOk = true;
	  }
      }
      break;

    case GAME_EVENT_LUA_CALL_SETDYNAMICENTITYTRANSLATION:
      {
	  int n;
	  (*Buffer) >> n;
	  if(n >= sizeof(Event.u.LuaCallSetDynamicEntityTranslation.cEntityID)) {
	    Log("** Warning ** : Entity name in replay too long, ignoring all events!");
	    bError = true;
	  }
	  else {
	    (*Buffer).readBuf(Event.u.LuaCallSetDynamicEntityTranslation.cEntityID,n);
	    Event.u.LuaCallSetDynamicEntityTranslation.cEntityID[n] = '\0';

	    (*Buffer) >> Event.u.LuaCallSetDynamicEntityTranslation.fX;
	    (*Buffer) >> Event.u.LuaCallSetDynamicEntityTranslation.fY;
	    (*Buffer) >> Event.u.LuaCallSetDynamicEntityTranslation.fPeriod;
	    (*Buffer) >> Event.u.LuaCallSetDynamicEntityTranslation.startTime;
	    (*Buffer) >> Event.u.LuaCallSetDynamicEntityTranslation.endTime;

	    bIsOk = true;
	  }
      }
      break;

    case GAME_EVENT_LUA_CALL_SETDYNAMICENTITYNONE:
      {
	  int n;
	  (*Buffer) >> n;
	  if(n >= sizeof(Event.u.LuaCallSetDynamicEntityNone.cEntityID)) {
	    Log("** Warning ** : Entity name in replay too long, ignoring all events!");
	    bError = true;
	  }
	  else {
	    (*Buffer).readBuf(Event.u.LuaCallSetDynamicEntityNone.cEntityID,n);
	    Event.u.LuaCallSetDynamicEntityNone.cEntityID[n] = '\0';

	    bIsOk = true;
	  }
      }
      break;

    case GAME_EVENT_LUA_CALL_SETDYNAMICBLOCKROTATION:
      {
	  int n;
	  (*Buffer) >> n;
	  if(n >= sizeof(Event.u.LuaCallSetDynamicBlockRotation.cBlockID)) {
	    Log("** Warning ** : Block name in replay too long, ignoring all events!");
	    bError = true;
	  }
	  else {
	    (*Buffer).readBuf(Event.u.LuaCallSetDynamicBlockRotation.cBlockID,n);
	    Event.u.LuaCallSetDynamicBlockRotation.cBlockID[n] = '\0';

	    (*Buffer) >> Event.u.LuaCallSetDynamicBlockRotation.fInitAngle;
	    (*Buffer) >> Event.u.LuaCallSetDynamicBlockRotation.fRadius;
	    (*Buffer) >> Event.u.LuaCallSetDynamicBlockRotation.fPeriod;
	    (*Buffer) >> Event.u.LuaCallSetDynamicBlockRotation.startTime;
	    (*Buffer) >> Event.u.LuaCallSetDynamicBlockRotation.endTime;

	    bIsOk = true;
	  }
      }
      break;

    case GAME_EVENT_LUA_CALL_SETDYNAMICBLOCKTRANSLATION:
      {
	  int n;
	  (*Buffer) >> n;
	  if(n >= sizeof(Event.u.LuaCallSetDynamicBlockTranslation.cBlockID)) {
	    Log("** Warning ** : Block name in replay too long, ignoring all events!");
	    bError = true;
	  }
	  else {
	    (*Buffer).readBuf(Event.u.LuaCallSetDynamicBlockTranslation.cBlockID,n);
	    Event.u.LuaCallSetDynamicBlockTranslation.cBlockID[n] = '\0';

	    (*Buffer) >> Event.u.LuaCallSetDynamicBlockTranslation.fX;
	    (*Buffer) >> Event.u.LuaCallSetDynamicBlockTranslation.fY;
	    (*Buffer) >> Event.u.LuaCallSetDynamicBlockTranslation.fPeriod;
	    (*Buffer) >> Event.u.LuaCallSetDynamicBlockTranslation.startTime;
	    (*Buffer) >> Event.u.LuaCallSetDynamicBlockTranslation.endTime;

	    bIsOk = true;
	  }
      }
      break;

    case GAME_EVENT_LUA_CALL_SETDYNAMICBLOCKNONE:
      {
	  int n;
	  (*Buffer) >> n;
	  if(n >= sizeof(Event.u.LuaCallSetDynamicBlockNone.cBlockID)) {
	    Log("** Warning ** : Block name in replay too long, ignoring all events!");
	    bError = true;
	  }
	  else {
	    (*Buffer).readBuf(Event.u.LuaCallSetDynamicBlockNone.cBlockID,n);
	    Event.u.LuaCallSetDynamicBlockNone.cBlockID[n] = '\0';

	    bIsOk = true;
	  }
      }
      break;

    case GAME_EVENT_LUA_CALL_CAMERAZOOM:
      {
	(*Buffer) >> Event.u.LuaCallCameraZoom.fZoom;
	bIsOk = true;
      }
      break;

    case GAME_EVENT_LUA_CALL_CAMERAMOVE:
      {
	(*Buffer) >> Event.u.LuaCallCameraMove.x;
	(*Buffer) >> Event.u.LuaCallCameraMove.y;
	bIsOk = true;
      }
      break;

      default:
	Log("** Warning ** : Failed to parse game events in replay, it will probably not play right!");
	bError = true;
	break;
      }

      if(bIsOk) {
	/* Seems ok, add it */
	RecordedGameEvent *p = new RecordedGameEvent;
	p->fTime = fEventTime;
	p->bPassed = false;
	p->Event.Type = EventType;
	p->Event.nSeq = 0;
	memcpy(&p->Event.u,&Event.u,sizeof(Event.u));
	v_ReplayEvents->push_back(p);
      }

    }
  }

  /*===========================================================================
    Encoding of event buffer 
    ===========================================================================*/
  void MotoGame::_SerializeGameEventQueue(DBuffer &Buffer,GameEvent *pEvent) {
    /* Note how we couldn't care less about most of the game events */    
    switch(pEvent->Type) {
    case GAME_EVENT_ENTITY_DESTROYED:
      {          
	int i;
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << (i=strlen(pEvent->u.EntityDestroyed.cEntityID));
	Buffer.writeBuf(pEvent->u.EntityDestroyed.cEntityID,i);
	Buffer << pEvent->u.EntityDestroyed.Type;
	Buffer << pEvent->u.EntityDestroyed.fSize;
	Buffer << pEvent->u.EntityDestroyed.fPosX;
	Buffer << pEvent->u.EntityDestroyed.fPosY;
      }
      break;

    case GAME_EVENT_LUA_CALL_SETENTITYPOS:
      {
	int i;
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << (i=strlen(pEvent->u.LuaCallSetentitypos.cEntityID));
	Buffer.writeBuf(pEvent->u.LuaCallSetentitypos.cEntityID,i);
	Buffer << pEvent->u.LuaCallSetentitypos.x;
	Buffer << pEvent->u.LuaCallSetentitypos.y;
      }
      break;

    case GAME_EVENT_LUA_CALL_CLEARMESSAGES:
      {
	Buffer << getTime();
	Buffer << pEvent->Type;
      }
      break;
      
    case GAME_EVENT_LUA_CALL_PLACEINGAMEARROW:
      {
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << pEvent->u.LuaCallPlaceingamearrow.x;
	Buffer << pEvent->u.LuaCallPlaceingamearrow.y;
	Buffer << pEvent->u.LuaCallPlaceingamearrow.angle;
      }
      break;
      
    case GAME_EVENT_LUA_CALL_PLACESCREENARROW:
      {
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << pEvent->u.LuaCallPlacescreenarrow.x;
	Buffer << pEvent->u.LuaCallPlacescreenarrow.y;
	Buffer << pEvent->u.LuaCallPlacescreenarrow.angle;
      }
      break;
      
    case GAME_EVENT_LUA_CALL_HIDEARROW:
      {
	Buffer << getTime();
	Buffer << pEvent->Type;
      }
      break;
      
    case GAME_EVENT_LUA_CALL_MESSAGE:
      {
	int i;
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << (i=strlen(pEvent->u.LuaCallMessage.cMessage));
	Buffer.writeBuf(pEvent->u.LuaCallMessage.cMessage,i);
      }
      break;
      
    case GAME_EVENT_LUA_CALL_MOVEBLOCK:
      {
	int i;
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << (i=strlen(pEvent->u.LuaCallMoveblock.cBlockID));
	Buffer.writeBuf(pEvent->u.LuaCallMoveblock.cBlockID,i);
	Buffer << pEvent->u.LuaCallMoveblock.x;
	Buffer << pEvent->u.LuaCallMoveblock.y;
      }
      break;
      
    case GAME_EVENT_LUA_CALL_SETBLOCKPOS:
      {
	int i;
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << (i=strlen(pEvent->u.LuaCallSetblockpos.cBlockID));
	Buffer.writeBuf(pEvent->u.LuaCallSetblockpos.cBlockID,i);
	Buffer << pEvent->u.LuaCallSetblockpos.x;
	Buffer << pEvent->u.LuaCallSetblockpos.y;
      }
      break;
      
    case GAME_EVENT_LUA_CALL_SETBLOCKCENTER:
      {
	int i;
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << (i=strlen(pEvent->u.LuaCallSetBlockCenter.cBlockID));
	Buffer.writeBuf(pEvent->u.LuaCallSetBlockCenter.cBlockID,i);
	Buffer << pEvent->u.LuaCallSetBlockCenter.x;
	Buffer << pEvent->u.LuaCallSetBlockCenter.y;
      }
      break;
      
    case GAME_EVENT_LUA_CALL_SETGRAVITY:
      {
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << pEvent->u.LuaCallSetgravity.x;
	Buffer << pEvent->u.LuaCallSetgravity.y;
      }
      break;
      
    case GAME_EVENT_LUA_CALL_SETPLAYERPOSITION:
      {
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << pEvent->u.LuaCallSetplayerposition.x;
	Buffer << pEvent->u.LuaCallSetplayerposition.y;
	Buffer << pEvent->u.LuaCallSetplayerposition.bRight;
      }
      break;

    case GAME_EVENT_LUA_CALL_SETDYNAMICENTITYROTATION:
      {
	int i;
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << (i=strlen(pEvent->u.LuaCallSetDynamicEntityRotation.cEntityID));
	Buffer.writeBuf(pEvent->u.LuaCallSetDynamicEntityRotation.cEntityID,i);
	Buffer << pEvent->u.LuaCallSetDynamicEntityRotation.fInitAngle;
	Buffer << pEvent->u.LuaCallSetDynamicEntityRotation.fRadius;
	Buffer << pEvent->u.LuaCallSetDynamicEntityRotation.fPeriod;
	Buffer << pEvent->u.LuaCallSetDynamicEntityRotation.startTime;
	Buffer << pEvent->u.LuaCallSetDynamicEntityRotation.endTime;
      }
      break;

    case GAME_EVENT_LUA_CALL_SETDYNAMICENTITYTRANSLATION:
      {
	int i;
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << (i=strlen(pEvent->u.LuaCallSetDynamicEntityTranslation.cEntityID));
	Buffer.writeBuf(pEvent->u.LuaCallSetDynamicEntityTranslation.cEntityID,i);
	Buffer << pEvent->u.LuaCallSetDynamicEntityTranslation.fX;
	Buffer << pEvent->u.LuaCallSetDynamicEntityTranslation.fY;
	Buffer << pEvent->u.LuaCallSetDynamicEntityTranslation.fPeriod;
	Buffer << pEvent->u.LuaCallSetDynamicEntityTranslation.startTime;
	Buffer << pEvent->u.LuaCallSetDynamicEntityTranslation.endTime;
      }
      break;

    case GAME_EVENT_LUA_CALL_SETDYNAMICENTITYNONE:
      {
	int i;
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << (i=strlen(pEvent->u.LuaCallSetDynamicEntityNone.cEntityID));
	Buffer.writeBuf(pEvent->u.LuaCallSetDynamicEntityNone.cEntityID,i);
      }
      break;

    case GAME_EVENT_LUA_CALL_SETDYNAMICBLOCKROTATION:
      {
	int i;
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << (i=strlen(pEvent->u.LuaCallSetDynamicBlockRotation.cBlockID));
	Buffer.writeBuf(pEvent->u.LuaCallSetDynamicBlockRotation.cBlockID,i);
	Buffer << pEvent->u.LuaCallSetDynamicBlockRotation.fInitAngle;
	Buffer << pEvent->u.LuaCallSetDynamicBlockRotation.fRadius;
	Buffer << pEvent->u.LuaCallSetDynamicBlockRotation.fPeriod;
	Buffer << pEvent->u.LuaCallSetDynamicBlockRotation.startTime;
	Buffer << pEvent->u.LuaCallSetDynamicBlockRotation.endTime;
      }
      break;

    case GAME_EVENT_LUA_CALL_SETDYNAMICBLOCKTRANSLATION:
      {
	int i;
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << (i=strlen(pEvent->u.LuaCallSetDynamicBlockTranslation.cBlockID));
	Buffer.writeBuf(pEvent->u.LuaCallSetDynamicBlockTranslation.cBlockID,i);
	Buffer << pEvent->u.LuaCallSetDynamicBlockTranslation.fX;
	Buffer << pEvent->u.LuaCallSetDynamicBlockTranslation.fY;
	Buffer << pEvent->u.LuaCallSetDynamicBlockTranslation.fPeriod;
	Buffer << pEvent->u.LuaCallSetDynamicBlockTranslation.startTime;
	Buffer << pEvent->u.LuaCallSetDynamicBlockTranslation.endTime;
      }
      break;

    case GAME_EVENT_LUA_CALL_SETDYNAMICBLOCKNONE:
      {
	int i;
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << (i=strlen(pEvent->u.LuaCallSetDynamicBlockNone.cBlockID));
	Buffer.writeBuf(pEvent->u.LuaCallSetDynamicBlockNone.cBlockID,i);
      }
      break;

    case GAME_EVENT_LUA_CALL_CAMERAZOOM:
      {
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << pEvent->u.LuaCallCameraZoom.fZoom;
      }
      break;

    case GAME_EVENT_LUA_CALL_CAMERAMOVE:
      {
	Buffer << getTime();
	Buffer << pEvent->Type;
	Buffer << pEvent->u.LuaCallCameraMove.x;
	Buffer << pEvent->u.LuaCallCameraMove.y;
      }
      break;
    }            
  }

  /*===========================================================================
    Matrix encodings
    ===========================================================================*/
  unsigned short MotoGame::_MatrixTo16Bits(const float *pfMatrix) {
    /* The idea is that we only need to store the first column of the matrix,
       as the second on is simply the first one transposed... Each of the
       two components of the first column is given 8 bits of precision */
    int n1 = (int)(pfMatrix[0] * 127.0f + 127.0f);   
    if(n1<0) n1=0;
    if(n1>255) n1=255;    
    unsigned char c1 = n1;
    int n2 = (int)(pfMatrix[2] * 127.0f + 127.0f);   
    if(n2<0) n2=0;
    if(n2>255) n2=255;
    unsigned char c2 = n2;
    return (unsigned short) ((((unsigned short)c1)<<8)|(unsigned short)c2);
  }
  
  void MotoGame::_16BitsToMatrix(unsigned short n16,float *pfMatrix) {
    /* Convert it back again */
    int n1 = (int)((n16&0xff00)>>8);
    int n2 = (int)(n16&0xff);
    pfMatrix[0] = (((float)n1) - 127.0f) / 127.0f;
    pfMatrix[2] = (((float)n2) - 127.0f) / 127.0f;
    
    /* Make sure the column is normalized */
    float d = sqrt(pfMatrix[0]*pfMatrix[0] + pfMatrix[2]*pfMatrix[2]);
    if(d == 0.0f) {
      /* It's null... */
      pfMatrix[0] = 1.0f; pfMatrix[1] = 0.0f;
      pfMatrix[2] = 0.0f; pfMatrix[3] = 1.0f;
    }
    else {
      pfMatrix[0] /= d;
      pfMatrix[2] /= d;
      
      /* Transpose second column */
      pfMatrix[1] = -pfMatrix[2];
      pfMatrix[3] = pfMatrix[0];
    }
  }

  /*===========================================================================
    Neat trick for converting floating-point numbers to 8 bits
    ===========================================================================*/
  char MotoGame::_MapCoordTo8Bits(float fRef,float fMaxDiff,float fCoord) {
    int n = (int)((127.0f * (fCoord-fRef))/fMaxDiff);
    if(n<-127) n=-127;
    if(n>127) n=127;
    return (char)(n&0xff);
  }
  
  float MotoGame::_Map8BitsToCoord(float fRef,float fMaxDiff,char c) {
    return fRef + (((float)c)/127.0f) * fMaxDiff;
  }

  /*===========================================================================
    Serializer
    ===========================================================================*/
  void MotoGame::getSerializedBikeState(SerializedBikeState *pState) {
    /* Get. */
    pState->fGameTime = m_fTime;
    
    if(m_BikeS.Dir == DD_LEFT) pState->cFlags = SER_BIKE_STATE_DIR_LEFT;
    else if(m_BikeS.Dir == DD_RIGHT) pState->cFlags = SER_BIKE_STATE_DIR_RIGHT;
    
    /* Calculate maximum X-axis difference between frame.x and other coords */
    float fMaxX = 0.0f;
    if( fabs(m_BikeS.CenterP.x - m_BikeS.FrontWheelP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.FrontWheelP.x);
    if( fabs(m_BikeS.CenterP.x - m_BikeS.RearWheelP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.RearWheelP.x);
    if(m_BikeS.Dir == DD_RIGHT) {
      if( fabs(m_BikeS.CenterP.x - m_BikeS.ElbowP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.ElbowP.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.ShoulderP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.ShoulderP.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.LowerBodyP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.LowerBodyP.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.KneeP.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.KneeP.x);
    }
    else if(m_BikeS.Dir == DD_LEFT) {
      if( fabs(m_BikeS.CenterP.x - m_BikeS.Elbow2P.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.Elbow2P.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.Shoulder2P.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.Shoulder2P.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.LowerBody2P.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.LowerBody2P.x);
      if( fabs(m_BikeS.CenterP.x - m_BikeS.Knee2P.x) > fMaxX) fMaxX = fabs(m_BikeS.CenterP.x - m_BikeS.Knee2P.x);
    }
    pState->fMaxXDiff = fMaxX;
    
    /* Calculate maximum Y-axis difference between frame.y and other coords */
    float fMaxY = 0.0f;
    if( fabs(m_BikeS.CenterP.y - m_BikeS.FrontWheelP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.FrontWheelP.y);
    if( fabs(m_BikeS.CenterP.y - m_BikeS.RearWheelP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.RearWheelP.y);
    if(m_BikeS.Dir == DD_RIGHT) {
      if( fabs(m_BikeS.CenterP.y - m_BikeS.ElbowP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.ElbowP.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.ShoulderP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.ShoulderP.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.LowerBodyP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.LowerBodyP.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.KneeP.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.KneeP.y);
    }
    else if(m_BikeS.Dir == DD_LEFT) {
      if( fabs(m_BikeS.CenterP.y - m_BikeS.Elbow2P.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.Elbow2P.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.Shoulder2P.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.Shoulder2P.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.LowerBody2P.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.LowerBody2P.y);
      if( fabs(m_BikeS.CenterP.y - m_BikeS.Knee2P.y) > fMaxY) fMaxY = fabs(m_BikeS.CenterP.y - m_BikeS.Knee2P.y);
    }
    pState->fMaxYDiff = fMaxY;
    
    /* Update engine stuff */    
    int n = (int)(((m_BikeS.fBikeEngineRPM-ENGINE_MIN_RPM)/ENGINE_MAX_RPM)*255.0f);
    if(n<0) n=0;
    if(n>255) n=255;
    pState->cBikeEngineRPM = n;
        
    /* Calculate serialization */
    pState->fFrameX = m_BikeS.CenterP.x;
    pState->fFrameY = m_BikeS.CenterP.y;
        
    pState->cFrontWheelX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.FrontWheelP.x);
    pState->cFrontWheelY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.FrontWheelP.y);
    pState->cRearWheelX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.RearWheelP.x);
    pState->cRearWheelY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.RearWheelP.y);
    
    //memcpy(pState->fFrontWheelRot,m_BikeS.fFrontWheelRot,sizeof(float)*4);
    //memcpy(pState->fRearWheelRot,m_BikeS.fRearWheelRot,sizeof(float)*4);
    //memcpy(pState->fFrameRot,m_BikeS.fFrameRot,sizeof(float)*4);
    
    pState->nFrontWheelRot = _MatrixTo16Bits(m_BikeS.fFrontWheelRot);
    pState->nRearWheelRot = _MatrixTo16Bits(m_BikeS.fRearWheelRot);
    pState->nFrameRot = _MatrixTo16Bits(m_BikeS.fFrameRot);
    
    //printf("[ %f %f \n"
    //       "  %f %f ]\n",pState->fFrameRot[0],pState->fFrameRot[1],pState->fFrameRot[2],pState->fFrameRot[3]);
           
    //unsigned short test = _MatrixTo16Bits(m_BikeS.fFrameRot);
    //float fTest[4];
    //_16BitsToMatrix(test,fTest);
    //
    //printf(" %f %f      %f %f\n"
    //       " %f %f      %f %f\n\n",
    //       m_BikeS.fFrameRot[0],m_BikeS.fFrameRot[1],      fTest[0],fTest[1],
    //       m_BikeS.fFrameRot[2],m_BikeS.fFrameRot[3],      fTest[2],fTest[3]);             
    
    if(m_BikeS.Dir == DD_RIGHT) {
      pState->cElbowX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.ElbowP.x); 
      pState->cElbowY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.ElbowP.y);
      pState->cShoulderX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.ShoulderP.x); 
      pState->cShoulderY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.ShoulderP.y);
      pState->cLowerBodyX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.LowerBodyP.x); 
      pState->cLowerBodyY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.LowerBodyP.y);
      pState->cKneeX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.KneeP.x); 
      pState->cKneeY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.KneeP.y);
    }
    else if(m_BikeS.Dir == DD_LEFT) {
      pState->cElbowX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.Elbow2P.x); 
      pState->cElbowY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.Elbow2P.y);
      pState->cShoulderX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.Shoulder2P.x); 
      pState->cShoulderY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.Shoulder2P.y);
      pState->cLowerBodyX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.LowerBody2P.x); 
      pState->cLowerBodyY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.LowerBody2P.y);
      pState->cKneeX = _MapCoordTo8Bits(pState->fFrameX,pState->fMaxXDiff,m_BikeS.Knee2P.x); 
      pState->cKneeY = _MapCoordTo8Bits(pState->fFrameY,pState->fMaxYDiff,m_BikeS.Knee2P.y);
    }
  }      

};


