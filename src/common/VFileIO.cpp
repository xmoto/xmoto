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

#include "helpers/Environment.h"
#ifdef WIN32
#include <direct.h>
#include <io.h>
#include <userenv.h>
#include <winbase.h>
#include <windows.h>
#else
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <stdarg.h>
#include <sys/stat.h>

#if BUILD_MACOS_BUNDLE
#include <CoreFoundation/CoreFoundation.h>
#include <limits.h> /* PATH_MAX */
#endif

#include "VFileIO.h"
#include "helpers/Log.h"
#include "helpers/SwapEndian.h"
#include "helpers/Text.h"
#include "helpers/VExcept.h"
#include "md5sum/md5file.h"

#include "XMSession_default.h"
#include "xmoto/UserConfig.h"

#include <iostream>

#ifdef WIN32
std::string win32_getHomeDir(bool i_asUtf8 = false) {
  HANDLE hToken;
  HANDLE proc;
  std::string v_dest;
  DWORD cchPath = 1024;

  proc = GetCurrentProcess();

  if (!OpenProcessToken(proc, TOKEN_QUERY, &hToken)) {
    throw Exception("Can't determine user directory (OpenProcessToken)");
  }

  if (i_asUtf8) {
    WCHAR szProfilePath[1024];
    char v_tmp[1024];

    if (!GetUserProfileDirectoryW(hToken, szProfilePath, &cchPath)) {
      throw Exception(
        "Can't determine user directory (GetUserProfileDirectory)");
    }
    WideCharToMultiByte(CP_UTF8, 0, szProfilePath, -1, v_tmp, 1024, NULL, NULL);
    v_dest = v_tmp;

  } else {
    TCHAR szProfilePath[1024];
    if (!GetUserProfileDirectory(hToken, szProfilePath, &cchPath)) {
      throw Exception(
        "Can't determine user directory (GetUserProfileDirectory)");
    }
    v_dest = szProfilePath;
  }

  return v_dest;
}

std::string win32_getPWDDir(bool i_asUtf8 = false) {
  std::string v_dest;
  int v_len;

  if (i_asUtf8) {
    WCHAR szProfilePath[1024];
    char v_tmp[1024];

    v_len = GetCurrentDirectoryW(1024, szProfilePath);
    WideCharToMultiByte(CP_UTF8, 0, szProfilePath, -1, v_tmp, 1024, NULL, NULL);
    v_dest = v_tmp;

  } else {
    TCHAR szProfilePath[1024];
    v_len = GetCurrentDirectory(1024, szProfilePath);
    v_dest = szProfilePath;
  }

  return v_dest;
}
#endif

std::string getPWDDir(bool i_asUtf8 = false) {
#ifdef WIN32
  return win32_getPWDDir(i_asUtf8);
#else
  return std::string(getenv("PWD"));
#endif
}

#if !defined(WIN32) && !defined(__MORPHOS__) && !defined(__amigaos4__)
void strlwr(char *pc) {
  for (unsigned int i = 0; i < strlen(pc); i++)
    pc[i] = tolower(pc[i]);
}
#endif

bool XMFS::m_isInitialized = false;

#ifndef WIN32
xdgHandle *XMFS::m_xdgHd = NULL;
#endif

bool str_match_wildcard(char *pcMWildcard,
                        char *pcMString,
                        bool CaseSensitive) {
  int nPos = 0;
  bool PrevIsWildcard = false;
  char c1[256 + 1], c2[256 + 1];
  char *pcWildcard, *pcString;

  if (CaseSensitive) {
    pcWildcard = pcMWildcard;
    pcString = pcMString;
  } else {
    strncpy(c1, pcMWildcard, sizeof(c1));
    c1[sizeof(c1) - 1] = '\0';

    strncpy(c2, pcMString, sizeof(c2));
    c2[sizeof(c2) - 1] = '\0';

    strlwr(c1);
    strlwr(c2);
    pcWildcard = c1;
    pcString = c2;
  }

  if (pcWildcard[0] == '\0')
    return true;

  for (unsigned i = 0; i < strlen(pcWildcard); i++) {
    /* If end of string is reached, we have a match */
    if (pcString[nPos] == '\0' && pcWildcard[i] == '\0')
      return true;

    /* Wildcard? */
    if (pcWildcard[i] == '?' || pcWildcard[i] == '*')
      PrevIsWildcard = true;
    else {
      /* Nope. What does we accept from the string? */
      if (PrevIsWildcard) {
        /* Read string until we get the right character */
        while (1) {
          if (pcString[nPos] == '\0')
            return false;

          /* Got the char? */
          if (pcString[nPos] == pcWildcard[i])
            break;

          nPos++;
        }
        i--;

        /* Clear wildcard flag */
        PrevIsWildcard = false;
      } else {
        /* The letter in the string MUST equal the letter in the wildcard */
        if (pcWildcard[i] != pcString[nPos])
          return false;

        nPos++; /* Next */
      }
    }
  }

  /* Result of random debugging :) */
  if (PrevIsWildcard || pcString[nPos] == '\0')
    return true;

  /* Not a match */
  return false;
}

void XMFS::_ThrowFileError(FileHandle *pfh, const std::string &Description) {
  char cBuf[512];
  snprintf(cBuf,
           512,
           "%s (%s%s): %s",
           pfh->Name.c_str(),
           pfh->bRead ? "I" : "",
           pfh->bWrite ? "O" : "",
           Description.c_str());
  throw Exception(cBuf);
}

std::string XMFS::getSystemDataDir() {
  return m_SystemDataDir;
}

std::string XMFS::getSystemLocaleDir() {
  return m_SystemLocaleDir;
}

std::string XMFS::getUserDir(FileDataType i_fdt) {
  switch (i_fdt) {
    case FDT_DATA:
      return m_UserDataDir;
      break;

    case FDT_CONFIG:
      return m_UserConfigDir;
      break;

    case FDT_CACHE:
      return m_UserCacheDir;
      break;
  }

  return "";
}

std::string XMFS::getUserDirUTF8(FileDataType i_fdt) {
  if (i_fdt != FDT_DATA) {
    throw Exception("Allow only utf8 from user data directory");
  }

  return m_UserDataDirUTF8;
}

/*===========================================================================
  Code soup :)  -- this is largely copy/paste material from an earlier project

  FutureRasmus: well, this is probably the crappiest code in the world. Period.
  ===========================================================================*/
