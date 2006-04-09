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
#define GAMETEXT_DRIVE                "Drive"
#define GAMETEXT_ENABLEAUDIO          "Enable Audio"
#define GAMETEXT_ENABLEENGINESOUND    "Enable Engine Sound"
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
#define GAMETEXT_HELPTEXT             "You control your bike using the keyboard:\n"                      \
                                      "\n"                                                               \
                                      "  Up arrow - Accelerates\n"                                       \
                                      "  Down arrow - Brakes\n"                                          \
                                      "  Left arrow - Rotates it counter-clockwise\n"                    \
                                      "  Right arrow - Rotates it clockwise\n"                           \
                                      "  Space - Turns around and drives in the other direction\n"       \
                                      "\n"                                                               \
                                      "Find all the strawberries and touch the flower to finish\n"       \
                                      "the level.\n"                                                     \
                                      "\n"                                                               \
                                      "Note that the above key assignments can be configured\n"          \
                                      "to suit your needs.\n"                                            \
                                      "\n"                                                               \
                                      "Read the README file or check out the website at\n"               \
                                      "http://xmoto.sourceforce.net for more information."
#define GAMETEXT_HIGH                 "High"
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
#define GAMETEXT_LOW                  "Low"
#define GAMETEXT_MEDIUM               "Medium"
#define GAMETEXT_MENUGFX              "Menu Graphics:"
#define GAMETEXT_MODIFYSELECTED       "Modify Selected..."
#define GAMETEXT_MONO                 "Mono"
#define GAMETEXT_NAME                 "Name"
#define GAMETEXT_NEWPROFILE           "New Profile..."
#define GAMETEXT_NO                   "No"
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
#define GAMETEXT_TRYAGAIN             "Try This Level Again"
#define GAMETEXT_UNKNOWNLEVEL         "(Unknown)"
#define GAMETEXT_USEPROFILE           "Use Profile"
#define GAMETEXT_VIDEO                "Video"
#define GAMETEXT_VIEW                 "View"
#define GAMETEXT_WARNING              "Warning:"
#define GAMETEXT_YES                  "Yes"

#endif