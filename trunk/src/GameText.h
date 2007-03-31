/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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

#ifndef __GAMETEXT_H__
#define __GAMETEXT_H__

#include "BuildConfig.h"

#include "Locales.h"

/*=============================================================================
Central place to keep many in-game text strings
=============================================================================*/

#define GAMETEXT_11KHZ                _("11 kHz")
#define GAMETEXT_16BIT                _("16-bit")
#define GAMETEXT_16BPP                _("16 bits per pixel")
#define GAMETEXT_22KHZ                _("22 kHz")
#define GAMETEXT_32BPP                _("32 bits per pixel")
#define GAMETEXT_44KHZ                _("44 kHz")
#define GAMETEXT_8BIT                 _("8-bit")
#define GAMETEXT_ABORT                _("Abort Playing")
#define GAMETEXT_ADDTOFAVORITE       _("Add to favorite")
#define GAMETEXT_ACTION               _("Action")
#define GAMETEXT_ALL                  _("All (on this computer)")
#define GAMETEXT_ALL_LEVELS           _("All levels")
#define GAMETEXT_ALLOWINTERNETCONN    _("Do you want to allow X-Moto connecting\n" \
                                        "to the Internet to look for more levels\n" \
                                        "and best times of other players?")
#define GAMETEXT_ALLRECORDS           _("All (on this computer)")
#define GAMETEXT_ALREADYUSED          _("Already used!")
#define GAMETEXT_AND                  _("and")
#define GAMETEXT_AUDIO                _("Audio")
#define GAMETEXT_AUTHOR               _("Author")
#define GAMETEXT_AUTOSAVEREPLAYS      _("Automatic saving of replays")
#define GAMETEXT_AUTOZOOM             _("Auto zoom")
#define GAMETEXT_BESTTIMES            _("Best Times")
#define GAMETEXT_BESTPLAYER           _("Best player")
#define GAMETEXT_BRAKE                _("Brake")
#define GAMETEXT_BRAKE2               _("Brake 2")
#define GAMETEXT_BUILTINLEVELS        _("Built-In Levels")
#define GAMETEXT_BY                   _("by")
#define GAMETEXT_CAMERAMOVEXDOWN      _("Move Camera to left")
#define GAMETEXT_CAMERAMOVEXUP        _("Move Camera to right")
#define GAMETEXT_CAMERAMOVEYDOWN      _("Move Camera down")
#define GAMETEXT_CAMERAMOVEYUP        _("Move Camera up")
#define GAMETEXT_CANCEL               _("Cancel")
#define GAMETEXT_CHANGE               _("Change...")
#define GAMETEXT_CHANGEDIR            _("Change direction")
#define GAMETEXT_CHANGEDIR2           _("Change direction 2")
#define GAMETEXT_CHANGEKEY            _("Change Key...")
#define GAMETEXT_CHECKINGFORLEVELS    _("Checking for new or updated levels...")
#define GAMETEXT_CHOOSELEVEL          _("Choose Level")
#define GAMETEXT_CLOSE                _("Close")
#define GAMETEXT_CONFIGUREJOYSTICK    _("Configure Joystick...")
#define GAMETEXT_CONTROLS             _("Controls")
#define GAMETEXT_CREDITS              _("Lead Programming:Rasmus Neckelmann;" \
                                        ":Nicolas Adenis-Lamarre;" \
                                        ":Emmanuel Gorse;" \
                                        ":Kees Jongenburger;" \
                                        ":;" \
                                        "Additional Programming:Eric Piel;" \
                                        ":Christoph Sommer;" \
                                        ":Frederic Huard;" \
                                        ":Brice Goglin;" \
                                        ":;" \
                                        "Graphics:Rasmus Neckelmann;" \
                                        ":Jens Erler;" \
                                        ":;" \
                                        "Audio:Jens Erler;" \
                                        ":Brandon Ross;" \
                                        ":;" \
                                        "Built-in Levels:Rasmus Neckelmann;" \
                                        ":Pasi Kallinen;" \
                                        ":Germain Gagnerot;" \
                                        ":Clive Crous;" \
                                        ":Torben Green;" \
                                        ":Thomas Kjaerulff;" \
                                        ":;" \
                                        "MacOS X Support:Dave Vasilevsky;" \
                                        ":Maxime Biais;" \
                                        ":;" \
                                        "Website Programming:Nicolas Adenis-Lamarre;" \
                                        ":Nx;" \
                                        ":;" \
                                        "Website Administration:Nicolas Adenis-Lamarre;" \
                                        ":Valentin;" \
                                        ":Nx;" \
                                        ":Alrj;" \
                                        ":;" \
                                        "Website Localization:Felix Schl;" \
                                        ":Nicolas Adenis-Lamarre;" \
                                        ":Trullo;" \
                                        ":Afaland;" \
                                        ":Jj;" \
                                        ":Vertigo;" \
                                        ":Pol Vinogradov;" \
                                        ":;" \
                                        "Forum:Valentin;" \
                                        ":;" \
                                        "Main testers:Jose Alberto Coelho;" \
                                        ":Frederic Huard;" \
                                        ":;" \
                                        "Thanks to:SnowBear (for KDE/gnome integration);" \
                                        ":Jonathan Sieber (for german translation);"\
                                        ":Samuel Mimram (for Debian packaging);" \
                                        ":Dark (for Gentoo packaging);" \
                                        ":Olivier Blin (for Mandriva packaging);" \
                                        ":Balazs Rozsa (for across/elma);" \
                                        ":Yves Berquin (for making tinyxml);" \
                                        ":L. Peter Deutsch (for the MD5 code);" \
                                        ":Petter Reinholdtsen;" \
                                        ":Jes Vestervang (for providing web space);" \
                                        ":Kenneth (for being such a little girl);"\
                                        ":Everyone who have made levels;" \
                                        ":People who have provided bug reports;" \
                                        ":Anyone who has helped in any way;")                                        
