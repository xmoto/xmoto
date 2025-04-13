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

/*
 *  Game application. (init-related stuff)
 */

#include "Game.h"
#include "GameText.h"
#include "PhysSettings.h"
#include "Sound.h"
#include "common/VFileIO.h"
#include "db/xmDatabase.h"
#include "helpers/Environment.h"
#include "helpers/Log.h"
#include "helpers/Random.h"
#include "helpers/Time.h"
#include "input/Input.h"
#include "input/Joystick.h"

#include "Credits.h"
#include "GeomsManager.h"
#include "LuaLibBase.h"
#include "Replay.h"
#include "SysMessage.h"
#include "XMDemo.h"
#include "common/Packager.h"
#include "common/VXml.h"
#include "common/XMArgs.h"
#include "common/XMSession.h"
#include "drawlib/DrawLib.h"
#include "gui/specific/GUIXMoto.h"
#include "helpers/SwapEndian.h"
#include "helpers/Text.h"
#include <curl/curl.h>

#include "states/StateEditProfile.h"
#include "states/StateMainMenu.h"
#include "states/StateManager.h"
#include "states/StateMessageBox.h"
#include "states/StatePlayingLocal.h"
#include "states/StatePreplayingGame.h"
#include "states/StatePreplayingReplay.h"
#include "states/StateReplaying.h"
#include "states/StateWaitServerInstructions.h"

#include "thread/UpgradeLevelsThread.h"

#include "UserConfig.h"
#include "include/xm_SDL_net.h"
#include "net/ActionReader.h"
#include "net/NetActions.h"
#include "net/NetClient.h"
#include "net/NetServer.h"

#if !defined(WIN32)
#include <signal.h>
#endif

#define MOUSE_DBCLICK_TIME 0.250f

#define XM_MAX_NB_LOOPS_WITH_NONETWORK 3
#define XM_MAX_NB_LOOPS_WITH_NORENDERING 5
#define XM_MAX_FRAMELATE_TO_FORCE_NORENDERING 10

#define XMSERVER_BUF 80
#define XMSERVER_STRBUF "80"

#ifdef __amigaos4__
#include <dos/dos.h>
#include <proto/exec.h>
BOOL LibraryOK(STRPTR libname, ULONG version, ULONG revision) {
  struct Library *libBase;
  BOOL ok = FALSE;
  if (libBase = (struct Library *)IExec->OpenLibrary(libname, version)) {
    if (libBase->lib_Version > version || (libBase->lib_Version == version &&
                                           libBase->lib_Revision >= revision)) {
      ok = TRUE;
    }
    IExec->CloseLibrary(libBase);
  }
  return ok;
}
#endif

int main(int nNumArgs, char **ppcArgs) {
#ifdef __amigaos4__
  /* checking for MiniGL 2.0 */

  if (!LibraryOK("minigl.library", 2, 0)) {
    printf("I need MiniGL 2.0 Beta or newer. Please upgrade!\n");
    return RETURN_FAIL;
  }
#endif

  /* Start application */
  try {
    /* Setup basic info */
    GameApp::instance()->run(nNumArgs, ppcArgs);
    GameApp::destroy();
  } catch (Exception &e) {
    if (Logger::isInitialized()) {
      LogError((std::string("Exception: ") + e.getMsg()).c_str());
    }

    printf("fatal exception : %s\n", e.getMsg().c_str());
    SDL_Quit(); /* make sure SDL shuts down gracefully */

#if defined(WIN32)
    char cBuf[1024];
    snprintf(cBuf,
             1024,
             "Fatal exception occurred: %s\n"
             "Consult the logs/xmoto.log file for more information about what\n"
             "might has occurred.\n",
             e.getMsg().c_str());
    MessageBox(NULL, cBuf, "X-Moto Error", MB_OK | MB_ICONERROR);
#endif
  }
  return 0;
}

void xmexit_term(int i_signal) {
  // it seems that it doesn't help return on first line of this function to go
  // back to the previous function

  if (GameApp::instance()->standAloneServer() != NULL) {
    if (Logger::isInitialized()) {
      LogInfo("signal received.");
    }
    GameApp::instance()->standAloneServer()->stop();
    GameApp::instance()->run_unload();
    GameApp::destroy();
  }
  exit(0);
}

void GameApp::run(int nNumArgs, char **ppcArgs) {
  try {
    run_load(nNumArgs, ppcArgs);
    run_loop();
  } catch (Exception &e) {
    run_unload();
    throw e;
  }
  run_unload();
}

