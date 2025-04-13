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

#include "Entity.h"
#include "Block.h"
#include "ChipmunkWorld.h"
#include "Level.h"
#include "PhysicsSettings.h"
#include "common/Theme.h"
#include "common/VFileIO.h"
#include "common/VXml.h"
#include "common/XMSession.h"
#include "helpers/Log.h"
#include "helpers/Random.h"
#include "xmoto/PhysSettings.h"
#include <chipmunk.h>
#include <sstream>

// don't excceed this number of particles to not reduce significantly the fps
#define PARTICLESSOURCE_TOTAL_MAX_PARTICLES 512
#define ENTITY_DEFAULT_SPRITE_NAME ""
#define ENTITY_DEFAULT_SIZE 1.0
#define ENTITY_DEFAULT_Z -1.0
#define PARTICLES_SOURCE_SMOKE_TIME_INCREMENT 5
#define PARTICLES_SOURCE_STAR_TIME_INCREMENT 2
#define PARTICLES_SOURCE_FIRE_TIME_INCREMENT 4
#define PARTICLES_SOURCE_DEBRIS_TIME_INCREMENT 2
#define PARTICLES_SOURCE_SPARKLE_TIME_INCREMENT 1
#define PS_SPARKLE_DELAY_MIN 150
#define PS_SPARKLE_DELAY_MAX 250

/*====================================================
Entity "an object that the biker can found on his way"
====================================================*/
/* includes: particle sources, strawberries, sprites, wreckers, end of levels*/

Entity::Entity(const std::string &i_id) {
  m_id = i_id;
  m_spriteName = ENTITY_DEFAULT_SPRITE_NAME;
  m_sprite = NULL;
  m_size = ENTITY_DEFAULT_SIZE;
  m_width = -1.0;
  m_height = -1.0;
  m_drawAngle = 0.0;
  m_drawReversed = false;
  m_z = ENTITY_DEFAULT_Z;
  m_doesKill = false;
  m_doesMakeWin = false;
  m_isToTake = false;
  m_BCircle.reset();
  m_isBBoxDirty = true;
  m_speciality = ET_NONE;
  m_isCheckpoint = false;
}

Entity::~Entity() {}

void Entity::loadToPlay(const std::string &i_ScriptSource) {
  m_dynamicPosition = m_initialPosition;
  /* make every entity alive */
  setAlive(true);
  m_isBBoxDirty = true;
}

void Entity::unloadToPlay() {}

bool Entity::IsToTake() const {
  return m_isToTake;
}

bool Entity::DoesMakeWin() const {
  return m_doesMakeWin;
}

bool Entity::DoesKill() const {
  return m_doesKill;
}

bool Entity::IsCheckpoint() const {
  return false;
}

Vector2f Entity::InitialPosition() const {
  return m_initialPosition;
}

float Entity::Width() const {
  return m_width;
}

float Entity::Height() const {
  return m_height;
}

float Entity::DrawAngle() const {
  return m_drawAngle;
}

bool Entity::DrawReversed() const {
  return m_drawReversed;
}

void Entity::translate(float x, float y) {
  setDynamicPosition(m_dynamicPosition + Vector2f(x, y));
  m_BCircle.translate(x, y);
}

void Entity::setInitialPosition(const Vector2f &i_initialPosition) {
  m_initialPosition = i_initialPosition;
  m_isBBoxDirty = true;
}

bool Entity::isAlive() const {
  return m_isAlive;
}

void Entity::setSpriteName(const std::string &i_spriteName) {
  m_spriteName = i_spriteName;
}

void Entity::setSprite(Sprite *i_sprite) {
  m_sprite = i_sprite;
}

void Entity::setSpeciality(EntitySpeciality i_speciality) {
  m_doesMakeWin = (i_speciality == ET_MAKEWIN);
  m_doesKill = (i_speciality == ET_KILL);
  m_isToTake = (i_speciality == ET_ISTOTAKE);
  m_isCheckpoint = (i_speciality == ET_CHECKPOINT);

  m_speciality = i_speciality;
}

void Entity::setWidth(float i_width) {
  m_width = i_width;
}

void Entity::setHeight(float i_height) {
  m_height = i_height;
}

void Entity::setDrawAngle(float i_drawAngle) {
  m_drawAngle = i_drawAngle;
}

void Entity::setDrawReversed(bool i_drawReversed) {
  m_drawReversed = i_drawReversed;
}

void Entity::setZ(float i_z) {
  m_z = i_z;
}

void Entity::setAlive(bool alive) {
  m_isAlive = alive;
}

bool Entity::updateToTime(int i_time,
                          Vector2f &i_gravity,
                          PhysicsSettings *i_physicsSettings,
                          bool i_allowParticules) {
  return false;
}

AABB &Entity::getAABB() {
  if (m_isBBoxDirty == true) {
    m_BCircle.reset();
    m_BCircle.init(m_dynamicPosition, m_size);
    m_isBBoxDirty = false;
  }

  return m_BCircle.getAABB();
}

void Entity::loadSpriteTextures() {
  // for playerStart, Joints, and level's theme remplacement
  if (getSprite() != NULL)
    getSprite()->loadTextures();
  else
    LogDebug("entity [%s] type [%s] Sprite [%s] NULL",
             m_id.c_str(),
             SpecialityToStr(m_speciality).c_str(),
             m_spriteName.c_str());
}

