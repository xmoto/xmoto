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

#include <vector>
#include "xmscene/Scene.h"

class MotoGame;
class XMMotoGameHooks;

class XMMotoGameHooks : public MotoGameHooks {
public:
  XMMotoGameHooks();
  virtual ~XMMotoGameHooks();
  void setGameApps(MotoGame *i_MotoGame);
  void OnTakeEntity();

private:
  MotoGame *m_MotoGame;
};

class Universe {
  public:
  Universe();
  ~Universe();

  std::vector<MotoGame*>& getScenes();
  MotoGame* getScene(std::string sceneId);
  void initPlay(int i_nbPlayer, bool i_multiScenes);

  Replay* getCurrentReplay();
  bool isAReplayToSave() const;
  void initReplay();
  void finalizeReplay(bool i_finished);   /* call to close the replay */
  void saveReplay(const std::string &Name);
  void isTheCurrentPlayAHighscore(bool& o_personal, bool& o_room);
  void TeleportationCheatTo(int i_player, Vector2f i_position);
  void switchFollowCamera();

  private:
  std::vector<MotoGame*>        m_scenes; /* Game objects */
  std::vector<XMMotoGameHooks*> m_motoGameHooks;
  Replay *m_pJustPlayReplay;

  void removeAllWorlds();

  void initCameras(int nbPlayer); // init camera according to the number of players and the existing scenes
  void addScene();
};
