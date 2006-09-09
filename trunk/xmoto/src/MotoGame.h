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

namespace vapp {
  /*===========================================================================
    Entity object types
    ===========================================================================*/
  enum EntityType {
    ET_UNASSIGNED,
    ET_SPRITE,
    ET_PLAYERSTART,
    ET_ENDOFLEVEL,
    ET_WRECKER,
    ET_STRAWBERRY,
    ET_PARTICLESOURCE,
    ET_DUMMY
  };
}

#include "VCommon.h"
#include "VApp.h"
#include "VMath.h"
#include "LevelSrc.h"
#include "BSP.h"
#include "DBuffer.h"
#include "Collision.h"
#include "ScriptDynamicObjects.h"
#include "SomersaultCounter.h"
#include "GameEvents.h"

namespace vapp {

  class MotoGameEvent;
  class Replay;
  class GameRenderer;

  /*===========================================================================
  Defines
  ===========================================================================*/
  
  /* This is the magic depth factor :)  - tweak to obtain max. stability */
  #define DEPTH_FACTOR    2

  /*===========================================================================
  Driving directions
  ===========================================================================*/
  enum DriveDir {
    DD_RIGHT,
    DD_LEFT
  };

  /*===========================================================================
  Edge effects
  ===========================================================================*/
  enum EdgeEffect {
    EE_UNASSIGNED,
    EE_GRASS,
    EE_REDBRICKS,
    EE_GRAYBRICKS,
    EE_BLUEBRICKS,
    EE_GRASSALT
  };

  /*===========================================================================
  Dummy helper - a point we'd like to track graphically
  ===========================================================================*/
  struct DummyHelper {
    Vector2f Pos;         /* position */
    float r,g,b;          /* What color? */
  };

  /*===========================================================================
  Controller struct
  ===========================================================================*/
  struct BikeController {
    float fDrive;         /* Throttle [0; 1] or Brake [-1; 0] */
    float fPull;          /* Pull back on the handle bar [0; 1] or push forward on the handle bar [-1; 0] */
    bool bChangeDir;      /* Change direction */
    
    /* Debug-enabled controls */
    bool bDebugDriveBack; 
    bool bDebug1;
  };

  /*===========================================================================
  Convex block vertex
  ===========================================================================*/
  struct ConvexBlockVertex {
    Vector2f P;                                   /* Position of vertex */
    Vector2f T;                                   /* Texture vertex */
  };

  /*===========================================================================
  Convex block
  ===========================================================================*/
  struct ConvexBlock {
    ConvexBlock() {
      pSrcBlock = NULL;
    }
  
    std::vector<ConvexBlockVertex *> Vertices;    /* Vertices */
    LevelBlock *pSrcBlock;                        /* Source block */
  };

  /*===========================================================================
  Dynamic block
  ===========================================================================*/
  struct DynamicBlock {
    DynamicBlock() {
      pSrcBlock = NULL;
      fRotation = 0.0f;
    }
    
    std::vector<ConvexBlock *> ConvexBlocks;      /* Polygons */
    LevelBlock *pSrcBlock;                        /* Source block */
    float fRotation;                              /* Block rotation */    
    Vector2f Position;                            /* Block position */
    bool bBackground;                             /* Background block */
    
    std::vector<Line *> CollisionLines;           /* Line to collide against */
  };

  /*===========================================================================
  Overlay edge (grass, effects, etc)
  ===========================================================================*/
  struct OverlayEdge {
    OverlayEdge() {
      pSrcBlock = NULL;
    }
  
    Vector2f P1,P2;                               /* Point-to-point */
    EdgeEffect Effect;                            /* What? */
    LevelBlock *pSrcBlock;                        /* Source block */
  };

  /*===========================================================================
  Bike params
  ===========================================================================*/
  struct BikeParams {
    /* Geometrical */
    float WR;             /* Wheel radius */
    float Ch;             /* Center of mass height */
    float Wb;             /* Wheel base */
    float RVx,RVy;        /* Position of rear susp. anchor */
    float FVx,FVy;        /* Position of front susp. anchor */

    float PSVx,PSVy;      /* Position of player shoulder */
    float PEVx,PEVy;      /* Position of player elbow */
    float PHVx,PHVy;      /* Position of player hand */
    float PLVx,PLVy;      /* Position of player lower body */
    float PKVx,PKVy;      /* Position of player knee */
    float PFVx,PFVy;      /* Position of player foot */
    
    float fHeadSize;      /* Radius */
    float fNeckLength;    /* Length of neck */
        
