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

#ifndef __GHOSTTRAIL_H__
#define __GHOSTTRAIL_H__

#include "BikeGhost.h"

class GhostTrail {
  public:
  GhostTrail(FileGhost* i_ghost); 
  ~GhostTrail();
  std::vector<Vector2f>* getGhostTrailData();
  std::vector<Vector2f>* getSimplifiedGhostTrailData();
  std::vector<Vector2f>* getInterpolatedGhostTrailData();
  bool getGhostTrailAvailable();
  
  private: 
  void initialize(); // lazy mode : initialize only the first time it's used
  
  FileGhost* m_ghost;
  bool m_initialized;
  std::vector<Vector2f> m_trailData;
  std::vector<Vector2f> m_simplifiedTrailData;
  std::vector<Vector2f> m_interpolatedTrailData;
  bool m_trailAvailable;
};
#endif