void XMFS::_FindFilesRecursive(const std::string &DirX,
                               const std::string &Wildcard,
                               std::vector<std::string> &List) {
/* Windows? */
#ifdef WIN32
  intptr_t fh;
  struct _finddata_t fd;

  std::string Dir = DirX;
  if (!DirX.empty() && DirX[DirX.length() - 1] == '/') {
    Dir = DirX.substr(0, DirX.length() - 1);
  }

  if ((fh = _findfirst((Dir + std::string("/") + Wildcard).c_str(), &fd)) !=
      -1L) {
    do {
      if (strcmp(fd.name, ".") && strcmp(fd.name, "..")) {
        List.push_back(Dir + std::string("/") + std::string(fd.name));
      }
    } while (_findnext(fh, &fd) == 0);
    _findclose(fh);
  }

  /* Now, recurse into sub-dirs */
  if ((fh = _findfirst((Dir + std::string("/*")).c_str(), &fd)) != -1L) {
    do {
      if (strcmp(fd.name, ".") && strcmp(fd.name, "..")) {
        std::string F = Dir + std::string("/") + std::string(fd.name);
        if (isDir(F)) {
          /* Recurse! */
          _FindFilesRecursive(F, Wildcard, List);
        }
      }
    } while (_findnext(fh, &fd) == 0);
    _findclose(fh);
  }
#else
  std::string Dir = DirX;

  struct dirent *dp;
  DIR *dirp = opendir(Dir.c_str());
  while (dirp) {
    if ((dp = readdir(dirp)) != NULL) {
      std::string F = Dir + std::string(dp->d_name);
      if (isDir(F)) {
        /* Recurse... */
        if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
          _FindFilesRecursive(F + std::string("/"), Wildcard, List);
        }
      } else {
        if (str_match_wildcard(
              (char *)Wildcard.c_str(), (char *)dp->d_name, true)) {
          /* Match! */
          if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
            List.push_back(F);
          }
        }
      }
    } else {
      closedir(dirp);
      break;
    }
  }
#endif
}

std::vector<std::string> XMFS::findPhysFiles(FileDataType i_fdt,
                                             std::string Files,
                                             bool bRecurse) {
  std::vector<std::string> Result;
  std::string Wildcard;
  std::string DataDirToSearch, AltDirToSearch, UDirToSearch = "";

  /* First seperate directory from wildcard */
  int n = Files.find_last_of('/');
  if (n < 0) {
    /* No directory specified */
    DataDirToSearch = m_SystemDataDir;
    AltDirToSearch = "";
    Wildcard = Files;
  } else {
    /* Seperate dir and wildcard */
    DataDirToSearch =
      m_SystemDataDir + std::string("/") + Files.substr(0, n + 1);
    UDirToSearch =
      getUserDir(i_fdt) + std::string("/") + Files.substr(0, n + 1);
    AltDirToSearch = Files.substr(0, n + 1);
    Wildcard = Files.substr(n + 1);
  }

/* 1. find in user dir */
/* 2. find in data dir */
/* 3. find in package  */

/* Windows? */
#ifdef WIN32
  if (UDirToSearch != "") {
    if (bRecurse) {
      _FindFilesRecursive(UDirToSearch, Wildcard, Result);
    } else {
      intptr_t fh;
      struct _finddata_t fd;

      if ((fh = _findfirst((UDirToSearch + Wildcard).c_str(), &fd)) != -1L) {
        do {
          if (strcmp(fd.name, ".") && strcmp(fd.name, "..")) {
            Result.push_back(UDirToSearch + std::string(fd.name));
          }
        } while (_findnext(fh, &fd) == 0);
        _findclose(fh);
      }
    }
  }

  if (i_fdt == FDT_DATA) {
    if (bRecurse) {
      _FindFilesRecursive(DataDirToSearch, Wildcard, Result);
    } else {
      intptr_t fh;
      struct _finddata_t fd;

      if ((fh = _findfirst((DataDirToSearch + Wildcard).c_str(), &fd)) != -1L) {
        do {
          if (strcmp(fd.name, ".") && strcmp(fd.name, "..")) {
            Result.push_back(DataDirToSearch + std::string(fd.name));
          }
        } while (_findnext(fh, &fd) == 0);
        _findclose(fh);
      }
    }
  }

#else /* Assume linux, unix... yalla yalla... */
  if (UDirToSearch != "") {
    if (bRecurse) {
      _FindFilesRecursive(UDirToSearch, Wildcard, Result);
    } else {
      struct dirent *dp;
      DIR *dirp = opendir(UDirToSearch.c_str());
      while (dirp) {
        if ((dp = readdir(dirp)) != NULL) {
          if (str_match_wildcard(
                (char *)Wildcard.c_str(), (char *)dp->d_name, true)) {
            /* Match! */
            if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
              Result.push_back(UDirToSearch + std::string(dp->d_name));
            }
          }
        } else {
          closedir(dirp);
          break;
        }
      }
    }
  }

  if (AltDirToSearch != UDirToSearch) {
    if (bRecurse) {
      _FindFilesRecursive(AltDirToSearch, Wildcard, Result);
    } else {
      struct dirent *dp;
      DIR *dirp = opendir(AltDirToSearch.c_str());
      while (dirp) {
        if ((dp = readdir(dirp)) != NULL) {
          if (str_match_wildcard(
                (char *)Wildcard.c_str(), (char *)dp->d_name, true)) {
            /* Match! */
            if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
              Result.push_back(AltDirToSearch + std::string(dp->d_name));
            }
          }
        } else {
          closedir(dirp);
          break;
        }
      }
    }
  }

  if (i_fdt == FDT_DATA) {
    if (DataDirToSearch != AltDirToSearch && DataDirToSearch != UDirToSearch) {
      if (bRecurse) {
        _FindFilesRecursive(DataDirToSearch, Wildcard, Result);
      } else {
        /* Search directories */
        struct dirent *dp;
        DIR *dirp = opendir(DataDirToSearch.c_str());
        while (dirp) {
          if ((dp = readdir(dirp)) != NULL) {
            if (str_match_wildcard(
                  (char *)Wildcard.c_str(), (char *)dp->d_name, true)) {
              /* Match! */
              if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
                Result.push_back(DataDirToSearch + std::string(dp->d_name));
              }
            }
          } else {
            closedir(dirp);
            break;
          }
        }
      }
    }
  }
#endif

  if (i_fdt == FDT_DATA) {
    /* Look in package */
    for (unsigned int i = 0; i < m_PackFiles.size(); i++) {
      /* Make sure about the directory... */
      int k = Files.find_last_of('/');
      std::string Ds1 = Files.substr(0, k + 1);
      k = m_PackFiles[i].Name.find_last_of('/');
      std::string Ds2 = m_PackFiles[i].Name.substr(0, k + 1);
      if (Ds1.substr(0, 2) == "./")
        Ds1.erase(Ds1.begin(), Ds1.begin() + 2);

      if (Ds1 == Ds2 && str_match_wildcard((char *)Files.c_str(),
                                           (char *)m_PackFiles[i].Name.c_str(),
                                           true)) {
        /* Match. */
        Result.push_back(m_PackFiles[i].Name);
      }
    }
  }

  /* Return file listing */
  return Result;
}

FileHandle *XMFS::openOFile(FileDataType i_fdt, const std::string &Path) {
  FileHandle *pfh = new FileHandle;

  /* Is it an absolute path? */
  if (isPathAbsolute(Path)) {
    /* Yup, not much to do here then */
    mkArborescence(Path);
    pfh->fp = fopen(Path.c_str(), "wb");
  } else {
    /* Nope, try the user dir */
    mkArborescence(getUserDir(i_fdt) + std::string("/") + Path);
    pfh->fp =
      fopen((getUserDir(i_fdt) + std::string("/") + Path).c_str(), "wb");
  }

  if (pfh->fp != NULL) {
    pfh->Type = FHT_STDIO;
    pfh->bWrite = true;
    pfh->Name = Path;
    return pfh;
  }
  delete pfh;
  return NULL;
}

