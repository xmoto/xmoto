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

#ifndef __GAMETEXT_H__
#define __GAMETEXT_H__

#include "BuildConfig.h"

#include "Locales.h"

/*=============================================================================
Central place to keep many in-game text strings
=============================================================================*/

// set the web lang to use
#define WEB_LANGUAGE _("en_EN")

#define GAMETEXT_11KHZ                _("11 kHz")
#define GAMETEXT_16BIT                _("16-bit")
#define GAMETEXT_16BPP                _("16 bits per pixel")
#define GAMETEXT_22KHZ                _("22 kHz")
#define GAMETEXT_32BPP                _("32 bits per pixel")
#define GAMETEXT_44KHZ                _("44 kHz")
#define GAMETEXT_8BIT                 _("8-bit")
#define GAMETEXT_ABORT                _("Quit level")
#define GAMETEXT_ACCOUNT_PASSWORD     _("Web account password for profile '%s':")
#define GAMETEXT_ADDTOFAVORITE        _("Add to favorite")
#define GAMETEXT_ADDTOBLACKLIST       _("Add to blacklist")
#define GAMETEXT_ACTION               _("Action")
#define GAMETEXT_ALL                  _("All (on this computer)")
#define GAMETEXT_ALL_LEVELS           _("All levels")
#define GAMETEXT_ALLOWINTERNETCONN    _("Do you want to allow X-Moto connecting\n" \
                                        "to the Internet to look for more levels\n" \
                                        "and best times of other players?")
#define GAMETEXT_ALLRECORDS           _("All (on this computer)")
#define GAMETEXT_AUDIO                _("Audio")
#define GAMETEXT_AUTHOR               _("Author")
#define GAMETEXT_AUTOMATIC            _("Automatic")
#define GAMETEXT_AUTOSAVEREPLAYS      _("Automatic saving of replays")
#define GAMETEXT_AUTOZOOM             _("Auto zoom")
#define GAMETEXT_DB                   _("Database")
#define GAMETEXT_CAMERAACTIVEZOOM     _("Camera Active Zoom")
#define GAMETEXT_BESTTIMES            _("Best Times")
#define GAMETEXT_BESTPLAYER           _("Best player")
#define GAMETEXT_BRAKE                _("Brake")
#define GAMETEXT_BY_PLAYER            _("(by %s)")
#define GAMETEXT_CAMERAMOVEXDOWN      _("Move Camera to left")
#define GAMETEXT_CAMERAMOVEXUP        _("Move Camera to right")
#define GAMETEXT_CAMERAMOVEYDOWN      _("Move Camera down")
#define GAMETEXT_CAMERAMOVEYUP        _("Move Camera up")
#define GAMETEXT_CANCEL               _("Cancel")
#define GAMETEXT_CHANGE               _("Change...")
#define GAMETEXT_CHANGEDIR            _("Change direction")
#define GAMETEXT_CHANGEKEY            _("Change Key...")
#define GAMETEXT_CHECKINGFORLEVELS    _("Checking for new or updated levels...")
#define GAMETEXT_CHILDREN_COMPLIANT   _("I'm a child")
#define GAMETEXT_CHOOSELEVEL          _("Choose Level")
#define GAMETEXT_CLEAN                _("Clean")
#define GAMETEXT_CLEAN_CONFIRM(A) ngettext("Are you sure you want to delete %i replay", "Are you sure you want to delete %i replay", A)
#define GAMETEXT_CLEAN_NOTHING_TO_DO  _("There is no replay to clean")
#define GAMETEXT_CLOSE                _("Close")
#define GAMETEXT_CONFIGUREJOYSTICK    _("Configure Joystick...")
#define GAMETEXT_CONTROLS             _("Controls")

