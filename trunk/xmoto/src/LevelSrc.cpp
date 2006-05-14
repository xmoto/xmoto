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
 *  Level files are stored as XML. The LevelSrc class represents one of these
 *  files and allows on-the-fly changes to everything - useful both in game 
 *  code and in editor code.
 */

#include "LevelSrc.h"
#include "VFileIO.h"

namespace vapp {
  /*===========================================================================
  Unload
  ===========================================================================*/
  void LevelSrc::_UnloadLevelData(void) {
    /* Free a bunch of memory */
    while(m_Blocks.size() > 0) {
      while(m_Blocks[m_Blocks.size()-1]->Vertices.size() > 0) {
        delete m_Blocks[m_Blocks.size()-1]->Vertices[ m_Blocks[m_Blocks.size()-1]->Vertices.size() - 1];
        m_Blocks[m_Blocks.size()-1]->Vertices.pop_back();
      }
    
      delete m_Blocks[m_Blocks.size()-1];
      m_Blocks.pop_back();
    }

    while(m_Zones.size() > 0) {
      while(m_Zones[m_Zones.size()-1]->Prims.size() > 0) {
        delete m_Zones[m_Zones.size()-1]->Prims[ m_Zones[m_Zones.size()-1]->Prims.size() - 1];
        m_Zones[m_Zones.size()-1]->Prims.pop_back();
      }
    
      delete m_Zones[m_Zones.size()-1];
      m_Zones.pop_back();
    }
    
    while(m_Entities.size() > 0) {
			while(m_Entities[m_Entities.size()-1]->Params.size() > 0) {
				delete m_Entities[m_Entities.size()-1]->Params[m_Entities[m_Entities.size()-1]->Params.size() - 1];
				m_Entities[m_Entities.size()-1]->Params.pop_back();
			}
			
			delete m_Entities[m_Entities.size()-1];
			m_Entities.pop_back();
    }
  }

