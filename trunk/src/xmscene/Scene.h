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

#ifndef __MOTOGAME_H__
#define __MOTOGAME_H__

#include "../helpers/VMath.h"
#include "../Collision.h"
#include "../helpers/Color.h"
#include "BasicSceneStructs.h"
#include "Bike.h"
#include <queue>

#define MOTOGAME_DEFAULT_GAME_MESSAGE_DURATION 500
#define REPLAY_SPEED_INCREMENT                 0.25
#define PHYS_STEP_SIZE                         1

class Level;
class BikeState;
class FileGhost;
class Ghost;
class NetGhost;
class PlayerBiker;
class ReplayBiker;
class MotoGameOnBikerHooks;
class LuaLibGame;
class xmDatabase;
class Camera;
class Entity;
class MotoGameEvent;
class Replay;
class GameRenderer;
class CollisionSystem;
class InputHandler;
class SerializedBikeState;
class DBuffer;
class RecordedGameEvent;
class Zone;
class SDynamicObject;
class Theme;
class BikerTheme;
class Biker;
class ChipmunkWorld;
class PhysicsSettings;

/*===========================================================================
  Serialized bike state
  ===========================================================================*/
#define SER_BIKE_STATE_DIR_LEFT         0x01
#define SER_BIKE_STATE_DIR_RIGHT        0x02

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
  int removeTime;                   /* The time when it should be removed */
  std::string Text;                 /* The text */
  int nAlpha;                       /* Alpha amount */
  bool bOnce;                       /* Unique message */
};  

/*===========================================================================
  Game object
  ===========================================================================*/

class MotoGameHooks {
public:
  virtual ~MotoGameHooks() {};
  virtual void OnTakeEntity() = 0;
};

class MotoGame {
public:          
  MotoGame();
  ~MotoGame();

  void setHooks(MotoGameHooks *i_motoGameHooks);

  /* update of the structure */
  void loadLevel(xmDatabase *i_db, const std::string& i_id_level);
  void prePlayLevel(Replay *recordingReplay,
		    bool i_playEvents);

  void playLevel();
  void updateLevel(int timeStep, Replay *i_recordedReplay = NULL);
  void endLevel();

  /* entities */
  void touchEntity(int i_player, Entity *pEntity, bool bHead); 
  void deleteEntity(Entity *pEntity);

  /* messages */
  void gameMessage(std::string Text,
		   bool bOnce = false,
		   int duration = MOTOGAME_DEFAULT_GAME_MESSAGE_DURATION);
  void clearGameMessages();
  void updateGameMessages();
  std::vector<GameMessage *> &getGameMessage(void) {return m_GameMessages;}

  /* serialization */
  static void getSerializedBikeState(BikeState *i_bikeState, int i_time, SerializedBikeState *pState, PhysicsSettings* i_physicsSettings);
  static void unserializeGameEvents(DBuffer *Buffer, std::vector<RecordedGameEvent *> *v_ReplayEvents, bool bDisplayInformation = false);

  /* events */
  void createGameEvent(MotoGameEvent *p_event);
  void destroyGameEvent(MotoGameEvent *p_event);
  MotoGameEvent* getNextGameEvent();
  int getNumPendingGameEvents();
  void cleanEventsQueue();
  void executeEvents(Replay *p_replay);

  /* player */
  void setPlayerPosition(int i_player, float x,float y,bool bFaceRight);
  const Vector2f &getPlayerPosition(int i_player);
  bool getPlayerFaceDir(int i_player);
  void setDeathAnim(bool b) {m_bDeathAnimEnabled=b;}

  /* Data interface */
  Level *getLevelSrc(void) {return m_pLevelSrc;}

  int getTime(void) {return m_time;}
  void setTime(int t) {m_time=t;}
  ArrowPointer &getArrowPointer(void) {return m_Arrow;}
  CollisionSystem *getCollisionHandler(void) {return &m_Collision;}

  void setShowGhostTimeDiff(bool b) { m_showGhostTimeDiff = b; }

  /* action for events */
  void SetEntityPos(std::string pEntityID, float pX, float pY);
  void SetEntityPos(Entity* pEntity,  float pX, float pY);
  void translateEntity(std::string pEntityID, float x, float y);
  void translateEntity(Entity* pEntity, float x, float y);
  void PlaceInGameArrow(float pX, float pY, float pAngle);
  void PlaceScreenArrow(float pX, float pY, float pAngle);
  void HideArrow();
  void MoveBlock(std::string pBlockID, float pX, float pY);
  void MoveBlock(Block* pBlock, float pX, float pY);
  void SetBlockPos(std::string pBlockID, float pX, float pY);
  void SetPhysicsBlockPos(std::string pBlockID, float pX, float pY);
  void SetBlockCenter(std::string pBlockID, float pX, float pY);
  void SetBlockRotation(std::string pBlockID, float pAngle);
  void SetBlockRotation(Block* pBlock, float pAngle);
  void SetEntityDrawAngle(std::string pEntityID, float pAngle);

  void CameraZoom(float pZoom);
  void CameraMove(float p_x, float p_y);
  void CameraRotate(float i_angle);
  void CameraAdaptToGravity();

  void killPlayer(int i_player);
  void playerEntersZone(int i_player, Zone *pZone);
  void playerLeavesZone(int i_player, Zone *pZone);
  void playerTouchesEntity(int i_player, std::string p_entityID, bool p_bTouchedWithHead);
  void addForceToPlayer(int i_player, const Vector2f& i_force, int i_startTime, int i_endTime);
  void entityDestroyed(const std::string& i_entityId);
  void addDynamicObject(SDynamicObject* p_obj);
  void removeSDynamicOfObject(std::string pObject);
  void addPenalityTime(int i_time);