#define GAMETEXT_CREDITS                                                                     \
  std::string(_("Lead Programming")) +                                                       \
    ":Rasmus Neckelmann;"                                                                    \
    ":Nicolas Adenis-Lamarre;"                                                               \
    ":Emmanuel Gorse;"                                                                       \
    ":Richard Franks;"                                                                       \
    ":Kees Jongenburger;"                                                                    \
    ":;"                                                                                     \
    + std::string(_("Additional Programming")) +                                             \
    ":Eric Piel;"                                                                            \
    ":Christoph Sommer;"                                                                     \
    ":Frederic Huard;"                                                                       \
    ":Brice Goglin;"                                                                         \
    ":Kristian Jagd;"                                                                        \
    ":Jens Erler;"                                                                           \
    ":;"                                                                                     \
    + std::string(_("Graphics")) +                                                           \
    ":Rasmus Neckelmann;"                                                                    \
    ":Jens Erler;"                                                                           \
    ":;"                                                                                     \
    + std::string(_("Audio")) +                                                              \
    ":Jens Erler;"                                                                           \
    ":Brandon Ross;"                                                                         \
    ":;"                                                                                     \
    + std::string(_("MacOS X Support")) +                                                    \
    ":Dave Vasilevsky;"                                                                      \
    ":Maxime Biais;"                                                                         \
    ":;"                                                                                     \
    + std::string(_("Localization")) +                                                       \
    ":Jozef Riha (" + std::string(_("Slovak")) + ");"                                        \
    ":Nicolas Adenis-Lamarre (" + std::string(_("French")) + ");"                            \
    ":Wilhelm Francke (" + std::string(_("Norwegian")) + ");"                                \
    ":Jonathan Sieber, Jens Erler (" + std::string(_("German")) + ");"                    \
    ":Maximum/Tuomo Koistinen (" + std::string(_("Finnish")) + ");"                                          \
    ":Trullo (" + std::string(_("Catalan")) + ");"                                           \
    ":Trullo (" + std::string(_("Spanish")) + ");"                                           \
    ":Mateusz Kosibowicz (" + std::string(_("Polish")) + ");"                                \
    ":Lucas Manzari (" + std::string(_("Italian")) + ");"                                    \
    ":Lukas Klingsbo (" + std::string(_("Swedish")) + ");"                                   \
    ":Jan KalÃ¡b (" + std::string(_("Czech")) + ");"                                          \
    ":" + std::string(_("Mikhail Brinchuk")) + " (" + std::string(_("Russian")) + ");"       \
    ":Lucas Hermann Negri (" + std::string(_("Brazilian Portuguese")) + ");"                 \
    ":Jānis Rūcis (" + std::string(_("Latvian")) + ");"                                      \
    ":Kristian Jagd (" + std::string(_("Danish")) + ");"                                     \
    ":;"                                                                                     \
    + std::string(_("Main testers")) +                                                       \
    ":Jose Alberto Coelho;"                                                                  \
    ":Frederic Huard;"                                                                       \
    ":Jens Erler;"                                                                           \
    ":;"                                                                                     \
    + std::string(_("Thanks to")) +                                                          \
    ":SnowBear ("           + std::string(_("for KDE/gnome integration")) 	     + ");"  \
    ":Samuel Mimram ("      + std::string(_("for Debian packaging"))      	     + ");"  \
    ":Dark ("               + std::string(_("for Gentoo packaging"))      	     + ");"  \
    ":Olivier Blin (" 	    + std::string(_("for Mandriva packaging"))    	     + ");"  \
    ":Dmitry Marakasov ("   + std::string(_("for FreeBSD packaging"))                + ");"  \
    ":Balazs Rozsa (" 	    + std::string(_("for across/elma"))           	     + ");"  \
    ":Yves Berquin (" 	    + std::string(_("for making tinyxml"))        	     + ");"  \
    ":L. Peter Deutsch ("   + std::string(_("for the MD5 code"))          	     + ");"  \
    ":Petter Reinholdtsen;"                                                                  \
    ":" 		    + std::string(_("Everyone who have made levels"))        + ";"   \
    ":" 		    + std::string(_("People who have provided bug reports")) + ";"   \
    ":" 		    + std::string(_("Anyone who has helped in any way"))     + ";"