EntitySpeciality Entity::SpecialityFromStr(std::string &i_typeStr) {
  if (i_typeStr == "PlayerStart")
    return ET_ISSTART;
  if (i_typeStr == "EndOfLevel")
    return ET_MAKEWIN;
  if (i_typeStr == "Wrecker")
    return ET_KILL;
  if (i_typeStr == "Strawberry")
    return ET_ISTOTAKE;
  if (i_typeStr == "ParticleSource")
    return ET_PARTICLES_SOURCE;
  if (i_typeStr == "Joint")
    return ET_JOINT;
  if (i_typeStr == "Checkpoint")
    return ET_CHECKPOINT;

  return ET_NONE;
}

std::string Entity::SpecialityToStr(EntitySpeciality i_speciality) {
  switch (i_speciality) {
    case ET_ISSTART:
      return "PlayerStart";
      break;
    case ET_MAKEWIN:
      return "EndOfLevel";
      break;
    case ET_KILL:
      return "Wrecker";
      break;
    case ET_ISTOTAKE:
      return "Strawberry";
      break;
    case ET_PARTICLES_SOURCE:
      return "ParticleSource";
      break;
    case ET_JOINT:
      return "Joint";
      break;
    case ET_CHECKPOINT:
      return "Checkpoint";
      break;
    default:
      return "Sprite";
  }
}

Sprite *Entity::loadSprite(const std::string &i_spriteName) {
  std::string spriteName;
  if (i_spriteName == "")
    spriteName = SpriteName();
  else
    spriteName = i_spriteName;

  Sprite *sprite =
    Theme::instance()->getSprite(SPRITE_TYPE_ANIMATION, spriteName);
  if (sprite == NULL) {
    sprite = Theme::instance()->getSprite(SPRITE_TYPE_EFFECT, spriteName);
  }

  return sprite;
}

Entity *Entity::createEntity(const std::string &id,
                             const std::string &typeId,
                             EntitySpeciality speciality,
                             const Vector2f &position,
                             float angle,
                             bool reversed,
                             float size,
                             float width,
                             float height,
                             float z,
                             const std::string &spriteName,
                             const std::string &typeName) {
  Entity *v_entity = NULL;

  switch (speciality) {
    case ET_PARTICLES_SOURCE: {
      if (typeName == "Smoke") {
        v_entity = new ParticlesSourceSmoke(id);
      } else if (typeName == "Fire") {
        v_entity = new ParticlesSourceFire(id);
      } else if (typeName == "Star") {
        v_entity = new ParticlesSourceStar(id);
      } else if (typeName == "Debris") {
        v_entity = new ParticlesSourceDebris(id);
      } else if (typeName == "Sparkle") {
        v_entity = new ParticlesSourceSparkle(id);
      } else {
        throw Exception("Entity " + id + " has an invalid type name");
      }
    } break;
    case ET_JOINT:
      v_entity = new Joint(id);
      break;
    case ET_CHECKPOINT:
      v_entity = new Checkpoint(id);
      break;
    default:
      v_entity = new Entity(id);
      break;
  }

  switch (speciality) {
    case ET_NONE:
      v_entity->setSpriteName(spriteName);
      v_entity->setSprite(v_entity->loadSprite());
      break;
    case ET_PARTICLES_SOURCE:
      v_entity->setSpriteName(typeName);

      // hard coded particles effects
      if (typeName == "Smoke") {
        // smoke has two sprites
        ((ParticlesSourceSmoke *)v_entity)
          ->setSprite(v_entity->loadSprite(std::string("Smoke1")), 0);
        ((ParticlesSourceSmoke *)v_entity)
          ->setSprite(v_entity->loadSprite(std::string("Smoke2")), 1);
      } else if (typeName == "Fire") {
        v_entity->setSprite(v_entity->loadSprite(std::string("Fire1")));
      } else if (typeName == "Debris") {
        v_entity->setSprite(v_entity->loadSprite(std::string("Debris1")));
        v_entity->setSpriteName("Debris1");
      } else if (typeName == "Sparkle") {
        v_entity->setSprite(v_entity->loadSprite(std::string("Fire1")));
      }
      break;
    case ET_JOINT:
      break;
    case ET_CHECKPOINT:
    default:
      v_entity->setSpriteName(typeId);
      v_entity->setSprite(v_entity->loadSprite());
  }
  v_entity->setSpeciality(speciality);
  v_entity->setInitialPosition(position);
  v_entity->setSize(size);
  if (width > 0.0) {
    v_entity->setWidth(width);
  }
  if (height > 0.0) {
    v_entity->setHeight(height);
  }
  if (angle > 0.0) {
    v_entity->setDrawAngle(angle);
  }
  v_entity->setDrawReversed(reversed);
  v_entity->setZ(z);

  return v_entity;
}