#define GAMETEXT_CREDITSBUTTON        _("Credits")
#define GAMETEXT_CURPLAYER            _("Player")
#define GAMETEXT_DATE                 _("Date")
#define GAMETEXT_DEATHANIM            _("Death Animation")
#define GAMETEXT_DEFAULTS             _("Defaults")
#define GAMETEXT_DELETEPLAYERMESSAGE  _("Do you really want to delete player?")
#define GAMETEXT_DELETEREPLAYMESSAGE  _("Do you really want to delete replay?")
#define GAMETEXT_DELETE               _("Delete")
#define GAMETEXT_DELETEFROMFAVORITE  _("Delete from favorite")
#define GAMETEXT_DELETEPROFILE        _("Delete")
#define GAMETEXT_DESCRIPTION          _("Description")
#define GAMETEXT_DISPLAYGHOSTTIMEDIFF _("Display Ghost time difference")
#define GAMETEXT_DISPLAYGHOSTINFO     _("Display Ghost Information")
#define GAMETEXT_DIRECTCONN           _("Direct Connection")
#define GAMETEXT_DLGHOST              _("Downloading the ghost...")
#define GAMETEXT_DLHIGHSCORE          _("Downloading the highscore...")
#define GAMETEXT_DLHIGHSCORES         _("Downloading high-scores...")
#define GAMETEXT_DLLEVELS             _("Downloading extra levels...\nPress ESC to abort.")
#define GAMETEXT_DLLEVELSCHECK        _("Checking for new levels...")
#define GAMETEXT_DLROOMSLISTCHECK     _("Checking for existing rooms...")
#define GAMETEXT_DLTHEME              _("Downloading theme data required by new levels...\nPress ESC to abort.")
#define GAMETEXT_DLTHEMESLISTCHECK    _("Checking for new themes...")
#define GAMETEXT_DOWNLOADLEVELS       _("Get More Levels!")
#define GAMETEXT_DRIVE                _("Drive")
#define GAMETEXT_DRIVE2               _("Drive 2")
#define GAMETEXT_ENABLEAUDIO          _("Enable Audio")
#define GAMETEXT_ENABLECHECKNEWLEVELSATSTARTUP  _("Check new levels at startup")
#define GAMETEXT_ENABLECHECKHIGHSCORESATSTARTUP _("Check new highscores at startup")
#define GAMETEXT_ENABLEAUTOUPLOADREPLAY _("Automatic highscores upload")
#define GAMETEXT_ENABLECONTEXTHELP    _("Enable Context Help")
#define GAMETEXT_ENABLEENGINESOUND    _("Enable Engine Sound")
#define GAMETEXT_ENABLEGHOST          _("Enable Ghost")
#define GAMETEXT_ENABLEINGAMEWORLDRECORD _("Show World Record in-game")
#define GAMETEXT_ENABLEMUSIC          _("Enable Music")
#define GAMETEXT_ENABLEWEBHIGHSCORES  _("Enable WWW Access")
#define GAMETEXT_ENTERPLAYERNAME      _("Enter player name")
#define GAMETEXT_ENTERREPLAYNAME      _("Enter name of replay")
#define GAMETEXT_ERRORSINLEVEL        _("There are errors in the level, don't expect it to be playable!")
#define GAMETEXT_EXTERNALLEVELS       _("External Levels")  
#define GAMETEXT_FAILEDCHECKLEVELS    _("Failed to check for levels.\nCheck your Internet connection!")
#define GAMETEXT_FAILEDDLHIGHSCORES   _("Failed to download high-scores.\nCheck your Internet connection!")
#define GAMETEXT_FAILEDDLLEVELS       _("Failed to download levels.\nCheck your Internet connection!")
#define GAMETEXT_FAILEDDLREPLAY       _("Failed to download the replay.\nCheck your Internet connection!")
#define GAMETEXT_FAILEDDLROOMSLIST    _("Failed to download the rooms list.\nCheck your Internet connection!")
#define GAMETEXT_FAILEDGETSELECTEDTHEME _("Failed to get the selected theme\nCheck your Internet connection!")
#define GAMETEXT_FAILEDTOINITLEVEL    _("Failed to initialize level!")
#define GAMETEXT_FAILEDTOLOADREPLAY   _("Failed to load replay!")
#define GAMETEXT_FAILEDTOSAVEREPLAY   _("Failed to save replay!\nMaybe you should try with another name?")
#define GAMETEXT_FAILEDUPDATETHEMESLIST _("Failed to update the theme list\nCheck your Internet connection!")
#define GAMETEXT_FILE                 _("File")
#define GAMETEXT_FILTER               _("Filter")
#define GAMETEXT_FINISH               _("Finished!")
#define GAMETEXT_FINISHTIME           _("Finish Time")
#define GAMETEXT_FLIPLEFT             _("Flip left")
#define GAMETEXT_FLIPLEFT2            _("Flip left 2")
#define GAMETEXT_FLIPRIGHT            _("Flip right")
#define GAMETEXT_FLIPRIGHT2           _("Flip right 2")
#define GAMETEXT_GAMEGFX              _("Game Graphics")
#define GAMETEXT_GENERAL              _("General")
#define GAMETEXT_GENERALINFO          _("General Info")
#define GAMETEXT_GETSELECTEDTHEME     _("Get this theme")
#define GAMETEXT_GHOST_LOCAL          _("Local best")
#define GAMETEXT_GHOST_BEST           _("Your best")
#define GAMETEXT_GHOST_STRATEGIES_TYPE  _("Type")
#define GAMETEXT_GHOST_STRATEGY_BESTOFROOM _("The highscore of the room")
#define GAMETEXT_GHOST_STRATEGY_MYBEST  _("The best of my replays (on this computer)")
#define GAMETEXT_GHOST_STRATEGY_THEBEST _("The best of the replays (on this computer)")
#define GAMETEXT_GHOSTOF              _("Ghost of")
#define GAMETEXT_GHOSTTAB             _("Ghost")
#define GAMETEXT_HELP                 _("Help")


