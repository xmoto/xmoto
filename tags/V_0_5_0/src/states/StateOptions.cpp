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

#include "StateOptions.h"
#include "../GameText.h"
#include "../Languages.h"
#include "../Game.h"
#include "../drawlib/DrawLib.h"
#include "../helpers/Text.h"
#include "../helpers/System.h"
#include "StateUpdateThemesList.h"
#include "StateUpdateRoomsList.h"
#include "StateUploadAllHighscores.h"
#include "StateUpdateTheme.h"
#include "StateCheckWww.h"
#include "StateSync.h"
#include "../Sound.h"
#include "StateRequestKey.h"
#include "StateEditWebConfig.h"
#include "../helpers/Log.h"
#include "../helpers/CmdArgumentParser.h"
#include "StateMessageBox.h"
#include "../SysMessage.h"
#include "../net/NetServer.h"
#include <sstream>

/* static members */
UIRoot*  StateOptions::m_sGUI = NULL;

StateOptions::StateOptions(bool drawStateBehind, bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind) {
  m_name = "StateOptions";

  StateManager::instance()->registerAsObserver("UPDATEPROFILE", this);
  StateManager::instance()->registerAsObserver("THEMES_UPDATED", this);
  StateManager::instance()->registerAsObserver("ROOMS_UPDATED", this);
  StateManager::instance()->registerAsObserver("CHANGE_WWW_ACCESS", this);
  StateManager::instance()->registerAsObserver("CONFIGURE_WWW_ACCESS", this);
  StateManager::instance()->registerAsObserver("ENABLEAUDIO_CHANGED", this);
  StateManager::instance()->registerAsObserver("REQUESTKEY", this);
  StateManager::instance()->registerAsObserver("SERVER_STATUS_CHANGED", this);
}

StateOptions::~StateOptions() {
  StateManager::instance()->unregisterAsObserver("UPDATEPROFILE", this);
  StateManager::instance()->unregisterAsObserver("THEMES_UPDATED", this);
  StateManager::instance()->unregisterAsObserver("ROOMS_UPDATED", this);
  StateManager::instance()->unregisterAsObserver("CHANGE_WWW_ACCESS", this);
  StateManager::instance()->unregisterAsObserver("CONFIGURE_WWW_ACCESS", this);
  StateManager::instance()->unregisterAsObserver("ENABLEAUDIO_CHANGED", this);
  StateManager::instance()->unregisterAsObserver("REQUESTKEY", this);
  StateManager::instance()->unregisterAsObserver("SERVER_STATUS_CHANGED", this);
}

void StateOptions::enter()
{ 
  createGUIIfNeeded();
  m_GUI = m_sGUI;

  updateOptions();
  updateServerStrings();
  updateJoysticksStrings();
}

void StateOptions::clean() {
  if(StateOptions::m_sGUI != NULL) {
    delete StateOptions::m_sGUI;
    StateOptions::m_sGUI = NULL;
  }
}

void StateOptions::checkEvents() {
  UIButton*    v_button;
  UIEdit*      v_edit;
  UIList*      v_list;
  std::string  v_id_level;

  // default button
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:DEFAULTS_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    StateMessageBox* v_msgboxState = new StateMessageBox(this, GAMETEXT_RESETTODEFAULTS, UI_MSGBOX_YES|UI_MSGBOX_NO);
    v_msgboxState->setId("RESETSTODEFAULTS");
    v_msgboxState->makeActiveButton(UI_MSGBOX_NO);
    StateManager::instance()->pushState(v_msgboxState);
  }

  // close
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:CLOSE_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    StateManager::instance()->sendAsynchronousMessage("OPTIONS_UPDATED");

    m_requestForEnd = true;
  }

  // general tab
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:SHOWMINIMAP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setShowMinimap(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:SHOWENGINECOUNTER"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setShowEngineCounter(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:INITZOOM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setEnableInitZoom(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:DEATHANIM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setEnableDeadAnimation(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:CAMERAACTIVEZOOM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setEnableActiveZoom(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:BEATINGMODE"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setBeatingMode(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:ENABLECONTEXTHELP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setEnableContextHelp(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:AUTOSAVEREPLAYS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAutosaveHighscoreReplays(v_button->getChecked()); 
  }

  v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:THEME_TAB:LIST"));
  if(v_list->isClicked()) {
    v_list->setClicked(false);
    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      XMSession::instance()->setTheme(pEntry->Text[0]);
      /* don't update theme, because it invalidates the relationship
	 between sprites and textures.
      if(Theme::instance()->Name() != XMSession::instance()->theme()) {
	GameApp::instance()->reloadTheme();
      }
      */
    }
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:THEME_TAB:UPDATE_LIST"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    StateManager::instance()->pushState(new StateUpdateThemesList());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:THEME_TAB:GET_SELECTED"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      StateManager::instance()->pushState(new StateUpdateTheme(pEntry->Text[0]));
    }
  }

  // video tab
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:16BPP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setBpp(16); 
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:32BPP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setBpp(32); 
  }

  v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:RESOLUTIONS_LIST"));
  if(v_list->isClicked()) {
    v_list->setClicked(false);
    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      int nW,nH;

      sscanf(pEntry->Text[0].c_str(),"%d X %d", &nW, &nH);
      XMSession::instance()->setResolutionWidth(nW);
      XMSession::instance()->setResolutionHeight(nH);
    }
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:WINDOWED"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setWindowed(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:MENULOW"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setMenuGraphics(GFX_LOW);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:MENUMEDIUM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setMenuGraphics(GFX_MEDIUM);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:MENUHIGH"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setMenuGraphics(GFX_HIGH);
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:GAMELOW"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGameGraphics(GFX_LOW);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:GAMEMEDIUM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGameGraphics(GFX_MEDIUM);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:GAMEHIGH"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGameGraphics(GFX_HIGH);
  }

  // sound
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:ENABLE_AUDIO"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    XMSession::instance()->enableAudio();
    XMSession::instance()->setEnableAudio(v_button->getChecked());
    Sound::setActiv(XMSession::instance()->enableAudio());
    updateAudioOptions();
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:RATE11KHZ"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAudioSampleRate(11025);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:RATE22KHZ"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAudioSampleRate(22050);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:RATE44KHZ"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAudioSampleRate(44100);
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:8BITS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAudioSampleBits(8);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:16BITS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAudioSampleBits(16);
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:MONO"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAudioChannels(1);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:STEREO"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAudioChannels(2);
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:ENABLE_ENGINE_SOUND"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setEnableEngineSound(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:ENABLE_MENU_MUSIC"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    XMSession::instance()->setEnableMenuMusic(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:ENABLE_GAME_MUSIC"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    XMSession::instance()->setEnableGameMusic(v_button->getChecked());
    updateAudioOptions();
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:ENABLE_MUSIC_ON_ALL_LEVELS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    XMSession::instance()->setMusicOnAllLevels(v_button->getChecked());
  }

  // controls
  v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:CONTROLS_TAB:KEY_ACTION_LIST"));
  if(v_list->isItemActivated()) {
    v_list->setItemActivated(false);

    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      char cBuf[1024];                

      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      snprintf(cBuf, 1024, GAMETEXT_PRESSANYKEYTO, pEntry->Text[0].c_str());
      StateManager::instance()->pushState(new StateRequestKey(cBuf, this));      
    }
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:CONTROLS_TAB:ENABLEJOYSTICKS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    XMSession::instance()->setEnableJoysticks(v_button->getChecked());
    InputHandler::instance()->enableJoysticks(XMSession::instance()->enableJoysticks());
  }


  // www
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:PROXYCONFIG"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    StateManager::instance()->pushState(new StateEditWebConfig());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:UPDATEHIGHSCORES"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    StateManager::instance()->pushState(new StateCheckWww(true));
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLEWEB"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setWWW(v_button->getChecked());
    updateWWWOptions();
    updateDbOptions();
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKNEWLEVELSATSTARTUP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setCheckNewLevelsAtStartup(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKHIGHSCORESATSTARTUP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setCheckNewHighscoresAtStartup(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:INGAMEWORLDRECORD"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setShowHighscoreInGame(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:USECRAPPYINFORMATION"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setUseCrappyPack(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:ALLOWWEBFORMS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setWebForms(v_button->getChecked());
  }


  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:WWW_PASSWORD"));
  if(v_edit->hasChanged()) {
    v_edit->setHasChanged(false);
    XMSession::instance()->setWwwPassword(v_edit->getCaption());
  }

  for(unsigned int i=0; i<XMSession::instance()->nbRoomsEnabled(); i++) {
      std::ostringstream v_strRoom;
      v_strRoom << i;

      v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							  + ":ROOMS_LIST"));
      if(v_list->isChanged()) {
	v_list->setChanged(false);
	if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
	  v_list->getEntries()[v_list->getSelected()];
	  XMSession::instance()->setIdRoom(i, *((std::string*)v_list->getEntries()[v_list->getSelected()]->pvUser));
	  updateProfileStrings();
	}
      }

      v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							      + ":UPDATE_ROOMS_LIST"));
      if(v_button->isClicked()) {
	v_button->setClicked(false);
	StateManager::instance()->pushState(new StateUpdateRoomsList());
      }

      v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							      + ":UPLOADHIGHSCOREALL_BUTTON"));
      if(v_button->isClicked()) {
	v_button->setClicked(false);
	StateManager::instance()->pushState(new StateUploadAllHighscores(i));
      }
  }

  for(unsigned int i=1; i<ROOMS_NB_MAX; i++) { // not for 0
      std::ostringstream v_strRoom;
      v_strRoom << i;

      v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							      + ":ROOM_ENABLED"));
      if(v_button->isClicked()) {
	v_button->setClicked(false);
	
	if(v_button->getChecked()) {
	  if(XMSession::instance()->nbRoomsEnabled() < i+1) {
	    XMSession::instance()->setNbRoomsEnabled(i+1);
	    updateWWWOptions();
	  }
	} else {
	  if(XMSession::instance()->nbRoomsEnabled() >= i+1) {
	    XMSession::instance()->setNbRoomsEnabled(i);
	    updateWWWOptions();
	  }
	}
      }
  }

  // server tab
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:SERVER_TAB:STARTATSTARTUP_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setServerStartAtStartup(v_button->getChecked());
  }

  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:SERVER_TAB:PORT"));

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:SERVER_TAB:DEFAULT_PORT"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setServerPort(DEFAULT_SERVERPORT);
    v_edit->enableWindow(false);
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:SERVER_TAB:CUSTOM_PORT"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setServerPort(atoi(v_edit->getCaption().c_str()));
    v_edit->enableWindow(true);
  }

  if(v_edit->isDisabled() == false) {
    if(v_edit->hasChanged() && v_button->getChecked()) {
      v_edit->setHasChanged(false);
      XMSession::instance()->setServerPort(atoi(v_edit->getCaption().c_str()));
    }
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:SERVER_TAB:STARTSTOP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    if(NetServer::instance()->isStarted()) {
      NetServer::instance()->stop();
    } else {
      NetServer::instance()->start();
    }
  }

  // ghosts
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:ENABLE_GHOSTS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setEnableGhosts(v_button->getChecked());
    updateGhostsOptions();
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:GHOST_STRATEGY_MYBEST"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGhostStrategy_MYBEST(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:GHOST_STRATEGY_THEBEST"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGhostStrategy_THEBEST(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:GHOST_STRATEGY_BESTOFREFROOM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGhostStrategy_BESTOFREFROOM(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:GHOST_STRATEGY_BESTOFOTHERROOMS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGhostStrategy_BESTOFOTHERROOMS(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:DISPLAY_GHOST_TIMEDIFFERENCE"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setShowGhostTimeDifference(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:DISPLAY_GHOSTS_INFOS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setShowGhostsInfos(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:DISPLAY_BIKERS_ARROWS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setShowBikersArrows(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:HIDEGHOSTS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setHideGhosts(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:MOTION_BLUR_GHOST"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGhostMotionBlur(v_button->getChecked());
  }

  // db
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:DB_TAB:SYNCHRONIZE_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    StateManager::instance()->pushState(new StateSync());
  }

  //v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:DB_TAB:SYNCHRONIZE_ONQUIT_CKB"));
  //if(v_button->isClicked()) {
  //  v_button->setClicked(false);
  //  XMSession::instance()->setDbsynchronizeOnQuit(v_button->getChecked());
  //}

  // language
  v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:TABS:LANGUAGE_TAB:LANGUAGE_LIST"));
  if(v_list->isClicked()) {
    v_list->setClicked(false);
    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      XMSession::instance()->setLanguage(pEntry->Text[1]);      
      //std::string v_locale = Locales::changeLocale(XMSession::instance()->language());
      //LogInfo("Locales changed to '%s'", v_locale.c_str());
      //StateManager::instance()->refreshStaticCaptions();
    }
  }

}

