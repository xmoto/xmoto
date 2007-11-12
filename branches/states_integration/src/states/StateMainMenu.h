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

class StateMainMenu : public StateMenu {
  public:
  StateMainMenu(GameApp* pGame,
		bool drawStateBehind    = false,
		bool updateStatesBehind = false
		);
  virtual ~StateMainMenu();
  
  virtual void enter();
  virtual void leave();
  /* called when a new state is pushed or poped on top of the
     current one*/
  virtual void enterAfterPop();
  virtual void leaveAfterPush();
  
  virtual bool update();
  virtual bool render();
  /* input */
  virtual void keyDown(int nKey, SDLMod mod,int nChar);
  virtual void keyUp(int nKey,   SDLMod mod);
  virtual void mouseDown(int nButton);
  virtual void mouseDoubleClick(int nButton);
  virtual void mouseUp(int nButton);
  
  static void clean();
  
  virtual void send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input);
  virtual void executeOneCommand(std::string cmd);

  protected:
  virtual void checkEvents();

  private:
  /* GUI */
  static UIRoot* m_sGUI;
  static void createGUIIfNeeded(GameApp* pGame);
  static UIWindow* makeWindowReplays(GameApp* pGame, UIWindow* i_parent);
  static UIWindow* makeWindowOptions(GameApp* pGame, UIWindow* i_parent);
  static UIWindow* makeWindowLevels(GameApp* pGame, UIWindow* i_parent);
  static UIWindow* makeWindowStats(GameApp* pGame, UIWindow* i_parent);
  static UIWindow* makeWindowOptions_general(GameApp* pGame, UIWindow* i_parent);
  static UIWindow* makeWindowOptions_video(GameApp* pGame, UIWindow* i_parent);
  static UIWindow* makeWindowOptions_audio(GameApp* pGame, UIWindow* i_parent);
  static UIWindow* makeWindowOptions_controls(GameApp* pGame, UIWindow* i_parent);
  static UIWindow* makeWindowOptions_rooms(GameApp* pGame, UIWindow* i_parent);
  static UIWindow* makeWindowOptions_ghosts(GameApp* pGame, UIWindow* i_parent);

  void updateProfile();

  void updateLevelsPacksList();
  void updateLevelsLists();
  void updateFavoriteLevelsList();
  void updateNewLevelsList();
  void updateReplaysList();
  void updateStats();

  /* options */
  void createThemesList(UIList *pList);
  void updateThemesList();
  void updateResolutionsList();
  void updateControlsList();
  void createRoomsList(UIList *pList);
  void updateRoomsList();
  void updateOptions();
  
  /* Main menu background / title */
  Texture *m_pTitleBL,*m_pTitleBR,*m_pTitleTL,*m_pTitleTR;      
  void drawBackground(); 

  /* lists */
  UILevelList* m_quickStartList;
  UILevelList* buildQuickStartList();
  void createLevelListsSql(UILevelList* io_levelsList, const std::string& i_sql);
  void createLevelLists(UILevelList *i_list, const std::string& i_packageName);

  void checkEventsLevelsFavoriteTab();
  void checkEventsLevelsNewTab();
  void checkEventsLevelsMultiTab();
  void checkEventsLevelsPackTab();
  void checkEventsReplays();
  void checkEventsMainWindow();
  void checkEventsOptions();
};

#endif
