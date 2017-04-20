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

#ifndef __ZONE_H__
#define __ZONE_H__

#include "helpers/VMath.h"
#include "common/VXml.h"
#include "BasicSceneStructs.h"
#include <vector>

class FileHandle;

/*===========================================================================
  Zone primitive
  ===========================================================================*/
class ZonePrim {
 public:
  ZonePrim();  
  virtual ~ZonePrim();

 virtual bool doesCircleTouch(const Vector2f& i_cp, float i_cr) = 0;
 virtual void saveBinary(FileHandle *i_pfh)  = 0;
 virtual ZonePrimType Type() const = 0;
 static ZonePrim* readFromBinary(FileHandle *i_pfh);

};

class ZonePrimBox : public ZonePrim {
 public:
  ZonePrimBox(float i_left, float i_right, float i_top, float i_bottom);  
  ~ZonePrimBox();

  virtual bool doesCircleTouch(const Vector2f& i_cp, float i_cr);
  virtual void saveBinary(FileHandle *i_pfh);
  virtual ZonePrimType Type() const;
  static ZonePrim* readFromXml(xmlNodePtr pElem);
  static ZonePrim* readFromBinary(FileHandle *i_pfh);

  float Left()   const;
  float Right()  const;
  float Top()    const;
  float Bottom() const;

 private:
  float m_left, m_right, m_top, m_bottom;
};

/*===========================================================================
  Zone
  ===========================================================================*/
class Zone {
 public:
  Zone(const std::string& i_id);
  ~Zone();
  std::string Id() const;
  std::vector<ZonePrim *> &Prims();

  bool doesCircleTouch(const Vector2f& i_cp, float i_cr);
  void saveBinary(FileHandle *i_pfh);
  static Zone* readFromXml(xmlNodePtr pElem);
  static Zone* readFromBinary(FileHandle *i_pfh);
  AABB& getAABB() {return m_BBox;}

 private:
  std::string m_id; /* Zone ID */
  std::vector<ZonePrim *> m_prims; /* Primitives forming zone */
  AABB m_BBox;
};

#endif /* __ZONE_H__ */