void StateOptions::createGUIIfNeeded() {
  if(m_sGUI != NULL)
    return;

  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot();
  m_sGUI->setFont(drawlib->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      drawlib->getDispWidth(),
		      drawlib->getDispHeight());

  UIWindow *v_window, *v_frame;
  UIStatic*  v_someText;
  UITabView* v_tabview;
  UIButton*  v_button;

  v_window = new UIFrame(m_sGUI, 5, 20, "",
			 m_sGUI->getPosition().nWidth -5 -5,
			 m_sGUI->getPosition().nHeight -20 -20);
  v_window->setID("MAIN");
   
  v_someText = new UIStatic(v_window, 0, 0, GAMETEXT_OPTIONS, v_window->getPosition().nWidth, 36);
  v_someText->setFont(drawlib->getFontMedium());

  v_tabview  = new UITabView(v_window, 20, 40, "", v_window->getPosition().nWidth-40, v_window->getPosition().nHeight-115);
  v_tabview->setID("TABS");
  v_tabview->setFont(drawlib->getFontSmall());
  v_tabview->setTabContextHelp(0, CONTEXTHELP_GENERAL_OPTIONS);
  v_tabview->setTabContextHelp(1, CONTEXTHELP_WWW_OPTIONS);
  v_tabview->setTabContextHelp(2, CONTEXTHELP_GHOSTS_OPTIONS);
  v_tabview->setTabContextHelp(3, CONTEXTHELP_DB_OPTIONS);
  v_tabview->setTabContextHelp(4, CONTEXTHELP_LANGUAGE_OPTIONS);

  v_frame = makeWindowOptions_general(v_tabview);
  v_frame = makeWindowOptions_rooms(v_tabview);
  v_frame = makeWindowOptions_ghosts(v_tabview);
  v_frame = makeWindowOptions_db(v_tabview);
  v_frame = makeWindowOptions_language(v_tabview);

  v_button = new UIButton(v_window, 20, v_window->getPosition().nHeight-68, GAMETEXT_DEFAULTS, 115, 57);
  v_button->setID("DEFAULTS_BUTTON");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_SMALL);      
  v_button->setContextHelp(CONTEXTHELP_DEFAULTS);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth-115 -20,
			  v_window->getPosition().nHeight-68, GAMETEXT_CLOSE, 115, 57);
  v_button->setID("CLOSE_BUTTON");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_SMALL);      
}