void GameApp::run_load(int nNumArgs, char **ppcArgs) {
  XMArguments v_xmArgs;
  m_isODEInitialized = false;
  bool v_useGraphics = true;
  bool v_missingFont = false;
  std::string v_missingFontError;

  Environment::init();

  /* check args */
  try {
    v_xmArgs.parse(nNumArgs, ppcArgs);
  } catch (Exception &e) {
    printf("syntax error : %s\n", e.getMsg().c_str());
    v_xmArgs.help(nNumArgs >= 1 ? ppcArgs[0] : "xmoto");
    quit();
    return; /* abort */
  }

  /* help */
  if (v_xmArgs.isOptHelp()) {
    v_xmArgs.help(nNumArgs >= 1 ? ppcArgs[0] : "xmoto");
    quit();
    return;
  }

  /* overwrite default by command line */
  if (v_xmArgs.isOptDefaultTheme()) {
    XMDefault::DefaultTheme = v_xmArgs.getOpt_defaultTheme_theme();
  }
  /**/

  /* init sub-systems */
  SwapEndian::Swap_Init();
  srand(time(NULL));

  /* package / unpackage */
  if (v_xmArgs.isOptPack()) {
    Packager::go(v_xmArgs.getOpt_pack_bin() == "" ? "xmoto.bin"
                                                  : v_xmArgs.getOpt_pack_bin(),
                 v_xmArgs.getOpt_pack_dir() == "" ? "."
                                                  : v_xmArgs.getOpt_pack_dir());
    quit();
    return;
  }
  if (v_xmArgs.isOptUnPack()) {
    Packager::goUnpack(
      v_xmArgs.getOpt_unpack_bin() == "" ? "xmoto.bin"
                                         : v_xmArgs.getOpt_unpack_bin(),
      v_xmArgs.getOpt_unpack_dir() == "" ? "." : v_xmArgs.getOpt_unpack_dir(),
      v_xmArgs.getOpt_unpack_noList() == false);
    quit();
    return;
  }
  /* ***** */

  if (v_xmArgs.isOptConfigPath()) {
    XMFS::init("xmoto",
               "xmoto.bin",
               v_xmArgs.isOptServerOnly() == false,
               v_xmArgs.getOpt_configPath_path());
  } else {
    XMFS::init("xmoto", "xmoto.bin", v_xmArgs.isOptServerOnly() == false);
  }
  Logger::init();

  /* c xmoto files */
  if (v_xmArgs.isOptBuildQueries()) {
    LevelsManager::writeDefaultPackages(XM_SQLQUERIES_GEN_FILE);
    quit();
    return;
  }

  /* load config file, the session */
  XMSession::createDefaultConfig(m_userConfig);

  try {
    m_userConfig->loadFile();
  } catch (Exception &e) {
    LogWarning("failed to load or parse user configuration file");
  }

  XMSession::setDefaultInstance("live");
  XMSession::instance()->loadConfig(
    m_userConfig); /* overload default session by userConfig */

  XMSession::instance()->loadArgs(
    &v_xmArgs); /* overload default session by xmargs     */

  Logger::setVerbose(
    XMSession::instance()->isVerbose()); /* apply verbose mode */
  Logger::setActiv(XMSession::instance()->noLog() ==
                   false); /* apply log activ mode */

  LogInfo(std::string("X-Moto " + XMBuild::getVersionString(true)).c_str());
  LogInfo("Started at %s", iso8601Date().c_str());
  auto endianness = SwapEndian::bigendian ? "big" : "little";
  LogInfo("System is %s-endian", endianness);

  LogInfo("User data   directory: %s", XMFS::getUserDir(FDT_DATA).c_str());
  LogInfo("User config directory: %s", XMFS::getUserDir(FDT_CONFIG).c_str());
  LogInfo("User cache  directory: %s", XMFS::getUserDir(FDT_CACHE).c_str());
  LogInfo("System data directory: %s", XMFS::getSystemDataDir().c_str());

  if (v_xmArgs.isOptListLevels() || v_xmArgs.isOptListReplays() ||
      v_xmArgs.isOptReplayInfos() || v_xmArgs.isOptServerOnly() ||
      v_xmArgs.isOptUpdateLevelsOnly()) {
    v_useGraphics = false;
  }

// doesn't work on windows
#if !defined(WIN32)
  if (v_xmArgs.isOptServerOnly()) {
    struct sigaction v_act;

    memset(&v_act, 0, sizeof(struct sigaction));
    v_act.sa_handler = xmexit_term;
    sigemptyset(&v_act.sa_mask);

    if (sigaction(SIGTERM, &v_act, NULL) != 0) {
      LogWarning("sigaction failed");
    }

    if (sigaction(SIGINT, &v_act, NULL) != 0) {
      LogWarning("sigaction failed");
    }
  }
#endif

  // init not so random numbers
  NotSoRandom::init();

  // gettext  requires database (pour la config de la langue)
  // database requires drawlib  (pour visualiser les upgrades)
  // drawlib  requires gettext  (pour choisir la fonte)

  /* database */
  /* thus, resolution/bpp/windowed parameters cannot be stored in the db (or
   * some minor modifications are required) */
  xmDatabase *pDb = xmDatabase::instance("main");
  pDb->preInitForProfileLoading(DATABASE_FILE);

  // db trace
  if (XMSession::instance()->sqlTrace()) {
    pDb->setTrace(XMSession::instance()->sqlTrace());
  }

  XMSession::instance("file")->loadConfig(m_userConfig);
  XMSession::instance("file")->loadProfile(
    XMSession::instance("file")->profile(), pDb);
  (*XMSession::instance()) = (*XMSession::instance("file"));

  XMSession::instance()->loadArgs(
    &v_xmArgs); /* overload default session by xmargs */
  // enable propagation only after overloading by command args
  XMSession::enablePropagation("file");

  LogInfo("SiteKey: %s", XMSession::instance()->sitekey().c_str());

#if USE_GETTEXT
  std::string v_locale = Locales::init(XMSession::instance()->language());
  LogInfo("Locales set to '%s' (directory '%s')",
          v_locale.c_str(),
          XMFS::getSystemLocaleDir().c_str());
  LogInfo("LANGUAGE=%s", Environment::get_variable("LANGUAGE").c_str());

  // some asian languages require ttf files ; change to us in case of problem
  try {
    DrawLib::checkFontPrerequites();
  } catch (Exception &e) {
    v_missingFont = true;
    v_missingFontError = e.getMsg();
    v_locale = Locales::init("en_US");
    LogWarning("font prerequisites missing, switching to en_US: %s",
               e.getMsg().c_str());
  }
#endif

  // _InitWin initializes SDL (and only SDL if the argument is false)
  _InitWin(v_useGraphics);

  /* drawlib */
  if (v_useGraphics) {
    /* init drawLib */
    drawLib = DrawLib::DrawLibFromName(XMSession::instance()->drawlib());

    if (drawLib == NULL) {
      throw Exception("Drawlib not initialized");
    }

    SysMessage::instance()->setDrawLib(drawLib);
    SysMessage::instance()->setConsoleSize(
      XMSession::instance()->consoleSize());

    drawLib->setNoGraphics(v_useGraphics == false);
    drawLib->setDontUseGLExtensions(XMSession::instance()->glExts() == false);
    drawLib->setDontUseGLVOBS(XMSession::instance()->glVOBS() == false);

    LogInfo("Wanted resolution: %ix%i",
            XMSession::instance()->resolutionWidth(),
            XMSession::instance()->resolutionHeight());
    drawLib->init(XMSession::instance()->resolutionWidth(),
                  XMSession::instance()->resolutionHeight(),
                  XMSession::instance()->windowed());
    /* drawlib can change the final resolution if it fails, then, reinit session
     * one's */
    XMSession::instance()->setResolutionWidth(drawLib->getDispWidth());
    XMSession::instance()->setResolutionHeight(drawLib->getDispHeight());
    XMSession::instance()->setWindowed(drawLib->getWindowed());
    LogInfo("Resolution: %ix%i",
            XMSession::instance()->resolutionWidth(),
            XMSession::instance()->resolutionHeight());

    /* set initial focus state */
    uint32_t wflags =
      SDL_GetWindowFlags(GameApp::instance()->getDrawLib()->getWindow());
    m_hasKeyboardFocus = wflags & SDL_WINDOW_INPUT_FOCUS;
    m_hasMouseFocus = wflags & SDL_WINDOW_MOUSE_FOCUS;
    m_isIconified = wflags & SDL_WINDOW_HIDDEN;
  } else {
    // Doesn't matter anyway
    m_hasKeyboardFocus = m_hasMouseFocus = m_isIconified = false;
  }

  /* init the database (fully, including upgrades) */
  if (!pDb->init(DATABASE_FILE,
                 XMSession::instance()->profile() == ""
                   ? std::string("")
                   : XMSession::instance()->profile(),
                 XMFS::getSystemDataDir(),
                 XMFS::getUserDir(FDT_DATA),
                 XMFS::binCheckSum(),
                 v_xmArgs.isOptNoDBDirsCheck() == false,
                 this)) {
    quit();
    return;
  }

  if (v_useGraphics) {
    // allocate the statemanager instance so that if it fails, it's not in a
    // thread (serverThread for example)
    StateManager::instance();
  }

  /* Init sound system */
  if (v_useGraphics) {
    Sound::init(XMSession::instance());
  }

  bool v_graphicAutomaticMode;
  v_graphicAutomaticMode =
    v_useGraphics && (v_xmArgs.isOptLevelID() || v_xmArgs.isOptLevelFile() ||
                      v_xmArgs.isOptReplay() || v_xmArgs.isOptDemo());

  // network requires the input information (to display the chat command)
  if (v_useGraphics) {
    Input::instance()->init(m_userConfig,
                            pDb,
                            XMSession::instance()->profile(),
                            XMSession::instance()->enableJoysticks());
  }

  // no command line need the network for the moment
  if (v_useGraphics || v_xmArgs.isOptServerOnly()) {
    initNetwork(v_xmArgs.isOptServerOnly(),
                v_xmArgs.isOptServerOnly() || v_graphicAutomaticMode);
  }

  /* Init renderer */
  if (v_useGraphics) {
    switchUglyMode(XMSession::instance()->ugly());
    switchTestThemeMode(XMSession::instance()->testTheme());
  }

  bool v_updateAfterInitDone = false;

  /* load theme */
  if (pDb->themes_isIndexUptodate() == false) {
    ThemeChoicer::initThemesFromDir(pDb);
    v_updateAfterInitDone = true;
  }

  if (v_xmArgs.isOptServerOnly() == false) {
    try {
      reloadTheme();
    } catch (Exception &e) {
      /* if the theme cannot be loaded, try to reload from files */
      /* perhaps that the xm.db comes from an other computer */
      LogWarning(
        "Theme cannot be reload, try to update themes into the database");
      ThemeChoicer::initThemesFromDir(pDb);
      reloadTheme();
    }
  }

  /* load levels */
  if (pDb->levels_isIndexUptodate() == false) {
    LevelsManager::instance()->reloadLevelsFromLvl(
      pDb, v_xmArgs.isOptServerOnly(), this);
    v_updateAfterInitDone = true;
  }
  LevelsManager::instance()->reloadExternalLevels(pDb, this);

  /* Update replays */
  if (pDb->replays_isIndexUptodate() == false) {
    initReplaysFromDir(pDb);
    v_updateAfterInitDone = true;
  }

  // levels_isIndexUptodate() && replays_isIndexUptodate() &&
  // themes_isIndexUptodate() done
  if (v_updateAfterInitDone) {
    pDb->setUpdateAfterInitDone(); // confirm to the database that the job is
    // done
  }

  /* List replays? */
  if (v_xmArgs.isOptListReplays()) {
    pDb->replays_print();
    quit();
    return;
  }

  if (v_xmArgs.isOptReplayInfos()) {
    Replay v_replay;
    std::string v_levelId;
    std::string v_player;

    v_levelId =
      v_replay.openReplay(v_xmArgs.getOpt_replayInfos_file(), v_player, true);
    if (v_levelId == "") {
      throw Exception("Invalid replay");
    }

    quit();
    return;
  }

  if (v_xmArgs.isOptLevelID()) {
    m_PlaySpecificLevelId = v_xmArgs.getOpt_levelID_id();
  }
  if (v_xmArgs.isOptLevelFile()) {
    m_PlaySpecificLevelFile = v_xmArgs.getOpt_levelFile_file();
  }
  if (v_xmArgs.isOptReplay()) {
    m_PlaySpecificReplay = v_xmArgs.getOpt_replay_file();
  }
  if (v_xmArgs.isOptDemo()) {
    try {
      m_PlaySpecificReplay = loadDemoReplay(v_xmArgs.getOpt_demo_file());
    } catch (Exception &e) {
      LogError("Unable to load the demo file");
    }
  }

  /* Should we clean the level cache? (can also be done when disabled) */
  if (v_xmArgs.isOptCleanCache()) {
    LevelsManager::cleanCache();
  }

  /* Should we clean non www levels ? */
  if (v_xmArgs.isOptCleanNoWWWLevels()) {
    pDb->levels_cleanNoWWWLevels();
  }

  /* -listlevels? */
  if (v_xmArgs.isOptListLevels()) {
    LevelsManager::instance()->printLevelsList(xmDatabase::instance("main"));
    quit();
    return;
  }

  /* -updateLevels */
  if (v_xmArgs.isOptUpdateLevelsOnly()) {
    UpgradeLevelsThread *m_upgradeLevelsThread;
    bool v_currentVerbosity = Logger::isVerbose();
    int v_res;

    m_upgradeLevelsThread =
      new UpgradeLevelsThread(XMSession::instance()->theme(), false, true);

    // force verbosity while updating levels
    Logger::setVerbose(true);
    v_res = m_upgradeLevelsThread->runInMain();
    if (v_res != 0 && v_res != 2 /* 2 for no level to update */) {
      LogError("Not able to update levels");
    }
    Logger::setVerbose(v_currentVerbosity);

    quit();
    return;
  }

  /* requires graphics now */
  if (v_useGraphics == false && v_xmArgs.isOptServerOnly() == false) {
    quit();
    return;
  }

  if (v_xmArgs.isOptServerOnly() == false) {
    _UpdateLoadingScreen();

    /* Find all files in the textures dir and load them */
    UITexture::setApp(this);
    UIWindow::setDrawLib(getDrawLib());
  }

  // init physics
  // if(dInitODE2(0) == 0) { /* erreur */} ; // ode 0.10
  dInitODE();

// check ode configuration
#if HAVEDGETCONFIGURATION == 1
  LogInfo("Ode config: %s", dGetConfiguration());
#ifdef dSINGLE
  LogInfo("Ode config: X-Moto compiled with dSINGLE defined");
  if (dCheckConfiguration("ODE_single_precision") != 1) {
    throw Exception(
      "XMoto compiled with ode single precision, but libode library wasn't.");
  }
#endif
#ifdef dDOUBLE
  LogInfo("Ode config: X-Moto compiled with dDOUBLE defined");
  if (dCheckConfiguration("ODE_double_precision") != 1) {
    throw Exception("XMoto compiled with ode as double precision, but libode "
                    "library wasn't.");
  }
#endif
#if !defined(dSINGLE) && !defined(dDOUBLE)
  LogInfo("Ode config: X-Moto compiled with dSINGLE and dDOUBLE undefined, "
          "dSINGLE is used by default");
  if (dCheckConfiguration("ODE_single_precision") != 1) {
    throw Exception(
      "XMoto compiled with ode single precision, but libode library wasn't.");
  }
#endif
#else
  // old ode version
  LogWarning("You've an old ode library, xmoto is not able to check whether "
             "it's compatible or not");
#endif
  m_isODEInitialized = true;

  LogInfo("Lua version: %s", LUA_RELEASE);

  /* load packs */
  LevelsManager::checkPrerequires();

  // don't need to create packs in server mode
  if (v_xmArgs.isOptServerOnly() == false) {
    LevelsManager::instance()->makePacks(XMSession::instance()->profile(),
                                         XMSession::instance()->idRoom(0),
                                         XMSession::instance()->debug(),
                                         XMSession::instance()->adminMode(),
                                         xmDatabase::instance("main"));
  }

  /* Update stats */
  if (v_xmArgs.isOptServerOnly() == false) {
    if (XMSession::instance()->profile() != "") {
      pDb->stats_xmotoStarted(XMSession::instance()->sitekey(),
                              XMSession::instance()->profile());
    }
  }

  // reset the loading screen
  if (DrawLib::isInitialized() == false) {
    _UpdateLoadingShell(); // no more loading screen
  }

  if (v_xmArgs.isOptServerOnly()) {
    try {
      // start the server
      m_standAloneServer = NetServer::instance();
      NetServer::instance()->setStandAloneOptions();
      NetServer::instance()->start(
        false,
        v_xmArgs.isOptServerPort() ? v_xmArgs.getOptServerPort_value()
                                   : XMSession::instance()->serverPort(),
        v_xmArgs.isOptServerAdminPassword()
          ? v_xmArgs.getOptServerAdminPassword_value()
          : "" /* else, no password */);
    } catch (Exception &e) {
      LogError((std::string("Exception: ") + e.getMsg()).c_str());
    }

    quit();
    return;

  } else {
    /* try to not run sql at the same time you enter in the main menu (a thread
     * to compute packs is run (concurrency)) */

    /* What to do? */
    if (m_PlaySpecificLevelFile != "") {
      try {
        m_PlaySpecificLevelId = LevelsManager::instance()->addExternalLevel(
          m_PlaySpecificLevelFile,
          xmDatabase::instance("main"),
          v_xmArgs.isOptServerOnly());
        if (m_PlaySpecificLevelId.empty()) {
          m_PlaySpecificLevelId = LevelsManager::instance()->LevelByFileName(
            m_PlaySpecificLevelFile, xmDatabase::instance("main"));
        }
      } catch (Exception &e) {
        m_PlaySpecificLevelId = m_PlaySpecificLevelFile;
      }
    }
    if ((m_PlaySpecificLevelId != "")) {
      /* ======= PLAY SPECIFIC LEVEL ======= */
      playLevel(m_PlaySpecificLevelId);
    } else if (m_PlaySpecificReplay != "") {
      /* ======= PLAY SPECIFIC REPLAY ======= */
      playReplay(m_PlaySpecificReplay);
    } else {
      /* display what must be displayed */
      StateManager::instance()->pushState(new StateMainMenu());
      if (XMSession::instance()->clientGhostMode() == false &&
          NetClient::instance()->isConnected()) {
        StateManager::instance()->pushState(new StateWaitServerInstructions());
      }
    }

    /* display error information to the player */
    if (v_missingFont) {
      SysMessage::instance()->displayError(
        GAMETEXT_TTF_MISSING + std::string("\n") + v_missingFontError);
    }
  }

  LogInfo("UserInit ended at %.3f", GameApp::getXMTime());
}

