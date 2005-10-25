/*=============================================================================
XMOTO
Copyright (C) 2005 Rasmus Neckelmann (neckelmann@gmail.com)

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
 *  This is an application wrapper for the editor.dat file which contains
 *  definitions of the entity types placeable by the editor.
 */
#define XMOTO_EDITOR
#include "VXml.h"
#include "EditorData.h"

namespace vapp {
  
  /*============================================================================
  Load and parse an editor data file
  ============================================================================*/
	void EditorData::loadFile(std::string Path) {
		XMLDocument Doc;
		
		/* Load */
		Doc.readFromFile( Path );
		
		/* Start parsing DOM */
		TiXmlDocument *pTree = Doc.getLowLevelAccess();
		
		TiXmlElement *pEditorData = pTree->FirstChildElement("editordata");
		if(pEditorData!=NULL) {
			/* Get stuff */
			for(TiXmlElement *pElem=pEditorData->FirstChildElement();pElem!=NULL;
			    pElem=pElem->NextSiblingElement()) {
				if(!strcmp(pElem->Value(),"entitytype")) {
					/* A new entity type for your editing pleasures... */
					const char *pc;
					std::string TypeID;
					float fSize,fPSize;
					
					pc = pElem->Attribute("id");					
					if(pc != NULL) TypeID = pc;
					else {
						Log("** Warning ** : %s : Type without valid identifier",Path.c_str());
						continue;
					}

					pc = pElem->Attribute("size");
					if(pc != NULL) fSize = atof(pc);
					else fSize = 1.0f;				
					
					pc = pElem->Attribute("psize");
					if(pc != NULL) fPSize = atof(pc);
					else fPSize = 1.0f;				
					
					EditorEntityType *pType = newType(TypeID);
					if(pType != NULL) {
						/* Set size */
						pType->fSize = fSize;
						pType->fPSize = fPSize;
					
						/* New type created ok... Now fill it out */
						TiXmlElement *pSymbol = pElem->FirstChildElement("symbol");
						if(pSymbol!=NULL) {
							/* Parse symbol */
							for(TiXmlElement *pSymbolElem=pSymbol->FirstChildElement();pSymbolElem!=NULL;
							    pSymbolElem=pSymbolElem->NextSiblingElement()) {
							  ETDraw *pDraw;
							  const char *pc;
							  int r,g,b,a;

								/* Load specific attributes */							  
								if(!strcmp(pSymbolElem->Value(),"circle")) {
									pDraw = newDraw(pType,ET_CIRCLE);
									
									pc = pSymbolElem->Attribute("radius");
									if(pc != NULL) pDraw->fRadius = atof(pc);	
									else pDraw->fRadius = 1.0f;

									pc = pSymbolElem->Attribute("border");
									if(pc != NULL) pDraw->fBorder = atof(pc);	
									else pDraw->fBorder = 1.0f;																									
								}
								else if(!strcmp(pSymbolElem->Value(),"text")) {
									pDraw = newDraw(pType,ET_TEXT);

									pDraw->Allign = ET_ALLIGN_CENTER;
									pc = pSymbolElem->Attribute("allign");
									if(pc != NULL) {
										if(!strcmp(pc,"center")) pDraw->Allign = ET_ALLIGN_CENTER;
										else if(!strcmp(pc,"left")) pDraw->Allign = ET_ALLIGN_LEFT;
										else if(!strcmp(pc,"right")) pDraw->Allign = ET_ALLIGN_RIGHT;
										else Log("** Warning ** : %s : Invalid allign '%s' of text (type '%s')",
                             Path.c_str(),pc,TypeID.c_str());										 
									}

									pDraw->VAllign = ET_ALLIGN_CENTER;
									pc = pSymbolElem->Attribute("vallign");
									if(pc != NULL) {
										if(!strcmp(pc,"center")) pDraw->VAllign = ET_ALLIGN_CENTER;
										else if(!strcmp(pc,"top")) pDraw->VAllign = ET_ALLIGN_TOP;
										else if(!strcmp(pc,"bottom")) pDraw->VAllign = ET_ALLIGN_BOTTOM;
										else Log("** Warning ** : %s : Invalid vallign '%s' of text (type '%s')",
                             Path.c_str(),pc,TypeID.c_str());										 
									}
									
									pc = pSymbolElem->Attribute("caption");
									if(pc != NULL) pDraw->Caption = pc;
									else pDraw->Caption = TypeID;
								}
								else if(!strcmp(pSymbolElem->Value(),"rect")) {
									pDraw = newDraw(pType,ET_RECT);

									pc = pSymbolElem->Attribute("border");
									if(pc != NULL) pDraw->fBorder = atof(pc);	
									else pDraw->fBorder = 1.0f;																									

									pc = pSymbolElem->Attribute("width");
									if(pc != NULL) pDraw->fWidth = atof(pc);	
									else pDraw->fWidth = 1.0f;																									

									pc = pSymbolElem->Attribute("height");
									if(pc != NULL) pDraw->fHeight = atof(pc);	
									else pDraw->fHeight = 1.0f;																									
								}
								else 
									Log("** Warning ** : %s : Unknown '%s' in symbol definition for type '%s'",
									    Path.c_str(),pSymbolElem->Value(),TypeID.c_str());								

								/* Common attributes */
								pc = pSymbolElem->Attribute("bcolor");
								if(pc != NULL) {
									sscanf(pc,"#%02X%02X%02X%02X",&r,&g,&b,&a);
									pDraw->BColor = MAKE_COLOR(r,g,b,a);
								}
								else pDraw->BColor = 0;
																
								pc = pSymbolElem->Attribute("fcolor");
								if(pc != NULL) {
									sscanf(pc,"#%02X%02X%02X%02X",&r,&g,&b,&a);
									pDraw->FColor = MAKE_COLOR(r,g,b,a);
								}
								else pDraw->FColor = -1;

								pc = pSymbolElem->Attribute("x");
								if(pc != NULL) pDraw->Position.x = atof(pc);									
								else pDraw->Position.x = 0.0f;

								pc = pSymbolElem->Attribute("y");
								if(pc != NULL) pDraw->Position.y = atof(pc);	
								else pDraw->Position.y = 0.0f;
								
							}
						}
						else Log("** Warning ** : %s : No valid symbol definition for type '%s'",
						         Path.c_str(),TypeID.c_str());
						         
						/* Get parameter list */
						for(TiXmlElement *pParamElem=pElem->FirstChildElement("param");
						    pParamElem!=NULL;pParamElem=pParamElem->NextSiblingElement("param")) {
						  std::string ParamName,ParamDefaultValue;
						  const char *pc;
						  
						  pc = pParamElem->Attribute("name");
						  if(pc != NULL) ParamName = pc;
						  else {
								Log("** Warning ** : %s : Unnamed parameter in type '%s'",
								    Path.c_str(),TypeID.c_str());
								continue; /* skip to next parameter */
						  }

						  pc = pParamElem->Attribute("default");
						  if(pc != NULL) ParamDefaultValue = pc;
							else ParamDefaultValue = "";
						    
							ETParam *pParam = newParam(pType,ParamName,ParamDefaultValue);
							if(pParam == NULL)
								Log("** Warning ** : %s : Parameter naming conflict in type '%s'",
								    Path.c_str(),TypeID.c_str());								
						}
					}
					else Log("** Warning ** : %s : Failed to create new type '%s'",
					         Path.c_str(),TypeID.c_str());
				}
			}
		}
		else Log("** Warning ** : %s : No <editordata> element in root",Path.c_str());
	}
	