#define GAMETEXT_CREDITSBUTTON        _("Credits")
#define GAMETEXT_PLAYER               _("Player")
#define GAMETEXT_DATE                 _("Date")
#define GAMETEXT_DBSYNCHRONIZE_EXPLANATION _("X-Moto uses a database on your computer to save all your scores and statistics. You can send these data using your web account on the X-Moto server. It is usefull to save your data or if you play X-Moto from different places.")
#define GAMETEXT_DBSYNCHRONIZE_WARNING _("If you manually copied the X-Moto user files (.xmoto) on some other computers, you should not use this functionnality otherwise your statistics will be duplicated/not synchronized. If you did that, you should remove the copies you did.")
#define GAMETEXT_DBSYNCHRONIZE _("Synchronise")
#define GAMETEXT_DBSYNCHRONIZE_ONQUIT _("Synchronize when X-Moto ends")
#define GAMETEXT_DB_UPGRADING               _("Upgrading database")
#define GAMETEXT_DB_UPGRADING_STATS_PROFILE _("Upgrading database stats for the profile")
#define GAMETEXT_DB_UPGRADING_PROFILE _("Upgrading database profile")
#define GAMETEXT_DEATHANIM            _("Death Animation")
#define GAMETEXT_DEFAULTS             _("Defaults")
#define GAMETEXT_DELETEPLAYERMESSAGE  _("Do you really want to delete player?")
#define GAMETEXT_DELETEREPLAYMESSAGE  _("Do you really want to delete replay?")
#define GAMETEXT_DELETE               _("Delete")
#define GAMETEXT_DELETEFROMFAVORITE  _("Delete from favorite")
#define GAMETEXT_DELETEFROMBLACKLIST  _("Delete from blacklist")
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
#define GAMETEXT_LOADNEWLEVELS        _("Loading new and updated levels...")
#define GAMETEXT_DLROOMSLISTCHECK     _("Checking for existing rooms...")
#define GAMETEXT_DLTHEME              _("Downloading theme data required by new levels...\nPress ESC to abort.")
#define GAMETEXT_DLTHEMESLISTCHECK    _("Checking for new themes...")
#define GAMETEXT_DOWNLOADLEVELS       _("Get More Levels!")
#define GAMETEXT_DRIVE                _("Drive")
#define GAMETEXT_ENABLEAUDIO          _("Enable Audio")
#define GAMETEXT_ENABLECHECKNEWLEVELSATSTARTUP  _("Check new levels at startup")
#define GAMETEXT_ENABLECHECKHIGHSCORESATSTARTUP _("Check new highscores at startup")
#define GAMETEXT_ENABLECONTEXTHELP    _("Enable Context Help")
#define GAMETEXT_ENABLEENGINESOUND    _("Enable Engine Sound")
#define GAMETEXT_ENABLEGHOST          _("Enable Ghost")
#define GAMETEXT_ENABLEINGAMEWORLDRECORD _("Show World Record in-game")
#define GAMETEXT_ENABLEMUSIC          _("Enable Music")
#define GAMETEXT_ENABLEWEBHIGHSCORES  _("Enable WWW Access")
#define GAMETEXT_ENTERPLAYERNAME      _("Enter player name")
#define GAMETEXT_ENTERREPLAYNAME      _("Enter name of replay")
#define GAMETEXT_ERRORSINLEVEL        _("There are errors in the level, don't expect it to be playable!")
#define GAMETEXT_CHECK_YOUR_WWW       _("Check your Internet connection !")  
#define GAMETEXT_FAILEDCHECKLEVELS    _("Failed to check for levels.")
#define GAMETEXT_FAILEDDLHIGHSCORES   _("Failed to download high-scores.")
#define GAMETEXT_FAILEDDLLEVELS       _("Failed to download levels.")
#define GAMETEXT_FAILEDDLREPLAY       _("Failed to download the replay.")
#define GAMETEXT_FAILEDDLROOMSLIST    _("Failed to download the rooms list.")
#define GAMETEXT_FAILEDGETSELECTEDTHEME _("Failed to get the selected theme")
#define GAMETEXT_FAILEDTOINITLEVEL    _("Failed to initialize level!")
#define GAMETEXT_FAILEDTOLOADREPLAY   _("Failed to load replay!")
#define GAMETEXT_FAILEDTOSAVEREPLAY   _("Failed to save replay!\nMaybe you should try with another name?")
#define GAMETEXT_FAILEDUPDATETHEMESLIST _("Failed to update the theme list")
#define GAMETEXT_FILE                 _("File")
#define GAMETEXT_FILTER               _("Filter")
#define GAMETEXT_FINISH               _("Finished!")
#define GAMETEXT_FINISHTIME           _("Finish Time")
#define GAMETEXT_FLIPLEFT             _("Flip left")
#define GAMETEXT_FLIPRIGHT            _("Flip right")
#define GAMETEXT_GAMEGFX              _("Game Graphics")
#define GAMETEXT_GENERAL              _("General")
#define GAMETEXT_GENERALINFO          _("General Info")
#define GAMETEXT_GETSELECTEDTHEME     _("Get this theme")
#define GAMETEXT_GHOST_LOCAL          _("Local best")
#define GAMETEXT_GHOST_BEST           _("Your best")
#define GAMETEXT_GHOST_STRATEGIES_TYPE  _("Type")
#define GAMETEXT_GHOST_STRATEGY_BESTOFREFROOM    _("The highscore of the reference room")
#define GAMETEXT_GHOST_STRATEGY_BESTOFOTHERROOMS _("The highscore of the other rooms")
#define GAMETEXT_GHOST_STRATEGY_MYBEST  _("The best of my replays (on this computer)")
#define GAMETEXT_GHOST_STRATEGY_THEBEST _("The best of the replays (on this computer)")
#define GAMETEXT_GHOSTOF              _("Ghost of %s")
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
"  PageDown/PageUp - " + std::string(_("Previous/Next level")) + "\n" \
"  F3 - " + std::string(_("Add/remove to favorite levels")) + "\n"\
"  ctrl+B - " + std::string(_("Add/remove to blacklisted levels")) + "\n"\
"  ctrl+M - " + std::string(_("change mirror mode")) + "\n"\
"  F8 - " + std::string(_("Enable/disable web")) + "\n" \
"  F9 - " + std::string(_("Enable/disable ugly mode")) + "\n" \
"  F5 - " + std::string(_("Reload levels, themes and replays from directories")) + "\n" \
"  F12 - " + std::string(_("Take a screenshot")) + "\n" \
"\n" \
 + std::string(_("Find all the strawberries and touch the flower to finish\nthe level.")) + "\n" \
 + std::string(_("Read the README file or check out the website at\nhttp://xmoto.tuxfamily.org for more information."))

