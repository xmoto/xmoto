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

#include "StateHelp.h"
#include "StatePreplayingCredits.h"
#include "StatePreplayingGame.h"
#include "drawlib/DrawLib.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"

StateHelp::StateHelp(bool drawStateBehind,
                     bool updateStatesBehind,
                     bool i_gameHelp,
                     bool i_allowSceneOver)
  : StateMenu(drawStateBehind, updateStatesBehind) {
  m_name = "StateHelp";
  m_gameHelp = i_gameHelp;
  m_allowSceneOver = i_allowSceneOver;
}

StateHelp::~StateHelp() {}

void StateHelp::enterAfterPop() {
  StateMenu::enterAfterPop();
}

void StateHelp::enter() {
  createGUI(); // create the gui each time because it's small and keys can
  // change
}

void StateHelp::leave() {
  StateMenu::leave();
  delete m_GUI;
}

void StateHelp::checkEvents() {
  if (m_allowSceneOver) {
    /* Find first tutorial level */
    UIButton *pTutorialButton =
      (UIButton *)m_GUI->getChild("FRAME:TUTORIAL_BUTTON");
    if (pTutorialButton->isClicked()) {
      pTutorialButton->setClicked(false);

      try {
        GameApp::instance()->setCurrentPlayingList(NULL);
        StateManager::instance()->pushState(
          new StatePreplayingGame("tut1", false));
      } catch (Exception &e) {
      }
    }
    /* View credits? */
    UIButton *pCreditsButton =
      (UIButton *)m_GUI->getChild("FRAME:CREDITS_BUTTON");
    if (pCreditsButton->isClicked()) {
      pCreditsButton->setClicked(false);

      try {
        StateManager::instance()->pushState(
          new StatePreplayingCredits("credits.rpl"));
      } catch (Exception &e) {
      }
    }
  }

  /* Close */
  UIButton *pCloseButton = (UIButton *)m_GUI->getChild("FRAME:CLOSE_BUTTON");
  if (pCloseButton->isClicked()) {
    pCloseButton->setClicked(false);

    m_requestForEnd = true;
  }
}

void StateHelp::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  StateMenu::xmKey(i_type, i_xmkey);

  if (i_type == INPUT_DOWN &&
      (i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE) ||
       i_xmkey.getJoyButton() == SDL_CONTROLLER_BUTTON_B ||
       i_xmkey == (*Input::instance()->getGlobalKey(INPUT_HELP)))) {
    m_requestForEnd = true;
  }
}

