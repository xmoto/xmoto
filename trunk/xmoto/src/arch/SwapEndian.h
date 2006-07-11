#ifndef __SWAPENDIAN_H__
#define __SWAPENDIAN_H__

typedef char byte;

class SwapEndian {
 public:
  static bool bigendien;

  static void   Swap_Init ();

  static short	BigShort    (short  s) {return _BigShort(s);   	}
  static short	LittleShort (short  s) {return _LittleShort(s);	}
  static int	BigLong     (int    l) {return _BigLong(l);    	}
  static int	LittleLong  (int    l) {return _LittleLong(l); 	}
  static float	BigFloat    (float  f) {return _BigFloat(f);   	}
  static float	LittleFloat (float  f) {return _LittleFloat(f);	}
  static double	BigDouble   (double d) {return _BigDouble(d);   }
  static double	LittleDouble(double d) {return _LittleDouble(d);}
  
 private:
  static short	(*_BigShort)    (short 	s);
  static short	(*_LittleShort) (short 	s);
  static int	(*_BigLong)     (int   	l);
  static int	(*_LittleLong)  (int   	l);
  static float	(*_BigFloat)    (float 	f);
  static float	(*_LittleFloat) (float 	f);
  static double	(*_BigDouble)   (double d);
  static double	(*_LittleDouble)(double d);
};

#endif /* __SWAPENDIAN_H__ */
