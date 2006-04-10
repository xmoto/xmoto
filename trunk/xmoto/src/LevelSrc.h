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

#ifndef __LEVELSRC_H__
#define __LEVELSRC_H__

#include "VApp.h"
#include "VXml.h"

namespace vapp {

	/*===========================================================================
  Zone primitive types
  ===========================================================================*/
  enum LevelZonePrimType {
    LZPT_UNASSIGNED = 0,
    LZPT_BOX
  };

	/*===========================================================================
  Zone primitive struct
  ===========================================================================*/
  struct LevelZonePrim {
    LevelZonePrim() {
      Type = LZPT_UNASSIGNED;
      fLeft = fRight = fTop = fBottom = 0.0f;
    }
  
    LevelZonePrimType Type;   /* Type of prim */
    
    /* For LZPT_BOX */
    float fLeft,fRight,fTop,fBottom;
  };

	/*===========================================================================
  Zone struct
  ===========================================================================*/
  struct LevelZone {
    LevelZone() {
      m_bInZone = false;
    }
  
    std::string ID;           /* Zone ID */
    std::vector<LevelZonePrim *> Prims; /* Primitives forming zone */
    
    /* For game internals only - sure, it's not cool to have them here, but
       it's easy :) */
    bool m_bInZone;
  };

	/*===========================================================================
  Vertex struct
  ===========================================================================*/
  struct LevelBlockVertex {
    LevelBlockVertex() {
      fX = fY = fTX = fTY = 0.0f;
      r = g = b = a = 0;
      bSelected = false;
    }
  
    float fX,fY;              /* Position relative to block center */
    float fTX,fTY;            /* Texture coordinates */
    int r,g,b,a;              /* Color/transluency */
    std::string EdgeEffect;   /* Apply this effect to the following edge */        
    
    /* For editor internals only */
    bool bSelected;
  };

	/*===========================================================================
  Block struct
  ===========================================================================*/
  struct LevelBlock {
    LevelBlock() {
      fTextureScale = 1.0f;
      fPosX = fPosY = 0.0f;
      bBackground = false;
      bWater = false;
    }
  
    std::string ID;           /* Block ID */
    std::string Texture;      /* Texture to use... */
    float fTextureScale;      /* Texture scaling constant to use */
    float fPosX,fPosY;        /* Position */
    std::vector<LevelBlockVertex *> Vertices; /* Vertices of block */
    bool bBackground;         /* Background block */
    bool bWater;              /* Water block */
  };

	/*===========================================================================
  Entity param struct
  ===========================================================================*/
	struct LevelEntityParam {
		std::string Name,Value;		/* Name/Value pair */
	};

	/*===========================================================================
  Entity struct
  ===========================================================================*/
	struct LevelEntity {
	  LevelEntity() {
	    fPosX = fPosY = 0.0f;
	    fSize = 1.0f;
	    bSelected = false;
	  }
	
		std::string ID;						/* Its own identifer */
		std::string TypeID;				/* Identifier of its type */
		float fPosX,fPosY;				/* Position */
		float fSize;              /* Size (radius) */
		std::vector<LevelEntityParam *> Params;	/* Parameters */
		bool bSelected;						/* Selected in editor */
	};

	/*===========================================================================
  Level info struct
  ===========================================================================*/
  struct LevelInfo {
    std::string Name;         /* Name of level */
    std::string Author;       /* Author of level */
    std::string Date;         /* When it was crafted */
    std::string Description;  /* Description */
    std::string Sky;          /* Level sky */
  };

	/*===========================================================================
  Level checksum struct
  ===========================================================================*/
  struct LevelCheckSum {
    unsigned long nCRC32;     /* Use CRC32 for now TODO: MD5? */
  };

	/*===========================================================================
	Level source object - holds all stored information about a level, used by 
	both the editor and the game itself.
  ===========================================================================*/
  class LevelSrc {
    public:
      LevelSrc() {}
      ~LevelSrc() {_UnloadLevelData();}
    
      /* Methods */
      void saveXML(void);
      void loadXML(void);
      
      bool probeCheckSum(LevelCheckSum *pSum);
      
      void exportBinary(const std::string &FileName,LevelCheckSum *pSum);
      bool importBinary(const std::string &FileName,LevelCheckSum *pSum);
      
