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

#ifndef __MOTOGAME_H__
#define __MOTOGAME_H__

#define MOTOGAME_DEFAULT_GAME_MESSAGE_DURATION 5.0

#include "VCommon.h"
#include "VApp.h"
#include "helpers/VMath.h"
#include "xmscene/Level.h"
#include "BSP.h"
#include "DBuffer.h"
#include "Collision.h"
#include "ScriptDynamicObjects.h"
#include "SomersaultCounter.h"
#include "GameEvents.h"

#include "xmscene/BasicSceneStructs.h"
#include "xmscene/BikeController.h"
#include "xmscene/BikeParameters.h"
#include "xmscene/BikeAnchors.h"
#include "xmscene/Bike.h"

/* This is the magic depth factor :)  - tweak to obtain max. stability */
#define DEPTH_FACTOR    2

class Level;

namespace vapp {

  class MotoGameEvent;
  class Replay;
  class GameRenderer;

  /*===========================================================================
  Serialized bike state
  ===========================================================================*/
  #define SER_BIKE_STATE_DIR_LEFT         0x01
  #define SER_BIKE_STATE_DIR_RIGHT        0x02
  
  /* IMPORTANT: This structure must be kept as is, otherwise replays will
                be broken! */
  struct SerializedBikeState {
    unsigned char cFlags;             /* State flags */
    float fGameTime;                  /* Game time */      
    float fFrameX,fFrameY;            /* Frame position */
    float fMaxXDiff,fMaxYDiff;        /* Addressing space around the frame */

    unsigned short nRearWheelRot;     /* Encoded rear wheel matrix */
    unsigned short nFrontWheelRot;    /* Encoded front wheel matrix */
    unsigned short nFrameRot;         /* Encoded frame matrix */
        
    unsigned char cBikeEngineRPM;     /* Maps to a float between 400 and 5000 */
    
    signed char cRearWheelX,cRearWheelY;     /* Rear wheel position */
    signed char cFrontWheelX,cFrontWheelY;   /* Front wheel position */
    signed char cElbowX,cElbowY;             /* Elbow position */
    signed char cShoulderX,cShoulderY;       /* Shoulder position */
    signed char cLowerBodyX,cLowerBodyY;     /* Ass position */
    signed char cKneeX,cKneeY;               /* Knee position */
  };
  
  /*===========================================================================
  Arrow pointer
  ===========================================================================*/
  struct ArrowPointer {
    ArrowPointer() {
      nArrowPointerMode = 0;
    }
  
    int nArrowPointerMode;
    Vector2f ArrowPointerPos;
    float fArrowPointerAngle;
  };

  /*===========================================================================
  Requested player state
  ===========================================================================*/
  struct GameReqPlayerPos {
    Vector2f Pos;
    bool bDriveRight;  
  };

  /*===========================================================================
  Game message
  ===========================================================================*/
  struct GameMessage {
    bool bNew;                        /* Uninitialized */
    Vector2f Pos,Vel;                 /* Screen position/velocity of message */
    float fRemoveTime;                /* The time when it should be removed */
    std::string Text;                 /* The text */
    int nAlpha;                       /* Alpha amount */
    bool bOnce;                       /* Unique message */
  };  
    
  struct RecordedGameEvent {
    MotoGameEvent *Event;           /* Event itself */
    bool bPassed;                   /* Whether we have passed it */
  };

  /*===========================================================================
  Game object
  ===========================================================================*/
  class MotoGame {
  public:          
    MotoGame();
    ~MotoGame();
    
    /* update of the structure */
    void prePlayLevel(
#if defined(ALLOW_GHOST)    
          Replay *m_pGhostReplay,
#endif
          Level *pLevelSrc,
          Replay *recordingReplay,
          bool bIsAReplay);

    void playLevel(
#if defined(ALLOW_GHOST)    
       Replay *m_pGhostReplay,
#endif
       Level *pLevelSrc, bool bIsAReplay);
    void updateLevel(float fTimeStep,SerializedBikeState *pReplayState,Replay *p_replay);
    void endLevel(void);

    /* entities */
    void touchEntity(Entity *pEntity, bool bHead); 
    void deleteEntity(Entity *pEntity);