#define GAMETEXT_HELPTEXT(accelerate_KEY, brake_KEY, rotate_counter_clockwise_KEY, rotate_clockwise_KEY, change_direction_KEY) \
std::string(_("You control your bike using the keyboard")) + ":\n" \
"  " + accelerate_KEY + " - " + _("Accelerates") + "\n" \
"  " + brake_KEY + " - " + std::string(_("Brakes")) + "\n" \
"  " + rotate_counter_clockwise_KEY + " - " + std::string(_("Rotates it counter-clockwise")) + "\n" \
"  " + rotate_clockwise_KEY + " - " + std::string(_("Rotates it clockwise")) + "\n" \
"  " + change_direction_KEY + " - " + std::string(_("Turns around and drives in the other direction")) + "\n" \
"  Esc - " + std::string(_("Pause")) + "\n" \
"  Return - " + std::string(_("Restart the level")) + "\n" \
"  F12 - " + std::string(_("Take a screenshot")) + "\n" \
"  F5 - " + std::string(_("Reload levels from Levels dir (if you create a new level)")) + "\n" \
"\n" \
 + std::string(_("Find all the strawberries and touch the flower to finish\nthe level.")) + "\n" \
 + std::string(_("Read the README file or check out the website at\nhttp://xmoto.tuxfamily.org for more information."))

