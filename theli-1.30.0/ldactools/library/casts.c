/* 15.06.2001:
 * removed compiler warnings under DEC
 * (changing int to unsigned int for all the loops)
 *
 * 16.04.2003:
 * replaced all L_ENDIAN by BSWAP what is used in all
 * the other modules to indicate low endian number
 * representation
 */

/*
 *  It assumed here that the machine uses IEEE floating point notation
 */
short short_cast(unsigned char *a)
{
     union {
       unsigned char c[2];
       short s;
     } cast;
     unsigned int i;

     for (i=0;i<sizeof(short);i++) {
#ifdef BSWAP
        cast.c[sizeof(short)-1-i] = a[i];
#else
        cast.c[i] = a[i];
#endif
     }
     return cast.s;
}

int int_cast(unsigned char *a)
{
     union {
       unsigned char c[4];
       int i;
     } cast;
     unsigned int i;

     for (i=0;i<sizeof(int);i++) {
#ifdef BSWAP
        cast.c[sizeof(int)-1-i] = a[i];
#else
        cast.c[i] = a[i];
#endif
     }
     return cast.i;
}

float float_cast(unsigned char *a)
{
     union {
       unsigned char c[4];
       float f;
     } cast;
     unsigned int i;

     for (i=0;i<sizeof(float);i++) {
#ifdef BSWAP
        cast.c[sizeof(float)-1-i] = a[i];
#else
        cast.c[i] = a[i];
#endif
     }
     return cast.f;
}

double double_cast(unsigned char *a)
{
     union {
       unsigned char c[8];
       double d;
     } cast;
     unsigned int i;

     for (i=0;i<sizeof(double);i++) {
#ifdef BSWAP
        cast.c[sizeof(double)-1-i] = a[i];
#else
        cast.c[i] = a[i];
#endif
     }
     return cast.d;
}

void cast_short(unsigned char *b, short a)
{
     union {
       unsigned char c[2];
       short s;
     } cast;
     unsigned int i;

     cast.s = a;
     for (i=0;i<sizeof(short);i++) {
#ifdef BSWAP
        b[sizeof(short)-1-i] = cast.c[i];
#else
        b[i] = cast.c[i];
#endif
     }
}

void cast_int(unsigned char *b, int a)
{
     union {
       unsigned char c[4];
       int i;
     } cast;
     unsigned int i;

     cast.i = a;
     for (i=0;i<sizeof(int);i++) {
#ifdef BSWAP
        b[sizeof(int)-1-i] = cast.c[i];
#else
        b[i] = cast.c[i];
#endif
     }
}

void cast_float(unsigned char *b, float a)
{
     union {
       unsigned char c[4];
       float f;
     } cast;
     unsigned int i;

     cast.f = a;
     for (i=0;i<sizeof(float);i++) {
#ifdef BSWAP
        b[sizeof(float)-1-i] = cast.c[i];
#else
        b[i] = cast.c[i];
#endif
     }
}

void cast_double(unsigned char *b, double a)
{
     union {
       unsigned char c[8];
       double d;
     } cast;
     unsigned int i;

     cast.d = a;
     for (i=0;i<sizeof(double);i++) {
#ifdef BSWAP
        b[sizeof(double)-1-i] = cast.c[i];
#else
        b[i] = cast.c[i];
#endif
     }
}