#define GAMETEXT_HIDEGHOSTS           _("Hide ghosts")
#define GAMETEXT_HIGH                 _("High")
#define GAMETEXT_LOAD_LEVEL_HOOK      _("Levels are being added into the database. Please wait.")
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
#define GAMETEXT_LANGUAGETAB          _("Language")
#define GAMETEXT_LANGUAGE_NAME        _("Language name")
#define GAMETEXT_LANGUAGE_CODE        _("Language code")
#define GAMETEXT_LEVEL                _("Level")
#define GAMETEXT_LEVEL_ADDED_TO_FAVORITE     _("Added to favorites")
#define GAMETEXT_LEVEL_DELETED_FROM_FAVORITE _("Deleted from favorites")
#define GAMETEXT_LEVEL_ADDED_TO_BLACKLIST     _("Added to blacklist")
#define GAMETEXT_LEVEL_DELETED_FROM_BLACKLIST _("Deleted from blacklist")
#define GAMETEXT_LEVELINFO            _("Level Info...")
#define GAMETEXT_LEVELNAME            _("Level Name")
#define GAMETEXT_LEVELCANNOTBELOADED  _("Level '%s' cannot be loaded")
#define GAMETEXT_LEVELPACK            _("Level Pack")
#define GAMETEXT_LEVELPACKS           _("Level Packs")
#define GAMETEXT_LEVELREQUIREDBYREPLAY _("Level '%s' required by replay!")
#define GAMETEXT_LEVELS               _("Levels")
#define GAMETEXT_LISTALL              _("List All")
#define GAMETEXT_LOADING              _("Loading...")
#define GAMETEXT_LOADINGLEVELS        _("Loading levels...")
#define GAMETEXT_LOADINGMENUGRAPHICS  _("Loading menu graphics...")
#define GAMETEXT_LOADINGREPLAYS       _("Loading replays...")
#define GAMETEXT_LOADINGSOUNDS        _("Loading sounds...")
#define GAMETEXT_LOADINGTEXTURES      _("Loading textures...")
#define GAMETEXT_LOGIN                _("Login")
#define GAMETEXT_LOW                  _("Low")
#define GAMETEXT_MAIN                 _("Main")
#define GAMETEXT_MOUSE                _("Mouse")
#define GAMETEXT_THEME                _("Theme")
#define GAMETEXT_MEDIUM               _("Medium")
#define GAMETEXT_MENUGFX              _("Menu Graphics")
#define CONTEXTHELP_MINISTATTIMEFORPACKLEVEL _("Total time you scored / total highscore time for levels you finished")
#define GAMETEXT_MISSINGTEXTURES      _("Level references unknown textures, it could be unplayable!")
#define GAMETEXT_MONO                 _("Mono")
#define GAMETEXT_MOSTPLAYEDLEVELSFOLLOW _("Following are your most played levels")
#define GAMETEXT_MOTIONBLURGHOST      _("Motion blur ghost")
#define GAMETEXT_MULTI                _("Multi")
#define GAMETEXT_NOMULTISCENES        _("Semi-cooperative mode")
#define GAMETEXT_MULTISTOPWHENONEFINISHES _("Stop the game once a player ends the level")
#define GAMETEXT_NB_PLAYERS           _("Number of players")
#define GAMETEXT_NEW                  _("New")
#define GAMETEXT_NEWERXMOTOREQUIRED   _("X-Moto %s or newer required to load level")
#define GAMETEXT_NEWHIGHSCORE         _("New highscore!")
#define GAMETEXT_NEWHIGHSCOREPERSONAL _("New personal highscore!")
#define GAMETEXT_NEWLEVELAVAIL(A)     ngettext("%i new or updated level available. Download now ?",  \
					       "%i new or updated levels available. Download now ?", \
					       A)
#define GAMETEXT_NEWLEVELS            _("New Levels")
#define GAMETEXT_NEWLEVELS_AVAIBLE    _("New levels available!")
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
                                      "to xmoto@tuxfamily.org\n" \
                                      "\n" \
                                      "Also visit http://xmoto.tuxfamily.org to make sure you've\n" \
                                      "got the latest version.")