Entity *Entity::readFromXml(xmlNodePtr pElem) {
  std::string v_id;
  std::string v_typeId;
  EntitySpeciality v_speciality;
  Vector2f v_position;
  float v_size = 0.2;
  float v_height = -1.0;
  float v_width = -1.0;
  float v_angle = -1.0;
  bool v_reversed = false;
  float v_z = ENTITY_DEFAULT_Z;
  std::string v_spriteName;
  std::string v_typeName;

  /* read xml information */
  v_id = XMLDocument::getOption(pElem, "id");
  v_typeId = XMLDocument::getOption(pElem, "typeid");
  v_speciality = Entity::SpecialityFromStr(v_typeId);

  xmlNodePtr pPosElem = XMLDocument::subElement(pElem, "position");
  if (pPosElem != NULL) {
    v_position.x = atof(XMLDocument::getOption(pPosElem, "x", "0").c_str());
    v_position.y = atof(XMLDocument::getOption(pPosElem, "y", "0").c_str());
    v_angle = atof(XMLDocument::getOption(pPosElem, "angle", "-1.0").c_str());
    v_reversed =
      XMLDocument::getOption(pPosElem, "reversed", "false") == "true";
  }
  xmlNodePtr pSizeElem = XMLDocument::subElement(pElem, "size");
  if (pSizeElem != NULL) {
    v_size = (atof(XMLDocument::getOption(pSizeElem, "r", "0.2").c_str()));
    v_width = atof(XMLDocument::getOption(pSizeElem, "width", "-1.0").c_str());
    v_height =
      atof(XMLDocument::getOption(pSizeElem, "height", "-1.0").c_str());
  }
  /* Get parameters */
  std::string v_paramName;
  std::string v_paramValue;
  for (xmlNodePtr pSubElem = XMLDocument::subElement(pElem, "param");
       pSubElem != NULL;
       pSubElem = XMLDocument::nextElement(pSubElem)) {
    v_paramName = XMLDocument::getOption(pSubElem, "name");
    v_paramValue = XMLDocument::getOption(pSubElem, "value");
    if (v_paramName == "z") {
      v_z = (atof(v_paramValue.c_str()));
    } else if (v_paramName == "name") {
      v_spriteName = v_paramValue;
    } else if (v_paramName == "type") {
      v_typeName = v_paramValue;
    }
  }

  return createEntity(v_id,
                      v_typeId,
                      v_speciality,
                      v_position,
                      v_angle,
                      v_reversed,
                      v_size,
                      v_width,
                      v_height,
                      v_z,
                      v_spriteName,
                      v_typeName);
}

void Entity::saveBinary(FileHandle *i_pfh) {
  XMFS::writeString(i_pfh, Id());
  XMFS::writeString(i_pfh, Entity::SpecialityToStr(Speciality()));
  XMFS::writeFloat_LE(i_pfh, Size());
  XMFS::writeFloat_LE(i_pfh, Width());
  XMFS::writeFloat_LE(i_pfh, Height());
  XMFS::writeFloat_LE(i_pfh, InitialPosition().x);
  XMFS::writeFloat_LE(i_pfh, InitialPosition().y);
  XMFS::writeFloat_LE(i_pfh, DrawAngle());
  XMFS::writeBool(i_pfh, DrawReversed());

  switch (Speciality()) {
    case ET_NONE:
    case ET_PARTICLES_SOURCE:
      XMFS::writeByte(i_pfh, 0x02);
      break;
    default:
      XMFS::writeByte(i_pfh, 0x01);
  }

  std::ostringstream v_z;
  v_z << Z();
  XMFS::writeString(i_pfh, "z");
  XMFS::writeString(i_pfh, v_z.str());

  if (Speciality() == ET_NONE) {
    XMFS::writeString(i_pfh, "name");
    XMFS::writeString(i_pfh, SpriteName());
  } else if (Speciality() == ET_PARTICLES_SOURCE) {
    XMFS::writeString(i_pfh, "type");
    XMFS::writeString(i_pfh, SpriteName());
  }
}

Entity *Entity::readFromBinary(FileHandle *i_pfh) {
  std::string v_id;
  std::string v_typeId;
  EntitySpeciality v_speciality;
  Vector2f v_position;
  float v_size = 0.2;
  float v_height = -1.0;
  float v_width = -1.0;
  float v_angle = -1.0;
  bool v_reversed = false;
  float v_z = ENTITY_DEFAULT_Z;
  std::string v_spriteName;
  std::string v_typeName;

  /* read values */
  v_id = XMFS::readString(i_pfh);
  v_typeId = XMFS::readString(i_pfh);
  v_speciality = Entity::SpecialityFromStr(v_typeId);
  v_size = XMFS::readFloat_LE(i_pfh);
  v_width = XMFS::readFloat_LE(i_pfh);
  v_height = XMFS::readFloat_LE(i_pfh);
  v_position.x = XMFS::readFloat_LE(i_pfh);
  v_position.y = XMFS::readFloat_LE(i_pfh);
  v_angle = XMFS::readFloat_LE(i_pfh);
  v_reversed = XMFS::readBool(i_pfh);
  std::string v_paramName;
  std::string v_paramValue;
  int nNumParams = XMFS::readByte(i_pfh);
  for (int j = 0; j < nNumParams; j++) {
    v_paramName = XMFS::readString(i_pfh);
    v_paramValue = XMFS::readString(i_pfh);

    if (v_paramName == "z") {
      v_z = atof(v_paramValue.c_str());
    } else if (v_paramName == "name") {
      v_spriteName = v_paramValue;
    } else if (v_paramName == "type") {
      v_typeName = v_paramValue;
    }
  }

  return createEntity(v_id,
                      v_typeId,
                      v_speciality,
                      v_position,
                      v_angle,
                      v_reversed,
                      v_size,
                      v_width,
                      v_height,
                      v_z,
                      v_spriteName,
                      v_typeName);
}

/*==========================================
        Checkpoints
==========================================*/

bool Checkpoint::IsCheckpoint() const {
  return true;
}

void Checkpoint::deactivate() {
  m_isVirgin = true;
}

bool Checkpoint::isActivated() const {
  return m_isVirgin == false;
}

void Checkpoint::activate(const std::vector<Entity *> &i_destroyedEntities,
                          DriveDir i_direction) {
  m_isVirgin = false;
  m_destroyedEntities = i_destroyedEntities;
  m_direction = i_direction;
}

