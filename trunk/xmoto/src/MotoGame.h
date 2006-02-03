/*=============================================================================
XMOTO
Copyright (C) 2005 Rasmus Neckelmann (neckelmann@gmail.com)

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

#include "VCommon.h"
#include "VApp.h"
#include "VMath.h"
#include "LevelSrc.h"
#include "BSP.h"
#include "DBuffer.h"
#include "Collision.h"

#define GAME_EVENT_QUEUE_SIZE				128
#define GAME_EVENT_OUTGOING_BUFFER  65536

namespace vapp {

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
    EE_GRASS
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
    float fDrive;         /* Drive throttle (0-1) */
    float fBrake;         /* Brake amount (0-1) */
    bool bPullBack;       /* Pull bike back, i.e. lift front wheel */
    bool bPushForward;    /* Push bike forward, i.e. lift rear wheel */
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
    
    char cRearWheelX,cRearWheelY;     /* Rear wheel position */
    char cFrontWheelX,cFrontWheelY;   /* Front wheel position */
    char cElbowX,cElbowY;             /* Elbow position */
    char cShoulderX,cShoulderY;       /* Shoulder position */
    char cLowerBodyX,cLowerBodyY;     /* Ass position */
    char cKneeX,cKneeY;               /* Knee position */
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
    BikeParams *pParams;
    BikeAnchors *pAnchors;
    
    /* Driving */
    float fCurBrake,fCurEngine;    
  };
  
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
    ET_PARTICLESOURCE
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
  };  

	/*===========================================================================
	Game event types
  ===========================================================================*/
  enum GameEventType {
		GAME_EVENT_PLAYER_DIES,
		GAME_EVENT_PLAYER_ENTERS_ZONE,
		GAME_EVENT_PLAYER_LEAVES_ZONE,
		GAME_EVENT_PLAYER_TOUCHES_ENTITY,
		GAME_EVENT_ENTITY_DESTROYED,
  };
  
	/*===========================================================================
	Game event
  ===========================================================================*/
  struct GameEventPlayerDies {
  	bool bWrecker;								/* Killed by wrecker */					
  };
  
  struct GameEventEntityDestroyed {
		/* Have enough information so that we can recreate the entity */
		char cEntityID[64];					  /* ID of entity */
		EntityType Type;              /* Type */
		float fSize;									/* Size of it */
		float fPosX,fPosY;					  /* Position of it */
  };
  
  struct GameEventPlayerEntersZone {
		LevelZone *pZone;							/* Pointer to zone */
  };
  
  struct GameEventPlayerLeavesZone {
		LevelZone *pZone;							/* Pointer to zone */
  };
  
  struct GameEventPlayerTouchesEntity {
		char cEntityID[64];					  /* ID of entity */
		bool bHead;										/* Touched with head? */
  };
  
  struct GameEvent {
    int nSeq;                         /* Sequence number */
		GameEventType Type;								/* Type of event */
		
		union GameEvent_u {								
			/* GAME_EVENT_PLAYER_DIES */		
			GameEventPlayerDies PlayerDies;								
			
			/* GAME_EVENT_ENTITY_DESTROYED */
			GameEventEntityDestroyed EntityDestroyed;
			
			/* GAME_EVENT_PLAYER_ENTERS_ZONE */
			GameEventPlayerEntersZone PlayerEntersZone;
			
			/* GAME_EVENT_PLAYER_LEAVES_ZONE */
			GameEventPlayerLeavesZone PlayerLeavesZone;

			/* GAME_EVENT_PLAYER_TOUCHES_ENTITY */
			GameEventPlayerTouchesEntity PlayerTouchesEntity;
		} u;		
  };
  
  struct RecordedGameEvent {
    float fTime;                    /* Time of event */
    GameEvent Event;                /* Event itself */
    bool bPassed;                   /* Whether we have passed it */
  };
    
	/*===========================================================================
	Game object
  ===========================================================================*/
  class MotoGame {
    public:          
      MotoGame() {m_pLevelSrc=NULL;
                  clearStates();}
      ~MotoGame() {endLevel();}     
    
      /* Methods */
      void playLevel(LevelSrc *pLevelSrc);
      void updateLevel(float fTimeStep,SerializedBikeState *pReplayState,DBuffer *pEventReplayBuffer);
      void endLevel(void);
      
      void touchEntity(Entity *pEntity,bool bHead); 
      void deleteEntity(Entity *pEntity);
      int countEntitiesByType(EntityType Type);
      Entity *findEntity(const std::string &ID);
      
      void clearStates(void);
      
      void gameMessage(std::string Text);
      void clearGameMessages(void);
      
      void getSerializedBikeState(SerializedBikeState *pState);
      void unserializeGameEvents(DBuffer &Buffer);
      void interpolateGameState(SerializedBikeState *pA,SerializedBikeState *pB,SerializedBikeState *p,float t);

      float getBikeEngineRPM(void);
      
      GameEvent *createGameEvent(GameEventType Type);
      GameEvent *getNextGameEvent(void);
      int getNumPendingGameEvents(void);
      
      /* Direct Lua interaction methods */
      bool scriptCallBool(std::string FuncName,bool bDefault=false);
      void scriptCallVoid(std::string FuncName);
      void scriptCallTblVoid(std::string Table,std::string FuncName);
      
      /* Data interface */
      bool isFinished(void) {return m_bFinished;}
      bool isDead(void) {return m_bDead;}
      LevelSrc *getLevelSrc(void) {return m_pLevelSrc;}
      std::vector<ConvexBlock *> &getBlocks(void) {return m_Blocks;}
      BikeState *getBikeState(void) {return &m_BikeS;}
      BikeController *getBikeController(void) {return &m_BikeC;}
      std::vector<GameMessage *> &getGameMessage(void) {return m_GameMessages;}
      std::vector<Entity *> &getEntities(void) {return m_Entities;}
      std::vector<Entity *> &getBSprites(void) {return m_BSprites;}
      std::vector<Entity *> &getFSprites(void) {return m_FSprites;}
      std::vector<OverlayEdge *> &getOverlayEdges(void) {return m_OvEdges;}
      float getTime(void) {return m_fTime;}
      void setTime(float f) {m_fTime=f;}
      float getFinishTime(void) {return m_fFinishTime;}
      ArrowPointer &getArrowPointer(void) {return m_Arrow;}
      bool isWheelSpinning(void) {return m_bWheelSpin;}
      Vector2f getWheelSpinPoint(void) {return m_WheelSpinPoint;}
      Vector2f getWheelSpinDir(void) {return m_WheelSpinDir;}
      CollisionSystem *getCollisionHandler(void) {return &m_Collision;}
            
      /* Debug */
      void resetDummies(void) {m_nNumDummies=0;}
      int getNumDummies(void) {return m_nNumDummies;}
      DummyHelper *getDummies(void) {return m_Dummies;}
      void addDummy(Vector2f Pos,float r,float g,float b);
    
    private:         
      /* Data */
      int m_nGameEventQueueReadIdx,m_nGameEventQueueWriteIdx;
      GameEvent m_GameEventQueue[GAME_EVENT_QUEUE_SIZE];
      
      float m_fTime,m_fLastAttitudeCon;
      float m_fFinishTime,m_fAttitudeCon;
      
      int m_nNumDummies;
      DummyHelper m_Dummies[100];
      
      int m_nStillFrames;

      ArrowPointer m_Arrow;               /* Arrow */  
      
      CollisionSystem m_Collision;        /* Collision system */
            
      LevelSrc *m_pLevelSrc;              /* Source of level */            
      lua_State *m_pL;                    /* Lua state associated with the
                                             level */
      std::vector<ConvexBlock *> m_Blocks;/* Blocks */
      std::vector<Entity *> m_Entities;   /* Entities */
      dWorldID m_WorldID;                 /* World ID */
      
      std::vector<RecordedGameEvent *> m_ReplayEvents; /* Events reconstructed from replay */
      
      std::vector<Entity *> m_FSprites;   /* Foreground sprites */
      std::vector<Entity *> m_BSprites;   /* Background sprites */
      std::vector<OverlayEdge *> m_OvEdges;/* Overlay edges */
      std::vector<Entity *> m_DelSchedule;/* Entities scheduled for deletion */
      std::vector<GameMessage *> m_GameMessages;
      
      BikeParams m_BikeP;                 /* Bike parameters */      
      BikeAnchors m_BikeA;                /* Important bike anchor points */
      BikeState m_BikeS;                  /* Bike state */
      BikeController m_BikeC;             /* Bike controller */
      
      bool m_bFinished,m_bDead;           /* Yir */
      
      int m_nLastEventSeq;
      
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
                                             
      /* Helpers */
      void _GenerateLevel(void);          /* Called by playLevel() to 
                                             prepare the level */
      void _CreateBlock(BSPPoly *pPoly,LevelBlock *pSrcBlock);
      void _CalculateBikeAnchors(void);
      int _IntersectWheelLevel(Vector2f Cp,float Cr,dContact *pContacts);
      int _IntersectWheelLine(Vector2f Cp,float Cr,int nNumContacts,dContact *pContacts,Vector2f A0,Vector2f A1);
      bool _IntersectHeadLevel(Vector2f Cp,float Cr);
      bool _IntersectHeadLine(Vector2f Cp,float Cr,Vector2f A0,Vector2f A1);
      bool _DoCircleTouchZone(const Vector2f &Cp,float Cr,LevelZone *pZone);
      bool _IntersectPointLevel(Vector2f Cp);
      void _UpdateZones(void);
      Entity *_SpawnEntity(std::string ID,EntityType Type,Vector2f Pos,LevelEntity *pSrc);
      void _KillEntity(Entity *pEnt);
      EntityType _TransEntityType(std::string Name);
      EdgeEffect _TransEdgeEffect(std::string Name);
      void _UpdateEntities(void);
      void _UpdateGameState(SerializedBikeState *pReplayState);
      char _MapCoordTo8Bits(float fRef,float fMaxDiff,float fCoord);
      float _Map8BitsToCoord(float fRef,float fMaxDiff,char c);
      unsigned short _MatrixTo16Bits(const float *pfMatrix);
      void _16BitsToMatrix(unsigned short n16,float *pfMatrix);
      void _SerializeGameEventQueue(DBuffer &Buffer,GameEvent *pEvent);
      
      void _UpdateReplayEvents(void);
      void _HandleReplayEvent(GameEvent *pEvent);
      void _HandleReverseReplayEvent(GameEvent *pEvent);
      
      /* MPhysics.cpp */
      void _UpdatePhysics(float fTimeStep);
      void _InitPhysics(void);
      void _UninitPhysics(void);
      void _PrepareBikePhysics(Vector2f StartPos);
      void _PrepareRider(Vector2f StartPos);
            
    };

};

#endif
