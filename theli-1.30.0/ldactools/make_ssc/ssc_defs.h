/*
   12.10.03:
   increased version number to 2.0.4 for fixing
   a bug in the BEST merging of objects
   (see file ssc_merge.c for more details)
*/

#ifndef MAXCHAR
#define MAXCHAR 256
#endif
#define MAXCOLS MAXCHAR
#define NEG(a) ((a>0)?-a:a);
#define POS(a) ((a>0)?a:-a);
#define DUMMAG 99
#define DUMCHECK 95
#define NPHOTS 4
#define FKEYS  11
#define NKEYS  3
#define ALL -1
/*
 * Values defining the processing state of an object
 */
#define NEW 0
#define CURRENT 1
#define RETAINED 2
#define DONE 3
#define BEST_MIN 0
#define BEST_MAX 1

#define		PROGRAM	"Source Catalog Production Program"
#define		NAME	"MakeSSC"
#define		VERS    "2.0.4"
#define		DATE	__DATE__
#define		AUTHOR	"E.R. Deul"
