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
 *  In-game rendering (init part)
 */
#include "VXml.h"
#include "VFileIO.h"
#include "xmscene/Scene.h"
#include "Renderer.h"
#include "GameText.h"
#include "drawlib/DrawLib.h"
#include "Game.h"

  /*===========================================================================
  Init at game start-up
  ===========================================================================*/
  void GameRenderer::init(void) { 
          
    /* Init GUI */
    getGUI()->setApp(getParent());
    getGUI()->setPosition(0,0,getParent()->getDrawLib()->getDispWidth(),getParent()->getDrawLib()->getDispHeight());
    getGUI()->setFont(getParent()->getDrawLib()->getFontSmall());    

    m_pInGameStats = new UIWindow(getGUI(),0,0,"",
				  getParent()->getDrawLib()->getDispWidth(),
				  getParent()->getDrawLib()->getDispHeight());
    m_pInGameStats->showWindow(false);

    m_playTimes.push_back(new UIStatic(m_pInGameStats,0,0,"00:00:00",200,25));
    m_playTimes[0]->setFont(getParent()->getDrawLib()->getFontMedium());
    m_playTimes[0]->setVAlign(UI_ALIGN_TOP);
    m_playTimes[0]->setHAlign(UI_ALIGN_LEFT);
    m_pBestTime   = new UIStatic(m_pInGameStats,0,28,"--:--:-- / --:--:--",800,20);
    m_pBestTime->setFont(getParent()->getDrawLib()->getFontSmall());
    m_pBestTime->setVAlign(UI_ALIGN_TOP);
    m_pBestTime->setHAlign(UI_ALIGN_LEFT);
    m_pBestTime->setContextHelp("Personal best time / best time on this computer");
    m_pReplayHelp = new UIStatic(m_pInGameStats, 150, 0, "", 640, 20);
    m_pReplayHelp->setFont(getParent()->getDrawLib()->getFontSmall());
    m_pReplayHelp->setVAlign(UI_ALIGN_TOP);
    m_pReplayHelp->setHAlign(UI_ALIGN_RIGHT);

    m_pWorldRecordTime = new UIStatic(m_pInGameStats,0,48,"",800,20);
    m_pWorldRecordTime->setFont(getParent()->getDrawLib()->getFontSmall());
    m_pWorldRecordTime->setVAlign(UI_ALIGN_TOP);
    m_pWorldRecordTime->setHAlign(UI_ALIGN_LEFT);

    m_pSpeed = new UIStatic(m_pInGameStats,0,60,"",60,20);
    m_pSpeed->setFont(getParent()->getDrawLib()->getFontSmall());
    m_pSpeed->setVAlign(UI_ALIGN_TOP);
    m_pSpeed->setHAlign(UI_ALIGN_RIGHT);

    /* new highscore ! */
    m_pInGameNewHighscore = new UIWindow(getGUI(),405,475,"",200,100);
    m_pInGameNewHighscore->showWindow(false);

    m_pNewHighscorePersonal_str = new UIStatic(m_pInGameNewHighscore,
					       0, 5,
					       GAMETEXT_NEWHIGHSCOREPERSONAL,
					       200, 20);
    m_pNewHighscorePersonal_str->setFont(getParent()->getDrawLib()->getFontSmall());
    m_pNewHighscorePersonal_str->setHAlign(UI_ALIGN_CENTER);
    m_pNewHighscorePersonal_str->showWindow(false);

    m_pNewHighscoreBest_str = new UIStatic(m_pInGameNewHighscore,
					   0, 0,
					   GAMETEXT_NEWHIGHSCORE,
					   200, 30);
    m_pNewHighscoreBest_str->setFont(getParent()->getDrawLib()->getFontMedium());
    m_pNewHighscoreBest_str->setHAlign(UI_ALIGN_CENTER);
    m_pNewHighscoreBest_str->showWindow(false);

    m_pNewHighscoreSave_str = new UIStatic(m_pInGameNewHighscore,
					   0, 25,
					   "",
					   200, 20);
    m_pNewHighscoreSave_str->setFont(getParent()->getDrawLib()->getFontSmall());
    m_pNewHighscoreSave_str->setHAlign(UI_ALIGN_CENTER);
    m_pNewHighscoreSave_str->showWindow(false);

    /* Overlays? */
    m_Overlay.init(getParent()->getDrawLib(),512,512);

    m_nParticlesRendered = 0;

  }

