#ifndef __SWAPENDIAN_H__
#define __SWAPENDIAN_H__

#include <iterator>
#include "../VCommon.h"	// For XMOTO_LITTLE_ENDIAN
#include "../xmscene/Scene.h"	// For SerializedBikeState

class SwapEndian {
 public:
  static bool bigendien;

  static void   Swap_Init ();

  static short	BigShort    (short  s) {return _BigShort(s);   	}
  static short	LittleShort (short  s) {return _LittleShort(s);	}
  static int	BigLong     (int    l) {return _BigLong(l);    	}
  static int	LittleLong  (int    l) {return _LittleLong(l); 	}
  static float 	BigFloat    (float  f) {return _BigFloat(f);   	}
  static float	LittleFloat (float  f) {return _LittleFloat(f);	}
  static double	BigDouble   (double d) {return _BigDouble(d);   }
  static double	LittleDouble(double d) {return _LittleDouble(d);}
  
  static void write4LFloat(char* str /* size = 4 */, float f);
  static float read4LFloat(const char* str);

  static void   BigSerializedBikeState   (SerializedBikeState& sbs)
    { _BigSerializedBikeState   (sbs); }
  static void   LittleSerializedBikeState(SerializedBikeState& sbs)
    { _LittleSerializedBikeState(sbs); }
  
#ifdef XMOTO_LITTLE_ENDIAN
  template <typename _T> static _T LittleIter(_T p, int) { return p; }
#else
  template <typename _T>
  static std::reverse_iterator<_T> LittleIter(_T p, int len) {
  	return std::reverse_iterator<_T>(p + len);
  }
#endif
  
 private:
  static short	(*_BigShort)    (short 	s);
  static short	(*_LittleShort) (short 	s);
  static int	(*_BigLong)     (int   	l);
  static int	(*_LittleLong)  (int   	l);
  static float	(*_BigFloat)    (float 	f);
  static float	(*_LittleFloat) (float 	f);
  static double	(*_BigDouble)   (double d);
  static double	(*_LittleDouble)(double d);
  static void	(*_BigSerializedBikeState)   (SerializedBikeState& sbs);
  static void	(*_LittleSerializedBikeState)(SerializedBikeState& sbs);
};

#endif /* __SWAPENDIAN_H__ */
