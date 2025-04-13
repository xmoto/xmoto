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

#ifndef __VFILEIO_H__
#define __VFILEIO_H__

#include <string>
#include <vector>
#ifndef WIN32
#include <basedir.h>
#endif
#include "VFileIO_types.h"

/*===========================================================================
  File handle types
  ===========================================================================*/
enum FileHandleType { FHT_UNASSIGNED, FHT_STDIO, FHT_PACKAGE };

/*===========================================================================
  Packaged files
  ===========================================================================*/
struct PackFile {
  PackFile() { nSize = nOffset = 0; }

  std::string Name;
  std::string md5sum;
  int nSize, nOffset;
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
  int nRead, nWrite;
  char cBuffer[256];

  FILE *fp; /* File pointer for I/O */

  int nOffset; /* in package */

  /* I/O mode */
  bool bRead, bWrite;
};

/*===========================================================================
  File system I/O static stuff
  ===========================================================================*/
class XMFS {
public:
  /* Methods */
  static void init(const std::string &AppDir,
                   const std::string &i_binFile,
                   bool i_graphics,
                   const std::string &i_userCustomDirPath = "");
  static void uninit();
  static bool isInitialized();

  static std::vector<std::string> findPhysFiles(FileDataType i_fdt,
                                                std::string Files,
                                                bool bRecurse = false);

  static bool copyFile(
    FileDataType i_fdt,
    const std::string &From,
    const std::string &To,
    /* To_really_done is out : it is the name of the file really written */
    std::string &To_really_done,
    bool mkdirs = false);

  static bool moveFile(const std::string &From, const std::string &To);

  static void deleteFile(FileDataType i_fdt, const std::string &File);

  static FileHandle *openOFile(FileDataType i_fdt, const std::string &Path);
  static FileHandle *openIFile(FileDataType i_fdt,
                               std::string Path,
                               bool i_includeCurrentDir = false);

  static void closeFile(FileHandle *pfh);
  static bool areSamePath(const std::string &i_path1,
                          const std::string &i_path2);

  static bool readBuf(FileHandle *pfh, char *pcBuf, unsigned int nBufSize);
  static bool writeBuf(FileHandle *pfh, char *pcBuf, unsigned int nBufSize);
  static bool setOffset(FileHandle *pfh, int nOffset);
  static bool setEnd(FileHandle *pfh);
  static int getOffset(FileHandle *pfh);
  static int getLength(FileHandle *pfh);
  static bool isEnd(FileHandle *pfh);
  static std::string readFileToEnd(FileHandle *pfh);
  static bool readNextLine(FileHandle *pfh,
                           std::string &Line); /* uses buffered reading */
  static int readByte(FileHandle *pfh);
  static int readShort(FileHandle *pfh);
  static int readInt(FileHandle *pfh);
  static float readFloat(FileHandle *pfh);
  static double readDouble(FileHandle *pfh);
  static std::string readString(FileHandle *pfh);
  static std::string readLongString(FileHandle *pfh);
  static bool readBool(FileHandle *pfh);

  static void writeByte(FileHandle *pfh, unsigned char v);
  static void writeShort(FileHandle *pfh, short v);
  static void writeInt(FileHandle *pfh, int v);
  static void writeFloat(FileHandle *pfh, float v);
  static void writeDouble(FileHandle *pfh, double v);
  static void writeString(FileHandle *pfh, const std::string &v);
  static void writeLongString(FileHandle *pfh, const std::string &v);
  static void writeBool(FileHandle *pfh, bool v);

  static void writeLine(FileHandle *pfh, const std::string &Line);
  static void writeLineF(FileHandle *pfh, char *pcFmt, ...);

  /* Endian-safe helpers */
  static void writeShort_LE(FileHandle *pfh, short v);
  static void writeInt_LE(FileHandle *pfh, int v);
  static void writeFloat_LE(FileHandle *pfh, float v);
  static int readShort_LE(FileHandle *pfh);
  static int readInt_LE(FileHandle *pfh);
  static float readFloat_LE(FileHandle *pfh);

  /* Allow caller to specify that the order is wrong. */
  static int readShort_MaybeLE(FileHandle *pfh, bool big);
  static int readInt_MaybeLE(FileHandle *pfh, bool big);
  static float readFloat_MaybeLE(FileHandle *pfh, bool big);

  /* For buffered reading: */
  static int readBufferedChar(FileHandle *pfh);
  static int peekNextBufferedChar(FileHandle *pfh);
  static int fillBuffer(FileHandle *pfh);

  /* File name mangling */
  static std::string getFileDir(const std::string &Path);
  static std::string getFileBaseName(const std::string &Path,
                                     bool withExt = false);
  static std::string getFileExtension(
    const std::string &Path); // do not require FS initialization

  /* Misc */
  static bool isDir(const std::string &AppDir);

  static int getFileTimeStamp(const std::string &Path);
  static bool isPathAbsolute(const std::string &Path);

  static int mkDir(const char *pcPath);

  /* Data interfaces */
  static std::string getUserDir(FileDataType i_fdt);
  static std::string getUserDirUTF8(FileDataType i_fdt);
  static std::string getSystemDataDir();
  static std::string getSystemLocaleDir();
  static bool isSystemDataDirAvailable() { return m_bGotSystemDataDir; }

  static std::string getUserReplaysDir() {
    return m_UserDataDir + std::string("/Replays");
  }
  static std::string getUserLevelsDir() {
    return m_UserDataDir + std::string("/Levels");
  }

  static bool doesRealFileOrDirectoryExists(const std::string &p_path);
  static bool isFileReadable(FileDataType i_fdt, const std::string &p_filename);
  static bool fileExists(FileDataType i_fdt, const std::string &p_filename);
  static void mkArborescence(const std::string &v_filepath);
  static void mkArborescenceDir(const std::string &v_dirpath);

  // return true if p_filepath is a path from user dir
  static bool isInUserDir(FileDataType i_fdt, const std::string &p_filepath);
  static bool isFileInDir(const std::string &p_dirpath,
                          const std::string &p_filepath);

  /* return false if the file is in the package */
  static bool isFileReal(const std::string &i_filePath);
  static std::string md5sum(FileDataType i_fdt, std::string i_filePath);

  /* return user_dir + i_relative_path if exists or data_dir + i_relative_path
   */
  static std::string FullPath(FileDataType i_fdt,
                              const std::string &i_relative_path);
  static std::string binCheckSum();

  static void deleteFile(const std::string &i_filepath);

private:
  static bool m_isInitialized;

#ifndef WIN32
  /* xdg basedir */
  static xdgHandle *m_xdgHd;
#endif

  /* Helper functions */
  static void _ThrowFileError(FileHandle *pfh, const std::string &Description);
  static void _FindFilesRecursive(const std::string &Dir,
                                  const std::string &Wildcard,
                                  std::vector<std::string> &List);

  /* Data */
  static std::string m_UserDataDir, m_UserDataDirUTF8, m_UserConfigDir,
    m_UserCacheDir, m_SystemDataDir, m_SystemLocaleDir;
  static bool m_bGotSystemDataDir;

  static std::string m_BinDataFile;
  static std::vector<PackFile> m_PackFiles;
  static std::string m_binCheckSum;

  // migrate from .xmoto to xdg base directories
  static void migrateFSToXdgBaseDirIfRequired(const std::string &AppDir);
  static void migrateFSToXdgBaseDirFile(const std::string &i_src,
                                        const std::string &i_dest);
};

#endif
