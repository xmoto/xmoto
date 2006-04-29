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

/*=============================================================================
Central place to keep many in-game text strings
=============================================================================*/

#define GAMETEXT_11KHZ                "11 kHz"
#define GAMETEXT_16BIT                "16-bit"
#define GAMETEXT_16BPP                "16 bits per pixel"
#define GAMETEXT_22KHZ                "22 kHz"
#define GAMETEXT_32BPP                "32 bits per pixel"
#define GAMETEXT_44KHZ                "44 kHz"
#define GAMETEXT_8BIT                 "8-bit"
#define GAMETEXT_ABORT                "Abort Playing"
#define GAMETEXT_ACTION               "Action"
#define GAMETEXT_ALL                  "All"
#define GAMETEXT_ALREADYUSED          "Already used!"
#define GAMETEXT_AUDIO                "Audio"
#define GAMETEXT_AUTHOR               "Author: "
#define GAMETEXT_BESTTIMES            "Best Times"
#define GAMETEXT_BRAKE                "Brake"
#define GAMETEXT_BUILTINLEVELS        "Built-In Levels"     
#define GAMETEXT_BY                   "by"
#define GAMETEXT_CANCEL               "Cancel"
#define GAMETEXT_CHANGE               "Change..."
#define GAMETEXT_CHANGEDIR            "Change direction"
#define GAMETEXT_CHANGEKEY            "Change Key..."
#define GAMETEXT_CHOOSELEVEL          "Choose Level"
#define GAMETEXT_CLOSE                "Close"
#define GAMETEXT_COMPLETED            " (Completed)"
#define GAMETEXT_CONFIGUREJOYSTICK    "Configure Joystick..."
#define GAMETEXT_CONTROLS             "Controls"
#define GAMETEXT_CURPLAYER            "Player:"
#define GAMETEXT_DATE                 "Date: "
#define GAMETEXT_DEFAULTS             "Defaults"
#define GAMETEXT_DELETEPLAYERMESSAGE  "Do you really want to delete player?"
#define GAMETEXT_DELETEREPLAYMESSAGE  "Do you really want to delete replay?"
#define GAMETEXT_DELETE               "Delete"
#define GAMETEXT_DELETEPROFILE        "Delete"
#define GAMETEXT_DESCRIPTION          "Description: "
#define GAMETEXT_DLHIGHSCORES         "Downloading high-scores..."
#define GAMETEXT_DOWNLOADLEVELS       "Get More Levels!"
#define GAMETEXT_DRIVE                "Drive"
#define GAMETEXT_ENABLEAUDIO          "Enable Audio"
#define GAMETEXT_ENABLECONTEXTHELP    "Enable Context Help"
#define GAMETEXT_ENABLEENGINESOUND    "Enable Engine Sound"
#define GAMETEXT_ENABLEINGAMEWORLDRECORD "Show World Record in-game"
#define GAMETEXT_ENABLEWEBHIGHSCORES  "Enable WWW Access"
#define GAMETEXT_ENTERPLAYERNAME      "Enter player name:"
#define GAMETEXT_ENTERREPLAYNAME      "Enter name of replay:"
#define GAMETEXT_ERRORSINLEVEL        "There's errors in the level, don't expect it to playable!"
#define GAMETEXT_EXTERNALLEVELS       "External Levels"     
#define GAMETEXT_FAILEDTOSAVEREPLAY   "Failed to save replay!\nMaybe you should try with another name?"
#define GAMETEXT_FILE                 "File"
#define GAMETEXT_FINISH               "Finished!"
#define GAMETEXT_FINISHTIME           "Finish Time"
#define GAMETEXT_FLIPLEFT             "Flip left"
#define GAMETEXT_FLIPRIGHT            "Flip right"
#define GAMETEXT_GAMEGFX              "Game Graphics:"
#define GAMETEXT_GENERAL              "General"
#define GAMETEXT_GENERALINFO          "General Info"
#define GAMETEXT_HELP                 "Help"

#if defined(ENABLE_ZOOMING)
  #define GAMETEXT_HELPTEXT(accelerate_KEY, brake_KEY, rotate_counter_clockwise_KEY, rotate_clockwise_KEY, change_direction_KEY, zoom_in_KEY, zoom_out_KEY)            \
                                        "You control your bike using the keyboard:\n"                      \
                                        "\n"                                                               \
                                        "  " + accelerate_KEY + " - Accelerates\n"                                       \
                                        "  " + brake_KEY + " - Brakes\n"                                          \
                                        "  " + rotate_counter_clockwise_KEY + " - Rotates it counter-clockwise\n"                    \
                                        "  " + rotate_clockwise_KEY + " - Rotates it clockwise\n"                           \
                                        "  " + change_direction_KEY + " - Turns around and drives in the other direction\n"       \
                                        "  " + zoom_in_KEY + " - Zoom in\n" \
                                        "  " + zoom_out_KEY + " - Zoom out\n"       \
                                        "\n" \
                                        "Find all the strawberries and touch the flower to finish\n"       \
                                        "the level.\n"                                                     \
                                        "\n"                                                               \
                                        "Read the README file or check out the website at\n"               \
                                        "http://xmoto.sourceforce.net for more information."
