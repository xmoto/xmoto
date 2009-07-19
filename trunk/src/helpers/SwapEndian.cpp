// source From Quake2
#include "SwapEndian.h"
#include <stdlib.h>
#include <algorithm>

typedef unsigned char byte;

static short  ShortSwap   (short s);
static short  ShortNoSwap (short s);
static int    LongSwap    (int   l);
static int    LongNoSwap  (int   l);
static float  FloatSwap   (float f);
static float  FloatNoSwap (float f);
static double DoubleSwap  (double d);
static double DoubleNoSwap(double d);
static void   SerializedBikeStateSwap  (SerializedBikeState& sbs);
static void   SerializedBikeStateNoSwap(SerializedBikeState& sbs);

// Generic endian swapping
template <typename _T>
static void SwapInPlace(_T& n) {
  std::reverse(reinterpret_cast<byte*>(&n),
    reinterpret_cast<byte*>(&n) + sizeof(_T));
}

template <typename _T>
static _T SwapCopy(_T n) {
  SwapInPlace(n);
  return n;
}


static short ShortSwap(short l) {
  return SwapCopy(l);
}

static short ShortNoSwap(short l) {
  return l;
}

static int LongSwap(int l) {
  return SwapCopy(l);
}

static int LongNoSwap(int l) {
  return l;
}

static float FloatSwap(float f) {
  return SwapCopy(f);
}

static float FloatNoSwap(float f) {
  return f;
}

static double DoubleSwap(double d) {
  return SwapCopy(d);
}

static double DoubleNoSwap(double d) {
  return d;
}

static void SerializedBikeStateSwap (SerializedBikeState& sbs) {
  SwapInPlace(sbs.fGameTime);
  SwapInPlace(sbs.fFrameX);
  SwapInPlace(sbs.fFrameY);
  SwapInPlace(sbs.fMaxXDiff);
  SwapInPlace(sbs.fMaxYDiff);
  SwapInPlace(sbs.nRearWheelRot);
  SwapInPlace(sbs.nFrontWheelRot);
  SwapInPlace(sbs.nFrameRot);
}

static void SerializedBikeStateNoSwap(SerializedBikeState& sbs) {
  // noop
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
void	(*SwapEndian::_BigSerializedBikeState)   (SerializedBikeState& sbs)
  = NULL;
void	(*SwapEndian::_LittleSerializedBikeState)(SerializedBikeState& sbs)
  = NULL;

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
    _BigSerializedBikeState    = SerializedBikeStateSwap;
    _LittleSerializedBikeState = SerializedBikeStateNoSwap;
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
    _BigSerializedBikeState    = SerializedBikeStateNoSwap;
    _LittleSerializedBikeState = SerializedBikeStateSwap;
  }
}

void SwapEndian::write4LFloat(char *str /* size = 4 */, float f) {
  float ftmp = LittleFloat(f);
  memcpy(str, &ftmp, 4);
}

float SwapEndian::read4LFloat(const char* str) {
  float f;
  memcpy(&f, str, 4);
  return LittleFloat(f);
}
