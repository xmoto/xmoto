// source From Quake2
#include "SwapEndian.h"
#include <stdlib.h>

short  ShortSwap   (short s);
short  ShortNoSwap (short s);
int    LongSwap    (int   l);
int    LongNoSwap  (int   l);
float  FloatSwap   (float f);
float  FloatNoSwap (float f);
double DoubleSwap  (double d);
double DoubleNoSwap(double d);

short ShortSwap(short l) {
  byte b1,b2; 
  b1 = l&255;
  b2 = (l>>8)&255;
  return (b1<<8) + b2;
}

short ShortNoSwap(short l) {
  return l;
}

int LongSwap(int l) {
  byte b1,b2,b3,b4;  
  b1 = l&255;
  b2 = (l>>8)&255;
  b3 = (l>>16)&255;
  b4 = (l>>24)&255;  
  return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

int LongNoSwap(int l) {
  return l;
}

float FloatSwap(float f) {
  union {
    float	f;
    byte	b[4];
  } dat1, dat2;
  
  dat1.f = f;
  dat2.b[0] = dat1.b[3];
  dat2.b[1] = dat1.b[2];
  dat2.b[2] = dat1.b[1];
  dat2.b[3] = dat1.b[0];
  return dat2.f;
}

float FloatNoSwap(float f) {
  return f;
}

double DoubleSwap(double d) {
  /* TODO !!! */
  return d;
}

double DoubleNoSwap(double d) {
  return d;
}

/*
  ================
  Swap_Init
  ================
*/

bool SwapEndian::bigendien = false;
short	(*SwapEndian::_BigShort)    (short  s) = NULL;
short	(*SwapEndian::_LittleShort) (short  s) = NULL;
int	(*SwapEndian::_BigLong)     (int    l) = NULL;
int	(*SwapEndian::_LittleLong)  (int    l) = NULL;
float	(*SwapEndian::_BigFloat)    (float  f) = NULL;
float	(*SwapEndian::_LittleFloat) (float  f) = NULL;
double	(*SwapEndian::_BigDouble)   (double d) = NULL;
double	(*SwapEndian::_LittleDouble)(double d) = NULL;

void SwapEndian::Swap_Init() {
  byte swaptest[2] = {1,0};
  
  // set the byte swapping variables in a portable manner	
  if ( *(short *)swaptest == 1) {
    bigendien     = false;
    _BigShort     = ShortSwap;
    _LittleShort  = ShortNoSwap;
    _BigLong      = LongSwap;
    _LittleLong   = LongNoSwap;
    _BigFloat     = FloatSwap;
    _LittleFloat  = FloatNoSwap;
    _BigDouble    = DoubleSwap;
    _LittleDouble = DoubleNoSwap;
  } else {
    bigendien    = true;
    _BigShort    = ShortNoSwap;
    _LittleShort = ShortSwap;
    _BigLong     = LongNoSwap;
    _LittleLong  = LongSwap;
    _BigFloat    = FloatNoSwap;
    _LittleFloat = FloatSwap;
    _BigDouble    = DoubleNoSwap;
    _LittleDouble = DoubleSwap;
  }
}
