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
 *  Data access.
 */

#ifdef WIN32
  #include <io.h>
  #include <direct.h>
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <dirent.h>
#endif
#include <sys/stat.h>

#include "helpers/VExcept.h"
#include "VApp.h"
#include "VFileIO.h"
#include "helpers/SwapEndian.h"

namespace vapp {

  /*===========================================================================
  Helper functions
  ===========================================================================*/
  /* old C rip... :) I should fix this */
  #define mbool bool
  #ifndef FALSE
  #define FALSE 0
  #endif
  #ifndef TRUE
  #define TRUE 1
  #endif
  #if !defined(_MSC_VER)
    void strlwr(char *pc) {
      for(unsigned int i=0; i<strlen(pc); i++) pc[i] = tolower(pc[i]);
    }
  #endif
  mbool str_match_wildcard(char *pcMWildcard,char *pcMString,mbool CaseSensitive) {
    int nPos=0;
    mbool PrevIsWildcard=FALSE;
    char c1[256],c2[256];
    char *pcWildcard,*pcString;
    
    if(CaseSensitive) {
      pcWildcard=pcMWildcard;
      pcString=pcMString;
    } 
    else {
      strncpy(c1,pcMWildcard,sizeof(c1));
      strncpy(c2,pcMString,sizeof(c2));
      strlwr(c1); strlwr(c2);
      pcWildcard=c1;
      pcString=c2;
    }
    
    if(pcWildcard[0]=='\0') return TRUE;
    
    for(unsigned i=0; i<strlen(pcWildcard); i++) {
      /* If end of string is reached, we have a match */
      if(pcString[nPos] == '\0' && pcWildcard[i] == '\0')
        return TRUE;
        
      /* Wildcard? */
      if(pcWildcard[i] == '?' || pcWildcard[i] == '*') 
        PrevIsWildcard=TRUE;
      else {
        /* Nope. What does we accept from the string? */
        if(PrevIsWildcard) {
          /* Read string until we get the right character */
          while(1) {
            if(pcString[nPos] == '\0') 
              return FALSE;
              
            /* Got the char? */
            if(pcString[nPos] == pcWildcard[i])
              break;
              
            nPos++;
          }
          i--;
        
          /* Clear wildcard flag */
          PrevIsWildcard=FALSE;
        }
        else {
          /* The letter in the string MUST equal the letter in the wildcard */
          if(pcWildcard[i] != pcString[nPos])
            return FALSE;
              
          nPos++; /* Next */
        }
      }
    }
    
    /* Result of random debugging :) */
    if(PrevIsWildcard || pcString[nPos]=='\0') return TRUE;
    
    /* Not a match */
    return FALSE;
  }

  void FS::_ThrowFileError(FileHandle *pfh,std::string Description) {
    char cBuf[512];
    sprintf(cBuf,"%s (%s%s): %s",pfh->Name.c_str(),pfh->bRead?"I":"",pfh->bWrite?"O":"",Description.c_str());
    throw Exception(cBuf);
  }

  /*===========================================================================
  Code soup :)  -- this is largely copy/paste material from an earlier project
  
  FutureRasmus: well, this is probably the crappiest code in the world. Period.
  ===========================================================================*/
  void FS::_FindFilesRecursive(const std::string &DirX,const std::string &Wildcard,std::vector<std::string> &List) {
    /* Windows? */
    #ifdef WIN32
      long fh;
      struct _finddata_t fd;
      
      std::string Dir = DirX;
      if(!DirX.empty() && DirX[DirX.length()-1] == '/') {
        Dir = DirX.substr(0,DirX.length()-1);
      }
      
      if((fh = _findfirst((Dir + std::string("/") + Wildcard).c_str(),&fd)) != -1L) {
        do {
          if(strcmp(fd.name,".") && strcmp(fd.name,"..")) {
            std::string F = Dir + std::string("/") + std::string(fd.name);
            bool bFound = false;
            for(int k = 0;k<List.size();k++) {
              if(getFileBaseName(List[k]) == getFileBaseName(F)) {  
                bFound = true;
                break;
              }
            }
            if(!bFound)
              List.push_back(F);
          }
        } while(_findnext(fh,&fd)==0);
        _findclose(fh);
      }
      
      /* Now, recurse into sub-dirs */
      if((fh = _findfirst( (Dir + std::string("/*")).c_str(),&fd)) != -1L) {
        do {
          if(strcmp(fd.name,".") && strcmp(fd.name,"..")) {
            std::string F = Dir + std::string("/") + std::string(fd.name);
            if(isDir(F)) {
              /* Recurse! */
              _FindFilesRecursive(F,Wildcard,List);
            }           
          }
        } while(_findnext(fh,&fd)==0);
        _findclose(fh);
      }      
    #else
      std::string Dir = DirX;
    
      struct dirent *dp;    
      DIR *dirp = opendir(Dir.c_str());
      while(dirp) {
        if((dp = readdir(dirp)) != NULL) {
          std::string F = Dir + std::string(dp->d_name);
          if(isDir(F)) {
            /* Recurse... */
            if(strcmp(dp->d_name,".") && strcmp(dp->d_name,"..")) {           
              _FindFilesRecursive(F + std::string("/"),Wildcard,List);
            }
          }
          else {          
            if(str_match_wildcard((char *)Wildcard.c_str(),(char *)dp->d_name,true)) {
              /* Match! */
              if(strcmp(dp->d_name,".") && strcmp(dp->d_name,"..")) {
                bool bFound = false;
                for(unsigned int k = 0;k<List.size();k++) {
                  if(FS::getFileBaseName(List[k]) == FS::getFileBaseName(F)) {  
                    bFound = true;
                    break;
                  }
                }
                if(!bFound)
                  List.push_back(F);
              }             
            }
          }
        }
        else {
          closedir(dirp);
          break;        
        }
      }       
    #endif    
  }

