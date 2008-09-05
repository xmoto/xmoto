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

#include "ReplayManager.h"

ReplayManager::ReplayManager()
{
}

ReplayManager::~ReplayManager()
{
}

void ReplayManager::createNewReplay()
{
}

void ReplayManager::addRecordingScene(Scene* pScene)
{
  m_scenes[m_scenes.size()] = pScene;
}

void ReplayManager::addObjectTypeToRecord(Scene* pScene, std::string name)
{
  ISerializer* pSer  = (ISerializer*)SerializerFactory::instance()->createObject(name);
  pSer->setPlaying(false);
  m_scenesSerializersRecording[pScene] = pSer;
}

/*
void ReplayManager::addObjectTypeToRecord(Scene* pScene, ISerializer* pSer)
{
  pSer->setPlaying(false);
  m_scenesSerializersRecording[pScene] = pSer;
}
*/

void ReplayManager::startRecordingReplay()
{
  m_recordReplay = true;
}

void ReplayManager::recordFrame()
{
  std::map<Scene*, std::vector<ISerializer*> >::iterator itScene;
  itScene = m_scenesSerializersRecording.begin();

  while(itScene != m_scenesSerializersRecording.end()) {
    Scene* pScene                          = (*itScene).first;
    std::vector<ISerializer*>& serializers = (*itScene).second;

    std::vector<ISerializer*>::iterator itSerializer;
    itSerializer = serializers.begin();

    while(itSerializer != serializers.end()) {
      (*itSerializer)->storeFrame(pScene);

      ++itSerializer;
    }

    ++itScene;
  }
}

void ReplayManager::endRecordingReplay()
{
}

Replay* ReplayManager::getCurrentRecordingReplay()
{
}

void ReplayManager::addPlayingReplay(Scene* pScene, std::string& fileName, bool mainReplay)
{
  // version 0: only player chunks
  // version 1: add events
  // version 2: generic handling

  // for non main replays, only load the biker informations

  Replay* pReplay = new Replay();
  // read the header of the replay, keeps the file handle open
  FileHandle* pfh = pReplay->openReplay(fileName);

  switch(pReplay->getVersion()) {
  case 0: {
    // only biker
    ISerializer* pBike = SerializerFactory::instance()->createObject("BikerSerializer");
    pBike->loadBuffer(pfh);
    pBike->setPlaying(true);
    pBike->unserializeFrames(pScene);
    m_scenesSerializersPlaying[pScene].push_back(pBike);
  }
    break;
  case 1: {
    // biker and events
    ISerializer* pBike  = SerializerFactory::instance()->createObject("BikerSerializer");
    pBike->loadBuffer(pfh);
    pBike->setPlaying(true);
    pBike->unserializeFrames(pScene);
    m_scenesSerializersPlaying[pScene].push_back(pBike);

    if(mainReplay == true) {
      ISerializer* pEvent = SerializerFactory::instance()->createObject("MotoGameEventManager");
      pEvent->loadBuffer(pfh);
      pEvent->setPlaying(true);
      pEvent->unserializeFrames(pScene);
      m_scenesSerializersPlaying[pScene].push_back(pEvent);
    }
  }
    break;
  case 2: {
    // everything
    int nbSavedStuffs  = FS::readByte(pfh); 
    for(int i=0; i<nbSavedStuffs; i++) {
      std::string stuff = FS::readString(pfh);
      if(mainReplay == false
	 && stuff != "BikerSerializer") {
	  skipBuffer(pfh);
      } else {
	ISerializer* pSer;
	pSer = SerializerFactory::instance()->createObject(stuff);
	if(pSer != NULL) {
	  pSer->loadBuffer(pfh);
	  pSer->setPlaying(true);
	  pSer->unserializeFrames(pScene);
	  m_scenesSerializersPlaying[pScene].push_back(pSer);
	} else {
	  LogWarning("Can't handle data type %s in replay %s",
		     stuff.c_str(), fileName.c_str());
	  skipBuffer(pfh);
	}
      }
    }

  }
    break;
  default:
    LogError("Unsuported replay version %d", pReplay->getVersion());
  }

  FS::closeFile(pfh);
}

void ReplayManager::playFrame()
{
  std::map<Scene*, std::vector<ISerializer*> >::iterator itScene;
  itScene= m_scenesSerializersPlaying.begin();

  while(itScene != m_scenesSerializersPlaying.end()) {
    Scene* pScene                          = (*itScene).first;
    std::vector<ISerializer*>& serializers = (*itScene).second;

    std::vector<ISerializer*>::iterator itSerializer;
    itSerializer = serializers.begin();

    while(itSerializer != serializers.end()) {
      (*itSerializer)->playFrame(pScene);

      ++itSerializer;
    }

    ++itScene;
  }
}

void ReplayManager::fastforward(int i_time)
{
}

void ReplayManager::fastrewind(int  i_time, int i_minimumNbFrame = 0)
{
}

void ReplayManager::reinitializePlayingReplays()
{
}

void ReplayManager::deleteReplay(std::string& ReplayName)
{
  FS::deleteFile(std::string("Replays/") + ReplayName + std::string(".rpl"));
}

std::string ReplayManager::giveAutomaticName()
{
  time_t date;
  time(&date);
  struct tm *TL = localtime(&date);

  char date_str[256];
  strftime(date_str, sizeof(date_str)-1, "%d-%m-%y %H_%M", TL);

  return std::string(date_str);
}

void ReplayManager::skipBuffer(FileHandle* pfh)
{
  unsigned int size   = FS::readInt_LE(pfh);
  bool useCompression = FS::readBool(pfh);

  if(useCompression == true) {
    int nCompressedSize = FS::readInt_LE(pfh);
    char* pcCompressed = new char [nCompressedSize];
    FS::readBuf(pfh, pcCompressed, nCompressedSize);
    delete [] pcCompressed;
  } else {
    char* pcData = new char [size];
    FS::readBuf(pfh, pcData, size);
    delete [] pcData;
  }
}