  /*===========================================================================
  Save level source
  ===========================================================================*/
  void LevelSrc::saveXML(void) {
    FileHandle *pfh = FS::openOFile(m_FileName);
    if(pfh == NULL) {
      /* Failed! */
      Log("** Warning ** : failed to save level '%s'",m_FileName.c_str());
      return;
    }
    
    FS::writeLineF(pfh,"<?xml version=\"1.0\" encoding=\"utf-8\"?>");
    
    if(m_LevelPack != "")
      FS::writeLineF(pfh,"<level id=\"%s\" levelpack=\"%s\">",m_ID.c_str(),m_LevelPack.c_str());
    else
      FS::writeLineF(pfh,"<level id=\"%s\">",m_ID.c_str());
    
    /* INFO */
    FS::writeLineF(pfh,"\t<info>");
    FS::writeLineF(pfh,"\t\t<name>%s</name>",m_Info.Name.c_str());
    FS::writeLineF(pfh,"\t\t<description>%s</description>",m_Info.Description.c_str());
    FS::writeLineF(pfh,"\t\t<author>%s</author>",m_Info.Author.c_str());
    FS::writeLineF(pfh,"\t\t<date>%s</date>",m_Info.Date.c_str());
    FS::writeLineF(pfh,"\t\t<sky>%s</sky>",m_Info.Sky.c_str());
    FS::writeLineF(pfh,"\t</info>");
    
    /* MISC */
    if(m_ScriptFile != "" && m_ScriptSource != "") {
      FS::writeLineF(pfh,"\t<script source=\"%s\">",m_ScriptFile.c_str());
      FS::writeByte(pfh,'\t'); FS::writeByte(pfh,'\t');
      FS::writeBuf(pfh,(char *)m_ScriptSource.c_str(),m_ScriptSource.length());
      FS::writeLineF(pfh,"</script>");
    }
    else if(m_ScriptFile != "")
      FS::writeLineF(pfh,"\t<script source=\"%s\"/>",m_ScriptFile.c_str());
    else if(m_ScriptSource != "") {
      FS::writeLineF(pfh,"\t<script>");
      FS::writeByte(pfh,'\t'); FS::writeByte(pfh,'\t');
      FS::writeBuf(pfh,(char *)m_ScriptSource.c_str(),m_ScriptSource.length());
      FS::writeLineF(pfh,"</script>");
    }
      
    FS::writeLineF(pfh,"\t<limits left=\"%f\" right=\"%f\" top=\"%f\" bottom=\"%f\"/>",m_fLeftLimit,m_fRightLimit,m_fTopLimit,m_fBottomLimit);
    
    /* BLOCKS */
    for(int i=0;i<m_Blocks.size();i++) {
      FS::writeLineF(pfh,"\t<block id=\"%s\">",m_Blocks[i]->ID.c_str());
      if(m_Blocks[i]->bBackground)
        FS::writeLineF(pfh,"\t\t<position x=\"%f\" y=\"%f\" background=\"true\"/>",m_Blocks[i]->fPosX,m_Blocks[i]->fPosY);
      else if(m_Blocks[i]->bWater)
        FS::writeLineF(pfh,"\t\t<position x=\"%f\" y=\"%f\" water=\"true\"/>",m_Blocks[i]->fPosX,m_Blocks[i]->fPosY);
      else
        FS::writeLineF(pfh,"\t\t<position x=\"%f\" y=\"%f\"/>",m_Blocks[i]->fPosX,m_Blocks[i]->fPosY);
      FS::writeLineF(pfh,"\t\t<usetexture id=\"%s\"/>",m_Blocks[i]->Texture.c_str());
      for(int j=0;j<m_Blocks[i]->Vertices.size();j++) {
        if(m_Blocks[i]->Vertices[j]->EdgeEffect != "")
          FS::writeLineF(pfh,"\t\t<vertex x=\"%f\" y=\"%f\" edge=\"%s\"/>",
            m_Blocks[i]->Vertices[j]->fX,m_Blocks[i]->Vertices[j]->fY,m_Blocks[i]->Vertices[j]->EdgeEffect.c_str());
        else
          FS::writeLineF(pfh,"\t\t<vertex x=\"%f\" y=\"%f\"/>",
            m_Blocks[i]->Vertices[j]->fX,m_Blocks[i]->Vertices[j]->fY);
      }
      FS::writeLineF(pfh,"\t</block>");
    }
    
    /* ENTITIES */
    for(int i=0;i<m_Entities.size();i++) {
			FS::writeLineF(pfh,"\t<entity id=\"%s\" typeid=\"%s\">",m_Entities[i]->ID.c_str(),m_Entities[i]->TypeID.c_str());
			FS::writeLineF(pfh,"\t\t<size r=\"%f\"/>",m_Entities[i]->fSize);
      FS::writeLineF(pfh,"\t\t<position x=\"%f\" y=\"%f\"/>",m_Entities[i]->fPosX,m_Entities[i]->fPosY);			
      for(int j=0;j<m_Entities[i]->Params.size();j++) {
				FS::writeLineF(pfh,"\t\t<param name=\"%s\" value=\"%s\"/>",
					m_Entities[i]->Params[j]->Name.c_str(),m_Entities[i]->Params[j]->Value.c_str());
      }
			FS::writeLineF(pfh,"\t</entity>");
    }
    
    /* ZONE */
    for(int i=0;i<m_Zones.size();i++) {
      FS::writeLineF(pfh,"\t<zone id=\"%s\">",m_Zones[i]->ID.c_str());
      for(int j=0;j<m_Zones[i]->Prims.size();j++) {
        if(m_Zones[i]->Prims[j]->Type == LZPT_BOX) {
          FS::writeLineF(pfh,"\t\t<box left=\"%f\" right=\"%f\" top=\"%f\" bottom=\"%f\"/>",
                  m_Zones[i]->Prims[j]->fLeft,m_Zones[i]->Prims[j]->fRight,
                  m_Zones[i]->Prims[j]->fTop,m_Zones[i]->Prims[j]->fBottom);
        }
      }
      FS::writeLineF(pfh,"\t</zone>");
    }
    
    FS::writeLineF(pfh,"\n</level>");           
    FS::closeFile(pfh);
  }
  
