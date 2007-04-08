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

#include "Entity.h"

#include "../VXml.h"
#include "../PhysSettings.h"

Entity::Entity(const std::string& i_id) {
  m_id          = i_id;
  m_spriteName  = ENTITY_DEFAULT_SPRITE_NAME;
  m_size        = ENTITY_DEFAULT_SIZE;
  m_width       = -1.0;
  m_height      = -1.0;
  m_drawAngle    = 0.0;
  m_drawReversed = false;
  m_z           = ENTITY_DEFAULT_Z;
  m_doesKill    = false;
  m_doesMakeWin = false;
  m_isToTake    = false;
  m_BBox.reset();
  m_isBBoxDirty = true;
}

Entity::~Entity() {
}

void Entity::loadToPlay() {
  m_dynamicPosition = m_initialPosition;
  /* make every entity alive */
  setAlive(true);
  m_isBBoxDirty = true;
}

void Entity::unloadToPlay() {
}

std::string Entity::Id() const {
  return m_id;
}

float Entity::Size() const {
  return m_size;
}

bool Entity::IsToTake() const {
  return m_isToTake;
}

bool Entity::DoesMakeWin() const {
  return m_doesMakeWin;
}

bool Entity::DoesKill() const {
  return m_doesKill;
}

const TColor& Entity::Color() const {
  return m_color;
}

EntitySpeciality Entity::Speciality() const {
  if(IsToTake())    return ET_ISTOTAKE;
  if(DoesKill())    return ET_KILL;
  if(DoesMakeWin()) return ET_MAKEWIN;
  if(SpriteName() == "PlayerStart") return ET_ISSTART;

  return ET_NONE;
}

Vector2f Entity::InitialPosition() const {
  return m_initialPosition;
}