#define GAMETEXT_HIGH                 _("High")
#define GAMETEXT_INDEX_CREATION       _("It seems that it's the first time you load this xmoto version.\nLevels index creation. It can take some minutes.")
#define GAMETEXT_INETCONF             _("Internet Configuration")
#define GAMETEXT_INITINPUT            _("Initializing input system...")
#define GAMETEXT_INITMENUS            _("Initializing menus...")
#define GAMETEXT_INITRENDERER         _("Initializing renderer...")
#define GAMETEXT_INITTEXT             _("Initializing text renderer...")
#define GAMETEXT_INITZOOM             _("View Level Initially")
#define GAMETEXT_JOYSTICK             _("Joystick")
#define GAMETEXT_JUSTDEAD             _("Oops!")
#define GAMETEXT_JUSTDEAD_RESTART     _("Press ENTER to try again")
#define GAMETEXT_JUSTDEAD_DISPLAYMENU _("Press ESC to display the menu")
#define GAMETEXT_KEY                  _("Key")
#define GAMETEXT_KEYBOARD             _("Keyboard")
#define GAMETEXT_LEVEL                _("Level")
#define GAMETEXT_LEVELINFO            _("Level Info...")
#define GAMETEXT_LEVELNAME            _("Level Name")
#define GAMETEXT_LEVELNOTFOUND        _("Level '%s' not found!")
#define GAMETEXT_LEVELPACK            _("Level Pack")
#define GAMETEXT_LEVELPACKS           _("Level Packs")
#define GAMETEXT_LEVELREQUIREDBYREPLAY _("Level '%s' required by replay!")
#define GAMETEXT_LEVELS               _("Levels")
#define GAMETEXT_LISTALL              _("List All")
#define GAMETEXT_LOADINGLEVELS        _("Loading levels...")
#define GAMETEXT_LOADINGMENUGRAPHICS  _("Loading menu graphics...")
#define GAMETEXT_LOADINGREPLAYS       _("Loading replays...")
#define GAMETEXT_LOADINGSOUNDS        _("Loading sounds...")
#define GAMETEXT_LOADINGTEXTURES      _("Loading textures...")
#define GAMETEXT_LOGIN                _("Login")
#define GAMETEXT_LOW                  _("Low")
#define GAMETEXT_MEDIUM               _("Medium")
#define GAMETEXT_MENUGFX              _("Menu Graphics")
#define GAMETEXT_MISSINGTEXTURES      _("Level references unknown textures, it could be unplayable!")
#define GAMETEXT_MONO                 _("Mono")
#define GAMETEXT_MOSTPLAYEDLEVELSFOLLOW _("Following are your most played levels")
#define GAMETEXT_MOTIONBLURGHOST      _("Motion blur ghost")
#define GAMETEXT_MULTI                _("Multi")
#define GAMETEXT_NAME                 _("Name")
#define GAMETEXT_NB_PLAYERS           _("Number of players")
#define GAMETEXT_NEWERXMOTOREQUIRED   _("X-Moto %s or newer required to load level")
#define GAMETEXT_NEWHIGHSCORE         _("New highscore!")
#define GAMETEXT_NEWHIGHSCOREPERSONAL _("New personal highscore!")
#define GAMETEXT_NEWLEVELAVAIL        _("%d new or updated level available. Download now?")
#define GAMETEXT_NEWLEVELS            _("New Levels")
#define GAMETEXT_NEWLEVELS_AVAIBLE    _("New levels available!")
#define GAMETEXT_NEWLEVELSAVAIL       _("%d new or updated levels available. Download now?")
#define GAMETEXT_NEWPROFILE           _("New Profile...")
#define GAMETEXT_NO                   _("No")
#define GAMETEXT_NONEWLEVELS          _("No new or updated levels available.\n\nTry again another time.")
#define GAMETEXT_NONEXTLEVEL          _("No level following this one, sorry.")
#define GAMETEXT_NOSTATS              _("No statistics for this profile.")
#define GAMETEXT_NOTFINISHED          _("Not finished")
#define GAMETEXT_NOTIFYATINIT         _("Important note!\n" \
                                      "\n" \
                                      "This is an in development version of X-Moto!\n" \
                                      "All kinds of feedback are highly appreciated, so the game\n" \
                                      "can get better.\n" \
                                      "Mail bugs, ideas, comments, feature requests, hatemail, etc\n" \
                                      "to nicolas at adenis-lamarre dot fr\n" \
                                      "\n" \
                                      "Also visit http://xmoto.tuxfamily.org to make sure you've\n" \
                                      "got the latest version.")
#define GAMETEXT_NUMLEVELS            _("# Levels")
#define GAMETEXT_OK                   _("OK")  
#define GAMETEXT_OPEN                 _("Open")
#define GAMETEXT_OPTIONS              _("Options")
#define GAMETEXT_OPTIONSREQURERESTART _("Some options will not take effect before next restart!")
#define GAMETEXT_PACK_AUTHORS 	      _("By author")
#define GAMETEXT_PACK_LAST 	      _("Last")
#define GAMETEXT_PACK_ROOM 	      _("Room")
#define GAMETEXT_PACK_SPECIAL 	      _("Special")
#define GAMETEXT_PACK_STANDARD 	      _("Standard")
#define GAMETEXT_PACK_WEBVOTES 	      _("Web votes")
#define GAMETEXT_PACK_STATS           _("Statistics")
#define GAMETEXT_PAUSE                _("Pause")
#define GAMETEXT_PASSWORD             _("Password")
#define GAMETEXT_PERSONAL             _("Personal")
#define GAMETEXT_PERSONALRECORDS      _("Personal")
#define GAMETEXT_PLAYER               _("Player")
#define GAMETEXT_PLAYERS              _("Players")
#define GAMETEXT_PLAYERPROFILE        _("Player Profile")
#define GAMETEXT_PLAYERPROFILES       _("Player Profiles")
#define GAMETEXT_PLAYNEXT             _("Play Next Level")
#define GAMETEXT_PORT                 _("Port")
#define GAMETEXT_PRESSANYKEYTO        _("Press key you want to '%s' or ESC to cancel...")
#define GAMETEXT_PROXYSERVER          _("Proxy Server")
#define GAMETEXT_PROXYCONFIG          _("Configure Proxy...")
#define GAMETEXT_QUIT                 _("Quit Game")
#define GAMETEXT_QUITMESSAGE          _("Do you really want to quit?")
#define GAMETEXT_RANDOMIZE            _("Randomize")
#define GAMETEXT_RELOADINGLEVELS      _("Reloading levels...")
#define GAMETEXT_RELOADINGREPLAYS     _("Reloading replays...")
#define GAMETEXT_REPLAY               _("Replay")
#define GAMETEXT_REPLAYHELPTEXT(current_speed) _("Stop[esc] ||[space]  << >>[left/right keys]  < >[up/down keys]   Speed:") + current_speed + "x"
#define GAMETEXT_REPLAYHELPTEXTNOREWIND(current_speed) _("Stop[esc] ||[space] >>[right key] < >[up/down keys]   Speed:") + current_speed + "x"
  // + "a" + "X"