  std::vector<std::string> FS::findPhysFiles(std::string Files,bool bRecurse) {
    std::vector<std::string> Result;
    std::string Wildcard;
    std::string DataDirToSearch,AltDirToSearch,UDirToSearch = "";
    
    /* First seperate directory from wildcard */
    int n = Files.find_last_of('/');
    if(n<0) {
      /* No directory specified */
      DataDirToSearch = m_DataDir;
      AltDirToSearch = "";
      Wildcard = Files;
    }
    else {
      /* Seperate dir and wildcard */
      DataDirToSearch = m_DataDir + std::string("/") + Files.substr(0,n+1);
      UDirToSearch = m_UserDir + std::string("/") + Files.substr(0,n+1);
      AltDirToSearch = Files.substr(0,n+1);
      Wildcard = Files.substr(n+1);
    }    

		/* 1. find in user dir */
		/* 2. find in data dir */
		/* 3. find in package  */

    /* Windows? */
#ifdef WIN32
		if(bRecurse) {
			_FindFilesRecursive(DataDirToSearch,Wildcard,Result);
		} else {
			long fh;
			struct _finddata_t fd;
			
			if((fh = _findfirst((DataDirToSearch + Wildcard).c_str(),&fd)) != -1L) {
				do {
					if(strcmp(fd.name,".") && strcmp(fd.name,"..")) {
						std::string F = DataDirToSearch + std::string(fd.name);
						bool bFound = false;
						for(int k = 0;k<Result.size();k++) {
							if(FS::getFileBaseName(Result[k]) == FS::getFileBaseName(F)) {  
								bFound = true;
								break;
							}
						}
						if(!bFound)
							Result.push_back(F);
					}
				} while(_findnext(fh,&fd)==0);
				_findclose(fh);
			}
		}
#else /* Assume linux, unix... yalla yalla... */
		if(UDirToSearch != "") {
			if(bRecurse) {
				_FindFilesRecursive(UDirToSearch,Wildcard,Result);        
			} else {
				struct dirent *dp;    
				DIR *dirp = opendir(UDirToSearch.c_str());
				while(dirp) {
					if((dp = readdir(dirp)) != NULL) {
						if(str_match_wildcard((char *)Wildcard.c_str(),(char *)dp->d_name,true)) {
							/* Match! */
							if(strcmp(dp->d_name,".") && strcmp(dp->d_name,"..")) {
								std::string F = UDirToSearch + std::string(dp->d_name);
								bool bFound = false;
								for(unsigned int k = 0;k<Result.size();k++) {
									if(FS::getFileBaseName(Result[k]) == FS::getFileBaseName(F)) {  
										bFound = true;
										break;
									}
								}
								if(!bFound)
									Result.push_back(F);
							}
						}
					} else {
						closedir(dirp);
						break;        
					}
				}
			}
		}

		if(AltDirToSearch != UDirToSearch) {
			if(bRecurse) {
				_FindFilesRecursive(AltDirToSearch,Wildcard,Result);
			} else {
				struct dirent *dp;    
				DIR *dirp = opendir(AltDirToSearch.c_str());
				while(dirp) {
					if((dp = readdir(dirp)) != NULL) {
						if(str_match_wildcard((char *)Wildcard.c_str(),(char *)dp->d_name,true)) {
							/* Match! */
							if(strcmp(dp->d_name,".") && strcmp(dp->d_name,"..")) {
								std::string F = AltDirToSearch + std::string(dp->d_name);
								bool bFound = false;
								for(unsigned int k = 0;k<Result.size();k++) {
									if(FS::getFileBaseName(Result[k]) == FS::getFileBaseName(F)) {  
										bFound = true;
										break;
									}
								}
								if(!bFound)
									Result.push_back(F);
							}
						}
					} else {
						closedir(dirp);
						break;        
					}
				}
			}
		}

		if(DataDirToSearch != AltDirToSearch && DataDirToSearch != UDirToSearch) {
			if(bRecurse) {
				_FindFilesRecursive(DataDirToSearch,Wildcard,Result);
			} else {
				/* Search directories */
				struct dirent *dp;    
        DIR *dirp = opendir(DataDirToSearch.c_str());
        while(dirp) {
          if((dp = readdir(dirp)) != NULL) {
            if(str_match_wildcard((char *)Wildcard.c_str(),(char *)dp->d_name,true)) {
              /* Match! */
              if(strcmp(dp->d_name,".") && strcmp(dp->d_name,"..")) {
                std::string F = DataDirToSearch + std::string(dp->d_name);
                bool bFound = false;
                for(unsigned int k = 0;k<Result.size();k++) {
                  if(FS::getFileBaseName(Result[k]) == FS::getFileBaseName(F)) {  
                    bFound = true;
                    break;
                  }
                }
                if(!bFound)
                  Result.push_back(F);
              }             
            }
          } else {
            closedir(dirp);
            break;        
          }
        }
      }
		}
    #endif
		
    /* Look in package */
    for(int i=0;i<m_nNumPackFiles;i++) {
      /* Make sure about the directory... */
      int k = Files.find_last_of('/');
      std::string Ds1 = Files.substr(0,k+1);
      k = m_PackFiles[i].Name.find_last_of('/');
      std::string Ds2 = m_PackFiles[i].Name.substr(0,k+1);
      if(Ds1.substr(0,2) == "./") Ds1.erase(Ds1.begin(),Ds1.begin()+2);

      if(Ds1 == Ds2 && str_match_wildcard((char *)Files.c_str(),(char *)m_PackFiles[i].Name.c_str(),true)) {
        /* Match. */
        Result.push_back(m_PackFiles[i].Name);
      }
    }
    
		//	for(int i=0;i<Result.size();i++) printf("%s\n",Result[i].c_str());

    /* Return file listing */
    return Result;
  }
  
