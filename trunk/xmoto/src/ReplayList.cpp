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

/* 
 *  Replay list class
 */
#include "Replay.h"

namespace vapp {

  void ReplayList::initFromDir() {
    this->clear();

    std::vector<std::string> ReplayFiles;
    ReplayFiles = FS::findPhysFiles("Replays/*.rpl");

    for(int i=0; i<ReplayFiles.size(); i++) {
      addReplay(FS::getFileBaseName(ReplayFiles[i]));
    }
  }

  void ReplayList::addReplay(const std::string &Replay) {
    ReplayInfo* rplInfos;

    if(FS::getFileBaseName(Replay) != "Latest") {    
      rplInfos = Replay::getReplayInfos(Replay);
      if(rplInfos != NULL) {
	m_Replays.push_back(rplInfos);
      }
    }
  }

  void ReplayList::delReplay(const std::string &Replay) {
    for(int i=0;i<m_Replays.size();i++) {
      if(m_Replays[i]->Name == Replay) {
	delete m_Replays[i];
	m_Replays.erase( m_Replays.begin() + i );
	break;
      }
    }
  }

  /*===========================================================================
  Clean up stuff
  ===========================================================================*/
  void ReplayList::clear(void) {
    for(int i=0;i<m_Replays.size();i++) {
      delete m_Replays[i];
    }
    m_Replays.clear();     
  }
  
  /*===========================================================================
  Look up replays in list
  ===========================================================================*/
  std::vector<ReplayInfo *>* ReplayList::findReplays(const std::string &PlayerName,const std::string &LevelID) {
    std::vector<ReplayInfo *> *Ret;
    Ret = new std::vector<ReplayInfo *>;

    /* find replays */
    for(int i=0;i<m_Replays.size();i++) {
      if((PlayerName=="" || PlayerName==m_Replays[i]->Player) &&
         (LevelID=="" || LevelID==m_Replays[i]->Level)) {
        /* Got one */
        Ret->push_back(m_Replays[i]);
      }
    }
    
    /* Return list */
    return Ret;
  }

};