#define GAMETEXT_NUMLEVELS            _("# Levels")
#define GAMETEXT_OK                   _("OK")  
#define GAMETEXT_OPEN                 _("Open")
#define GAMETEXT_OPERATION_COMPLETED  _("Operation completed")
#define GAMETEXT_OPTIONS              _("Options")
#define GAMETEXT_OPTIONSREQURERESTART _("Some options will not take effect before next restart!")
#define GAMETEXT_PACK_AUTHORS 	      _("By author")
#define GAMETEXT_PACK_MEDALS 	      _("Medals")
#define GAMETEXT_PACK_ROOM 	      _("Room")
#define GAMETEXT_PACK_SPECIAL 	      _("Special")
#define GAMETEXT_PACK_STANDARD 	      _("Standard")
#define GAMETEXT_PACK_WEBVOTES 	      _("Web votes")
#define GAMETEXT_PACK_STATS           _("Statistics")
#define GAMETEXT_PACK_BEST_DRIVERS    _("By best driver")
#define GAMETEXT_PACK_BY_LENGTH       _("By length")
#define GAMETEXT_PAUSE                _("Pause")
#define GAMETEXT_PASSWORD             _("Password")
#define GAMETEXT_PERSONAL             _("Personal")
#define GAMETEXT_PERSONALRECORDS      _("Personal")
#define GAMETEXT_NPLAYER(A)           ngettext("%i player", "%i players", A)
#define GAMETEXT_PLAYERPROFILE        _("Player Profile")
#define GAMETEXT_PLAYERPROFILES       _("Player Profiles")
#define GAMETEXT_PLAYNEXT             _("Play Next Level")
#define GAMETEXT_PORT                 _("Port")
#define GAMETEXT_PRESSANYKEYTO        _("Press key you want to '%s' or ESC to cancel...")
#define GAMETEXT_PROXYSERVER          _("Proxy Server")
#define GAMETEXT_PROXYCONFIG          _("Configure Proxy...")
#define GAMETEXT_QUICKSTART           _("Quick start")
#define GAMETEXT_QUIT                 _("Quit Game")
#define GAMETEXT_QUITMESSAGE          _("Do you really want to quit?")
#define GAMETEXT_RANDOMIZE            _("Randomize")
#define GAMETEXT_RELOADINGLEVELS      _("Reloading levels...")
#define GAMETEXT_RELOADINGREPLAYS     _("Reloading replays...")
#define GAMETEXT_RELOADINGTHEMES      _("Reloading themes...")
#define GAMETEXT_REPLAY               _("Replay")
#define GAMETEXT_REPLAYHELPTEXT(current_speed) _("Stop[esc] ||[space]  << >>[left/right keys]  < >[up/down keys]   Speed:") + current_speed + "x"
#define GAMETEXT_REPLAYHELPTEXTNOREWIND(current_speed) _("Stop[esc] ||[space] >>[right key] < >[up/down keys]   Speed:") + current_speed + "x"
  // + "a" + "X"
#define GAMETEXT_REPLAYNOTFOUND       _("The replay can't be played!")
#define GAMETEXT_REPLAYOF             _("Replay of %s")
#define GAMETEXT_REPLAYS              _("Replays")
#define GAMETEXT_RESTART              _("Restart This Level")
#define GAMETEXT_RESUME               _("Resume Playing")
#define GAMETEXT_ROOM                 _("Room")
#define GAMETEXT_RUNWINDOWED          _("Run Windowed")
#define GAMETEXT_SAVE                 _("Save")
#define GAMETEXT_SAVE_AS              _("Saved as %s")
#define GAMETEXT_SAVEREPLAY           _("Save Replay")
#define GAMETEXT_SCREENRES            _("Screen Resolution")
#define GAMETEXT_SHOW                 _("Show")
#define GAMETEXT_SHOWENGINECOUNTER    _("Speedometer")
#define GAMETEXT_SHOWINFO             _("Info...")
#define GAMETEXT_SHOWMINIMAP          _("Show Mini Map")
#define GAMETEXT_STARTLEVEL           _("Play!")
#define GAMETEXT_STATISTICS           _("Statistics")
#define GAMETEXT_STATS                _("STATS")
#define GAMETEXT_STEREO               _("Stereo")
#define GAMETEXT_SYNC_DOWN           _("Synchronisation down")
#define GAMETEXT_SYNC_UP           _("Synchronisation up")
#define GAMETEXT_SYNC_DONE         _("Synchronisation done successfully")
#define GAMETEXT_THEMES               _("Theme")
#define GAMETEXT_THEMEHOSTED          _("Available")
#define GAMETEXT_THEMENOTHOSTED       _("To download")
#define GAMETEXT_THEMEREQUIREUPDATE   _("To be updated")
#define GAMETEXT_THEMEUPTODATE        _("The theme is now up to date")
#define GAMETEXT_TIME                 _("Time")
#define GAMETEXT_TRYAGAIN             _("Try This Level Again")
#define GAMETEXT_TUTORIAL             _("Tutorial")
#define GAMETEXT_UNKNOWN              _("Unknown")
#define GAMETEXT_UNPACKED_LEVELS_PACK _("Unpacked levels")
#define GAMETEXT_UNUPDATABLETHEMEONWEB _("Can't update this theme !\nThe theme is not available on the web\nor your theme list is not up to date")
#define GAMETEXT_UPDATE               _("Update")
#define GAMETEXT_UPDATED              _("Updated")
#define GAMETEXT_UPDATEHIGHSCORES     _("Check WWW")
#define GAMETEXT_UPDATEROOMSSLIST     _("Update the rooms list")
#define GAMETEXT_UPDATETHEMESLIST     _("Update the theme list")
#define GAMETEXT_UPDATINGLEVELS       _("Updating level lists...")
#define GAMETEXT_UPLOAD_HIGHSCORE     _("Upload")
#define GAMETEXT_UPLOAD_ALL_HIGHSCORES _("Upload all highscores")
#define GAMETEXT_UPLOAD_HIGHSCORE_ERROR _("An unexpected error has occurred. There may be some problems with the web site.")
#define GAMETEXT_UPLOAD_HIGHSCORE_WEB_WARNING_BEFORE _("Oh no !")
#define GAMETEXT_UPLOADING_HIGHSCORE  _("Uploading the highscore...")
#define GAMETEXT_USECRAPPYINFORMATION _("Use crappy information")
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
#define GAMETEXT_WWWTAB               _("WWW")
#define GAMETEXT_WWWROOMSTAB_REFERENCE _("Reference room")
#define GAMETEXT_WWWROOMSTAB_OTHER     _("Room %i")

