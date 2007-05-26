/*=============================================================================
XMOTO
Copyright (C) 2007 Jan Polak (benetnash@mail.icpnet.pl)

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

#ifndef __LEVEL_OBJECT_H__
#define __LEVEL_OBJECT_H__

#include "helpers/VMath.h"
/*
benetnash: I assumed following depth ranges:
(object with bigger depth will be covered by object with smaller)
*/

/* each background layer will be deeper this value*/
#define DEPTH_LAYER_GAIN	100



enum LevelObjectDepth{
	OBJECT_DEPTH_BACK_LAYERS = 1000,
	OBJECT_DEPTH_BACK_BLOCKS = 500,
	OBJECT_DEPTH_BACK_SPRITES = 400,
	OBJECT_DEPTH_BACK_PARTICLES = 300,
	OBJECT_DEPTH_BLOCKS = 200,
 	OBJECT_DEPTH_MIDDLE_SPRITES = 100,
	OBJECT_DEPTH_BIKE = 0,
	OBJECT_DEPTH_FRONT_SPRITES = -100,
	OBJECT_DEPTH_FRONT_PARTICLES = -200,
	OBJECT_DEPTH_FRONT_LAYERS = -1000,
};

/* declaration of used classes */
namespace vapp {
	class GameRenderer;
};

class LevelObject {
	public:
		LevelObject(bool is_pseudoClass = false);
		
		void setDepth(float _z);
		float getDepth();
		
		/* set depth depending on object type/state/etc*/
		virtual void setDepthAuto() = 0;
		
		/* we need this function because Block must overload it - 
		with block we have also to compare textures*/
		virtual int compareDepths(LevelObject* lo);
		
		virtual void render(vapp::GameRenderer* r) = 0;
		virtual bool isVisible(AABB& screenBox);
		
		bool isOnMiniMap();
		void setOnMiniMap(bool val);
		
		bool isPseudoClass();
		
		virtual AABB& getAABB();
		
	protected:
		AABB        m_BBox;
		bool        m_isBBoxDirty;
		
		/* pseudo calss must be deleted at the end of level*/
		const bool	pseudoClass;
		
	private:
		/* depth of object */
		float __z;
		bool m_isOnMiniMap;
};

/* used for sorting levels */
inline bool LevelObjectComparer(LevelObject* lo1, LevelObject* lo2)
{ 
	return lo1->compareDepths(lo2);
}

/* pseudo-object used for rendering bikes and ghosts */
class PseudoObjectBikes: public LevelObject {
	public:
		PseudoObjectBikes();
		virtual void render(vapp::GameRenderer* r);
		
		/* inline to speed-up */
		virtual bool isVisible(AABB& screenBox) {
			return true;
		}
		
		virtual void setDepthAuto();
};

/* pseudo-object used for rendering entities which dynamicly 
 appear on the screen */
class PseudoObjectExternEntities: public LevelObject {
	public:
		PseudoObjectExternEntities();
		virtual void render(vapp::GameRenderer* r);
		
		/* inline to speed-up */
		virtual bool isVisible(AABB& screenBox) {
			return true;
		}
		
		virtual void setDepthAuto();
};

#endif /* __LEVEL_OBJECT_H__ */
