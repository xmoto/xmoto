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

#ifndef __EDITORDATA_H__
#define __EDITORDATA_H__

#include "VApp.h"
#include "VXml.h"
#include "helpers/VMath.h"

namespace vapp {

	/*============================================================================
	Editor entity type param
	============================================================================*/
	struct ETParam {
		std::string Name;										/* Name of parameter */
		std::string DefaultValue;						/* Its default value */
	};

	/*============================================================================
	Editor entity drawing procedure element types
	============================================================================*/
	enum ETDrawType {
		ET_CIRCLE,													/* Draw a circle */
		ET_TEXT,														/* Draw a text string */
		ET_RECT															/* Draw a rectangle */
	};

	/*============================================================================
	Editor entity drawing procedure element allignments
	============================================================================*/
	enum ETAllign {
		ET_ALLIGN_LEFT,ET_ALLIGN_RIGHT,ET_ALLIGN_TOP,ET_ALLIGN_BOTTOM,
		ET_ALLIGN_CENTER
	};

	/*============================================================================
	Editor entity drawing procedure element
	============================================================================*/
	struct ETDraw {
	  ETDraw() {
	    Type = ET_CIRCLE;
	    fBorder = 0.0f;
	    BColor = FColor = -1;
	    fRadius = 1.0f;
	    Allign = VAllign = ET_ALLIGN_CENTER;
	    fWidth = fHeight = 1.0f;
	  }
	
		ETDrawType Type;
		
		/* Common */
		Vector2f Position;
		float fBorder;
		Color BColor,FColor;
		
		/* ET_CIRCLE */
		float fRadius;
		
		/* ET_TEXT */
		ETAllign Allign,VAllign;
		std::string Caption;
		
		/* ET_RECT */
		float fWidth,fHeight;
	};

	/*============================================================================
	Editor entity type
	============================================================================*/
	struct EditorEntityType {
	  EditorEntityType() {
	    fSize = fPSize = 1.0f;
	  }
	
		std::string ID;											/* Identifier */
		float fSize;												/* Size (for selection) */
		float fPSize;                       /* Physical size (in-game) */
		std::vector<ETDraw *> DrawProc;			/* Drawing procedure */
		std::vector<ETParam *> Params;			/* Parameters */
	};

	/*============================================================================
	Editor entity type database (XML based)
	============================================================================*/
	class EditorData {
		public:
			~EditorData() {_Free();}
		
			void loadFile(std::string Path);
			EditorEntityType *newType(std::string ID);
			ETParam *newParam(EditorEntityType *pType,std::string Name,std::string DefaultValue);
			ETDraw *newDraw(EditorEntityType *pType,ETDrawType DrawType);
			EditorEntityType *getTypeByID(std::string ID);			
			ETParam *getParamByName(EditorEntityType *pType,std::string Name);
			void consoleDump(void);
			
			/* Data interface */
			std::vector<EditorEntityType *> &getTypes(void) {return m_EntityTypes;}
					
		private:
			/* Data */
			std::vector<EditorEntityType *> m_EntityTypes;	
			
			/* Helpers */
			void _Free(void);
	};
	
}

#endif