#else
  #define GAMETEXT_HELPTEXT(accelerate_KEY, brake_KEY, rotate_counter_clockwise_KEY, rotate_clockwise_KEY, change_direction_KEY)   \
                                        "You control your bike using the keyboard:\n"                      \
                                        "\n"                                                               \
                                        "  " + accelerate_KEY + " - Accelerates\n"                                       \
                                        "  " + brake_KEY + " - Brakes\n"                                          \
                                        "  " + rotate_counter_clockwise_KEY + " - Rotates it counter-clockwise\n"                    \
                                        "  " + rotate_clockwise_KEY + " - Rotates it clockwise\n"                           \
                                        "  " + change_direction_KEY + " - Turns around and drives in the other direction\n"       \
                                        "\n" \
                                        "Find all the strawberries and touch the flower to finish\n"       \
                                        "the level.\n"                                                     \
                                        "\n"                                                               \
                                        "Read the README file or check out the website at\n"               \
                                        "http://xmoto.sourceforce.net for more information."

#endif

#define GAMETEXT_HIGH                 "High"
#define GAMETEXT_INITINPUT            "Initializing input system..."
#define GAMETEXT_INITMENUS            "Initializing menus..."
#define GAMETEXT_INITRENDERER         "Initializing renderer..."
#define GAMETEXT_INITTEXT             "Initializing text renderer..."
#define GAMETEXT_JOYSTICK             "Joystick"
#define GAMETEXT_JUSTDEAD             "Oops!"
#define GAMETEXT_KEY                  "Key"
#define GAMETEXT_KEYBOARD             "Keyboard"
#define GAMETEXT_LEVEL                "Level"
#define GAMETEXT_LEVELINFO            "Level Info..."
#define GAMETEXT_LEVELISSCRIPTED      "(Scripted level)"
#define GAMETEXT_LEVELNAME            "Level Name: "
#define GAMETEXT_LEVELNOTFOUND        "Level '%s' not found!"
#define GAMETEXT_LEVELPACK            "Level Pack"
#define GAMETEXT_LEVELPACKS           "Level Packs"
#define GAMETEXT_LEVELREQUIREDBYREPLAY "Level '%s' required by replay!"
#define GAMETEXT_LEVELS               "Levels"
#define GAMETEXT_LISTALL              "List All"
#define GAMETEXT_LOADINGLEVELS        "Loading levels..."
#define GAMETEXT_LOADINGMENUGRAPHICS  "Loading menu graphics..."
#define GAMETEXT_LOADINGSOUNDS        "Loading sounds..."
#define GAMETEXT_LOADINGTEXTURES      "Loading textures..."
#define GAMETEXT_LOW                  "Low"
#define GAMETEXT_MEDIUM               "Medium"
#define GAMETEXT_MENUGFX              "Menu Graphics:"
#define GAMETEXT_MODIFYSELECTED       "Modify Selected..."
#define GAMETEXT_MONO                 "Mono"
#define GAMETEXT_NA                   "N/A"
#define GAMETEXT_NAME                 "Name"
#define GAMETEXT_NEWPROFILE           "New Profile..."
#define GAMETEXT_NO                   "No"
#define GAMETEXT_NONE                 "(None)"
#define GAMETEXT_NONEXTLEVEL          "No level following this one, sorry."
#define GAMETEXT_NOTFINISHED          "(Not finished)"
#define GAMETEXT_NOTIFYATINIT         "Important note!\n"                                                \
                                      "\n"                                                               \
                                      "This is an alpha release of X-Moto, which means that the\n"       \
                                      "game you're experiencing right now indeed isn't the\n"            \
                                      "final version of the game.\n"                                     \
                                      "All kinds of feedback are highly appreciated, so the game\n"      \
                                      "can get better.\n"                                                \
                                      "Mail bugs, ideas, comments, feature requests, hatemail, etc\n"    \
                                      "to neckelmann@gmail.com\n"                                        \
                                      "\n"                                                               \
                                      "Also visit http://xmoto.sourceforge.net to make sure you've\n"    \
                                      "got the latest version."
