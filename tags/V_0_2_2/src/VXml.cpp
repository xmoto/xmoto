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
 *  XML support through the tinyxml library.
 */
#include "VApp.h"
#include "VXml.h"
#include "VFileIO.h"
#include "CRCHash.h"

namespace vapp {

  /*===========================================================================
  Load XML document from disk
  ===========================================================================*/
  void XMLDocument::readFromFile(std::string File,unsigned long *pnCRC32) {
    /* Clean up if a doc already is loaded */
    if(m_pXML) delete m_pXML;
    
    /* Load */
    std::string Line,Doc = "";    
    
    FileHandle *pfh = FS::openIFile(File);
    if(pfh==NULL) return;

    m_pXML = new TiXmlDocument;
    
    while(FS::readNextLine(pfh,Line)) {
      if(Line.length() > 0) {
        Doc.append(Line);
        Doc.append("\n");
      }
    }    
    FS::closeFile(pfh);
    
    /* Are we just going to need the CRC? */
    if(pnCRC32 != NULL) {
      *pnCRC32 = CRC32::computeCRC32((const unsigned char *)Doc.c_str(),Doc.length());
    }
    else {
      /* Parse XML */
      m_pXML->Parse(Doc.c_str());
      if(m_pXML->Error()) {
        Log("** Warning ** : XML-parsing error in '%s' : %s",File.c_str(),m_pXML->ErrorDesc());
      }
    }
  }
      
  /*===========================================================================
  Save XML document from disk
  ===========================================================================*/
  void XMLDocument::writeToFile(std::string File) {
    /* Anything? */
    if(!m_pXML) return;
    FileHandle *pfh = FS::openOFile(File);
    if(pfh==NULL || pfh->Type!=FHT_STDIO) return;
    m_pXML->Print(pfh->fp);
    FS::closeFile(pfh);
  }

  std::string XML::str2xmlstr(std::string str) {
    std::string v_res = "";
    for(int i=0; i<str.length(); i++) {
      switch(str[i]) {
      case '&':
	v_res.append("&amp;");
	break;
      case '<':
	v_res.append("&lt;");
	break;
      case '>':
	v_res.append("&gt;");
	break;
      case '\"':
	v_res.append("&quot;");
	break;
      case '\'':
	v_res.append("&apos;");
	break;
      default:
	char c[2] = {str[i], '\0'};
	v_res.append(c);
      }
    }
    return v_res;
  }
    
}