      LevelEntity *createEntity(std::string TypeID,float x,float y);
                              
      /* Data interface */ 
      bool isScripted(void) {if(m_ScriptFile!="" || m_ScriptSource!="") return true; return false;}           
      std::string &getFileName(void) {return m_FileName;}
      void setFileName(const std::string &File) {m_FileName = File;}     
      LevelInfo *getLevelInfo(void) {return &m_Info;}
      std::string &getID(void) {return m_ID;}
      void setID(const std::string &ID) {m_ID = ID;}
      std::string &getScriptFile(void) {return m_ScriptFile;}
      std::string &getScriptSource(void) {return m_ScriptSource;}
      void setScriptFile(const std::string &ScriptFile) {m_ScriptFile = ScriptFile;}
      std::vector<LevelBlock *> &getBlockList(void) {return m_Blocks;}
      std::vector<LevelZone *> &getZoneList(void) {return m_Zones;}
      std::vector<LevelEntity *> &getEntityList(void) {return m_Entities;}
      float getLeftLimit(void) {return m_fLeftLimit;}
      float getRightLimit(void) {return m_fRightLimit;}
      float getTopLimit(void) {return m_fTopLimit;}
      float getBottomLimit(void) {return m_fBottomLimit;}      
      float getPlayerStartX(void) {return m_fPlayerStartX;}
      float getPlayerStartY(void) {return m_fPlayerStartY;}
      void setLevelPack(const std::string &s) {m_LevelPack = s;}
      const std::string &getLevelPack(void) {return m_LevelPack;}
      void setPlayerStart(float x,float y) {m_fPlayerStartX=x ;m_fPlayerStartY=y;}
      void setLimits(float fLeft,float fRight,float fTop,float fBottom) {
        m_fLeftLimit=fLeft; m_fRightLimit=fRight; m_fTopLimit=fTop; m_fBottomLimit=fBottom;      
      }
      LevelBlock *getBlockByID(std::string ID) {
        for(int i=0;i<m_Blocks.size();i++) if(m_Blocks[i]->ID == ID) return m_Blocks[i];
        return NULL;
      }
      LevelZone *getZoneByID(std::string ID) {
        for(int i=0;i<m_Zones.size();i++) if(m_Zones[i]->ID == ID) return m_Zones[i];
        return NULL;
      }
      LevelEntity *getEntityByID(std::string ID) {
        for(int i=0;i<m_Entities.size();i++) if(m_Entities[i]->ID == ID) return m_Entities[i];
				return NULL;
      }
      std::vector<LevelEntity *> getEntitiesByTypeID(std::string TypeID) {
				std::vector<LevelEntity *> Ret;
        for(int i=0;i<m_Entities.size();i++) if(m_Entities[i]->TypeID == TypeID) Ret.push_back(m_Entities[i]);
				return Ret;				
      }
      std::string getEntityParam(LevelEntity *pEnt,std::string ID,std::string Def="") {
        for(int i=0;i<pEnt->Params.size();i++) if(pEnt->Params[i]->Name == ID) return pEnt->Params[i]->Value;
        return Def;
      }
    
    private:
      /* Data */
      std::string m_LevelPack;        /* In this level pack */
      XMLDocument m_XML;              /* Plain XML source */      
      std::string m_FileName;         /* Current file name */
      
      LevelInfo m_Info;               /* Level info */
      
      std::string m_ID;               /* Level ID */
      
      std::string m_ScriptFile;       /* Script file name */      
      std::string m_ScriptSource;     /* Script source code */
      
      float m_fLeftLimit,m_fRightLimit,m_fTopLimit,m_fBottomLimit; /* Limits */
      float m_fPlayerStartX,m_fPlayerStartY; /* Player start pos */
      
      std::vector<LevelBlock *> m_Blocks; /* Level blocks */
      std::vector<LevelZone *> m_Zones; /* Level zones */
      std::vector<LevelEntity *> m_Entities; /* Level entities */
      
      /* Helper methods */
      TiXmlElement *_FindElement(TiXmlElement *pRoot,const std::string &Name);
      std::string _GetOption(TiXmlElement *pElem,std::string Name,std::string Default="");
      std::string _GetElementText(TiXmlElement *pRoot,std::string Name);
      void _AppendText(std::string &Text,const std::string &Append);
      void _UnloadLevelData(void);
  };

};

#endif
