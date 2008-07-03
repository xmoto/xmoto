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
class Universe;

class StateFinished : public StateMenu {
  public:
  StateFinished(Universe* i_universe,
		StateMenuContextReceiver* i_receiver = NULL,
		bool drawStateBehind    = true,
		bool updateStatesBehind = false
		);
  virtual ~StateFinished();
  
  virtual void enter();
  virtual void leave();
  
  /* input */
  virtual void keyDown(int nKey, SDLMod mod,int nChar, const std::string& i_utf8Char);
  
  static void clean();

  virtual void send(const std::string& i_id,
		    UIMsgBoxButton i_button,
		    const std::string& i_input);
  
 protected:
  virtual void checkEvents();
  Universe* m_universe;
  std::string m_roomsTimes;

  private:
  void makeBestTimesWindow(UIBestTimes* pWindow,
			   const std::string& PlayerName,
			   const std::string& LevelID,
			   int i_finishTime);      

  /* GUI */
  static UIRoot* m_sGUI;
  static void createGUIIfNeeded();
};

#endif
