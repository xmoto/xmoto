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

#include "extSDL_net.h"
#include <errno.h>

// read the .h to understand why i redefine SDLNet_TCP_Send
#if !defined(WIN32) && !defined(__APPLE__)
#include <sys/socket.h>

#define SOCKET	int

struct _TCPsocket {
	int ready;
	SOCKET channel;
	IPaddress remoteAddress;
	IPaddress localAddress;
	int sflag;
};

int SDLNet_TCP_Send_noBlocking(TCPsocket sock, const void *datap, int len)
{
	const Uint8 *data = (const Uint8 *)datap;	/* For pointer arithmetic */
	int sent, left;

	/* Server sockets are for accepting connections only */
	if ( sock->sflag ) {
		SDLNet_SetError("Server sockets cannot send");
		return(-1);
	}

	/* Keep sending data until it's sent or an error occurs */
	left = len;
	sent = 0;
	errno = 0;
	do {
		len = send(sock->channel, (const char *) data, left, MSG_DONTWAIT);
		if ( len > 0 ) {
			sent += len;
			left -= len;
			data += len;
		}
	} while ( (left > 0) && ((len > 0) || (errno == EINTR)) );

	return(sent);
}

#else
// i don't know whether it's blocking or not ; i mainly want it works for the servers on linux
int SDLNet_TCP_Send_noBlocking(TCPsocket sock, const void *datap, int len) {
  return SDLNet_TCP_Send(sock, datap, len);
}
#endif