#define GAMETEXT_XHOURS               _("%d hours") 
#define GAMETEXT_XMINUTES             _("%d minutes")

#define GAMETEXT_XMOTOGLOBALSTATS_SINCE        _("Stats since: %s")
#define GAMETEXT_XMOTOGLOBALSTATS_START(A)     ngettext("X-Moto started %d time", "X-Moto started %d times", A)
#define GAMETEXT_XMOTOGLOBALSTATS_DIFFERENT(A) ngettext("%d different level", "%d different levels", A)
#define GAMETEXT_XMOTOGLOBALSTATS_TIMEPLAYED   _("Time played: %s")
#define GAMETEXT_XMOTOLEVELSTATS_PLAYS(A)    ngettext("%d play", "%d plays", A)
#define GAMETEXT_XMOTOLEVELSTATS_DEATHS(A)   ngettext("%d death", "%d deaths", A)
#define GAMETEXT_XMOTOLEVELSTATS_FINISHED(A) ngettext("%d finished", "%d finished", A)
#define GAMETEXT_XMOTOLEVELSTATS_RESTART(A)  ngettext("%d restart", "%d restarts", A)

#define GAMETEXT_XSECONDS             _("%d seconds")
#define GAMETEXT_YES                  _("Yes")
#define GAMETEXT_YES_FOR_ALL          _("Yes to all")
#define GAMETEXT_ZOOMIN  	      _("Zoom in")
#define GAMETEXT_ZOOMINIT 	      _("Reinitialize zoom")
#define GAMETEXT_ZOOMOUT 	      _("Zoom out")

/* Context help strings */
#define CONTEXTHELP_UPDATEHIGHSCORES _("Download the latest X-Moto world records and check for new levels")
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
#define CONTEXTHELP_USECRAPPYINFORMATION _("Use crappy information from the website to update the crappy pack")
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
#define CONTEXTHELP_HTTPPROXY _("Select this if you connect to the Internet through an HTTP proxy")
#define CONTEXTHELP_SOCKS4PROXY _("Select this if you connect to the Internet through a SOCKS4 proxy")
#define CONTEXTHELP_SOCKS5PROXY _("Select this if you connect to the Internet through a SOCKS5 proxy")
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
#define CONTEXTHELP_UPLOAD_HIGHSCORE_ALL _("Upload your best replays to your room (useful when it is just created)")
#define CONTEXTHELP_WWW_MAIN_OPTIONS _("Configure the main www options")
#define CONTEXTHELP_WWW_OPTIONS _("Configure the www options")
#define CONTEXTHELP_WWW_ROOMS_OPTIONS _("Choose where to upload/download replays, highscore lists (for more information, check the website)")
#define CONTEXTHELP_WWW_ROOMS_LIST _("Choose your room")
#define CONTEXTHELP_UPDATEROOMSLIST _("Get the rooms list from the web")
#define CONTEXTHELP_ROOM_LOGIN _("Upload of highscore requires to log in (Except for WR)")
#define CONTEXTHELP_ROOM_PASSWORD _("Upload of highscore requires a password (Except for WR)")
#define CONTEXTHELP_ENGINE_COUNTER _("Show the speedometer when playing")
#define CONTEXTHELP_DEATHANIM _("Enable animation of bike falling apart when dead")
#define CONTEXTHELP_INITZOOM _("Automatically scroll over the level before starting playing it")
#define CONTEXTHELP_CAMERAACTIVEZOOM _("Run a dynamic zoom while playing")
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
#define CONTEXTHELP_LEVEL_FILTER _("Enter text to filter the level list")
#define CONTEXTHELP_MULTI _("Choose the number of players")
#define CONTEXTHELP_PROXYLOGIN    _("Your proxy login")
#define CONTEXTHELP_PROXYPASSWORD _("Your password associated with your login")
#define CONTEXTHELP_REPLAYS_FILTER _("Enter text to filter the replay list")
#define CONTEXTHELP_QUICKSTART _("Press to play X-Moto immediately")
#define CONTEXTHELP_QUICKSTART_QUALITY    _("Increase or decrease the quality of the levels selected")
#define CONTEXTHELP_QUICKSTART_DIFFICULTY _("Increase or decrease the difficulty of the levels selected")
#define CONTEXTHELP_MULTISTOPWHENONEFINISHES _("Stop the game once a player ends the level or continue while a player is running")
#define CONTEXTHELP_GHOST_STRATEGY_MYBEST _("The best of my replays (on this computer)")
#define CONTEXTHELP_GHOST_STRATEGY_THEBEST _("The best of the replays (on this computer)")
#define CONTEXTHELP_GHOST_STRATEGY_BESTOFREFROOM _("The highscore of the reference room")
#define CONTEXTHELP_GHOST_STRATEGY_BESTOFOTHERROOMS _("The highscore of the other rooms")
#define CONTEXTHELP_HIDEGHOSTS _("Don't show the ghosts while playing")
#define CONTEXTHELP_NOMULTISCENES _("The scene is shared by all the players")
#define CONTEXTHELP_LANGUAGE_OPTIONS _("Change the X-Moto language")
#define CONTEXTHELP_LANGUAGE_NAME _("Name of the language")
#define CONTEXTHELP_LANGUAGE_CODE _("Code of the language")
#define CONTEXTHELP_REPLAYS_CLEAN _("Delete replays that X-Moto estimates to be not interesting to keep")
#define CONTEXTHELP_CHILDREN_COMPLIANT _("Remove levels not suitable for children")
#define CONTEXTHELP_ROOM_ENABLE _("Enable this room")
#define CONTEXTHELP_WWW_PASSWORD _("Password associated to your profile for web access")
#define CONTEXTHELP_GENERAL_MAIN_OPTIONS _("Main options")
#define CONTEXTHELP_THEME_OPTIONS        _("Configure the X-Moto theme")
#define CONTEXTHELP_GHOSTS_OPTIONS       _("Configure ghosts displayed while playing the game")
#define CONTEXTHELP_DB_OPTIONS           _("Configure options linked to your X-Moto dabatase")
#define CONTEXTHELP_DBSYNCHRONIZE _("Synchronise your database on the web")
#define CONTEXTHELP_DBSYNCHRONIZE_ONQUIT _("Synchronise your database when your quit X-Moto")

