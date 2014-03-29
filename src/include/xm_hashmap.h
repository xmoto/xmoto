#ifndef __XMHASHMAP_H__
#define __XMHASHMAP_H__

#ifdef __GNUC__
#if (__GNUC__ >= 3)
#include <ext/hash_map>
  namespace HashNamespace=__gnu_cxx;
#else
#include <hash_map>
#define HashNamespace std
#endif
#else // #ifdef __GNUC__
#include <hash_map>
namespace HashNamespace=std;
#endif
struct hashcmp_str {
  bool operator()(const char* s1, const char* s2) const {
    if(s1 == NULL || s2 == NULL) {
      return false;
    }
    return strcmp(s1, s2) == 0;
  }
};

#endif