    /* messages */
    void gameMessage(std::string Text,
         bool bOnce = false,
         float fDuration = MOTOGAME_DEFAULT_GAME_MESSAGE_DURATION);
    void clearGameMessages();
    void updateGameMessages();
    std::vector<GameMessage *> &getGameMessage(void) {return m_GameMessages;}
      
    /* serialization */
    void getSerializedBikeState(SerializedBikeState *pState);
    static void unserializeGameEvents(DBuffer *Buffer, std::vector<RecordedGameEvent *> *v_ReplayEvents);
    void interpolateGameState(SerializedBikeState *pA,SerializedBikeState *pB,SerializedBikeState *p,float t);

    /* events */
    void createGameEvent(MotoGameEvent *p_event);
    void destroyGameEvent(MotoGameEvent *p_event);
    MotoGameEvent* getNextGameEvent();
    int getNumPendingGameEvents();
    void cleanEventsQueue();
    void executeEvents(Replay *p_replay);

    /* information about engine */
    float getBikeEngineRPM(void);
    float getBikeEngineSpeed();
      
    /* player */
    void setPlayerPosition(float x,float y,bool bFaceRight);
    const Vector2f &getPlayerPosition(void);
    bool getPlayerFaceDir(void);
    void setDeathAnim(bool b) {m_bDeathAnimEnabled=b;}
    
    /* Direct Lua interaction methods */
    bool scriptCallBool(std::string FuncName,bool bDefault=false);
    void scriptCallVoid(std::string FuncName);
    void scriptCallTblVoid(std::string Table,std::string FuncName);
    void scriptCallVoidNumberArg(std::string FuncName, int n);

    /* Data interface */
    void resetAutoDisabler(void) {m_nStillFrames=0;}
    bool isInitOK(void) {return m_bLevelInitSuccess;}
    bool isFinished(void) {return m_bFinished;}
    bool isDead(void) {return m_bDead;}
    Level *getLevelSrc(void) {return m_pLevelSrc;}
    BikeState *getBikeState(void) {return &m_BikeS;}
    bool isSqueeking(void) {return m_bSqueeking;}
    float howMuchSqueek(void) {return m_fHowMuchSqueek;}

#if defined(ALLOW_GHOST)
      BikeState *getGhostBikeState(void) {return &m_GhostBikeS;}
      bool isGhostActive() {return m_isGhostActive;}
      void setGhostActive(bool s) {m_isGhostActive = s;}
      std::vector<float> m_myLastStrawberries;
      std::vector<float> m_ghostLastStrawberries;
      float m_myDiffOfGhost; /* time diff between the ghost and the player */
#endif

      BikeController *getBikeController(void) {return &m_BikeC;}
      float getTime(void) {return m_fTime;}
      void setTime(float f) {m_fTime=f;}
      float getFinishTime(void) {return m_fFinishTime;}
      ArrowPointer &getArrowPointer(void) {return m_Arrow;}
      bool isWheelSpinning(void) {return m_bWheelSpin;}
      Vector2f getWheelSpinPoint(void) {return m_WheelSpinPoint;}
      Vector2f getWheelSpinDir(void) {return m_WheelSpinDir;}
      CollisionSystem *getCollisionHandler(void) {return &m_Collision;}
      void setGravity(float x,float y) {m_PhysGravity.x=x; m_PhysGravity.y=y; resetAutoDisabler();}
      const Vector2f &getGravity(void) {return m_PhysGravity;}
        
#if defined(ALLOW_GHOST)  
      void UpdateGhostFromReplay(SerializedBikeState *pReplayState);
      float getGhostDiff() {return m_myDiffOfGhost;}
      void setShowGhostTimeDiff(bool b) { m_showGhostTimeDiff = b; }
#endif

      /* action for events */
      void SetEntityPos(String pEntityID, float pX, float pY);
      void PlaceInGameArrow(float pX, float pY, float pAngle);
      void PlaceScreenArrow(float pX, float pY, float pAngle);
      void HideArrow();
      void MoveBlock(String pBlockID, float pX, float pY);
      void SetBlockPos(String pBlockID, float pX, float pY);
      void SetBlockCenter(String pBlockID, float pX, float pY);
      void SetBlockRotation(String pBlockID, float pAngle);

