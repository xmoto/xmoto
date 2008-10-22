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

#ifndef __REPLAYMANAGER_H__
#define __REPLAYMANAGER_H__

#include "helpers/Singleton.h"
#include <string>
#include <map>

class Replay;
class ISerializer;
class Scene;

class ReplayManager : public Singleton<ReplayManager> {
  friend class Singleton<ReplayManager>;
private:
  ReplayManager();
  ~ReplayManager();

public:
  // recording
  // how to use it:
  // 1. create the replay with createNewReplay
  // 2. add scenes with addRecordingScene. the scene will register each object type it want to record with addObjectTypeToRecord
  // 3. start recording the replay with startRecordingReplay
  // 4. record the frames with recordFrame
  // 5. end the replay with endRecordingReplay

  // level id = "multi" for multilevels replays.
  // player id = "multi" for multiplayers replays
  void createNewReplay();
  // we record scenes in the replay, a scene is linked to a level,
  // there can be 1 to n players in a scene.
  void addRecordingScene(Scene* pScene);
  // we need serializer where each ISerialisable object stored its own frames (like blocks and bikes).
  void addObjectTypeToRecord(Scene* pScene, std::string name);

  // and serializer which stored all the frames (like events)
  //  void addObjectTypeToRecord(Scene* pScene, ISerializer* pSer);

  void startRecordingReplay();
  void recordFrame();
  void endRecordingReplay();

  Replay* getCurrentRecordingReplay();

  void enableCompression(bool enabled) {
    m_compressionEnabled = enabled;
  }

  // stop recording after a cheated teleportation
  void disableRecordingReplay() {
    m_recordReplay = false;
  }

  // playing
  //void addPlayingReplay(Scene* pScene, std::string& fileName, bool mainReplay);
  // temporary returns the Replay*
  Ghost* addPlayingReplay(Scene* pScene, bool mainReplay, std::string& fileName,
			   std::string i_info,
			   Theme *i_theme, BikerTheme* i_bikerTheme,
			   const TColor& i_filterColor,
			   const TColor& i_filterUglyColor);
  void playFrame();
  void fastforward(int i_time);
  // i_minimumNbFrame, because sometimes, rewind do nothing if i_time
  // is too small
  void fastrewind(int  i_time, int i_minimumNbFrame = 0);

  void reinitializePlayingReplays();

  // util
  void deleteReplay(std::string& ReplayName);
  std::string giveAutomaticName();

  void skipBuffer(FileHandle* pfh);

private:
  // what's that string for ??
  std::map<std::string, Replay*>      m_playingReplay;
  std::map<unsigned int, Scene*>      m_scenes;
  std::map<Scene*, std::vector<ISerializer*> > m_scenesSerializersRecording;
  std::map<Scene*, std::vector<ISerializer*> > m_scenesSerializersPlaying;

  bool m_compressionEnabled;

  int m_curTimeStamp;

  bool m_recordReplay;
};

#endif