  FileHandle *FS::openOFile(std::string Path) {
    FileHandle *pfh = new FileHandle;
    
    /* Is it an absolute path? */
    if(isPathAbsolute(Path)) {    
      /* Yup, not much to do here then */
      pfh->fp = fopen(Path.c_str(),"wb");
    }    
    else {
      /* Nope, try the user dir. We are not going to create files relative
         to the working-dir, that would be f*cked :) */
      pfh->fp = fopen((m_UserDir + std::string("/") + Path).c_str(),"wb");
    }
    
    if(pfh->fp != NULL) { 
      pfh->Type = FHT_STDIO;
      pfh->bWrite = true;
      pfh->Name = Path;      
      return pfh;
    }
    delete pfh;
    return NULL;
  }
  
  FileHandle *FS::openIFile(std::string Path) {
    FileHandle *pfh = new FileHandle;
    
    /* Okay. Absolute path? */
    if(isPathAbsolute(Path)) {      
      /* Yes ma'am */
      pfh->fp = fopen(Path.c_str(),"rb");      
      pfh->Type = FHT_STDIO;
    }
    else {
      /* Try current working dir */
      //pfh->fp = fopen(Path.c_str(),"rb");
      //if(pfh->fp == NULL) {
			/* No luck. Try the user-dir then */
			pfh->fp = fopen((m_UserDir + std::string("/") + Path).c_str(),"rb");        
			if(pfh->fp == NULL && m_bGotDataDir) {
				/* Not there either, the data-dir is our last chance */
				pfh->fp = fopen((m_DataDir + std::string("/") + Path).c_str(),"rb");        
			}
			//}
      pfh->Type = FHT_STDIO;
    }
    
    if(pfh->fp == NULL) {
      /* No luck so far, look in the data package */
      for(int i=0;i<m_nNumPackFiles;i++) {              
        if(m_PackFiles[i].Name == Path ||
          (std::string("./") + m_PackFiles[i].Name) == Path) {
          /* Found it, yeah. */
          pfh->fp = fopen(m_BinDataFile.c_str(),"rb");
          if(pfh->fp != NULL) {
            fseek(pfh->fp,m_PackFiles[i].nOffset,SEEK_SET);    
            pfh->Type = FHT_PACKAGE;
            pfh->nSize = m_PackFiles[i].nSize;
            pfh->nOffset = ftell(pfh->fp);
//            printf("OPENED '%s' IN PACKAGE!\n",Path.c_str());
          }          
          else break;
        }
      }      
    }
    
    if(pfh->fp != NULL) { 
      pfh->bRead = true;
      pfh->Name = Path;
      
      if(pfh->Type == FHT_STDIO) {
        fseek(pfh->fp,0,SEEK_END);
        pfh->nSize = ftell(pfh->fp);
        fseek(pfh->fp,0,SEEK_SET);        
      }
      return pfh;
    }
    
    delete pfh;
    return NULL;
  }
    
  void FS::closeFile(FileHandle *pfh) {
    if(pfh->Type == FHT_STDIO) {
      fclose(pfh->fp);
    }
    else if(pfh->Type == FHT_PACKAGE) {
      fclose(pfh->fp);
    }
    else _ThrowFileError(pfh,"closeFile -> invalid type");
    delete pfh;
  } 