void GameApp::manageEvent(SDL_Event *Event) {
  static int nLastMouseClickX = -100, nLastMouseClickY = -100;
  static int nLastMouseClickButton = -100;
  static float fLastMouseClickTime = 0.0f;
  int nX, nY;
  std::string utf8Char;

  if (Event->type == SDL_KEYDOWN || Event->type == SDL_KEYUP) {
    /* ignore modifier-only key presses */
    switch (Event->key.keysym.sym) {
      case SDLK_RSHIFT:
      case SDLK_LSHIFT:
      case SDLK_RCTRL:
      case SDLK_LCTRL:
      case SDLK_RALT:
      case SDLK_LALT:
      case SDLK_RGUI:
      case SDLK_LGUI:
        return;
    }
  }

  SDL_StartTextInput();

  switch (Event->type) {
    case SDL_TEXTINPUT:
      utf8Char = Event->text.text;

      StateManager::instance()->xmKey(INPUT_TEXT,
                                      XMKey(0, (SDL_Keymod)0, utf8Char));
      break;

    case SDL_KEYDOWN:
      utf8Char = unicode2utf8(Event->key.keysym.sym);

      StateManager::instance()->xmKey(INPUT_DOWN,
                                      XMKey(Event->key.keysym.sym,
                                            (SDL_Keymod)Event->key.keysym.mod,
                                            utf8Char,
                                            Event->key.repeat));
      break;

    case SDL_KEYUP:
      utf8Char = unicode2utf8(Event->key.keysym.sym);

      StateManager::instance()->xmKey(INPUT_UP,
                                      XMKey(Event->key.keysym.sym,
                                            (SDL_Keymod)Event->key.keysym.mod,
                                            utf8Char));
      break;

    case SDL_QUIT:
      /* Force quit */
      quit();
      break;

    case SDL_MOUSEBUTTONDOWN:
      /* Is this a double click? */
      getMousePos(&nX, &nY);
      if (nX == nLastMouseClickX && nY == nLastMouseClickY &&
          nLastMouseClickButton == Event->button.button &&
          (getXMTime() - fLastMouseClickTime) < MOUSE_DBCLICK_TIME) {
        /* Pass double click */
        StateManager::instance()->xmKey(INPUT_DOWN,
                                        XMKey(Event->button.button, 1));
      } else {
        /* Pass ordinary click */
        StateManager::instance()->xmKey(INPUT_DOWN,
                                        XMKey(Event->button.button));
      }
      fLastMouseClickTime = getXMTime();
      nLastMouseClickX = nX;
      nLastMouseClickY = nY;
      nLastMouseClickButton = Event->button.button;

      break;
    case SDL_MOUSEBUTTONUP:
      StateManager::instance()->xmKey(INPUT_UP, XMKey(Event->button.button));
      break;
    case SDL_MOUSEWHEEL:
      StateManager::instance()->xmKey(INPUT_SCROLL, XMKey(*Event));
      break;
    case SDL_CONTROLLERAXISMOTION:
      StateManager::instance()->xmKey(
        Input::instance()->joystickAxisSens(Event->caxis.value),
        XMKey(Input::instance()->getJoyById(Event->cbutton.which),
              Event->caxis.axis,
              Event->caxis.value));
      break;
    case SDL_USEREVENT: {
      if (Event->user.code == SDL_CONTROLLERAXISMOTION) {
        const auto &event = *(static_cast<JoyAxisEvent *>(Event->user.data1));
        StateMenu *state =
          dynamic_cast<StateMenu *>(StateManager::instance()->getTopState());
        if (state) {
          state->getGUI()->joystickAxisMotion(event);
        }
      }
      break;
    }

    case SDL_CONTROLLERBUTTONDOWN: {
      case SDL_CONTROLLERBUTTONUP:
        InputEventType type = Input::eventState(Event->type);
        XMKey key = XMKey(Input::instance()->getJoyById(Event->cbutton.which),
                          Event->cbutton.button);
        StateManager::instance()->xmKey(type, key);
        break;
    }

    case SDL_WINDOWEVENT: {
      bool hasFocus = false;

      switch (Event->window.event) {
        case SDL_WINDOWEVENT_ENTER:
        case SDL_WINDOWEVENT_LEAVE: {
          switch (Event->window.event) {
            case SDL_WINDOWEVENT_ENTER:
              hasFocus = true;
              break;
            case SDL_WINDOWEVENT_LEAVE:
              hasFocus = false;
              break;
          }

          if (!m_hasKeyboardFocus)
            StateManager::instance()->changeFocus(hasFocus);

          m_hasMouseFocus = hasFocus;
          break;
        }
        case SDL_WINDOWEVENT_FOCUS_GAINED: /* fall through */
        case SDL_WINDOWEVENT_FOCUS_LOST: {
          switch (Event->window.event) {
            case SDL_WINDOWEVENT_FOCUS_GAINED:
              hasFocus = true;
              break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
              hasFocus = false;
              break;
          }

          if (hasFocus) {
            // Pending keydown events need to be flushed after gaining focus
            // because SDL2 will send them for keys that were pressed outside
            // the game (e.g. when alt-tabbing in)
            SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYDOWN);
          } else {
            /*
             * With SDL2, input events come in in the opposite order from SDL1.2
             * (the order used to be: "focus lost" -> "key released").
             * We need to invalidate the keys here so they don't persist after
             * focus is lost
             */
            StatePlayingLocal *state = dynamic_cast<StatePlayingLocal *>(
              StateManager::instance()->getTopState());
            if (state)
              state->dealWithActivedKeys();
          }

          if (!m_hasMouseFocus)
            StateManager::instance()->changeFocus(hasFocus);

          m_hasKeyboardFocus = hasFocus;
          break;
        }
        case SDL_WINDOWEVENT_EXPOSED:
          StateManager::instance()->setInvalidated(true);
          break;
        case SDL_WINDOWEVENT_SHOWN:
          StateManager::instance()->changeVisibility(true);
          m_isIconified = false;
          break;
        case SDL_WINDOWEVENT_HIDDEN:
          StateManager::instance()->changeVisibility(false);
          m_isIconified = true;
          break;
      }

      break;
    }
    case SDL_DROPFILE: {
      char *file = Event->drop.file;
      std::string path(file);
      SDL_free(file);
      StateManager::instance()->fileDrop(path);

      break;
    }
  }
}

