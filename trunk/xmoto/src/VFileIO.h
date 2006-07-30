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

#ifndef __VFILEIO_H__
#define __VFILEIO_H__

#include "VCommon.h"
#include "tinyxml/tinyxml.h"

namespace vapp {

	/*===========================================================================
	File handle types
  ===========================================================================*/
  enum FileHandleType {
    FHT_UNASSIGNED,
    FHT_STDIO,
    FHT_PACKAGE
  };

	/*===========================================================================
	Packaged files
  ===========================================================================*/
  #define MAX_PACK_FILES      1024
  struct PackFile {
    PackFile() {
      nSize = nOffset = 0;
    }
  
    std::string Name;
    int nSize,nOffset;
  };

	/*===========================================================================
	File handles
  ===========================================================================*/
  struct FileHandle {
    FileHandle() {
      Type = FHT_UNASSIGNED;      
      nRead = nWrite = nSize = 0;
      fp = NULL;
      bRead = bWrite = false;
    }
  
    std::string Name;
    FileHandleType Type;    
    int nSize;

    /* Buffer stuff */
    int nRead,nWrite;
    char cBuffer[256];

    FILE *fp;                 /* File pointer for I/O */
  
    int nOffset; /* in package */   
    
    /* I/O mode */
    bool bRead,bWrite;
  };

	/*===========================================================================
	File system I/O static stuff
  ===========================================================================*/
  class FS {
    public:        
      /* Methods */
      static void init(std::string AppDir);
      
      static std::vector<std::string> findPhysFiles(std::string Files,bool bRecurse = false);
      
      static bool copyFile(const std::string &From,const std::string &To);
      static void deleteFile(const std::string &File);
      
      static void writeLog(const std::string &s);
      static FileHandle *openOFile(std::string Path);
      static FileHandle *openIFile(std::string Path);
      static void closeFile(FileHandle *pfh);

      static bool readBuf(FileHandle *pfh,char *pcBuf, unsigned int nBufSize);
      static bool writeBuf(FileHandle *pfh,char *pcBuf, unsigned int nBufSize);     
      static bool setOffset(FileHandle *pfh,int nOffset);
      static bool setEnd(FileHandle *pfh);
      static int getOffset(FileHandle *pfh);
      static int getLength(FileHandle *pfh);
      static bool isEnd(FileHandle *pfh);
      
      static bool readNextLine(FileHandle *pfh,std::string &Line); /* uses buffered reading */
      
      static int readByte(FileHandle *pfh);
      static int readShort(FileHandle *pfh);
      static int readInt(FileHandle *pfh);
      static float readFloat(FileHandle *pfh);
      static double readDouble(FileHandle *pfh);
      static std::string readString(FileHandle *pfh);
      static std::string readLongString(FileHandle *pfh);
      static bool readBool(FileHandle *pfh);    

      static void writeByte(FileHandle *pfh,unsigned char v);
      static void writeShort(FileHandle *pfh,short v);
      static void writeInt(FileHandle *pfh,int v);
      static void writeFloat(FileHandle *pfh,float v);
      static void writeDouble(FileHandle *pfh,double v);
      static void writeString(FileHandle *pfh,std::string v);
      static void writeLongString(FileHandle *pfh,std::string v);
      static void writeBool(FileHandle *pfh,bool v);    
      
      static void writeLine(FileHandle *pfh,std::string Line);
      static void writeLineF(FileHandle *pfh,char *pcFmt,...);
      
      /* Endian-safe helpers */
      static void writeShort_LE(FileHandle *pfh,short v);
      static void writeInt_LE(FileHandle *pfh,int v);
      static void writeFloat_LE(FileHandle *pfh,float v);
      static int readShort_LE(FileHandle *pfh);
      static int readInt_LE(FileHandle *pfh);
      static float readFloat_LE(FileHandle *pfh);      
      
      /* For buffered reading: */
      static int readBufferedChar(FileHandle *pfh);
      static int peekNextBufferedChar(FileHandle *pfh); 
      static int fillBuffer(FileHandle *pfh);      
      
      /* File name mangling */
      static std::string getFileDir(std::string Path);
      static std::string getFileBaseName(std::string Path);
      
      /* Misc */
      static bool isDir(std::string AppDir);
      static int getFileTimeStamp(const std::string &Path);
      static bool isPathAbsolute(std::string Path);
      
      static int mkDir(const char *pcPath);
      
      /* Data interfaces */
      static std::string getUserDir(void) {return m_UserDir;}
      static std::string getDataDir(void) {return m_DataDir;}
      static bool isDataDirAvailable(void) {return m_bGotDataDir;}
      
      static std::string getReplaysDir(void) {return m_UserDir + std::string("/Replays");}
      static std::string getLevelsDir(void) {return m_UserDir + std::string("/Levels");}
    
      static bool doesDirectoryExist(std::string p_path);
      static bool isFileReadable(std::string p_filename);
      static bool fileExists(std::string p_filename);
      static void mkArborescence(std::string v_filepath);

      // return true if p_filepath is a path from user dir
      static bool isInUserDir(std::string p_filepath);

    private:
      /* Helper functions */
      static void _ThrowFileError(FileHandle *pfh,std::string Description);
      static void _FindFilesRecursive(const std::string &Dir,const std::string &Wildcard,std::vector<std::string> &List);
      
      /* Data */
      static std::string m_UserDir,m_DataDir;
      static bool m_bGotDataDir;
      
      static std::string m_BinDataFile;      
      static int m_nNumPackFiles;
      static PackFile m_PackFiles[MAX_PACK_FILES];
  };

}

#endif

