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

#include "XMArgs.h"
#include "VFileIO.h"
#include "XMBuild.h"
#include "helpers/VExcept.h"
#include "xmoto/LevelsManager.h"
#include <cstdio>
#include <sstream>
#include <stdlib.h>

XMArguments::XMArguments() {
  m_opt_pack = false;
  m_opt_unpack = false;
  m_unpack_noList = false;
  m_opt_res = false;
  m_res_dispWidth = 0;
  m_res_dispHeight = 0;
  m_opt_fs = false;
  m_opt_win = false;
  m_opt_noexts = false;
  m_opt_novobs = false;
  m_opt_drawlib = false;
  m_opt_ugly = false;
  m_opt_noLog = false;
  m_opt_nowww = false;
  m_opt_help = false;
  m_opt_verbose = false;
  m_opt_debug = false;
  m_opt_sqlTrace = false;
  m_opt_fps = false;
  m_opt_replay = false;
  m_opt_listReplays = false;
  m_opt_replayInfos = false;
  m_opt_levelID = false;
  m_opt_levelFile = false;
  m_opt_listLevels = false;
  m_opt_demo = false;
  m_opt_profile = false;
  m_opt_timedemo = false;
  m_opt_testTheme = false;
  m_opt_benchmark = false;
  m_opt_cleanCache = false;
  m_opt_cleanNoWWWLevels = false;
  m_opt_gdebug = false;
  m_opt_configpath = false;
  m_opt_nosound = false;
  m_opt_videoRecording = false;
  m_opt_videoRecordingDivision = false;
  m_opt_videoRecordingFramerate = false;
  m_opt_videoRecordingStartTime = false;
  m_opt_videoRecordingEndTime = false;
  m_opt_hidePlayingInformation = false;
  m_opt_forceChildrenCompliant = false;
  m_opt_default_theme = false;
  m_opt_noDBDirsCheck = false;
  m_opt_serverOnly = false;
  m_opt_serverPort = false;
  m_opt_serverAdminPassword = false;
  m_opt_updateLevelsOnly = false;
  m_opt_clientConnectAtStartup = false;
  m_opt_adminMode = false;
  m_opt_buildQueries = false;
}