/*===========================================
        Joints
===========================================*/

void Joint::saveBinary(FileHandle *i_pfh) {
  Entity::saveBinary(i_pfh);

  XMFS::writeByte(i_pfh, 0x03);

  XMFS::writeString(i_pfh, "type");
  XMFS::writeString(i_pfh, jointTypeToStr(getJointType()));
  XMFS::writeString(i_pfh, "start");
  XMFS::writeString(i_pfh, getStartBlockId());
  XMFS::writeString(i_pfh, "end");
  XMFS::writeString(i_pfh, getEndBlockId());
}

void Joint::readFromBinary(FileHandle *i_pfh) {
  std::string v_paramName;
  std::string v_paramValue;
  int nNumParams = XMFS::readByte(i_pfh);
  for (int j = 0; j < nNumParams; j++) {
    v_paramName = XMFS::readString(i_pfh);
    v_paramValue = XMFS::readString(i_pfh);

    if (v_paramName == "type") {
      setJointType(jointTypeFromStr(v_paramValue));
    } else if (v_paramName == "start") {
      setStartBlockId(v_paramValue);
    } else if (v_paramName == "end") {
      setEndBlockId(v_paramValue);
    }
  }
}

void Joint::readFromXml(xmlNodePtr pElem) {
  xmlNodePtr pJointElem = XMLDocument::subElement(pElem, "joint");

  if (pJointElem != NULL) {
    std::string v_type = XMLDocument::getOption(pJointElem, "type", "");
    std::string v_start =
      XMLDocument::getOption(pJointElem, "connection-start", "");
    std::string v_end =
      XMLDocument::getOption(pJointElem, "connection-end", "");
    if (v_type != "" && v_start != "" && v_end != "") {
      setJointType(Joint::jointTypeFromStr(v_type));
      setStartBlockId(v_start);
      setEndBlockId(v_end);
    }
  }
}

jointType Joint::jointTypeFromStr(std::string &i_typeStr) {
  if (i_typeStr == "pivot")
    return Pivot;
  else if (i_typeStr == "pin")
    return Pin;
  else
    return JointNone;
}
std::string Joint::jointTypeToStr(jointType i_type) {
  switch (i_type) {
    case Pivot:
      return "pivot";
      break;
    case Pin:
      return "pin";
      break;
    case JointNone:
    default:
      return "";
      break;
  }
}

void Joint::loadToPlay(Level *i_level, ChipmunkWorld *i_chipmunkWorld) {
  Entity::loadToPlay("");

  setStartBlock(i_level->getBlockById(getStartBlockId()));
  setEndBlock(i_level->getBlockById(getEndBlockId()));

  cpBody *body1;
  cpBody *body2;
  body1 = getStartBlock()->getPhysicBody();
  body2 = getEndBlock()->getPhysicBody();

  if (body1 == NULL || body2 == NULL)
    return;

  // we don't want them to collide. if a block is already attached to
  // a joint, reuse its collision group
  int group1 = getStartBlock()->getPhysicShape()->group;
  int group2 = getEndBlock()->getPhysicShape()->group;
  int group;
  // we don't handle the case where the two blocks are already
  // attached to other joints and already have a group.
  // TODO::handle that.

  // what about joints mixing both background and normal blocks ?
  if (group1 != 0 && group1 != 1) {
    group = group1;
  } else if (group2 != 0 && group2 != 1) {
    group = group2;
  } else {
    group = getCurrentCollisionGroup();
    setNextCollisionGroup();
  }

  // update only if not background block
  if (group1 != 1)
    getStartBlock()->getPhysicShape()->group = group;
  if (group2 != 1)
    getEndBlock()->getPhysicShape()->group = group;

  m_associatedSpace = i_chipmunkWorld->getSpace();
  switch (getJointType()) {
    case Pivot:
      cpVect v;
      v.x = DynamicPosition().x * CHIP_SCALE_RATIO;
      v.y = DynamicPosition().y * CHIP_SCALE_RATIO;

      m_joint = cpPivotJointNew(body1, body2, v);
      cpSpaceAddJoint(m_associatedSpace, m_joint);
      break;
    case Pin:

      m_joint = cpPinJointNew(body1, body2, cpvzero, cpvzero);
      cpSpaceAddJoint(m_associatedSpace, m_joint);
      break;
    case JointNone:
    default:
      break;
  }
}

void Joint::unloadToPlay() {
  if (m_associatedSpace && m_joint) {
    cpSpaceRemoveJoint(m_associatedSpace, m_joint);
    cpJointFree(m_joint);
    m_joint = nullptr;
  }
  // is cpSpaceRemoveJoint needed ?
}

unsigned int Joint::getCurrentCollisionGroup() {
  return m_currentCollisionGroup;
}

void Joint::setNextCollisionGroup() {
  m_currentCollisionGroup += 1;

  // 0 for normal physic blocks
  // 1 for background physic blocks
  // 2-31 for jointed blocks
  if (m_currentCollisionGroup > 31)
    m_currentCollisionGroup = 2;
}

unsigned int Joint::m_currentCollisionGroup = 2;

/*===========================================
                                Particle Effects
===========================================*/

int ParticlesSource::m_totalOfParticles = 0;
bool ParticlesSource::m_allowParticleGeneration = true;

void ParticlesSource::setAllowParticleGeneration(bool i_value) {
  m_allowParticleGeneration = i_value;
}