#define GAMETEXT_NUMLEVELS            "# Levels"
#define GAMETEXT_OK                   "OK"     
#define GAMETEXT_OPEN                 "Open"
#define GAMETEXT_OPTIONS              "Options"
#define GAMETEXT_OPTIONSREQURERESTART "Some options will not take effect before next restart!"
#define GAMETEXT_PAUSE                "Pause"
#define GAMETEXT_PERSONAL             "Personal"
#define GAMETEXT_PLAYER               "Player"
#define GAMETEXT_PLAYERPROFILE        "Player Profile"
#define GAMETEXT_PLAYERPROFILES       "Player Profiles"
#define GAMETEXT_PLAYNEXT             "Play Next Level"
#define GAMETEXT_PRESSANYKEYTO        "Press key you want to '%s' or ESC to cancel..."
#define GAMETEXT_QUIT                 "Quit Game"
#define GAMETEXT_QUITMESSAGE          "Do you really want to quit?"
#define GAMETEXT_REPLAY               "Replay"
#define GAMETEXT_REPLAYHELPTEXT       "  ESC: Stop   Left arrow key: Rewind   Right arrow key: Fast Forward"
#define GAMETEXT_REPLAYS              "View Replays"
#define GAMETEXT_RESTART              "Restart This Level"
#define GAMETEXT_RESUME               "Resume Playing"
#define GAMETEXT_RUNWINDOWED          "Run Windowed"
#define GAMETEXT_SAVE                 "Save"
#define GAMETEXT_SAVEREPLAY           "Save Replay"
#define GAMETEXT_SCREENRES            "Screen Resolution"
#define GAMETEXT_SHOW                 "Show"
#define GAMETEXT_SHOWINFO             "Info..."
#define GAMETEXT_SHOWMINIMAP          "Show Mini Map"
#define GAMETEXT_SCRIPTED             "Scripted"
#define GAMETEXT_SKIPPED              " (Skipped)"
#define GAMETEXT_STARTLEVEL           "Play!"
#define GAMETEXT_STEREO               "Stereo"
#define GAMETEXT_TIME                 "Time:"
#define GAMETEXT_TRYAGAIN             "Try This Level Again"
#define GAMETEXT_TUTORIAL             "Tutorial"
#define GAMETEXT_UNKNOWNLEVEL         "(Unknown)"
#define GAMETEXT_UPDATINGLEVELS       "Updating level lists..."
#define GAMETEXT_USEPROFILE           "Use Profile"
#define GAMETEXT_VIDEO                "Video"
#define GAMETEXT_VIEW                 "View"
#define GAMETEXT_WARNING              "Warning:"
#define GAMETEXT_WORLDRECORD          "World Record: "
#define GAMETEXT_YES                  "Yes"
#define GAMETEXT_ZOOMIN  	      "Zoom in"
#define GAMETEXT_ZOOMOUT 	      "Zoom out"