void GameApp::run_loop() {
  SDL_Event Event;
  int v_frameTime = 0;

  while (m_bQuit == false) {
    /* strategie :
       no network :
         update // done on the same computer as rendering, so, if update is
       late, rendering is late and so on
         render
         wait(so that 0.01 seconds by loop happend)

       network:
         update
         render // skip rendering if late to let others tasks happend (rendering
       is the biggest cost)
         update network (wait for network if nothing to do so that 0.01 seconds
       by loop happend or skip if no time)
    */

    // computer the delta
    if (NetClient::instance()->isConnected()) {
      v_frameTime = 1000 / StateManager::instance()->getMaxFps();

      // update the delta time to the reality
      m_frameLate += (GameApp::getXMTimeInt() - m_lastFrameTimeStamp) -
                     v_frameTime; // update the delta

      // too much delta, reset the delta
      if (m_frameLate > 100 || m_frameLate < -100) {
        m_frameLate = 0;
      }
      m_lastFrameTimeStamp = GameApp::getXMTimeInt();
    }
    //

    // update the game

    /* Handle SDL events */
    SDL_PumpEvents();

    // wait on event if xmoto won't be update/rendered
    if (StateManager::instance()->needUpdateOrRender()) {
      while (SDL_PollEvent(&Event)) {
        manageEvent(&Event);
      }
    } else {
      if (SDL_WaitEvent(&Event) == 1) {
        manageEvent(&Event);
      }
      while (SDL_PollEvent(&Event)) {
        manageEvent(&Event);
      }
    }

    /* Update user app */
    // update sound
    Sound::update();

    // update game
    StateManager::instance()->update();

    // update graphics
    // skip rendering if too much late (network mode)
    if (NetClient::instance()->isConnected()) {
      if (m_frameLate < XM_MAX_FRAMELATE_TO_FORCE_NORENDERING ||
          m_loopWithoutRendering > XM_MAX_NB_LOOPS_WITH_NORENDERING) {
        StateManager::instance()->render();
        m_loopWithoutRendering = 0;
      } else {
        m_loopWithoutRendering++;
        // printf("skip rendering (%i)\n", m_loopWithoutRendering);
      }
    } else {
      StateManager::instance()->render();
    }

    // update network
    // skip network update if not the time
    if (NetClient::instance()->isConnected()) {
      int v_timeout = v_frameTime -
                      (GameApp::getXMTimeInt() - m_lastFrameTimeStamp) -
                      m_frameLate;

      if (m_loopWithoutNetwork > XM_MAX_NB_LOOPS_WITH_NONETWORK) {
        v_timeout = 1; // force a minimum network
        // printf("force networking\n");
      }

      // manage network
      if (v_timeout > 0 ||
          XMSession::instance()->timedemo()) { // only when you've time to do it
        NetClient::instance()->manageNetwork(v_timeout,
                                             xmDatabase::instance("main"));
        m_loopWithoutNetwork = 0;
      } else {
        m_loopWithoutNetwork++;
      }
    } else {
      /* pause system without having to wait for the net at the same time */
      if (XMSession::instance()->timedemo() == false) {
        GameApp::wait(m_lastFrameTimeStamp,
                      m_frameLate,
                      StateManager::instance()->getMaxFps());
      }
    }
  }
}

