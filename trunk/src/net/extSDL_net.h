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

#ifndef __EXTSDL_NET_H__
#define __EXTSDL_NET_H__

#include "../include/xm_SDL_net.h"

/*
  from SDL_net-1.2.7
  this file add SDLNet_TCP_Send_noBlocking function to sdl_net
  because the function SDLNet_TCP_Send is blocking
  only the MSG_NOWAIT flag was added
*/

int SDLNet_TCP_Send_noBlocking(TCPsocket sock, const void *datap, int len);

#endif