void StateHelp::createGUI() {
  UIFrame *v_frame;
  UIStatic *v_someText;
  GameApp *pGame = GameApp::instance();
  DrawLib *drawLib = pGame->getDrawLib();

  m_GUI = new UIRoot(&m_screen);
  m_GUI->setFont(drawLib->getFontSmall());
  m_GUI->setPosition(0, 0, m_screen.getDispWidth(), m_screen.getDispHeight());

  int v_offsetX = m_screen.getDispWidth() / 10;
  int v_offsetY = m_screen.getDispHeight() / 10;

  v_frame = new UIFrame(m_GUI,
                        v_offsetX,
                        m_gameHelp ? v_offsetY / 2 : v_offsetY,
                        "",
                        m_screen.getDispWidth() - 2 * v_offsetX,
                        m_gameHelp ? m_screen.getDispHeight() - v_offsetY
                                   : m_screen.getDispHeight() - 2 * v_offsetY);
  v_frame->setID("FRAME");

  v_someText = new UIStatic(
    v_frame, 0, 0, GAMETEXT_HELP, v_frame->getPosition().nWidth, 36);
  v_someText->setFont(drawLib->getFontMedium());
  v_someText = new UIStatic(
    v_frame,
    10,
    40,
    m_gameHelp
      ?

      GAMETEXT_HELPTEXT_PLAYINGLEVEL(
        Input::instance()->getPlayerKey(INPUT_DRIVE, 0)->toFancyString(),
        Input::instance()->getPlayerKey(INPUT_BRAKE, 0)->toFancyString(),
        Input::instance()->getPlayerKey(INPUT_FLIPLEFT, 0)->toFancyString(),
        Input::instance()->getPlayerKey(INPUT_FLIPRIGHT, 0)->toFancyString(),
        Input::instance()->getPlayerKey(INPUT_CHANGEDIR, 0)->toFancyString(),
        Input::instance()->getGlobalKey(INPUT_RESTARTLEVEL)->toFancyString(),
        Input::instance()->getGlobalKey(INPUT_PREVIOUSLEVEL)->toFancyString(),
        Input::instance()->getGlobalKey(INPUT_NEXTLEVEL)->toFancyString(),
        Input::instance()->getGlobalKey(INPUT_SWITCHWWWACCESS)->toFancyString(),
        Input::instance()->getGlobalKey(INPUT_SCREENSHOT)->toFancyString(),
        Input::instance()->getGlobalKey(INPUT_LEVELINFO)->toFancyString(),
        Input::instance()->getGlobalKey(INPUT_SWITCHFPS)->toFancyString(),
        Input::instance()->getGlobalKey(INPUT_SWITCHUGLYMODE)->toFancyString(),
        Input::instance()->getGlobalKey(INPUT_SWITCHPLAYER)->toFancyString(),
        Input::instance()->getGlobalKey(INPUT_SWITCHFAVORITE)->toFancyString(),
        Input::instance()->getGlobalKey(INPUT_HELP)->toFancyString(),
        Input::instance()->getGlobalKey(INPUT_RELOADFILESTODB)->toFancyString(),
        Input::instance()->getGlobalKey(INPUT_PLAYINGPAUSE)->toFancyString(),
        Input::instance()
          ->getGlobalKey(INPUT_SWITCHTRACKINGSHOTMODE)
          ->toFancyString(),
        Input::instance()
          ->getGlobalKey(INPUT_SWITCHGFXQUALITYMODE)
          ->toFancyString(),
        Input::instance()->getGlobalKey(INPUT_SWITCHGFXMODE)->toFancyString())
      : GAMETEXT_HELPTEXT(
          Input::instance()->getPlayerKey(INPUT_DRIVE, 0)->toFancyString(),
          Input::instance()->getPlayerKey(INPUT_BRAKE, 0)->toFancyString(),
          Input::instance()->getPlayerKey(INPUT_FLIPLEFT, 0)->toFancyString(),
          Input::instance()->getPlayerKey(INPUT_FLIPRIGHT, 0)->toFancyString(),
          Input::instance()->getPlayerKey(INPUT_CHANGEDIR, 0)->toFancyString(),
          Input::instance()->getGlobalKey(INPUT_RESTARTLEVEL)->toFancyString(),
          Input::instance()->getGlobalKey(INPUT_PREVIOUSLEVEL)->toFancyString(),
          Input::instance()->getGlobalKey(INPUT_NEXTLEVEL)->toFancyString(),
          Input::instance()
            ->getGlobalKey(INPUT_SWITCHWWWACCESS)
            ->toFancyString(),
          Input::instance()->getGlobalKey(INPUT_SCREENSHOT)->toFancyString(),
          Input::instance()->getGlobalKey(INPUT_SWITCHFPS)->toFancyString(),
          Input::instance()
            ->getGlobalKey(INPUT_SWITCHUGLYMODE)
            ->toFancyString(),
          Input::instance()->getGlobalKey(INPUT_SWITCHPLAYER)->toFancyString(),
          Input::instance()
            ->getGlobalKey(INPUT_SWITCHFAVORITE)
            ->toFancyString(),
          Input::instance()->getGlobalKey(INPUT_HELP)->toFancyString(),
          Input::instance()
            ->getGlobalKey(INPUT_RELOADFILESTODB)
            ->toFancyString(),
          Input::instance()->getGlobalKey(INPUT_PLAYINGPAUSE)->toFancyString(),
          Input::instance()
            ->getGlobalKey(INPUT_SWITCHTRACKINGSHOTMODE)
            ->toFancyString(),
          Input::instance()
            ->getGlobalKey(INPUT_SWITCHGFXQUALITYMODE)
            ->toFancyString(),
          Input::instance()
            ->getGlobalKey(INPUT_SWITCHGFXMODE)
            ->toFancyString()),
    v_frame->getPosition().nWidth - 20,
    m_gameHelp ? v_frame->getPosition().nHeight - 46
               : v_frame->getPosition().nHeight - 56);
  v_someText->setFont(drawLib->getFontSmall());
  v_someText->setVAlign(UI_ALIGN_TOP);
  v_someText->setHAlign(UI_ALIGN_LEFT);

  if (m_allowSceneOver) {
    UIButton *pTutorialButton =
      new UIButton(v_frame,
                   v_frame->getPosition().nWidth - 240,
                   v_frame->getPosition().nHeight - 62,
                   GAMETEXT_TUTORIAL,
                   115,
                   57);
    pTutorialButton->setContextHelp(CONTEXTHELP_TUTORIAL);
    pTutorialButton->setFont(drawLib->getFontSmall());
    pTutorialButton->setType(UI_BUTTON_TYPE_SMALL);
    pTutorialButton->setID("TUTORIAL_BUTTON");

    UIButton *pCreditsButton = new UIButton(v_frame,
                                            v_frame->getPosition().nWidth - 360,
                                            v_frame->getPosition().nHeight - 62,
                                            GAMETEXT_CREDITSBUTTON,
                                            115,
                                            57);
    pCreditsButton->setContextHelp(CONTEXTHELP_CREDITS);
    pCreditsButton->setFont(drawLib->getFontSmall());
    pCreditsButton->setType(UI_BUTTON_TYPE_SMALL);
    pCreditsButton->setID("CREDITS_BUTTON");
  }

  UIButton *pCloseButton = new UIButton(v_frame,
                                        v_frame->getPosition().nWidth - 120,
                                        v_frame->getPosition().nHeight - 62,
                                        GAMETEXT_CLOSE,
                                        115,
                                        57);
  pCloseButton->setFont(drawLib->getFontSmall());
  pCloseButton->setType(UI_BUTTON_TYPE_SMALL);
  pCloseButton->setID("CLOSE_BUTTON");
}
