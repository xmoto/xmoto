#include "md5file.h"
#include "md5.h"
//#include "VFileIO.h"

#define MD5_BUFFER_SIZE 8192

std::string inthex(unsigned int i);
std::string byteToStr(unsigned char b);

std::string byteToStr(unsigned char b) {
  unsigned int part_a, part_b;

  part_a = ((unsigned int) b) / 16;
  part_b = ((unsigned int) b) % 16;

  return inthex(part_a) + inthex(part_b);
}

std::string inthex(unsigned int i) {
  switch(i) {
  case 0:
    return "0";
    break;
  case 1:
    return "1";
    break;
  case 2:
    return "2";
    break;
  case 3:
    return "3";
    break;
  case 4:
    return "4";
    break;
  case 5:
    return "5";
    break;
  case 6:
    return "6";
    break;
  case 7:
    return "7";
    break;
  case 8:
    return "8";
    break;
  case 9:
    return "9";
    break;
  case 10:
    return "a";
    break;
  case 11:
    return "b";
    break;
  case 12:
    return "c";
    break;
  case 13:
    return "d";
    break;
  case 14:
    return "e";
    break;
  case 15:
    return "f";
    break;
  }

  return "?";
}

std::string md5file(std::string p_filename) {

  md5_state_s md5state;
  md5_byte_t  md5digest[16];
  md5_byte_t  pcBuf[MD5_BUFFER_SIZE];
  std::string sum;
  int nb_read;

  //vapp::FileHandle *fh;
  FILE *fh;

  //fh = vapp::FS::openIFile(p_filename);
  fh = fopen(p_filename.c_str(), "r");

  if(fh == NULL) {
    return "";
  }

  md5_init(&md5state);

  //while(vapp::FS::readBuf(fh, (char *) pcBuf, MD5_BUFFER_SIZE)) {
  while( (nb_read = fread(pcBuf, sizeof(md5_byte_t), MD5_BUFFER_SIZE, fh)) > 0) {
    md5_append(&md5state, pcBuf, nb_read);
  }

  md5_finish(&md5state, md5digest);

  //vapp::FS::closeFile(fh);
  fclose(fh);

  sum = "";
  for(int i=0; i<16; i++) {
    sum += byteToStr(md5digest[i]);
  }

  return sum;
}
