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

#ifndef __CHIPMUNKHELPER_H__
#define __CHIPMUNKHELPER_H__

class cpSpace;
class cpBody;

class ChipmunkHelper
{
public:
	static ChipmunkHelper *Instance();
	void reInstance();
	~ChipmunkHelper() {};
	cpSpace *getSpace();
	void setSpace(cpSpace* s);

	cpBody *getStaticBody();
	cpBody *getFrontWheel();
	cpBody *getBackWheel();
	void setStaticBody(cpBody *body);
	void setFrontWheel(cpBody *body);
	void setBackWheel(cpBody *body);
	void setGravity(float i_x, float i_y);

private:
	static ChipmunkHelper *mp_instance;
	void initPhysics();
	cpSpace *m_space;
	cpBody *m_body;

        cpBody *m_ab;	// wheel anchors
        cpBody *m_af;
	cpBody *m_wb;	// wheel bodies
	cpBody *m_wf;
	
};

#endif