#define GAMETEXT_REPLAYNOTFOUND       _("The replay can't be played!")
#define GAMETEXT_REPLAYS              _("Replays")
#define GAMETEXT_RESTART              _("Restart This Level")
#define GAMETEXT_RESUME               _("Resume Playing")
#define GAMETEXT_ROOM                 _("Room")
#define GAMETEXT_RUNWINDOWED          _("Run Windowed")
#define GAMETEXT_SAVE                 _("Save")
#define GAMETEXT_SAVE_AS              _("Saved as")
#define GAMETEXT_SAVEREPLAY           _("Save Replay")
#define GAMETEXT_SCREENRES            _("Screen Resolution")
#define GAMETEXT_SHOW                 _("Show")
#define GAMETEXT_SHOWENGINECOUNTER    _("Speedometer")
#define GAMETEXT_SHOWINFO             _("Info...")
#define GAMETEXT_SHOWMINIMAP          _("Show Mini Map")
#define GAMETEXT_SCRIPTED             _("Scripted")
#define GAMETEXT_STARTLEVEL           _("Play!")
#define GAMETEXT_STATISTICS           _("Statistics")
#define GAMETEXT_STATS                _("STATS")
#define GAMETEXT_STEREO               _("Stereo")
#define GAMETEXT_THEMES               _("Theme")
#define GAMETEXT_THEMEHOSTED          _("Available")
#define GAMETEXT_THEMENOTHOSTED       _("To download")
#define GAMETEXT_THEMEREQUIREUPDATE   _("To be updated")
#define GAMETEXT_THEMEUPTODATE        _("The theme is now up to date")
#define GAMETEXT_TIME                 _("Time")
#define GAMETEXT_TRYAGAIN             _("Try This Level Again")
#define GAMETEXT_TUTORIAL             _("Tutorial")
#define GAMETEXT_UNKNOWNLEVEL         _("Unknown")
#define GAMETEXT_UNPACKED_LEVELS_PACK _("Unpacked levels")
#define GAMETEXT_UNUPDATABLETHEMEONWEB _("Can't update this theme !\nThe theme is not avaible on the web\nor your theme list is not up to date")
#define GAMETEXT_UPDATE               _("Update")
#define GAMETEXT_UPDATEHIGHSCORES     _("Check WWW")
#define GAMETEXT_UPDATEROOMSSLIST     _("Update the rooms list")
#define GAMETEXT_UPDATETHEMESLIST     _("Update the theme list")
#define GAMETEXT_UPDATINGLEVELS       _("Updating level lists...")
#define GAMETEXT_UPLOAD_HIGHSCORE     _("Upload highscore")
#define GAMETEXT_UPLOAD_HIGHSCORE_ERROR _("An unexcepted error occured\nThe website has perhaps some troubles")
#define GAMETEXT_UPLOAD_HIGHSCORE_WEB_WARNING_BEFORE _("Oh no !")
#define GAMETEXT_UPLOADING_HIGHSCORE  _("Uploading the highscore...")
#define GAMETEXT_USEENVVARS           _("Use Environment Vars")
#define GAMETEXT_USEPROFILE           _("Use Profile")
#define GAMETEXT_USINGHTTPPROXY       _("Using HTTP Proxy")
#define GAMETEXT_USINGSOCKS4PROXY     _("Using SOCKS4 Proxy")
#define GAMETEXT_USINGSOCKS5PROXY     _("Using SOCKS5 Proxy")
#define GAMETEXT_VIDEO                _("Video")
#define GAMETEXT_VIEW                 _("View")
#define GAMETEXT_VIEWTHEHIGHSCORE     _("View the highscore")
#define GAMETEXT_WANTTOUPDATELEVEL    _("Do you want to update level \"%s\"?")
#define GAMETEXT_WARNING              _("Warning")
#define GAMETEXT_WORLDRECORDNA        "--:--:--"
#define GAMETEXT_WWWMAINTAB           _("Main")
#define GAMETEXT_WWWTAB               _("WWW")
#define GAMETEXT_WWWROOMSTAB          _("Rooms")