void GameApp::run_unload() {
  if (Logger::isInitialized()) {
    LogInfo("UserUnload started at %.3f", GameApp::getXMTime());
  }

  if (m_pWebHighscores != NULL) {
    delete m_pWebHighscores;
  }

  if (m_pWebLevels != NULL) {
    delete m_pWebLevels;
  }

  if (Input::instance() != NULL) {
    Input::instance()
      ->uninit(); // uinit the input, but you can still save the config
  }

  uninitNetwork();

  // detroy the demo
  if (m_xmdemo != NULL) {
    m_xmdemo->destroyFiles();
    delete m_xmdemo;
  }

  StateManager::destroy();

  if (Sound::isInitialized()) {
    Sound::uninit();
  }

  if (m_isODEInitialized == true) {
    dCloseODE(); // uninit ODE
  }

  SysMessage::destroy();

  if (Logger::isInitialized()) {
    LogDebug("UserUnload saveConfig at %.3f", GameApp::getXMTime());
  }
  auto dbInstance = xmDatabase::instance("main");
  if (drawLib != NULL &&
      dbInstance->isOpen()) { /* save config only if drawLib was initialized */
    XMSession::instance("file")->save(m_userConfig,
                                      xmDatabase::instance("main"));
    Input::instance()->saveConfig(
      m_userConfig, dbInstance, XMSession::instance("file")->profile());
    m_userConfig->saveFile();
  }

  if (Logger::isInitialized()) {
    LogDebug("UserUnload saveConfig ended at %.3f", GameApp::getXMTime());
  }

  if (drawLib != NULL) {
    drawLib->unInit();
  }

  Input::destroy();
  LevelsManager::destroy();
  GeomsManager::destroy();
  Theme::destroy();
  XMSession::destroy("live");
  XMSession::destroy("file");

  if (Logger::isInitialized()) {
    LogInfo("UserUnload ended at %.3f", GameApp::getXMTime());
  }

  /* Shutdown SDL */
  SDL_Quit();

  XMLDocument::clean();

  if (Logger::isInitialized()) {
    Logger::uninit();
  }

  Environment::uninit();

  if (XMFS::isInitialized()) {
    XMFS::uninit();
  }
}

