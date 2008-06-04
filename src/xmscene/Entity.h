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

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <string>
#include "helpers/Color.h"
#include "helpers/VMath.h"
#include "BasicSceneStructs.h"

class EntityParticle;
class TiXmlElement;
class FileHandle;
class Sprite;

/**
  An entity is an object that the biker can found on his way
*/
class Entity {
 public:
  Entity(const std::string& i_id);
  virtual ~Entity();

  /* PlayerStart0, Flower0, Strawberry0, ... */
  std::string Id() const;
  inline float Size() const
  {
    return m_size;
  }
  Vector2f InitialPosition() const;
  inline Vector2f DynamicPosition() const
  {
    return m_dynamicPosition;  
  }
  bool DoesKill() const; /** have this entity the property to kill ? (like wecker) */
  bool DoesMakeWin() const; /** have this entity the property to make win ? (like flower) */
  bool IsToTake() const; /* return true is the entity must be taken by the player */
  inline float Z() const
  {
    return m_z;
  }
  float Width() const;
  float Height() const;
  float DrawAngle() const;
  bool  DrawReversed() const;
  const TColor& Color() const;
  inline const std::string& SpriteName() const
  {
    return m_spriteName;
  }
  Sprite* getSprite() const;
  bool  isAlive() const;

  virtual void loadToPlay();
  virtual void unloadToPlay();
  void setDynamicPosition(const Vector2f& i_dynamicPosition);
  void setInitialPosition(const Vector2f& i_initialPosition);
  void setId(const std::string& i_id);
  void setSize(float i_size);
  void setWidth(float i_width);
  void setHeight(float i_height);
  void setDrawAngle(float i_drawAngle);
  void setDrawReversed(bool i_drawReversed);
  void setZ(float i_z);
  void setSpriteName(const std::string& i_spriteName);
  void setSprite(Sprite* i_sprite);
  void setColor(const TColor& i_color);
  void setAlive(bool alive);

  void saveXml(FileHandle *i_pfh);
  void saveBinary(FileHandle *i_pfh);
  static Entity* readFromXml(TiXmlElement *pElem);
  static Entity* readFromBinary(FileHandle *i_pfh);

  static EntitySpeciality  SpecialityFromStr(std::string& i_typeStr);
  static std::string       SpecialityToStr(EntitySpeciality i_type);
  inline EntitySpeciality Speciality() const {
    /* replays only store one property per entity, this is this one */
    return m_speciality;
  }
  void setSpeciality(EntitySpeciality i_speciality);

  virtual bool updateToTime(int i_time, Vector2f& i_gravity);

  AABB& getAABB();

protected:
  std::string m_id;              /** Its own identifer */
  std::string m_spriteName;      /** Name of the sprite to be drawn */
  Sprite*     m_sprite;
  Vector2f    m_initialPosition; /** Position at the start of the level */
  Vector2f    m_dynamicPosition; /** Current position */
  float       m_size;            /** Size (radius) */
  float       m_width, m_height; /** size of the picture, negativ it the theme size must be used */
  float       m_drawAngle;
  bool        m_drawReversed;
  float       m_z;               /** deep coord */
  bool        m_doesKill;
  bool        m_doesMakeWin;
  bool        m_isToTake;
  /* Use to know if a script shall update the pos of the entity*/
  bool        m_isAlive;
  TColor      m_color;

  AABB        m_BBox;
  bool        m_isBBoxDirty;
  EntitySpeciality m_speciality;

  static Entity* createEntity(const std::string& id, const std::string& typeId,
			      EntitySpeciality speciality,
			      const Vector2f& position, float angle,
			      bool reversed, float size,
			      float width, float height, float z,
			      const std::string& spriteName, const std::string& typeName);
  Sprite* loadSprite(const std::string& i_spriteName = "");
};

typedef enum particleSourceType {None, Smoke, Fire, Star, Debris} particleSourceType;

class ParticlesSource : public Entity {
 public:
  ParticlesSource(const std::string& i_id, int i_particleTime_increment);
  virtual ~ParticlesSource();

  virtual void loadToPlay();
  virtual void unloadToPlay();
  virtual bool updateToTime(int i_time, Vector2f& i_gravity);
  inline std::vector<EntityParticle*>& Particles() {
    return m_particles;
  }
  virtual void addParticle(int i_curTime) = 0;

  static void setAllowParticleGeneration(bool i_value);

  inline particleSourceType getType(){
    return m_type;
  }

 protected:
  std::vector<EntityParticle*> m_particles;
  std::vector<EntityParticle*> m_deadParticles;
  particleSourceType m_type;

