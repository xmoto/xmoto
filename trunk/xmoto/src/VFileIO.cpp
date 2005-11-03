/*=============================================================================
XMOTO
Copyright (C) 2005 Rasmus Neckelmann (neckelmann@gmail.com)

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
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <dirent.h>
#endif
#include <sys/stat.h>

#include "VExcept.h"
#include "VApp.h"
#include "VFileIO.h"

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
      for(int i=0;i<strlen(pc);i++) pc[i] = tolower(pc[i]);
    }
  #endif
  mbool str_match_wildcard(char *pcMWildcard,char *pcMString,mbool CaseSensitive) {
	  int i,nPos=0;
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
  	
	  for(i=0;i<strlen(pcWildcard);i++) {
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
  Code soup :)  -- this is largely copy/paste material from earlier project
  ===========================================================================*/
  std::vector<std::string> FS::findPhysFiles(std::string Files) {
    std::vector<std::string> Result;
    std::string Wildcard;
    std::string DirToSearch,AltDirToSearch,UDirToSearch = "";
    
    //printf("FIND PHYS FILES [%s]\n",Files.c_str());
    
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

    /* First seperate directory from wildcard */
    int n = Files.find_last_of('/');
    if(n<0) {
      /* No directory specified */
      DirToSearch = m_DataDir;
      AltDirToSearch = m_DataDir;      
      Wildcard = Files;
    }
    else {
      /* Seperate dir and wildcard */
      DirToSearch = m_DataDir + std::string("/") + Files.substr(0,n+1);
      UDirToSearch = m_UserDir + std::string("/") + Files.substr(0,n+1);
      AltDirToSearch = Files.substr(0,n+1);
      Wildcard = Files.substr(n+1);
    }    

    /* TODO: !!!! Quick hack, clean up please */
    
//    printf("W=%s   D=%s  AD=%s\n",Wildcard.c_str(),DirToSearch.c_str(),AltDirToSearch.c_str());    
    /* Windows? */
    #ifdef WIN32
      long fh;
      struct _finddata_t fd;
      
      if((fh = _findfirst((DirToSearch + Wildcard).c_str(),&fd)) != -1L) {
        do {
					if(strcmp(fd.name,".") && strcmp(fd.name,"..")) {
					  std::string F = DirToSearch + std::string(fd.name);
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
    #else /* Assume linux, unix... yalla yalla... */
      /* Search directories */
      struct dirent *dp;    
      DIR *dirp = opendir(DirToSearch.c_str());
      while(dirp) {
        if((dp = readdir(dirp)) != NULL) {
          if(str_match_wildcard((char *)Wildcard.c_str(),(char *)dp->d_name,true)) {
            /* Match! */
            if(strcmp(dp->d_name,".") && strcmp(dp->d_name,"..")) {
					    std::string F = DirToSearch + std::string(dp->d_name);
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
          }
        }
        else {
          closedir(dirp);
          break;        
        }
      }

      if(UDirToSearch != "") {
        dirp = opendir(UDirToSearch.c_str());
        DirToSearch = UDirToSearch;
        while(dirp) {
          if((dp = readdir(dirp)) != NULL) {
            if(str_match_wildcard((char *)Wildcard.c_str(),(char *)dp->d_name,true)) {
              /* Match! */
              if(strcmp(dp->d_name,".") && strcmp(dp->d_name,"..")) {
					      std::string F = DirToSearch + std::string(dp->d_name);
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
            }
          }
          else {
            closedir(dirp);
            break;        
          }
        }
      }

      if(AltDirToSearch != DirToSearch) {
        dirp = opendir(AltDirToSearch.c_str());
        DirToSearch = AltDirToSearch;
        while(dirp) {
          if((dp = readdir(dirp)) != NULL) {
            if(str_match_wildcard((char *)Wildcard.c_str(),(char *)dp->d_name,true)) {
              /* Match! */
              if(strcmp(dp->d_name,".") && strcmp(dp->d_name,"..")) {
					      std::string F = DirToSearch + std::string(dp->d_name);
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
            }
          }
          else {
            closedir(dirp);
            break;        
          }
        }
      }
    #endif
    
    //printf("\n");
    //for(int i=0;i<Result.size();i++) printf("%s\n",Result[i].c_str());
    
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
      /* Maybe it's in the data package then? */
      bool bGotIt = false;
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
            bGotIt = true;
          }          
          else break;
        }
      }
    
      if(!bGotIt) {
        /* Try current working dir */
        pfh->fp = fopen(Path.c_str(),"rb");
        if(pfh->fp == NULL) {
          /* No luck. Try the user-dir then */
          pfh->fp = fopen((m_UserDir + std::string("/") + Path).c_str(),"rb");        
          if(pfh->fp == NULL && m_bGotDataDir) {
            /* Not there either, the data-dir is our last chance */
            pfh->fp = fopen((m_DataDir + std::string("/") + Path).c_str(),"rb");        
          }
        }
        pfh->Type = FHT_STDIO;
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

  bool FS::readBuf(FileHandle *pfh,char *pcBuf,int nBufSize) {
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
  
  bool FS::writeBuf(FileHandle *pfh,char *pcBuf,int nBufSize) {
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
    char v;
    if(!readBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"readByte -> failed");
    return v;
  } 
  
  int FS::readShort(FileHandle *pfh) {
    short v;
    if(!readBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"readShort -> failed");
    return v;
  } 
  
  int FS::readInt(FileHandle *pfh) {
    int v;
    if(!readBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"readInt -> failed");
    return v;
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
    v=readShort(pfh);
    char cBuf[65536];  
    if(!readBuf(pfh,(char *)cBuf,v)) _ThrowFileError(pfh,"readLongString -> failed");
    cBuf[v] = '\0';
    return std::string(cBuf);  
  }
  
  bool FS::readBool(FileHandle *pfh) {
    bool v;
    if(!readBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"readBool -> failed");
    return v;
  }   

  void FS::writeByte(FileHandle *pfh,unsigned char v) {
    if(!writeBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"writeByte -> failed");
  }
  
  void FS::writeShort(FileHandle *pfh,short v) {
    if(!writeBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"writeShort -> failed");
  }
  
  void FS::writeInt(FileHandle *pfh,int v) {
    if(!writeBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"writeInt -> failed");
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
    writeShort(pfh,v.length());
    if(!writeBuf(pfh,(char *)v.c_str(),v.length())) _ThrowFileError(pfh,"writeLongString -> failed");
  }
  
  void FS::writeBool(FileHandle *pfh,bool v) {
    if(!writeBuf(pfh,(char *)&v,sizeof(v))) _ThrowFileError(pfh,"writeBool -> failed");
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
      return std::string("./");
    }
    return Path.substr(0,n+1);
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
  Initialize file system fun
  ===========================================================================*/
  std::string FS::m_UserDir,FS::m_DataDir; /* Globals */
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
      //if(!isDir(m_UserDir + std::string("/Replays"))) {
      //  if(mkdir((m_UserDir + std::string("/Replays")).c_str(),S_IRUSR|S_IWUSR|S_IRWXU)) { /* drwx------ */
      //    Log("** Warning ** : failed to create user replay directory '%s'!",(m_UserDir + std::string("/Replays")).c_str());
      //  }
      //  if(!isDir(m_UserDir + std::string("/Replays"))) {
      //    /* Still no dir... */
      //    throw Exception("could not create user replay directory");
      //  }
      //}

      /* The same goes for the /Levels dir */
      if(!isDir(m_UserDir + std::string("/Levels"))) {
        if(mkdir((m_UserDir + std::string("/Levels")).c_str(),S_IRUSR|S_IWUSR|S_IRWXU)) { /* drwx------ */
          Log("** Warning ** : failed to create user levels directory '%s'!",(m_UserDir + std::string("/Levels")).c_str());
        }
        if(!isDir(m_UserDir + std::string("/Levels"))) {
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

  
};

