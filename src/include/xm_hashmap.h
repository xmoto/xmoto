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
  bool operator()(std::string s1, std::string s2) {
    return s1 == s2;
  }
};

#endif