  /*===========================================================================
  Load level source
  ===========================================================================*/
  void LevelSrc::loadXML(void) {
    /* Load XML document and fetch tinyxml handle */
    _UnloadLevelData();
    m_LevelCheckSum.nCRC32 = 0;
    m_XML.readFromFile( m_FileName, /*&m_LevelCheckSum.nCRC32*/ NULL );
    
    TiXmlDocument *pDoc = m_XML.getLowLevelAccess();
    
    /* Start the fantastic parsing by fetching the <level> element */
    TiXmlElement *pLevelElem = _FindElement(NULL,std::string("level"));    
    if(pLevelElem == NULL) return; /* TODO: error */
    
    /* Get level ID */
    m_ID = _GetOption(pLevelElem,"id");
    if(m_ID == "") return; /* TODO: error */    
    
    /* Get level pack */
    m_LevelPack = _GetOption(pLevelElem,"levelpack");
    
    /* Set default info */
    m_Info.Name = m_FileName;
    m_Info.Date = "";
    m_Info.Description = "";
    m_Info.Author = "";
    m_Info.Sky = "sky1";
    
    /* Get level <info> element */
    TiXmlElement *pInfoElem = _FindElement(pLevelElem,std::string("info"));
    if(pInfoElem != NULL) {
      /* Name */
      std::string Tmp = _GetElementText(pInfoElem,"name");
      if(Tmp != "") m_Info.Name = Tmp;

      /* Author */
      Tmp = _GetElementText(pInfoElem,"author");
      if(Tmp != "") m_Info.Author = Tmp;

      /* Description */
      Tmp = _GetElementText(pInfoElem,"description");
      if(Tmp != "") m_Info.Description = Tmp;

      /* Date */
      Tmp = _GetElementText(pInfoElem,"date");
      if(Tmp != "") m_Info.Date = Tmp;

      /* Sky */
      Tmp = _GetElementText(pInfoElem,"sky");
      if(Tmp != "") m_Info.Sky = Tmp;
    }
    
    /* Get script */
    m_ScriptFile = "";
    m_ScriptSource = "";
    
    TiXmlElement *pScriptElem = _FindElement(pLevelElem,std::string("script"));
    if(pScriptElem != NULL) {
      /* External script file specified? */
      m_ScriptFile = _GetOption(pScriptElem,"source");      
      
      /* Encapsulated script? */
      for(TiXmlNode *pScript=pScriptElem->FirstChild();pScript!=NULL;
          pScript=pScript->NextSibling()) {
        if(pScript->Type() == TiXmlNode::TEXT) {
          m_ScriptSource.append(pScript->Value());
        }
      }
    }    
    
    /* Get level limits */
    m_fBottomLimit = m_fLeftLimit = -50.0f;
    m_fTopLimit = m_fRightLimit = 50.0f;
    TiXmlElement *pLimitsElem = _FindElement(pLevelElem,std::string("limits"));
    if(pLimitsElem != NULL) {
      m_fBottomLimit = atof( _GetOption(pLimitsElem,"bottom","-50").c_str() );
      m_fLeftLimit = atof( _GetOption(pLimitsElem,"left","-50").c_str() );
      m_fTopLimit = atof( _GetOption(pLimitsElem,"top","50").c_str() );
      m_fRightLimit = atof( _GetOption(pLimitsElem,"right","50").c_str() );
    }
    
    /* Get player start */
    m_fPlayerStartX = m_fPlayerStartY = 0.0f;
    TiXmlElement *pPlayerStartElem = _FindElement(pLevelElem,std::string("playerstart"));
    if(pPlayerStartElem != NULL) {
      m_fPlayerStartX = atof( _GetOption(pPlayerStartElem,"x","0").c_str() );
      m_fPlayerStartY = atof( _GetOption(pPlayerStartElem,"y","0").c_str() );
    }
    
    /* Get entities */
    for(TiXmlElement *pElem = pLevelElem->FirstChildElement("entity"); pElem!=NULL; 
        pElem=pElem->NextSiblingElement("entity")) {
      /* Allocate it */
      LevelEntity *pEntity = new LevelEntity;
      
      pEntity->ID = _GetOption(pElem,"id");
      pEntity->TypeID = _GetOption(pElem,"typeid");
      
      TiXmlElement *pPosElem = pElem->FirstChildElement("position");
      if(pPosElem != NULL) {
				pEntity->fPosX = atof(_GetOption(pPosElem,"x","0").c_str());
				pEntity->fPosY = atof(_GetOption(pPosElem,"y","0").c_str());
      }
      
      TiXmlElement *pSizeElem = pElem->FirstChildElement("size");
      if(pSizeElem != NULL) {
        pEntity->fSize = atof(_GetOption(pSizeElem,"r","0.2").c_str());
      }
      
      /* Get parameters */
      for(TiXmlElement *pParamElem = pElem->FirstChildElement("param"); pParamElem!=NULL;
          pParamElem=pParamElem->NextSiblingElement("param")) {
				LevelEntityParam *pParam = new LevelEntityParam;
				pParam->Name = _GetOption(pParamElem,"name");
				pParam->Value = _GetOption(pParamElem,"value");
				
				pEntity->Params.push_back( pParam );
      }
      
      /* Add it to the list */
      m_Entities.push_back( pEntity );
		}    
    
    /* Get zones */
    for(TiXmlElement *pElem = pLevelElem->FirstChildElement(); pElem!=NULL; pElem=pElem->NextSiblingElement()) {
      if(!strcmp(pElem->Value(),"zone")) {
        /* Got one */
        LevelZone *pZone = new LevelZone;
        
        pZone->ID = _GetOption(pElem,"id");
        
        if(pZone->ID == "") {
          /* TODO: error */
          delete pZone;
          continue;
        }        
        
        /* Get primitives */
        for(TiXmlElement *pj = pElem->FirstChildElement(); pj!=NULL; pj=pj->NextSiblingElement()) {
          if(!strcmp(pj->Value(),"box")) {
            /* Alloc */
            LevelZonePrim *pPrim = new LevelZonePrim;                        
            pPrim->Type = LZPT_BOX;
            pPrim->fBottom = atof( _GetOption(pj,"bottom","0").c_str() );
            pPrim->fTop = atof( _GetOption(pj,"top","0").c_str() );
            pPrim->fLeft = atof( _GetOption(pj,"left","0").c_str() );
            pPrim->fRight = atof( _GetOption(pj,"right","0").c_str() );
                                    
            /* Add it */
            pZone->Prims.push_back( pPrim );
          }
        }   
        
        pZone->m_bInZone = false;     
        
        /* Add it to the list */
        m_Zones.push_back( pZone );
      }
    }
    
    /* Get blocks */
    for(TiXmlElement *pElem = pLevelElem->FirstChildElement(); pElem!=NULL; pElem=pElem->NextSiblingElement()) {
      if(!strcmp(pElem->Value(),"block")) {
        /* Got one */
        LevelBlock *pBlock = new LevelBlock;
        
        pBlock->fPosX = 0.0f;
        pBlock->fPosY = 0.0f;
        pBlock->fTextureScale = 1.0f;
        pBlock->ID = _GetOption(pElem,"id");
        
        if(pBlock->ID == "") {
          /* TODO: error */
          delete pBlock;
          continue;
        }
        
        pBlock->Texture = "default";        
        
        TiXmlElement *pUseTextureElem = _FindElement(pElem,std::string("usetexture"));
        TiXmlElement *pPositionElem = _FindElement(pElem,std::string("position"));                
        
        if(pUseTextureElem != NULL) {
          pBlock->Texture = _GetOption(pUseTextureElem,"id","default");
          pBlock->fTextureScale = atof( _GetOption(pUseTextureElem,"scale","1").c_str() );
        }
        if(pPositionElem != NULL) {
          pBlock->fPosX = atof( _GetOption(pPositionElem,"x","0").c_str() );
          pBlock->fPosY = atof( _GetOption(pPositionElem,"y","0").c_str() );      
          
          if(_GetOption(pPositionElem,"background","false") == "true")
            pBlock->bBackground = true;

          if(_GetOption(pPositionElem,"water","false") == "true")
            pBlock->bWater = true;
        }
        
        /* Get vertices */
        for(TiXmlElement *pj = pElem->FirstChildElement(); pj!=NULL; pj=pj->NextSiblingElement()) {
          if(!strcmp(pj->Value(),"vertex")) {
            /* Alloc */
            LevelBlockVertex *pVertex = new LevelBlockVertex;
                        
            pVertex->bSelected = false;
                        
            pVertex->fX = atof( _GetOption(pj,"x","0").c_str() );
            pVertex->fY = atof( _GetOption(pj,"y","0").c_str() );
            pVertex->EdgeEffect = _GetOption(pj,"edge","");
            
            std::string k;
            k = _GetOption(pj,"tx","");
            if(k != "") pVertex->fTX = atof( k.c_str() );
            else pVertex->fTX = pVertex->fX * pBlock->fTextureScale;
            k = _GetOption(pj,"ty","");
            if(k != "") pVertex->fTY = atof( k.c_str() );
            else pVertex->fTY = pVertex->fY * pBlock->fTextureScale;
            
            pVertex->r = atoi( _GetOption(pj,"r","255").c_str() );
            pVertex->g = atoi( _GetOption(pj,"g","255").c_str() );
            pVertex->b = atoi( _GetOption(pj,"b","255").c_str() );
            pVertex->a = atoi( _GetOption(pj,"a","255").c_str() );
            
            /* Add it */
            pBlock->Vertices.push_back( pVertex );
          }
        }
        
        /* Add it */
        m_Blocks.push_back( pBlock );
      }
    }  
    
    /* Find out where the player starts */
    if(getEntitiesByTypeID("PlayerStart").size()>0) {
			LevelEntity *pPlayerStart = getEntitiesByTypeID("PlayerStart")[0];
			m_fPlayerStartX = pPlayerStart->fPosX;
			m_fPlayerStartY = pPlayerStart->fPosY;
		}
		else {
			Log("** Warning ** : %s : No player start location found",m_FileName.c_str());
			m_fPlayerStartX = m_fPlayerStartY = 0.0f;
		}
  }
  
