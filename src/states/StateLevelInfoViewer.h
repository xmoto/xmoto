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

#ifndef __STATELEVELINFOVIEWER_H__
#define __STATELEVELINFOVIEWER_H__

#include "StateManager.h"
#include "StateMenu.h"

class UIRoot;

class StateLevelInfoViewer : public StateMenu {
public:
  StateLevelInfoViewer(const std::string &level,
                       bool drawStateBehind = true,
                       bool updateStatesBehind = true);
  virtual ~StateLevelInfoViewer();

  virtual void enter();
  /* called when a new state is pushed or poped on top of the
     current one*/
  virtual void enterAfterPop();

  /* input */
  virtual void xmKey(InputEventType i_type, const XMKey &i_xmkey);

  static void clean();

protected:
  virtual void checkEvents();

  virtual void sendFromMessageBox(const std::string &i_id,
                                  UIMsgBoxButton i_button,
                                  const std::string &i_input);
  virtual void executeOneCommand(std::string cmd, std::string args);

private:
  /* GUI */
  static UIRoot *m_sGUI;
  static void createGUIIfNeeded(RenderSurface *i_screen);
  void updateGUI();

  void updateLevelInfoViewerBestTimes();
  void updateLevelInfoViewerReplays();

  std::string m_level;
};

#endif
