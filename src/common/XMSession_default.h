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

#ifndef __XMCONF_DEFAULT_H__
#define __XMCONF_DEFAULT_H__

#include <string>

/* here you can change the default option values for profiles */
/*
  for some customizable values, you use a variable, not a define
*/

// max number of rooms ; set at compilation
#define ROOMS_NB_MAX 6

// database file
#define DATABASE_FILE "xm.db"

#define DEFAULT_CONTROLLERMODE "Keyboard"
#define DEFAULT_LANGUAGE ""
#define DEFAULT_THEME CONFIGURE_DEFAULT_THEME
#define DEFAULT_WEBROOM_ID "1"
#define DEFAULT_WEBCONFATINIT true
#define DEFAULT_PROFILE ""
#define DEFAULT_VERBOSE false
#define DEFAULT_RESOLUTION_WIDTH 800
#define DEFAULT_RESOLUTION_HEIGHT 600
#define DEFAULT_MAXRENDERFPS 50
#define DEFAULT_WINDOWED true
#define DEFAULT_USETHEMECURSOR true
#define DEFAULT_GLEXTS true
#define DEFAULT_GLVOBS true
#define DEFAULT_DRAWLIB "OPENGL"
#define DEFAULT_WWW true
#define DEFAULT_WWW_PASSWORD ""
#define DEFAULT_BENCHMARK false
#define DEFAULT_DEBUG false
#define DEFAULT_SQLTRACE false
#define DEFAULT_GDEBUG false
#define DEFAULT_TIMEDEMO false
#define DEFAULT_FPS false
#define DEFAULT_UGLY false
#define DEFAULT_UGLYOVER false
#define DEFAULT_NOLOG false
#define DEFAULT_TESTTHEME false
#define DEFAULT_GHOST_MYBEST true
#define DEFAULT_GHOST_THEBEST false
#define DEFAULT_GHOSTBESTREFROOM true
#define DEFAULT_GHOSTBESTOTHERROOMS false
#define DEFAULT_AUTOSAVEHIGHSCORESREPLAYS true
#define DEFAULT_DISABLEANIMATIONS false
#define DEFAULT_ENABLEGHOSTS true
#define DEFAULT_ENABLEENGINESOUND true
#define DEFAULT_SHOWENGINECOUNTER false
#define DEFAULT_SHOWMINIMAP true
#define DEFAULT_MULTISTOPWHENONEFINISHES true
#define DEFAULT_ENABLEMENUMUSIC true
#define DEFAULT_ENABLEGAMEMUSIC true
#define DEFAULT_ENABLEDEADANIMATION true
#define DEFAULT_MENUGRAPHICS GFX_HIGH
#define DEFAULT_GAMEGRAPHICS GFX_HIGH
#define DEFAULT_QUICKSTARTQUALITYMIN 1
#define DEFAULT_QUICKSTARTQUALITYMAX 5
#define DEFAULT_QUICKSTARTDIFFICULTYMIN 1
#define DEFAULT_QUICKSTARTDIFFICULTYMAX 5
#define DEFAULT_MULTINBPLAYERS 1
#define DEFAULT_MULTIGAMEMODE MULTI_MODE_TIME_ATTACK
#define DEFAULT_MULTISCENES true
#define DEFAULT_ENABLECONTEXTHELP true
#define DEFAULT_ENABLEAUDIO true
#define DEFAULT_AUDIOSAMPLERATE 44100
#define DEFAULT_AUDIOSAMPLEBITS 16
#define DEFAULT_AUDIOCHANNELS 2
#define DEFAULT_ENABLEAUDIOENGINE true
#define DEFAULT_CHECKNEWLEVELSATSTARTUP true
#define DEFAULT_CHECKNEWHIGHSCORESATSTARTUP true
#define DEFAULT_SHOWHIGHSCOREINGAME true
#define DEFAULT_SHOWNEXTMEDALINGAME false
#define DEFAULT_NBROOMSENABLED 1
#define DEFAULT_SHOWGHOSTTIMEDIFFERENCE true
#define DEFAULT_GHOSTMOTIONBLUR true
#define DEFAULT_SHOWGHOSTSINFOS true
#define DEFAULT_SHOWBIKERSARROWS true
#define DEFAULT_HIDEGHOSTS false
#define DEFAULT_REPLAYFRAMERATE 25.0
#define DEFAULT_STOREREPLAYS true
#define DEFAULT_ENABLEREPLAYINTERPOLATION true
#define DEFAULT_SCREENSHOTFORMAT "png"
#define DEFAULT_NOTIFYATINIT true
#define DEFAULT_MIRRORMODE false
#define DEFAULT_USECRAPPYPACK true
#define DEFAULT_PERMANENTCONSOLE false
#define DEFAULT_SHOWGAMEINFORMATIONINCONSOLE true
#define DEFAULT_CONSOLESIZE 5
#define DEFAULT_USECHILDRENCOMPLIANT false
#define DEFAULT_FORCECHILDRENCOMPLIANT false
#define DEFAULT_ENABLEVIDEORECORDING false
#define DEFAULT_VIDEORECORDINGSTARTTIME -1
#define DEFAULT_VIDEORECORDINGENDTIME -1
#define DEFAULT_HIDEPLAYINGINFORMATION false
#define DEFAULT_ENABLEINITZOOM true
#define DEFAULT_ENABLEACTIVEZOOM true
#define DEFAULT_ENABLETRAILCAM false
#define DEFAULT_GHOSTTRAILRENDERING false
#define DEFAULT_ENABLEJOYSTICKS false
#define DEFAULT_BEATINGMODE false
#define DEFAULT_WEBFORMS true
#define DEFAULT_PROXY_PORT -1
#define DEFAULT_PROXY_TYPE ""
#define DEFAULT_PROXY_SERVER ""
#define DEFAULT_PROXY_AUTHUSER ""
#define DEFAULT_PROXY_AUTHPWD ""
#define DEFAULT_DBSYNCHRONIZEONQUIT false
#define DEFAULT_SERVERSTARTATSTARTUP false
#define DEFAULT_CLIENTCONNECTATSTARTUP false
#define DEFAULT_SERVERPORT 4130
#define DEFAULT_SERVERMAXCLIENTS 64
#define DEFAULT_CLIENTSERVERNAME "games.tuxfamily.org"
#define DEFAULT_CLIENTGHOSTMODE true
#define DEFAULT_CLIENTSERVERPORT DEFAULT_SERVERPORT
#define DEFAULT_CLIENTFRAMERATEUPLOAD 10
#define DEFAULT_MUSICONALLLEVELS true
#define DEFAULT_ADMINMODE false
#define DEFAULT_LOGRETENTIONCOUNT 10

class XMDefault {
public:
  static std::string DefaultTheme;
};

#endif