  /*===========================================================================
  Easy-to-use function for adding entities
  ===========================================================================*/
  LevelEntity *LevelSrc::createEntity(std::string TypeID,float x,float y) {
		/* First determine a good default name */
		char cID[256];
		std::string ID = "";
		for(int i=0;i<10000;i++) {
			sprintf(cID,"My%s%d",TypeID.c_str(),i);
			if(getEntityByID(cID) == NULL) {
				ID = cID;
				break;
			}
		}
		
		if(ID == "") {
			Log("** Warning ** : LevelSrc::createEntity() - Too many entities. You're sick.");
			return NULL;
		}
		
		/* Create it */
		LevelEntity *pEntity = new LevelEntity;
		pEntity->fPosX = x;
		pEntity->fPosY = y;
		pEntity->ID = ID;
		pEntity->TypeID = TypeID;
		pEntity->bSelected = false;
		pEntity->fSize = 0.2f;
		
		m_Entities.push_back( pEntity );
		
		/* Note that no parameters are defined -- that's the job of the caller */
		return pEntity;
  }
  
  /*===========================================================================
  Append. yeah.
  ===========================================================================*/
  void LevelSrc::_AppendText(std::string &Text,const std::string &Append) {
    const char *pc = Append.c_str();
    int i=0;
    std::string A = "";
    char c[2];
    
    while(1) {
      if(pc[i] == '\0' || pc[i] == ' ' || pc[i] == '\t') {
        if(A != "") {
          Text.append( A );
          Text.append( " " );
          A = "";
        }
        if(pc[i] == '\0') break;
      }
      else {
        c[0] = pc[i];
        c[1] = '\0';
        A.append( c );
      }
      
      i++;
    }
  }
  