#define GAMETEXT_XHOURS               _("%d hours") 
#define GAMETEXT_XMINUTES             _("%d minutes")
#define GAMETEXT_XMOTOGLOBALSTATS     _("(Stats since: %s)\n"                                         \
                                      "X-Moto started %d times; %d plays (%d different levels),\n"  \
                                      "%d deaths, %d finishes, and %d restarts.\nTime played: %s")
#define GAMETEXT_XMOTOLEVELSTATS      _("%d plays, %d deaths, %d finishes, and %d restarts")
#define GAMETEXT_XSECONDS             _("%d seconds")
#define GAMETEXT_YES                  _("Yes")
#define GAMETEXT_YES_FOR_ALL          _("Yes to all")
#define GAMETEXT_ZOOMIN  	      _("Zoom in")
#define GAMETEXT_ZOOMINIT 	      _("Reinitialize zoom")
#define GAMETEXT_ZOOMOUT 	      _("Zoom out")

/* Context help strings */
#define CONTEXTHELP_UPDATEHIGHSCORES _("Download the latest X-Moto world records and check for new levels")
#define CONTEXTHELP_AUTO_UPLOAD_REPLAY _("After beating a highscore upload your result to server")
#define CONTEXTHELP_PROXYCONFIG _("Configure how you are connected to the Internet")
#define CONTEXTHELP_PLAY_THIS_LEVEL_AGAIN _("Play this level again")
#define CONTEXTHELP_SAVE_A_REPLAY _("Save a replay for later viewing")
#define CONTEXTHELP_PLAY_NEXT_LEVEL _("Play next level")
#define CONTEXTHELP_BACK_TO_MAIN_MENU _("Back to the main menu")
#define CONTEXTHELP_QUIT_THE_GAME _("Quit the game")
#define CONTEXTHELP_BACK_TO_GAME _("Back to the game")
#define CONTEXTHELP_TRY_LEVEL_AGAIN_FROM_BEGINNING _("Try this level again from the beginning")
#define CONTEXTHELP_PLAY_NEXT_INSTEAD _("Play next level instead")
#define CONTEXTHELP_TRY_LEVEL_AGAIN _("Try this level again")
#define CONTEXTHELP_BUILT_IN_AND_EXTERNALS _("Built-in and stand-alone external levels")
#define CONTEXTHELP_LEVEL_PACKS _("Levels grouped together in level packs")
#define CONTEXTHELP_LEVELS _("Browse levels available to you")
#define CONTEXTHELP_REPLAY_LIST _("View list of recorded replays")
#define CONTEXTHELP_OPTIONS _("Configure X-Moto preferences")
#define CONTEXTHELP_HELP _("Instructions of how to play X-Moto")
#define CONTEXTHELP_CHANGE_PLAYER _("Change player profile")
#define CONTEXTHELP_TUTORIAL _("Play tutorial of how to play the game")
#define CONTEXTHELP_OFFICIAL_LEVELS _("Official built-in levels")
#define CONTEXTHELP_EXTERNAL_LEVELS _("Unofficial stand-alone external levels")
#define CONTEXTHELP_PLAY_SELECTED_LEVEL _("Play the selected level")
#define CONTEXTHELP_LEVEL_INFO _("View general information about the level, best times, and replays")
#define CONTEXTHELP_RUN_REPLAY _("Run the selected replay")
#define CONTEXTHELP_DELETE_REPLAY _("Permanently delete the selected replay")
#define CONTEXTHELP_ALL_REPLAYS _("Show replays of all players in list")
#define CONTEXTHELP_GENERAL_OPTIONS _("General X-Moto preferences")
#define CONTEXTHELP_VIDEO_OPTIONS _("Configure graphical options")
#define CONTEXTHELP_AUDIO_OPTIONS _("Configure audio options")
#define CONTEXTHELP_CONTROL_OPTIONS _("Configure control options")
#define CONTEXTHELP_SAVE_OPTIONS _("Save options")
#define CONTEXTHELP_DEFAULTS _("Revert options to defaults")
#define CONTEXTHELP_MINI_MAP _("Show a map of your surroundings when playing")
#define CONTEXTHELP_DOWNLOAD_BEST_TIMES _("Automatically download best times off the net when the game starts")
#define CONTEXTHELP_INGAME_WORLD_RECORD _("Show the World Record for a given level when playing")
#define CONTEXTHELP_HIGHCOLOR _("Enable high-color graphics")
#define CONTEXTHELP_TRUECOLOR _("Enable true-color graphics")
#define CONTEXTHELP_RESOLUTION _("Select graphics resolution")
#define CONTEXTHELP_RUN_IN_WINDOW _("Run the game in a window")
#define CONTEXTHELP_LOW_MENU _("Not so fancy menu graphics")
#define CONTEXTHELP_MEDIUM_MENU _("A bit more fancy menu graphics")
#define CONTEXTHELP_HIGH_MENU _("Fanciest menu graphics")
#define CONTEXTHELP_LOW_GAME _("Disable most graphics not important to the gameplay")
#define CONTEXTHELP_MEDIUM_GAME _("Disable some of the most resource-intensive graphics, like particles")
#define CONTEXTHELP_HIGH_GAME _("Enable all graphical effects")
#define CONTEXTHELP_SOUND_ON _("Turn on sound effects")
#define CONTEXTHELP_11HZ _("Poor sound quality")
#define CONTEXTHELP_22HZ _("Normal sound quality")
#define CONTEXTHELP_44HZ _("Best sound quality")
#define CONTEXTHELP_8BIT _("8-bit sound samples, poor quality")
#define CONTEXTHELP_16BIT _("16-bit sound samples, good quality")
#define CONTEXTHELP_MONO _("Mono (single channel) audio")
#define CONTEXTHELP_STEREO _("Stereo (two channel) audio")
#define CONTEXTHELP_ENGINE_SOUND _("Turn on engine noise")
#define CONTEXTHELP_SELECT_ACTION _("Select action to re-configure to another key")
#define CONTEXTHELP_VIEW_LEVEL_PACK _("View contents of level pack")
#define CONTEXTHELP_SELECT_PLAYER_PROFILE _("Select a player profile to use")
#define CONTEXTHELP_USE_PLAYER_PROFILE _("Use the selected player profile")
#define CONTEXTHELP_CREATE_PLAYER_PROFILE _("Create new player profile")
#define CONTEXTHELP_CLOSE_PROFILE_EDITOR _("Close profile editor")
#define CONTEXTHELP_DELETE_PROFILE _("Permanently delete selected player profile, including best times")
#define CONTEXTHELP_SELECT_LEVEL_IN_LEVEL_PACK _("Select a level in the level pack to play")
#define CONTEXTHELP_CLOSE_LEVEL_PACK _("Close level pack")
#define CONTEXTHELP_GENERAL_INFO _("General information about the level")
#define CONTEXTHELP_BEST_TIMES_INFO _("View best times for the level")
#define CONTEXTHELP_REPLAYS_INFO _("View locally stored replays of the level")
#define CONTEXTHELP_ONLY_SHOW_PERSONAL_BESTS _("Only show personal best times for this level")
#define CONTEXTHELP_SHOW_ALL_BESTS _("Show all best times for this level")
#define CONTEXTHELP_ONLY_SHOW_PERSONAL_REPLAYS _("Only show personal replays for this level")
#define CONTEXTHELP_SHOW_ALL_REPLAYS _("Show all replays for this level")
#define CONTEXTHELP_RUN_SELECTED_REPLAY _("Run selected replay")
#define CONTEXTHELP_SHOWCONTEXTHELP _("Show helpful help strings such as this one")
#define CONTEXTHELP_DOWNLOADLEVELS _("Let X-Moto look for more levels on the net, and install them automatically")
#define CONTEXTHELP_DIRECTCONN _("Select this if you got a direct connection to the Internet")
#define CONTEXTHELP_HTTPPROXY _("Select this if you connects to the Internet through an HTTP proxy")
#define CONTEXTHELP_SOCKS4PROXY _("Select this if you connects to the Internet through a SOCKS4 proxy")
#define CONTEXTHELP_SOCKS5PROXY _("Select this if you connects to the Internet through a SOCKS5 proxy")
#define CONTEXTHELP_OKPROXY _("Use these settings")
#define CONTEXTHELP_PROXYSERVER _("Write the IP address or host name of proxy server to use")
#define CONTEXTHELP_PROXYPORT _("Write the port number used by the proxy server")
#define CONTEXTHELP_NEW_LEVELS _("New levels and levels updated from the Internet")

