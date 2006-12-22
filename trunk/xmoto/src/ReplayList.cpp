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

#define CURRENT_REPLAY_INDEX_FILE_VERSION 2

namespace vapp {

  void ReplayList::initFromCache() {
    int v_nbReplays;
    std::string v_replayName;

    clear();

    vapp::FileHandle *pfh = vapp::FS::openIFile(ReplayIndexFileName());
    if(pfh == NULL) {
      throw Exception((std::string("Unable to open file ") + ReplayIndexFileName()).c_str());
    }

    try {
      int v_version = vapp::FS::readInt_LE(pfh); /* version */
      if(v_version != CURRENT_REPLAY_INDEX_FILE_VERSION) {
	throw Exception("Invalid replay index file version");
      }
      v_nbReplays = vapp::FS::readInt_LE(pfh);
      for(int i=0; i<v_nbReplays; i++) {
	try {
	  ReplayInfo *pRpl  = new ReplayInfo;
	  pRpl->Level       = vapp::FS::readString(pfh);
	  pRpl->Name        = vapp::FS::readString(pfh);
	  pRpl->Player      = vapp::FS::readString(pfh);
	  pRpl->nTimeStamp  = vapp::FS::readInt_LE(pfh);
	  pRpl->fFrameRate  = vapp::FS::readFloat_LE(pfh);
	  pRpl->fFinishTime = vapp::FS::readFloat_LE(pfh);
	  m_Replays.push_back(pRpl);
	} catch(Exception &e) {
	}
      }
    } catch(Exception &e) {
      clear();
      vapp::FS::closeFile(pfh);
      throw e;
    }
    vapp::FS::closeFile(pfh);
  }

  void ReplayList::saveCache() {
    /* for windows : your must remove the file before create it */
    remove(ReplayIndexFileName().c_str());
    
    FileHandle *pfh = FS::openOFile(ReplayIndexFileName());
    if(pfh == NULL) {
      throw Exception((std::string("Unable to open file ") + ReplayIndexFileName()).c_str());
      return;
    }
    
    try {
      vapp::FS::writeInt_LE(pfh, CURRENT_REPLAY_INDEX_FILE_VERSION); /* version */
      vapp::FS::writeInt_LE(pfh, m_Replays.size());
      for(int i=0; i<m_Replays.size(); i++) {
	vapp::FS::writeString(pfh,   m_Replays[i]->Level);
	vapp::FS::writeString(pfh,   m_Replays[i]->Name);
	vapp::FS::writeString(pfh,   m_Replays[i]->Player);
	vapp::FS::writeInt_LE(pfh,   m_Replays[i]->nTimeStamp);
	vapp::FS::writeFloat_LE(pfh, m_Replays[i]->fFrameRate);
	vapp::FS::writeFloat_LE(pfh, m_Replays[i]->fFinishTime);
      }
    } catch(Exception &e) {
      vapp::FS::closeFile(pfh);
      throw e;
    }
    vapp::FS::closeFile(pfh);
  }

  std::string ReplayList::ReplayIndexFileName() {
    return vapp::FS::getUserDir() + "/" + "Replays/replays.index";
  }

  void ReplayList::initFromDir() {
    this->clear();

    std::vector<std::string> ReplayFiles;
    ReplayFiles = FS::findPhysFiles("Replays/*.rpl");

    for(unsigned int i=0; i<ReplayFiles.size(); i++) {
      addReplay(FS::getFileBaseName(ReplayFiles[i]), false);
    }
    saveCache();
  }

  void ReplayList::addReplay(const std::string &Replay, bool i_saveCache) {
    ReplayInfo* rplInfos;

    if(FS::getFileBaseName(Replay) == "Latest") {
      return;
    }

    /* Already got this replay? */
    for(unsigned int i=0;i<m_Replays.size();i++) {
      if(m_Replays[i]->Name == Replay) {
	/* Yeah */
	return;
      }
    }
    
    rplInfos = Replay::getReplayInfos(Replay);
    if(rplInfos != NULL) {
      m_Replays.push_back(rplInfos);
    }

    if(i_saveCache) {
      saveCache();
    }
  }

  void ReplayList::delReplay(const std::string &Replay, bool i_saveCache) {
    for(unsigned int i=0;i<m_Replays.size();i++) {
      if(m_Replays[i]->Name == Replay) {
	delete m_Replays[i];
	m_Replays.erase( m_Replays.begin() + i );
	break;
      }
    }

    if(i_saveCache) {
      saveCache();
    }
  }

  /*===========================================================================
  Clean up stuff
  ===========================================================================*/
  void ReplayList::clear(bool i_saveCache) {
    for(unsigned int i=0;i<m_Replays.size();i++) {
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
    for(unsigned int i=0;i<m_Replays.size();i++) {
      if((PlayerName == "" || PlayerName==m_Replays[i]->Player) &&
         (LevelID == "" || LevelID==m_Replays[i]->Level)) {
        /* Got one */
        Ret->push_back(m_Replays[i]);
      }
    }
    
    /* Return list */
    return Ret;
  }

}

