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

#include "xmscene/Level.h"
#include "xmDatabase.h"

void xmDatabase::levels_add_begin(bool i_isToReload) {
  simpleSql("BEGIN TRANSACTION;");

  if(i_isToReload) {
    simpleSql("DELETE FROM levels WHERE isToReload=1;");
  } else {
    simpleSql("DELETE FROM levels;");
  }
}

void xmDatabase::levels_add_end() {
  simpleSql("COMMIT;");
}

void xmDatabase::levels_updateDB(const std::vector<Level *>&    i_levels,
				 bool i_isToReload,
				 XmDatabaseUpdateInterface *i_interface) {

  simpleSql("BEGIN TRANSACTION;");
  simpleSql("DELETE FROM levels;");

  for(int i=0; i<i_levels.size(); i++) {
    levels_add(i_levels[i]->Id()      , i_levels[i]->FileName(), i_levels[i]->Name(),
	       i_levels[i]->Checksum(), i_levels[i]->Author()  , i_levels[i]->Description(),
	       i_levels[i]->Date()    , i_levels[i]->Pack()    , i_levels[i]->PackNum(),
	       i_levels[i]->Music()   , i_levels[i]->isScripted(),
	       i_isToReload
	       );
  }

  simpleSql("COMMIT;");
}

bool xmDatabase::levels_isIndexUptodate() const {
  return m_requiredLevelsUpdateAfterInit == false;
}

void xmDatabase::levels_addToFavorite(const std::string& i_profile, const std::string& i_id_level) {
  simpleSql("INSERT INTO levels_favorite(id_profile, id_level) "
	    "VALUES(\"" + protectString(i_profile)
	    + "\", \""  + protectString(i_id_level)+ "\");");
}

void xmDatabase::levels_delToFavorite(const std::string& i_profile, const std::string& i_id_level) {
  simpleSql("DELETE FROM levels_favorite "
	    "WHERE id_profile=\""   + protectString(i_profile)
	    + "\" AND  id_level=\"" + protectString(i_id_level)+ "\";");
}

void xmDatabase::updateDB_favorite(const std::string& i_profile,
				   XmDatabaseUpdateInterface *i_interface) {
  vapp::XMLDocument v_favoriteLevelsXml;
  TiXmlDocument *v_favoriteLevelsXmlData;
  TiXmlElement *v_favoriteLevelsXmlDataElement;
  const char *pc;
  std::string v_levelId;
  
  v_favoriteLevelsXml.readFromFile("favoriteLevels.xml");
  v_favoriteLevelsXmlData = v_favoriteLevelsXml.getLowLevelAccess();

  if(v_favoriteLevelsXmlData == NULL) {
    throw Exception("error : unable to analyze xml favoriteLevels file");
  }

  v_favoriteLevelsXmlDataElement = v_favoriteLevelsXmlData->FirstChildElement("favoriteLevels");
  
  if(v_favoriteLevelsXmlDataElement == NULL) {
    throw Exception("error : unable to analyze xml favoriteLevels file");
  }
    
  try {
    simpleSql("BEGIN TRANSACTION;");

    TiXmlElement *pVarElem = v_favoriteLevelsXmlDataElement->FirstChildElement("level");
    while(pVarElem != NULL) {
      v_levelId = "";
      
      pc = pVarElem->Attribute("id");
      if(pc != NULL) {
	v_levelId = pc;
	
	/* add the level into the list */
	simpleSql("INSERT INTO levels_favorite(id_profile, id_level) "
		  "VALUES(\"" + protectString(i_profile)
		  + "\", \""  + protectString(v_levelId)+ "\");");
      }
      pVarElem = pVarElem->NextSiblingElement("level");
    }
    simpleSql("COMMIT;");
  } catch(Exception &e) {
    simpleSql("ROLLBACK;");
  }
}

void xmDatabase::levels_cleanNew() {
   simpleSql("DELETE FROM levels_new;");
}

void xmDatabase::levels_addToNew(const std::string& i_id_level, bool i_isAnUpdate) {
   simpleSql("INSERT INTO levels_new(id_level, isAnUpdate) "
	     "VALUES (\"" + protectString(i_id_level) + "\", "
	     + std::string(i_isAnUpdate ? "1":"0") + ");");
}

void xmDatabase::levels_add(const std::string& i_id_level,
			    const std::string& i_filepath,
			    const std::string& i_name,
			    const std::string& i_checkSum,
			    const std::string& i_author,
			    const std::string& i_description,
			    const std::string& i_date,
			    const std::string& i_packName,
			    const std::string& i_packNum,
			    const std::string& i_music,
			    bool i_isScripted,
			    bool i_isToReload) {
  simpleSql("INSERT INTO levels(id_level,"
	    "filepath, name, checkSum, author, description, "
	    "date_str, packName, packNum, music, isScripted, isToReload) "
	    "VALUES(\"" + protectString(i_id_level) + "\", " +
	    "\"" + protectString(i_filepath)        + "\", " +
	    "\"" + protectString(i_name)            + "\", " +
	    "\"" + protectString(i_checkSum)        + "\", " +
	    "\"" + protectString(i_author)          + "\", " +
	    "\"" + protectString(i_description)     + "\", " +
	    "\"" + protectString(i_date)            + "\", " +
	    "\"" + protectString(i_packName)        + "\", " +
	    "\"" + protectString(i_packNum)         + "\", " +
	    "\"" + protectString(i_music)           + "\", " +
	    std::string(i_isScripted  ? "1" : "0")  + ", "   +
	    std::string(i_isToReload  ? "1" : "0")  +
	    std::string(");"));
}

void xmDatabase::levels_update(const std::string& i_id_level,
			       const std::string& i_filepath, const std::string& i_name,
			       const std::string& i_checkSum,
			       const std::string& i_author, const std::string& i_description,
			       const std::string& i_date, const std::string& i_packName, const std::string& i_packNum,
			       const std::string& i_music, bool i_isScripted,
			       bool i_isToReload) {
  simpleSql("UPDATE levels SET name=\""  + 
	    protectString(i_name)     	 + "\", filepath=\"" +
	    protectString(i_filepath) 	 + "\", checkSum=\"" + 
	    protectString(i_checkSum) 	 + "\", author=\"" +
	    protectString(i_author)   	 + "\", description=\"" +
	    protectString(i_description) + "\", date_str=\"" +
	    protectString(i_date)        + "\", packName=\"" +
	    protectString(i_packName)    + "\", packNum=\"" +
	    protectString(i_packNum)     + "\", music=\"" +
	    protectString(i_music)       + "\", isScripted=" +
	    std::string(i_isScripted  ? "1" : "0") + ", isToReload=" +
	    std::string(i_isToReload  ? "1" : "0") + " WHERE id_level=\"" +
	    protectString(i_id_level)    + "\";");
}