ParticlesSource::ParticlesSource(const std::string &i_id,
                                 int i_particleTime_increment)
  : Entity(i_id) {
  m_lastParticleTime = 0;
  m_particleTime_increment = i_particleTime_increment;
  m_type = None;
  m_speciality = ET_PARTICLES_SOURCE;
}

ParticlesSource::~ParticlesSource() {
  deleteParticles();
}

void ParticlesSource::loadToPlay() {
  Entity::loadToPlay("");
  m_lastParticleTime = 0;
}

void ParticlesSource::unloadToPlay() {
  Entity::unloadToPlay();

  deleteParticles();
}

bool ParticlesSource::updateToTime(int i_time,
                                   Vector2f &i_gravity,
                                   PhysicsSettings *i_physicsSettings,
                                   bool i_allowParticules) {
  unsigned int i;

  if (i_allowParticules == false) {
    return false;
  }

  if (i_time > m_lastParticleTime + m_particleTime_increment) {
    i = 0;
    while (i < m_particles.size()) {
      if (i_time > m_particles[i]->KillTime()) {
        addDeadParticle(m_particles[i]);
        m_particles.erase(m_particles.begin() + i);
        m_totalOfParticles--;
      } else {
        m_particles[i]->updateToTime(
          i_time, i_gravity, i_physicsSettings, i_allowParticules);
        i++;
      }
    }
    m_lastParticleTime = i_time;
    return true;
  } else {
    /* return in the past */
    if (i_time < m_lastParticleTime - m_particleTime_increment) {
      deleteParticles();
    }
  }
  return false;
}

bool ParticlesSource::hasReachedMaxParticles() {
  return m_totalOfParticles >= PARTICLESSOURCE_TOTAL_MAX_PARTICLES ||
         m_allowParticleGeneration == false;
}

void ParticlesSource::deleteParticles() {
  for (unsigned int i = 0; i < m_particles.size(); i++) {
    delete m_particles[i];
  }
  m_totalOfParticles -= m_particles.size();
  m_particles.clear();

  for (unsigned int i = 0; i < m_deadParticles.size(); i++) {
    delete m_deadParticles[i];
  }
  m_deadParticles.clear();
}

EntityParticle *ParticlesSource::getExistingParticle() {
  if (m_deadParticles.size() > 0) {
    EntityParticle *pEntityParticle = m_deadParticles.back();
    m_deadParticles.pop_back();

    return pEntityParticle;
  } else {
    return NULL;
  }
}

void ParticlesSource::addDeadParticle(EntityParticle *pEntityParticle) {
  m_deadParticles.push_back(pEntityParticle);
}

/*===========================================
        Multiple Particle Effects (many sprites)
===========================================*/
ParticlesSourceMultiple::ParticlesSourceMultiple(const std::string &i_id,
                                                 int i_particleTime_increment,
                                                 unsigned int i_nbSprite)
  : ParticlesSource(i_id, i_particleTime_increment) {
  // m_sprites.reserve(i_nbSprite);
}

ParticlesSourceMultiple::~ParticlesSourceMultiple() {
  m_sprites.clear();
}

Sprite *ParticlesSourceMultiple::getSprite(unsigned int sprite) const {
  return m_sprites[sprite];
}

void ParticlesSourceMultiple::setSprite(Sprite *i_sprite, unsigned int sprite) {
  // to increment m_sprite size
  m_sprites.push_back(NULL);
  m_sprites[sprite] = i_sprite;
}

void ParticlesSourceMultiple::loadSpriteTextures() {
  std::vector<Sprite *>::iterator it = m_sprites.begin();

  while (it != m_sprites.end()) {
    (*it)->loadTextures();

    ++it;
  }
}

/*===========================================
                                 Smoke Effect
===========================================*/
ParticlesSourceSmoke::ParticlesSourceSmoke(const std::string &i_id)
  : ParticlesSourceMultiple(i_id, PARTICLES_SOURCE_SMOKE_TIME_INCREMENT, 2) {
  m_type = Smoke;
}

ParticlesSourceSmoke::~ParticlesSourceSmoke() {}

void ParticlesSourceSmoke::addParticle(int i_curTime) {
  if (hasReachedMaxParticles() == true)
    return;

  Vector2f v_velocity(NotSoRandom::randomNum(-0.6, 0.6),
                      NotSoRandom::randomNum(0.2, 0.6));
  int v_killTime = i_curTime + 1000;
  std::string v_spriteName = "Smoke1";
  unsigned int v_spriteIndex = 0;
  if (NotSoRandom::randomNum(0, 1) < 0.5) {
    v_spriteName = "Smoke2";
    v_spriteIndex = 1;
  }

  EntityParticle *pEntityParticle = getExistingParticle();
  if (pEntityParticle == NULL) {
    pEntityParticle = new SmokeParticle(
      DynamicPosition(), v_velocity, v_killTime, v_spriteName);
  } else {
    ((SmokeParticle *)pEntityParticle)
      ->init(DynamicPosition(), v_velocity, v_killTime, v_spriteName);
  }

  pEntityParticle->setSpriteIndex(v_spriteIndex);
  m_particles.push_back(pEntityParticle);
  m_totalOfParticles++;
}

bool ParticlesSourceSmoke::updateToTime(int i_time,
                                        Vector2f &i_gravity,
                                        PhysicsSettings *i_physicsSettings,
                                        bool i_allowParticules) {
  if (ParticlesSource::updateToTime(
        i_time, i_gravity, i_physicsSettings, i_allowParticules) == true) {
    /* Generate smoke */
    if (NotSoRandom::randomNum(0, 5) < 1)
      addParticle(i_time);

    return true;
  }
  return false;
}

