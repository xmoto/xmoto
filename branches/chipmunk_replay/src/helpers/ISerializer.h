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

#ifndef __ISERIALIZER_H__
#define __ISERIALIZER_H__

#include <string>
#include "helpers/Factory.h"
#include "helpers/Singleton.h"
#include "DBuffer.h"
#include "helpers/ISerializable.h"
#include "helpers/Log.h"

class Scene;
class FileHandle;

class SerializerFactory : public Factory, public Singleton<SerializerFactory> {
  friend class Singleton<SerializerFactory>;
private:
  SerializerFactory() {}
  ~SerializerFactory() {}
};


class ISerializer {
public:
  // call a game initialization to register the different serializers
  // in the serializer factory
  static void registerSerializers();

  ISerializer() {
  }
  virtual ~ISerializer() {
  }

  void initBufferPlaying() {
    m_buffer.initOutput(1024);
  }

  void initBufferRecording() {
    m_buffer.initInput(1024);
  }

  // if the serializer need to do things before begining to store frames
  virtual void initStoring(Scene* pScene) = 0;

  // a frame contains all the objects that this Serializer handle. In
  // a frame there's only the objects which have moved since the last
  // frame
  virtual void storeFrame(Scene* pScene) = 0;

  // we userialize all the frames and put individual frames
  // in each ISerializable objects
  virtual void unserializeFrames(Scene* pScene) = 0;

  // apply recorded state to objects
  virtual void playFrame(Scene* pScene) = 0;

  DBuffer* getBuffer() {
    return &m_buffer;
  }

  void loadBuffer(FileHandle* pfh) {
    
  }

  void setPlaying(bool playing) {
    m_playing = playing;
  }

protected:
  DBuffer m_buffer;
  // true: playing
  // false: recording
  bool    m_playing;
};


// store frames in each objects.  we can have the case where there's
// only one ISerializable object, for example with the events, where
// we prefer to store all the replay's events in one object (and the
// events beeing ISerializable objects too).
template <typename T> class ISerializerImplEach : public ISerializer {
public:
  ISerializerImplEach() {
  }
  virtual ~ISerializerImplEach() {
  }

  void unserializeFrames(Scene* pScene) {
    int frameId;
    int timeStamp;
    std::string id = "";
    ISerializable<T>* pObject;
    T* pObjectState;

    try {
      while(m_buffer.numRemainingBytes() > 0) {
	unsigned int nbObjects;
	m_buffer >> nbObjects;

	for(unsigned int i=0; i<nbObjects; i++){
	  m_buffer >> id;
	  pObject = getObject(id, pScene);
	  pObjectState = new T;
	  pObject->unserializeOneState(m_buffer, *pObjectState);
	  pObject->addState(pObjectState);
	}
      }
    } catch(Exception& e) {
      LogWarning("unable to unserialize object [%s].", id.c_str());
    }
  }

protected:
  // the id is stored in the serialized state, get the actual object
  // from the scene using its id.
  // throw an exception if the object doesn't exist.
  virtual ISerializable<T>* getObject(std::string id, Scene* pScene) = 0;
};

#endif