#define CONTEXTHELP_GHOST_MODE _("Show the ghost if possible in the game")
#define CONTEXTHELP_GHOST_STRATEGIES _("Choose which ghost to display")
#define CONTEXTHELP_MOTIONBLURGHOST _("Make motion blur effect for the ghost (if supported by your graphics card)")
#define CONTEXTHELP_DISPLAY_GHOST_INFO _("When starting a level with a ghost, display who the ghost is of")
#define CONTEXTHELP_DISPLAY_GHOST_TIMEDIFF _("Display the time difference between the ghost and you")
#define CONTEXTHELP_AUTOSAVEREPLAYS _("If you make a highscore it will automatically be saved as a replay")
#define CONTEXTHELP_VIEWTHEHIGHSCORE _("View the replay of the room's highscore")

#define CONTEXTHELP_ENABLE_CHECK_NEW_LEVELS_AT_STARTUP _("Check for new levels at startup")
#define CONTEXTHELP_ENABLE_CHECK_HIGHSCORES_AT_STARTUP _("Check for new highscores at startup")

#define CONTEXTHELP_STATS _("Show various statistics about X-Moto")
#define CONTEXTHELP_UPDATESTATS _("Update statistics (also happens each time X-Moto is started)")
#define CONTEXTHELP_MUSIC _("Enables background music in the main menu")