/*===========================================
                                 Fire Effect
===========================================*/
ParticlesSourceFire::ParticlesSourceFire(const std::string &i_id)
  : ParticlesSource(i_id, PARTICLES_SOURCE_FIRE_TIME_INCREMENT) {
  m_type = Fire;
}

ParticlesSourceFire::~ParticlesSourceFire() {}

void ParticlesSourceFire::addParticle(int i_curTime) {
  if (hasReachedMaxParticles() == true)
    return;

  Vector2f v_velocity(NotSoRandom::randomNum(-1, 1),
                      NotSoRandom::randomNum(0.1, 0.3));
  int v_killTime = i_curTime + 500;
  std::string v_spriteName = SpriteName();

  EntityParticle *pEntityParticle = getExistingParticle();
  if (pEntityParticle == NULL) {
    pEntityParticle =
      new FireParticle(DynamicPosition(), v_velocity, v_killTime, v_spriteName);
  } else {
    ((FireParticle *)pEntityParticle)
      ->init(DynamicPosition(), v_velocity, v_killTime, v_spriteName);
  }

  m_particles.push_back(pEntityParticle);
  m_totalOfParticles++;
}

bool ParticlesSourceFire::updateToTime(int i_time,
                                       Vector2f &i_gravity,
                                       PhysicsSettings *i_physicsSettings,
                                       bool i_allowParticules) {
  if (ParticlesSource::updateToTime(
        i_time, i_gravity, i_physicsSettings, i_allowParticules) == true) {
    /* Generate fire */
    /* maximum 5s for a fire particule, but it can be destroyed before */
    addParticle(i_time);
    return true;
  }
  return false;
}
/*===========================================
                                 Star Effect
===========================================*/
ParticlesSourceStar::ParticlesSourceStar(const std::string &i_id)
  : ParticlesSource(i_id, PARTICLES_SOURCE_STAR_TIME_INCREMENT) {
  setSpriteName("Star");
  m_type = Star;
}

ParticlesSourceStar::~ParticlesSourceStar() {}

void ParticlesSourceStar::addParticle(int i_curTime) {
  if (hasReachedMaxParticles() == true)
    return;

  Vector2f v_velocity(randomNum(-2, 2), randomNum(0, 2));
  int v_killTime = i_curTime + 500;
  std::string v_spriteName = SpriteName();

  EntityParticle *pEntityParticle = getExistingParticle();
  if (pEntityParticle == NULL) {
    pEntityParticle =
      new StarParticle(DynamicPosition(), v_velocity, v_killTime, v_spriteName);
  } else {
    ((StarParticle *)pEntityParticle)
      ->init(DynamicPosition(), v_velocity, v_killTime, v_spriteName);
  }

  m_particles.push_back(pEntityParticle);
  m_totalOfParticles++;
}
/*===========================================
                                 Debris Effect
===========================================*/
ParticlesSourceDebris::ParticlesSourceDebris(const std::string &i_id)
  : ParticlesSource(i_id, PARTICLES_SOURCE_DEBRIS_TIME_INCREMENT) {
  // debris are created outside of an entity, so we have to put the
  // sprite name by hand...
  setSpriteName("Debris1");
  m_type = Debris;
}

ParticlesSourceDebris::~ParticlesSourceDebris() {}

void ParticlesSourceDebris::addParticle(int i_curTime) {
  if (hasReachedMaxParticles())
    return;

  Vector2f v_velocity(randomNum(-2, 2), randomNum(0, 2));
  int v_killTime = i_curTime + 300;
  std::string v_spriteName = SpriteName();

  EntityParticle *pEntityParticle = getExistingParticle();
  if (pEntityParticle == NULL) {
    pEntityParticle = new DebrisParticle(
      DynamicPosition(), v_velocity, v_killTime, v_spriteName);
  } else {
    ((DebrisParticle *)pEntityParticle)
      ->init(DynamicPosition(), v_velocity, v_killTime, v_spriteName);
  }

  m_particles.push_back(pEntityParticle);
  m_totalOfParticles++;
}

bool ParticlesSourceDebris::updateToTime(int i_time,
                                         Vector2f &i_gravity,
                                         PhysicsSettings *i_physicsSettings,
                                         bool i_allowParticules) {
  return ParticlesSource::updateToTime(
    i_time, i_gravity, i_physicsSettings, i_allowParticules);
}

/*===========================================
                                 Sparkle Effect
===========================================*/
ParticlesSourceSparkle::ParticlesSourceSparkle(const std::string &i_id)
  : ParticlesSource(i_id, PARTICLES_SOURCE_SPARKLE_TIME_INCREMENT) {
  m_type = Sparkle;
  m_last_time = 0;
}

ParticlesSourceSparkle::~ParticlesSourceSparkle() {}

void ParticlesSourceSparkle::addParticle(int i_curTime) {
  if (hasReachedMaxParticles() == true)
    return;

  Vector2f v_velocity(NotSoRandom::randomNum(-4, 4),
                      NotSoRandom::randomNum(0, 2));
  int v_killTime = i_curTime + 500;
  std::string v_spriteName = SpriteName();

  EntityParticle *pEntityParticle = getExistingParticle();
  if (pEntityParticle == NULL) {
    pEntityParticle = new SparkleParticle(
      DynamicPosition(), v_velocity, v_killTime, v_spriteName);
  } else {
    ((SparkleParticle *)pEntityParticle)
      ->init(DynamicPosition(), v_velocity, v_killTime, v_spriteName);
  }

  m_particles.push_back(pEntityParticle);
  m_totalOfParticles++;
}