  /*===========================================================================
  Get formatted element text
  ===========================================================================*/
  std::string LevelSrc::_GetElementText(TiXmlElement *pRoot,std::string Name) {
    TiXmlElement *pFirst;
    std::string Text = "";
  
    /* Find out where to start */
    if(pRoot == NULL) pFirst = m_XML.getLowLevelAccess()->FirstChildElement();
    else pFirst = pRoot->FirstChildElement();
    
    /* Scan through elements */
    for(TiXmlElement *pElem = pFirst;pElem!=NULL;pElem=pElem->NextSiblingElement()) {
      if(!strcmp(pElem->Value(),Name.c_str())) {
        /* This is the one -- format text*/
        for(TiXmlNode *pNode = pElem->FirstChild();pNode!=NULL;pNode=pNode->NextSibling()) {
          if(pNode->Type() == TiXmlNode::TEXT) {
            /* Ohh... text. Append it */
            _AppendText(Text,std::string(pNode->Value()));
          }
          else if(pNode->Type() == TiXmlNode::ELEMENT) {
            /* Hmm, some kind of formatting element */
            if(!strcmp(pNode->Value(),"br")) {
              /* HTML-style line break */
              Text.append( "\n" );              
            }
          }
        }
        if(Text.length() > 0) {
          if(Text[ Text.length()-1 ] == ' ') {
            Text = Text.substr(0,Text.length()-1);
          }
        }
        return Text;
      }
    }
    
    /* Nothing */
    return "";
  }
  