void XMArguments::parse(int i_argc, char **i_argv) {
  std::string v_opt;
  std::string v_arg;

  int i = 1;
  while (i < i_argc) {
    v_opt = i_argv[i];

    if (v_opt == "--pack") {
      m_opt_pack = true;
      if (i + 1 < i_argc) {
        m_pack_bin = i_argv[i + 1];
        i++;
      }
      if (i + 1 < i_argc) {
        m_pack_dir = i_argv[i + 1];
        i++;
      }
    } else if (v_opt == "--unpack") {
      m_opt_unpack = true;
      if (i + 1 < i_argc) {
        m_unpack_bin = i_argv[i + 1];
        i++;
      }
      if (i + 1 < i_argc) {
        m_unpack_dir = i_argv[i + 1];
        i++;
      }
      if (i + 1 < i_argc) {
        v_arg = i_argv[i + 1];
        if (v_arg != "no_list") {
          throw Exception("only no_list is allow here");
        }
        m_unpack_noList = true;
        i++;
      }
    } else if (v_opt == "--nosound") {
      m_opt_nosound = true;
    } else if (v_opt == "-v" || v_opt == "--verbose") {
      m_opt_verbose = true;
    } else if (v_opt == "-res" || v_opt == "--resolution") {
      m_opt_res = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing resolution");
      }
      sscanf(i_argv[i + 1], "%ix%i", &m_res_dispWidth, &m_res_dispHeight);
      i++;
    } else if (v_opt == "-win" || v_opt == "--windowed") {
      m_opt_win = true;
    } else if (v_opt == "-fs" || v_opt == "--fullscreen") {
      m_opt_fs = true;
    } else if (v_opt == "--noexts") {
      m_opt_noexts = true;
    } else if (v_opt == "--novobs") {
      m_opt_novobs = true;
    } else if (v_opt == "--drawlib") {
      m_opt_drawlib = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing drawlib name");
      }
      m_drawlib_lib = i_argv[i + 1];
      i++;
    } else if (v_opt == "-h" || v_opt == "-?" || v_opt == "--help" ||
               v_opt == "-help") {
      m_opt_help = true;
    } else if (v_opt == "--nowww") {
      m_opt_nowww = true;
    } else if (v_opt == "-r" || v_opt == "--replay") {
      m_opt_replay = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing replay file");
      }
      m_replay_file = i_argv[i + 1];
      i++;
    } else if (v_opt == "-l" || v_opt == "--level") {
      m_opt_levelID = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing level id");
      }
      m_levelID_id = XMArguments::levelArg2levelId(i_argv[i + 1]);
      i++;
    } else if (v_opt == "--levelFile") {
      m_opt_levelFile = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing level file");
      }
      m_levelFile_file = i_argv[i + 1];
      i++;
    } else if (v_opt == "-d" || v_opt == "--debug") {
      m_opt_debug = true;
    } else if (v_opt == "--sqlTrace") {
      m_opt_sqlTrace = true;
    } else if (v_opt == "--children") {
      m_opt_forceChildrenCompliant = true;
    } else if (v_opt == "-p" || v_opt == "--profile") {
      m_opt_profile = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing profile");
      }
      m_profile_value = i_argv[i + 1];
      i++;
    } else if (v_opt == "--gdebug") {
      m_opt_gdebug = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing gdebug file");
      }
      m_gdebug_file = i_argv[i + 1];
      i++;
    } else if (v_opt == "-ll" || v_opt == "--listlevels") {
      m_opt_listLevels = true;
    } else if (v_opt == "-lr" || v_opt == "--listreplays") {
      m_opt_listReplays = true;
    } else if (v_opt == "-td" || v_opt == "--timedemo") {
      m_opt_timedemo = true;
    } else if (v_opt == "--fps") {
      m_opt_fps = true;
    } else if (v_opt == "--ugly") {
      m_opt_ugly = true;
    } else if (v_opt == "--testTheme") {
      m_opt_testTheme = true;
    } else if (v_opt == "--benchmark") {
      m_opt_benchmark = true;
    } else if (v_opt == "--cleancache") {
      m_opt_cleanCache = true;
    } else if (v_opt == "--noLog") {
      m_opt_noLog = true;
    } else if (v_opt == "--cleanNoWWWLevels") {
      m_opt_cleanNoWWWLevels = true;
    } else if (v_opt == "-ri" || v_opt == "--replayInfos") {
      m_opt_replayInfos = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing replay");
      }
      m_replayInfos_file = i_argv[i + 1];
      i++;
    } else if (v_opt == "--configpath") {
      m_opt_configpath = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing config path");
      }
      m_configpath_path = i_argv[i + 1];
      i++;
    } else if (v_opt == "--videoRecording") {
      m_opt_videoRecording = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing video name");
      }
      m_opt_videoRecording_name = i_argv[i + 1];
      i++;
    } else if (v_opt == "--videoRecordingSizeDivision") {
      m_opt_videoRecordingDivision = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing value");
      }
      m_opt_videoRecordingDivision_value = atoi(i_argv[i + 1]);
      i++;
    } else if (v_opt == "--videoRecordingFramerate") {
      m_opt_videoRecordingFramerate = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing value");
      }
      m_opt_videoRecordingFramerate_value = atoi(i_argv[i + 1]);
      if (m_opt_videoRecordingFramerate_value < 1) {
        m_opt_videoRecordingFramerate_value = 1;
      }
      i++;
    } else if (v_opt == "--videoRecordingStartTime") {
      m_opt_videoRecordingStartTime = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing value");
      }
      m_opt_videoRecordingStartTime_value = atoi(i_argv[i + 1]);
      i++;
    } else if (v_opt == "--videoRecordingEndTime") {
      m_opt_videoRecordingEndTime = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing value");
      }
      m_opt_videoRecordingEndTime_value = atoi(i_argv[i + 1]);
      i++;
    } else if (v_opt == "--hidePlayingInformation") {
      m_opt_hidePlayingInformation = true;
    } else if (v_opt == "--noDBDirsCheck") {
      m_opt_noDBDirsCheck = true;
    } else if (v_opt == "--server") {
      m_opt_serverOnly = true;
    } else if (v_opt == "--serverPort") {
      m_opt_serverPort = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing value");
      }
      m_opt_serverPort_value = atoi(i_argv[i + 1]);
      i++;
    } else if (v_opt == "--serverAdminPassword") {
      m_opt_serverAdminPassword = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing value");
      }
      m_opt_serverAdminPassword_value = i_argv[i + 1];
      i++;
    } else if (v_opt == "--updateLevelsOnly") {
      m_opt_updateLevelsOnly = true;
    } else if (v_opt == "--connectAtStartup") {
      m_opt_clientConnectAtStartup = true;
    } else if (v_opt == "--defaultTheme") {
      m_opt_default_theme = true;
      if (i + 1 >= i_argc) {
        throw SyntaxError("missing default theme name");
      }
      m_opt_default_theme_value = i_argv[i + 1];
      i++;
    } else if (v_opt == "--admin") { // hidden option to control website from
      // the game ; keep undocumented
      m_opt_adminMode = true;
    } else if (v_opt == "--buildQueries") { // hidden option to control website
      // from the game ; keep undocumented
      m_opt_buildQueries = true;
    } else if (v_opt.rfind("-psn_", 0) == 0) {
      /* macOS sometimes passes a "Process Serial Number" (psn) to applications.
       * Ignore. */
    } else {
      /* check if the parameter is a file */
      v_arg = i_argv[i];
      std::string v_extension = XMFS::getFileExtension(v_arg);

      if (v_extension == "rpl") { /* replay file */
        m_opt_replay = true;
        m_replay_file = v_arg;
      } else if (v_extension == "lvl") { /* level file */
        m_opt_levelFile = true;
        m_levelFile_file = v_arg;
      } else if (v_extension == "dmo") {
        m_opt_demo = true;
        m_demo_file = v_arg;
      } else {
        throw Exception("Invalid option \"" + v_opt + "\"");
      }
    }

    i++;
  }
}

