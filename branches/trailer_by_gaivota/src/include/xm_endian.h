#if !defined(WIN32) && !defined(__APPLE__) && !defined(__MACH__)
  #if defined(__FreeBSD__) || defined(__OpenBSD__)
    #include <sys/endian.h>
  #else
    #include <endian.h>
  #endif
#endif
