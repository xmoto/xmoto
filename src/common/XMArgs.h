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

#ifndef __XMARGS_H__
#define __XMARGS_H__

#include <string>

class XMArguments {
public:
  XMArguments();
  void parse(int i_argc, char **i_argv);
  void help(const std::string &i_cmd);

  bool isOptPack() const;
  std::string getOpt_pack_bin() const;
  std::string getOpt_pack_dir() const;
  bool isOptUnPack() const;
  std::string getOpt_unpack_bin() const;
  std::string getOpt_unpack_dir() const;
  bool getOpt_unpack_noList() const;
  bool isOptVerbose() const;
  bool isOptRes() const;
  int getOpt_res_dispWidth() const;
  int getOpt_res_dispHeight() const;
  bool isOptWindowed() const;
  bool isOptFs() const;
  bool isOptNoExts() const;
  bool isOptNoVOBS() const;
  bool isOptDrawlib() const;
  std::string getOpt_drawlib_lib() const;
  bool isOptHelp() const;
  bool isOptNoWWW() const;
  bool isOptReplay() const;
  std::string getOpt_replay_file() const;
  bool isOptLevelID() const;
  std::string getOpt_levelID_id() const;
  bool isOptLevelFile() const;
  std::string getOpt_levelFile_file() const;
  bool isOptDemo() const;
  std::string getOpt_demo_file() const;
  bool isOptDebug() const;
  bool isOptSqlTrace() const;
  bool isOptProfile() const;
  std::string getOpt_profile_value() const;
  bool isOptGDebug() const;
  std::string getOpt_gdebug_file() const;
  bool isOptListLevels() const;
  bool isOptListReplays() const;
  bool isOptTimedemo() const;
  bool isOptFps() const;
  bool isOptUgly() const;
  bool isOptNoLog() const;
  bool isOptTestTheme() const;
  bool isOptBenchmark() const;
  bool isOptCleanCache() const;
  bool isOptCleanNoWWWLevels() const;
  bool isOptReplayInfos() const;
  std::string getOpt_replayInfos_file() const;
  bool isOptConfigPath() const;
  std::string getOpt_configPath_path() const;
  bool isOptNoSound() const;
  bool isOptDefaultTheme() const;
  std::string getOpt_defaultTheme_theme() const;
  bool isOptVideoRecording() const;
  std::string getOptVideoRecording_name() const;
  bool isOptVideoRecordingDivision() const;
  int getOptVideoRecordingDivision_value() const;
  bool isOptVideoRecordingFramerate() const;
  int getOptVideoRecordingFramerate_value() const;
  bool isOptVideoRecordingStartTime() const;
  int getOptVideoRecordingStartTime_value() const;
  bool isOptVideoRecordingEndTime() const;
  int getOptVideoRecordingEndTime_value() const;
  bool isOptHidePlayingInformation() const;
  bool isOptForceChildrenCompliant() const;
  bool isOptNoDBDirsCheck() const;
  bool isOptServerOnly() const;
  bool isOptServerPort() const;
  int getOptServerPort_value() const;
  bool isOptServerAdminPassword() const;
  std::string getOptServerAdminPassword_value() const;
  bool isOptUpdateLevelsOnly() const;
  bool isOptClientConnectAtStartup() const;
  bool isOptAdminMode() const;
  bool isOptBuildQueries() const;

private:
  /* pack options */
  bool m_opt_pack;
  std::string m_pack_bin;
  std::string m_pack_dir;
  bool m_opt_unpack;
  std::string m_unpack_bin;
  std::string m_unpack_dir;
  bool m_unpack_noList;

  /* graphics */
  bool m_opt_res;
  int m_res_dispWidth;
  int m_res_dispHeight;
  bool m_opt_fs;
  bool m_opt_win;
  bool m_opt_noexts;
  bool m_opt_novobs;
  bool m_opt_drawlib;
  std::string m_drawlib_lib;
  bool m_opt_ugly;
  bool m_opt_nosound;
  bool m_opt_noLog;

  /* config */
  bool m_opt_configpath;
  std::string m_configpath_path;

  /* web */
  bool m_opt_nowww;

  /* extra */
  bool m_opt_help;
  bool m_opt_verbose;
  bool m_opt_debug;
  bool m_opt_sqlTrace;
  bool m_opt_fps;
  bool m_opt_gdebug;
  std::string m_gdebug_file;

  /* replays */
  bool m_opt_replay;
  std::string m_replay_file;
  bool m_opt_listReplays;
  bool m_opt_replayInfos;
  std::string m_replayInfos_file;

  /* levels */
  bool m_opt_levelID;
  std::string m_levelID_id;
  bool m_opt_levelFile;
  std::string m_levelFile_file;
  bool m_opt_listLevels;

  /* demos */
  bool m_opt_demo;
  std::string m_demo_file;

  /* profile */
  bool m_opt_profile;
  std::string m_profile_value;
  bool m_opt_forceChildrenCompliant;

  /* game */
  bool m_opt_timedemo;
  bool m_opt_testTheme;
  bool m_opt_benchmark;
  bool m_opt_cleanCache;
  bool m_opt_cleanNoWWWLevels;

  /* video */
  bool m_opt_videoRecording;
  std::string m_opt_videoRecording_name;
  bool m_opt_videoRecordingDivision;
  int m_opt_videoRecordingDivision_value;
  bool m_opt_videoRecordingFramerate;
  int m_opt_videoRecordingFramerate_value;
  bool m_opt_videoRecordingStartTime; /* value in cent of seconds, a negativ
                                         value for always */
  int m_opt_videoRecordingStartTime_value;
  bool m_opt_videoRecordingEndTime; /* value in cent of seconds, a negativ value
                                       for always */
  int m_opt_videoRecordingEndTime_value;
  bool m_opt_hidePlayingInformation;

  /* update levels only */
  bool m_opt_updateLevelsOnly;

  /* server */
  bool m_opt_serverOnly;
  bool m_opt_serverPort;
  int m_opt_serverPort_value;
  bool m_opt_serverAdminPassword;
  std::string m_opt_serverAdminPassword_value;

  /* net */
  bool m_opt_clientConnectAtStartup;

  /* specific */
  bool m_opt_noDBDirsCheck;

  /* default config replacement for new profiles */
  bool m_opt_default_theme;
  std::string m_opt_default_theme_value;

  /* at command line, you can pass [0-9]* for level of id _iLXX_ */
  static std::string levelArg2levelId(std::string i_arg);

  /* admin */
  bool m_opt_adminMode;

  /* c files */
  bool m_opt_buildQueries;
};

#endif
