/*
 * C version of the source extraction program used in the Astroscan pipeline
 *
 * Definitions include file
 *
 * History
 * Creation: dd 25-09-1994  E.R. Deul
 *
 */

#define BITS_PER_BYTE 8

#define MAXCHAR  255

#define PIX_BYTE     8
#define PIX_SHORT   16
#define PIX_LONG    32
#define PIX_FLOAT  -32
#define PIX_DOUBLE -64

#define FITSBLKSIZ 2880

#define DEAD         8
#define EDGE        16
#define SATURATED   32
#define TRUNCATED   64
#define BLENDED    128

#define NONE 0
#define BACKMAP 1
#define CORRECTED 2
#define THRESHOLD 3
#define OBJECTNR 4