void GameApp::wait(int &io_lastFrameTimeStamp,
                   int &io_frameLate,
                   int i_maxFps) {
  if (io_lastFrameTimeStamp < 0) {
    io_lastFrameTimeStamp = getXMTimeInt();
  }

  /* Does app want us to delay a bit after the frame? */
  int currentTimeStamp = getXMTimeInt();
  int currentFrameMinDuration = 1000 / i_maxFps;
  int lastFrameDuration = currentTimeStamp - io_lastFrameTimeStamp;
  // late from the lasts frame is not forget
  int delta = currentFrameMinDuration - (lastFrameDuration + io_frameLate);

  // if we have toooo much late (1/10 second), let reset delta
  int maxDelta = -100;
  if (delta < maxDelta) {
    delta = 0;
  }

  if (delta > 0) {
    // we're in advance
    // -> sleep
    int beforeSleep = getXMTimeInt();
    SDL_Delay(delta);
    int afterSleep = getXMTimeInt();
    int sleepTime = afterSleep - beforeSleep;

    // now that we have sleep, see if we don't have too much sleep
    if (sleepTime >= delta) {
      int tooMuchSleep = sleepTime - delta;
      io_frameLate = tooMuchSleep;
    }
  } else {
    // we're late
    // -> update late time
    io_frameLate = (-delta);
  }

  // the sleeping time is not included in the next frame time
  io_lastFrameTimeStamp = getXMTimeInt();
}