    /* Physical */
    float Wm;             /* Wheel mass [kg] */
    float BPm;            /* Player body part mass [kg] */
    float Fm;             /* Frame mass [kg] */
    float IL;             /* Frame "inertia" length [m] */
    float IH;             /* Frame "inertia" height [m] */
        
    /* Braking/engine performance */
    float fMaxBrake,fMaxEngine;
  };

  /*===========================================================================
  Bike anchor points (relative to center of mass)
  ===========================================================================*/
  struct BikeAnchors {
    Vector2f Tp;          /* Point on the ground, exactly between the wheels */
    Vector2f Rp;          /* Center of rear wheel */
    Vector2f Fp;          /* Center of front wheel */
    Vector2f AR;          /* Rear suspension anchor */
    Vector2f AF;          /* Front suspension anchor */
    Vector2f AR2;         /* Rear suspension anchor (Alt.) */
    Vector2f AF2;         /* Front suspension anchor (Alt.) */

    Vector2f PTp;         /* Player torso center */
    Vector2f PULp;        /* Player upper leg center */
    Vector2f PLLp;        /* Player lower leg center */
    Vector2f PUAp;        /* Player upper arm center */
    Vector2f PLAp;        /* Player lower arm center */
    Vector2f PHp;         /* Player hand center */
    Vector2f PFp;         /* Player foot center */

    Vector2f PTp2;        /* Player torso center (Alt.) */
    Vector2f PULp2;       /* Player upper leg center (Alt.) */
    Vector2f PLLp2;       /* Player lower leg center (Alt.) */
    Vector2f PUAp2;       /* Player upper arm center (Alt.) */
    Vector2f PLAp2;       /* Player lower arm center (Alt.) */
    Vector2f PHp2;        /* Player hand center (Alt.) */
    Vector2f PFp2;        /* Player foot center (Alt.) */
  };

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
  Bike state 
  ===========================================================================*/
  struct BikeState {
    DriveDir Dir;         /* Driving left or right? */
  
    float fBikeEngineRPM;
  
    Vector2f RearWheelP;  /* Rear wheel position */
    Vector2f FrontWheelP; /* Front wheel position */
    Vector2f SwingAnchorP;/* Swing arm anchor position */
    Vector2f FrontAnchorP;/* Front suspension anchor position */    
    Vector2f SwingAnchor2P;/* Swing arm anchor position (Alt.) */
    Vector2f FrontAnchor2P;/* Front suspension anchor position (Alt.) */    
    Vector2f CenterP;     /* Center position */

    Vector2f PlayerTorsoP;/* Position of player's torso */
    Vector2f PlayerULegP; /* Position of player's upper leg */
    Vector2f PlayerLLegP; /* Position of player's lower leg */
    Vector2f PlayerUArmP; /* Position of player's upper arm */
    Vector2f PlayerLArmP; /* Position of player's upper arm */

    Vector2f PlayerTorso2P;/* Position of player's torso (Alt.) */
    Vector2f PlayerULeg2P; /* Position of player's upper leg (Alt.) */
    Vector2f PlayerLLeg2P; /* Position of player's lower leg (Alt.) */
    Vector2f PlayerUArm2P; /* Position of player's upper arm (Alt.) */ 
    Vector2f PlayerLArm2P; /* Position of player's upper arm (Alt.) */
        
    /* Internals */
    float fFrontWheelRot[4];
    float fRearWheelRot[4];
    float fFrameRot[4];
    
    Vector2f WantedHandP,WantedFootP;
    Vector2f WantedHand2P,WantedFoot2P;    
    
    Vector2f HandP;
    Vector2f ElbowP;
    Vector2f ShoulderP;
    Vector2f LowerBodyP;
    Vector2f KneeP;
    Vector2f FootP;
    Vector2f HeadP;        /* NB! not a phys. body */

    Vector2f Hand2P;
    Vector2f Elbow2P;
    Vector2f Shoulder2P;
    Vector2f LowerBody2P;
    Vector2f Knee2P;
    Vector2f Foot2P;
    Vector2f Head2P;        /* NB! not a phys. body */

    //dReal *pfPlayerTorsoPos;
    //dReal *pfPlayerTorsoRot;
    //dReal *pfPlayerULegPos;
    //dReal *pfPlayerULegRot;
    //dReal *pfPlayerLLegPos;
    //dReal *pfPlayerLLegRot;
    //dReal *pfPlayerUArmPos;
    //dReal *pfPlayerUArmRot;
    //dReal *pfPlayerLArmPos;
    //dReal *pfPlayerLArmRot;