bool ParticlesSourceSparkle::updateToTime(int i_time,
                                          Vector2f &i_gravity,
                                          PhysicsSettings *i_physicsSettings,
                                          bool i_allowParticules) {
  /*if(ParticlesSource::updateToTime(i_time, i_gravity, i_physicsSettings) ==
true) {
addParticle(i_time);
return true;
}*/
  ParticlesSource::updateToTime(
    i_time, i_gravity, i_physicsSettings, i_allowParticules);
  if (i_time > m_last_time + NotSoRandom::randomNum(PS_SPARKLE_DELAY_MIN,
                                                    PS_SPARKLE_DELAY_MAX)) {
    m_last_time = i_time;
    for (int i = 0; i < NotSoRandom::randomNum(4, 10); i++) {
      addParticle(i_time);
    }
    // return true;
  }
  return false;
}

/*===========================================
                                 Base for an single particle
===========================================*/
EntityParticle::EntityParticle()
  : Entity("") {
  m_spriteIndex = 0;
}

EntityParticle::~EntityParticle() {}

void EntityParticle::init(const Vector2f &i_position,
                          const Vector2f &i_velocity,
                          int i_killTime) {
  setDynamicPosition(i_position);
  m_velocity = i_velocity;
  m_killTime = i_killTime;
  m_acceleration = Vector2f(0, 0);
  m_ang = 0;
  m_angAcc = 0;
  m_angVel = 0;
  setColor(TColor(255, 255, 255, 255));
  setSize(0.5);
}

bool EntityParticle::updateToTime(int i_time,
                                  Vector2f &i_gravity,
                                  PhysicsSettings *i_physicsSettings,
                                  bool i_allowParticules) {
  if (i_allowParticules == false) {
    return false;
  }

  float v_timeStep = 0.025;

  m_velocity += m_acceleration * v_timeStep;
  setDynamicPosition(DynamicPosition() + m_velocity * v_timeStep);
  m_angVel += m_angAcc * v_timeStep;
  m_ang += m_angVel * v_timeStep;

  return true;
}

/*===========================================
                                Single Smoke Particle
===========================================*/
SmokeParticle::SmokeParticle(const Vector2f &i_position,
                             const Vector2f &i_velocity,
                             int i_killTime,
                             std::string i_spriteName)
  : EntityParticle() {
  init(i_position, i_velocity, i_killTime, i_spriteName);
}

SmokeParticle::~SmokeParticle() {}

void SmokeParticle::init(const Vector2f &i_position,
                         const Vector2f &i_velocity,
                         int i_killTime,
                         std::string i_spriteName) {
  EntityParticle::init(i_position, i_velocity, i_killTime);

  int cc = (int)NotSoRandom::randomNum(0, 50);
  setColor(TColor(cc, cc, cc, 255));
  setSize(NotSoRandom::randomNum(0, 0.2));
  m_angVel = NotSoRandom::randomNum(-60, 60);
  setSpriteName(i_spriteName);
}

bool SmokeParticle::updateToTime(int i_time,
                                 Vector2f &i_gravity,
                                 PhysicsSettings *i_physicsSettings,
                                 bool i_allowParticules) {
  EntityParticle::updateToTime(
    i_time, i_gravity, i_physicsSettings, i_allowParticules);

  if (i_allowParticules == false) {
    return false;
  }

  float v_timeStep = 0.025;
  TColor v_color(Color());

  /* grow */
  setSize(Size() + v_timeStep * 1.0f);
  /* accelerate upwards */
  m_acceleration = Vector2f(0.2, 0.5);

  int v_c =
    (Color().Red() + (int)(NotSoRandom::randomNum(40, 50) * v_timeStep)) & 0xFF;
  v_color.setRed(v_c);
  v_color.setBlue(v_c);
  v_color.setGreen(v_c);

  int v_a = Color().Alpha() - (int)(120.0f * v_timeStep);
  if (v_a >= 0) {
    v_color.setAlpha(v_a);
    setColor(v_color);
  } else {
    m_killTime = i_time;
  }

  return false;
}

/*===========================================
                                Single Fire Particle
===========================================*/
FireParticle::FireParticle(const Vector2f &i_position,
                           const Vector2f &i_velocity,
                           int i_killTime,
                           std::string i_spriteName)
  : EntityParticle() {
  init(i_position, i_velocity, i_killTime, i_spriteName);
}

FireParticle::~FireParticle() {}

void FireParticle::init(const Vector2f &i_position,
                        const Vector2f &i_velocity,
                        int i_killTime,
                        std::string i_spriteName) {
  EntityParticle::init(i_position, i_velocity, i_killTime);

  m_fireSeed = NotSoRandom::randomNum(0, 100);
  setSize(0.17);
  setColor(TColor(255, 255, 0, 255));
  setSpriteName(i_spriteName);
}

