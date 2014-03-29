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

#ifndef __PACKAGER_H__
#define __PACKAGER_H__

  /*===========================================================================
  Packager class
  ===========================================================================*/
  class Packager {
    public:
      /* Methods */
      static void go(const std::string &BinFile, const std::string &DataDir);
      static void goUnpack(const std::string &BinFile,const std::string &OutPath,bool bWritePackageList);
      
    private:
      /* Helpers */
      static FILE *createFile(const std::string &TargetDir,const std::string &Path);
  };

#endif

