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

#ifndef __VXML_H__
#define __VXML_H__

#include "VCommon.h"
#include "tinyxml/tinyxml.h"

namespace vapp {

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
      void readFromFile(std::string File,unsigned long *pnCRC32=NULL);
      void writeToFile(std::string File);      
      
      /* Data interface */
      TiXmlDocument *getLowLevelAccess(void) {return m_pXML;}
    
    private:
      /* Data */
      TiXmlDocument *m_pXML;
  };

}

#endif