/*===========================================================================
Update loading screen
===========================================================================*/
void GameApp::_UpdateLoadingShell(const std::string &NextTask,
                                  int i_percentage) {
  char buf[XMSERVER_BUF]; // max line size allowed

  printf("\r%" XMSERVER_STRBUF "s", ""); // erase
  if (i_percentage >= 0) {
    snprintf(buf, XMSERVER_BUF, "%s (%i%%)", NextTask.c_str(), i_percentage);
  } else {
    snprintf(buf, XMSERVER_BUF, "%s", NextTask.c_str());
  }
  printf("\r%s", buf);
  fflush(stdout);
}

void GameApp::_UpdateLoadingScreen(const std::string &NextTask,
                                   int i_percentage) {
  FontManager *v_fm;
  FontGlyph *v_fg;
  int v_fh;
  RenderSurface v_screen(
    Vector2i(0, 0),
    Vector2i(getDrawLib()->getDispWidth(), getDrawLib()->getDispHeight()));

  getDrawLib()->clearGraphics();
  getDrawLib()->resetGraphics();

  v_fm = getDrawLib()->getFontBig();
  v_fg = v_fm->getGlyph(GAMETEXT_LOADING);
  v_fh = v_fg->realHeight();
  v_fm->printString(getDrawLib(),
                    v_fg,
                    v_screen.getDispWidth() / 2 - 256,
                    v_screen.getDispHeight() / 2 - 30,
                    MAKE_COLOR(255, 255, 255, 255));

  if (NextTask != "") {
    v_fm = getDrawLib()->getFontSmall();
    v_fg = v_fm->getGlyph(NextTask);
    v_fm->printString(getDrawLib(),
                      v_fg,
                      v_screen.getDispWidth() / 2 - 256,
                      v_screen.getDispHeight() / 2 - 30 + v_fh + 2,
                      MAKE_COLOR(255, 255, 255, 255));
  }

  if (i_percentage != -1) {
    getDrawLib()->drawBox(Vector2f(0, v_screen.getDispHeight() - 5),
                          Vector2f(((int)(v_screen.getDispWidth() *
                                          ((float)(i_percentage)) / 100.0)),
                                   v_screen.getDispHeight()),
                          0,
                          MAKE_COLOR(255, 255, 255, 255));
  }

  getDrawLib()->flushGraphics();
}

