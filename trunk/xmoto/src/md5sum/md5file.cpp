#include "md5file.h"
#include "md5.h"
//#include "VFileIO.h"

#define MD5_BUFFER_SIZE 8192

std::string md5file(std::string p_filename) {

  md5_state_s md5state;
  md5_byte_t  md5digest[16];
  md5_byte_t  pcBuf[MD5_BUFFER_SIZE];
  int nb_read;

  //vapp::FileHandle *fh;
  FILE *fh;

  //fh = vapp::FS::openIFile(p_filename);
  fh = fopen(p_filename.c_str(), "rb");

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
  
  std::string sum = "";
  for(int i=0; i<16; i++) {
    char c[3];
    sprintf(c,"%02x",md5digest[i]);   
    sum += c;
  }

  return sum;
}