#define VPACKAGENAME_LEVELS_WITH_NO_HIGHSCORE   _("Levels with no highscore")
#define VPACKAGENAME_INCOMPLETED_LEVELS         _("Levels you have not completed")
#define VPACKAGENAME_YOU_HAVE_NOT_THE_HIGHSCORE _("You're not the highscore holder")
#define VPACKAGENAME_ALL_LEVELS                 _("All levels")
#define VPACKAGENAME_NEW_LEVELS                 _("New and updated levels")
#define VPACKAGENAME_FAVORITE_LEVELS            _("Favorite levels")
#define VPACKAGENAME_BLACKLIST_LEVELS            _("Blacklisted levels")
#define VPACKAGENAME_NICEST_LEVELS              _("Nicest levels")
#define VPACKAGENAME_HARDEST_LEVELS             _("Hardest levels")
#define VPACKAGENAME_EASIEST_LEVELS             _("Easiest levels")
#define VPACKAGENAME_CRAPPIEST_LEVELS            _("Crappiest levels")
#define VPACKAGENAME_SCRIPTED                   _("Scripted levels")
#define VPACKAGENAME_MUSICAL                    _("Musical levels")
#define VPACKAGENAME_PHYSICS                    _("Physics levels")
#define VPACKAGENAME_BEST_DRIVER                _("By best driver")
#define VPACKAGENAME_LAST_PLAYED                _("Last played levels")
#define VPACKAGENAME_NEVER_PLAYED               _("Never played levels")
#define VPACKAGENAME_MOST_PLAYED                _("Most played levels")
#define VPACKAGENAME_LESS_PLAYED                _("Less played levels (but played)")
#define VPACKAGENAME_LAST_HIGHSCORES            _("Last highscores")
#define VPACKAGENAME_LAST_LEVELS                _("Last levels")
#define VPACKAGENAME_OLDEST_HIGHSCORES          _("Oldest highscores")
#define VPACKAGENAME_MEDAL_PLATINIUM            _("Platinum")
#define VPACKAGENAME_MEDAL_GOLD                 _("Gold")
#define VPACKAGENAME_MEDAL_SILVER               _("Silver")
#define VPACKAGENAME_MEDAL_BRONZE               _("Bronze")
#define VPACKAGENAME_MEDAL_NONE                 _("No medal")
#define VPACKAGENAME_MY_LEVELS                  _("My levels")
#define VPACKAGENAME_CRAPPY_LEVELS              _("Crappy levels")
#define VPACKAGENAME_SHORT_LEVELS               _("Shortest levels")
#define VPACKAGENAME_MEDIUM_LEVELS              _("Medium levels")
#define VPACKAGENAME_LONG_LEVELS                _("Long levels")
#define VPACKAGENAME_VERY_LONG_LEVELS           _("Very long levels")

