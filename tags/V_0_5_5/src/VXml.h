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

#ifndef __VXML_H__
#define __VXML_H__

#include "VCommon.h"
#include "tinyxml/tinyxml.h"
#include <string>
#include "VFileIO_types.h"

  /*===========================================================================
  XML document
  ===========================================================================*/
  class XMLDocument {
    public:
      XMLDocument() {m_pXML = NULL;}
      ~XMLDocument() {if(m_pXML) delete m_pXML;}
    
      /* Methods */
      /* (if pnCRC32!=NULL, then readFromFile() should not parse XML,
          but just calculate the CRC32 of the text file) */
      void readFromFile(FileDataType i_fdt, std::string File,unsigned long *pnCRC32=NULL, bool i_includeCurrentDir=false);
      void writeToFile(FileDataType i_fdt, std::string File);      
      
      /* Data interface */
      TiXmlDocument *getLowLevelAccess(void) {return m_pXML;}
    
    private:
      /* Data */
      TiXmlDocument *m_pXML;
  };


  class XML {
  public:
    static std::string str2xmlstr(std::string str);
    static std::string getOption(TiXmlElement *pElem,std::string Name,std::string Default="");
    static TiXmlElement *findElement(XMLDocument& i_source, TiXmlElement *pRoot,const std::string &Name); 
    static std::string getElementText(XMLDocument& i_source, TiXmlElement *pRoot,std::string Name);
    static void appendText(std::string &Text,const std::string &Append);
  };

#endif