      void setRenderer(GameRenderer *p_renderer);
      void CameraZoom(float pZoom);
      void CameraMove(float p_x, float p_y);

      void killPlayer();
      void playerEntersZone(Zone *pZone);
      void playerLeavesZone(Zone *pZone);
      void playerTouchesEntity(std::string p_entityID, bool p_bTouchedWithHead);
      void entityDestroyed(const std::string& i_entityId);
      void addDynamicObject(SDynamicObject* p_obj);
      void removeSDynamicOfObject(std::string pObject);
      void addPenalityTime(float fTime);

      void createKillEntityEvent(std::string p_entityID);

      unsigned int getNbRemainingStrawberries();
      void makePlayerWin();

      void setBodyDetach(bool state);

      bool isTouching(const Entity& i_entity) const;
      void setTouching(Entity& i_entity, bool i_touching);     
      bool isTouching(const Zone& i_zone) const;
      void setTouching(Zone& i_zone, bool i_isTouching);

  private:         
      /* Data */
      std::queue<MotoGameEvent*> m_GameEventQueue;
      
      float m_fTime,m_fNextAttitudeCon;
      float m_fFinishTime,m_fAttitudeCon;
      
      int m_nStillFrames;
      
      bool m_bSqueeking;
      float m_fHowMuchSqueek;
      bool m_bDeathAnimEnabled;
      bool m_bLevelInitSuccess;

      Vector2f m_PhysGravity; /* gravity */

      ArrowPointer m_Arrow;               /* Arrow */  
      
      CollisionSystem m_Collision;        /* Collision system */
            
      Level *m_pLevelSrc;              /* Source of level */            
      lua_State *m_pL;                    /* Lua state associated with the
                                             level */
      std::vector<Entity *> m_DestroyedEntities; /* destroyed entities */
      dWorldID m_WorldID;                 /* World ID */
      
      std::vector<Entity *> m_DelSchedule;/* Entities scheduled for deletion */
      std::vector<GameMessage *> m_GameMessages;
      
      BikeState m_BikeS;                  /* Bike state */

#if defined(ALLOW_GHOST)  
      BikeState m_GhostBikeS;             /* ghost state */
      bool m_isGhostActive;               /* is ghost active : must it be displayed, ... */
      bool m_showGhostTimeDiff;
#endif

      GameRenderer *m_renderer;

      /* count somersault */
      SomersaultCounter m_somersaultCounter;

      bool bFrontWheelTouching;
      bool bRearWheelTouching;

      std::vector<SDynamicObject*> m_SDynamicObjects;

      BikeController m_BikeC;             /* Bike controller */
      
      bool m_bFinished,m_bDead;           /* Yir */
      
      int m_nLastEventSeq;
      
      bool m_bFirstPhysicsUpdate;

      Vector2f m_PrevRearWheelP;          /* Prev. rear wheel position */
      Vector2f m_PrevFrontWheelP;         /* Prev. front wheel position */
      Vector2f m_PrevHeadP;
      Vector2f m_PrevHead2P;
      Vector2f m_PrevActiveHead;
      
      /* Wheels spinning dirt up... muzakka! :D */
      bool m_bWheelSpin;                  /* Do it captain */
      Vector2f m_WheelSpinPoint,m_WheelSpinDir; /* Where and how much */
      
      /* Data - bike bodies, joints and masses */
      dBodyID m_FrameBodyID;              /* Frame of bike */
      dMass m_FrameMass;              
      dBodyID m_RearWheelBodyID;          /* Rear wheel of bike */
      dMass m_RearWheelMass;
      dBodyID m_FrontWheelBodyID;         /* Front wheel of bike */
      dMass m_FrontWheelMass;    
        
      dBodyID m_PlayerTorsoBodyID;
      dMass m_PlayerTorsoMass;
      dBodyID m_PlayerLArmBodyID;
      dMass m_PlayerLArmMass;
      dBodyID m_PlayerUArmBodyID;
      dMass m_PlayerUArmMass;
      dBodyID m_PlayerLLegBodyID;
      dMass m_PlayerLLegMass;
      dBodyID m_PlayerULegBodyID;
      dMass m_PlayerULegMass;
      dBodyID m_PlayerHandAnchorBodyID;
      dMass m_PlayerHandAnchorMass;
      dBodyID m_PlayerFootAnchorBodyID;
      dMass m_PlayerFootAnchorMass;
      