std::string XMArguments::levelArg2levelId(std::string i_arg) {
  std::string v_res = i_arg;

  /* If it is a plain number */
  bool v_isANumber = true;
  for (unsigned int i = 0; i < v_res.length(); i++) {
    if (v_res[i] < '0' || v_res[i] > '9') {
      v_isANumber = false;
    }
  }

  if (v_isANumber) {
    int nNum = atoi(v_res.c_str()) - 1;
    if (nNum >= 0) {
      std::ostringstream v_num;
      if (nNum < 10) {
        v_num << "0";
      }
      v_num << nNum;
      v_res = "_iL" + v_num.str() + "_";
    }
  }

  return v_res;
}

bool XMArguments::isOptPack() const {
  return m_opt_pack;
}

std::string XMArguments::getOpt_pack_bin() const {
  return m_pack_bin;
}

std::string XMArguments::getOpt_pack_dir() const {
  return m_pack_dir;
}

bool XMArguments::isOptUnPack() const {
  return m_opt_unpack;
}

std::string XMArguments::getOpt_unpack_bin() const {
  return m_unpack_bin;
}

std::string XMArguments::getOpt_unpack_dir() const {
  return m_unpack_dir;
}

bool XMArguments::getOpt_unpack_noList() const {
  return m_unpack_noList;
}

bool XMArguments::isOptVerbose() const {
  return m_opt_verbose;
}

bool XMArguments::isOptRes() const {
  return m_opt_res;
}

int XMArguments::getOpt_res_dispWidth() const {
  return m_res_dispWidth;
}

int XMArguments::getOpt_res_dispHeight() const {
  return m_res_dispHeight;
}

bool XMArguments::isOptWindowed() const {
  return m_opt_win;
}

bool XMArguments::isOptFs() const {
  return m_opt_fs;
}

bool XMArguments::isOptNoExts() const {
  return m_opt_noexts;
}

bool XMArguments::isOptNoVOBS() const {
  return m_opt_novobs;
}

bool XMArguments::isOptDrawlib() const {
  return m_opt_drawlib;
}

std::string XMArguments::getOpt_drawlib_lib() const {
  return m_drawlib_lib;
}

bool XMArguments::isOptHelp() const {
  return m_opt_help;
}

bool XMArguments::isOptNoWWW() const {
  return m_opt_nowww;
}

bool XMArguments::isOptReplay() const {
  return m_opt_replay;
}

std::string XMArguments::getOpt_replay_file() const {
  return m_replay_file;
}

bool XMArguments::isOptLevelID() const {
  return m_opt_levelID;
}

std::string XMArguments::getOpt_levelID_id() const {
  return m_levelID_id;
}

bool XMArguments::isOptLevelFile() const {
  return m_opt_levelFile;
}

std::string XMArguments::getOpt_levelFile_file() const {
  return m_levelFile_file;
}

bool XMArguments::isOptDemo() const {
  return m_opt_demo;
}

std::string XMArguments::getOpt_demo_file() const {
  return m_demo_file;
}

bool XMArguments::isOptDebug() const {
  return m_opt_debug;
}

bool XMArguments::isOptSqlTrace() const {
  return m_opt_sqlTrace;
}

bool XMArguments::isOptProfile() const {
  return m_opt_profile;
}

bool XMArguments::isOptForceChildrenCompliant() const {
  return m_opt_forceChildrenCompliant;
}

std::string XMArguments::getOpt_profile_value() const {
  return m_profile_value;
}

bool XMArguments::isOptGDebug() const {
  return m_opt_gdebug;
}