UIWindow* StateOptions::makeWindowOptions_general(UIWindow* i_parent) {
  UIWindow *v_window, *v_mainWindow;
  UIButton*  v_button;
  UIList*    v_list;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  v_mainWindow = new UIWindow(i_parent, 0, 26, GAMETEXT_GENERAL,
			  i_parent->getPosition().nWidth, i_parent->getPosition().nHeight -26);
  v_mainWindow->setID("GENERAL_TAB");

  UITabView *v_generalTabs = new UITabView(v_mainWindow, 0, 0, "", v_mainWindow->getPosition().nWidth, v_mainWindow->getPosition().nHeight);
  v_generalTabs->setID("TABS");
  v_generalTabs->setFont(drawlib->getFontSmall());
  v_generalTabs->setTabContextHelp(0, CONTEXTHELP_GENERAL_MAIN_OPTIONS);
  v_generalTabs->setTabContextHelp(1, CONTEXTHELP_THEME_OPTIONS);
  v_generalTabs->setTabContextHelp(2, CONTEXTHELP_VIDEO_OPTIONS);
  v_generalTabs->setTabContextHelp(3, CONTEXTHELP_AUDIO_OPTIONS);
  v_generalTabs->setTabContextHelp(4, CONTEXTHELP_CONTROL_OPTIONS);


  /* main */
  v_window = new UIWindow(v_generalTabs, 20, 30, GAMETEXT_MAIN, v_generalTabs->getPosition().nWidth-30, v_generalTabs->getPosition().nHeight);
  v_window->setID("MAIN_TAB");

  v_button = new UIButton(v_window, 5, 33-10, GAMETEXT_SHOWMINIMAP, (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("SHOWMINIMAP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_MINI_MAP);
  
  v_button = new UIButton(v_window, 5, 63-10, GAMETEXT_SHOWENGINECOUNTER, (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("SHOWENGINECOUNTER");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_ENGINE_COUNTER);

  v_button = new UIButton(v_window, 5, 93-10, GAMETEXT_ENABLECONTEXTHELP, (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLECONTEXTHELP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_SHOWCONTEXTHELP);
 
  v_button = new UIButton(v_window, 5, 123-10, GAMETEXT_AUTOSAVEREPLAYS, (v_window->getPosition().nWidth-40)/*/2*/, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("AUTOSAVEREPLAYS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_AUTOSAVEREPLAYS);

  v_button = new UIButton(v_window, 5+(v_window->getPosition().nWidth+40)/2, 33-10, GAMETEXT_INITZOOM,
			  (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("INITZOOM");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_INITZOOM);

  v_button = new UIButton(v_window, 5+(v_window->getPosition().nWidth+40)/2, 63-10, GAMETEXT_DEATHANIM,
			  (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("DEATHANIM");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_DEATHANIM);

  /* Button to enable/disable active zoom */
  v_button = new UIButton(v_window, 5+(v_window->getPosition().nWidth+40)/2, 93-10, GAMETEXT_CAMERAACTIVEZOOM,
			  (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("CAMERAACTIVEZOOM");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_CAMERAACTIVEZOOM);

  /* Button to enable/disable beating mode */
  v_button = new UIButton(v_window, 5+(v_window->getPosition().nWidth+40)/2, 123-10, GAMETEXT_BEATINGMODE,
			  (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("BEATINGMODE");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_BEATINGMODE);

  /* theme */
  v_window = new UIWindow(v_generalTabs, 20, 30, GAMETEXT_THEME, v_generalTabs->getPosition().nWidth-30, v_generalTabs->getPosition().nHeight);
  v_window->setID("THEME_TAB");
  v_window->showWindow(false);

  v_list = new UIList(v_window, 5, 20, "", 
		      v_window->getPosition().nWidth-20, v_window->getPosition().nHeight-95-20);
  v_list->setID("LIST");
  v_list->setFont(drawlib->getFontSmall());
  v_list->addColumn(GAMETEXT_THEMES, (v_list->getPosition().nWidth*3) / 5);
  v_list->addColumn("", (v_list->getPosition().nWidth*2) / 5);
  v_list->setContextHelp(CONTEXTHELP_THEMES);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth -200 -200, v_window->getPosition().nHeight - 95,
			  GAMETEXT_UPDATETHEMESLIST, 207, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("UPDATE_LIST");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_UPDATETHEMESLIST);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth -200, v_window->getPosition().nHeight - 95,
			  GAMETEXT_GETSELECTEDTHEME, 207, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("GET_SELECTED");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GETSELECTEDTHEME);

  v_window = makeWindowOptions_video(v_generalTabs);
  v_window = makeWindowOptions_audio(v_generalTabs);
  v_window = makeWindowOptions_controls(v_generalTabs);

  return v_mainWindow;
}

UIWindow* StateOptions::makeWindowOptions_video(UIWindow* i_parent) {
  UIWindow*  v_window;
  UIButton*  v_button;
  UIList*    v_list;
  UIStatic*  v_someText;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  v_window = new UIWindow(i_parent, 20, 40, GAMETEXT_VIDEO, i_parent->getPosition().nWidth-40, i_parent->getPosition().nHeight);
  v_window->setID("VIDEO_TAB");
  v_window->showWindow(false);

  v_button = new UIButton(v_window, 5, 5, GAMETEXT_16BPP, (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("16BPP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20023);
  v_button->setContextHelp(CONTEXTHELP_HIGHCOLOR);

  v_button = new UIButton(v_window, 5 + (v_window->getPosition().nWidth-40)/2, 5, GAMETEXT_32BPP,
			  (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("32BPP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20023);
  v_button->setContextHelp(CONTEXTHELP_TRUECOLOR);
    
  v_list = new UIList(v_window, 5, 43, "", v_window->getPosition().nWidth - 10, v_window->getPosition().nHeight - 43 - 10 - 140);
  v_list->setID("RESOLUTIONS_LIST");
  v_list->setFont(drawlib->getFontSmall());
  v_list->addColumn(GAMETEXT_SCREENRES, v_list->getPosition().nWidth, CONTEXTHELP_SCREENRES);
  v_list->setContextHelp(CONTEXTHELP_RESOLUTION);

  v_button = new UIButton(v_window,5, v_window->getPosition().nHeight - 43 - 10 - 90, GAMETEXT_RUNWINDOWED,
			  v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("WINDOWED");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_RUN_IN_WINDOW);

  v_someText = new UIStatic(v_window, 5, v_window->getPosition().nHeight - 43 - 10 - 60, std::string(GAMETEXT_MENUGFX) +":", 120, 28);
  v_someText->setFont(drawlib->getFontSmall());    

  v_button = new UIButton(v_window, 120, v_window->getPosition().nHeight - 43 - 10 - 60, GAMETEXT_LOW,
			  (v_window->getPosition().nWidth-120)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("MENULOW");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20024);
  v_button->setContextHelp(CONTEXTHELP_LOW_MENU);
  
  v_button = new UIButton(v_window, 120+(v_window->getPosition().nWidth-120)/3,
			  v_window->getPosition().nHeight - 43 - 10 - 60, GAMETEXT_MEDIUM,(v_window->getPosition().nWidth-120)/3,28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("MENUMEDIUM");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20024);
  v_button->setContextHelp(CONTEXTHELP_MEDIUM_MENU);
  
  v_button = new UIButton(v_window, 120+(v_window->getPosition().nWidth-120)/3*2,
			  v_window->getPosition().nHeight - 43 - 10 - 60, GAMETEXT_HIGH,(v_window->getPosition().nWidth-120)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("MENUHIGH");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20024);
  v_button->setContextHelp(CONTEXTHELP_HIGH_MENU);
  
  v_someText = new UIStatic(v_window, 5, v_window->getPosition().nHeight - 43 - 10 - 30, std::string(GAMETEXT_GAMEGFX) + ":", 120, 28);
  v_someText->setFont(drawlib->getFontSmall());    

  v_button = new UIButton(v_window, 120, v_window->getPosition().nHeight - 43 - 10 - 30,
			  GAMETEXT_LOW,(v_window->getPosition().nWidth-120)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("GAMELOW");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20025);
  v_button->setContextHelp(CONTEXTHELP_LOW_GAME);

  v_button = new UIButton(v_window, 120+(v_window->getPosition().nWidth-120)/3, v_window->getPosition().nHeight - 43 - 10 - 30,
			  GAMETEXT_MEDIUM,(v_window->getPosition().nWidth-120)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("GAMEMEDIUM");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20025);
  v_button->setContextHelp(CONTEXTHELP_MEDIUM_GAME);

  v_button = new UIButton(v_window, 120+(v_window->getPosition().nWidth-120)/3*2, v_window->getPosition().nHeight - 43 - 10 - 30,
			  GAMETEXT_HIGH, (v_window->getPosition().nWidth-120)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("GAMEHIGH");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20025);
  v_button->setContextHelp(CONTEXTHELP_HIGH_GAME);

  return v_window;
}

UIWindow* StateOptions::makeWindowOptions_audio(UIWindow* i_parent) {
  UIWindow*  v_window;
  UIButton*  v_button;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  v_window = new UIWindow(i_parent, 20, 40, GAMETEXT_AUDIO, i_parent->getPosition().nWidth-40, i_parent->getPosition().nHeight);
  v_window->setID("AUDIO_TAB");
  v_window->showWindow(false);

  v_button = new UIButton(v_window, 5, 5, GAMETEXT_ENABLEAUDIO, v_window->getPosition().nWidth-10, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLE_AUDIO");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_SOUND_ON);
    
  v_button = new UIButton(v_window, 25, 33, GAMETEXT_11KHZ, (v_window->getPosition().nWidth-40)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("RATE11KHZ");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(10023);
  v_button->setContextHelp(CONTEXTHELP_11HZ);
    
  v_button = new UIButton(v_window, 25+(v_window->getPosition().nWidth-40)/3, 33, GAMETEXT_22KHZ,
			  (v_window->getPosition().nWidth-40)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("RATE22KHZ");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(10023);
  v_button->setContextHelp(CONTEXTHELP_22HZ);
    
  v_button = new UIButton(v_window, 25+(v_window->getPosition().nWidth-40)/3*2, 33, GAMETEXT_44KHZ,
			  (v_window->getPosition().nWidth-40)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("RATE44KHZ");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(10023);
  v_button->setContextHelp(CONTEXTHELP_44HZ);

  v_button = new UIButton(v_window, 25, 61, GAMETEXT_8BIT, (v_window->getPosition().nWidth-40)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("8BITS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(10024);
  v_button->setContextHelp(CONTEXTHELP_8BIT);

  v_button = new UIButton(v_window, 25+(v_window->getPosition().nWidth-40)/3, 61, GAMETEXT_16BIT,
			  (v_window->getPosition().nWidth-40)/3, 28);    
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("16BITS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(10024);
  v_button->setContextHelp(CONTEXTHELP_16BIT);

  v_button = new UIButton(v_window, 25, 89, GAMETEXT_MONO, (v_window->getPosition().nWidth-40)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("MONO");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(10025);
  v_button->setContextHelp(CONTEXTHELP_MONO);
    
  v_button = new UIButton(v_window, 25+(v_window->getPosition().nWidth-40)/3, 89, GAMETEXT_STEREO,
			  (v_window->getPosition().nWidth-40)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("STEREO");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(10025);
  v_button->setContextHelp(CONTEXTHELP_STEREO);

  v_button = new UIButton(v_window, 5, 117, GAMETEXT_ENABLEENGINESOUND, v_window->getPosition().nWidth-10, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLE_ENGINE_SOUND");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_ENGINE_SOUND);
    
  v_button = new UIButton(v_window, 5, 145, GAMETEXT_ENABLEMENUMUSIC, v_window->getPosition().nWidth-10, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLE_MENU_MUSIC");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_MENUMUSIC);

  v_button = new UIButton(v_window, 5, 173, GAMETEXT_ENABLEGAMEMUSIC, v_window->getPosition().nWidth-10, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLE_GAME_MUSIC");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GAMEMUSIC);

  v_button = new UIButton(v_window, 25, 201, GAMETEXT_MUSICONALLLEVELS, v_window->getPosition().nWidth-10, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLE_MUSIC_ON_ALL_LEVELS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_MUSICONALLLEVELS);

  return v_window;
}

UIWindow* StateOptions::makeWindowOptions_controls(UIWindow* i_parent) {
  UIWindow*  v_window;
  UIList*    v_list;
  UIStatic* v_someText;
  UIButton* v_button;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  v_window = new UIWindow(i_parent, 20, 40, GAMETEXT_CONTROLS, i_parent->getPosition().nWidth-40, i_parent->getPosition().nHeight);
  v_window->setID("CONTROLS_TAB");
  v_window->showWindow(false);

  v_list = new UIList(v_window, 5, 5, "", v_window->getPosition().nWidth-10, v_window->getPosition().nHeight -43 -5 -57);
  v_list->setID("KEY_ACTION_LIST");
  v_list->setFont(drawlib->getFontSmall());
  v_list->addColumn(GAMETEXT_ACTION, 250);
  v_list->addColumn(GAMETEXT_KEY, v_list->getPosition().nWidth - 250);
  v_list->addColumn("", 0); // internal key name
  v_list->setContextHelp(CONTEXTHELP_SELECT_ACTION);

  v_button = new UIButton(v_window, 5, v_window->getPosition().nHeight -43 -57 +10,
			  GAMETEXT_ENABLEJOYSTICKS, 300, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLEJOYSTICKS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50028);
  v_button->setContextHelp(CONTEXTHELP_ENABLEJOYSTICKS);

  v_someText = new UIStatic(v_window, 300, v_window->getPosition().nHeight -43 -57,
			    GAMETEXT_NOJOYSTICKFOUND, v_window->getPosition().nWidth-300-5, 57);
  v_someText->setID("STATIC_JOYSTICK_FOUND");
  v_someText->setHAlign(UI_ALIGN_RIGHT);
  v_someText->setFont(drawlib->getFontSmall()); 

  return v_window;
}

UIWindow* StateOptions::makeWindowOptions_rooms(UIWindow* i_parent) {
  UIWindow *v_window, *v_wwwwindow;
  UIStatic* v_someText;
  UIEdit*   v_edit;
  UIButton* v_button;
  DrawLib*  drawlib = GameApp::instance()->getDrawLib();

  v_wwwwindow = new UIWindow(i_parent, 0, 26, GAMETEXT_WWWTAB, i_parent->getPosition().nWidth, i_parent->getPosition().nHeight-26);
  v_wwwwindow->setID("WWW_TAB");
  v_wwwwindow->showWindow(false);

  UITabView *v_wwwTabs = new UITabView(v_wwwwindow, 0, 0, "", v_wwwwindow->getPosition().nWidth, v_wwwwindow->getPosition().nHeight);
  v_wwwTabs->setHideDisabledTabs(true);
  v_wwwTabs->setID("TABS");
  v_wwwTabs->setFont(drawlib->getFontSmall());
  v_wwwTabs->setTabContextHelp(0, CONTEXTHELP_WWW_MAIN_OPTIONS);
  v_wwwTabs->setTabContextHelp(1, CONTEXTHELP_WWW_ROOMS_OPTIONS);
  v_wwwTabs->setTabContextHelp(2 + ROOMS_NB_MAX - 1, CONTEXTHELP_SERVER_OPTIONS);

  v_window = new UIWindow(v_wwwTabs, 20, 30, GAMETEXT_MAIN, v_wwwTabs->getPosition().nWidth-30,
			  v_wwwTabs->getPosition().nHeight);
  v_window->setID("MAIN_TAB");

  v_button = new UIButton(v_window, 5, 0, GAMETEXT_ENABLEWEBHIGHSCORES, (v_window->getPosition().nWidth-40), 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLEWEB");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_DOWNLOAD_BEST_TIMES);

  // password
  char buf[256];
  snprintf(buf, 256, GAMETEXT_ACCOUNT_PASSWORD, XMSession::instance()->profile().c_str());
  v_someText = new UIStatic(v_window, 35, 25, buf, v_window->getPosition().nWidth-35, 30);
  v_someText->setID("WWW_PASSWORD_STATIC");
  v_someText->setHAlign(UI_ALIGN_LEFT);
  v_someText->setFont(drawlib->getFontSmall()); 

  v_edit = new UIEdit(v_window, 35, 50, "", 150, 25);
  v_edit->setFont(drawlib->getFontSmall());
  v_edit->setID("WWW_PASSWORD");
  v_edit->hideText(true);
  v_edit->setContextHelp(CONTEXTHELP_WWW_PASSWORD);

  v_button = new UIButton(v_window, 5, 80, GAMETEXT_ENABLECHECKNEWLEVELSATSTARTUP,v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLECHECKNEWLEVELSATSTARTUP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_ENABLE_CHECK_NEW_LEVELS_AT_STARTUP);
  
  v_button = new UIButton(v_window, 5, 110, GAMETEXT_ENABLECHECKHIGHSCORESATSTARTUP, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLECHECKHIGHSCORESATSTARTUP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_ENABLE_CHECK_HIGHSCORES_AT_STARTUP);

  v_button = new UIButton(v_window, 5, 140, GAMETEXT_ALLOWWEBFORMS, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ALLOWWEBFORMS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_ALLOWWEBFORMS);

  v_button = new UIButton(v_window, 5, 170, GAMETEXT_ENABLEINGAMEWORLDRECORD, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("INGAMEWORLDRECORD");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_INGAME_WORLD_RECORD);

  v_button = new UIButton(v_window, 5, 200, GAMETEXT_USECRAPPYINFORMATION, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("USECRAPPYINFORMATION");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_USECRAPPYINFORMATION);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth-225, v_window->getPosition().nHeight -57 - 20 -20,
			  GAMETEXT_PROXYCONFIG, 207, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("PROXYCONFIG");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_PROXYCONFIG);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth-225-200, v_window->getPosition().nHeight -57 - 20 -20,
			  GAMETEXT_UPDATEHIGHSCORES, 207, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("UPDATEHIGHSCORES");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_UPDATEHIGHSCORES);


  for(unsigned int i=0; i<ROOMS_NB_MAX; i++) {
    v_window = makeRoomTab(v_wwwTabs, i);
  }

  // server config
  v_window = new UIWindow(v_wwwTabs, 20, 30, GAMETEXT_SERVER, v_wwwTabs->getPosition().nWidth-30,
			  v_wwwTabs->getPosition().nHeight);
  v_window->setID("SERVER_TAB");
  v_window->showWindow(false);

  v_button = new UIButton(v_window, 5, 0, GAMETEXT_STARTATSTARTUP, 400, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("STARTATSTARTUP_BUTTON");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_STARTSERVERATSTARTUP);

  // port
  v_someText = new UIStatic(v_window, 5, 30, GAMETEXT_PORT + std::string(":"),
			    200, 30);
  v_someText->setHAlign(UI_ALIGN_LEFT);
  v_someText->setFont(drawlib->getFontSmall()); 

  v_button = new UIButton(v_window, 20, 60, GAMETEXT_DEFAULT_PORT, 200, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setGroup(501);
  v_button->setID("DEFAULT_PORT");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_SERVERPORT);

  v_button = new UIButton(v_window, 20, 90, GAMETEXT_CUSTOM_PORT, 200, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setGroup(501);
  v_button->setID("CUSTOM_PORT");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_SERVERPORT);

  v_edit = new UIEdit(v_window, 20+35, 120, "", 100, 25);
  v_edit->setFont(drawlib->getFontSmall());
  v_edit->setID("PORT");
  v_edit->setContextHelp(CONTEXTHELP_SERVERPORT);

  // server status
  v_someText = new UIStatic(v_window, 0, v_window->getPosition().nHeight-30-20-20, "...",
			    v_window->getPosition().nWidth, 30);
  v_someText->setID("STATUS");
  v_someText->setHAlign(UI_ALIGN_CENTER);
  v_someText->setFont(drawlib->getFontSmall()); 

  // start/stop
  v_button = new UIButton(v_window, v_window->getPosition().nWidth/2- 207/2,
			  v_window->getPosition().nHeight -57 - 20 -20 - 30,
			  "", 207, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("STARTSTOP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_SERVERSTARTSTOP);

  return v_wwwwindow;
}

void StateOptions::updateServerStrings() {
  UIStatic* v_someText;
  UIButton* v_button;

  v_someText = reinterpret_cast<UIStatic *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:SERVER_TAB:STATUS"));
  v_someText->setCaption(NetServer::instance()->isStarted() ?
			GAMETEXT_SERVERSTATUSON : GAMETEXT_SERVERSTATUSOFF);

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:SERVER_TAB:STARTSTOP"));
  v_button->setCaption(NetServer::instance()->isStarted() ?
		       GAMETEXT_SERVERSTOP : GAMETEXT_SERVERSTART);
}

UIWindow* StateOptions::makeWindowOptions_ghosts(UIWindow* i_parent) {
  UIWindow*  v_window;
  UIButton*  v_button;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  v_window = new UIWindow(i_parent, 20, 40, GAMETEXT_GHOSTTAB, i_parent->getPosition().nWidth-40, i_parent->getPosition().nHeight);
  v_window->setID("GHOSTS_TAB");
  v_window->showWindow(false);

  v_button = new UIButton(v_window, 5, 5, GAMETEXT_ENABLEGHOST, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLE_GHOSTS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GHOST_MODE);

  v_button = new UIButton(v_window, 5+20, 34, GAMETEXT_GHOST_STRATEGY_MYBEST, v_window->getPosition().nWidth-40,28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("GHOST_STRATEGY_MYBEST");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GHOST_STRATEGY_MYBEST);

  v_button = new UIButton(v_window, 5+20, 63, GAMETEXT_GHOST_STRATEGY_THEBEST, v_window->getPosition().nWidth-40,28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("GHOST_STRATEGY_THEBEST");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GHOST_STRATEGY_THEBEST);

  v_button = new UIButton(v_window, 5+20, 92, GAMETEXT_GHOST_STRATEGY_BESTOFREFROOM, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("GHOST_STRATEGY_BESTOFREFROOM");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GHOST_STRATEGY_BESTOFREFROOM);

  v_button = new UIButton(v_window, 5+20, 121, GAMETEXT_GHOST_STRATEGY_BESTOFOTHERROOMS, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("GHOST_STRATEGY_BESTOFOTHERROOMS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GHOST_STRATEGY_BESTOFOTHERROOMS);

  v_button = new UIButton(v_window, 5, 150, GAMETEXT_DISPLAYGHOSTTIMEDIFF, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("DISPLAY_GHOST_TIMEDIFFERENCE");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_DISPLAY_GHOST_TIMEDIFF);

  v_button = new UIButton(v_window, 5, 179, GAMETEXT_HIDEGHOSTS, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("HIDEGHOSTS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_HIDEGHOSTS);

  v_button = new UIButton(v_window, 5, 208, GAMETEXT_DISPLAYGHOSTINFO, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("DISPLAY_GHOSTS_INFOS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_DISPLAY_GHOST_INFO);

  v_button = new UIButton(v_window, 5, 237, GAMETEXT_DISPLAYBIKERARROW, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("DISPLAY_BIKERS_ARROWS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_DISPLAY_BIKER_ARROW);

  v_button = new UIButton(v_window, 5, 266, GAMETEXT_MOTIONBLURGHOST, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("MOTION_BLUR_GHOST");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_MOTIONBLURGHOST);

  return v_window;
}

UIWindow* StateOptions::makeWindowOptions_language(UIWindow* i_parent) {
  UIWindow*  v_window;
  UIList* v_list;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();
  UIListEntry *pEntry;
  unsigned int n = 0;

  v_window = new UIWindow(i_parent, 20, 40, GAMETEXT_LANGUAGETAB, i_parent->getPosition().nWidth-40, i_parent->getPosition().nHeight);
  v_window->setID("LANGUAGE_TAB");
  v_window->showWindow(false);

  /* list */
  v_list = new UIList(v_window, 0, 0, "", v_window->getPosition().nWidth, v_window->getPosition().nHeight-40-20);
  v_list->setID("LANGUAGE_LIST");
  v_list->setFont(drawlib->getFontSmall());
  v_list->addColumn(GAMETEXT_LANGUAGE_NAME, v_list->getPosition().nWidth - 250, CONTEXTHELP_LANGUAGE_NAME);
  v_list->addColumn(GAMETEXT_LANGUAGE_CODE, 250,CONTEXTHELP_LANGUAGE_CODE);

  pEntry = v_list->addEntry(GAMETEXT_AUTOMATIC, NULL);
  pEntry->Text.push_back("");
  if(XMSession::instance()->language() == "") v_list->setRealSelected(n);
  n++;

  unsigned int i=0;
  while(LANGUAGES[i][LANGUAGE_NAME] != NULL) {
    pEntry = v_list->addEntry(LANGUAGES[i][LANGUAGE_NAME], NULL);
    pEntry->Text.push_back(LANGUAGES[i][LANGUAGE_CODE]);
    if(XMSession::instance()->language() == LANGUAGES[i][LANGUAGE_CODE])
      v_list->setRealSelected(i+1);

    i++;
  }

  return v_window;
}

UIWindow* StateOptions::makeWindowOptions_db(UIWindow* i_parent) {
  UIWindow*  v_window;
  UIStatic*  v_someText;
  UIButton*  v_button;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  v_window = new UIWindow(i_parent, 20, 40, GAMETEXT_DB, i_parent->getPosition().nWidth-40, i_parent->getPosition().nHeight);
  v_window->setID("DB_TAB");
  v_window->showWindow(false);

  /* explanation */
  v_someText = new UIStatic(v_window, 0, 10, splitText(GAMETEXT_DBSYNCHRONIZE_EXPLANATION, 59),
			    v_window->getPosition().nWidth, v_window->getPosition().nHeight/2 - 57/2 - 10);
  v_someText->setHAlign(UI_ALIGN_CENTER);
  v_someText->setVAlign(UI_ALIGN_BOTTOM);
  v_someText->setFont(drawlib->getFontSmall()); 

  /* synchronisation */
  v_button = new UIButton(v_window, (v_window->getPosition().nWidth-175)/2, (v_window->getPosition().nHeight-57)/2,
			  GAMETEXT_DBSYNCHRONIZE, 170, 57);
  v_button->setID("SYNCHRONIZE_BUTTON");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_SMALL);      
  v_button->setContextHelp(CONTEXTHELP_DBSYNCHRONIZE);

  /* warning */
  v_someText = new UIStatic(v_window, 0,  v_window->getPosition().nHeight/2 + 57/2 + 10 , splitText(GAMETEXT_DBSYNCHRONIZE_WARNING, 59),
			    v_window->getPosition().nWidth, v_window->getPosition().nHeight/2);
  v_someText->setHAlign(UI_ALIGN_CENTER);
  v_someText->setVAlign(UI_ALIGN_TOP);
  v_someText->setFont(drawlib->getFontSmall()); 
  v_someText->setNormalColor(MAKE_COLOR(255,0,0,255));

  /* synchronize each time xmoto is closed ? */
  //v_button = new UIButton(v_window, 20, (v_window->getPosition().nHeight/2) + 57,
  //			  GAMETEXT_DBSYNCHRONIZE_ONQUIT, v_window->getPosition().nWidth-20, 28);
  //v_button->setType(UI_BUTTON_TYPE_CHECK);
  //v_button->setFont(drawlib->getFontSmall());
  //v_button->setID("SYNCHRONIZE_ONQUIT_CKB");
  //v_button->setContextHelp(CONTEXTHELP_DBSYNCHRONIZE_ONQUIT); 

  return v_window;
}

UIWindow* StateOptions::makeRoomTab(UIWindow* i_parent, unsigned int i_number) {
  UIWindow* v_window;
  UIList*   v_list;
  UIButton*  v_button;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  std::string v_roomTitle;
  std::ostringstream v_strRoom;
  v_strRoom << i_number;

  if(i_number == 0) {
    v_roomTitle = GAMETEXT_WWWROOMSTAB_REFERENCE;
  } else {
    char v_ctmp[64];
    snprintf(v_ctmp, 64, GAMETEXT_WWWROOMSTAB_OTHER, i_number + 1);
    v_roomTitle = v_ctmp;
  }

  v_window = new UIWindow(i_parent, 4, 25, v_roomTitle,
			  i_parent->getPosition().nWidth-20, i_parent->getPosition().nHeight - 15);
  v_window->setID("ROOMS_TAB_" + v_strRoom.str());
  v_window->showWindow(false);

  if(i_number != 0) {
    v_button = new UIButton(v_window, 0, 0, "", 40, 28);
    v_button->setType(UI_BUTTON_TYPE_CHECK);
    v_button->setID("ROOM_ENABLED");
    v_button->setFont(drawlib->getFontSmall());
    v_button->setContextHelp(CONTEXTHELP_ROOM_ENABLE);
  }

  v_list = new UIList(v_window, 20, 25, "", v_window->getPosition().nWidth-215 - 20 - 20, v_window->getPosition().nHeight - 20 - 50);
  v_list->setID("ROOMS_LIST");
  v_list->setFont(drawlib->getFontSmall());
  v_list->addColumn(GAMETEXT_ROOM, v_list->getPosition().nWidth);
  v_list->setContextHelp(CONTEXTHELP_WWW_ROOMS_LIST);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth - 215, 25 + 57*0, GAMETEXT_UPDATEROOMSSLIST, 215, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("UPDATE_ROOMS_LIST");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_UPDATEROOMSLIST);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth - 215, 25 + 57*1, GAMETEXT_UPLOAD_ALL_HIGHSCORES, 215, 57);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("UPLOADHIGHSCOREALL_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_UPLOAD_HIGHSCORE_ALL);	

  return v_window;
}

void StateOptions::updateOptions() {
  UIButton* v_button;
  UIEdit*   v_edit;

  // options/general
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:SHOWMINIMAP"));
  v_button->setChecked(XMSession::instance()->showMinimap());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:SHOWENGINECOUNTER"));
  v_button->setChecked(XMSession::instance()->showEngineCounter());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:INITZOOM"));
  v_button->setChecked(XMSession::instance()->enableInitZoom());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:DEATHANIM"));
  v_button->setChecked(XMSession::instance()->enableDeadAnimation());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:CAMERAACTIVEZOOM"));
  v_button->setChecked(XMSession::instance()->enableActiveZoom());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:ENABLECONTEXTHELP"));
  v_button->setChecked(XMSession::instance()->enableContextHelp());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:AUTOSAVEREPLAYS"));
  v_button->setChecked(XMSession::instance()->autosaveHighscoreReplays());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:MAIN_TAB:BEATINGMODE"));
  v_button->setChecked(XMSession::instance()->beatingMode());

  // video
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:16BPP"));
  v_button->setChecked(XMSession::instance()->bpp() == 16);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:32BPP"));
  v_button->setChecked(XMSession::instance()->bpp() == 32); 

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:WINDOWED"));
  v_button->setChecked(XMSession::instance()->windowed());

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:MENULOW"));
  v_button->setChecked(XMSession::instance()->menuGraphics() == GFX_LOW);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:MENUMEDIUM"));
  v_button->setChecked(XMSession::instance()->menuGraphics() == GFX_MEDIUM);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:MENUHIGH"));
  v_button->setChecked(XMSession::instance()->menuGraphics() == GFX_HIGH);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:GAMELOW"));
  v_button->setChecked(XMSession::instance()->gameGraphics() == GFX_LOW);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:GAMEMEDIUM"));
  v_button->setChecked(XMSession::instance()->gameGraphics() == GFX_MEDIUM);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:GAMEHIGH"));
  v_button->setChecked(XMSession::instance()->gameGraphics() == GFX_HIGH);

  // audio
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:ENABLE_AUDIO"));
  v_button->setChecked(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:RATE11KHZ"));
  v_button->setChecked(XMSession::instance()->audioSampleRate() == 11025);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:RATE22KHZ"));
  v_button->setChecked(XMSession::instance()->audioSampleRate() == 22050);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:RATE44KHZ"));
  v_button->setChecked(XMSession::instance()->audioSampleRate() == 44100);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:8BITS"));
  v_button->setChecked(XMSession::instance()->audioSampleBits() == 8);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:16BITS"));
  v_button->setChecked(XMSession::instance()->audioSampleBits() == 16);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:MONO"));
  v_button->setChecked(XMSession::instance()->audioChannels() == 1);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:STEREO"));
  v_button->setChecked(XMSession::instance()->audioChannels() == 2);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:ENABLE_ENGINE_SOUND"));
  v_button->setChecked(XMSession::instance()->enableEngineSound());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:ENABLE_MENU_MUSIC"));
  v_button->setChecked(XMSession::instance()->enableMenuMusic());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:ENABLE_GAME_MUSIC"));
  v_button->setChecked(XMSession::instance()->enableGameMusic());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:ENABLE_MUSIC_ON_ALL_LEVELS"));
  v_button->setChecked(XMSession::instance()->musicOnAllLevels());

  // controls
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:CONTROLS_TAB:ENABLEJOYSTICKS"));
  v_button->setChecked(XMSession::instance()->enableJoysticks());

  // www
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLEWEB"));
  v_button->setChecked(XMSession::instance()->www());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKNEWLEVELSATSTARTUP"));
  v_button->setChecked(XMSession::instance()->checkNewLevelsAtStartup());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKHIGHSCORESATSTARTUP"));
  v_button->setChecked(XMSession::instance()->checkNewHighscoresAtStartup());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:INGAMEWORLDRECORD"));
  v_button->setChecked(XMSession::instance()->showHighscoreInGame());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:USECRAPPYINFORMATION"));
  v_button->setChecked(XMSession::instance()->useCrappyPack());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:ALLOWWEBFORMS"));
  v_button->setChecked(XMSession::instance()->webForms());

  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:WWW_PASSWORD"));
  v_edit->setCaption(XMSession::instance()->wwwPassword());

  for(unsigned int i=0; i<ROOMS_NB_MAX; i++) {
      bool v_enabled = true;
      std::ostringstream v_strRoom;
      v_strRoom << i;

      if(i != 0) {
	v_enabled = XMSession::instance()->nbRoomsEnabled() > i;
	v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
								+ ":ROOM_ENABLED"));
	v_button->setChecked(v_enabled);
      }
   }
  // server
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:SERVER_TAB:STARTATSTARTUP_BUTTON"));
  v_button->setChecked(XMSession::instance()->serverStartAtStartup());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:SERVER_TAB:DEFAULT_PORT"));
  v_button->setChecked(XMSession::instance()->serverPort() == DEFAULT_SERVERPORT);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:SERVER_TAB:CUSTOM_PORT"));
  v_button->setChecked(XMSession::instance()->serverPort() != DEFAULT_SERVERPORT);
  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:SERVER_TAB:PORT"));
  v_edit->enableWindow(XMSession::instance()->serverPort() != DEFAULT_SERVERPORT);
  std::ostringstream v_strPort;
  v_strPort << XMSession::instance()->serverPort();
  v_edit->setCaption(v_strPort.str());

  // ghosts
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:ENABLE_GHOSTS"));
  v_button->setChecked(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:GHOST_STRATEGY_MYBEST"));
  v_button->setChecked(XMSession::instance()->ghostStrategy_MYBEST());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:GHOST_STRATEGY_THEBEST"));
  v_button->setChecked(XMSession::instance()->ghostStrategy_THEBEST());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:GHOST_STRATEGY_BESTOFREFROOM"));
  v_button->setChecked(XMSession::instance()->ghostStrategy_BESTOFREFROOM());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:GHOST_STRATEGY_BESTOFOTHERROOMS"));
  v_button->setChecked(XMSession::instance()->ghostStrategy_BESTOFOTHERROOMS());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:DISPLAY_GHOST_TIMEDIFFERENCE"));
  v_button->setChecked(XMSession::instance()->showGhostTimeDifference());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:DISPLAY_GHOSTS_INFOS"));
  v_button->setChecked(XMSession::instance()->showGhostsInfos());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:DISPLAY_BIKERS_ARROWS"));
  v_button->setChecked(XMSession::instance()->showBikersArrows());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:HIDEGHOSTS"));
  v_button->setChecked(XMSession::instance()->hideGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:MOTION_BLUR_GHOST"));
  v_button->setChecked(XMSession::instance()->ghostMotionBlur());

  // db
  //v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:DB_TAB:SYNCHRONIZE_ONQUIT_CKB"));
  //v_button->setChecked(XMSession::instance()->dbsynchronizeOnQuit());

  // update rights on the options
  updateAudioOptions();
  updateWWWOptions();
  updateDbOptions();
  updateGhostsOptions();

  // update lists in options
  updateThemesList();
  updateResolutionsList();
  updateControlsList();
  updateRoomsList();
}


void StateOptions::updateAudioOptions() {
  UIButton* v_button;

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:ENABLE_AUDIO"));
  v_button->setChecked(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:RATE11KHZ"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:RATE22KHZ"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:RATE44KHZ"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:8BITS"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:16BITS"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:MONO"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:STEREO"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:ENABLE_ENGINE_SOUND"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:ENABLE_MENU_MUSIC"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:ENABLE_GAME_MUSIC"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:AUDIO_TAB:ENABLE_MUSIC_ON_ALL_LEVELS"));
  v_button->enableWindow(XMSession::instance()->enableAudio() && XMSession::instance()->enableGameMusic());
}

void StateOptions::updateDbOptions() {
  UIButton* v_button;

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:DB_TAB:SYNCHRONIZE_BUTTON"));
  v_button->enableWindow(XMSession::instance()->www());

  //v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:DB_TAB:SYNCHRONIZE_ONQUIT_CKB"));
  //v_button->enableWindow(XMSession::instance()->www());
}

void StateOptions::updateWWWOptions() {
  UIButton* v_button;
  UIList*   v_list;
  UIStatic* v_someText;
  UIEdit*   v_edit;
  UIWindow *v_window;
  bool v_enabled;

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLEWEB"));
  v_button->setChecked(XMSession::instance()->www());

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKNEWLEVELSATSTARTUP"));
  v_button->enableWindow(XMSession::instance()->www());

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKHIGHSCORESATSTARTUP"));
  v_button->enableWindow(XMSession::instance()->www());

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:ALLOWWEBFORMS"));
  v_button->enableWindow(XMSession::instance()->www());

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:PROXYCONFIG"));
  v_button->enableWindow(XMSession::instance()->www());

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:UPDATEHIGHSCORES"));
  v_button->enableWindow(XMSession::instance()->www());

  v_someText = reinterpret_cast<UIStatic *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:WWW_PASSWORD_STATIC"));
  v_someText->enableWindow(XMSession::instance()->www());
  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:WWW_PASSWORD"));
  v_edit->enableWindow(XMSession::instance()->www());

  for(unsigned int i=0; i<ROOMS_NB_MAX; i++) {
      std::ostringstream v_strRoom;
      v_strRoom << i;
      v_enabled = XMSession::instance()->nbRoomsEnabled() > i;

      if(i != 0) {
	v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
								+ ":ROOM_ENABLED"));
	v_window = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()));

	if(XMSession::instance()->nbRoomsEnabled() >= i) {
	  v_window->enableWindow(true);
	  v_button->enableWindow(true);
	  v_button->setChecked(XMSession::instance()->nbRoomsEnabled() > i);
	} else {
	  v_window->enableWindow(false);
	  v_button->enableWindow(false);
	  v_button->setChecked(false);
	}
      }

      v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							      + ":UPDATE_ROOMS_LIST"));
      v_button->enableWindow(XMSession::instance()->www() && v_enabled);

      v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							      + ":UPLOADHIGHSCOREALL_BUTTON"));
      v_button->enableWindow(XMSession::instance()->www() && v_enabled);

      v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							  + ":ROOMS_LIST"));
      v_list->enableWindow(v_enabled);

  }
}

void StateOptions::updateGhostsOptions() {
  UIButton* v_button;

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:GHOST_STRATEGY_MYBEST"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:GHOST_STRATEGY_THEBEST"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:GHOST_STRATEGY_BESTOFREFROOM"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:GHOST_STRATEGY_BESTOFOTHERROOMS"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:DISPLAY_GHOST_TIMEDIFFERENCE"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:DISPLAY_GHOSTS_INFOS"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:DISPLAY_BIKERS_ARROWS"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:HIDEGHOSTS"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:TABS:GHOSTS_TAB:MOTION_BLUR_GHOST"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  
  if(GameApp::instance()->getDrawLib()->useShaders() == false) {
    v_button->enableWindow(false);
    v_button->setChecked(false);
  }
}

void StateOptions::updateThemesList() {
  UIList* v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:THEME_TAB:LIST"));
  createThemesList(v_list);

  std::string v_themeName = XMSession::instance()->theme();
  int nTheme = 0;
  for(unsigned int i=0; i<v_list->getEntries().size(); i++) {
    if(v_list->getEntries()[i]->Text[0] == v_themeName) {
      nTheme = i;
      break;
    }
  }
  v_list->setRealSelected(nTheme); 
}

void StateOptions::updateResolutionsList() {
  UIList* v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:VIDEO_TAB:RESOLUTIONS_LIST"));
  char cBuf[256];
  snprintf(cBuf, 256, "%d X %d", XMSession::instance()->resolutionWidth(), XMSession::instance()->resolutionHeight());
  int nMode = 0;

  v_list->clear();
  std::vector<std::string>* modes = System::getDisplayModes(XMSession::instance()->windowed());
  for(unsigned int i=0; i < modes->size(); i++) {
    v_list->addEntry((*modes)[i].c_str());
    if(std::string((*modes)[i]) == std::string(cBuf)) {
      nMode = i;
    }
  }
  delete modes;
  v_list->setRealSelected(nMode);
}

void StateOptions::updateControlsList() {
  UIList *pList = (UIList *)m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:CONTROLS_TAB:KEY_ACTION_LIST");
  pList->clear();
    
  UIListEntry *p;

  for(unsigned int i=0; i<INPUT_NB_PLAYERS; i++) {
    std::ostringstream v_n;

    if(i != 0) {
      v_n << " " << (i+1);
    }    

    p = pList->addEntry(GAMETEXT_DRIVE + v_n.str());
    p->Text.push_back(InputHandler::instance()->getDRIVE(i).toFancyString());
    p->Text.push_back(InputHandler::instance()->getDRIVE(i).toString());

    p = pList->addEntry(GAMETEXT_BRAKE + v_n.str());
    p->Text.push_back(InputHandler::instance()->getBRAKE(i).toFancyString());
    p->Text.push_back(InputHandler::instance()->getBRAKE(i).toString());

    p = pList->addEntry(GAMETEXT_FLIPLEFT + v_n.str());
    p->Text.push_back(InputHandler::instance()->getFLIPLEFT(i).toFancyString());
    p->Text.push_back(InputHandler::instance()->getFLIPLEFT(i).toString());

    p = pList->addEntry(GAMETEXT_FLIPRIGHT + v_n.str());
    p->Text.push_back(InputHandler::instance()->getFLIPRIGHT(i).toFancyString());
    p->Text.push_back(InputHandler::instance()->getFLIPRIGHT(i).toString());

    p = pList->addEntry(GAMETEXT_CHANGEDIR + v_n.str());
    p->Text.push_back(InputHandler::instance()->getCHANGEDIR(i).toFancyString());
    p->Text.push_back(InputHandler::instance()->getCHANGEDIR(i).toString());

    for(unsigned int k=0; k<MAX_SCRIPT_KEY_HOOKS; k++) {
      std::ostringstream v_k;
      v_k << (k+1);
      
      p = pList->addEntry(GAMETEXT_SCRIPTACTION + v_n.str() + " " + v_k.str());
      p->Text.push_back(InputHandler::instance()->getSCRIPTACTION(i, k).toFancyString());
      p->Text.push_back(InputHandler::instance()->getSCRIPTACTION(i, k).toString());
    } 
  }

  p = pList->addEntry(GAMETEXT_SWITCHFAVORITE);
  p->Text.push_back(InputHandler::instance()->getSwitchFavorite().toFancyString());
  p->Text.push_back(InputHandler::instance()->getSwitchFavorite().toString());

  p = pList->addEntry(GAMETEXT_SWITCHBLACKLIST);
  p->Text.push_back(InputHandler::instance()->getSwitchBlacklist().toFancyString());
  p->Text.push_back(InputHandler::instance()->getSwitchBlacklist().toString());

  p = pList->addEntry(GAMETEXT_SWITCHUGLYMODE);
  p->Text.push_back(InputHandler::instance()->getSwitchUglyMode().toFancyString());
  p->Text.push_back(InputHandler::instance()->getSwitchUglyMode().toString());

}

void StateOptions::createThemesList(UIList *pList) {
  char **v_result;
  unsigned int nrow;
  UIListEntry *pEntry;
  std::string v_id_theme;
  std::string v_ck1, v_ck2;
  
  /* get selected item */
  std::string v_selected_themeName = "";
  if(pList->getSelected() >= 0 && pList->getSelected() < pList->getEntries().size()) {
    UIListEntry *pEntry = pList->getEntries()[pList->getSelected()];
    v_selected_themeName = pEntry->Text[0];
  }
  
  /* recreate the list */
  pList->clear();
  
  v_result = xmDatabase::instance("main")->readDB("SELECT a.id_theme, a.checkSum, b.checkSum "
				      "FROM themes AS a LEFT OUTER JOIN webthemes AS b "
				      "ON a.id_theme=b.id_theme ORDER BY a.id_theme;",
			  nrow);
  for(unsigned int i=0; i<nrow; i++) {
    v_id_theme = xmDatabase::instance("main")->getResult(v_result, 3, i, 0);
    v_ck1      = xmDatabase::instance("main")->getResult(v_result, 3, i, 1);
    if(xmDatabase::instance("main")->getResult(v_result, 3, i, 2) != NULL) {
      v_ck2      = xmDatabase::instance("main")->getResult(v_result, 3, i, 2);
    }
    
    pEntry = pList->addEntry(v_id_theme.c_str(),
			     NULL);
    if(v_ck1 == v_ck2 || v_ck2 == "") {
      pEntry->Text.push_back(GAMETEXT_THEMEHOSTED);
    } else {
      pEntry->Text.push_back(GAMETEXT_THEMEREQUIREUPDATE);
    }
  }
  xmDatabase::instance("main")->read_DB_free(v_result);
  
  v_result = xmDatabase::instance("main")->readDB("SELECT a.id_theme FROM webthemes AS a LEFT OUTER JOIN themes AS b "
				      "ON a.id_theme=b.id_theme WHERE b.id_theme IS NULL;",
				      nrow);
  for(unsigned int i=0; i<nrow; i++) {
    v_id_theme = xmDatabase::instance("main")->getResult(v_result, 1, i, 0);
    pEntry = pList->addEntry(v_id_theme.c_str(), NULL);
    pEntry->Text.push_back(GAMETEXT_THEMENOTHOSTED);
  }
  xmDatabase::instance("main")->read_DB_free(v_result);
  
  /* reselect the previous theme */
  if(v_selected_themeName != "") {
    int nTheme = 0;
    for(unsigned int i=0; i<pList->getEntries().size(); i++) {
      if(pList->getEntries()[i]->Text[0] == v_selected_themeName) {
	nTheme = i;
	break;
      }
    }
    pList->setRealSelected(nTheme);
  }  
}

void StateOptions::updateRoomsList() {
  UIList* v_list;
  unsigned int j;
  bool v_found;

  for(unsigned int i=0; i<ROOMS_NB_MAX; i++) {
      std::ostringstream v_strRoom;
      v_strRoom << i;

      v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							  + ":ROOMS_LIST"));
      createRoomsList(v_list);

      std::string v_id_room = XMSession::instance()->idRoom(i);
      if(v_id_room == "") { v_id_room = DEFAULT_WEBROOM_ID; }
      j = 0;
      v_found = false;
      while(j<v_list->getEntries().size() && v_found == false) {
	if((*(std::string*)v_list->getEntries()[j]->pvUser) == v_id_room) {
	  v_list->setRealSelected(j);
	  v_found = true;
	}
	j++;
      }
  }
}

void StateOptions::createRoomsList(UIList *pList) {
  UIListEntry *pEntry;
  std::string v_selected_roomName = "";
  char **v_result;
  unsigned int nrow;
  std::string v_roomName, v_roomId;

  /* get selected item */
  if(pList->getSelected() >= 0 && pList->getSelected() < pList->getEntries().size()) {
    UIListEntry *pEntry = pList->getEntries()[pList->getSelected()];
    v_selected_roomName = pEntry->Text[0];
  }

  /* recreate the list */
  for(unsigned int i=0; i<pList->getEntries().size(); i++) {
    delete reinterpret_cast<std::string *>(pList->getEntries()[i]->pvUser);
  }
  pList->clear();

  // WR room
  v_result = xmDatabase::instance("main")->readDB("SELECT id_room, name FROM webrooms WHERE id_room = 1;", nrow);
  for(unsigned int i=0; i<nrow; i++) {
    v_roomId   = xmDatabase::instance("main")->getResult(v_result, 2, i, 0);
    v_roomName = xmDatabase::instance("main")->getResult(v_result, 2, i, 1);
    pEntry = pList->addEntry(v_roomName, reinterpret_cast<void *>(new std::string(v_roomId)));    
  }
  xmDatabase::instance("main")->read_DB_free(v_result);

  v_result = xmDatabase::instance("main")->readDB("SELECT id_room, name FROM webrooms ORDER BY UPPER(name);", nrow);
  for(unsigned int i=0; i<nrow; i++) {
    v_roomId   = xmDatabase::instance("main")->getResult(v_result, 2, i, 0);
    v_roomName = xmDatabase::instance("main")->getResult(v_result, 2, i, 1);
    pEntry = pList->addEntry(v_roomName, reinterpret_cast<void *>(new std::string(v_roomId)));    
  }
  xmDatabase::instance("main")->read_DB_free(v_result);

  /* reselect the previous room */
  if(v_selected_roomName != "") {
    int nRoom = 0;
    for(unsigned int i=0; i<pList->getEntries().size(); i++) {
      if(pList->getEntries()[i]->Text[0] == v_selected_roomName) {
	nRoom = i;
	break;
      }
    }
    pList->setRealSelected(nRoom);
  }
}

void StateOptions::updateJoysticksStrings() {
  UIStatic*  v_someText; 
  v_someText = reinterpret_cast<UIStatic *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:CONTROLS_TAB:STATIC_JOYSTICK_FOUND"));
  unsigned int v_nbJoy = InputHandler::instance()->getJoysticksNames().size();

  if(v_nbJoy == 0) {
    v_someText->setCaption(GAMETEXT_NOJOYSTICKFOUND);    
  } else if(v_nbJoy == 1) {
    char buf[256];
    snprintf(buf, 256, GAMETEXT_JOYSTICKSFOUND(v_nbJoy), v_nbJoy);
    v_someText->setCaption(buf + std::string(" : ") + InputHandler::instance()->getJoysticksNames()[0]);
  } else {
    char buf[256];
    snprintf(buf, 256, GAMETEXT_JOYSTICKSFOUND(v_nbJoy), v_nbJoy);
    v_someText->setCaption(buf);
  }
}

void StateOptions::updateProfileStrings() {
  UIStatic* v_strTxt;

  // www password static string
  v_strTxt = reinterpret_cast<UIStatic *>(m_GUI->getChild("MAIN:TABS:WWW_TAB:TABS:MAIN_TAB:WWW_PASSWORD_STATIC"));
  char buf[256];
  snprintf(buf, 256, GAMETEXT_ACCOUNT_PASSWORD, XMSession::instance()->profile().c_str());
  v_strTxt->setCaption(buf);
}

void StateOptions::executeOneCommand(std::string cmd, std::string args) {
	char v_tmp[256];

  LogDebug("cmd [%s [%s]] executed by state [%s].",
	   cmd.c_str(), args.c_str(), getName().c_str());

  if(cmd == "REQUESTKEY") {
    UIList* v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:TABS:GENERAL_TAB:TABS:CONTROLS_TAB:KEY_ACTION_LIST"));
    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {    
      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];

      std::string key = CmdArgumentParser::instance()->getString(args);

      // is key used
      for(unsigned int i=0;i<v_list->getEntries().size();i++) {
				if(v_list->getSelected() != i) {
					if(v_list->getEntries()[i]->Text[2] == key) {
						// switch keys
						v_list->getEntries()[i]->Text[1] = pEntry->Text[1];
						v_list->getEntries()[i]->Text[2] = pEntry->Text[2];
						setInputKey(v_list->getEntries()[i]->Text[0], v_list->getEntries()[i]->Text[2]);
						
						snprintf(v_tmp, 256, GAMETEXT_SWITCHKEY, v_list->getEntries()[i]->Text[0].c_str(), v_list->getEntries()[i]->Text[1].c_str());
						SysMessage::instance()->displayInformation(v_tmp);
					}
				}
      }
      pEntry->Text[1] = XMKey(key).toFancyString();
      pEntry->Text[2] = key;
      setInputKey(pEntry->Text[0], key);
    }
  }

  else if(cmd == "UPDATEPROFILE") {
    updateProfileStrings();
    updateOptions();
  }

  else if(cmd == "THEMES_UPDATED") {
    updateThemesList();
  }

  else if(cmd == "ROOMS_UPDATED") {
    updateRoomsList();      
  }

  else if(cmd == "CHANGE_WWW_ACCESS" || cmd == "CONFIGURE_WWW_ACCESS") {
    updateWWWOptions();
    updateDbOptions();
  }

  else if(cmd == "ENABLEAUDIO_CHANGED") {
    updateAudioOptions();
  }

  else if(cmd == "SERVER_STATUS_CHANGED") {
    updateServerStrings();
  }

}

void StateOptions::sendFromMessageBox(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  if(i_id == "RESETSTODEFAULTS") {
    if(i_button == UI_MSGBOX_YES) {
      InputHandler::instance()->setDefaultConfig();
      XMSession::instance()->setToDefault();
      updateOptions();      
    }
  }

  else {
    StateMenu::sendFromMessageBox(i_id, i_button, i_input);
  }
}

void StateOptions::setInputKey(const std::string& i_strKey, const std::string& i_key) {

  for(int i=0; i<INPUT_NB_PLAYERS; i++) {
    std::ostringstream v_n;

    if(i != 0) {
      v_n << " " << (i+1);
    }  

    if(i_strKey == GAMETEXT_DRIVE + v_n.str()) {
      InputHandler::instance()->setDRIVE(i, XMKey(i_key));
    }
    
    if(i_strKey == GAMETEXT_BRAKE + v_n.str()) {
      InputHandler::instance()->setBRAKE(i, XMKey(i_key));
    }
    
    if(i_strKey == GAMETEXT_FLIPLEFT + v_n.str()) {
      InputHandler::instance()->setFLIPLEFT(i, XMKey(i_key));
    }
    
    if(i_strKey == GAMETEXT_FLIPRIGHT + v_n.str()) {
      InputHandler::instance()->setFLIPRIGHT(i, XMKey(i_key));
    }
    
    if(i_strKey == GAMETEXT_CHANGEDIR + v_n.str()) {
      InputHandler::instance()->setCHANGEDIR(i, XMKey(i_key));
    }

    for(unsigned int k=0; k<MAX_SCRIPT_KEY_HOOKS; k++) {
      std::ostringstream v_k;
      v_k << (k+1);
      if(i_strKey == GAMETEXT_SCRIPTACTION + v_n.str() + " " + v_k.str()) {
	InputHandler::instance()->setSCRIPTACTION(i, k, XMKey(i_key));
      }
    }
  }

  if(i_strKey == GAMETEXT_SWITCHFAVORITE) {
    InputHandler::instance()->setSwitchFavorite(XMKey(i_key));
  }

  if(i_strKey == GAMETEXT_SWITCHBLACKLIST) {
    InputHandler::instance()->setSwitchBlacklist(XMKey(i_key));
  }

  if(i_strKey == GAMETEXT_SWITCHUGLYMODE) {
    InputHandler::instance()->setSwitchUglyMode(XMKey(i_key));
  }
}