void GameApp::initNetwork(bool i_forceNoServerStarted,
                          bool i_forceNoClientStarted) {
  if (SDLNet_Init() == -1) {
    throw Exception(SDLNet_GetError());
  }

  // start server if gui
  if (i_forceNoServerStarted == false) {
    if (XMSession::instance()->serverStartAtStartup()) {
      NetServer::instance()->start(
        true,
        XMSession::instance()
          ->serverPort()); // in graphics mode, use the port of the options
    }
  }

  if (i_forceNoClientStarted == false) {
    if (XMSession::instance()->clientConnectAtStartup()) {
      /* special case : if xmoto is it's own server, you must wait that the
         server is started
         test ip is not a good solution, by the way, people starting the server
         should use it or lost less than 1 second at startup
       */
      if (XMSession::instance()->serverStartAtStartup()) {
        // wait that server is started
        unsigned int v_delay = 0;
        while (NetServer::instance()->acceptConnections() == false &&
               v_delay < 5000 /* wait maximum 5 seconds the server */) {
          LogInfo("Server still not accepting connections, wait (%.2f seconds)",
                  v_delay / 1000.0);
          SDL_Delay(100);
          v_delay += 100;
        }
      }

      try {
        NetClient::instance()->connect(
          XMSession::instance()->clientServerName(),
          XMSession::instance()->clientServerPort());
        // don't send in ghost mode to be compatible with old servers
        if (XMSession::instance()->clientGhostMode() == false) {
          NetClient::instance()->changeMode(
            XMSession::instance()->clientGhostMode() ? NETCLIENT_GHOST_MODE
                                                     : NETCLIENT_SLAVE_MODE);
        }
      } catch (Exception &e) {
        SysMessage::instance()->displayError(
          GAMETEXT_UNABLETOCONNECTONTHESERVER);
        LogError("Unable to connect to the server");
      }
    }
  }
}

void GameApp::uninitNetwork() {
  // stop the client
  if (NetClient::instance()->isConnected()) {
    NetClient::instance()->disconnect();
  }
  NetClient::destroy();

  if (NetServer::instance()->isStarted()) {
    NetServer::instance()->stop();
  }

  NetServer::destroy();

  if (Logger::isInitialized()) {
    NetAction::logStats();
    ActionReader::logStats();
  }

  SDLNet_Quit();
}
