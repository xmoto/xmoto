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

ReplayManager()
~ReplayManager()

void createNewReplay()
{
}

void addRecordingScene(Scene* pScene)
{
  m_scenes[m_scenes.size()] = pScene;
}

void addObjectTypeToRecord(Scene* pScene, std::string name)
{
  m_scenesSerializersRecording[pScene] = (ISerializer*)SerializerFactory::instance()->createObject(name);
}

void addObjectTypeToRecord(Scene* pScene, ISerializer* pSerializer)
{
  m_scenesSerializersRecording[pScene] = pSerializer
}

void startRecordingReplay()
{
  m_recordReplay = true;
}

void recordFrame()
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

void endRecordingReplay()
{
}

Replay* getCurrentRecordingReplay()
{
}

void addPlayingReplay(Scene* pScene, std::string& fileName)
{
  // version 0: only player chunks
  // version 1: add events
  // version 2: generic handling

  Replay* pReplay = new Replay();
  // read the header of the replay, keeps the file handle open
  FileHandle* pfh = pReplay->openReplay(fileName);

  switch(pReplay->getVersion()) {
  case 0: {
    // only biker
    ISerializer* pBike = SerializerFactory::instance()->createObject("BikerSerializer");
    pBike->loadBuffer(pfh);
    m_scenesSerializersPlaying[pScene].push_back(pBike);
  }
    break;
  case 1: {
    // biker and events
    ISerializer* pBike  = SerializerFactory::instance()->createObject("BikerSerializer");
    ISerializer* pEvent = SerializerFactory::instance()->createObject("MotoGameEventManager");
    pBike->loadBuffer(pfh);
    pEvent->loadBuffer(pfh);
    m_scenesSerializersPlaying[pScene].push_back(pBike);
    m_scenesSerializersPlaying[pScene].push_back(pEvent);
  }
    break;
  case 2: {
    // everything
    int nbSavedStuffs  = FS::readByte(pfh); 
    for(int i=0; i<nbSavedStuffs; i++) {
      std::string stuff = FS::readString(pfh);
      ISerializer* pSer = SerializerFactory::instance()->createObject(stuff);
      pSer->loadBuffer(pfh);
      m_scenesSerializersPlaying[pScene].push_back(pSer);
    }

  }
    break;
  default:
    LogError();
  }

  FS::closeFile(pfh);
}

void playFrame()
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

void fastforward(int i_time)
{
}

void fastrewind(int  i_time, int i_minimumNbFrame = 0)
{
}

void reinitializePlayingReplays()
{
}

void deleteReplay(std::string& ReplayName)
{
}

std::string giveAutomaticName()
{
}