  void createKillEntityEvent(std::string p_entityID);

  unsigned int getNbRemainingStrawberries();
  void makePlayerWin(int i_player);

  void setGravity(float x,float y);
  const Vector2f &getGravity(void);

  ReplayBiker* addReplayFromFile(std::string i_ghostFile,
				 Theme *i_theme, BikerTheme* i_bikerTheme,
				 bool i_enableEngineSound);
  FileGhost* addGhostFromFile(std::string i_ghostFile, const std::string& i_info, bool i_isReference,
			      Theme *i_theme, BikerTheme* i_bikerTheme,
			      const TColor& i_filterColor,
			      const TColor& i_filterUglyColor);
  PlayerBiker* addPlayerBiker(int i_localNetId, Vector2f i_position, DriveDir i_direction,
			      Theme *i_theme, BikerTheme* i_bikerTheme,
			      const TColor& i_filterColor,
			      const TColor& i_filterUglyColor,
			      bool i_enableEngineSound);

  NetGhost* addNetGhost(const std::string& i_info, Theme *i_theme,
			BikerTheme* i_bikerTheme,
			const TColor& i_filterColor,
			const TColor& i_filterUglyColor);

  std::vector<Ghost *> &Ghosts();
  std::vector<Biker*> &Players();

  bool doesPlayEvents() const;

  PhysicsSettings* getPhysicsSettings();

  void fastforward(int i_time);
  void fastrewind(int i_time);
  void pause();
  void faster(float i_increment = REPLAY_SPEED_INCREMENT);
  void slower(float i_increment = REPLAY_SPEED_INCREMENT);
  float getSpeed() const;
  void setSpeed(float f);
  bool isPaused();

  void handleEvent(MotoGameEvent *pEvent);

  void setInfos(const std::string& i_infos);
  std::string getInfos() const;

  LuaLibGame* getLuaLibGame();

  std::vector<Camera*>& Cameras();

  Camera* getCamera();
  unsigned int  getNumberCameras();
  void setCurrentCamera(unsigned int currentCamera);
  unsigned int  getCurrentCamera();
  void addCamera(Vector2i upperleft, Vector2i downright, bool i_useActiveZoom = true);
  void resetFollow();
  void removeCameras();
  void setAutoZoomCamera();
  bool isAutoZoomCamera();

private:

  /* Data */
  std::queue<MotoGameEvent*> m_GameEventQueue;
  int m_time;
  float m_floattantTimeStepDiff; // to play slowly replay
  int m_lastStateSerializationTime;
  int m_lastStateUploadTime;
  float m_speed_factor; /* nb hundreadths to increment each time ; is a float so that manage slow */
  bool  m_is_paused;

  MotoGameHooks *m_motoGameHooks;
  std::string m_infos;
  bool m_bDeathAnimEnabled;
  Vector2f m_PhysGravity; /* gravity */
  ArrowPointer m_Arrow;               /* Arrow */  
  CollisionSystem m_Collision;        /* Collision system */
  Level *m_pLevelSrc;              /* Source of level */            
  LuaLibGame *m_luaGame;

  std::vector<Entity *> m_DestroyedEntities; /* destroyed entities */
  std::vector<Entity *> m_DelSchedule;/* Entities scheduled for deletion */
  std::vector<GameMessage *> m_GameMessages;

  std::vector<Biker*> m_players;

  bool m_showGhostTimeDiff;
  void onRewinding();

  std::vector<Ghost*> m_ghosts;
  std::vector<float> m_myLastStrawberries;

  GameRenderer* m_renderer;
  ChipmunkWorld* m_chipmunkWorld;

  std::vector<SDynamicObject*> m_SDynamicObjects;

  int m_nLastEventSeq;

  PhysicsSettings* m_physicsSettings;

  /* for EveryHundreath function */
  int m_lastCallToEveryHundreath;
  bool m_playEvents;

  // some part of the game can be update only half on the time
  bool m_halfUpdate;

  std::vector<Camera*> m_cameras;
  unsigned int m_currentCamera;

  void cleanGhosts();
  void cleanPlayers();

  /* Helpers */
  void _GenerateLevel(void);          /* Called by playLevel() to 
					 prepare the level */
  bool _DoCircleTouchZone(const Vector2f &Cp,float Cr,Zone *pZone);
  void _KillEntity(Entity *pEnt);
  void _UpdateEntities(void);
  void _UpdateZones(void);
  bool touchEntityBodyExceptHead(const BikeState &pBike, const Entity &p_entity);

  void DisplayDiffFromGhost();

  void cleanScriptDynamicObjects();
  void nextStateScriptDynamicObjects(int i_nbCents);

  void _SerializeGameEventQueue(DBuffer* Buffer,MotoGameEvent *pEvent);      
  void _UpdateDynamicCollisionLines(void);
};

class MotoGameOnBikerHooks : public OnBikerHooks {
public:
  MotoGameOnBikerHooks(MotoGame* i_motoGame, int i_playerNumber);
  virtual ~MotoGameOnBikerHooks();
  void onSomersaultDone(bool i_counterclock);
  void onWheelTouches(int i_wheel, bool i_touch);
  void onHeadTouches();

private:
  MotoGame* m_motoGame;
  int m_playerNumber;
};

#endif
