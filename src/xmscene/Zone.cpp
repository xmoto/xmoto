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

#include "Zone.h"
#include "../VFileIO.h"
#include "../VXml.h"
#include <sstream>

ZonePrim::ZonePrim() {
}

ZonePrim::~ZonePrim() {
}

ZonePrimBox::ZonePrimBox(float i_left, float i_right, float i_top, float i_bottom) {
  m_left   = i_left;
  m_right  = i_right;
  m_top    = i_top;
  m_bottom = i_bottom;
}

ZonePrimBox::~ZonePrimBox() {
}

bool ZonePrimBox::doesCircleTouch(const Vector2f& i_cp, float i_cr) {
  /* Check each zone primitive */

  /* Do simple AABB-check */
  Vector2f CMin(i_cp.x - i_cr, i_cp.y - i_cr);
  Vector2f CMax(i_cp.x + i_cr, i_cp.y + i_cr);        
  Vector2f FMin(CMin.x < m_left   ? CMin.x : m_left,
    CMin.y < m_bottom ? CMin.y : m_bottom);
  Vector2f FMax(CMax.x > m_right  ? CMax.x : m_right,
    CMax.y > m_top    ? CMax.y : m_top);
  if(FMax.x - FMin.x < (CMax.x - CMin.x + m_right - m_left) &&
     FMax.y - FMin.y < (CMax.y - CMin.y + m_top   - m_bottom)) {
    return true; /* Touch! */
  }
  
  /* No touching! */
  return false;
}

float ZonePrimBox::Left() const {
  return m_left;
}

float ZonePrimBox::Right() const {
  return m_right;
}

float ZonePrimBox::Top() const {
  return m_top;
}

float ZonePrimBox::Bottom() const {
  return m_bottom;
}

Zone::Zone(const std::string& i_id) {
  m_id = i_id;
  m_isDeathZone = false;
}

Zone::~Zone() {
  for(unsigned int i=0; i<m_prims.size(); i++) {
    delete m_prims[i];
  }
}

std::string Zone::Id() const {
  return m_id;
}

std::vector<ZonePrim *>& Zone::Prims() {
  return m_prims;
}

/*===========================================================================
  Check whether the given circle touches the zone
  ===========================================================================*/
bool Zone::doesCircleTouch(const Vector2f& i_cp, float i_cr) {
  /* Check each zone primitive */
  for(unsigned int i=0; i<Prims().size(); i++) {
    if(Prims()[i]->doesCircleTouch(i_cp, i_cr)) {
      return true;
    }
  }
    
  /* No touching! */
  return false;
}

Zone* Zone::readFromXml(TiXmlElement *pElem) {
  Zone *v_zone = new Zone(XML::getOption(pElem,"id"));
  
  /* Get primitives */
  for(TiXmlElement *pj = pElem->FirstChildElement("box"); pj!=NULL; pj=pj->NextSiblingElement("box")) {
    v_zone->m_prims.push_back(ZonePrimBox::readFromXml(pj));
  }  
  
  return v_zone;
}

void Zone::saveBinary(FileHandle *i_pfh) {
  FS::writeString(i_pfh,Id());
  FS::writeByte(i_pfh, Prims().size());
        
  for(unsigned int j=0;j<Prims().size();j++) {
    FS::writeInt_LE(i_pfh,(int) (Prims()[j]->Type()));
    Prims()[j]->saveBinary(i_pfh);
  }
}

void Zone::setDeathZone(bool i_value) {
  m_isDeathZone = i_value;
}

bool Zone::isDeathZone() {
  return m_isDeathZone;
}
        
void Zone::updateDeathZone(std::string i_scriptSource) {
  /* This function parses the Script Source for death zones: zones, 
     that typically conist of OnEnter() and Game.KillPlayer() and end */
     
  //determine the positions between death zone function start and end
  std::string v_deathZoneFunctionString = m_id + ".OnEnter()";
  size_t v_posStart = i_scriptSource.find(v_deathZoneFunctionString);
  if(v_posStart == std::string::npos) {
    //Zone isnt in the script? hum, strange but
    return;
  } 
  size_t v_posEnd = i_scriptSource.find("end", v_posStart);
  size_t v_found = i_scriptSource.find(".KillPlayer", v_posStart);
  if(v_found > v_posStart && v_found < v_posEnd) {
    //looks like we identified a death zone!
    this->setDeathZone(true);
  }
}
        
void ZonePrimBox::saveBinary(FileHandle *i_pfh) {
  FS::writeFloat_LE(i_pfh,m_left);
  FS::writeFloat_LE(i_pfh,m_right);
  FS::writeFloat_LE(i_pfh,m_top);
  FS::writeFloat_LE(i_pfh,m_bottom);
}

Zone* Zone::readFromBinary(FileHandle *i_pfh) {
  Zone *v_zone = new Zone(FS::readString(i_pfh));
  ZonePrimType v_zonePrimType;
  
  int nNumPrims = FS::readByte(i_pfh);
  v_zone->m_prims.reserve(nNumPrims);
  for(int j=0;j<nNumPrims;j++) {
    v_zonePrimType = (ZonePrimType) FS::readInt_LE(i_pfh);

    switch(v_zonePrimType) {
    case LZPT_BOX:
      v_zone->m_prims.push_back(ZonePrimBox::readFromBinary(i_pfh));
      break;
    }
  }

  return v_zone;
}

ZonePrim* ZonePrimBox::readFromXml(TiXmlElement *i_elem) {
  float v_bottom, v_top, v_left, v_right;

  v_bottom = atof( XML::getOption(i_elem,"bottom","0").c_str() );
  v_top    = atof( XML::getOption(i_elem,"top","0").c_str() );
  v_left   = atof( XML::getOption(i_elem,"left","0").c_str() );
  v_right  = atof( XML::getOption(i_elem,"right","0").c_str() );
  
  return new ZonePrimBox(v_left, v_right, v_top, v_bottom);
}

ZonePrim* ZonePrimBox::readFromBinary(FileHandle *i_pfh) {
  float v_bottom, v_top, v_left, v_right;

  v_left   = FS::readFloat_LE(i_pfh);
  v_right  = FS::readFloat_LE(i_pfh);
  v_top    = FS::readFloat_LE(i_pfh);
  v_bottom = FS::readFloat_LE(i_pfh);

  return new ZonePrimBox(v_left, v_right, v_top, v_bottom);
}

ZonePrimType ZonePrimBox::Type() const {
  return LZPT_BOX;
}
