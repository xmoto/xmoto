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

#include "Trainer.h"


Trainer::Trainer()
{
  m_forLevelId = "";
  m_storePosReadIndex = -1;
  m_trainerHasBeenUsed=false;
}


Trainer::~Trainer()
{
}


bool Trainer::trainerHasBeenUsed(){
    return m_trainerHasBeenUsed;
}


bool Trainer::isRestorePositionAvailable( std::string levelId )
{
  if( levelId != m_forLevelId )
    return false;
  return m_storePosReadIndex >= 0;
}


int Trainer::getCurrentRestoreIndex()
{
  return m_storePosReadIndex;
}


int Trainer::getMaxRestoreIndex()
{
  return (int)m_storePos.size()-1;
}


Vector2f Trainer::getCurrentRestorePosition( std::string levelId )
{
  m_trainerHasBeenUsed=true;
  if( levelId != m_forLevelId )
    changeLevel( levelId );
  if( m_storePosReadIndex < 0 || m_storePosReadIndex >= (int)m_storePos.size() )
    return Vector2f(0,0);
  return m_storePos[m_storePosReadIndex];
}


Vector2f Trainer::getPreviousRestorePosition( std::string levelId )
{
  m_trainerHasBeenUsed=true;
  if( levelId != m_forLevelId )
    changeLevel( levelId );
  if( m_storePosReadIndex >= 1 )
    m_storePosReadIndex--;
  return getCurrentRestorePosition( levelId );
}


Vector2f Trainer::getNextRestorePosition( std::string levelId )
{
  m_trainerHasBeenUsed=true;
  if( levelId != m_forLevelId )
    changeLevel( levelId );
  if( m_storePosReadIndex >= 0 && m_storePosReadIndex < (int)m_storePos.size()-1 )
    m_storePosReadIndex++;
  return getCurrentRestorePosition( levelId );
}


void Trainer::storePosition( std::string levelId, Vector2f pos )
{
  if( levelId != m_forLevelId ){
    changeLevel( levelId );
  }
  m_storePos.push_back( pos );
  m_storePosReadIndex = m_storePos.size()-1;
}

void Trainer::changeLevel( std::string toLevelId )
{
  m_forLevelId = toLevelId;
  m_storePos.clear();
  m_storePosReadIndex = -1;
}

void Trainer::resetTrainerUse() {
  m_trainerHasBeenUsed = false;
}