FileHandle *XMFS::openIFile(FileDataType i_fdt,
                            std::string Path,
                            bool i_includeCurrentDir) {
  FileHandle *pfh = new FileHandle;

  /* Okay. Absolute path? */
  if (isPathAbsolute(Path)) {
    /* Yes ma'am */
    pfh->fp = fopen(Path.c_str(), "rb");
    pfh->Type = FHT_STDIO;
  } else {
    /* user dir ? */
    pfh->fp =
      fopen((getUserDir(i_fdt) + std::string("/") + Path).c_str(), "rb");

    /* data dir ? */
    if (i_fdt == FDT_DATA) {
      if (pfh->fp == NULL) {
        if (m_bGotSystemDataDir) {
          pfh->fp =
            fopen((m_SystemDataDir + std::string("/") + Path).c_str(), "rb");
        }

        /* current working dir ? */
        if (pfh->fp == NULL) {
          if (i_includeCurrentDir) {
            pfh->fp = fopen(Path.c_str(), "rb");
          }
        }
      }
    }

    /* got something ? */
    if (pfh->fp != NULL) {
      pfh->Type = FHT_STDIO;
    }
  }

  if (i_fdt == FDT_DATA) {
    if (pfh->fp == NULL) {
      int v_packsize = m_PackFiles.size();

      // remove all ./ of the Path
      std::string v_path = Path;
      while (v_path.substr(0, 2) == "./") {
        v_path = v_path.substr(2, v_path.length() - 2);
      }

      /* No luck so far, look in the data package */
      for (unsigned int i = 0; i < v_packsize; i++) {
        if (m_PackFiles[i].Name == v_path) {
          /* Found it, yeah. */
          pfh->fp = fopen(m_BinDataFile.c_str(), "rb");
          if (pfh->fp != NULL) {
            fseek(pfh->fp, m_PackFiles[i].nOffset, SEEK_SET);
            pfh->Type = FHT_PACKAGE;
            pfh->nSize = m_PackFiles[i].nSize;
            pfh->nOffset = ftell(pfh->fp);
            //            printf("OPENED '%s' IN PACKAGE!\n",Path.c_str());
          } else
            break;
        }
      }
    }
  }

  if (pfh->fp == NULL) {
    delete pfh;
    return NULL;
  }

  pfh->bRead = true;
  pfh->Name = Path;

  if (pfh->Type == FHT_STDIO) {
    fseek(pfh->fp, 0, SEEK_END);
    pfh->nSize = ftell(pfh->fp);
    fseek(pfh->fp, 0, SEEK_SET);
  }

  return pfh;
}

void XMFS::closeFile(FileHandle *pfh) {
  if (pfh->Type == FHT_STDIO) {
    fclose(pfh->fp);
  } else if (pfh->Type == FHT_PACKAGE) {
    fclose(pfh->fp);
  } else
    _ThrowFileError(pfh, "closeFile -> invalid type");
  delete pfh;
}

bool XMFS::readBuf(FileHandle *pfh, char *pcBuf, unsigned int nBufSize) {
  if (!pfh->bRead)
    _ThrowFileError(pfh, "readBuf -> write-only");
  if (pfh->Type == FHT_STDIO) {
    if (fread(pcBuf, 1, nBufSize, pfh->fp) != nBufSize)
      return false;
  } else if (pfh->Type == FHT_PACKAGE) {
    int nRem = pfh->nSize - (ftell(pfh->fp) - pfh->nOffset);
    //      printf("reading %d bytes. %d left  (%d %d
    //      %d).\n",nBufSize,nRem,pfh->nSize,ftell(pfh->fp),pfh->nOffset);
    if (nBufSize == 0)
      return true;
    if (nRem > 0) {
      if (fread(pcBuf, 1, nBufSize, pfh->fp) != nBufSize)
        return false;
    } else
      return false;
  } else
    _ThrowFileError(pfh, "readBuf -> invalid type");
  return true;
}

bool XMFS::writeBuf(FileHandle *pfh, char *pcBuf, unsigned int nBufSize) {
  if (!pfh->bWrite)
    _ThrowFileError(pfh, "writeBuf -> read-only");
  if (pfh->Type == FHT_STDIO) {
    if (fwrite(pcBuf, 1, nBufSize, pfh->fp) != nBufSize)
      return false;
  } else
    _ThrowFileError(pfh, "writeBuf -> invalid type");
  return true;
}

bool XMFS::setOffset(FileHandle *pfh, int nOffset) {
  if (pfh->Type == FHT_STDIO) {
    fseek(pfh->fp, nOffset, SEEK_SET);
  } else if (pfh->Type == FHT_PACKAGE) {
    fseek(pfh->fp, nOffset + pfh->nOffset, SEEK_SET);
  } else
    _ThrowFileError(pfh, "setOffset -> invalid type");
  return true; /* blahh, this is not right, but... */
}

bool XMFS::setEnd(FileHandle *pfh) {
  if (pfh->Type == FHT_STDIO) {
    fseek(pfh->fp, 0, SEEK_END);
  } else if (pfh->Type == FHT_PACKAGE) {
    fseek(pfh->fp, pfh->nSize + pfh->nOffset, SEEK_SET);
  } else
    _ThrowFileError(pfh, "setEnd -> invalid type");
  return true; /* ... */
}

int XMFS::getOffset(FileHandle *pfh) {
  int nOffset = 0;
  if (pfh->Type == FHT_STDIO) {
    nOffset = ftell(pfh->fp);
  } else if (pfh->Type == FHT_PACKAGE) {
    nOffset = ftell(pfh->fp) - pfh->nOffset;
  } else
    _ThrowFileError(pfh, "getOffset -> invalid type");
  return nOffset;
}

int XMFS::getLength(FileHandle *pfh) {
  return pfh->nSize;
}

bool XMFS::isEnd(FileHandle *pfh) {
  bool bEnd = true;
  if (pfh->Type == FHT_STDIO) {
    if (!feof(pfh->fp))
      bEnd = false;
  } else if (pfh->Type == FHT_PACKAGE) {
    int nRem = pfh->nSize - (ftell(pfh->fp) - pfh->nOffset);
    if (nRem > 0)
      bEnd = false;
  } else
    _ThrowFileError(pfh, "isEnd -> invalid type");
  return bEnd;
}

std::string XMFS::readFileToEnd(FileHandle *pfh) {
  std::string v_res;
  char v_buffer[16384 + 1];
  int v_remaining = getLength(pfh);
  int v_toread;

  v_res = "";

  while (v_remaining > 0) {
    v_toread = v_remaining > 16384 ? 16384 : v_remaining;
    if (readBuf(pfh, v_buffer, v_toread) == false) {
      _ThrowFileError(pfh, "readFileToEnd -> read failed");
    }
    v_buffer[v_toread] = '\0';
    v_res += v_buffer;
    v_remaining -= v_toread;
  }

  return v_res;
}

bool XMFS::readNextLine(FileHandle *pfh, std::string &Line) {
  int c;
  char b[2];
  Line = "";

  if (peekNextBufferedChar(pfh) == 0)
    return false;

  /* Read until the next new-line */
  while (1) {
    try {
      c = readBufferedChar(pfh);
      if (c == 0)
        break;

      if (c == '\r' || c == '\n') {
        /* New-line... -- if \r and the next is \n OR if \n and the next is \r,
           then skip it
           -- in this way we support all imaginable text file formats - almost
           ;) */
        if ((c == '\r' && peekNextBufferedChar(pfh) == '\n') ||
            (c == '\n' && peekNextBufferedChar(pfh) == '\r')) {
          readBufferedChar(pfh);
        }
        break;
      }

      /* Append char to line */
      b[0] = c;
      b[1] = '\0'; /* not very c++-ish, i know :( */
      Line.append(b);
    } catch (Exception e) {
      return false;
    }
  }

  return true;
}

