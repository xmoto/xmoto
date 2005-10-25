#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "Packager.h"

namespace vapp {
 
  /*===========================================================================
  Packager main - note that this is a VERY simplistic approach to data files
  ===========================================================================*/
  void Packager::go(void) {
    printf("X-Moto Data Packager!\n");
    printf("---------------------\n");
    
    printf("Finding out which files to package...\n");
    FILE *fp=fopen("package.lst","r");
    char cBuf[256];
    
    std::vector<std::string> FileList;
    
    while(!feof(fp) && fgets(cBuf,sizeof(cBuf)-1,fp)!=NULL) {
      /* Strip leading and tailing white-spaces */
      while(cBuf[0] == ' ' || cBuf[0] == '\t') {
        char cTemp[256];
        strcpy(cTemp,&cBuf[1]); strcpy(cBuf,cTemp);
      }
      for(int i=strlen(cBuf)-1;i>=0;i--) {
        if(cBuf[i] == ' ' || cBuf[i] == '\t' || cBuf[i] == '\n' || cBuf[i] == '\r') cBuf[i]='\0';
        else break;
      }
      
      if(cBuf[0]) {
        /* Add it to the list */
        printf("  %s\n",cBuf);
        FileList.push_back(cBuf);
      }      
    }
    fclose(fp);
    
    /* Info */
    printf("%d files scheduled for packaging!\n",FileList.size());
    printf("Creating package 'xmoto.bin'...\n");
    
    /* Do it */
    fp = fopen("xmoto.bin","wb");
    fwrite("XBI1",4,1,fp);
    for(int i=0;i<FileList.size();i++) {
      /* Open and load entire file into memory (yikes!! (but all files a pretty small :P)) */
      FILE *in = fopen(FileList[i].c_str(),"rb");
      if(in != NULL) {
        fseek(in,0,SEEK_END);
        int nSize = ftell(in);
        fseek(in,0,SEEK_SET);        
        unsigned char *pcBuf = new unsigned char[nSize];
        fread(pcBuf,nSize,1,in);
        fclose(in);
        
        /* Got it. */
        unsigned char c = FileList[i].length();
        fputc(c,fp);
        fwrite(FileList[i].c_str(),c,1,fp);
        fwrite(&nSize,4,1,fp);        
        fwrite(pcBuf,nSize,1,fp);
        
        /* NEXT! */
        delete [] pcBuf;               
      }
      else printf("!!! FAILED TO ADD '%s'\n",FileList[i].c_str());
    }
    fclose(fp);
  }
  
};