  bool FS::readBuf(FileHandle *pfh,char *pcBuf, unsigned int nBufSize) {
    if(!pfh->bRead) _ThrowFileError(pfh,"readBuf -> write-only");  
    if(pfh->Type == FHT_STDIO) {
      if(fread(pcBuf,1,nBufSize,pfh->fp) != nBufSize) return false;
    }
    else if(pfh->Type == FHT_PACKAGE) {
      int nRem = pfh->nSize - (ftell(pfh->fp) - pfh->nOffset);
//      printf("reading %d bytes. %d left  (%d %d %d).\n",nBufSize,nRem,pfh->nSize,ftell(pfh->fp),pfh->nOffset);
      if(nBufSize == 0) return true;
      if(nRem > 0) {
        if(fread(pcBuf,1,nBufSize,pfh->fp) != nBufSize) return false;
      }
      else return false;
    }
    else _ThrowFileError(pfh,"readBuf -> invalid type");
    return true;
  } 
  
  bool FS::writeBuf(FileHandle *pfh, char *pcBuf, unsigned int nBufSize) {
    if(!pfh->bWrite) _ThrowFileError(pfh,"writeBuf -> read-only");  
    if(pfh->Type == FHT_STDIO) {
      if(fwrite(pcBuf,1,nBufSize,pfh->fp) != nBufSize) return false;
    }
    else _ThrowFileError(pfh,"writeBuf -> invalid type");
    return true;
  } 
  
  bool FS::setOffset(FileHandle *pfh,int nOffset) {
    if(pfh->Type == FHT_STDIO) {
      fseek(pfh->fp,nOffset,SEEK_SET);
    }
    else if(pfh->Type == FHT_PACKAGE) {
      fseek(pfh->fp,nOffset + pfh->nOffset,SEEK_SET);
    }
    else _ThrowFileError(pfh,"setOffset -> invalid type");
    return true; /* blahh, this is not right, but... */
  } 
  
  bool FS::setEnd(FileHandle *pfh) {
    if(pfh->Type == FHT_STDIO) {
      fseek(pfh->fp,0,SEEK_END);
    }
    else if(pfh->Type == FHT_PACKAGE) {
      fseek(pfh->fp,pfh->nSize + pfh->nOffset,SEEK_SET);
    }
    else _ThrowFileError(pfh,"setEnd -> invalid type");
    return true; /* ... */
  } 
  
  int FS::getOffset(FileHandle *pfh) {
    int nOffset=0;
    if(pfh->Type == FHT_STDIO) {
      nOffset = ftell(pfh->fp);
    }
    else if(pfh->Type == FHT_PACKAGE) {
      nOffset = ftell(pfh->fp) - pfh->nOffset;
    }
    else _ThrowFileError(pfh,"getOffset -> invalid type");
    return nOffset;
  } 
  
  int FS::getLength(FileHandle *pfh) {
    return pfh->nSize;
  } 
  
  bool FS::isEnd(FileHandle *pfh) {
    bool bEnd = true;
    if(pfh->Type == FHT_STDIO) {
      if(!feof(pfh->fp)) bEnd = false;
    }
    else if(pfh->Type == FHT_PACKAGE) {
      int nRem = pfh->nSize - (ftell(pfh->fp) - pfh->nOffset);
      if(nRem > 0) bEnd = false;
    }
    else _ThrowFileError(pfh,"isEnd -> invalid type");
    return bEnd;
  } 
    
  bool FS::readNextLine(FileHandle *pfh,std::string &Line) {
    int c;
    char b[2];
    Line = "";
    
    if(peekNextBufferedChar(pfh)==0) return false;
    
    /* Read until the next new-line */
    while(1) {
      try {
        c=readBufferedChar(pfh);
        if(c==0) break;
        
        if(c=='\r' || c=='\n') {
          /* New-line... -- if \r and the next is \n OR if \n and the next is \r, then skip it
            -- in this way we support all imaginable text file formats - almost ;) */
          if((c=='\r' && peekNextBufferedChar(pfh)=='\n') ||
            (c=='\n' && peekNextBufferedChar(pfh)=='\r')) {
            readBufferedChar(pfh);
          }        
          break;
        }
        
        /* Append char to line */
        b[0] = c; b[1] = '\0'; /* not very c++-ish, i know :( */
        Line.append(b);
      }
      catch(Exception e) {
        return false;
      }
    }
    
    return true;
  }   
  
  int FS::readByte(FileHandle *pfh) {
    signed char v;
    if(!readBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"readByte -> failed");
    return v;
  } 
  
  int FS::readShort(FileHandle *pfh) {
    short v;
    if(!readBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"readShort -> failed");
    return v;
  } 
  
  int FS::readInt(FileHandle *pfh) {
    int32_t nv;
    if(!readBuf(pfh,(char *)&nv,sizeof(4))) _ThrowFileError(pfh,"readInt -> failed");
    return static_cast<int>(nv);
  } 
  
  float FS::readFloat(FileHandle *pfh) {
    float v;
    if(!readBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"readFloat -> failed");
    return v;
  } 
  
  double FS::readDouble(FileHandle *pfh) {
    double v;
    if(!readBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"readDouble -> failed");
    return v;
  } 
  
  std::string FS::readString(FileHandle *pfh) {
    unsigned char v;
    v=readByte(pfh);
    char cBuf[256];  
    if(!readBuf(pfh,(char *)cBuf,v)) _ThrowFileError(pfh,"readString -> failed");
    cBuf[v] = '\0';
    return std::string(cBuf);  
  }  
  