int XMFS::readByte(FileHandle *pfh) {
  signed char v;
  if (!readBuf(pfh, (char *)&v, sizeof(v)))
    _ThrowFileError(pfh, "readByte -> failed");
  return v;
}

int XMFS::readShort(FileHandle *pfh) {
  short v;
  if (!readBuf(pfh, (char *)&v, sizeof(v)))
    _ThrowFileError(pfh, "readShort -> failed");
  return v;
}

int XMFS::readInt(FileHandle *pfh) {
  int32_t nv;
  if (!readBuf(pfh, (char *)&nv, sizeof(4)))
    _ThrowFileError(pfh, "readInt -> failed");
  return static_cast<int>(nv);
}

float XMFS::readFloat(FileHandle *pfh) {
  float v;
  if (!readBuf(pfh, (char *)&v, sizeof(v)))
    _ThrowFileError(pfh, "readFloat -> failed");
  return v;
}

double XMFS::readDouble(FileHandle *pfh) {
  double v;
  if (!readBuf(pfh, (char *)&v, sizeof(v)))
    _ThrowFileError(pfh, "readDouble -> failed");
  return v;
}

std::string XMFS::readString(FileHandle *pfh) {
  unsigned char v;
  v = readByte(pfh);
  char cBuf[256];
  if (!readBuf(pfh, (char *)cBuf, v))
    _ThrowFileError(pfh, "readString -> failed");
  cBuf[v] = '\0';
  return std::string(cBuf);
}

std::string XMFS::readLongString(FileHandle *pfh) {
  unsigned short v;
  v = readShort_LE(pfh);
  char cBuf[65536];
  if (!readBuf(pfh, (char *)cBuf, v))
    _ThrowFileError(pfh, "readLongString -> failed");
  cBuf[v] = '\0';
  return std::string(cBuf);
}

// We use one-byte bools
bool XMFS::readBool(FileHandle *pfh) {
  return static_cast<bool>(readByte(pfh));
}

void XMFS::writeByte(FileHandle *pfh, unsigned char v) {
  if (!writeBuf(pfh, (char *)&v, sizeof(v)))
    _ThrowFileError(pfh, "writeByte -> failed");
}

void XMFS::writeShort(FileHandle *pfh, short v) {
  if (!writeBuf(pfh, (char *)&v, sizeof(v)))
    _ThrowFileError(pfh, "writeShort -> failed");
}

void XMFS::writeInt(FileHandle *pfh, int v) {
  int32_t nv;
  nv = static_cast<int32_t>(v);
  if (!writeBuf(pfh, (char *)&nv, 4))
    _ThrowFileError(pfh, "writeInt -> failed");
}

void XMFS::writeFloat(FileHandle *pfh, float v) {
  if (!writeBuf(pfh, (char *)&v, sizeof(v)))
    _ThrowFileError(pfh, "writeFloat -> failed");
}

void XMFS::writeDouble(FileHandle *pfh, double v) {
  if (!writeBuf(pfh, (char *)&v, sizeof(v)))
    _ThrowFileError(pfh, "writeDouble -> failed");
}

void XMFS::writeString(FileHandle *pfh, const std::string &v) {
  writeByte(pfh, v.length());
  if (!writeBuf(pfh, (char *)v.c_str(), v.length()))
    _ThrowFileError(pfh, "writeString -> failed");
}

void XMFS::writeLongString(FileHandle *pfh, const std::string &v) {
  writeShort_LE(pfh, v.length());
  if (!writeBuf(pfh, (char *)v.c_str(), v.length()))
    _ThrowFileError(pfh, "writeLongString -> failed");
}

void XMFS::writeBool(FileHandle *pfh, bool v) {
  writeByte(pfh, static_cast<unsigned char>(v));
}

void XMFS::writeLine(FileHandle *pfh, const std::string &Line) {
  char cBuf[2048];
  snprintf(cBuf, 2048, "%s\n", Line.c_str());
  if (!writeBuf(pfh, cBuf, strlen(cBuf)))
    _ThrowFileError(pfh, "writeLine -> failed");
}

void XMFS::writeLineF(FileHandle *pfh, char *pcFmt, ...) {
  char cBuf[2048], cBuf2[2048 + 1];
  va_list List;
  va_start(List, pcFmt);
  vsnprintf(cBuf, sizeof(cBuf), pcFmt, List);
  va_end(List);
  snprintf(cBuf2, sizeof(cBuf2), "%s\n", cBuf);
  if (!writeBuf(pfh, cBuf2, strlen(cBuf2)))
    _ThrowFileError(pfh, "writeLineF -> failed");
}

/* For buffered reading: */
int XMFS::readBufferedChar(FileHandle *pfh) {
  /* Need to buffer? */
  if (pfh->nWrite == pfh->nRead)
    if (fillBuffer(pfh) == 0)
      return 0; /* end-of-file */

  /* Read buffer */
  int c = pfh->cBuffer[pfh->nRead];
  pfh->nRead++;
  if (pfh->nRead >= 256)
    pfh->nRead = 0;

  /* Return */
  return c;
}

int XMFS::peekNextBufferedChar(FileHandle *pfh) {
  /* Need to buffer? */
  if (pfh->nWrite == pfh->nRead)
    if (fillBuffer(pfh) == 0)
      return 0; /* end-of-file */

  /* Read buffer, but don't increment index */
  int c = pfh->cBuffer[pfh->nRead];

  /* Return */
  return c;
}

int XMFS::fillBuffer(FileHandle *pfh) {
  int nRemaining = getLength(pfh) - getOffset(pfh);
  int nBytes = nRemaining < 256 / 2 ? nRemaining : 256 / 2;
  int r = nBytes, t;

  /* Fill half of the buffer */
  while (r > 0) {
    t = (256 - pfh->nWrite) > r ? r : (256 - pfh->nWrite);
    if (!readBuf(pfh, &pfh->cBuffer[pfh->nWrite], t))
      _ThrowFileError(pfh, "fillBuffer -> read failed");
    pfh->nWrite += t;
    if (pfh->nWrite == 256)
      pfh->nWrite = 0;
    r -= t;
  }

  /* Return number of bytes read */
  return nBytes;
}

/*===========================================================================
  Extract directory name from path
  ===========================================================================*/
std::string XMFS::getFileDir(const std::string &Path) {
  int n = Path.find_last_of("/");
  if (n < 0)
    n = Path.find_last_of("\\");
  if (n < 0) {
    /* No directory info */
    return std::string(".");
  }
  std::string v_fileDir = Path.substr(0, n);
  if (v_fileDir == "") {
    v_fileDir = "/";
  }
  return v_fileDir;
}

/*===========================================================================
  Extract file name (optionally with extension) from path
  ===========================================================================*/
