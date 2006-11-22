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

Entity::Entity(const std::string& i_id) {
  m_id     = i_id;
  m_size   = 1.0;
  m_width  = -1.0;
  m_height = -1.0;
  m_z      = 1.0;
}

Entity::~Entity() {
}

void Entity::loadToPlay() {
  m_dynamicPosition = m_initialPosition;
}

void Entity::unloadToPlay() {
}

std::string Entity::Id() const {
  return m_id;
}

float Entity::Size() const {
  return m_size;
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

void Entity::setType(EntityType i_type) {
  m_type = i_type;
}

void Entity::setInitialPosition(const Vector2f& i_initialPosition) {
  m_initialPosition = i_initialPosition;
}

std::string Entity::SpriteName() const {
  return m_spriteName;
}

void Entity::setSpriteName(const std::string& i_spriteName) {
  m_spriteName = i_spriteName;
}

Vector2f Entity::DynamicPosition() const {
  return m_dynamicPosition;  
}

void Entity::setSize(float i_size) {
  m_size = i_size;
}

void Entity::setWidth(float i_width) {
  m_width = i_width;
}

void Entity::setHeight(float i_height) {
  m_height = i_height;
}

void Entity::setZ(float i_z) {
  m_z = i_z;
}

EntityType Entity::Type() const {
  return m_type;
}

bool Entity::updateToTime(vapp::MotoGame& i_scene) {
  return false;
}

ParticlesSource::ParticlesSource(const std::string& i_id) : Entity(i_id) {
  m_type  = ET_PARTICLESOURCE;
  m_nextParticleTime = 0.0;
}

ParticlesSource::~ParticlesSource() {
  deleteParticles();
}

float EntityParticle::Size() const {
  return m_size;
}

TColor EntityParticle::Color() const {
  return m_color;
}

void ParticlesSource::loadToPlay() {
  Entity::loadToPlay();
  m_nextParticleTime = 0.0;
  m_particleTime_increment = 0.025;
} 

void ParticlesSource::unloadToPlay() {
  Entity::unloadToPlay();

  deleteParticles();
}

bool ParticlesSource::updateToTime(vapp::MotoGame& i_scene) {
  unsigned int i;

  if(i_scene.getTime() > m_nextParticleTime) {  
    i = 0;
    while(i < m_particles.size()) {
      if(i_scene.getTime() > m_particles[i]->KillTime()) {
	delete m_particles[i];
	m_particles.erase(m_particles.begin() + i);
      } else {
	m_particles[i]->updateToTime(i_scene);
	i++;
      }
    }
    m_nextParticleTime = i_scene.getTime() + m_particleTime_increment;
    return true;
  }
  return false;
}

ParticlesSourceSmoke::ParticlesSourceSmoke(const std::string& i_id)
  : ParticlesSource(i_id) {
  m_particleTime_increment = 0.050;
}

ParticlesSourceSmoke::~ParticlesSourceSmoke() {
}

ParticlesSourceFire::ParticlesSourceFire(const std::string& i_id)
  : ParticlesSource(i_id) {
  m_spriteName = "Fire";
}

ParticlesSourceFire::~ParticlesSourceFire() {
}

ParticlesSourceStar::ParticlesSourceStar(const std::string& i_id)
  : ParticlesSource(i_id) {
  m_spriteName = "Star";
}

ParticlesSourceStar::~ParticlesSourceStar() {
}

ParticlesSourceDebris::ParticlesSourceDebris(const std::string& i_id)
  : ParticlesSource(i_id) {
  m_spriteName = "Debris1";
}

ParticlesSourceDebris::~ParticlesSourceDebris() {
}

bool ParticlesSourceSmoke::updateToTime(vapp::MotoGame& i_scene) {
  if(ParticlesSource::updateToTime(i_scene)) {
    /* Generate smoke */
    if(randomNum(0,5) < 1) {
      if(randomNum(0,1) < 0.5) {
	addParticle(Vector2f(randomNum(-0.6,0.6), randomNum(0.2,0.6)), i_scene.getTime() + 10.0, "Smoke1");
      } else {
	addParticle(Vector2f(randomNum(-0.6,0.6), randomNum(0.2,0.6)), i_scene.getTime() + 10.0, "Smoke2");
      }
    }
    return true;
  }
  return false;
}

void SmokeParticle::updateToTime(vapp::MotoGame& i_scene) {
  EntityParticle::updateToTime(i_scene);

  float v_timeStep = 0.025;

  m_size += v_timeStep * 1.0f; /* grow */
  m_acceleration = Vector2f(0.2, 0.5);  /* accelerate upwards */

  int v_c = m_color.Red() + (int)(randomNum(40,50) * v_timeStep);
  m_color.setRed(v_c > 255 ? 255 : v_c);
  m_color.setBlue(v_c > 255 ? 255 : v_c);
  m_color.setGreen(v_c > 255 ? 255 : v_c);

  int v_a = m_color.Alpha() - (int)(120.0f * v_timeStep);
  if(v_a >= 0) {
    m_color.setAlpha(v_a);
  } else {
    m_killTime = i_scene.getTime();
  }
}

void FireParticle::updateToTime(vapp::MotoGame& i_scene) {
  EntityParticle::updateToTime(i_scene);

  float v_timeStep = 0.035;

  int v_g = m_color.Green() - (int)(randomNum(190,210) * v_timeStep);
  m_color.setGreen(v_g < 0 ? 0 : v_g);

  int v_b = m_color.Blue()  - (int)(randomNum(400,400) * v_timeStep);
  m_color.setBlue(v_b < 0 ? 0 : v_b);

  int v_a = m_color.Alpha() - (int)(200.0f * v_timeStep);
  if(v_a >= 0) {
    m_color.setAlpha(v_a);
  } else {
    m_killTime = i_scene.getTime();
  }
      
  m_velocity.x = sin((i_scene.getTime() - 
		      m_spawnTime + m_fireSeed) * randomNum(5,15)) * 0.8f
    +
    sin((i_scene.getTime() - m_fireSeed) * 10) * 0.3;
  m_acceleration.y = 2.0;
}

bool ParticlesSourceFire::updateToTime(vapp::MotoGame& i_scene) {
  if(ParticlesSource::updateToTime(i_scene)) {
    /* Generate fire */
    for(int k=0;k<12;k++) {
      /* maximum 10s for a fire particule, but it can be destroyed before */
      ParticlesSource::addParticle(Vector2f(randomNum(-1,1),randomNum(0.1,0.3)), i_scene.getTime() + 10.0);
    }
    return true;
  }
  return false;
}

bool ParticlesSourceDebris::updateToTime(vapp::MotoGame& i_scene) {
  return ParticlesSource::updateToTime(i_scene); 
}

void EntityParticle::updateToTime(vapp::MotoGame& i_scene) {
  float v_timeStep = 0.025;

  m_velocity += m_acceleration * v_timeStep;
  m_position += m_velocity     * v_timeStep;
  m_angVel   += m_angAcc       * v_timeStep;
  m_ang      += m_angVel       * v_timeStep;
}

float EntityParticle::KillTime() const {
  return m_killTime;
}

void Entity::setDynamicPosition(const Vector2f& i_dynamicPosition) {
  m_dynamicPosition = i_dynamicPosition;
}

EntityParticle::EntityParticle(const Vector2f& i_position, const Vector2f i_velocity, float i_killTime, std::string i_spriteName) {
  m_front    	 = true;
  m_position 	 = i_position;
  m_velocity 	 = i_velocity;
  m_killTime 	 = i_killTime;
  m_acceleration = Vector2f(0,0);
  m_ang          = 0;
  m_angAcc       = 0;
  m_angVel       = 0;
  m_color        = TColor(255, 255, 255, 255);
  m_size         = 0.5;
  m_spriteName   = i_spriteName;
}

EntityParticle::~EntityParticle() {
}

void Entity::saveXml(vapp::FileHandle *i_pfh) {
  vapp::FS::writeLineF(i_pfh,"\t<entity id=\"%s\" typeid=\"%s\">", Id().c_str(), Entity::TypeToStr(Type()).c_str());
  vapp::FS::writeLineF(i_pfh,"\t\t<size r=\"%f\"/>",Size());
  vapp::FS::writeLineF(i_pfh,"\t\t<position x=\"%f\" y=\"%f\"/>", InitialPosition().x, InitialPosition().y);      

  vapp::FS::writeLineF(i_pfh,"\t\t<param name=\"%s\" value=\"%.2f\"/>",
                       "z", Z());

  if(Type() == ET_SPRITE) {
    vapp::FS::writeLineF(i_pfh,"\t\t<param name=\"%s\" value=\"%.2f\"/>",
			 "name", SpriteName().c_str());
  }

  if(Type() == ET_PARTICLESOURCE) {
    vapp::FS::writeLineF(i_pfh,"\t\t<param name=\"%s\" value=\"%.2f\"/>",
			 "type", SpriteName().c_str());
  }

  vapp::FS::writeLineF(i_pfh,"\t</entity>");
}

EntityType Entity::TypeFromStr(std::string i_typeStr) {
  if(i_typeStr == "PlayerStart")    return ET_PLAYERSTART;
  if(i_typeStr == "EndOfLevel")     return ET_ENDOFLEVEL;
  if(i_typeStr == "Wrecker")        return ET_WRECKER;
  if(i_typeStr == "Strawberry")     return ET_STRAWBERRY;
  if(i_typeStr == "ParticleSource") return ET_PARTICLESOURCE;

  return ET_SPRITE;
}

std::string Entity::TypeToStr(EntityType i_type) {
  switch(i_type) {
    case ET_PLAYERSTART :
      return "PlayerStart";
      break;
    case ET_ENDOFLEVEL :
      return "EndOfLevel";
      break;
    case ET_WRECKER :
      return "Wrecker";
      break;
    case ET_STRAWBERRY :
      return "Strawberry";
      break;
    case ET_PARTICLESOURCE :
      return "ParticleSource";
      break;
  default:
      return "Sprite";
  }
}

Entity* Entity::readFromXml(TiXmlElement *pElem) {
  std::string v_id;
  std::string v_typeId;
  EntityType  v_type;
  Vector2f    v_position;
  float       v_size   = 0.2;
  float       v_height = -1.0;
  float       v_width  = -1.0;
  float       v_z      = 0.5;
  std::string v_spriteName;
  std::string v_typeName;

  /* read xml information */
  v_id         = vapp::XML::getOption(pElem,"id");
  v_typeId     = vapp::XML::getOption(pElem,"typeid");
  v_type       = Entity::TypeFromStr(v_typeId);
  TiXmlElement *pPosElem = pElem->FirstChildElement("position");
  if(pPosElem != NULL) {
    v_position.x = atof(vapp::XML::getOption(pPosElem,"x","0").c_str());
    v_position.y = atof(vapp::XML::getOption(pPosElem,"y","0").c_str());
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

  if(v_type == ET_PARTICLESOURCE) {
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

  switch(v_type) {
  case ET_SPRITE:
    v_entity->setSpriteName(v_spriteName);
    break;
  case ET_PARTICLESOURCE:
      v_entity->setSpriteName(v_typeName);
    break;
  default:
    v_entity->setSpriteName(v_typeId);
  }
  v_entity->setType(v_type);
  v_entity->setInitialPosition(v_position);
  v_entity->setSize(v_size);
  if(v_width > 0.0) {
    v_entity->setWidth(v_width);
  }
  if(v_height > 0.0) {
    v_entity->setHeight(v_height);
  }  
  v_entity->setZ(v_z);

  return v_entity;
}

void Entity::saveBinary(vapp::FileHandle *i_pfh) {
  vapp::FS::writeString(i_pfh,   Id());
  vapp::FS::writeString(i_pfh,   Entity::TypeToStr(Type()));
  vapp::FS::writeFloat_LE(i_pfh, Size());
  vapp::FS::writeFloat_LE(i_pfh, Width());       
  vapp::FS::writeFloat_LE(i_pfh, Height()); 
  vapp::FS::writeFloat_LE(i_pfh, InitialPosition().x);
  vapp::FS::writeFloat_LE(i_pfh, InitialPosition().y);
  
  switch(Type()) {
  case ET_SPRITE:
  case ET_PARTICLESOURCE:
    vapp::FS::writeByte(i_pfh, 0x02);
    break;
  default:
    vapp::FS::writeByte(i_pfh, 0x01);
  }

  
  std::ostringstream v_z;
  v_z << Z();
  vapp::FS::writeString(i_pfh, "z");
  vapp::FS::writeString(i_pfh, v_z.str());

  if(Type() == ET_SPRITE) {
    vapp::FS::writeString(i_pfh, "name");
    vapp::FS::writeString(i_pfh, SpriteName());
  }

  if(Type() == ET_PARTICLESOURCE) {
    vapp::FS::writeString(i_pfh, "type");
    vapp::FS::writeString(i_pfh, SpriteName());
  }
}

Entity* Entity::readFromBinary(vapp::FileHandle *i_pfh) {
  std::string v_id;
  std::string v_typeId;
  EntityType  v_type;
  Vector2f    v_position;
  float       v_size   = 0.2;
  float       v_height = -1.0;
  float       v_width  = -1.0;
  float       v_z      = 0.5;
  std::string v_spriteName;
  std::string v_typeName;

  /* read values */
  v_id         = vapp::FS::readString(i_pfh);
  v_typeId     = vapp::FS::readString(i_pfh);
  v_type       = Entity::TypeFromStr(v_typeId);
  v_size       = vapp::FS::readFloat_LE(i_pfh);
  v_width      = vapp::FS::readFloat_LE(i_pfh);
  v_height     = vapp::FS::readFloat_LE(i_pfh);
  v_position.x = vapp::FS::readFloat_LE(i_pfh);
  v_position.y = vapp::FS::readFloat_LE(i_pfh);
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

  if(v_type == ET_PARTICLESOURCE) {
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

  switch(v_type) {
  case ET_SPRITE:
    v_entity->setSpriteName(v_spriteName);
    break;
  case ET_PARTICLESOURCE:
      v_entity->setSpriteName(v_typeName);
    break;
  default:
    v_entity->setSpriteName(v_typeId);
  }

  v_entity->setType(v_type);
  v_entity->setInitialPosition(v_position);
  v_entity->setSize(v_size);
  if(v_width > 0.0) {
    v_entity->setWidth(v_width);
  }
  if(v_height > 0.0) {
    v_entity->setHeight(v_height);
  }  
  v_entity->setZ(v_z);

  return v_entity;
}

void ParticlesSource::clearAfterRewind() {
  Entity::clearAfterRewind();
  deleteParticles();
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

Vector2f EntityParticle::Position() const {
  return m_position;
}

float EntityParticle::Angle() const {
  return m_ang;
}

std::string EntityParticle::SpriteName() const {
  return m_spriteName;
}

void Entity::clearAfterRewind() {
}

void ParticlesSource::addParticle(Vector2f i_velocity, float i_killTime) {
  addParticle(i_velocity, i_killTime, m_spriteName);
}

void ParticlesSourceStar::addParticle(Vector2f i_velocity, float i_killTime, std::string i_spriteName) {
  m_particles.push_back(new StarParticle(m_dynamicPosition, i_killTime, i_spriteName));
}

StarParticle::StarParticle(const Vector2f& i_position, float i_killTime, std::string i_spriteName)
  : EntityParticle(i_position, Vector2f(randomNum(-2,2),randomNum(0,2)), i_killTime, i_spriteName) {
  m_angVel       = randomNum(-60,60);
  m_acceleration = Vector2f(0,-4);
}

StarParticle::~StarParticle() {
}

DebrisParticle::DebrisParticle(const Vector2f& i_position, const Vector2f i_velocity, float i_killTime, std::string i_spriteName)
  : EntityParticle(i_position, Vector2f(randomNum(-2,2),randomNum(0,2)), i_killTime, i_spriteName) {
  m_angVel       = randomNum(-60,60);
  m_acceleration = Vector2f(0,-4);
  int cc     	 = (int) randomNum(0, 250);
  m_color    	 = TColor(cc, cc, cc, 255);
  m_velocity 	*= randomNum(1.5, 0.5);
  m_velocity 	+= Vector2f(randomNum(-0.2,0.2), randomNum(-0.2,0.2));
  m_size     	 = randomNum(0.02f,0.04f);
}

DebrisParticle::~DebrisParticle() {
}

void DebrisParticle::updateToTime(vapp::MotoGame& i_scene) {
  EntityParticle::updateToTime(i_scene);

  float v_timeStep = 0.025;

  m_acceleration = i_scene.getGravity() * (-5.5f / PHYS_WORLD_GRAV);

  int v_a = m_color.Alpha() - (int)(120.0f * v_timeStep);
  if(v_a >= 0) {
    m_color.setAlpha(v_a);
  } else {
    m_killTime = i_scene.getTime();
  }
}

FireParticle::FireParticle(const Vector2f& i_position, const Vector2f i_velocity, float i_killTime, std::string i_spriteName)
  : EntityParticle(i_position, i_velocity, i_killTime, i_spriteName) {
  m_fireSeed = randomNum(0,100);
  m_size = 0.09;
  m_color = TColor(255,255,0,255);
}

FireParticle::~FireParticle() {
}

SmokeParticle::SmokeParticle(const Vector2f& i_position, const Vector2f i_velocity, float i_killTime, std::string i_spriteName) 
  : EntityParticle(i_position, i_velocity, i_killTime, i_spriteName) {
  int cc   = (int) randomNum(0, 50);
  m_color  = TColor(cc, cc, cc, 255);
  m_size   = randomNum(0, 0.2);
  m_angVel = randomNum(-60, 60);
}

SmokeParticle::~SmokeParticle() {
}


void ParticlesSourceSmoke::addParticle(Vector2f i_velocity, float i_killTime, std::string i_spriteName) {
  m_particles.push_back(new SmokeParticle(m_dynamicPosition, i_velocity, i_killTime, i_spriteName));
}

void ParticlesSourceFire::addParticle(Vector2f i_velocity, float i_killTime, std::string i_spriteName) {
  m_particles.push_back(new FireParticle(m_dynamicPosition, i_velocity, i_killTime, i_spriteName));
}

void ParticlesSourceDebris::addParticle(Vector2f i_velocity, float i_killTime, std::string i_spriteName) {
  m_particles.push_back(new DebrisParticle(m_dynamicPosition, i_velocity, i_killTime, i_spriteName));
}
