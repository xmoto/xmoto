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

#ifndef __STATEPREPLAYING_H__
#define __STATEPREPLAYING_H__

#include "StateScene.h"

class CameraAnimation;
class FontGlyph;

class StatePreplaying : public StateScene {
public:
  StatePreplaying(const std::string i_idlevel, bool i_sameLevel);
  virtual ~StatePreplaying();

  virtual void enter();
  virtual void leave();
  /* called when a new state is pushed or poped on top of the
     current one*/
  virtual void enterAfterPop();
  virtual void leaveAfterPush();

  virtual bool update();
  virtual bool render();
  /* input */
  virtual void xmKey(InputEventType i_type, const XMKey &i_xmkey);

  virtual void sendFromMessageBox(const std::string &i_id,
                                  UIMsgBoxButton i_button,
                                  const std::string &i_input);

protected:
  std::string m_idlevel;
  bool m_sameLevel;
  bool m_animationReady;

  virtual void initUniverse() = 0;
  virtual void preloadLevels() = 0;
  virtual void initPlayers() = 0;
  virtual void runPlaying() = 0;
  virtual bool shouldBeAnimated() const;

  virtual void addLocalGhosts();
  virtual void addWebGhosts();
  virtual bool allowGhosts();

  // which state to display on failure
  virtual void onLoadingFailure(const std::string &i_msg);

private:
  void secondInitPhase();

  bool m_playAnimation; // must the animation be played ; must be rearmed each
  // time you play a new level

  /* animation */
  CameraAnimation *m_cameraAnim;

  unsigned int m_ghostDownloadMessageType;

  bool m_isInitialized; // because you can keep this state open initialized if a
  // msg box appears
  std::string m_errorLoading;
};

#endif