std::string XMFS::getFileBaseName(const std::string &Path, bool withExt) {
  int n = Path.find_last_of("/");
  if (n < 0)
    n = Path.find_last_of("\\");
  std::string FName;

  if (n >= 0) {
    FName = Path.substr(n + 1, Path.length() - n - 1);
  } else {
    FName = Path;
  }

  if (withExt)
    return FName;

  n = FName.find_last_of(".");
  if (n < 0)
    return FName;

  return FName.substr(0, n);
}

std::string XMFS::getFileExtension(const std::string &Path) {
  int n;
  n = Path.find_last_of(".");
  if (n < 0)
    return "";
  return Path.substr(n + 1, Path.length() - n - 1);
}

/*===========================================================================
  Find out how old this file thing is... (physical file system only)
  ===========================================================================*/
int XMFS::getFileTimeStamp(const std::string &Path) {
  struct stat S;

  if (stat(Path.c_str(), &S)) {
    return 0;
  }

  return (int)S.st_mtime; /* no we don't care about exact value */
}

/*===========================================================================
  Is that a dir or what? - and similar stuffin'
  ===========================================================================*/
bool XMFS::isDir(const std::string &Path) {
  struct stat S;

  if (stat(Path.c_str(), &S)) {
    return false;
  }

  if (S.st_mode & S_IFDIR)
    return true; /* it is a directory */

  return false; /* not a directory */
}

bool XMFS::isPathAbsolute(const std::string &Path) {
/* Windows? */
#ifdef WIN32
  /* Check for drive letter */
  if (Path.length() >= 3) {
    if (isalpha(Path.at(0)) && Path.at(1) == ':' &&
        (Path.at(2) == '\\' || Path.at(2) == '/'))
      return true;
  }
#endif

  /* Otherwise simply look for leading slash/backslash */
  if (Path.length() >= 1) {
    if (Path.at(0) == '\\' || Path.at(0) == '/')
      return true;
  }

  /* Not absolute */
  return false;
}

/*===========================================================================
  Delete file
  ===========================================================================*/
void XMFS::deleteFile(FileDataType i_fdt, const std::string &File) {
  std::string FullFile;

  if (getUserDir(i_fdt) == "") {
    throw Exception("Unable to delete the file " + File);
  }

  /* Absolute file path? */
  if (isPathAbsolute(File)) {
    /* Yeah, check if file is in user directory - otherwise we won't delete it
     */
    if (File.length() < getUserDir(i_fdt).length() ||
        File.substr(0, getUserDir(i_fdt).length()) != getUserDir(i_fdt)) {
      throw Exception("Unable to delete the file " + File);
    }

    FullFile = File;
  } else {
    /* We can only delete user files */
    FullFile = getUserDir(i_fdt) + std::string("/") + File;
  }

  if (remove(FullFile.c_str())) {
    throw Exception("Unable to delete the file " + File);
  }
}

/*===========================================================================
  Copy file
  ===========================================================================*/
bool XMFS::copyFile(FileDataType i_fdt,
                    const std::string &From,
                    const std::string &To,
                    std::string &To_really_done,
                    bool mkdirs) {
  /* All file copying must happen inside the user directory... */
  if (getUserDir(i_fdt) == "") {
    LogWarning("No user directory, can't copy file '%s' to '%s'",
               From.c_str(),
               To.c_str());
    return false;
  }

  std::string FullFrom = getUserDir(i_fdt) + std::string("/") + From;
  std::string FullTo = getUserDir(i_fdt) + std::string("/") + To;

  if (mkdirs) {
    mkArborescence(FullTo);
  }

  /* Does the destination file already exist? */
  FILE *fp = fopen(FullTo.c_str(), "rb");
  if (fp != NULL) {
    fclose(fp);

    /* Yup, modify its name - strip extension */
    int k = FullTo.find_last_of(".");
    std::string FullToExt, FullToBaseName;
    if (k >= 0) {
      FullToBaseName = FullTo.substr(0, k);
      FullToExt = FullTo.substr(k);
    } else {
      FullToBaseName = FullTo;
      FullToExt = "";
    }

    int i = 1;
    while (1) {
      char cTemp[1024];
      snprintf(
        cTemp, 1024, "%s (%d)%s", FullToBaseName.c_str(), i, FullToExt.c_str());

      /* What about this file then? */
      fp = fopen(cTemp, "rb");
      if (fp == NULL) {
        /* Good one */
        FullTo = cTemp;
        break;
      } else {
        fclose(fp);
        i++;
      }
      /* Next */
    }
  }

  /* Open files and copy */
  FILE *in = fopen(FullFrom.c_str(), "rb");
  if (in != NULL) {
    FILE *out = fopen(FullTo.c_str(), "wb");
    if (out != NULL) {
      To_really_done = FullTo;

      /* Get input file size */
      fseek(in, 0, SEEK_END);
      int nFileSize = ftell(in);
      fseek(in, 0, SEEK_SET);
      int nTotalWritten = 0;

      /* Good, start copying */
      while (!feof(in)) {
        char cBuf[8192];

        int nRead = fread(cBuf, 1, sizeof(cBuf), in);
        if (nRead > 0) {
          int nWritten = fwrite(cBuf, 1, nRead, out);
          if (nWritten > 0) {
            nTotalWritten += nWritten;
          }
        } else
          break;
      }

      /* Did we copy everything? */
      if (nTotalWritten != nFileSize) {
        /* Nope, clean up the mess and bail out */
        fclose(in);
        fclose(out);
        remove(FullTo.c_str());
        LogWarning("Failed to copy all of '%s' to '%s'",
                   FullFrom.c_str(),
                   FullTo.c_str());
        return false;
      }

      fclose(out);
    } else {
      fclose(in);
      LogWarning("Failed to open file for output: %s", FullTo.c_str());
      return false;
    }

    fclose(in);
  } else {
    LogWarning("Failed to open file for input: %s", FullFrom.c_str());
    return false;
  }

  /* OK */
  return true;
}

bool XMFS::moveFile(const std::string &From, const std::string &To) {
  return rename(From.c_str(), To.c_str()) == 0;
}

/*===========================================================================
  Initialize file system fun
  ===========================================================================*/
std::string XMFS::m_UserDataDir = "";
std::string XMFS::m_UserConfigDir = "";
std::string XMFS::m_UserCacheDir = "";
std::string XMFS::m_UserDataDirUTF8 = "";
std::string XMFS::m_SystemDataDir = ""; /* Globals */
std::string XMFS::m_SystemLocaleDir = "";
bool XMFS::m_bGotSystemDataDir;
std::string XMFS::m_BinDataFile = "<unavailable>";
std::string XMFS::m_binCheckSum = "";
std::vector<PackFile> XMFS::m_PackFiles;

const char *buildBinFilePath = "bin/xmoto.bin";

