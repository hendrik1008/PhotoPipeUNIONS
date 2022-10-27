/*
 				fitscat_defs.h

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	The LDAC Tools
*
*	Author:		E.BERTIN, DeNIS/LDAC
*
*	Contents:	Simplified version of the LDACTools: internal defs
*
*	Last modify:	23/01/97
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/*
 * 02.05.2006:
 * I ensure that BSWAP and INT64 are not defined multiple times.
 *
 * 04.06.2007:
 * Changes to include Large File Support to the LDAC tools:
 * On 32-bit systems calls to fseek are changed to fseeko
 * which takes an argument of type 'off_t' as offset argument.
 * This argument can be turned into a 64 bit type with compiler
 * flags (_FILE_OFFSET_BITS). Similarily the command fseek is
 * changed to fseeko on 32-bit machines. The types of concerned
 * variables are changed accordingly.
 */

#ifndef __FITSCAT_DEFS__
#define __FITSCAT_DEFS__

/*
   we include here stdlib.h to avoid redefinitions of
   EXIT_FAILURE and EXIT_SUCCESS
*/

#include<stdlib.h>
/*------------------------ what, who, when and where ------------------------*/

#define		BANNER		"LDACTools"
#define		VERSION		"1.1 (Jan 23 97)"
#define		COPYRIGHT	"Emmanuel BERTIN (bertin@iap.fr)"
#define		INSTITUTE	"IAP/Leiden"


/*----------------------------- Internal constants --------------------------*/

#define	OUTPUT		stderr	/* where all msgs are sent */
#define	DATA_BUFSIZE	(1024*1024)

#ifndef PI
#define	PI		3.14159265359	/* never met before? */
#endif

/* NOTES:
We must have:		MAXCHARS >= 16
			DATA_BUFSIZE >= 2 although DATA_BUFSIZE >= 100000
					  is better!!
*/

/*------------ Set defines according to machine's specificities -------------*/

#ifdef DEC_ALPHA
#ifndef BSWAP
#define	BSWAP
#endif
#ifndef INT64
#define	INT64
#endif
#endif

#ifdef HP_UX
#define	_HPUX_SOURCE
#endif

#ifdef PC_LINUX
#ifndef BSWAP
#define	BSWAP
#endif
#endif

#ifdef SUN_ULTRASPARC
#ifndef INT64
#define	INT64
#endif
#endif

/*--------------------- in case of missing constants ------------------------*/

#ifndef         SEEK_SET
#define         SEEK_SET        0
#endif
#ifndef         SEEK_CUR
#define         SEEK_CUR        1
#endif

#ifndef	EXIT_SUCCESS
#define	EXIT_SUCCESS		0
#endif
#ifndef	EXIT_FAILURE
#define	EXIT_FAILURE		-1
#endif

/*--------------------------------- typedefs --------------------------------*/
typedef	unsigned char	BYTE;			/* a byte */

#ifndef ULONGDEF
#define ULONGDEF
#ifdef INT64
typedef int             LONG;
typedef unsigned int    ULONG;
#else
typedef long            LONG;
typedef unsigned long   ULONG;
#endif
#endif
	
/*------------------------------- Other Macros -----------------------------*/

#ifdef _LARGEFILE_SOURCE
#define	FSEEKO	fseeko
#define	FTELLO	ftello
#else
#define	FSEEKO	fseek
#define	FTELLO	ftell
#endif

#define QFREAD(ptr, size, file, fname) \
		{if (fread(ptr, (size_t)(size), (size_t)1, file)!=1) \
		  error(EXIT_FAILURE, "*Error* while reading ", fname);;}

#define QFWRITE(ptr, size, file, fname) \
		{if (fwrite(ptr, (size_t)(size), (size_t)1, file)!=1) \
		  error(EXIT_FAILURE, "*Error* while writing ", fname);;}

#define	QFSEEK(file, offset, pos, fname) \
		{if (FSEEKO(file, (offset), pos)) \
		  error(EXIT_FAILURE,"*Error*: File positioning failed in ", \
			fname);;}

#define	QFTELL(pos, file, fname) \
		{if ((pos=FTELLO(file))==-1) \
		  error(EXIT_FAILURE,"*Error*: File position unknown in ", \
			fname);;}


#define	QFREE(x)	{if (x) free(x); x = NULL;}

#define	QCALLOC(ptr, typ, nel) \
		{if (!(ptr = (typ *)calloc((size_t)(nel),sizeof(typ)))) \
		  error(EXIT_FAILURE, "Not enough memory for ", \
			#ptr " (" #nel " elements) !");;}

#define	QMALLOC(ptr, typ, nel) \
		{if (!(ptr = (typ *)malloc((size_t)(nel)*sizeof(typ)))) \
		  error(EXIT_FAILURE, "Not enough memory for ", \
			#ptr " (" #nel " elements) !");;}

#define	QMEMCPY(ptrin, ptrout, typ, nel) \
		{if (!(ptrout = (typ *)malloc((size_t)(nel)*sizeof(typ)))) \
		  error(EXIT_FAILURE, "Not enough memory for ", \
			#ptrout " (" #nel " elements) !"); \
		memcpy(ptrout, ptrin, (size_t)(nel)*sizeof(typ));}

#define	QREALLOC(ptr, typ, nel) \
		{if (!(ptr = (typ *)realloc(ptr, (size_t)(nel)*sizeof(typ)))) \
		  error(EXIT_FAILURE, "Not enough memory for ", \
			#ptr " (" #nel " elements) !");;}

#define	RINT(x)	(int)(floor(x+0.5))


#define	QPRINTF		if (qflag) fprintf

#define	QFPRINTF(w,x)	{if (qflag) \
				fprintf(w, "\33[1M> %s\n\33[1A",x);;}


#define	QGETKEY(tab, key, keyname, dest) \
	{if (!(key = name_to_key(tab, keyname))) \
	  error(EXIT_FAILURE, "*Error*: No such parameter in catalog: ", \
			keyname); \
	 dest = key->ptr;}

#ifndef MIN
#define MIN(a,b) (a<b?a:b)
#endif

#endif
