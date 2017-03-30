#include "md5file.h"
#include "md5.h"
//#include "VFileIO.h"

#define MD5_BUFFER_SIZE 8192

std::string md5file(std::string p_filename) {

  md5_state_s md5state;
  md5_byte_t  md5digest[16];
  md5_byte_t  pcBuf[MD5_BUFFER_SIZE];
  int nb_read;

  //FileHandle *fh;
  FILE *fh;

  //fh = FS::openIFile(p_filename);
  fh = fopen(p_filename.c_str(), "rb");

  if(fh == NULL) {
    return "";
  }

  md5_init(&md5state);

  //while(FS::readBuf(fh, (char *) pcBuf, MD5_BUFFER_SIZE)) {
  while( (nb_read = fread(pcBuf, sizeof(md5_byte_t), MD5_BUFFER_SIZE, fh)) > 0) {
    md5_append(&md5state, pcBuf, nb_read);
  }

  md5_finish(&md5state, md5digest);

  //FS::closeFile(fh);
  fclose(fh);
  
  std::string sum = "";
  for(int i=0; i<16; i++) {
    char c[3];
    snprintf(c, 3, "%02x", md5digest[i]);   
    sum += c;
  }

  return sum;
}

std::string md5Contents(std::string p_md5File) {
  FILE* fh;
  fh = fopen(p_md5File.c_str(), "rb");
  if(fh == NULL) {
    return "";
  }
   
  char v_md5web[33];
  int nbread = fread(v_md5web, sizeof(char), 32, fh);
  if(nbread == 32) {
    v_md5web[32] = '\0';
  } else {
    v_md5web[0] = '\0';
  }
    
  fclose(fh);

  return std::string(v_md5web);
}

std::string md5sum(const std::string& i_content) {
  md5_state_s md5state;
  md5_byte_t  md5digest[16];
  std::string sum = "";

  md5_init(&md5state);
  md5_append(&md5state, (md5_byte_t*)(i_content.c_str()), i_content.size());
  md5_finish(&md5state, md5digest);

  for(int i=0; i<16; i++) {
    char c[3];
    snprintf(c, 3, "%02x", md5digest[i]);   
    sum += c;
  }

  return sum;
}
