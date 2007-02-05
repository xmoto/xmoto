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
 *  User configuration management.
 */
#include "VFileIO.h"
#include "VXml.h"
#include "UserConfig.h"

namespace vapp {

  /*===========================================================================
  Load from XML
  ===========================================================================*/
	void UserConfig::loadFile(void) {
	  XMLDocument ConfigDoc;
	  
	  ConfigDoc.readFromFile("config.dat");
	  TiXmlDocument *pConfigData = ConfigDoc.getLowLevelAccess();
	  if(pConfigData != NULL) {
	    TiXmlElement *pUserConfigElem = pConfigData->FirstChildElement("userconfig");
	    if(pUserConfigElem != NULL) {
	      for(TiXmlElement *pVarElem = pUserConfigElem->FirstChildElement("var"); pVarElem!=NULL;
	          pVarElem = pVarElem->NextSiblingElement("var")) {
	        std::string Name,Value;
	        const char *pc;
	        pc = pVarElem->Attribute("name");
	        if(pc!=NULL) Name = pc;
	        pc = pVarElem->Attribute("value");
	        if(pc!=NULL) Value = pc;	        
	        
	        setValue(Name,Value);
	      }
	    }
	    else 
	      Log("** Warning ** : no configuration in 'config.dat'");
	  }
	  else
	    Log("** Warning ** : failed to load or parse user configuration 'config.dat'");
	}

  /*===========================================================================
  Save to XML
  ===========================================================================*/
	void UserConfig::saveFile(void) {
	  /* Save configuration */
	  FileHandle *pfh = FS::openOFile("config.dat");
	  if(pfh != NULL) {
	    FS::writeLine(pfh,"<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	    FS::writeLine(pfh,"");
	    FS::writeLine(pfh,"<userconfig>");
	    
	    for(int i=0;i<m_Vars.size();i++) {
	      char cBuf[256];
	      sprintf(cBuf,"\t<var name=\"%s\" value=\"%s\"/>",m_Vars[i]->Name.c_str(),m_Vars[i]->Value.c_str());
	      FS::writeLine(pfh,cBuf);
	    }
	  
	    FS::writeLine(pfh,"</userconfig>");
	    FS::writeLine(pfh,"");
	    FS::closeFile(pfh);
	  }
	  else
	    Log("** Warning ** : failed to save user configuration 'config.dat'");
	}

  /*===========================================================================
  Public helpers
  ===========================================================================*/
	float UserConfig::getFloat(std::string Name) {
	  return atof(getValue(Name).c_str());
	}
	
	std::string UserConfig::getString(std::string Name) {
	  return getValue(Name);
	}
	
	bool UserConfig::getBool(std::string Name) {
	  if(getValue(Name) == "true") return true;
	  return false;
	}
	
	int UserConfig::getInteger(std::string Name) {
	  return atoi(getValue(Name).c_str());
	}
	
	void UserConfig::setFloat(std::string Name,float v) {
	  char cBuf[256]; sprintf(cBuf,"%f",v);
	  setValue(Name,cBuf);
	}
	
	void UserConfig::setString(std::string Name,std::string v) {
	  setValue(Name,v);
	}
	
	void UserConfig::setBool(std::string Name,bool v) {
    if(v)
	    setValue(Name,"true");
	  else
	    setValue(Name,"false");	  
	}
	
	void UserConfig::setInteger(std::string Name,int v) {
	  char cBuf[256]; sprintf(cBuf,"%d",v);
	  setValue(Name,cBuf);
	}

  /*===========================================================================
  Low level management and fun
  ===========================================================================*/
	UserConfigVar *UserConfig::createVar(std::string Name,std::string DefaultValue) {
	  if(_FindVarByName(Name) != NULL) return NULL;
    UserConfigVar *pVar = new UserConfigVar;
    
    pVar->Name = Name;
    pVar->DefaultValue = pVar->Value = DefaultValue;
    m_Vars.push_back(pVar);
    
    return pVar;    
	}
	
	std::string UserConfig::getDefaultValue(std::string Name) {
	  UserConfigVar *pVar = _FindVarByName(Name);
	  if(pVar == NULL) {
	    Log("** Warning ** : impossible to get default value of unknown configuration variable '%s'",Name.c_str());
	    return "";
	  }
	  return pVar->DefaultValue;
	}
	
	std::string UserConfig::getValue(std::string Name) {
	  UserConfigVar *pVar = _FindVarByName(Name);
	  if(pVar == NULL) {
	    Log("** Warning ** : impossible to get value of unknown configuration variable '%s'",Name.c_str());
	    return "";
	  }
	  return pVar->Value;
	}
	
	void UserConfig::setValue(std::string Name,std::string Value) {
	  UserConfigVar *pVar = _FindVarByName(Name);
	  if(pVar == NULL) {
	    Log("** Warning ** : impossible to set value of unknown configuration variable '%s'",Name.c_str());
	    return;
	  }
	  
	  /* If this is a NEW value, then set "changed" flag */
	  if(pVar->Value != Value) setChanged(true);
    	  
    /* Set it */
	  pVar->Value = Value;
	}

  /*===========================================================================
  Free config
  ===========================================================================*/
  void UserConfig::_FreeUserConfig(void) {
    for(int i=0;i<m_Vars.size();i++) delete m_Vars[i];
  }
	    
  /*===========================================================================
  Misc helpers
  ===========================================================================*/
	UserConfigVar *UserConfig::_FindVarByName(std::string Name) {
	  for(int i=0;i<m_Vars.size();i++) {
	    if(m_Vars[i]->Name == Name) return m_Vars[i];
	  }
	  return NULL;
	}
	
};
