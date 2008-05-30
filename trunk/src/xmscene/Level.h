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

#ifndef __LEVELSRC_H__
#define __LEVELSRC_H__

#include <string>
#include "BasicSceneStructs.h"
#include "helpers/VMath.h"

class Block;
class Entity;
class MotoGame;
class xmDatabase;
class FileHandle;
class SkyApparence;
class Zone;
class CollisionSystem;
class XMLDocument;
class ChipmunkWorld;
class Sprite;

/*===========================================================================
  Level source object - holds all stored information about a level
  ===========================================================================*/
class Level {
 public:
  Level();
  ~Level();

  bool loadReducedFromFile();
  void loadFullyFromFile();
  bool isFullyLoaded() const;
  void exportBinaryHeader(FileHandle *pfh);
  void importBinaryHeader(FileHandle *pfh);
  void importHeader(const std::string& i_id,
		    const std::string& i_checkSum,
		    const std::string& i_pack,
		    const std::string& i_packNum,
		    const std::string& i_name,
		    const std::string& i_description,
		    const std::string& i_author,
		    const std::string& i_date,
		    const std::string& i_music,
		    bool i_isScripted,
		    bool i_isPhysics);
  void rebuildCache();

  void loadXML();
  void saveXML();

  /* load level so that it is possible to play */
  /* to replay a level, unload then, reload it */
  int loadToPlay(ChipmunkWorld* i_chipmunkWorld); /* return the number of errors found */
  void unloadToPlay();

  std::string Id()          const;
  std::string Name()        const;
  std::string Author()      const;
  std::string Date()        const;
  std::string Description() const;
  std::string Music()       const;
  bool isXMotoTooOld()      const;
  std::string getRequiredVersion() const;
  std::string Pack()        const;
  std::string PackNum()     const;
  float LeftLimit()         const;
  float RightLimit()        const;
  float TopLimit()          const;
  float BottomLimit()       const;
  Vector2f PlayerStart()    const;
  const SkyApparence* Sky()        const;
  void setLimits(float v_leftLimit, float v_rightLimit, float v_topLimit, float v_bottomLimit);

  std::string FileName() const;
  void setFileName(const std::string& i_filename);
  std::string Checksum() const;
  bool isScripted() const;
  bool isPhysics() const;

  void updatePhysics(int timeStep, CollisionSystem* p_CollisionSystem, ChipmunkWorld* i_chipmunkWorld);

  Block*  getBlockById(const std::string& i_id);
  Entity* getEntityById(const std::string& i_id);
  Entity* getStartEntity();
  Zone*   getZoneById(const std::string& i_id);

  void setId(const std::string& i_id);
  void setName(const std::string& i_name);
  void setDescription(const std::string& i_description);
  void setDate(const std::string& i_date);
  void setAuthor(const std::string& i_author);
  void setCollisionSystem(CollisionSystem* p_CollisionSystem);

  std::string scriptFileName() const;
  std::string scriptSource() const;
  void setScriptFileName(const std::string& i_scriptFileName);
  std::vector<Block *> &Blocks();
  std::vector<Entity *> &Entities();
  /* entities which are not part of original level, but which are generated while playing */
  std::vector<Entity *> &EntitiesExterns();
  std::vector<Zone *> &Zones();
  std::vector<Zone *> &TouchingZones(); /* zones that the biker is touching */

  void killEntity(const std::string& i_entityId);
  unsigned int countToTakeEntities();
  void revertEntityDestroyed(const std::string& i_entityId);

  static int compareLevel(const Level& i_lvl1, const Level& i_lvl2);    
  static int compareLevelSamePack(const Level& i_lvl1, const Level& i_lvl2);    
  static int compareVersionNumbers(const std::string &i_v1, const std::string &i_v2);

  /* because some objects like entities have an internal movement */
  void updateToTime(MotoGame& i_scene);
  /* this method calls objects because rewind in replay can required some actions (like removing particles) */

  /* the entity will be destroyed by the level */
  void spawnEntity(Entity *v_entity);
  
  std::string SpriteForStrawberry() const;
  std::string SpriteForWecker() const;
  std::string SpriteForFlower() const;
  std::string SpriteForStar() const;
  std::string SoundForPickUpStrawberry() const;

  Sprite* strawberrySprite();
  Sprite* wreckerSprite();
  Sprite* flowerSprite();
  Sprite* starSprite();

  int getNumberLayer() {
    return m_numberLayer;
  }

  Vector2f getLayerOffset(int layer) {
    return m_layerOffsets[layer];
  }

  bool isLayerFront(int layer){
    return m_isLayerFront[layer];
  }

  std::vector<Vector2f>& getLayerOffsets(){
    return m_layerOffsets;
  }

  static void removeFromCache(xmDatabase *i_db, const std::string& i_id_level);

 private:
  std::string m_id;                 /* Level ID */
  std::string m_name;               /* Name of level */
  std::string m_author;             /* Author of level */
  std::string m_date;               /* When it was crafted */
  std::string m_description;        /* Description */
  std::string m_music;              /* music */
  std::string m_requiredVersion;    /* Required X-Moto version */
  std::string m_pack;               /* In this level pack */
  std::string m_packNum;            /* value to sort levels */
  std::string m_fileName;
  XMLDocument *m_xmlSource;    /* Plain XML source */      
  std::string m_scriptFileName;     /* Script file name */      
  std::string m_scriptSource;       /* Script source code */
  std::string m_checkSum;
  bool m_xmotoTooOld;               /* Flag set if our X-Moto version is too low to load level */      
  float m_leftLimit, m_rightLimit, m_topLimit, m_bottomLimit; /* Limits */
  Vector2f m_playerStart;           /* Player start pos */
  std::vector<Block *>  m_blocks;   /* Level blocks */
  std::vector<Zone *>   m_zones;    /* Level zones */
  std::vector<Entity *> m_entities; /* Level entities */
  std::vector<Entity *> m_entitiesDestroyed;
  std::vector<Entity *> m_entitiesExterns;
  Entity               *m_startEntity; /* entity where the player start */
  bool m_isBodyLoaded;
  CollisionSystem* m_pCollisionSystem;
  /* to avoid calculate it each frame */
  int  m_nbEntitiesToTake;
  std::string m_borderTexture;
  SkyApparence* m_sky;
  bool m_isScripted;
  bool m_isPhysics;

  int m_numberLayer;
  /* vector is the offset, and if bool == true, then it's a front layer*/
  std::vector<Vector2f> m_layerOffsets;
  std::vector<bool>     m_isLayerFront;

  std::string m_rSpriteForStrawberry;
  std::string m_rSpriteForWecker;
  std::string m_rSpriteForFlower;
  std::string m_rSpriteForStar;
  std::string m_rSoundForPickUpStrawberry;

  Sprite* m_strawberrySprite;
  Sprite* m_wreckerSprite;
  Sprite* m_flowerSprite;
  Sprite* m_starSprite;

  void addLimits();
  void exportBinary(const std::string &i_fileName, const std::string& i_sum);
  bool importBinary(const std::string &i_fileName, const std::string& i_sum);
  bool importBinaryHeaderFromFile(const std::string &i_fileName, const std::string& i_sum);
  std::string getNameInCache() const;

  void loadRemplacementSprites();

  void unloadLevelBody();
};

#endif
