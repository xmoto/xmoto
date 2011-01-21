#ifndef MD5FILE
#define MD5FILE

#include <string>
#include <stdio.h>

std::string md5file(std::string p_filename);

std::string md5Contents(std::string p_md5File);
std::string md5sum(const std::string& i_content);

#endif /* MD5FILE */
