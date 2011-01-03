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

#ifndef __STATEOPTIONS_H__
#define __STATEOPTIONS_H__

#include "StateManager.h"
#include "StateMenu.h"

class StateOptions : public StateMenu {
  public:
  StateOptions(bool drawStateBehind     = true,
		bool updateStatesBehind = false);
  virtual ~StateOptions();

  virtual void enter();
  static void clean();  

  virtual void sendFromMessageBox(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input);
  virtual void executeOneCommand(std::string cmd, std::string args);

  protected:
  virtual void checkEvents();

  private:
  /* GUI */
  static UIRoot* m_sGUI;
  static void createGUIIfNeeded(RenderSurface* i_screen);

  static UIWindow* makeWindowOptions_general(UIWindow* i_parent);
  static UIWindow* makeWindowOptions_video(UIWindow* i_parent);
  static UIWindow* makeWindowOptions_audio(UIWindow* i_parent);
  static UIWindow* makeWindowOptions_controls(UIWindow* i_parent);
  static UIWindow* makeWindowOptions_rooms(UIWindow* i_parent);
  static UIWindow* makeWindowOptions_ghosts(UIWindow* i_parent);
  static UIWindow* makeWindowOptions_db(UIWindow* i_parent);
  static UIWindow* makeWindowOptions_language(UIWindow* i_parent);
  static UIWindow* makeRoomTab(UIWindow* i_parent, unsigned int i_number);
  static UIWindow* makeWindowOptions_infos(UIWindow* i_parent);
  static void makeWindowOptions_infos_line(UIWindow* i_parent, const std::string& i_name, const std::string& i_value, int hpos);

  void updateOptions();
  void createThemesList(UIList *pList);
  void updateThemesList();
  void updateResolutionsList();
  void updateControlsList();
  void createRoomsList(UIList *pList);
  void updateRoomsList();
  void updateAudioOptions();
  void updateWWWOptions();
  void updateDbOptions();
  void updateTrailCamOptions();
  void updateGhostsOptions();
  void updateMedalOptions();
  void updateProfileStrings();
  void updateJoysticksStrings();
  void updateServerStrings();

  void setInputKey(const std::string& i_strKey, const std::string& i_key);
};

#endif
