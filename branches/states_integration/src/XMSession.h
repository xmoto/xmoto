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

#ifndef __XMCONF_H__
#define __XMCONF_H__

#include <string>

#define THEME_DEFAULT_THEMENAME "Classic"

/*
  XMSession   : current session options
  UserConfig  : config file options
  XMArguments : command line options

  first, XMSession constructor init session values,
  then, UserConfig are loaded, and finally, XMArguments overwrite session's values if they are defined
*/

class XMArguments;
class UserConfig;

enum GraphicsLevel {
  GFX_LOW,
  GFX_MEDIUM,
  GFX_HIGH
};

class XMSession {
  public:
  XMSession();
  void load(const XMArguments* i_xmargs);
  void load(UserConfig* m_Config);
  void save(UserConfig* m_Config);
  bool isVerbose() const;

  void setUseGraphics(bool i_value);
  bool useGraphics() const;
  int resolutionWidth() const;
  int resolutionHeight() const;
  int bpp() const;
  bool windowed() const;
  void setResolutionWidth(int i_value);
  void setResolutionHeight(int i_value);
  void setBpp(int i_value);
  void setWindowed(bool i_value);
  bool glExts() const;
  std::string drawlib() const;
  bool www() const;
  void setWWW(bool i_value);
  bool benchmark() const;
  bool debug() const;
  bool sqlTrace() const;
  std::string profile() const;
  void setProfile(const std::string& i_profile);
  bool gDebug() const;
  std::string gDebugFile() const;
  bool timedemo() const;
  bool fps() const;
  void setFps(bool i_value);
  bool ugly() const;
  void setUgly(bool i_value);
  bool uglyOver() const;
  void setUglyOver(bool i_value);
  bool testTheme() const;
  void setTestTheme(bool i_value);
  bool ghostStrategy_MYBEST() const;
  void setGhostStrategy_MYBEST(bool i_value);
  bool ghostStrategy_THEBEST() const;
  void setGhostStrategy_THEBEST(bool i_value);
  bool ghostStrategy_BESTOFROOM() const;
  void setGhostStrategy_BESTOFROOM(bool i_value);
  bool autosaveHighscoreReplays() const;
  void setAutosaveHighscoreReplays(bool i_value);
  void setEnableGhosts(bool i_value);
  bool enableGhosts() const;
  void setEnableEngineSound(bool i_value);
  bool enableEngineSound() const;
  void setShowEngineCounter(bool i_value);
  bool showEngineCounter() const;
  void setShowMinimap(bool i_value);
  bool showMinimap() const;
  void setMultiStopWhenOneFinishes(bool i_value);
  bool MultiStopWhenOneFinishes() const;
  void setEnableMenuMusic(bool i_value);
  bool enableMenuMusic() const;
  void setEnableInitZoom(bool i_value);
  bool enableInitZoom() const;
  void setEnableDeadAnimation(bool i_value);
  bool enableDeadAnimation() const;
  void setMenuGraphics(GraphicsLevel i_value);
  GraphicsLevel menuGraphics() const;
  void setGameGraphics(GraphicsLevel i_value);
  GraphicsLevel gameGraphics() const;
  void setQuickStartQualityMIN(int i_value);
  int quickStartQualityMIN() const;
  void setQuickStartQualityMAX(int i_value);
  int quickStartQualityMAX() const;
  void setQuickStartDifficultyMIN(int i_value);
  int quickStartDifficultyMIN() const;
  void setQuickStartDifficultyMAX(int i_value);
  int quickStartDifficultyMAX() const;
  void setMultiNbPlayers(int i_value);
  int multiNbPlayers() const;
  void setMultiEnablStopWheNoneFinishes(bool i_value);
  bool multiEnablStopWheNoneFinishes() const;
  void setEnableContextHelp(bool i_value);
  bool enableContextHelp() const;
  void setTheme(const std::string& i_value);
  std::string theme() const;
  void setEnableAudio(bool i_value);
  bool enableAudio() const;
  void setAudioSampleRate(int i_value);
  int audioSampleRate() const;
  void setAudioSampleBits(int i_value);
  int audioSampleBits() const;
  void setAudioChannels(int i_value);
  int audioChannels() const;
  void setEnableAudioEngine(bool i_value);
  bool enableAudioEngine() const;
  void setCheckNewLevelsAtStartup(bool i_value);
  bool checkNewLevelsAtStartup() const;
  void setCheckNewHighscoresAtStartup(bool i_value);
  bool checkNewHighscoresAtStartup() const;
  void setShowHighscoreInGame(bool i_value);
  bool showHighscoreInGame() const;

  private:
  bool m_verbose;
  bool m_useGraphics;
  int  m_resolutionWidth;
  int  m_resolutionHeight;
  int  m_bpp;
  bool m_windowed;
  bool m_glExts;
  std::string m_drawlib;
  bool m_www;
  bool m_benchmark;
  bool m_debug;
  bool m_sqlTrace;
  std::string m_profile;
  bool m_gdebug;
  std::string m_gdebug_file;
  bool m_timedemo;
  bool m_fps;
  bool m_ugly;
  bool m_uglyOver;
  bool m_testTheme;
  bool m_autosaveHighscoreReplays;
  bool m_ghostStrategy_MYBEST;
  bool m_ghostStrategy_THEBEST;
  bool m_ghostStrategy_BESTOFROOM;
  bool m_enableGhosts;
  bool m_enableEngineSound;
  bool m_showEngineCounter;
  bool m_showMinimap;
  bool m_multiStopWhenOneFinishes; /* in multiplayer, stop the game when one finishes the level */
  bool m_enableMenuMusic;
  bool m_enableInitZoom;
  bool m_enableDeadAnimation;
  GraphicsLevel m_menuGraphics;
  GraphicsLevel m_gameGraphics;
  int m_quickStartQualityMIN;
  int m_quickStartQualityMAX;
  int m_quickStartDifficultyMIN;
  int m_quickStartDifficultyMAX;
  int m_multiNbPlayers;
  bool m_multiEnablStopWheNoneFinishes;
  bool m_enableContextHelp;
  std::string m_theme;
  bool m_enableAudio;
  int m_audioSampleRate;
  int m_audioSampleBits;
  int m_audioChannels;
  bool m_enableAudioEngine;
  bool m_checkNewLevelsAtStartup;
  bool m_checkNewHighscoresAtStartup;
  bool m_showHighscoreInGame;
};

#endif