  std::string FS::readLongString(FileHandle *pfh) {
    unsigned short v;
    v=readShort_LE(pfh);
    char cBuf[65536];  
    if(!readBuf(pfh,(char *)cBuf,v)) _ThrowFileError(pfh,"readLongString -> failed");
    cBuf[v] = '\0';
    return std::string(cBuf);  
  }
  
  // We use one-byte bools
  bool FS::readBool(FileHandle *pfh) {
    return static_cast<bool>(readByte(pfh));
  }   

  void FS::writeByte(FileHandle *pfh,unsigned char v) {
    if(!writeBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"writeByte -> failed");
  }
  
  void FS::writeShort(FileHandle *pfh,short v) {
    if(!writeBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"writeShort -> failed");
  }
  
  void FS::writeInt(FileHandle *pfh,int v) {
    int32_t nv;
    nv = static_cast<int32_t>(v);
    if(!writeBuf(pfh,(char *)&nv, 4)) _ThrowFileError(pfh,"writeInt -> failed");
  }
  
  void FS::writeFloat(FileHandle *pfh,float v) {
    if(!writeBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"writeFloat -> failed");
  }
  
  void FS::writeDouble(FileHandle *pfh,double v) {
    if(!writeBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"writeDouble -> failed");
  }
  
  void FS::writeString(FileHandle *pfh,std::string v) {
    writeByte(pfh,v.length());
    if(!writeBuf(pfh,(char *)v.c_str(),v.length())) _ThrowFileError(pfh,"writeString -> failed");
  }
  
  void FS::writeLongString(FileHandle *pfh,std::string v) {
    writeShort_LE(pfh,v.length());
    if(!writeBuf(pfh,(char *)v.c_str(),v.length())) _ThrowFileError(pfh,"writeLongString -> failed");
  }
  
  void FS::writeBool(FileHandle *pfh,bool v) {
    writeByte(pfh,static_cast<unsigned char>(v));
  }
  
  void FS::writeLine(FileHandle *pfh,std::string Line) {
    char cBuf[512];
    sprintf(cBuf,"%s\r\n",Line.c_str());
    if(!writeBuf(pfh,cBuf,strlen(cBuf))) _ThrowFileError(pfh,"writeLine -> failed");
  }
  
  void FS::writeLineF(FileHandle *pfh,char *pcFmt,...) {
    char cBuf[512],cBuf2[512];
    va_list List;
    va_start(List,pcFmt);
    vsprintf(cBuf,pcFmt,List);
    va_end(List);
    sprintf(cBuf2,"%s\r\n",cBuf);
    if(!writeBuf(pfh,cBuf2,strlen(cBuf2))) _ThrowFileError(pfh,"writeLineF -> failed");
  }
    
  /* For buffered reading: */
  int FS::readBufferedChar(FileHandle *pfh) {
    /* Need to buffer? */
    if(pfh->nWrite==pfh->nRead)
      if(fillBuffer(pfh) == 0) return 0; /* end-of-file */
    
    /* Read buffer */
    int c=pfh->cBuffer[pfh->nRead];
    pfh->nRead++;
    if(pfh->nRead >= 256) pfh->nRead=0;
    
    /* Return */
    return c;
  } 
  
  int FS::peekNextBufferedChar(FileHandle *pfh) {
    /* Need to buffer? */
    if(pfh->nWrite==pfh->nRead)
      if(fillBuffer(pfh) == 0) return 0; /* end-of-file */
    
    /* Read buffer, but don't increment index */
    int c=pfh->cBuffer[pfh->nRead];
    
    /* Return */
    return c;
  } 
  
  int FS::fillBuffer(FileHandle *pfh) {
    int nRemaining = getLength(pfh) - getOffset(pfh);
    int nBytes = nRemaining < 256/2 ? nRemaining : 256/2;
    int r = nBytes,t;

    /* Fill half of the buffer */
    while(r>0) {
      t=(256 - pfh->nWrite) > r ? r : (256 - pfh->nWrite);
      if(!readBuf(pfh,&pfh->cBuffer[pfh->nWrite],t)) _ThrowFileError(pfh,"fillBuffer -> read failed");
      pfh->nWrite+=t;
      if(pfh->nWrite==256) pfh->nWrite=0;
      r-=t;
    }
    
    /* Return number of bytes read */
    return nBytes;
  } 
  
  /*===========================================================================
  Extract directory name from path
  ===========================================================================*/
  std::string FS::getFileDir(std::string Path) {
    int n = Path.find_last_of("/");
    if(n<0) n = Path.find_last_of("\\");
    if(n<0) {
      /* No directory info */
      return std::string(".");
    }
    std::string v_fileDir = Path.substr(0,n);
    if(v_fileDir == "") {
      v_fileDir = "/";
    }
    return v_fileDir;
  }
  
  /*===========================================================================
  Extract file name (with no extension) from path
  ===========================================================================*/
  std::string FS::getFileBaseName(std::string Path) {
    int n = Path.find_last_of("/");
    if(n<0) n = Path.find_last_of("\\");
    std::string FName;
    if(n>=0) {
      FName = Path.substr(n+1,Path.length()-n-1);     
    }
    else {
      FName = Path;      
    }
    n = FName.find_last_of(".");
    if(n<0) return FName;
    return FName.substr(0,n);
  }