#define CONTEXTHELP_THEMES _("Choose the X-Moto graphics theme")
#define CONTEXTHELP_UPDATETHEMESLIST _("Check for new themes on the web")
#define CONTEXTHELP_GETSELECTEDTHEME _("Download or update the selected theme")
#define CONTEXTHELP_UPLOAD_HIGHSCORE _("Upload the replay on the website of highscores")
#define CONTEXTHELP_WWW_MAIN_TAB _("Configure the main www options")
#define CONTEXTHELP_WWW_ROOMS_TAB _("Choose where to upload/download replays, highscore lists (for more information, check the website)")
#define CONTEXTHELP_WWW_ROOMS_LIST _("Choose your room")
#define CONTEXTHELP_UPDATEROOMSLIST _("Get the rooms list from the web")
#define CONTEXTHELP_ROOM_LOGIN _("Upload of highscore requires to log in (Except for WR)")
#define CONTEXTHELP_ROOM_PASSWORD _("Upload of highscore requires a password (Except for WR)")
#define CONTEXTHELP_ENGINE_COUNTER _("Show the speedometer when playing")
#define CONTEXTHELP_DEATHANIM _("Enable animation of bike falling apart when dead")
#define CONTEXTHELP_INITZOOM _("Automatically scroll over the level before starting playing it")
#define CONTEXTHELP_CREDITS _("View the X-Moto credits")
#define CONTEXTHELP_REPLAYCOL _("Name of replay")
#define CONTEXTHELP_REPLAYLEVELCOL _("Level played in replay")
#define CONTEXTHELP_REPLAYPLAYERCOL _("Player who recorded the replay")
#define CONTEXTHELP_SCREENRES _("Click to select screen resolution")
#define CONTEXTHELP_LEVELPACKNUMLEVELS _("Completed levels / total number of levels in pack")
#define CONTEXTHELP_LEVELPACK _("Name of level pack")
#define CONTEXTHELP_ADDTOFAVORITE _("Add the level to the favorite levels list")
#define CONTEXTHELP_DELETEFROMFAVORITE _("Delete the level from the favorite levels list")
#define CONTEXTHELP_RANDOMIZE _("Puzzle the levels pack list")
#define CONTEXTHELP_LEVEL_FILTER _("Put a text to limit the level list")
#define CONTEXTHELP_MULTI _("Choose the number of players")

#define VPACKAGENAME_LEVELS_WITH_NO_HIGHSCORE   _("Levels with no highscore")
#define VPACKAGENAME_INCOMPLETED_LEVELS         _("Levels you have not completed")
#define VPACKAGENAME_YOU_HAVE_NOT_THE_HIGHSCORE _("You're not the highscore holder")
#define VPACKAGENAME_ALL_LEVELS                 _("All levels")
#define VPACKAGENAME_NEW_LEVELS                 _("New levels")
#define VPACKAGENAME_UPDATED_LEVELS             _("Updated levels")
#define VPACKAGENAME_FAVORITE_LEVELS            _("Favorite levels")
#define VPACKAGENAME_NICEST_LEVELS              _("Nicest levels")
#define VPACKAGENAME_HARDEST_LEVELS             _("Hardest levels")
#define VPACKAGENAME_EASIEST_LEVELS             _("Easiest levels")
#define VPACKAGENAME_CRAPIEST_LEVELS            _("Crapiest levels")
#define VPACKAGENAME_SCRIPTED                   _("Scripted levels")
#define VPACKAGENAME_MUSICAL                    _("Musical")
#define VPACKAGENAME_BEST_DRIVER                _("By best driver")
#define VPACKAGENAME_NEVER_PLAYED               _("Never played levels")
#define VPACKAGENAME_MOST_PLAYED                _("Most played levels")
#define VPACKAGENAME_LESS_PLAYED                _("Less played levels (but played)")

#define SYS_MSG_UGLY_MODE_ENABLED   _("Ugly mode enabled")
#define SYS_MSG_UGLY_MODE_DISABLED  _("Ugly mode disabled")
#define SYS_MSG_THEME_MODE_ENABLED  _("Theme mode enabled")
#define SYS_MSG_THEME_MODE_DISABLED _("Theme mode disabled")
#define SYS_MSG_WWW_ENABLED   	    _("Web connexion enabled")
#define SYS_MSG_WWW_DISABLED  	    _("Web connexion disabled")

#endif