  /*===========================================================================
  Find element in document object model
  ===========================================================================*/
  TiXmlElement *LevelSrc::_FindElement(TiXmlElement *pRoot,const std::string &Name) {
    TiXmlElement *pFirst;
  
    /* Find out where to start */
    if(pRoot == NULL) pFirst = m_XML.getLowLevelAccess()->FirstChildElement();
    else pFirst = pRoot->FirstChildElement();
    
    /* Scan through elements */
    for(TiXmlElement *pElem = pFirst;pElem!=NULL;pElem=pElem->NextSiblingElement()) {
      if(!strcmp(pElem->Value(),Name.c_str())) {
        /* This is the one */
        return pElem;
      }
      
      /* Recurse */
      if(pElem->FirstChildElement() != NULL) {
        TiXmlElement *pRet = _FindElement(pElem,Name);
        if(pRet != NULL) return pRet;
      }
    }
    
    /* Found nothing */
    return NULL;
  }
  
  /*===========================================================================
  Find element option/attribute
  ===========================================================================*/
  std::string LevelSrc::_GetOption(TiXmlElement *pElem,std::string Name,std::string Default) {
    const char *pc = pElem->Attribute(Name.c_str());
    if(pc == NULL) return Default;
    return std::string(pc);
  }
  
  /*===========================================================================
  Calculate checksum of level XML before loading it
  ===========================================================================*/
  bool LevelSrc::probeCheckSum(LevelCheckSum *pSum) {
    m_XML.readFromFile( m_FileName,&pSum->nCRC32 );      
    return true;
  }
      
  /*===========================================================================
  Export binary level file
  ===========================================================================*/
  void LevelSrc::exportBinary(const std::string &FileName,LevelCheckSum *pSum) {
    /* Export binary... */
    FileHandle *pfh = FS::openOFile(FileName);
    if(pfh == NULL) {
      Log("** Warning ** : Failed to export binary: %s",FileName.c_str());
    }
    else {
      /* Write tag */
      FS::writeBuf(pfh,"XBL1",4);
      
      /* Write CRC32 of XML */
      FS::writeInt_LE(pfh,pSum->nCRC32);
            
      /* Write header */
      FS::writeString(pfh,m_ID);
      FS::writeString(pfh,m_LevelPack);
      FS::writeString(pfh,m_Info.Name);
      FS::writeString(pfh,m_Info.Description);
      FS::writeString(pfh,m_Info.Author);
      FS::writeString(pfh,m_Info.Date);
      FS::writeString(pfh,m_Info.Sky);
      FS::writeString(pfh,m_ScriptFile);
      
      FS::writeFloat_LE(pfh,m_fLeftLimit);
      FS::writeFloat_LE(pfh,m_fRightLimit);
      FS::writeFloat_LE(pfh,m_fTopLimit);
      FS::writeFloat_LE(pfh,m_fBottomLimit);
      
      /* Write script (if any) */
      FS::writeInt_LE(pfh,m_ScriptSource.length());
      FS::writeBuf(pfh,(char *)m_ScriptSource.c_str(),m_ScriptSource.length());
      
      /* Write blocks */
      FS::writeInt_LE(pfh,m_Blocks.size());
      for(int i=0;i<m_Blocks.size();i++) {
        FS::writeString(pfh,m_Blocks[i]->ID);
        FS::writeBool(pfh,m_Blocks[i]->bBackground);
        FS::writeBool(pfh,m_Blocks[i]->bWater);
        FS::writeString(pfh,m_Blocks[i]->Texture);
        FS::writeFloat_LE(pfh,m_Blocks[i]->fPosX);
        FS::writeFloat_LE(pfh,m_Blocks[i]->fPosY);
        
        FS::writeShort_LE(pfh,m_Blocks[i]->Vertices.size());
        
        for(int j=0;j<m_Blocks[i]->Vertices.size();j++) {
          FS::writeFloat_LE(pfh,m_Blocks[i]->Vertices[j]->fX);
          FS::writeFloat_LE(pfh,m_Blocks[i]->Vertices[j]->fY);
          FS::writeString(pfh,m_Blocks[i]->Vertices[j]->EdgeEffect);
        }        
      }
      
      /* Write entities */
      FS::writeInt_LE(pfh,m_Entities.size());
      for(int i=0;i<m_Entities.size();i++) {
			  FS::writeString(pfh,m_Entities[i]->ID);
			  FS::writeString(pfh,m_Entities[i]->TypeID);
			  FS::writeFloat_LE(pfh,m_Entities[i]->fSize);
        FS::writeFloat_LE(pfh,m_Entities[i]->fPosX);
        FS::writeFloat_LE(pfh,m_Entities[i]->fPosY);
        FS::writeByte(pfh,m_Entities[i]->Params.size());       
        for(int j=0;j<m_Entities[i]->Params.size();j++) {
          FS::writeString(pfh,m_Entities[i]->Params[j]->Name);
          FS::writeString(pfh,m_Entities[i]->Params[j]->Value);        
        }
      }  
      
      /* Write zones */
      FS::writeInt_LE(pfh,m_Zones.size());
      for(int i=0;i<m_Zones.size();i++) {
        FS::writeString(pfh,m_Zones[i]->ID);
        FS::writeByte(pfh,m_Zones[i]->Prims.size());
        
        for(int j=0;j<m_Zones[i]->Prims.size();j++) {
          FS::writeInt_LE(pfh,(int)m_Zones[i]->Prims[j]->Type);
        
          if(m_Zones[i]->Prims[j]->Type == LZPT_BOX) {
            FS::writeFloat_LE(pfh,m_Zones[i]->Prims[j]->fLeft);
            FS::writeFloat_LE(pfh,m_Zones[i]->Prims[j]->fRight);
            FS::writeFloat_LE(pfh,m_Zones[i]->Prims[j]->fTop);
            FS::writeFloat_LE(pfh,m_Zones[i]->Prims[j]->fBottom);
          }
        }
      }
                
      /* clean up */
      FS::closeFile(pfh);
    }
  }
  
