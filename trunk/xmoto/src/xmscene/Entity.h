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

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "../MotoGame.h"
#include "../helpers/VMath.h"
#include "../helpers/Color.h"
#include "BasicSceneStructs.h"

namespace vapp {
  class MotoGame;
}

class Entity {
 public:
  Entity(const std::string& i_id);
  virtual ~Entity();

  std::string Id()     const; /* PlayerStart0, Flower0, Strawberry0, ... */

  float Size()         const;
  Vector2f InitialPosition() const;
  Vector2f DynamicPosition() const;
  virtual void loadToPlay();
  virtual void unloadToPlay();
  void setDynamicPosition(const Vector2f& i_dynamicPosition);
  void setInitialPosition(const Vector2f& i_initialPosition);
  float Z() const;
  float Width() const;
  float Height() const;
  void setId(const std::string& i_id);
  void setType(EntityType i_type);
  void setSize(float i_size);
  void setWidth(float i_width);
  void setHeight(float i_height);
  void setZ(float i_z);
  void setSpriteName(const std::string& i_spriteName);

  void saveXml(vapp::FileHandle *i_pfh);
  void saveBinary(vapp::FileHandle *i_pfh);
  static Entity* readFromXml(TiXmlElement *pElem);
  static Entity* readFromBinary(vapp::FileHandle *i_pfh);

  static EntityType  TypeFromStr(std::string i_typeStr);
  static std::string TypeToStr(EntityType i_type);

  EntityType  Type()       const; /* PLAYERSTART | SPRITE | FLOWER | ... */
  std::string SpriteName() const; /* PlayerStart, Flower, EndOfLevel, Bird, ... */
  virtual bool updateToTime(vapp::MotoGame& i_scene);    /* return true if the entity died */
  virtual void clearAfterRewind();

 public:

  std::string m_id;                    /* Its own identifer */
  EntityType  m_type;
  std::string m_spriteName;
  Vector2f    m_initialPosition;       /* Position */
  Vector2f    m_dynamicPosition;
  float       m_size;                  /* Size (radius) */
  float       m_width, m_height;       /* size of the picture, negativ it the theme size must be used */
  float       m_z;
};

class EntityParticle {
 public:
  EntityParticle(const Vector2f& i_position, const Vector2f i_velocity, float i_killTime, std::string i_spriteName = "");
  virtual ~EntityParticle();

  virtual void updateToTime(vapp::MotoGame& i_scene);
  float KillTime()    	   const;
  Vector2f Position() 	   const;
  float    Angle()    	   const;
  float    Size()     	   const;
  TColor   Color()    	   const;
  std::string SpriteName() const;

 protected:
    Vector2f m_position, m_velocity, m_acceleration; /* Position, velocity, and acceleration */
    float  m_ang, m_angVel, m_angAcc; /* Angular version of the above */
    float  m_spawnTime;
    float  m_killTime;
    bool   m_front;
    TColor m_color;
    float  m_size;
    std::string m_spriteName; /* replacement for entity spriteName */
};

class SmokeParticle : public EntityParticle {
 public:
  SmokeParticle(const Vector2f& i_position, const Vector2f i_velocity, float i_killTime, std::string i_spriteName);
  ~SmokeParticle();
  virtual void updateToTime(vapp::MotoGame& i_scene);

 private:
};

class FireParticle : public EntityParticle {
 public:
  FireParticle(const Vector2f& i_position, const Vector2f i_velocity, float i_killTime, std::string i_spriteName);
  ~FireParticle();

  virtual void updateToTime(vapp::MotoGame& i_scene);

 private:
  float m_fireSeed;
};

class StarParticle : public EntityParticle {
 public:
  StarParticle(const Vector2f& i_position, float i_killTime, std::string i_spriteName);
  ~StarParticle();

 private:
};

class DebrisParticle : public EntityParticle {
 public:
  DebrisParticle(const Vector2f& i_position, const Vector2f i_velocity, float i_killTime, std::string i_spriteName);
  ~DebrisParticle();

  virtual void updateToTime(vapp::MotoGame& i_scene);

 private:
};

class ParticlesSource : public Entity {
 public:
  ParticlesSource(const std::string& i_id);
  virtual ~ParticlesSource();

  virtual void loadToPlay();
  virtual void unloadToPlay();
  virtual bool updateToTime(vapp::MotoGame& i_scene);
  virtual void clearAfterRewind();
  void addParticle(Vector2f i_velocity, float i_killTime);
  virtual void addParticle(Vector2f i_velocity, float i_killTime, std::string i_spriteName) = 0;
  std::vector<EntityParticle *> &Particles();

 protected:
  std::vector<EntityParticle *> m_particles;
  float       m_nextParticleTime;
  float       m_particleTime_increment;

 private:
  void deleteParticles();
};

class ParticlesSourceSmoke : public ParticlesSource {
 public:
  ParticlesSourceSmoke(const std::string& i_id);
  ~ParticlesSourceSmoke();

  bool updateToTime(vapp::MotoGame& i_scene);
  void addParticle(Vector2f i_velocity, float i_killTime, std::string i_spriteName);

 private:
};

class ParticlesSourceFire : public ParticlesSource {
 public:
  ParticlesSourceFire(const std::string& i_id);
  ~ParticlesSourceFire();

  bool updateToTime(vapp::MotoGame& i_scene);
  void addParticle(Vector2f i_velocity, float i_killTime, std::string i_spriteName);

 private:
};

class ParticlesSourceStar : public ParticlesSource {
 public:
  ParticlesSourceStar(const std::string& i_id);
  ~ParticlesSourceStar();

  void addParticle(Vector2f i_velocity, float i_killTime, std::string i_spriteName);

 private:
};

class ParticlesSourceDebris : public ParticlesSource {
 public:
  ParticlesSourceDebris(const std::string& i_id);
  ~ParticlesSourceDebris();

  bool updateToTime(vapp::MotoGame& i_scene);
  void addParticle(Vector2f i_velocity, float i_killTime);
  void addParticle(Vector2f i_velocity, float i_killTime, std::string i_spriteName);

 private:
};

#endif /* __ENTITY_H__ */
