/*
 * C version of the source extraction program used in the Astroscan pipeline
 *
 * Types definition include file
 *
 * History
 * Creation: dd 25-09-1994  E.R. Deul
 *
 */

typedef unsigned int POSINT;

typedef struct {
   int      bitpix;
   POSINT   xwid, ywid;
   int      px0, py0;
   double   xcen, ycen;
   double   dx, dy;
   double   bscale,bzero;
   float    *map;
   float    *bck;
   char     *flag, *info;
} image_struct;

typedef struct {
   double      aver;
   double      sigma;
   double      xpos;
   double      ypos;
   double      bzero, bscale;
   int         nlevs;
} back_struct;

typedef struct {
   image_struct image;
   back_struct  *backgr;
   double       backav, sigave, backsig;
} field_struct;
