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

#include "GhostTrail.h"

#define TRAIL_INTERPOLATED_TRAIL_INTERNODE_LENGTH 0.3
#define TRAIL_INTERPOLATION_STEP 0.1

GhostTrail::GhostTrail() {
  m_renderGhostTrail = false;
}

GhostTrail::~GhostTrail() {
  m_trailData.clear();
  m_simplifiedTrailData.clear();
  m_interpolatedTrailData.clear();
}

void GhostTrail::initGhostTrail(FileGhost* i_ghost) {
//  m_renderGTBeforeTS = false;
  
  if(i_ghost != 0){// && m_trailData.size() == 0){
  try {
    m_renderGhostTrail= false;
    m_trailData.clear();
    m_interpolatedTrailData.clear();
    m_simplifiedTrailData.clear();
  
    for(unsigned int i=0; i < (*i_ghost->getAllGhostBikeStates()).size(); i++) {
      m_trailData.push_back( (*i_ghost->getAllGhostBikeStates())[i]->CenterP );
    }/*
    
   unsigned int v_trailDataSize = m_trailData.size();
   if(v_trailDataSize != 0) {
    int step = int( v_trailDataSize / (v_trailDataSize/2));
    for( unsigned int i=0; i<v_trailDataSize; i++) {
      if( i%step == 0 ) m_simplifiedTrailData.push_back(m_trailData[i]); //every stepth index
    } 
   }
   */
   
   
   // now lets try real linear interpolation
   
   Vector2f v_P_old = m_trailData[0], 
		      v_P_new, 
		      v_vecCum = Vector2f(0,0),
		      v_vecTmp;
   float v_time = TRAIL_INTERPOLATION_STEP;
   
   for(unsigned int i=1; i<m_trailData.size(); i++) {
   
     //calculate the rise of the function (m) from our current two trail points: p1 to p0
     float dy = m_trailData[i].y - m_trailData[i-1].y,
           dx = m_trailData[i].x - m_trailData[i-1].x;
     v_P_old = m_trailData[i-1];
     v_vecTmp = Vector2f(dx,dy);
     
     //check if teleportation occured, lets assume that 7 is a size big enough for beeing usable as marker
     Vector2f v_checkTeleport = Vector2f(  m_trailData[i].x -  v_P_old.x, m_trailData[i].y - v_P_old.y);
     if( v_checkTeleport.length() > 5 ) {
         continue;  
     }
     
     // check if new position vector is very near the next simplifiedtrailData, else pushback vectors in its direction, assume 0.2
     int i=0;
     do {
         // so lets push back new vector2fsm untilk we're near the next point on the simplified trail
         v_P_new.x = (v_vecTmp.x / v_vecTmp.length()) * v_time + v_P_old.x;
         v_P_new.y = (v_vecTmp.y / v_vecTmp.length()) * v_time + v_P_old.y;
         
         m_interpolatedTrailData.push_back(v_P_new);
         v_P_old = v_P_new;
         
         i++;
         v_checkTeleport = Vector2f( m_trailData[i].x -  v_P_old.x, m_trailData[i].y - v_P_old.y);
         if( v_checkTeleport.length() > 5 ) {
           continue;  
         }
    
     } while( i<int(v_vecTmp.length()/v_time) );
   }
   
   // now interpolated path: cumulate Vector.length, every n length pushBack median
   // interpolated in time, not position
   float v_cumulum = 0; 
   for(unsigned int i=1; i<m_interpolatedTrailData.size(); i++) {
     Vector2f v_vec = Vector2f(fabs(m_interpolatedTrailData[i].x - m_interpolatedTrailData[i-1].x), fabs(m_interpolatedTrailData[i].y - m_interpolatedTrailData[i-1].y));
     v_cumulum+=v_vec.length();
     if(v_cumulum > TRAIL_INTERPOLATED_TRAIL_INTERNODE_LENGTH) {
       m_simplifiedTrailData.push_back(m_interpolatedTrailData[i-1]);
       v_cumulum = 0;
     }
   }
   
   
   m_trailAvailable = true; 
  }
  
  catch(Exception &e) {} 
 }
}

void GhostTrail::setRenderGhostTrail(bool i_render) {
  m_renderGhostTrail = i_render;
}

bool GhostTrail::getRenderGhostTrail() {
  return m_renderGhostTrail;
}

void GhostTrail::toggleRenderGhostTrail() {
  if(m_renderGhostTrail)
	m_renderGhostTrail=false;
  else m_renderGhostTrail=true;  
}
