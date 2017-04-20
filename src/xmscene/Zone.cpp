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
#include "common/VFileIO.h"
#include "common/VXml.h"
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

Zone* Zone::readFromXml(xmlNodePtr pElem) {
  Zone *v_zone = new Zone(XMLDocument::getOption(pElem, "id"));
  
  /* Get primitives */
  for(xmlNodePtr pSubElem = XMLDocument::subElement(pElem, "box");
      pSubElem != NULL;
      pSubElem = XMLDocument::nextElement(pSubElem)) {
	v_zone->m_prims.push_back(ZonePrimBox::readFromXml(pSubElem));
  }
  
  return v_zone;
}

void Zone::saveBinary(FileHandle *i_pfh) {
  XMFS::writeString(i_pfh,Id());
  XMFS::writeByte(i_pfh, Prims().size());
        
  for(unsigned int j=0;j<Prims().size();j++) {
    XMFS::writeInt_LE(i_pfh,(int) (Prims()[j]->Type()));
    Prims()[j]->saveBinary(i_pfh);
  }
}
        
void ZonePrimBox::saveBinary(FileHandle *i_pfh) {
  XMFS::writeFloat_LE(i_pfh,m_left);
  XMFS::writeFloat_LE(i_pfh,m_right);
  XMFS::writeFloat_LE(i_pfh,m_top);
  XMFS::writeFloat_LE(i_pfh,m_bottom);
}

Zone* Zone::readFromBinary(FileHandle *i_pfh) {
  Zone *v_zone = new Zone(XMFS::readString(i_pfh));
  ZonePrimType v_zonePrimType;
  
  int nNumPrims = XMFS::readByte(i_pfh);
  v_zone->m_prims.reserve(nNumPrims);
  for(int j=0;j<nNumPrims;j++) {
    v_zonePrimType = (ZonePrimType) XMFS::readInt_LE(i_pfh);

    switch(v_zonePrimType) {
    case LZPT_BOX:
      v_zone->m_prims.push_back(ZonePrimBox::readFromBinary(i_pfh));
      break;
    }
  }

  return v_zone;
}

ZonePrim* ZonePrimBox::readFromXml(xmlNodePtr pElem) {
  float v_bottom, v_top, v_left, v_right;

  v_bottom = atof( XMLDocument::getOption(pElem,"bottom","0").c_str() );
  v_top    = atof( XMLDocument::getOption(pElem,"top","0").c_str() );
  v_left   = atof( XMLDocument::getOption(pElem,"left","0").c_str() );
  v_right  = atof( XMLDocument::getOption(pElem,"right","0").c_str() );
  
  return new ZonePrimBox(v_left, v_right, v_top, v_bottom);
}

ZonePrim* ZonePrimBox::readFromBinary(FileHandle *i_pfh) {
  float v_bottom, v_top, v_left, v_right;

  v_left   = XMFS::readFloat_LE(i_pfh);
  v_right  = XMFS::readFloat_LE(i_pfh);
  v_top    = XMFS::readFloat_LE(i_pfh);
  v_bottom = XMFS::readFloat_LE(i_pfh);

  return new ZonePrimBox(v_left, v_right, v_top, v_bottom);
}

ZonePrimType ZonePrimBox::Type() const {
  return LZPT_BOX;
}
