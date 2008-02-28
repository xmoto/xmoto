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
  StateMainMenu(bool drawStateBehind    = false,
		bool updateStatesBehind = false);
  virtual ~StateMainMenu();
  
  virtual void enter();
  /* called when a new state is pushed or poped on top of the
     current one*/
  virtual void enterAfterPop();
  
  virtual bool render();
  /* input */
  virtual void keyDown(int nKey, SDLMod mod,int nChar);
  
  static void clean();
  static void refreshStaticCaptions();
  
  virtual void send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input);
  virtual void send(const std::string& i_id, const std::string& i_message);
  virtual void executeOneCommand(std::string cmd);

  protected:
  virtual void checkEvents();

  private:
  /* GUI */
  static UIRoot* m_sGUI;
  static void createGUIIfNeeded();
  static UIWindow* makeWindowReplays(UIWindow* i_parent);
  static UIWindow* makeWindowOptions(UIWindow* i_parent);
  static UIWindow* makeWindowLevels(UIWindow* i_parent);
  static UIWindow* makeWindowStats(UIWindow* i_parent);
  static UIWindow* makeWindowOptions_general(UIWindow* i_parent);
  static UIWindow* makeWindowOptions_video(UIWindow* i_parent);
  static UIWindow* makeWindowOptions_audio(UIWindow* i_parent);
  static UIWindow* makeWindowOptions_controls(UIWindow* i_parent);
  static UIWindow* makeWindowOptions_rooms(UIWindow* i_parent);
  static UIWindow* makeWindowOptions_ghosts(UIWindow* i_parent);
  static UIWindow* makeWindowOptions_language(UIWindow* i_parent);

  void updateProfile();

  void updateLevelsPacksList();
  void updateLevelsLists();
  void updateFavoriteLevelsList();
  void updateNewLevelsList();
  void updateReplaysList();
  void updateStats();
  void updateNewLevels();

  /* options */
  void createThemesList(UIList *pList);
  void updateThemesList();
  void updateResolutionsList();
  void updateControlsList();
  void createRoomsList(UIList *pList);
  void updateRoomsList();
  void updateOptions();
  void updateAudioOptions();
  void updateWWWOptions();
  void updateGhostsOptions();

  /* Main menu background / title */
  Texture *m_pTitleBL,*m_pTitleBR,*m_pTitleTL,*m_pTitleTR;      
  void drawBackground(); 

  /* lists */
  UILevelList* m_quickStartList;
  UILevelList* buildQuickStartList();
  void createLevelListsSql(UILevelList* io_levelsList, const std::string& i_sql);
  void createLevelLists(UILevelList *i_list, const std::string& i_packageName);
  void updateLevelsPackInPackList(const std::string& v_levelPack);

  void checkEventsLevelsFavoriteTab();
  void checkEventsLevelsNewTab();
  void checkEventsLevelsMultiTab();
  void checkEventsLevelsPackTab();
  void checkEventsReplays();
  void checkEventsMainWindow();
  void checkEventsOptions();

  void updateInfoFrame();
  void updateReplaysRights();

  std::string getInfoFrameLevelId();
  void setInputKey(const std::string& i_strKey, const std::string& i_key);

  bool m_require_updateFavoriteLevelsList;
  bool m_require_updateReplaysList;
  bool m_require_updateLevelsList;
  bool m_require_updateStats;
};

#endif