  std::string FS::getFileExtension(std::string Path) {
    int n;
    n = Path.find_last_of(".");
    if(n<0) return "";
    return Path.substr(n+1, Path.length() - n - 1);
  }

  /*=========================================================================== 
  Find out how old this file thing is... (physical file system only)
  ===========================================================================*/
  int FS::getFileTimeStamp(const std::string &Path) {
    struct stat S;

    if(stat(Path.c_str(),&S)) { 
      return 0;
    }
    
    return (int)S.st_mtime; /* no we don't care about exact value */
  }
  
  /*=========================================================================== 
  Is that a dir or what? - and similar stuffin'
  ===========================================================================*/
  bool FS::isDir(std::string Path) {
    struct stat S;

    if(stat(Path.c_str(),&S)) { 
      return false;
    }
    
    if(S.st_mode & S_IFDIR) return true; /* it is a directory */
    
    return false; /* not a directory */    
  }

  bool FS::isPathAbsolute(std::string Path) {
    /* Windows? */
    #ifdef WIN32
      /* Check for drive letter */
      if(Path.length() >= 3) {
        if(isalpha(Path.at(0)) && Path.at(1)==':' && (Path.at(2)=='\\' || Path.at(2)=='/'))
          return true;
      }
    #endif
    
    /* Otherwise simply look for leading slash/backslash */
    if(Path.length() >= 1) {
      if(Path.at(0) == '\\' || Path.at(0) == '/') return true;
    }
    
    /* Not absolute */
    return false;
  }
  
  /*===========================================================================
  Delete file
  ===========================================================================*/  
  bool FS::deleteFile(const std::string &File) {
    std::string FullFile;
    
    if(m_UserDir == "") {
      Log("** Warning ** : No user directory, can't delete file: %s",File.c_str());
      return false;
    }
    
    /* Absolute file path? */
    if(isPathAbsolute(File)) {
      /* Yeah, check if file is in user directory - otherwise we won't delete 
         it */
      if(File.length() < m_UserDir.length() || File.substr(0,m_UserDir.length()) != m_UserDir) return false;
      
      FullFile = File;
    }
    else {  
      /* We can only delete user files */      
      FullFile = m_UserDir + std::string("/") + File;
    }

    if(remove(FullFile.c_str())) {
      return false;
    }

    return true;
  }
  