    //dReal *pfPlayerTorso2Pos;
    //dReal *pfPlayerTorso2Rot;
    //dReal *pfPlayerULeg2Pos;
    //dReal *pfPlayerULeg2Rot;
    //dReal *pfPlayerLLeg2Pos;
    //dReal *pfPlayerLLeg2Rot;
    //dReal *pfPlayerUArm2Pos;
    //dReal *pfPlayerUArm2Rot;
    //dReal *pfPlayerLArm2Pos;
    //dReal *pfPlayerLArm2Rot;

    Vector2f RRearWheelP; /* Relaxed rear wheel position */
    Vector2f RFrontWheelP;/* Relaxed front wheel position */
    Vector2f PrevRq;      /* Previous error (rear) */
    Vector2f PrevFq;      /* Previous error (front) */
    Vector2f PrevPFq;     /* Previous error (player foot) */
    Vector2f PrevPHq;     /* Previous error (player hand) */
    Vector2f PrevPFq2;    /* Previous error (player foot) (Alt.) */
    Vector2f PrevPHq2;    /* Previous error (player hand) (Alt.) */
    
    /* Bonusinfo */
    BikeAnchors *pAnchors;
    
    /* Driving */
    float fCurBrake,fCurEngine;    
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
  Entity object
  ===========================================================================*/
  struct Entity {
    Entity() {
      Type = ET_UNASSIGNED;
      pSrc = NULL;
      fSize = 1.0f;
      fSpriteZ = 1;
      fNextParticleTime = 0;
      bTouched = false;
    }
  
    std::string ID;
    EntityType Type;
    LevelEntity *pSrc;
    bool bTouched;
    
    float fSize;                      /* Size */
    Vector2f Pos;                     /* Position */    
    
    /* ET_SPRITE */
    float fSpriteZ;
    std::string SpriteType;
    
    /* ET_PLAYERSTART */
    
    /* ET_ENDOFLEVEL */
    
    /* ET_WRECKER */
    
    /* ET_STRAWBERRY */
    
    /* ET_PARTICLESOURCE */
    std::string ParticleType;
    float fNextParticleTime;
    
    /* ET_DUMMY */
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
      MotoGame() {m_pLevelSrc=NULL;
                  m_bSqueeking=false;
                  clearStates();
      m_lastCallToEveryHundreath = 0.0;
#if defined(ALLOW_GHOST)
      m_showGhostTimeDiff = true;
      m_isGhostActive = false;
#endif
      m_renderer = NULL;
      m_isScriptActiv = false;

      bFrontWheelTouching = false;
      bRearWheelTouching  = false;
      }
      ~MotoGame() {endLevel();}     
    
      /* Methods */
      void prePlayLevel(
#if defined(ALLOW_GHOST)    
			Replay *m_pGhostReplay,
#endif
			LevelSrc *pLevelSrc,
			Replay *recordingReplay,
			bool bIsAReplay);

      void playLevel(
#if defined(ALLOW_GHOST)    
		     Replay *m_pGhostReplay,
#endif
		     LevelSrc *pLevelSrc, bool bIsAReplay);
      void updateLevel(float fTimeStep,SerializedBikeState *pReplayState,Replay *p_replay);
      void endLevel(void);
      
      void touchEntity(Entity *pEntity,bool bHead); 
      void deleteEntity(Entity *pEntity);
      int countEntitiesByType(EntityType Type);
      Entity *findEntity(const std::string &ID);
      
      void clearStates(void);
      
      void gameMessage(std::string Text,bool bOnce = false, float fDuration = 5.0);
      void clearGameMessages(void);
      void updateGameMessages();
      
      void getSerializedBikeState(SerializedBikeState *pState);
      static void unserializeGameEvents(DBuffer *Buffer, std::vector<RecordedGameEvent *> *v_ReplayEvents);
      void interpolateGameState(SerializedBikeState *pA,SerializedBikeState *pB,SerializedBikeState *p,float t);

      float getBikeEngineRPM(void);
      float getBikeEngineSpeed();

      void createGameEvent(MotoGameEvent *p_event);
      void destroyGameEvent(MotoGameEvent *p_event);
      MotoGameEvent* getNextGameEvent();
      int getNumPendingGameEvents();
      void cleanEventsQueue();
      void executeEvents(Replay *p_replay);      

      void setPlayerPosition(float x,float y,bool bFaceRight);
      const Vector2f &getPlayerPosition(void);
      bool getPlayerFaceDir(void);

      Entity *getEntityByID(const std::string &ID);
      
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
      LevelSrc *getLevelSrc(void) {return m_pLevelSrc;}
      std::vector<ConvexBlock *> &getBlocks(void) {return m_Blocks;}
      BikeState *getBikeState(void) {return &m_BikeS;}
      BikeParams *getBikeParams() { return &m_BikeP;}
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
      std::vector<GameMessage *> &getGameMessage(void) {return m_GameMessages;}
      std::vector<Entity *> &getEntities(void) {return m_Entities;}
      std::vector<DynamicBlock *> &getDynBlocks(void) {return m_DynBlocks;}
      std::vector<OverlayEdge *> &getOverlayEdges(void) {return m_OvEdges;}
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
        
