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

#ifndef __STATEFINISHED_H__
#define __STATEFINISHED_H__

#include "StateManager.h"
#include "StateMenu.h"

class UIRoot;
class UIBestTimes;
class xmDatabase;

class StateFinished : public StateMenu {
  public:
  StateFinished(GameApp* pGame,
		StateMenuContextReceiver* i_receiver = NULL,
		bool drawStateBehind    = true,
		bool updateStatesBehind = false
		);
  virtual ~StateFinished();
  
  virtual void enter();
  virtual void leave();
  /* called when a new state is pushed or poped on top of the
     current one*/
  virtual void enterAfterPop();
  virtual void leaveAfterPush();
  
  virtual bool update();
  virtual bool render();
  /* input */
  virtual void keyDown(int nKey, SDLMod mod,int nChar);
  virtual void keyUp(int nKey,   SDLMod mod);
  virtual void mouseDown(int nButton);
  virtual void mouseDoubleClick(int nButton);
  virtual void mouseUp(int nButton);
  
  static void clean();

  virtual void send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input);
  
 protected:
  virtual void checkEvents();

  private:
  void makeBestTimesWindow(UIBestTimes* pWindow, xmDatabase* i_db,
			   const std::string& PlayerName, const std::string& LevelID,
			   float fFinishTime, const std::string& TimeStamp);      

  /* GUI */
  static UIRoot* m_sGUI;
  static void createGUIIfNeeded(GameApp* pGame);
};

#endif