#define VPACKAGENAME_DESC_LEVELS_WITH_NO_HIGHSCORE   _("Levels with no highscore in your room")
#define VPACKAGENAME_DESC_INCOMPLETED_LEVELS         _("Levels you have not completed")
#define VPACKAGENAME_DESC_YOU_HAVE_NOT_THE_HIGHSCORE _("Somebody in the room made a better highscore than yours")
#define VPACKAGENAME_DESC_ALL_LEVELS                 _("All X-Moto levels")
#define VPACKAGENAME_DESC_NEW_LEVELS                 _("New and updated levels since your last check")
#define VPACKAGENAME_DESC_FAVORITE_LEVELS            _("Your favorite levels")
#define VPACKAGENAME_DESC_BLACKLIST_LEVELS            _("Your blacklisted levels")
#define VPACKAGENAME_DESC_NICEST_LEVELS          _("Nicest X-Moto levels according to the web votes")
#define VPACKAGENAME_DESC_HARDEST_LEVELS         _("Hardest X-Moto levels according to the web votes")
#define VPACKAGENAME_DESC_EASIEST_LEVELS         _("Easiest X-Moto levels according to the web votes")
#define VPACKAGENAME_DESC_CRAPPIEST_LEVELS       _("Crappiest X-Moto levels according to the web votes")
#define VPACKAGENAME_DESC_SCRIPTED                   _("X-Moto levels which are dynamic")
#define VPACKAGENAME_DESC_MUSICAL                    _("X-Moto levels with a background music")
#define VPACKAGENAME_DESC_PHYSICS                    _("X-Moto levels using strongly physics")
#define VPACKAGENAME_DESC_LAST_PLAYED                _("Last levels you have played")
#define VPACKAGENAME_DESC_NEVER_PLAYED               _("X-Moto levels you never played")
#define VPACKAGENAME_DESC_MOST_PLAYED                _("X-Moto levels you have played the most")
#define VPACKAGENAME_DESC_LESS_PLAYED                _("X-Moto levels you have played only a few times")
#define VPACKAGENAME_DESC_LAST_HIGHSCORES        _("X-Moto levels having a recent highscore in your room")
#define VPACKAGENAME_DESC_LAST_LEVELS                _("Last created X-Moto levels")
#define VPACKAGENAME_DESC_OLDEST_HIGHSCORES      _("X-Moto levels having an old highscore in your room")
#define VPACKAGENAME_DESC_MEDAL_PLATINIUM        _("You have the room highscore")
#define VPACKAGENAME_DESC_MEDAL_GOLD             _("Your personal highscore finish time is almost the room highscore (95%)")
#define VPACKAGENAME_DESC_MEDAL_SILVER           _("Your personal highscore finish time is almost the room highscore (90%-95%)")
#define VPACKAGENAME_DESC_MEDAL_BRONZE           _("Your personal highscore finish time is almost the room highscore (80%-90%)")
#define VPACKAGENAME_DESC_MEDAL_NONE             _("The room highscore is really better than your own highscore")
#define VPACKAGENAME_DESC_STANDARD               _("Level pack \"%s\"")
#define VPACKAGENAME_DESC_BEST_DRIVERS           _("Highscores by %s")
#define VPACKAGENAME_DESC_MY_LEVELS              _("External levels (the one you put into the 'My levels' directory)")
#define VPACKAGENAME_DESC_CRAPPY_LEVELS          _("Levels marked 'crappy' are hidden in all other packs")
#define VPACKAGENAME_DESC_SHORT_LEVELS           _("The shortest levels (less than 25 seconds)")
#define VPACKAGENAME_DESC_MEDIUM_LEVELS          _("The medium levels (25 seconds to 60)")
#define VPACKAGENAME_DESC_LONG_LEVELS            _("The long levels (1 minute to 2 minutes)")
#define VPACKAGENAME_DESC_VERY_LONG_LEVELS       _("The longest levels (more than 2 minutes)")

#define SYS_MSG_UGLY_MODE_ENABLED      	_("Ugly mode enabled")
#define SYS_MSG_UGLY_MODE_DISABLED     	_("Ugly mode disabled")
#define SYS_MSG_THEME_MODE_ENABLED     	_("Theme mode enabled")
#define SYS_MSG_THEME_MODE_DISABLED    	_("Theme mode disabled")
#define SYS_MSG_UGLY_OVER_MODE_ENABLED  _("UglyOver mode enabled")
#define SYS_MSG_UGLY_OVER_MODE_DISABLED _("UglyOver mode disabled")
#define SYS_MSG_WWW_ENABLED   	       	_("Web connection enabled")
#define SYS_MSG_WWW_DISABLED  	       	_("Web connection disabled")
#define SYS_MSG_INTERPOLATION_ENABLED  	_("Replay interpolation enabled")
#define SYS_MSG_INTERPOLATION_DISABLED 	_("Replay interpolation disabled")
#define SYS_MSG_FPS_ENABLED             _("Fps enabled")
#define SYS_MSG_FPS_DISABLED            _("Fps disabled")
#define SYS_MSG_AUDIO_ENABLED           _("Audio enabled")
#define SYS_MSG_AUDIO_DISABLED          _("Audio disabled")

#define SYS_MSG_TRAIN_NO_RESTORE_AVAIL  _("No training positions stored")
#define SYS_MSG_TRAIN_RESTORING         _("Training position %i/%i restored")
#define SYS_MSG_TRAIN_STORED            _("Stored as training position %i")

// font to use to choice the GROUP : currently, two groups, GENERAL and ASIAN
#define FONT_GROUP _("FontGroup:GENERAL")

#endif
