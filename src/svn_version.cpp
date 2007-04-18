#include <string>

std::string svn_version() {
  std::string v_rev;
  v_rev = SVN_REV;

  if(v_rev == "") {
    v_rev = "$Revision$";
  }
  return v_rev;
}