bool FireParticle::updateToTime(int i_time,
                                Vector2f &i_gravity,
                                PhysicsSettings *i_physicsSettings,
                                bool i_allowParticules) {
  EntityParticle::updateToTime(
    i_time, i_gravity, i_physicsSettings, i_allowParticules);

  if (i_allowParticules == false) {
    return false;
  }

  float v_timeStep = 0.040;
  TColor v_color(Color());

  int v_g =
    Color().Green() - (int)(NotSoRandom::randomNum(190, 210) * v_timeStep);
  v_color.setGreen(v_g < 0 ? 0 : v_g);

  int v_b =
    Color().Blue() - (int)(NotSoRandom::randomNum(400, 400) * v_timeStep);
  v_color.setBlue(v_b < 0 ? 0 : v_b);

  int v_a = Color().Alpha() - (int)(250.0f * v_timeStep);
  if (v_a >= 0) {
    v_color.setAlpha(v_a);
    setColor(v_color);
  } else {
    m_killTime = i_time;
  }

  m_velocity.x =
    sin((i_time + m_fireSeed) * NotSoRandom::randomNum(5, 15)) * 0.004f +
    sin((i_time - m_fireSeed) * 0.1) * 0.3;
  m_acceleration.y = 3.0;

  return false;
}

/*===========================================
                                Single Star Particle
===========================================*/
StarParticle::StarParticle(const Vector2f &i_position,
                           const Vector2f &i_velocity,
                           int i_killTime,
                           std::string i_spriteName)
  : EntityParticle() {
  init(i_position, i_velocity, i_killTime, i_spriteName);
}

StarParticle::~StarParticle() {}

void StarParticle::init(const Vector2f &i_position,
                        const Vector2f &i_velocity,
                        int i_killTime,
                        std::string i_spriteName) {
  EntityParticle::init(i_position, i_velocity, i_killTime);

  m_angVel = NotSoRandom::randomNum(-60, 60);
  m_acceleration = Vector2f(0, -4);
  setSpriteName(i_spriteName);
}
/*===========================================
                                Single Debris Particle
===========================================*/
DebrisParticle::DebrisParticle(const Vector2f &i_position,
                               const Vector2f &i_velocity,
                               int i_killTime,
                               std::string i_spriteName)
  : EntityParticle() {
  init(i_position, i_velocity, i_killTime, i_spriteName);
}

DebrisParticle::~DebrisParticle() {}

void DebrisParticle::init(const Vector2f &i_position,
                          const Vector2f &i_velocity,
                          int i_killTime,
                          std::string i_spriteName) {
  EntityParticle::init(i_position, i_velocity, i_killTime);

  m_angVel = NotSoRandom::randomNum(-60, 60);
  m_acceleration = Vector2f(0, -4);
  int cc = (int)NotSoRandom::randomNum(0, 250);
  setColor(TColor(cc, cc, cc, 255));
  m_velocity *= NotSoRandom::randomNum(1.5, 0.5);
  m_velocity += Vector2f(NotSoRandom::randomNum(-0.2, 0.2),
                         NotSoRandom::randomNum(-0.2, 0.2));
  setSize(NotSoRandom::randomNum(0.02f, 0.04f));
  setSpriteName(i_spriteName);
}

bool DebrisParticle::updateToTime(int i_time,
                                  Vector2f &i_gravity,
                                  PhysicsSettings *i_physicsSettings,
                                  bool i_allowParticules) {
  EntityParticle::updateToTime(
    i_time, i_gravity, i_physicsSettings, i_allowParticules);

  if (i_allowParticules == false) {
    return false;
  }

  float v_timeStep = 0.025;
  TColor v_color(Color());

  m_acceleration = i_gravity * (-5.5f / -(i_physicsSettings->WorldGravity()));

  int v_a = v_color.Alpha() - (int)(120.0f * v_timeStep);
  if (v_a >= 0) {
    v_color.setAlpha(v_a);
    setColor(v_color);
  } else {
    m_killTime = i_time;
  }

  return true;
}
/*===========================================
                                Single Sparkle Particle
===========================================*/
SparkleParticle::SparkleParticle(const Vector2f &i_position,
                                 const Vector2f &i_velocity,
                                 int i_killTime,
                                 std::string i_spriteName)
  : EntityParticle() {
  init(i_position, i_velocity, i_killTime, i_spriteName);
}

SparkleParticle::~SparkleParticle() {}

void SparkleParticle::init(const Vector2f &i_position,
                           const Vector2f &i_velocity,
                           int i_killTime,
                           std::string i_spriteName) {
  EntityParticle::init(i_position, i_velocity, i_killTime);

  m_SparkleSeed = NotSoRandom::randomNum(0, 100);
  setSize(0.05);
  setColor(TColor(255, 255, 0, 255));
  setSpriteName(i_spriteName);
}

bool SparkleParticle::updateToTime(int i_time,
                                   Vector2f &i_gravity,
                                   PhysicsSettings *i_physicsSettings,
                                   bool i_allowParticules) {
  EntityParticle::updateToTime(
    i_time, i_gravity, i_physicsSettings, i_allowParticules);

  if (i_allowParticules == false) {
    return false;
  }

  float v_timeStep = 0.040;
  TColor v_color(Color());

  int v_g =
    Color().Green() - (int)(NotSoRandom::randomNum(190, 210) * v_timeStep);
  v_color.setGreen(v_g < 0 ? 0 : v_g);

  int v_b =
    Color().Blue() - (int)(NotSoRandom::randomNum(400, 400) * v_timeStep);
  v_color.setBlue(v_b < 0 ? 0 : v_b);

  int v_a = Color().Alpha() - (int)(250.0f * (v_timeStep / 1.5));
  if (v_a >= 0) {
    v_color.setAlpha(v_a);
    setColor(v_color);
  } else {
    m_killTime = i_time;
  }

  // m_velocity.x = cos((i_time + m_SparkleSeed)) * 0.1f;

  m_acceleration.y = -1.0;

  return false;
}
