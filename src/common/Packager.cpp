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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include "Packager.h"
#include "VCommon.h"
#include "VFileIO.h"
#include "helpers/SwapEndian.h"
#include "md5sum/md5file.h"

/*===========================================================================
Unpackager
===========================================================================*/
void Packager::goUnpack(const std::string &BinFile,
                        const std::string &OutPath,
                        bool bWritePackageList) {
  printf("X-Moto Data Un-Packager!\n");
  printf("---------------------\n");

  FILE *fp_lst = NULL;

  if (bWritePackageList) {
    /* Determine path of package.lst */
    std::string PackageListPath = "";

    int i = BinFile.find_last_of('/');
    if (i < 0)
      i = BinFile.find_last_of('\\');

    if (i >= 0) {
      PackageListPath = BinFile.substr(0, i + 1) + "package.lst";
    } else {
      PackageListPath = "package.lst";
    }
    /* TODO: maybe we should warn if we're overwriting anything... */

    printf("Writing to package list: %s\n", PackageListPath.c_str());

    fp_lst = fopen(PackageListPath.c_str(), "w");
    if (fp_lst == NULL)
      printf("(Warning: failed to open file for output)\n");
  }

  /* Try opening the .bin-file */
  FILE *fp = fopen(BinFile.c_str(), "rb");
  if (fp != NULL) {
    char cBuf[256];
    char md5sum[256];
    int md5sumLen;

    fread(cBuf, 4, 1, fp);
    if (!strncmp(cBuf, "XBI3", 4)) {
      /* get bin sum */
      md5sumLen = fgetc(fp);
      fread(md5sum, md5sumLen, 1, fp);
      md5sum[md5sumLen] = '\0';
      /* this is not useful for unpack */

      int nNameLen;
      while ((nNameLen = fgetc(fp)) >= 0) {
        /* Extract name of file to extract */
        fread(cBuf, nNameLen, 1, fp);
        cBuf[nNameLen] = '\0';

        md5sumLen = fgetc(fp);
        fread(md5sum, md5sumLen, 1, fp);
        md5sum[md5sumLen] = '\0';

        /* Write name to package.lst file */
        if (fp_lst != NULL)
          fprintf(fp_lst, "%s\n", cBuf);

        printf("Extracting: %s\n", cBuf);

        /* Read file size */
        int nSize;
        unsigned char nSizeBuf[4];
        fread(nSizeBuf, 4, 1, fp);
        nSize = nSizeBuf[0] | nSizeBuf[1] << 8 | nSizeBuf[2] << 16 |
                nSizeBuf[3] << 24;

        /* Open target file for output */
        FILE *fp_out = createFile(OutPath, cBuf);
        if (fp_out != NULL) {
          /* Extract */
          char cExtractBuf[16384];
          unsigned int nRemaining = nSize;
          while (nRemaining > 0) {
            unsigned int nToExtract = sizeof(cExtractBuf) > nRemaining
                                        ? nRemaining
                                        : sizeof(cExtractBuf);
            unsigned int nExtracted = fread(cExtractBuf, 1, nToExtract, fp);
            fwrite(cExtractBuf, 1, nExtracted, fp_out);

            nRemaining -= nExtracted;
          }

          fclose(fp_out);
        } else {
          printf("(Warning: failed to open file for output)\n");

          fseek(fp, nSize, SEEK_CUR); /* skip file */
        }
      }
    } else
      printf("!!! NOT VALID .bin FILE: %s\n", BinFile.c_str());

    fclose(fp);
  } else
    printf("!!! NOT FOUND: %s\n", BinFile.c_str());

  if (fp_lst != NULL)
    fclose(fp_lst);
}

FILE *Packager::createFile(const std::string &TargetDir,
                           const std::string &Path) {
  /* Create sub-dirs first */
  std::string Rem = Path;
  std::string Extra = TargetDir;
  int i;
  while ((i = Rem.find_first_of('/') < 0 ? Rem.find_first_of('\\')
                                         : Rem.find_first_of('/')) >= 0) {
    std::string SubDir = Rem.substr(0, i);

    std::string SubDirPath = Extra + "/" + SubDir;
    Extra = SubDirPath;

    if (!XMFS::isDir(SubDirPath)) {
      /* Try making dir */
      printf("Making directory: %s\n", SubDirPath.c_str());
      XMFS::mkDir(SubDirPath.c_str());
    }

    Rem = Rem.substr(i + 1);
  }

  /* Create file */
  return fopen((TargetDir + "/" + Path).c_str(), "wb");
}

/*===========================================================================
Packager main - note that this is a VERY simplistic approach to data files
===========================================================================*/
void Packager::go(const std::string &BinFile, const std::string &DataDir) {
  char cBuf[256];

  printf("X-Moto Data Packager!\n");
  printf("---------------------\n");
  printf("Finding out which files to package...\n");

  FILE *fp;
  std::string pkglist = DataDir + "/package.lst";
  if ((fp = fopen(pkglist.c_str(), "r")) == NULL) {
    printf("Can't find the file package.lst...\n");
    return;
  }

  std::vector<std::string> FileList;

  while (!feof(fp) && fgets(cBuf, sizeof(cBuf) - 1, fp) != NULL) {
    std::string s = cBuf;

    /* Strip leading and tailing white-spaces */
    std::string::size_type lead = s.find_first_not_of("\t ");
    std::string::size_type trail = s.find_last_not_of("\t \n\r");

    if (trail != std::string::npos) {
      s = s.substr(lead, trail - lead + 1);
      // printf("  %s\n",s.c_str());

      /* Add it to the list */
      FileList.push_back(s);
    }
  }
  fclose(fp);

  /* Info */
  printf("%lu files scheduled for packaging!\n", FileList.size());
  printf("Creating package '%s'...\n", BinFile.c_str());

  /* Do it */
  fp = fopen(BinFile.c_str(), "wb");

  if (fp == NULL) {
    throw Exception("Cannot open " + BinFile);
    return;
  }
  fwrite("XBI3", 4, 1, fp);

  /* control sum */
  unsigned char c;
  std::string sum = md5file(pkglist);
  c = sum.length();
  fputc(c, fp);
  fwrite(sum.c_str(), c, 1, fp);

  for (unsigned int i = 0; i < FileList.size(); i++) {
    /* Open and load entire file into memory (yikes!! (but all files a pretty
     * small :P)) */
    FILE *in = fopen((DataDir + "/" + FileList[i]).c_str(), "rb");
    if (in != NULL) {
      fseek(in, 0, SEEK_END);
      int nSize = ftell(in);
      fseek(in, 0, SEEK_SET);
      unsigned char *pcBuf = new unsigned char[nSize];
      fread(pcBuf, nSize, 1, in);
      fclose(in);

      /* Got it. */
      unsigned char c = FileList[i].length();
      fputc(c, fp);
      fwrite(FileList[i].c_str(), c, 1, fp);

      std::string md5sum = md5file((DataDir + "/" + FileList[i]));
      c = md5sum.length();
      fputc(c, fp);
      fwrite(md5sum.c_str(), c, 1, fp);

      int LnSize = SwapEndian::LittleLong(nSize);
      fwrite(&LnSize, 4, 1, fp);
      fwrite(pcBuf, nSize, 1, fp);

      /* NEXT! */
      delete[] pcBuf;
    } else
      printf("!!! FAILED TO ADD '%s'\n", FileList[i].c_str());
  }
  fclose(fp);
}