  /*============================================================================
  Return entity type definition by identifier
  ============================================================================*/
	EditorEntityType *EditorData::getTypeByID(std::string ID) {
		for(int i=0;i<m_EntityTypes.size();i++)
			if(m_EntityTypes[i]->ID == ID) return m_EntityTypes[i];
		return NULL;
	}
	
  /*============================================================================
  Scan through the type's list of parameters, and look for the given name
  ============================================================================*/
	ETParam *EditorData::getParamByName(EditorEntityType *pType,std::string Name) {
		for(int i=0;i<pType->Params.size();i++)
			if(pType->Params[i]->Name == Name) return pType->Params[i];
		return NULL;
	}

  /*============================================================================
  Allocate a new empty type
  ============================================================================*/
	EditorEntityType *EditorData::newType(std::string ID) {
		/* Is ID in use? */
		if(getTypeByID(ID)) return NULL;
		
		/* Allocate and register */
		EditorEntityType *pType = new EditorEntityType;
		pType->ID = ID;
		pType->fSize = 1.0f;
		
		m_EntityTypes.push_back(pType);
		
		return pType;
	}
	
  /*============================================================================
  Add parameter to type
  ============================================================================*/
	ETParam *EditorData::newParam(EditorEntityType *pType,std::string Name,
	                              std::string DefaultValue) {
		/* Name in use? */
		if(getParamByName(pType,Name)) return NULL;
		
		/* Allocate and register */
		ETParam *pParam = new ETParam;
		pParam->Name = Name;
		pParam->DefaultValue = DefaultValue;
		
		pType->Params.push_back(pParam);
		
		return pParam;		
	}
	
  /*============================================================================
  Add draw command to type
  ============================================================================*/
	ETDraw *EditorData::newDraw(EditorEntityType *pType,ETDrawType DrawType) {
		/* Allocate and register */
		ETDraw *pDraw = new ETDraw;
		pDraw->Type = DrawType;
		
		pType->DrawProc.push_back(pDraw);
		
		return pDraw;
	}
	
  /*============================================================================
  Dump debug bonus info to stdout
  ============================================================================*/
	void EditorData::consoleDump(void) {
		printf("****** Editor Data ******\n");
		printf(" %d entity type(s):\n\n",m_EntityTypes.size());
		
		for(int i=0;i<m_EntityTypes.size();i++) {
			EditorEntityType *pType = m_EntityTypes[i];
			printf("  TYPEID=%s\n",pType->ID.c_str());
			printf("  SIZE=%f\n",pType->fSize);
			for(int j=0;j<pType->Params.size();j++) {
				printf("  PARAM NAME=\"%s\" DEFAULT=\"%s\"\n",
				       pType->Params[j]->Name.c_str(),pType->Params[j]->DefaultValue.c_str());
			}
			printf("  %d symbol part(s)\n",pType->DrawProc.size());
			
			printf("\n");
		}
	}

  /*============================================================================
  Clean up
  ============================================================================*/
	void EditorData::_Free(void) {
		for(int i=0;i<m_EntityTypes.size();i++) {
			for(int j=0;j<m_EntityTypes[i]->DrawProc.size();j++) {
				delete m_EntityTypes[i]->DrawProc[j];
			}
			for(int j=0;j<m_EntityTypes[i]->Params.size();j++) {
				delete m_EntityTypes[i]->Params[j];
			}
			delete m_EntityTypes[i];
		}
		m_EntityTypes.clear();
	}

};