  /*===========================================================================
  Copy file
  ===========================================================================*/
  bool FS::copyFile(const std::string &From,const std::string &To, std::string &To_really_done) {
    /* All file copying must happen inside the user directory... */
    if(m_UserDir == "") {
      Log("** Warning ** : No user directory, can't copy file '%s' to '%s'",From.c_str(),To.c_str());
      return false;
    }

    std::string FullFrom = m_UserDir + std::string("/") + From;
    std::string FullTo = m_UserDir + std::string("/") + To;
    
    /* Does the destination file already exist? */
    FILE *fp = fopen(FullTo.c_str(),"rb");
    if(fp != NULL) {
      fclose(fp);

      /* Yup, modify its name - strip extension */
      int k = FullTo.find_last_of(".");
      std::string FullToExt,FullToBaseName;
      if(k>=0) {
        FullToBaseName = FullTo.substr(0,k);
        FullToExt = FullTo.substr(k);
      }
      else {
        FullToBaseName = FullTo;
        FullToExt = "";
      }
      
      int i = 1;
      while(1) {
        char cTemp[1024];
        sprintf(cTemp,"%s (%d)%s",FullToBaseName.c_str(),i,FullToExt.c_str());
        
        /* What about this file then? */
        fp = fopen(cTemp,"rb");
        if(fp == NULL) {
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
    FILE *in = fopen(FullFrom.c_str(),"rb");
    if(in != NULL) {
      FILE *out = fopen(FullTo.c_str(),"wb");
      if(out != NULL) {
  To_really_done = FullTo;

        /* Get input file size */
        fseek(in,0,SEEK_END);
        int nFileSize = ftell(in);
        fseek(in,0,SEEK_SET);
        int nTotalWritten = 0;
      
        /* Good, start copying */
        while(!feof(in)) {
          char cBuf[8192];
          
          int nRead = fread(cBuf,1,sizeof(cBuf),in);
          if(nRead > 0) {
            int nWritten = fwrite(cBuf,1,nRead,out);
            if(nWritten > 0) {
              nTotalWritten += nWritten;  
            }
          }
          else break;
        }
        
        /* Did we copy everything? */
        if(nTotalWritten != nFileSize) {
          /* Nope, clean up the mess and bail out */
          fclose(in);
          fclose(out);
          remove(FullTo.c_str());
          Log("** Warning ** : Failed to copy all of '%s' to '%s'",FullFrom.c_str(),FullTo.c_str());
          return false;
        }
        
        fclose(out);
      }
      else {
        fclose(in);
        Log("** Warning ** : Failed to open file for output: %s",FullTo.c_str());
        return false;      
      }
      
      fclose(in);
    }
    else {
      Log("** Warning ** : Failed to open file for input: %s",FullFrom.c_str());
      return false;
    }
    
    /* OK */
    return true;
  }
  
  /*===========================================================================
  Write message to log
  ===========================================================================*/
  void FS::writeLog(const std::string &s) {
    if(m_UserDir != "") {
      FILE *fp = fopen((m_UserDir + std::string("/xmoto.log")).c_str(),"a");
      if(fp != NULL) {
        fprintf(fp,"%s\n",s.c_str());
        fclose(fp);
      }
    }
  } 
  
  /*===========================================================================
  Initialize file system fun
  ===========================================================================*/
  std::string FS::m_UserDir="",FS::m_DataDir; /* Globals */
  bool FS::m_bGotDataDir;
  std::string FS::m_BinDataFile = "";
  int FS::m_nNumPackFiles = 0;
  PackFile FS::m_PackFiles[MAX_PACK_FILES];
    
  void FS::init(std::string AppDir) {    
    m_bGotDataDir = false;
    m_UserDir = "";
    m_DataDir = "";
  
    #if defined(WIN32) /* Windoze... */
      char cModulePath[256];
      
      int i=GetModuleFileName(GetModuleHandle(NULL),cModulePath,sizeof(cModulePath)-1);
      cModulePath[i]='\0';
      for(i=strlen(cModulePath)-1;i>=0;i--) {
        if(cModulePath[i] == '/' || cModulePath[i] == '\\') {
          cModulePath[i] = '\0';
          break;
        }
      }
        
      /* Valid path? */
      if(isDir(cModulePath)) {
        /* Alright, use this dir */    
        m_UserDir = cModulePath;
        m_DataDir = cModulePath;     
        
        m_bGotDataDir = true;
      } 
      else throw Exception("invalid process directory");
           
    #else /* Assume unix-like */
      /* Determine users home dir, so we can find out where to get/save user 
        files */
      m_UserDir = getenv("HOME");            
      if(!isDir(m_UserDir)) 
        throw Exception("invalid user home directory");
      
      m_UserDir = m_UserDir + std::string("/.") + AppDir;
      
      /* If user-dir isn't there, try making it */
      if(!isDir(m_UserDir)) {
        if(mkdir(m_UserDir.c_str(),S_IRUSR|S_IWUSR|S_IRWXU)) { /* drwx------ */
          Log("** Warning ** : failed to create user directory '%s'!",m_UserDir.c_str());
        }
        if(!isDir(m_UserDir)) {
          /* Still no dir... */
          throw Exception("could not create user directory");
        }
      }
      
      /* If there is no Replay-dir in user-dir try making that too */
      if(!isDir(getReplaysDir())) {
        if(mkdir(getReplaysDir().c_str(),S_IRUSR|S_IWUSR|S_IRWXU)) { /* drwx------ */
          Log("** Warning ** : failed to create user replay directory '%s'!",(m_UserDir + std::string("/Replays")).c_str());
        }
        if(!isDir(getReplaysDir())) {
          /* Still no dir... */
          throw Exception("could not create user replay directory");
        }
      }

      /* Make sure we got a level cache dir */
      if(!isDir(m_UserDir + std::string("/LCache"))) {
        if(mkdir((m_UserDir + std::string("/LCache")).c_str(),S_IRUSR|S_IWUSR|S_IRWXU)) { /* drwx------ */
          Log("** Warning ** : failed to create user LCache directory '%s'!",(m_UserDir + std::string("/LCache")).c_str());
        }
        if(!isDir(m_UserDir + std::string("/LCache"))) {
          /* Still no dir... */
          throw Exception("could not create user LCache directory");
        }
      }

      /* The same goes for the /Levels dir */
      if(!isDir(getLevelsDir())) {
        if(mkdir(getLevelsDir().c_str(),S_IRUSR|S_IWUSR|S_IRWXU)) { /* drwx------ */
          Log("** Warning ** : failed to create user levels directory '%s'!",(m_UserDir + std::string("/Levels")).c_str());
        }
        if(!isDir(getLevelsDir())) {
          /* Still no dir... */
          throw Exception("could not create user levels directory");
        }
      }
      
      /* And the data dir? */
      m_DataDir = std::string(GAMEDATADIR);
      if(isDir(m_DataDir)) {
        /* Got a system-wide installation to fall back to! */
        m_bGotDataDir = true;
      }
    #endif    

    /* Delete old log */    
    remove( (m_UserDir + std::string("/xmoto.log")).c_str() );
    
    /* Info */
    Log("User directory: %s",m_UserDir.c_str());      
    if(m_bGotDataDir) {
      Log("Data directory: %s",m_DataDir.c_str());
      m_BinDataFile = m_DataDir + std::string("/xmoto.bin");
    }
    else {
      Log("Data directory: (local)");    
      m_BinDataFile = "xmoto.bin";

      /* If it is UNIX and we're running in local data mode, AND we can't find the
         xmoto.bin file, then we can safely assume the user haven't read the
         instructions :) */
      #if !defined(WIN32)
        FILE *fptest = fopen(m_BinDataFile.c_str(),"rb");
        if(fptest == NULL) {
          Log("Failed to find the data files! Please either install the program\n"
              "with 'make install' or 'cd' into the 'bin' directory, and start\n"
              "the program from there.\n");
         
          throw Exception("Can't find data!");
        }
        fclose(fptest);
      #endif
    }
          
    /* Initialize binary data package if any */
    FILE *fp = fopen(m_BinDataFile.c_str(),"rb");
    m_nNumPackFiles = 0;
    if(fp != NULL) {
      Log("Initializing binary data package...\n");
      
      char cBuf[256];
      fread(cBuf,4,1,fp);
      if(!strncmp(cBuf,"XBI1",4)) {
        int nNameLen;
        while((nNameLen=fgetc(fp)) >= 0) {
//          printf("%d  \n",nNameLen);
          /* Read file name */
          fread(cBuf,nNameLen,1,fp);
          cBuf[nNameLen] = '\0';
//          printf("      %s\n",cBuf);
          
          /* Read file size */
          int nSize;

          /* Patch by Michel Daenzer (applied 2005-10-25) */
          unsigned char nSizeBuf[4];
          fread(nSizeBuf,4,1,fp);
          nSize = nSizeBuf[0] | nSizeBuf[1] << 8 | nSizeBuf[2] << 16 | nSizeBuf[3] << 24;
          
          if(m_nNumPackFiles < MAX_PACK_FILES) {
            m_PackFiles[m_nNumPackFiles].Name = cBuf;
            m_PackFiles[m_nNumPackFiles].nOffset = ftell(fp);
            m_PackFiles[m_nNumPackFiles].nSize = nSize;
            m_nNumPackFiles ++;
          }
          else
            Log("** Warning ** : Too many files in binary data package! (Limit=%d)",MAX_PACK_FILES);          

          fseek(fp,nSize,SEEK_CUR);
        }
      }
      else
        Log("** Warning ** : Invalid binary data package format!");
      
      fclose(fp);
    }        
  }

  /*===========================================================================
  Hmm, for some reason I have to do this. Don't ask me why.
  ===========================================================================*/
  int FS::mkDir(const char *pcPath) {
    #if defined(_MSC_VER)
      return _mkdir(pcPath);
    #else
      return mkdir(pcPath,S_IRUSR|S_IWUSR|S_IRWXU);
    #endif
  }

  /*===========================================================================
  Endian-safe I/O helpers
  ===========================================================================*/
  void FS::writeShort_LE(FileHandle *pfh,short v) {    
    writeShort(pfh, SwapEndian::LittleShort(v));
  }
  
  void FS::writeInt_LE(FileHandle *pfh,int v) {
    writeInt(pfh, SwapEndian::LittleLong(v));
  }
  
  void FS::writeFloat_LE(FileHandle *pfh,float v) {
    writeFloat(pfh, SwapEndian::LittleFloat(v));
  }
  
  int FS::readShort_LE(FileHandle *pfh) {
    short v = readShort(pfh);
    return SwapEndian::LittleShort(v);
  }
  
  int FS::readInt_LE(FileHandle *pfh) {
    int v = readInt(pfh);
    return SwapEndian::LittleLong(v);
  }

  float FS::readFloat_LE(FileHandle *pfh) {
    float v = readFloat(pfh);
    return SwapEndian::LittleFloat(v);
  }  
  
  int FS::readShort_MaybeLE(FileHandle *pfh, bool big) {
    short v = readShort(pfh);
    return big ? SwapEndian::BigShort(v) : SwapEndian::LittleShort(v);
  }
  
  int FS::readInt_MaybeLE(FileHandle *pfh, bool big) {
    int v = readInt(pfh);
    return big ? SwapEndian::BigLong(v) : SwapEndian::LittleLong(v);
  }
  
  float FS::readFloat_MaybeLE(FileHandle *pfh, bool big) {
    float v = readFloat(pfh);
    return big ? SwapEndian::BigFloat(v) : SwapEndian::LittleFloat(v);
  }
  
  
  bool FS::isInUserDir(std::string p_filepath) {
    std::string v_userDir;
    std::string v_fileDir;

    v_userDir = getUserDir();
    v_fileDir = getFileDir(p_filepath);

    if(v_userDir.length() > v_fileDir.length()) {
      return false;
    }

    return v_fileDir.substr(0, v_userDir.length()) == v_userDir;
  }

  bool FS::doesDirectoryExist(std::string p_path) {
    struct stat S;
    return stat(p_path.c_str(), &S) == 0;
  }

  bool FS::isFileReadable(std::string p_filename) {
    FileHandle *fh = openIFile(p_filename);
    if(fh == NULL) {
      return false;
    }
    closeFile(fh);
    return true;
  }

  bool FS::fileExists(std::string p_filename) {
    return isFileReadable(p_filename);
  }

  void FS::mkArborescence(std::string v_filepath) {
    std::string v_parentDir = getFileDir(v_filepath);

    if(doesDirectoryExist(v_parentDir)) {
      return;
    }

    mkArborescence(v_parentDir);
    if(mkDir(v_parentDir.c_str()) != 0) {
      throw Exception("Can't create directory " + v_parentDir);
    }
  }

}

