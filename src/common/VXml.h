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

#include <string>
#include "VFileIO_types.h"
#include <libxml/parser.h>
#include <libxml/tree.h>

class XMLDocument {
  public:
  XMLDocument();
  ~XMLDocument();

  // clean the class
  static void clean();
  
  /* Methods */
  void readFromFile(FileDataType i_fdt, std::string File, bool i_includeCurrentDir=false);

  xmlNodePtr getRootNode(const char* rootNameToCheck = NULL);

  static xmlNodePtr subElement (xmlNodePtr node, const char* name);
  static xmlNodePtr nextElement(xmlNodePtr node, const char* name = NULL);

  static std::string getOption(xmlNodePtr node, const char* name, std::string Default = "");
  static std::string getOption(xmlNodePtr node, xmlChar*    name, std::string Default = "");
  static std::string getElementText(xmlNodePtr node);

  static std::string str2xmlstr(std::string str);
  
  private:
  static void XMLDocumentErrorFunc(void* ctx, const char* msg, ...);

  xmlDocPtr m_doc;
};

#endif