std::string XMArguments::getOpt_gdebug_file() const {
  return m_gdebug_file;
}

bool XMArguments::isOptListLevels() const {
  return m_opt_listLevels;
}

bool XMArguments::isOptListReplays() const {
  return m_opt_listReplays;
}

bool XMArguments::isOptTimedemo() const {
  return m_opt_timedemo;
}

bool XMArguments::isOptFps() const {
  return m_opt_fps;
}

bool XMArguments::isOptUgly() const {
  return m_opt_ugly;
}

bool XMArguments::isOptNoLog() const {
  return m_opt_noLog;
}

bool XMArguments::isOptTestTheme() const {
  return m_opt_testTheme;
}

bool XMArguments::isOptBenchmark() const {
  return m_opt_benchmark;
}

bool XMArguments::isOptCleanCache() const {
  return m_opt_cleanCache;
}

bool XMArguments::isOptCleanNoWWWLevels() const {
  return m_opt_cleanNoWWWLevels;
}

bool XMArguments::isOptReplayInfos() const {
  return m_opt_replayInfos;
}

std::string XMArguments::getOpt_replayInfos_file() const {
  return m_replayInfos_file;
}

bool XMArguments::isOptConfigPath() const {
  return m_opt_configpath;
}

bool XMArguments::isOptNoSound() const {
  return m_opt_nosound;
}

bool XMArguments::isOptDefaultTheme() const {
  return m_opt_default_theme;
}

std::string XMArguments::getOpt_defaultTheme_theme() const {
  return m_opt_default_theme_value;
}

std::string XMArguments::getOpt_configPath_path() const {
  return m_configpath_path;
}

bool XMArguments::isOptVideoRecording() const {
  return m_opt_videoRecording;
}

std::string XMArguments::getOptVideoRecording_name() const {
  return m_opt_videoRecording_name;
}

bool XMArguments::isOptVideoRecordingDivision() const {
  return m_opt_videoRecordingDivision;
}

int XMArguments::getOptVideoRecordingDivision_value() const {
  return m_opt_videoRecordingDivision_value;
}

bool XMArguments::isOptVideoRecordingFramerate() const {
  return m_opt_videoRecordingFramerate;
}

int XMArguments::getOptVideoRecordingFramerate_value() const {
  return m_opt_videoRecordingFramerate_value;
}

bool XMArguments::isOptVideoRecordingStartTime() const {
  return m_opt_videoRecordingStartTime;
}

int XMArguments::getOptVideoRecordingStartTime_value() const {
  return m_opt_videoRecordingStartTime_value;
}

bool XMArguments::isOptVideoRecordingEndTime() const {
  return m_opt_videoRecordingEndTime;
}

int XMArguments::getOptVideoRecordingEndTime_value() const {
  return m_opt_videoRecordingEndTime_value;
}

bool XMArguments::isOptHidePlayingInformation() const {
  return m_opt_hidePlayingInformation;
}

bool XMArguments::isOptNoDBDirsCheck() const {
  return m_opt_noDBDirsCheck;
}

bool XMArguments::isOptServerOnly() const {
  return m_opt_serverOnly;
}

bool XMArguments::isOptServerPort() const {
  return m_opt_serverPort;
}

int XMArguments::getOptServerPort_value() const {
  return m_opt_serverPort_value;
}

bool XMArguments::isOptServerAdminPassword() const {
  return m_opt_serverAdminPassword;
}

std::string XMArguments::getOptServerAdminPassword_value() const {
  return m_opt_serverAdminPassword_value;
}

bool XMArguments::isOptClientConnectAtStartup() const {
  return m_opt_clientConnectAtStartup;
}

bool XMArguments::isOptUpdateLevelsOnly() const {
  return m_opt_updateLevelsOnly;
}

bool XMArguments::isOptAdminMode() const {
  return m_opt_adminMode;
}

bool XMArguments::isOptBuildQueries() const {
  return m_opt_buildQueries;
}

