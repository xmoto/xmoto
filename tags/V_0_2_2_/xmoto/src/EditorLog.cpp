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

/* 
 *  Handling of the log window of the editor.
 */
#define XMOTO_EDITOR
#include <stdio.h>
#include <stdarg.h>
#include "EditorLog.h"

namespace vapp {
  
  /*============================================================================
  Append message to the log
  ============================================================================*/
  void EditorLog::msg(char *pcFmt,...) {
		va_list List;
		char cBuf[256];
		
		va_start(List,pcFmt);
		vsprintf(cBuf,pcFmt,List);
		va_end(List);
		
		m_Messages.push_back(cBuf);
  }
  
  /*============================================================================
  Draw the log window
  ============================================================================*/
	void EditorLog::drawWindow(App *pApp,Vector2f Pos,Vector2f Size,Color BColor,Color FColor) {
		Vector2f CP;
		
		CP = Pos + Vector2f(0,Size.y-12);
	
		/* Output lines */
		for(int i=m_Messages.size()-1;i>=0;i--) {
			pApp->drawText(CP,m_Messages[i],BColor,FColor);
			CP = CP - Vector2f(0,12);
			if(CP.y < Pos.y) break;
		}
	}
	
  /*============================================================================
  Clear log
  ============================================================================*/
	void EditorLog::clear(void) {
		m_Messages.clear();
	}

}

