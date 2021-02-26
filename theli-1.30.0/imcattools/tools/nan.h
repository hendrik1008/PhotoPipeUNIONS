/* 23.01.2005:
   I corrected inaccuracies in the definition
   of _D0. It is now only defined if it was
   not already known and set according to the BSWAP
   (instead of LITTLEENDIAN) flag
*/

#ifndef _D0
  #ifdef BSWAP
        #define _D0     3
  #else
        #define _D0     0
  #endif
#endif

#define _DOFF   4
#define _DMAX   ((1<<(15-_DOFF))-1)
#define _DNAN   (0x8000|_DMAX<<_DOFF|1<<(_DOFF-1))

#define NBITS   (48+_DOFF)
#if _D0
#define INIT(w0)        0, 0, 0, w0
#else
#define INIT(w0)        w0, 0, 0, 0
#endif

typedef const union {
        unsigned short _W[4];
        double  _D;
} _Dconst;

