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

#ifndef __STATEMAINMENU_H__
#define __STATEMAINMENU_H__

#include "StateManager.h"
#include "StateMenu.h"

class Texture;
class UILevelList;
class LevelsPacksCountUpdateThread;
class CheckWwwThread;

class StateMainMenu : public StateMenu {
public:
  StateMainMenu(bool drawStateBehind = false, bool updateStatesBehind = false);
  virtual ~StateMainMenu();

  virtual void enter();
  virtual void leave();
  /* called when a new state is pushed or poped on top of the
     current one*/
  virtual void enterAfterPop();

  virtual bool render();
  /* input */
  virtual void xmKey(InputEventType i_type, const XMKey &i_xmkey);

  static void clean();
  static void refreshStaticCaptions();

  virtual void sendFromMessageBox(const std::string &i_id,
                                  UIMsgBoxButton i_button,
                                  const std::string &i_input);
  virtual void executeOneCommand(std::string cmd, std::string args);

protected:
  virtual void checkEvents();

private:
  /* GUI */
  static UIRoot *m_sGUI;
  static void createGUIIfNeeded(RenderSurface *i_screen);
  static UIWindow *makeWindowReplays(UIWindow *i_parent);
  static UIWindow *makeWindowLevels(UIWindow *i_parent);
  static UIWindow *makeWindowStats(UIWindow *i_parent);

  void updateProfileStrings();

  void updateLevelsPacksCountDetached();
  void updateLevelsPacksList();
  void updateLevelsLists();
  void updateFavoriteLevelsList();
  void updateNewLevelsList();
  void updateReplaysList();
  void updateStats();
  void updateNewLevels();

  void remakePacks();

  /* Main menu background / title */
  Texture *m_pTitleBL, *m_pTitleBR, *m_pTitleTL, *m_pTitleTR;
  void drawBackground();

  /* lists */
  UILevelList *m_quickStartList;
  UILevelList *buildQuickStartList();
  void createLevelListsSql(UILevelList *io_levelsList,
                           const std::string &i_sql);
  void createLevelLists(UILevelList *i_list, const std::string &i_packageName);
  void updateLevelsPackInPackList(const std::string &v_levelPack);

  void checkEventsLevelsFavoriteTab();
  void checkEventsLevelsNewTab();
  void checkEventsNetworkTab();
  void updateClientStrings();
  void checkEventsLevelsMultiTab();
  void checkEventsLevelsPackTab();
  void checkEventsReplays();
  void checkEventsMainWindow();
  void updateOptions();
  void updateInfoFrame();
  void updateReplaysRights();

  UILevelList *getInfoFrameLevelsList();
  std::string getInfoFrameLevelId();

  bool m_require_updateFavoriteLevelsList;
  bool m_require_updateReplaysList;
  bool m_require_updateLevelsList;
  bool m_require_updateStats;

  LevelsPacksCountUpdateThread *m_levelsPacksCountThread;
  bool m_initialLevelsPacksDone;
  CheckWwwThread *m_checkWwwThread;
};

#endif