void XMFS::init(const std::string &AppDir,
                const std::string &i_binFile,
                bool i_graphics,
                const std::string &i_userCustomDirPath) {
  m_bGotSystemDataDir = false;
  m_UserDataDir = "";
  m_UserConfigDir = "";
  m_UserCacheDir = "";
  m_UserDataDirUTF8 = "";
  m_SystemDataDir = "";
  m_SystemLocaleDir = "";

  std::string v_mod_userCustomDirPath = i_userCustomDirPath;
  std::string v_mod_userCustomDirPathUtf8 =
    i_userCustomDirPath; // the argument must be utf8 -- so it can not work on
  // windows for example

  // make v_mod_userCustomDirPath absolute
  if (v_mod_userCustomDirPath != "") {
    if (isPathAbsolute(v_mod_userCustomDirPath) == false) {
      v_mod_userCustomDirPath = getPWDDir() + "/" + v_mod_userCustomDirPath;
      v_mod_userCustomDirPathUtf8 =
        getPWDDir(true) + "/" + v_mod_userCustomDirPathUtf8;
      if (isPathAbsolute(v_mod_userCustomDirPath) == false) {
        throw Exception("Custom directory must be absolute");
      }
    }
  }

#ifndef WIN32
  // xdg
  if ((m_xdgHd = (xdgHandle *)malloc(sizeof(xdgHandle))) == NULL) {
    throw Exception("xdgbasedir allocation failed");
  }
  if ((xdgInitHandle(m_xdgHd)) == 0) {
    throw Exception("xdgbasedir initialisation failed");
  }
#endif

#if defined(WIN32) /* Windoze... */
  char cModulePath[256];

  int i = GetModuleFileName(
    GetModuleHandle(NULL), cModulePath, sizeof(cModulePath) - 1);
  cModulePath[i] = '\0';
  for (i = strlen(cModulePath) - 1; i >= 0; i--) {
    if (cModulePath[i] == '/' || cModulePath[i] == '\\') {
      cModulePath[i] = '\0';
      break;
    }
  }

  /* Valid path? */
  if (isDir(cModulePath)) {
    /* Alright, use this dir */
    if (v_mod_userCustomDirPath != "") {
      m_UserDataDir = v_mod_userCustomDirPath;
      m_UserDataDirUTF8 = v_mod_userCustomDirPathUtf8;
      m_UserConfigDir = v_mod_userCustomDirPath;
      m_UserCacheDir = v_mod_userCustomDirPath;
    } else {
      /* not used even if it works because i'm not sure it returns utf-8 chars,
      required for sqlite_open
      //m_UserDataDir     = xdgDataHome(m_xdgHd)   + std::string("/") + AppDir;
      //m_UserConfigDir   = xdgConfigHome(m_xdgHd) + std::string("/") + AppDir;
      //m_UserCacheDir    = xdgCacheHome(m_xdgHd)  + std::string("/") + AppDir;
      */

      m_UserDataDir =
        win32_getHomeDir() + std::string("/.local/share/") + AppDir;
      m_UserConfigDir = win32_getHomeDir() + std::string("/.config/") + AppDir;
      m_UserCacheDir = win32_getHomeDir() + std::string("/.cache/") + AppDir;
      m_UserDataDirUTF8 =
        win32_getHomeDir(true) + std::string("/.local/share/") + AppDir;
    }

    m_SystemDataDir = cModulePath;
    m_bGotSystemDataDir = true;

    if (m_bGotSystemDataDir) {
      m_SystemLocaleDir = m_SystemDataDir + "/share/locale";
      if (!isDir(m_SystemLocaleDir)) {
        LogWarning("System locale directory doesn't exist!");
      }
    } else {
      LogWarning("No system data directory, can't load locales");
    }

  } else {
    throw Exception("invalid process directory");
  }
#elif defined(__MORPHOS__) || defined(__amigaos4__)
  if (v_mod_userCustomDirPath != "") {
    m_UserDataDir = v_mod_userCustomDirPath;
    m_UserDataDirUTF8 = v_mod_userCustomDirPathUtf8;
    m_UserConfigDir = v_mod_userCustomDirPath;
    m_UserCacheDir = v_mod_userCustomDirPath;
  } else {
    m_UserDataDir = "PROGDIR:userdata"; /*getenv("HOME");*/
    if (!isDir(m_UserDataDir))
      throw Exception("invalid user home directory");

    m_UserConfigDir = m_UserDataDir;
    m_UserCacheDir = m_UserDataDir;
    m_UserDataDirUTF8 = m_UserDataDir;
  }

  /* And the data dir? */
  m_SystemDataDir = std::string("PROGDIR:data");
  if (isDir(m_SystemDataDir)) {
    /* Got a system-wide installation to fall back to! */
    m_bGotSystemDataDir = true;
  }
#else /* Assume unix-like */
  /* Determine users home dir, so we can find out where to get/save user
     files */

  if (v_mod_userCustomDirPath != "") {
    m_UserDataDir = v_mod_userCustomDirPath;
    m_UserDataDirUTF8 = v_mod_userCustomDirPathUtf8;
    m_UserConfigDir = v_mod_userCustomDirPath;
    m_UserCacheDir = v_mod_userCustomDirPath;
  } else {
    m_UserDataDir = xdgDataHome(m_xdgHd) + std::string("/") + AppDir;
    m_UserConfigDir = xdgConfigHome(m_xdgHd) + std::string("/") + AppDir;
    m_UserCacheDir = xdgCacheHome(m_xdgHd) + std::string("/") + AppDir;
    m_UserDataDirUTF8 = m_UserDataDir;
  }

#if BUILD_MACOS_BUNDLE
  CFBundleRef bundle = CFBundleGetMainBundle();
  CFURLRef res = CFBundleCopyResourcesDirectoryURL(bundle);
  char path[PATH_MAX];
  if (!CFURLGetFileSystemRepresentation(res, TRUE, (uint8_t *)path, PATH_MAX)) {
    throw Exception("Failed to get bundle path");
  }
  CFRelease(res);

  char real[PATH_MAX];
  m_SystemDataDir = std::string(realpath(path, real));
  if (!isDir(m_SystemDataDir)) {
    throw Exception("Bundle path doesn't exist: " + m_SystemDataDir);
  } else {
    m_bGotSystemDataDir = true;
  }

  m_SystemLocaleDir = m_SystemDataDir + "/locale";
#else
  /* And the data dir? */
  for (char const *const *c_dir = xdgDataDirectories(m_xdgHd); *c_dir != NULL;
       c_dir++) {
    std::string dir = std::string(*c_dir);
    if (isDir(dir + "xmoto")) {
      /* Got a system-wide installation to fall back to! */
      m_SystemDataDir = dir;
      m_bGotSystemDataDir = true;
      break;
    }
  }

  /* Check if this is an AppImage */
  if (!m_bGotSystemDataDir) {
    std::string appDir = Environment::get_variable("APPDIR");

    if (appDir.length() > 0 && doesRealFileOrDirectoryExists(appDir)) {
      m_SystemDataDir = appDir + "/usr/share";
      m_bGotSystemDataDir = true;
    }
  }

  /* Try some default fallbacks */
  if (!m_bGotSystemDataDir) {
    const std::vector<std::string> dataDirs = {
      "/usr/share",
      "/usr/local/share",
    };

    for (auto &dir : dataDirs) {
      if (isDir(dir + "/xmoto")) {
        m_SystemDataDir = dir;
        m_bGotSystemDataDir = true;
        break;
      }
    }
  }

  /* On NixOS, the data is in ../share/xmoto relative to the X-Moto executable,
     which resides somewhere in the /nix/store directory. Gets the path to the
     X-Moto executable, removes the last two path components, and adds /share.
   */
  if (!m_bGotSystemDataDir) {
    /* TODO should be moved to a proper function/var. */
    std::string onNixOS = Environment::get_variable("NIX_PATH");
    if (onNixOS.length() > 0) {
      std::string path;
      path.resize(PATH_MAX + 1);
      ssize_t length = readlink("/proc/self/exe", path.data(), PATH_MAX);
      path[length] = '\0';
      if (path.length() > 0 && doesRealFileOrDirectoryExists(path)) {
        std::string share = path;
        share = getFileDir(getFileDir(share)) + "/share";
        if (isDir(share)) {
          m_SystemDataDir = share;
          m_bGotSystemDataDir = true;
        }
      }
    }
  }

  /* Default to /usr/share */
  if (!m_bGotSystemDataDir) {
    /* Fail here to make things easier to debug in the future. */
    throw Exception("Failed to find suitable folder containing system data for m_SystemDataDir");
  }

  m_SystemLocaleDir = m_SystemDataDir + "/locale";
  m_SystemDataDir = m_SystemDataDir + "/xmoto";
#endif
#endif /* Assume unix-like */

  bool v_requireMigration = false;

  /* If user-dir isn't there, try making it */
  if (isDir(m_UserDataDir) == false) {
    mkArborescenceDir(m_UserDataDir);
    /*
      if the user data directory, it's the first time it is used, try to migrate
      old data
    */
    v_requireMigration = true;
  }
  if (isDir(m_UserConfigDir) == false) {
    mkArborescenceDir(m_UserConfigDir);
  }
  if (isDir(m_UserCacheDir) == false) {
    mkArborescenceDir(m_UserCacheDir);
  }

  /* Info */
  if (m_bGotSystemDataDir) {
    m_BinDataFile = m_SystemDataDir + "/" + i_binFile;
  }

  /* Try loading from the current folder first. Useful for debugging. */
  const char *localBinFile = i_binFile.c_str();
  FILE *fp = fopen(localBinFile, "rb");
  if (fp == NULL) {
      /* No bin file in current directory. Fallback to bin/ subdirectory. */
      fp = fopen(buildBinFilePath, "rb");
  }

  if (fp == NULL) {
      /* No bin file under the bin directory either. Fallback to system directory. */
      fp = fopen(m_BinDataFile.c_str(), "rb");
  }

  /* Check if any of the potential bin package location was found. */
  if (fp == NULL) {
    throw Exception("Package " + i_binFile + " not found ! Searched: ./" + localBinFile + ", " + buildBinFilePath + " and " + m_BinDataFile.c_str() + ".");
  }

  char cBuf[256];
  char md5sum[256];
  fread(cBuf, 4, 1, fp);
  if (!strncmp(cBuf, "XBI3", 4)) {
    int nNameLen;
    int md5sumLen;
    PackFile v_packFile;
    size_t v_extension_place;
    std::string v_extension;

    /* get bin checksum of the package.list */
    md5sumLen = fgetc(fp);
    fread(md5sum, md5sumLen, 1, fp);
    md5sum[md5sumLen] = '\0';
    m_binCheckSum = md5sum;

    while ((nNameLen = fgetc(fp)) >= 0) {
      //          printf("%d  \n",nNameLen);
      /* Read file name */
      fread(cBuf, nNameLen, 1, fp);
      cBuf[nNameLen] = '\0';

      md5sumLen = fgetc(fp);
      fread(md5sum, md5sumLen, 1, fp);
      md5sum[md5sumLen] = '\0';
      //          printf("      %s\n",cBuf);

      /* Read file size */
      int nSize;

      /* Patch by Michel Daenzer (applied 2005-10-25) */
      unsigned char nSizeBuf[4];
      fread(nSizeBuf, 4, 1, fp);
      nSize =
        nSizeBuf[0] | nSizeBuf[1] << 8 | nSizeBuf[2] << 16 | nSizeBuf[3] << 24;

      v_packFile.Name = cBuf;

      v_extension = "";
      v_extension_place = v_packFile.Name.rfind(".");
      if (v_extension_place != std::string::npos) {
        v_extension = v_packFile.Name.substr(v_extension_place + 1);
      }

      // don't add unuseful graphics files
      if (i_graphics || (v_extension != "png" && // picture
                         v_extension != "jpg" && // picture
                         v_extension != "frag" && // graphical card files
                         v_extension != "vert" && // graphical card files
                         v_extension != "ogg" && // music
                         v_extension != "wav" // sound
                         )) {
        v_packFile.md5sum = md5sum;
        v_packFile.nOffset = ftell(fp);
        v_packFile.nSize = nSize;
        m_PackFiles.push_back(v_packFile);
      }

      fseek(fp, nSize, SEEK_CUR);
    }
  } else {
    throw Exception("Invalid binary data package format");
  }
  fclose(fp);

  m_isInitialized = true;

  /* migrate old files if required */
  if (v_requireMigration) {
    /*
      migrating from $HOME/.xmoto to xdg directories if necessary
    */
    migrateFSToXdgBaseDirIfRequired(AppDir);
  }

  /* create base directories if missing -- do it after migration of old files
   * while on windows, you cannot do the move without removing the directory */
  if (isDir(getUserReplaysDir()) == false) {
    mkArborescenceDir(getUserReplaysDir());
  }
  if (isDir(m_UserCacheDir + std::string("/LCache")) == false) {
    mkArborescenceDir(m_UserCacheDir + std::string("/LCache"));
  }
  if (isDir(getUserLevelsDir()) == false) {
    mkArborescenceDir(getUserLevelsDir());
  }
}

