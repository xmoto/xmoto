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

#ifndef __STATEUPDATETHEMES_H__
#define __STATEUPDATETHEMES_H__

#include "StateUpdate.h"

class UpdateThemeThread;

class StateUpdateTheme : public StateUpdate {
public:
  StateUpdateTheme(GameApp* pGame,
		   const std::string& i_id_theme,
		   bool drawStateBehind    = true,
		   bool updateStatesBehind = false);
  virtual ~StateUpdateTheme();

  virtual void enter();
  virtual bool update();

  virtual void keyDown(int nKey, SDLMod mod,int nChar);

  void send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input);

protected:
  UpdateThemeThread* m_pThread;

 private:
  std::string m_id_theme;
};

#endif