float Entity::Z() const {
  return m_z;
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

void Entity::setInitialPosition(const Vector2f& i_initialPosition) {
  m_initialPosition = i_initialPosition;
  m_isBBoxDirty = true;
}

std::string Entity::SpriteName() const {
  return m_spriteName;
}

bool Entity::isAlive() const {
  return m_isAlive;
}

void Entity::setSpriteName(const std::string& i_spriteName) {
  m_spriteName = i_spriteName;
}

Vector2f Entity::DynamicPosition() const {
  return m_dynamicPosition;  
}

void Entity::setSize(float i_size) {
  m_size = i_size;
  m_isBBoxDirty = true;
}

void Entity::setSpeciality(EntitySpeciality i_speciality) {
  m_doesMakeWin = i_speciality == ET_MAKEWIN;
  m_doesKill    = i_speciality == ET_KILL;
  m_isToTake    = i_speciality == ET_ISTOTAKE;
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

void Entity::setColor(const TColor& i_color) {
  m_color = i_color;
}

void Entity::setAlive(bool alive) {
  m_isAlive = alive;
}


bool Entity::updateToTime(float i_time, Vector2f i_gravity) {
  return false;
}

AABB& Entity::getAABB()
{
  if(m_isBBoxDirty == true){
    m_BBox.reset();
    float size = Size();
    m_BBox.addPointToAABB2f(m_dynamicPosition.x-size,
			    m_dynamicPosition.y-size);
    m_BBox.addPointToAABB2f(m_dynamicPosition.x+size,
			    m_dynamicPosition.y+size);

    m_isBBoxDirty = false;
  }

  return m_BBox;
}


ParticlesSource::ParticlesSource(const std::string& i_id, float i_particleTime_increment)
  : Entity(i_id) {
  m_lastParticleTime       = 0.0;
  m_particleTime_increment = i_particleTime_increment;
}

ParticlesSource::~ParticlesSource() {
  deleteParticles();
}

void ParticlesSource::loadToPlay() {
  Entity::loadToPlay();
  m_lastParticleTime = 0.0;
} 

void ParticlesSource::unloadToPlay() {
  Entity::unloadToPlay();

  deleteParticles();
}

bool ParticlesSource::updateToTime(float i_time, Vector2f i_gravity) {
  unsigned int i;

  if(i_time > m_lastParticleTime + m_particleTime_increment) {  
    i = 0;
    while(i < m_particles.size()) {
      if(i_time > m_particles[i]->KillTime()) {
	delete m_particles[i];
	m_particles.erase(m_particles.begin() + i);
      } else {
	m_particles[i]->updateToTime(i_time, i_gravity);
	i++;
      }
    }
    m_lastParticleTime = i_time;
    return true;
  } else {
    /* return in the past */
    if(i_time < m_lastParticleTime - m_particleTime_increment) {
      deleteParticles();   
    }
  }
  return false;
}

ParticlesSourceSmoke::ParticlesSourceSmoke(const std::string& i_id)
  : ParticlesSource(i_id, PARTICLES_SOURCE_SMOKE_TIME_INCREMENT) {

}

ParticlesSourceSmoke::~ParticlesSourceSmoke() {
}

ParticlesSourceFire::ParticlesSourceFire(const std::string& i_id)
  : ParticlesSource(i_id, PARTICLES_SOURCE_FIRE_TIME_INCREMENT) {
  setSpriteName("Fire");
}

ParticlesSourceFire::~ParticlesSourceFire() {
}

ParticlesSourceStar::ParticlesSourceStar(const std::string& i_id)
  : ParticlesSource(i_id, PARTICLES_SOURCE_STAR_TIME_INCREMENT) {
  setSpriteName("Star");
}

ParticlesSourceStar::~ParticlesSourceStar() {
}

ParticlesSourceDebris::ParticlesSourceDebris(const std::string& i_id)
  : ParticlesSource(i_id, PARTICLES_SOURCE_DEBRIS_TIME_INCREMENT) {
  setSpriteName("Debris1");
}

ParticlesSourceDebris::~ParticlesSourceDebris() {
}

bool ParticlesSourceSmoke::updateToTime(float i_time, Vector2f i_gravity) {
  if(ParticlesSource::updateToTime(i_time, i_gravity)) {
    /* Generate smoke */
    if(randomNum(0,5) < 1) {
      if(randomNum(0,1) < 0.5) {
	      addParticle(Vector2f(randomNum(-0.6,0.6), randomNum(0.2,0.6)), i_time + 10.0, "Smoke1");
      } else {
	      addParticle(Vector2f(randomNum(-0.6,0.6), randomNum(0.2,0.6)), i_time + 10.0, "Smoke2");
      }
    }
    return true;
  }
  return false;
}

bool SmokeParticle::updateToTime(float i_time, Vector2f i_gravity) {
  EntityParticle::updateToTime(i_time, i_gravity);

  float v_timeStep = 0.025;
  TColor v_color(Color());

  setSize(Size() + v_timeStep * 1.0f); /* grow */
  m_acceleration = Vector2f(0.2, 0.5);  /* accelerate upwards */

  int v_c = Color().Red() + (int)(randomNum(40,50) * v_timeStep);
  v_color.setRed(v_c > 255 ? 255 : v_c);
  v_color.setBlue(v_c > 255 ? 255 : v_c);
  v_color.setGreen(v_c > 255 ? 255 : v_c);

  int v_a = Color().Alpha() - (int)(120.0f * v_timeStep);
  if(v_a >= 0) {
    v_color.setAlpha(v_a);
    setColor(v_color);
  } else {
    m_killTime = i_time;
  }

  return false;
}

bool FireParticle::updateToTime(float i_time, Vector2f i_gravity) {
  EntityParticle::updateToTime(i_time, i_gravity);

  float v_timeStep = 0.035;
  TColor v_color(Color());

  int v_g = Color().Green() - (int)(randomNum(190,210) * v_timeStep);
  v_color.setGreen(v_g < 0 ? 0 : v_g);

  int v_b = Color().Blue()  - (int)(randomNum(400,400) * v_timeStep);
  v_color.setBlue(v_b < 0 ? 0 : v_b);

  int v_a = Color().Alpha() - (int)(250.0f * v_timeStep);
  if(v_a >= 0) {
    v_color.setAlpha(v_a);
    setColor(v_color);
  } else {
    m_killTime = i_time;
  }
      
  m_velocity.x = sin((i_time + m_fireSeed) * randomNum(5,15)) * 0.8f
    +
    sin((i_time - m_fireSeed) * 10) * 0.3;
  m_acceleration.y = 2.0;

  return false;
}

bool ParticlesSourceFire::updateToTime(float i_time, Vector2f i_gravity) {
  if(ParticlesSource::updateToTime(i_time, i_gravity)) {
    /* Generate fire */
    for(int k=0;k<3;k++) {
      /* maximum 10s for a fire particule, but it can be destroyed before */
      ParticlesSource::addParticle(Vector2f(randomNum(-1,1),randomNum(0.1,0.3)), i_time + 10.0);
    }
    return true;
  }
  return false;
}

bool ParticlesSourceDebris::updateToTime(float i_time, Vector2f i_gravity) {
  return ParticlesSource::updateToTime(i_time, i_gravity);
}

bool EntityParticle::updateToTime(float i_time, Vector2f i_gravity) {
  float v_timeStep = 0.025;

  m_velocity += m_acceleration * v_timeStep;
  setDynamicPosition(DynamicPosition() + m_velocity * v_timeStep);
  m_angVel   += m_angAcc       * v_timeStep;
  m_ang      += m_angVel       * v_timeStep;

  return true;
}

float EntityParticle::KillTime() const {
  return m_killTime;
}

void Entity::setDynamicPosition(const Vector2f& i_dynamicPosition) {
  m_dynamicPosition = i_dynamicPosition;
  m_isBBoxDirty = true;
}

EntityParticle::EntityParticle(const Vector2f& i_position, const Vector2f i_velocity, float i_killTime)
: Entity("") {
  setDynamicPosition(i_position);
  m_velocity 	 = i_velocity;
  m_killTime 	 = i_killTime;
  m_acceleration = Vector2f(0,0);
  m_ang          = 0;
  m_angAcc       = 0;
  m_angVel       = 0;
  setColor(TColor(255, 255, 255, 255));
  setSize(0.5);
}

EntityParticle::~EntityParticle() {
}

void Entity::saveXml(vapp::FileHandle *i_pfh) {
  vapp::FS::writeLineF(i_pfh,"\t<entity id=\"%s\" typeid=\"%s\">", Id().c_str(), Entity::SpecialityToStr(Speciality()).c_str());
  vapp::FS::writeLineF(i_pfh,"\t\t<size r=\"%f\"/>",Size());
  vapp::FS::writeLineF(i_pfh,"\t\t<position x=\"%f\" y=\"%f\"/>", InitialPosition().x, InitialPosition().y);      

  vapp::FS::writeLineF(i_pfh,"\t\t<param name=\"%s\" value=\"%.2f\"/>",
                       "z", Z());

  if(Speciality() == ET_NONE) {
    vapp::FS::writeLineF(i_pfh,"\t\t<param name=\"%s\" value=\"%.2f\"/>",
			 "name", SpriteName().c_str());
  }

  if(Speciality() == ET_PARTICLES_SOURCE) {
    vapp::FS::writeLineF(i_pfh,"\t\t<param name=\"%s\" value=\"%.2f\"/>",
			 "type", SpriteName().c_str());
  }

  vapp::FS::writeLineF(i_pfh,"\t</entity>");
}

EntitySpeciality Entity::SpecialityFromStr(std::string i_typeStr) {
  if(i_typeStr == "PlayerStart")    return ET_ISSTART;
  if(i_typeStr == "EndOfLevel")     return ET_MAKEWIN;
  if(i_typeStr == "Wrecker")        return ET_KILL;
  if(i_typeStr == "Strawberry")     return ET_ISTOTAKE;
  if(i_typeStr == "ParticleSource") return ET_PARTICLES_SOURCE;

  return ET_NONE;
}

std::string Entity::SpecialityToStr(EntitySpeciality i_speciality) {
  switch(i_speciality) {
    case ET_ISSTART :
      return "PlayerStart";
      break;
    case ET_MAKEWIN :
      return "EndOfLevel";
      break;
    case ET_KILL :
      return "Wrecker";
      break;
    case ET_ISTOTAKE :
      return "Strawberry";
      break;
    case ET_PARTICLES_SOURCE :
      return "ParticleSource";
      break;
  default:
      return "Sprite";
  }
}

Entity* Entity::readFromXml(TiXmlElement *pElem) {
  std::string v_id;
  std::string v_typeId;
  EntitySpeciality  v_speciality;
  Vector2f    v_position;
  float       v_size   = 0.2;
  float       v_height = -1.0;
  float       v_width  = -1.0;
  float       v_angle  = -1.0;
  bool        v_reversed = false;
  float       v_z      = ENTITY_DEFAULT_Z;
  std::string v_spriteName;
  std::string v_typeName;

  /* read xml information */
  v_id         = vapp::XML::getOption(pElem,"id");
  v_typeId     = vapp::XML::getOption(pElem,"typeid");
  v_speciality = Entity::SpecialityFromStr(v_typeId);
  TiXmlElement *pPosElem = pElem->FirstChildElement("position");
  if(pPosElem != NULL) {
    v_position.x = atof(vapp::XML::getOption(pPosElem,"x","0").c_str());
    v_position.y = atof(vapp::XML::getOption(pPosElem,"y","0").c_str());
    v_angle      = atof(vapp::XML::getOption(pPosElem,"angle","-1.0").c_str());
    v_reversed   = vapp::XML::getOption(pPosElem,"reversed","false") == "true";
  }
  TiXmlElement *pSizeElem = pElem->FirstChildElement("size");
  if(pSizeElem != NULL) {
    v_size = (atof(vapp::XML::getOption(pSizeElem,"r","0.2").c_str()));
    v_width = atof(vapp::XML::getOption(pSizeElem,"width","-1.0").c_str());
    v_height = atof(vapp::XML::getOption(pSizeElem,"height","-1.0").c_str());
  }
  /* Get parameters */
  std::string v_paramName;
  std::string v_paramValue;
  for(TiXmlElement *pParamElem = pElem->FirstChildElement("param"); pParamElem!=NULL;
      pParamElem=pParamElem->NextSiblingElement("param")) {   
    v_paramName  = vapp::XML::getOption(pParamElem,"name");
    v_paramValue = vapp::XML::getOption(pParamElem,"value");
    if(v_paramName == "z") {
      v_z = (atof(v_paramValue.c_str()));
    } else if(v_paramName == "name") {
      v_spriteName = v_paramValue;
    } else if(v_paramName == "type") {
      v_typeName = v_paramValue;
    }
  }

  /* Create the entity */
  Entity *v_entity;

  if(v_speciality == ET_PARTICLES_SOURCE) {
    if       (v_typeName == "Smoke") {
      v_entity = new ParticlesSourceSmoke(v_id);
    } else if(v_typeName == "Fire")   {
      v_entity = new ParticlesSourceFire(v_id);
    } else if(v_typeName == "Star")   {
      v_entity = new ParticlesSourceStar(v_id);
    } else if(v_typeName == "Debris") {
      v_entity = new ParticlesSourceDebris(v_id);
    } else {
      throw Exception("Entity " + v_id + " has an invalid type name");
    }
  } else {
    v_entity = new Entity(v_id);
  }

  switch(v_speciality) {
  case ET_NONE:
    v_entity->setSpriteName(v_spriteName);
    break;
  case ET_PARTICLES_SOURCE:
      v_entity->setSpriteName(v_typeName);
    break;
  default:
    v_entity->setSpriteName(v_typeId);
  }
  v_entity->setSpeciality(v_speciality);
  v_entity->setInitialPosition(v_position);
  v_entity->setSize(v_size);
  if(v_width > 0.0) {
    v_entity->setWidth(v_width);
  }
  if(v_height > 0.0) {
    v_entity->setHeight(v_height);
  }
  if(v_angle > 0.0) {
    v_entity->setDrawAngle(v_angle);
  }
  v_entity->setDrawReversed(v_reversed);
  v_entity->setZ(v_z);

  return v_entity;
}

void Entity::saveBinary(vapp::FileHandle *i_pfh) {
  vapp::FS::writeString(i_pfh,   Id());
  vapp::FS::writeString(i_pfh,   Entity::SpecialityToStr(Speciality()));
  vapp::FS::writeFloat_LE(i_pfh, Size());
  vapp::FS::writeFloat_LE(i_pfh, Width());       
  vapp::FS::writeFloat_LE(i_pfh, Height()); 
  vapp::FS::writeFloat_LE(i_pfh, InitialPosition().x);
  vapp::FS::writeFloat_LE(i_pfh, InitialPosition().y);
  vapp::FS::writeFloat_LE(i_pfh, DrawAngle());
  vapp::FS::writeBool(i_pfh,  DrawReversed());
  
  switch(Speciality()) {
  case ET_NONE:
  case ET_PARTICLES_SOURCE:
    vapp::FS::writeByte(i_pfh, 0x02);
    break;
  default:
    vapp::FS::writeByte(i_pfh, 0x01);
  }

  std::ostringstream v_z;
  v_z << Z();
  vapp::FS::writeString(i_pfh, "z");
  vapp::FS::writeString(i_pfh, v_z.str());

  if(Speciality() == ET_NONE) {
    vapp::FS::writeString(i_pfh, "name");
    vapp::FS::writeString(i_pfh, SpriteName());
  }

  if(Speciality() == ET_PARTICLES_SOURCE) {
    vapp::FS::writeString(i_pfh, "type");
    vapp::FS::writeString(i_pfh, SpriteName());
  }
}

Entity* Entity::readFromBinary(vapp::FileHandle *i_pfh) {
  std::string v_id;
  std::string v_typeId;
  EntitySpeciality  v_speciality;
  Vector2f    v_position;
  float       v_size   = 0.2;
  float       v_height = -1.0;
  float       v_width  = -1.0;
  float       v_angle  = -1.0;
  bool        v_reversed = false;
  float       v_z      = ENTITY_DEFAULT_Z;
  std::string v_spriteName;
  std::string v_typeName;

  /* read values */
  v_id         = vapp::FS::readString(i_pfh);
  v_typeId     = vapp::FS::readString(i_pfh);
  v_speciality = Entity::SpecialityFromStr(v_typeId);
  v_size       = vapp::FS::readFloat_LE(i_pfh);
  v_width      = vapp::FS::readFloat_LE(i_pfh);
  v_height     = vapp::FS::readFloat_LE(i_pfh);
  v_position.x = vapp::FS::readFloat_LE(i_pfh);
  v_position.y = vapp::FS::readFloat_LE(i_pfh);
  v_angle      = vapp::FS::readFloat_LE(i_pfh);
  v_reversed   = vapp::FS::readBool(i_pfh);
  std::string v_paramName;
  std::string v_paramValue;
  int nNumParams = vapp::FS::readByte(i_pfh);
  for(int j=0;j<nNumParams;j++) {
    v_paramName  = vapp::FS::readString(i_pfh);
    v_paramValue = vapp::FS::readString(i_pfh);

    if(v_paramName == "z") {
      v_z = atof(v_paramValue.c_str());
    } else if(v_paramName == "name") {
      v_spriteName = v_paramValue;
    } else if(v_paramName == "type") {
      v_typeName   = v_paramValue;
    }
  }

  /* Create the entity */
  Entity *v_entity;

  if(v_speciality == ET_PARTICLES_SOURCE) {
    if       (v_typeName == "Smoke") {
      v_entity = new ParticlesSourceSmoke(v_id);
    } else if(v_typeName == "Fire")   {
      v_entity = new ParticlesSourceFire(v_id);
    } else if(v_typeName == "Star")   {
      v_entity = new ParticlesSourceStar(v_id);
    } else if(v_typeName == "Debris") {
      v_entity = new ParticlesSourceDebris(v_id);
    } else {
      throw Exception("Entity " + v_id + " has an invalid type name");
    }
  } else {
    v_entity = new Entity(v_id);
  }

  switch(v_speciality) {
  case ET_NONE:
    v_entity->setSpriteName(v_spriteName);
    break;
  case ET_PARTICLES_SOURCE:
      v_entity->setSpriteName(v_typeName);
    break;
  default:
    v_entity->setSpriteName(v_typeId);
  }

  v_entity->setSpeciality(v_speciality);
  v_entity->setInitialPosition(v_position);
  v_entity->setSize(v_size);
  if(v_width > 0.0) {
    v_entity->setWidth(v_width);
  }
  if(v_height > 0.0) {
    v_entity->setHeight(v_height);
  }
  if(v_angle > 0.0) {
    v_entity->setDrawAngle(v_angle);
  }
  v_entity->setDrawReversed(v_reversed);
  v_entity->setZ(v_z);

  return v_entity;
}

void ParticlesSource::deleteParticles() {
  for(unsigned int i=0; i<m_particles.size(); i++) {
    delete m_particles[i];
  }
  m_particles.clear();
}

std::vector<EntityParticle *>& ParticlesSource::Particles() {
  return m_particles;
}

EntitySpeciality ParticlesSource::Speciality() const {
  return ET_PARTICLES_SOURCE;
}

float EntityParticle::Angle() const {
  return m_ang;
}

void ParticlesSource::addParticle(Vector2f i_velocity, float i_killTime) {
  addParticle(i_velocity, i_killTime, SpriteName());
}

void ParticlesSourceStar::addParticle(Vector2f i_velocity, float i_killTime, std::string i_spriteName) {
  m_particles.push_back(new StarParticle(DynamicPosition(), i_killTime, SpriteName()));
}

StarParticle::StarParticle(const Vector2f& i_position, float i_killTime, std::string i_spriteName)
  : EntityParticle(i_position, Vector2f(randomNum(-2,2),randomNum(0,2)), i_killTime) {
  m_angVel       = randomNum(-60,60);
  m_acceleration = Vector2f(0,-4);
  setSpriteName(i_spriteName);
}

StarParticle::~StarParticle() {
}

DebrisParticle::DebrisParticle(const Vector2f& i_position, const Vector2f i_velocity, float i_killTime, std::string i_spriteName)
  : EntityParticle(i_position, Vector2f(randomNum(-2,2),randomNum(0,2)), i_killTime) {
  m_angVel       = randomNum(-60,60);
  m_acceleration = Vector2f(0,-4);
  int cc     	 = (int) randomNum(0, 250);
  setColor(TColor(cc, cc, cc, 255));
  m_velocity 	*= randomNum(1.5, 0.5);
  m_velocity 	+= Vector2f(randomNum(-0.2,0.2), randomNum(-0.2,0.2));
  setSize(randomNum(0.02f,0.04f));
  setSpriteName(i_spriteName);
}

DebrisParticle::~DebrisParticle() {
}

bool DebrisParticle::updateToTime(float i_time, Vector2f i_gravity) {
  EntityParticle::updateToTime(i_time, i_gravity);

  float v_timeStep = 0.025;
  TColor v_color(Color());

  m_acceleration = i_gravity * (-5.5f / PHYS_WORLD_GRAV);

  int v_a = v_color.Alpha() - (int)(120.0f * v_timeStep);
  if(v_a >= 0) {
    v_color.setAlpha(v_a);
    setColor(v_color);
  } else {
    m_killTime = i_time;
  }

  return true;
}

FireParticle::FireParticle(const Vector2f& i_position, const Vector2f i_velocity, float i_killTime, std::string i_spriteName)
  : EntityParticle(i_position, i_velocity, i_killTime) {
  m_fireSeed = randomNum(0,100);
  setSize(0.17);
  setColor(TColor(255,255,0,255));
  setSpriteName(i_spriteName);
}

FireParticle::~FireParticle() {
}

SmokeParticle::SmokeParticle(const Vector2f& i_position, const Vector2f i_velocity, float i_killTime, std::string i_spriteName) 
  : EntityParticle(i_position, i_velocity, i_killTime) {
  int cc   = (int) randomNum(0, 50);
  setColor(TColor(cc, cc, cc, 255));
  setSize(randomNum(0, 0.2));
  m_angVel = randomNum(-60, 60);
  setSpriteName(i_spriteName);
}

SmokeParticle::~SmokeParticle() {
}


void ParticlesSourceSmoke::addParticle(Vector2f i_velocity, float i_killTime, std::string i_spriteName) {
  m_particles.push_back(new SmokeParticle(DynamicPosition(), i_velocity, i_killTime, i_spriteName));
}

void ParticlesSourceFire::addParticle(Vector2f i_velocity, float i_killTime, std::string i_spriteName) {
  m_particles.push_back(new FireParticle(DynamicPosition(), i_velocity, i_killTime, i_spriteName));
}

void ParticlesSourceDebris::addParticle(Vector2f i_velocity, float i_killTime, std::string i_spriteName) {
  m_particles.push_back(new DebrisParticle(DynamicPosition(), i_velocity, i_killTime, i_spriteName));
}