void XMFS::uninit() {
#ifndef WIN32
  if (m_xdgHd != NULL) {
    xdgWipeHandle(m_xdgHd);
    free(m_xdgHd);
  }
#endif
  m_isInitialized = false;
}

bool XMFS::isInitialized() {
  return m_isInitialized;
}

/*===========================================================================
  Hmm, for some reason I have to do this. Don't ask me why.
  ===========================================================================*/
int XMFS::mkDir(const char *pcPath) {
#if defined(WIN32)
  return _mkdir(pcPath);
#else
  return mkdir(pcPath, S_IRUSR | S_IWUSR | S_IRWXU);
#endif
}

std::string XMFS::binCheckSum() {
  return XMFS::m_binCheckSum;
}

/*===========================================================================
  Endian-safe I/O helpers
  ===========================================================================*/
void XMFS::writeShort_LE(FileHandle *pfh, short v) {
  writeShort(pfh, SwapEndian::LittleShort(v));
}

void XMFS::writeInt_LE(FileHandle *pfh, int v) {
  writeInt(pfh, SwapEndian::LittleLong(v));
}

void XMFS::writeFloat_LE(FileHandle *pfh, float v) {
  writeFloat(pfh, SwapEndian::LittleFloat(v));
}

int XMFS::readShort_LE(FileHandle *pfh) {
  short v = readShort(pfh);
  return SwapEndian::LittleShort(v);
}

int XMFS::readInt_LE(FileHandle *pfh) {
  int v = readInt(pfh);
  return SwapEndian::LittleLong(v);
}

float XMFS::readFloat_LE(FileHandle *pfh) {
  float v = readFloat(pfh);
  return SwapEndian::LittleFloat(v);
}

int XMFS::readShort_MaybeLE(FileHandle *pfh, bool big) {
  short v = readShort(pfh);
  return big ? SwapEndian::BigShort(v) : SwapEndian::LittleShort(v);
}

int XMFS::readInt_MaybeLE(FileHandle *pfh, bool big) {
  int v = readInt(pfh);
  return big ? SwapEndian::BigLong(v) : SwapEndian::LittleLong(v);
}