  /*===========================================================================
  Import binary level file
  ===========================================================================*/
  bool LevelSrc::importBinary(const std::string &FileName,LevelCheckSum *pSum) {
    _UnloadLevelData();
    bool bRet = true;

    m_fPlayerStartX = m_fPlayerStartY = 0.0f;

    /* Import binary */
    FileHandle *pfh = FS::openIFile(FileName);
    if(pfh == NULL) {
      return false;
    }
    else {
      /* Read tag - it tells something about the format */
      char cTag[5];
      FS::readBuf(pfh,(char *)cTag,4);
      cTag[4] = '\0';
      int nFormat = 0;
      if(!strcmp(cTag,"XBL1"))
        nFormat = 1;
        
      if(nFormat == 1) {
        /* Read "format 1" binary level */
        m_LevelCheckSum.nCRC32 = pSum->nCRC32;
        
        /* Right CRC? */
        if(FS::readInt_LE(pfh) != pSum->nCRC32) {
          Log("** Warning ** : CRC check failed, can't import: %s",FileName.c_str());
          bRet = false;
        }
        else {
          /* Read header */
          m_ID = FS::readString(pfh);
          m_LevelPack = FS::readString(pfh);
          m_Info.Name = FS::readString(pfh);
          m_Info.Description = FS::readString(pfh);
          m_Info.Author = FS::readString(pfh);
          m_Info.Date = FS::readString(pfh);
          m_Info.Sky = FS::readString(pfh);
          m_ScriptFile = FS::readString(pfh);

          m_fLeftLimit = FS::readFloat(pfh);
          m_fRightLimit = FS::readFloat(pfh);
          m_fTopLimit = FS::readFloat(pfh);
          m_fBottomLimit = FS::readFloat(pfh);

          m_fLeftLimit = FS::readFloat_LE(pfh);
          m_fRightLimit = FS::readFloat_LE(pfh);
          m_fTopLimit = FS::readFloat_LE(pfh);
          m_fBottomLimit = FS::readFloat_LE(pfh);

          /* Read embedded script */
          int nScriptSourceLen = FS::readInt_LE(pfh);
          if(nScriptSourceLen > 0) {
            char *pcTemp = new char[nScriptSourceLen+1];
            FS::readBuf(pfh,(char *)pcTemp,nScriptSourceLen);
            pcTemp[nScriptSourceLen]='\0';
            
            m_ScriptSource = pcTemp;
            
            delete [] pcTemp;           
          }
          else
            m_ScriptSource = "";

          /* Read blocks */
          int nNumBlocks = FS::readInt_LE(pfh);
          m_Blocks.reserve(nNumBlocks);
          for(int i=0;i<nNumBlocks;i++) {
            LevelBlock *pBlock = new LevelBlock;
            pBlock->ID = FS::readString(pfh);
            pBlock->bBackground = FS::readBool(pfh);
            pBlock->bWater = FS::readBool(pfh);
            pBlock->Texture = FS::readString(pfh);
            pBlock->fPosX = FS::readFloat_LE(pfh);
            pBlock->fPosY = FS::readFloat_LE(pfh);
            
            int nNumVertices = FS::readShort_LE(pfh);
            pBlock->Vertices.reserve(nNumVertices);
            for(int j=0;j<nNumVertices;j++) {
              LevelBlockVertex *pV = new LevelBlockVertex;
              pV->fX = FS::readFloat_LE(pfh);
              pV->fY = FS::readFloat_LE(pfh);
              pV->EdgeEffect = FS::readString(pfh);
                            
              pBlock->Vertices.push_back(pV);
            }
            
            m_Blocks.push_back(pBlock);
          }

          /* Read entities */
          int nNumEntities = FS::readInt_LE(pfh);
          m_Entities.reserve(nNumEntities);
          for(int i=0;i<nNumEntities;i++) {
            LevelEntity *pEnt = new LevelEntity;
            pEnt->ID = FS::readString(pfh);
            pEnt->TypeID = FS::readString(pfh);
            pEnt->fSize = FS::readFloat_LE(pfh);
            pEnt->fPosX = FS::readFloat_LE(pfh);
            pEnt->fPosY = FS::readFloat_LE(pfh);
            
            int nNumParams = FS::readByte(pfh);
            pEnt->Params.reserve(nNumParams);
            for(int j=0;j<nNumParams;j++) {
              LevelEntityParam *pP = new LevelEntityParam;
              pP->Name = FS::readString(pfh);
              pP->Value = FS::readString(pfh);
              
              pEnt->Params.push_back(pP);
            }
            
            m_Entities.push_back(pEnt);
            
            /* Player start? */
            if(pEnt->TypeID == "PlayerStart") {
              m_fPlayerStartX = pEnt->fPosX;
              m_fPlayerStartY = pEnt->fPosY;
            }
          }
          
          /* Read zones */
          int nNumZones = FS::readInt_LE(pfh);
          m_Zones.reserve(nNumZones);
          for(int i=0;i<nNumZones;i++) {
            LevelZone *pZone = new LevelZone;
            pZone->ID = FS::readString(pfh);
            
            int nNumPrims = FS::readByte(pfh);
            pZone->Prims.reserve(nNumPrims);
            for(int j=0;j<nNumPrims;j++) {
              LevelZonePrim *pP = new LevelZonePrim;
              pP->Type = (LevelZonePrimType)FS::readInt_LE(pfh);
              
              if(pP->Type == LZPT_BOX) {
                pP->fLeft = FS::readFloat_LE(pfh);
                pP->fRight = FS::readFloat_LE(pfh);
                pP->fTop = FS::readFloat_LE(pfh);
                pP->fBottom = FS::readFloat_LE(pfh);
              }
              else {
                Log("** Warning ** : Invalid zone primitive encountered in: %s",FileName.c_str());
                delete pP;
                delete pZone;
                bRet = false;
                break;
              }
              
              pZone->Prims.push_back(pP);
            }
            
            if(!bRet) break;
            
            m_Zones.push_back(pZone);
          }                                                                       
        }
      }
      else {
        Log("** Warning ** : Invalid binary format (%d), can't import: %s",nFormat,FileName.c_str());
        bRet = false;
      }
             
      /* clean up */
      FS::closeFile(pfh);
    }
    
    return bRet;
  }

  int LevelSrc::compareLevel(const LevelSrc *p_lvl1, const LevelSrc *p_lvl2) {
    if(p_lvl1->m_Info.Name == p_lvl2->m_Info.Name) {
      return 0;
    }
    
    if(p_lvl1->m_Info.Name > p_lvl2->m_Info.Name) {
      return 1;
    } 

    return -1;
  }
    
};