void XMArguments::help(const std::string &i_cmd) {
  printf("X-Moto %s\n", XMBuild::getVersionString().c_str());
  printf("usage:  %s [options]\n"
         "options:\n",
         i_cmd.c_str());

  printf("\tFILE\n\t\tOpen the replay or the level from file FILE.\n");
  printf("\t-l, --level ID\n\t\tStart playing the given level right away.\n");
  printf("\t--levelFile FILE\n\t\tStart playing the given level right away.\n");
  printf("\t-r, --replay NAME\n\t\tPlayback replay with the given name.\n");
  printf("\t-ri, --replayInfos REPLAY NAME\n\t\tDisplay information about a "
         "replay.\n");
  printf(
    "\t-p, --profile NAME\n\t\tUse the profile NAME as the player profile.\n");
  printf("\t--children\n\t\tForce children mode.\n");
  printf("\t--configpath PATH\n\t\tUse the path PATH as the xmoto "
         "configuration path.\n");
  printf("\t-ll, --listlevels\n\t\tOutputs a list of all installed levels.\n");
  printf("\t-lr, --listreplays\n\t\tOutputs a list of all replays.\n");
  printf("\t--nowww\n\t\tDisable web connection at startup.\n");
  printf("\t--nosound\n\t\tDisable sound at startup.\n");
  printf("\t-res, --resolution WIDTHxHEIGHT\n\t\tSpecifies display resolution "
         "to use.\n");
  printf("\t-fs, --fullscreen\n\t\tForces fullscreen mode.\n");
  printf("\t-win, --windowed\n\t\tForces windowed mode.\n");
  printf("\t-v, --verbose\n\t\tBe verbose.\n");
  printf("\t--noexts\n\t\tDon't use any OpenGL extensions.\n");
  printf("\t--novobs\n\t\tDon't use VOB OpenGL extension "
         "(GL_ARB_vertex_buffer_object).\n");
  printf("\t--fps\n\t\tDisplay framerate.\n");
  printf("\t--ugly\n\t\tEnable 'ugly' mode, suitable for computers without\n");
  printf("\t--testTheme\n\t\tDisplay forms around the theme to check it.\n");
  printf("\t-d, --debug\n\t\tEnable debug mode.\n");
  printf("\t--sqlTrace\n\t\tEnable sql trace mode.\n");
  printf("\t-td, --timedemo\n\t\tNo delaying, maximum framerate.\n");
  printf("\t\ta good OpenGL-enabled video card.\n");
  printf("\t--benchmark\n\t\tOnly meaningful when combined with --replay\n");
  printf("\t\tand --timedemo. Useful to determine the graphics\n");
  printf("\t\tperformance.\n");
  printf("\t--cleancache\n\t\tDeletes the content of the level cache.\n");
  printf("\t--cleanNoWWWLevels\n\t\tCheck web levels list and remove levels "
         "which are not available on the web.\n");
  printf("\t--noLog\n\t\tDon't log information into the xmoto.log file\n");
  printf("\t--videoRecording\n\t\tEnable video recording.\n");
  printf("\t--videoRecordingSizeDivision DIVISION\n\t\tChange video size "
         "(1=full, 2=50%%, 4=25%%).\n");
  printf(
    "\t--videoRecordingFramerate FRAMERATE\n\t\tChange video framerate.\n");
  printf("\t--videoRecordingStartTime NBCENTSOFSECONDS\n\t\tStart recording "
         "video after this game time.\n");
  printf("\t--videoRecordingEndTime NBCENTSOFSECONDS\n\t\tStop recording video "
         "after this game time.\n");
  printf("\t--hidePlayingInformation\n\t\tDon't show some information while "
         "playing/replaying ; useful to make nicer video.\n");
  printf("\t--drawlib DRAWLIB\n\t\tChoose the render to use (default one is "
         "OPENGL if available).\n");
  printf(
    "\t--defaultTheme THEME\n\t\tDefault theme for new profiles created.\n");
  printf("\t--noDBDirsCheck\n\t\tDon't check that system and user dirs changed "
         "at startup.\n");
  printf("\t--server\n\t\tRun X-Moto as a server only (no gui).\n");
  printf(
    "\t--serverPort PORT\n\t\tSpecify the server port (with --server only).\n");
  printf("\t--serverAdminPassword PASSWORD\n\t\tSpecify a server admin "
         "password which is always valid (with --server only).\n");
  printf("\t--updateLevelsOnly\n\t\tOnly update levels (no gui).\n");
  printf(
    "\t--connectAtStartup\n\t\tConnect the client to the server at startup.\n");
  printf("\t-h, -?, -help, --help\n\t\tDisplay this message.\n");
  printf(
    "\t--pack [BIN] [DIR]\n\t\tBuild the BIN package from directory DIR.\n");
  printf("\t--unpack [BIN] [DIR] [no_list]\n\t\tUnpack the BIN package into "
         "the dir DIR.\n");
  printf(
    "\t--buildQueries\n\t\tBuild sql queries into file " XM_SQLQUERIES_GEN_FILE
    ".\n");
// If both sdlgfx and opengl are available give the user the posibility
// to select the draw library
#ifdef ENABLE_SDLGFX
#ifdef ENABLE_OPENGL
  printf("\t--drawlib [OPENGL|SDLGFX]\n\t\tSelect the draw library to use.\n");
#endif
#endif
}