float XMFS::readFloat_MaybeLE(FileHandle *pfh, bool big) {
  float v = readFloat(pfh);
  return big ? SwapEndian::BigFloat(v) : SwapEndian::LittleFloat(v);
}

bool XMFS::isFileInDir(const std::string &p_dirpath,
                       const std::string &p_filepath) {
  std::string v_fileDir;

  v_fileDir = getFileDir(p_filepath);

  if (p_dirpath.length() > v_fileDir.length()) {
    return false;
  }

  return v_fileDir.substr(0, p_dirpath.length()) == p_dirpath;
}

bool XMFS::isInUserDir(FileDataType i_fdt, const std::string &p_filepath) {
  return isFileInDir(getUserDir(i_fdt), p_filepath);
}

bool XMFS::doesRealFileOrDirectoryExists(const std::string &p_path) {
  struct stat S;
  return stat(p_path.c_str(), &S) == 0;
}

bool XMFS::isFileReadable(FileDataType i_fdt, const std::string &p_filename) {
  FileHandle *fh = openIFile(i_fdt, p_filename);
  if (fh == NULL) {
    return false;
  }
  closeFile(fh);
  return true;
}

bool XMFS::fileExists(FileDataType i_fdt, const std::string &p_filename) {
  return isFileReadable(i_fdt, p_filename);
}

void XMFS::mkArborescence(const std::string &v_filepath) {
  std::string v_parentDir = getFileDir(v_filepath);

  if (doesRealFileOrDirectoryExists(v_parentDir)) {
    return;
  }

  mkArborescence(v_parentDir);
  if (XMFS::mkDir(v_parentDir.c_str()) != 0) {
    throw Exception("Can't create directory " + v_parentDir);
  }
}

void XMFS::mkArborescenceDir(const std::string &v_dirpath) {
  mkArborescence(v_dirpath + "/file.tmp");
}

std::string XMFS::md5sum(FileDataType i_fdt, std::string i_filePath) {
  i_filePath = FullPath(i_fdt, i_filePath);

  /* is it a file from the pack or a real file ? */
  if (XMFS::isFileReal(i_filePath)) {
    return md5file(i_filePath);
  }

  if (i_fdt == FDT_DATA) {
    /* package */
    for (unsigned int i = 0; i < m_PackFiles.size(); i++) {
      if (m_PackFiles[i].Name == i_filePath ||
          (std::string("./") + m_PackFiles[i].Name) == i_filePath) {
        /* Found it, yeah. */
        // printf("md5sum(%s) = %s\n", m_PackFiles[i].Name.c_str(),
        // m_PackFiles[i].md5sum.c_str());
        return m_PackFiles[i].md5sum;
      }
    }
  }

  return "";
}

bool XMFS::isFileReal(const std::string &i_filePath) {
  FILE *fp;

  fp = fopen(i_filePath.c_str(), "rb");
  if (fp == NULL) {
    return false;
  }
  fclose(fp);

  return true;
}

std::string XMFS::FullPath(FileDataType i_fdt,
                           const std::string &i_relative_path) {
  if (fileExists(i_fdt,
                 getUserDir(i_fdt) + std::string("/") + i_relative_path)) {
    return getUserDir(i_fdt) + std::string("/") + i_relative_path;
  }

  if (i_fdt == FDT_DATA) {
    if (fileExists(i_fdt,
                   m_SystemDataDir + std::string("/") + i_relative_path)) {
      return m_SystemDataDir + std::string("/") + i_relative_path;
    }
  }

  return i_relative_path;
}

bool XMFS::areSamePath(const std::string &i_path1, const std::string &i_path2) {
#ifdef WIN32
  return txtToLower(i_path1) == txtToLower(i_path2);
#else
  return i_path1 == i_path2;
#endif
}

void XMFS::migrateFSToXdgBaseDirFile(const std::string &i_src,
                                     const std::string &i_dest) {
  if (doesRealFileOrDirectoryExists(i_src) == false) {
    return;
  }

  // write in log information about the migration
  printf("Migrate file '%s' to '%s'\n", i_src.c_str(), i_dest.c_str());

  // create the directory if required
  mkArborescence(i_dest);

  // move the file
  if (moveFile(i_src, i_dest) == false) {
    throw Exception("Unable to migrate the file " + i_src + " to " + i_dest);
  }
}

void XMFS::migrateFSToXdgBaseDirIfRequired(const std::string &AppDir) {
  std::string v_oldUserDir;

#ifndef WIN32
  v_oldUserDir = getenv("HOME") + std::string("/.") + AppDir;
#else
  v_oldUserDir = win32_getHomeDir() + std::string("/.") + AppDir;
#endif

  if (isDir(v_oldUserDir) == false) {
    // no need to do the conversion
    return;
  }

  try {
    migrateFSToXdgBaseDirFile(v_oldUserDir + "/xm.db",
                              XMFS::getUserDir(FDT_DATA) + "/" + DATABASE_FILE);
    migrateFSToXdgBaseDirFile(v_oldUserDir + "/config.dat",
                              XMFS::getUserDir(FDT_CONFIG) + "/" +
                                std::string(XM_CONFIGFILE));
    migrateFSToXdgBaseDirFile(v_oldUserDir + "/players.bin",
                              XMFS::getUserDir(FDT_DATA) + "/players.bin");
    migrateFSToXdgBaseDirFile(v_oldUserDir + "/favoriteLevels.xml",
                              XMFS::getUserDir(FDT_DATA) +
                                "/favoriteLevels.xml");
    migrateFSToXdgBaseDirFile(v_oldUserDir + "/stats.xml",
                              XMFS::getUserDir(FDT_DATA) + "/stats.xml");
    migrateFSToXdgBaseDirFile(v_oldUserDir + "/LCache",
                              XMFS::getUserDir(FDT_CACHE) + "/LCache");
    migrateFSToXdgBaseDirFile(v_oldUserDir + "/Levels",
                              XMFS::getUserDir(FDT_DATA) + "/Levels");
    migrateFSToXdgBaseDirFile(v_oldUserDir + "/Replays",
                              XMFS::getUserDir(FDT_DATA) + "/Replays");
    migrateFSToXdgBaseDirFile(v_oldUserDir + "/Screenshots",
                              XMFS::getUserDir(FDT_DATA) + "/Screenshots");
    migrateFSToXdgBaseDirFile(v_oldUserDir + "/Textures",
                              XMFS::getUserDir(FDT_DATA) + "/Textures");
    migrateFSToXdgBaseDirFile(v_oldUserDir + "/Themes",
                              XMFS::getUserDir(FDT_DATA) + "/Themes");
    migrateFSToXdgBaseDirFile(v_oldUserDir + "/Physics",
                              XMFS::getUserDir(FDT_DATA) + "/Physics");
    migrateFSToXdgBaseDirFile(v_oldUserDir + "/Shaders",
                              XMFS::getUserDir(FDT_DATA) + "/Shaders");

  } catch (Exception &e) {
    throw Exception("Unable move files for migration : " + e.getMsg());
  }
}

void XMFS::deleteFile(const std::string &i_filepath) {
  if (remove(i_filepath.c_str()) != 0) {
    throw Exception("cannot remove file " + i_filepath);
  }
}