/* Context help strings */
#define CONTEXTHELP_PLAY_THIS_LEVEL_AGAIN "Play this level again"
#define CONTEXTHELP_SAVE_A_REPLAY "Save a replay for later viewing"
#define CONTEXTHELP_PLAY_NEXT_LEVEL "Play next level"
#define CONTEXTHELP_BACK_TO_MAIN_MENU "Back to the main menu"
#define CONTEXTHELP_QUIT_THE_GAME "Quit the game"
#define CONTEXTHELP_BACK_TO_GAME "Back to the game"
#define CONTEXTHELP_TRY_LEVEL_AGAIN_FROM_BEGINNING "Try this level again from the beginning"
#define CONTEXTHELP_PLAY_NEXT_INSTEAD "Play next level instead"
#define CONTEXTHELP_TRY_LEVEL_AGAIN "Try this level again"
#define CONTEXTHELP_BUILT_IN_AND_EXTERNALS "Built-in and stand-alone external levels"
#define CONTEXTHELP_LEVEL_PACKS "Levels grouped together in level packs"
#define CONTEXTHELP_REPLAY_LIST "View list of recorded replays"
#define CONTEXTHELP_OPTIONS "Configure X-Moto preferences"
#define CONTEXTHELP_HELP "Instructions of how to play X-Moto"
#define CONTEXTHELP_CHANGE_PLAYER "Change player profile"
#define CONTEXTHELP_TUTORIAL "Play tutorial of how to play the game"
#define CONTEXTHELP_OFFICIAL_LEVELS "Official built-in levels"
#define CONTEXTHELP_EXTERNAL_LEVELS "Unofficial stand-alone external levels"
#define CONTEXTHELP_PLAY_SELECTED_LEVEL "Play the selected level"
#define CONTEXTHELP_LEVEL_INFO "View general information about the level, best times, and replays"
#define CONTEXTHELP_RUN_REPLAY "Run the selected replay"
#define CONTEXTHELP_DELETE_REPLAY "Permanently delete the selected replay"
#define CONTEXTHELP_ALL_REPLAYS "Show replays of all players in list"
#define CONTEXTHELP_GENERAL_OPTIONS "General X-Moto preferences"
#define CONTEXTHELP_VIDEO_OPTIONS "Configure graphical options"
#define CONTEXTHELP_AUDIO_OPTIONS "Configure audio options"
#define CONTEXTHELP_CONTROL_OPTIONS "Configure control options"
#define CONTEXTHELP_SAVE_OPTIONS "Save options"
#define CONTEXTHELP_DEFAULTS "Revert options to defaults"
#define CONTEXTHELP_MINI_MAP "Show a map of your surroundings when playing"
#define CONTEXTHELP_DOWNLOAD_BEST_TIMES "Automatically download best times off the net when the game starts"
#define CONTEXTHELP_INGAME_WORLD_RECORD "Show the World Record for a given level when playing"
#define CONTEXTHELP_HIGHCOLOR "Enable high-color graphics"
#define CONTEXTHELP_TRUECOLOR "Enable true-color graphics"
#define CONTEXTHELP_RESOLUTION "Select graphics resolution"
#define CONTEXTHELP_RUN_IN_WINDOW "Run the game in a window"
#define CONTEXTHELP_LOW_MENU "Not so fancy menu graphics"
#define CONTEXTHELP_MEDIUM_MENU "A bit more fancy menu graphics"
#define CONTEXTHELP_HIGH_MENU "Fanciest menu graphics"
#define CONTEXTHELP_LOW_GAME "Disable most graphics not important to the gameplay"
#define CONTEXTHELP_MEDIUM_GAME "Disable some of the most resource-intensive graphics, like particles"
#define CONTEXTHELP_HIGH_GAME "Enable all graphical effects"
#define CONTEXTHELP_SOUND_ON "Turn on sound effects"
#define CONTEXTHELP_11HZ "Poor sound quality"
#define CONTEXTHELP_22HZ "Normal sound quality"
#define CONTEXTHELP_44HZ "Best sound quality"
#define CONTEXTHELP_8BIT "8-bit sound samples, poor quality"
#define CONTEXTHELP_16BIT "16-bit sound samples, good quality"
#define CONTEXTHELP_MONO "Mono (single channel) audio"
#define CONTEXTHELP_STEREO "Stereo (two channel) audio"
#define CONTEXTHELP_ENGINE_SOUND "Turn on engine noise"
#define CONTEXTHELP_SELECT_ACTION "Select action to re-configure to another key"
#define CONTEXTHELP_VIEW_LEVEL_PACK "View contents of level pack"
#define CONTEXTHELP_SELECT_PLAYER_PROFILE "Select a player profile to use"
#define CONTEXTHELP_USE_PLAYER_PROFILE "Use the selected player profile"
#define CONTEXTHELP_CREATE_PLAYER_PROFILE "Create new player profile"
#define CONTEXTHELP_CLOSE_PROFILE_EDITOR "Close profile editor"
#define CONTEXTHELP_DELETE_PROFILE "Permanently delete selected player profile, including best times"
#define CONTEXTHELP_SELECT_LEVEL_IN_LEVEL_PACK "Select a level in the level pack to play"
#define CONTEXTHELP_CLOSE_LEVEL_PACK "Close level pack"
#define CONTEXTHELP_GENERAL_INFO "General information about the level"
#define CONTEXTHELP_BEST_TIMES_INFO "View best times for the level"
#define CONTEXTHELP_REPLAYS_INFO "View locally stored replays of the level"
#define CONTEXTHELP_ONLY_SHOW_PERSONAL_BESTS "Only show personal best times for this level"
#define CONTEXTHELP_SHOW_ALL_BESTS "Show all best times for this level"
#define CONTEXTHELP_ONLY_SHOW_PERSONAL_REPLAYS "Only show personal replays for this level"
#define CONTEXTHELP_SHOW_ALL_REPLAYS "Show all replays for this level"
#define CONTEXTHELP_RUN_SELECTED_REPLAY "Run selected replay"
#define CONTEXTHELP_SHOWCONTEXTHELP "Show helpful help strings such as this one"
#define CONTEXTHELP_DOWNLOADLEVELS "Let X-Moto look for more levels on the net, and install them automatically"

#endif