      /* Debug */
      void resetDummies(void) {m_nNumDummies=0;}
      int getNumDummies(void) {return m_nNumDummies;}
      DummyHelper *getDummies(void) {return m_Dummies;}
      void addDummy(Vector2f Pos,float r,float g,float b);
    
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
      DynamicBlock *GetDynamicBlockByID(const std::string &ID);

      void setRenderer(GameRenderer *p_renderer);
      void CameraZoom(float pZoom);
      void CameraMove(float p_x, float p_y);

      void killPlayer();
      void playerEntersZone(LevelZone *pZone);
      void playerLeavesZone(LevelZone *pZone);
      void playerTouchesEntity(std::string p_entityID, bool p_bTouchedWithHead);
      void entityDestroyed(std::string p_entityID, EntityType p_type,
			   float p_fSize, float p_fPosX, float p_fPosY);
      void addDynamicObject(SDynamicObject* p_obj);
      void removeSDynamicOfObject(std::string pObject);

      void revertEntityDestroyed(std::string p_entityID);
      void createKillEntityEvent(std::string p_entityID);

      unsigned int getNbRemainingStrawberries();
      void makePlayerWin();

  private:         
      /* Data */
      std::queue<MotoGameEvent*> m_GameEventQueue;
      
      float m_fTime,m_fNextAttitudeCon;
      float m_fFinishTime,m_fAttitudeCon;
      
      int m_nNumDummies;
      DummyHelper m_Dummies[100];
      
      int m_nStillFrames;
      
      bool m_bSqueeking;
      float m_fHowMuchSqueek;
      bool m_bLevelInitSuccess;

      Vector2f m_PhysGravity; /* gravity */

      ArrowPointer m_Arrow;               /* Arrow */  
      
      CollisionSystem m_Collision;        /* Collision system */
            
      LevelSrc *m_pLevelSrc;              /* Source of level */            
      lua_State *m_pL;                    /* Lua state associated with the
                                             level */
      std::vector<ConvexBlock *> m_Blocks;/* Blocks */
      std::vector<Entity *> m_Entities;   /* Entities */
      std::vector<Entity *> m_DestroyedEntities; /* destroyed entities */
      dWorldID m_WorldID;                 /* World ID */
      
      std::vector<OverlayEdge *> m_OvEdges;/* Overlay edges */
      std::vector<Entity *> m_DelSchedule;/* Entities scheduled for deletion */
      std::vector<GameMessage *> m_GameMessages;
      std::vector<DynamicBlock *> m_DynBlocks; /* Dynamic blocks */
      
      BikeParams m_BikeP;                 /* Bike physics */      
      BikeAnchors m_BikeA;                /* Important bike anchor points */
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
      
      /* Teleport next frame? */
      bool m_bTeleport;
      GameReqPlayerPos m_TeleportDest; 
            
      /* for EveryHundreath function */
      float m_lastCallToEveryHundreath;
            
      bool m_isScriptActiv; /* change this variable to activ/desactiv scripting */

      /* Helpers */
      void _GenerateLevel(void);          /* Called by playLevel() to 
                                             prepare the level */
      ConvexBlock *_CreateBlock(BSPPoly *pPoly,LevelBlock *pSrcBlock);
      void _CalculateBikeAnchors(void);
      int _IntersectWheelLevel(Vector2f Cp,float Cr,dContact *pContacts);
      int _IntersectWheelLine(Vector2f Cp,float Cr,int nNumContacts,dContact *pContacts,Vector2f A0,Vector2f A1);
      bool _IntersectHeadLevel(Vector2f Cp,float Cr,const Vector2f &LastCp);
      bool _IntersectHeadLine(Vector2f Cp,float Cr,Vector2f A0,Vector2f A1);
      bool _DoCircleTouchZone(const Vector2f &Cp,float Cr,LevelZone *pZone);
      bool _IntersectPointLevel(Vector2f Cp);
      void _UpdateZones(void);
      Entity *_SpawnEntity(std::string ID,EntityType Type,Vector2f Pos,LevelEntity *pSrc);
      void _KillEntity(Entity *pEnt);
      void CleanEntities(); /* clean memories of entities */
      EntityType _TransEntityType(std::string Name);
      EdgeEffect _TransEdgeEffect(std::string Name);
      void _UpdateEntities(void);
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