      dJointID m_FootHingeID;
      dJointID m_KneeHingeID;
      dJointID m_LowerBodyHingeID;
      dJointID m_ShoulderHingeID;
      dJointID m_ElbowHingeID;
      dJointID m_HandHingeID;

      dBodyID m_PlayerTorsoBodyID2;
      dBodyID m_PlayerLArmBodyID2;
      dBodyID m_PlayerUArmBodyID2;
      dBodyID m_PlayerLLegBodyID2;
      dBodyID m_PlayerULegBodyID2;
      dBodyID m_PlayerHandAnchorBodyID2;
      dBodyID m_PlayerFootAnchorBodyID2;
      
      dJointID m_FootHingeID2;
      dJointID m_KneeHingeID2;
      dJointID m_LowerBodyHingeID2;
      dJointID m_ShoulderHingeID2;
      dJointID m_ElbowHingeID2;
      dJointID m_HandHingeID2;
            
      dJointGroupID m_ContactGroup;       /* Contact joint group */     
      
      bool m_bodyDetach;

      /* Teleport next frame? */
      bool m_bTeleport;
      GameReqPlayerPos m_TeleportDest; 
            
      /* for EveryHundreath function */
      float m_lastCallToEveryHundreath;
            
      bool m_isScriptActiv; /* change this variable to activ/desactiv scripting */
      
      std::vector<Entity *> m_entitiesTouching;
      std::vector<Zone *>   m_zonesTouching;

      void clearStates();

      /* Helpers */
      void _GenerateLevel(void);          /* Called by playLevel() to 
                                             prepare the level */
      int _IntersectWheelLevel(Vector2f Cp,float Cr,dContact *pContacts);
      int _IntersectWheelLine(Vector2f Cp,float Cr,int nNumContacts,dContact *pContacts,Vector2f A0,Vector2f A1);
      bool _IntersectHeadLevel(Vector2f Cp,float Cr,const Vector2f &LastCp);
      bool _IntersectHeadLine(Vector2f Cp,float Cr,Vector2f A0,Vector2f A1);
      bool _DoCircleTouchZone(const Vector2f &Cp,float Cr,Zone *pZone);
      bool _IntersectPointLevel(Vector2f Cp);
      Entity *_SpawnEntity(std::string ID,EntitySpeciality Type,Vector2f Pos, Entity *pSrc);
      void _KillEntity(Entity *pEnt);
      void _UpdateEntities(void);
      void _UpdateZones(void);
      bool touchEntityBodyExceptHead(const BikeState &pBike, const Entity &p_entity);
      void _UpdateGameState(SerializedBikeState *pReplayState);
      /* static */ void _UpdateStateFromReplay(SerializedBikeState *pReplayState,BikeState *pBikeS);

#if defined(ALLOW_GHOST)
      void UpdateDiffFromGhost();
      void DisplayDiffFromGhost();
      void InitGhostLastStrawberries(Replay *p_ghostReplay);
#endif

      void cleanScriptDynamicObjects();
      void nextStateScriptDynamicObjects();

      signed char _MapCoordTo8Bits(float fRef,float fMaxDiff,float fCoord);
      float _Map8BitsToCoord(float fRef,float fMaxDiff,signed char c);
      unsigned short _MatrixTo16Bits(const float *pfMatrix);
      void _16BitsToMatrix(unsigned short n16,float *pfMatrix);
      void _SerializeGameEventQueue(DBuffer &Buffer,MotoGameEvent *pEvent);
      
      void _UpdateReplayEvents(Replay *p_replay);
      void _HandleReplayEvent(MotoGameEvent *pEvent);
      
      void _UpdateDynamicCollisionLines(void);
      
      /* MPhysics.cpp */
      void _UpdatePhysics(float fTimeStep);
      void _InitPhysics(void);
      void _UninitPhysics(void);
      void _PrepareBikePhysics(Vector2f StartPos);
      void _PrepareRider(Vector2f StartPos);
    };

}

#endif
