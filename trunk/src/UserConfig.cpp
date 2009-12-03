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

/* 
 *  User configuration management.
 */
#include "VFileIO.h"
#include "VXml.h"
#include "UserConfig.h"
#include "helpers/Log.h"


  /*===========================================================================
  Load from XML
  ===========================================================================*/
	void UserConfig::loadFile(void) {
	  XMLDocument ConfigDoc;
	  
	  ConfigDoc.readFromFile(FDT_CONFIG, XM_CONFIGFILE);
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
	      LogWarning("no configuration in " XM_CONFIGFILE);
	  }
	  else
	    LogWarning("failed to load or parse user configuration " XM_CONFIGFILE);
	}

  /*===========================================================================
  Save to XML
  ===========================================================================*/
	void UserConfig::saveFile(void) {
	  /* Save configuration */
	  FileHandle *pfh = XMFS::openOFile(FDT_CONFIG, XM_CONFIGFILE);
	  if(pfh != NULL) {
	    XMFS::writeLine(pfh,"<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	    XMFS::writeLine(pfh,"");
	    XMFS::writeLine(pfh,"<userconfig>");
	    
	    for(unsigned int i=0;i<m_Vars.size();i++) {
	      char cBuf[256];
	      snprintf(cBuf, 256, "\t<var name=%-25s value=%s />",
								 ("\"" + XML::str2xmlstr(m_Vars[i]->Name)  + "\"").c_str(),
								 ("\"" + XML::str2xmlstr(m_Vars[i]->Value) + "\"").c_str());
	      XMFS::writeLine(pfh,cBuf);
	    }
	  
	    XMFS::writeLine(pfh,"</userconfig>");
	    XMFS::writeLine(pfh,"");
	    XMFS::closeFile(pfh);
	  }
	  else
	    LogWarning("failed to save user configuration " XM_CONFIGFILE);
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
	  char cBuf[256];
	  snprintf(cBuf,256, "%f", v);
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
	  char cBuf[256];
	  snprintf(cBuf, 256, "%d", v);
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
	    LogWarning("impossible to get default value of unknown configuration variable '%s'",Name.c_str());
	    return "";
	  }
	  return pVar->DefaultValue;
	}
	
	std::string UserConfig::getValue(std::string Name) {
	  UserConfigVar *pVar = _FindVarByName(Name);
	  if(pVar == NULL) {
	    LogWarning("impossible to get value of unknown configuration variable '%s'",Name.c_str());
	    return "";
	  }
	  return pVar->Value;
	}
	
	void UserConfig::setValue(std::string Name,std::string Value) {
	  UserConfigVar *pVar = _FindVarByName(Name);
	  if(pVar == NULL) {
	    LogWarning("impossible to set value of unknown configuration variable '%s'",Name.c_str());
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
    for(unsigned int i=0;i<m_Vars.size();i++) delete m_Vars[i];
  }
	    
  /*===========================================================================
  Misc helpers
  ===========================================================================*/
	UserConfigVar *UserConfig::_FindVarByName(std::string Name) {
	  for(unsigned int i=0;i<m_Vars.size();i++) {
	    if(m_Vars[i]->Name == Name) return m_Vars[i];
	  }
	  return NULL;
	}