  static int m_totalOfParticles;
  static bool hasReachedMaxParticles();

  // return NULL if no dead particle exists
  EntityParticle* getExistingParticle();
  void addDeadParticle(EntityParticle* pEntityParticle);

 private:
  int         m_lastParticleTime;
  int         m_particleTime_increment;
  static bool m_allowParticleGeneration;

  void deleteParticles();
};


class ParticlesSourceMultiple : public ParticlesSource {
public:
  ParticlesSourceMultiple(const std::string& i_id,
			  int i_particleTime_increment,
			  unsigned int i_nbSprite);
  virtual ~ParticlesSourceMultiple();

  Sprite* getSprite(unsigned int sprite=0) const;
  void setSprite(Sprite* i_sprite, unsigned int sprite=0);

protected:
  std::vector<Sprite*> m_sprites;
};



class ParticlesSourceSmoke : public ParticlesSourceMultiple {
 public:
  ParticlesSourceSmoke(const std::string& i_id);
  virtual ~ParticlesSourceSmoke();

  bool updateToTime(int i_time, Vector2f& i_gravity);
  void addParticle(int i_curTime);
};



class ParticlesSourceFire : public ParticlesSource {
 public:
  ParticlesSourceFire(const std::string& i_id);
  virtual ~ParticlesSourceFire();

  bool updateToTime(int i_time, Vector2f& i_gravity);
  void addParticle(int i_curTime);
};

class ParticlesSourceStar : public ParticlesSource {
 public:
  ParticlesSourceStar(const std::string& i_id);
  virtual ~ParticlesSourceStar();

  void addParticle(int i_curTime);
};

class ParticlesSourceDebris : public ParticlesSource {
 public:
  ParticlesSourceDebris(const std::string& i_id);
  virtual ~ParticlesSourceDebris();

  bool updateToTime(int i_time, Vector2f& i_gravity);
  void addParticle(int i_curTime);
};




/**
  An EntityParticle is an entity which as a movement and which dies
*/
class EntityParticle : public Entity {
 public:
  EntityParticle();
  virtual ~EntityParticle();

  virtual  bool updateToTime(int i_time, Vector2f& i_gravity);
  int           KillTime() const;
  float         Angle() const;

  // for fast spriteName check
  inline unsigned int  spriteIndex() {
    return m_spriteIndex;
  }
  void setSpriteIndex(unsigned int spriteIndex){
    m_spriteIndex = spriteIndex;
  }

 protected:
  void init(const Vector2f& i_position, const Vector2f& i_velocity, int i_killTime);
  /* Position, velocity, and acceleration */
  Vector2f      m_velocity, m_acceleration;
  /* Angular version of the above */
  float         m_ang, m_angVel, m_angAcc;
  int           m_killTime;
  unsigned int  m_spriteIndex;
};


class SmokeParticle : public EntityParticle {
 public:
  SmokeParticle(const Vector2f& i_position, const Vector2f& i_velocity, int i_killTime, std::string i_spriteName);
  virtual ~SmokeParticle();
  void init(const Vector2f& i_position, const Vector2f& i_velocity, int i_killTime, std::string i_spriteName);

  virtual bool updateToTime(int i_time, Vector2f& i_gravity);

 private:
};

class FireParticle : public EntityParticle {
 public:
  FireParticle(const Vector2f& i_position, const Vector2f& i_velocity, int i_killTime, std::string i_spriteName);
  virtual ~FireParticle();
  void init(const Vector2f& i_position, const Vector2f& i_velocity, int i_killTime, std::string i_spriteName);

  virtual bool updateToTime(int i_time, Vector2f& i_gravity);

 private:
  float m_fireSeed;
};

class StarParticle : public EntityParticle {
 public:
  StarParticle(const Vector2f& i_position, const Vector2f& i_velocity, int i_killTime, std::string i_spriteName);
  virtual ~StarParticle();
  void init(const Vector2f& i_position, const Vector2f& i_velocity, int i_killTime, std::string i_spriteName);

 private:
};

class DebrisParticle : public EntityParticle {
 public:
  DebrisParticle(const Vector2f& i_position, const Vector2f& i_velocity, int i_killTime, std::string i_spriteName);
  virtual ~DebrisParticle();
  void init(const Vector2f& i_position, const Vector2f& i_velocity, int i_killTime, std::string i_spriteName);

  virtual bool updateToTime(int i_time, Vector2f& i_gravity);

 private:
};

#endif /* __ENTITY_H__ */
