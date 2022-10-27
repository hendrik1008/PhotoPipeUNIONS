/*
 				fitscat.h

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	The LDAC Tools
*
*	Author:		E.BERTIN, DeNIS/LDAC
*
*	Contents:	Simplified versin of the LDACTools: main include file
*
*	Last modify:	16/01/98
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/*
 * 15.07.01:
 * changed the enum type access_type to access_type_def.
 * The old name conflicted with a member of the 'catstruct'
 * structure.
 *
 * 24.01.05:
 * changed the second argument of new_key from char * to
 * const char * to avoid comoiler warnings.
 *
 * 30.01.05:
 * I adapted the definition of addhistoryto_cat to the new
 * syntax of that program (see file fitscat.c for documentation)
 *
 * I added a definition for the new function historyto_cat
 * (see file fitscat.c for documentation)
 *
 * 04.06.2007:
 * Changes to include Large File Support to the LDAC tools:
 * On 32-bit systems calls to fseek are changed to fseeko
 * which takes an argument of type 'off_t' as offset argument.
 * This argument can be turned into a 64 bit type with compiler
 * flags (_FILE_OFFSET_BITS). Similarily the command fseek is
 * changed to fseeko on 32-bit machines. The types of concerned
 * variables are changed accordingly.
 *
 * 13.08.2008:
 * I changed the prototype for the function historyto_cat (second
 * argument goes from char * to const char *) to avoid
 * a C++ warning.
 *
 * 24.08.2010:
 * I test for _FILE_OFFSET_BITS being defined now tih '#ifdef' instead
 * of '#if'. The last gived compiler remarks with the Intel 'icc'
 * compiler.
 */

/* ifdef to make it work with CPP */

#ifndef __FITSCAT_H__
#define __FITSCAT_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <sys/types.h>

#define	MAXCHARS	256	/* max. number of characters */

/*---------------------------- return messages ------------------------------*/

#ifndef	RETURN_OK
#define	RETURN_OK		0
#endif
#ifndef	RETURN_ERROR
#define	RETURN_ERROR		(-1)
#endif
#ifndef	RETURN_FATAL_ERROR
#define	RETURN_FATAL_ERROR	(-2)
#endif

/*--------------------------- FITS BitPix coding ----------------------------*/

#define		BP_BYTE		8
#define		BP_SHORT	16
#define		BP_LONG		32
#define		BP_FLOAT	(-32)
#define		BP_DOUBLE	(-64)

/*-------------------------------- macros -----------------------------------*/

/* size (in bytes) of one FITS block */

#define		FBSIZE		2880L	

/* FITS size after adding padding */

#define		PADTOTAL(x)	(((x-1)/FBSIZE+1)*FBSIZE)

/* extra size to add for padding */

#define		PADEXTRA(x)	((FBSIZE - (x%FBSIZE))% FBSIZE)

/*--------------------------------- typedefs --------------------------------*/

typedef enum            {H_INT, H_FLOAT, H_EXPO, H_BOOL, H_STRING, H_COMMENT,
			H_HCOMMENT, H_KEY}	h_type;

typedef enum		{T_BYTE, T_SHORT, T_LONG, T_FLOAT, T_DOUBLE, T_STRING}
				t_type;		/* Type of data */

						/* type of FITS-header data */
typedef enum		{WRITE_ONLY, READ_ONLY}
				access_type_def;	/* Type of access */
typedef enum		{ASCII, SKYCAT}
				output_type;    /* Type of output */


#ifdef _FILE_OFFSET_BITS
#define OFF_T	off_t
#else
#define OFF_T	long
#endif

/*------------------------------- constants ---------------------------------*/


/*---------------------------------- key ------------------------------------*/

typedef struct structkey
  {
  char		name[80];		/* name */
  char		comment[80];		/* a comment */
  void		*ptr;			/* pointer to the data */
  h_type	htype;			/* standard ``h_type'' (display) */
  t_type	ttype;			/* standard ``t_type'' (storage) */
  char		printf[80];		/* printing format (C Convention) */
  char		unit[80];		/* physical unit */
  int		naxis;			/* number of dimensions */
  int		*naxisn;		/* pointer to an array of dim. */
  int		nobj;			/* number of objects */
  int		nbytes;			/* number of bytes per element */
  int		pos;			/* position within file */
  struct structkey	*prevkey;	/* previous key within the chain */
  struct structkey	*nextkey;	/* next key within the chain */
  struct structtab	*tab;		/* (original) parent tab */
  }		keystruct;

/*------------------------------- catalog  ---------------------------------*/

typedef struct structcat
  {
  char		filename[MAXCHARS];	/* file name */
  FILE		*file;			/* pointer to the file structure */
  struct structtab *tab;		/* pointer to the first table */
  int		ntab;			/* number of tables included */
  access_type_def	access_type;		/* READ_ONLY or WRITE_ONLY */
  }		catstruct;

/*-------------------------------- table  ----------------------------------*/

typedef struct structtab
  {
  int		bitpix;			/* Bits per element */
  int		bytepix;		/* Bytes per element */
  int		naxis;			/* number of dimensions */
  int		*naxisn;		/* array of dimensions */
  int		tfields;		/* number of fields */
  int		pcount, gcount;		/* alignment of the data */
  OFF_T  	tabsize;		/* total table size (bytes) */
  char		xtension[82];		/* FITS extension type */
  char		extname[82];		/* FITS extension name */
  char		*headbuf;		/* buffer containing the header */
  int		headnblock;		/* number of FITS blocks */
  char		*bodybuf;		/* buffer containing the body */
  OFF_T		bodypos;		/* position of the body in the file */
  struct structcat *cat;		/* (original) parent catalog */
  struct structtab *prevtab, *nexttab;	/* previous and next tab in chain */
  int		seg;			/* segment position */
  int		nseg;			/* number of tab segments */
  keystruct	*key;			/* pointer to keys */
  int		nkey;			/* number of keys */
  }		tabstruct;


/*------------------------------- functions ---------------------------------*/

extern catstruct	*new_cat(int ncat),
			*read_cat(char *filename),
			*read_cats(char **filenames, int ncat);

extern tabstruct	*init_readobj(tabstruct *tab),
			*name_to_tab(catstruct *cat, char *tabname, int seg),
			*new_tab(char *tabname),
			*pos_to_tab(catstruct *cat, int pos, int seg),
			*asc2bin_tab(catstruct *catin, char *tabinname,
				catstruct *catout, char *taboutname);

extern keystruct	*name_to_key(tabstruct *tab, char *keyname),
			*new_key(const char *keyname),
			*pos_to_key(tabstruct *tab, int pos),
			*read_key(tabstruct *tab, char *keyname);

extern void	end_readobj(tabstruct *keytab, tabstruct *tab),
		end_writeobj(catstruct *cat, tabstruct *tab),
		error(int, char *, char *),
		fixexponent(char *s),
		free_cat(catstruct *cat, int ncat),
		free_key(keystruct *key),
		free_tab(tabstruct *tab),
		init_writeobj(catstruct *cat, tabstruct *tab),
		print_obj(FILE *stream, tabstruct *tab),
		read_keys(tabstruct *tab, char **keynames, keystruct **keys,
			int nkeys, unsigned char *mask),
		read_basic(tabstruct *tab),
		readbasic_head(tabstruct *tab),
		save_cat(catstruct *cat, char *filename),
		save_tab(catstruct *cat, tabstruct *tab),
		show_keys(tabstruct *tab, char **keynames, keystruct **keys,
			int nkeys, char **SpecNames, int nspec,
                        unsigned char *mask, FILE *stream,
			int strflag,int banflag, int leadflag,
                        output_type o_type),
		swapbytes(void *, int, int),
		*ttypeconv(void *ptr, t_type ttypein, t_type ttypeout),
		warning(char *, char *);

extern char	*tdisptoprintf(char *tdisp),
		*printftotdisp(char *cprintf),
		*fitsnfind(char *fitsbuf, char *str, int nblock),
		**tabs_list(catstruct *cat, int *n),
		**keys_list(tabstruct *tab, int *n);

extern int	about_cat(catstruct *cat, FILE *stream),
		about_tab(catstruct *cat, char *tabname, FILE *stream),
		addhistoryto_cat(catstruct *cat, char *str, int timestamp),
		add_key(keystruct *key, tabstruct *tab, int pos),
		addkeyto_head(tabstruct *tab, keystruct *key),
		add_tab(tabstruct *tab, catstruct *cat, int pos),
		blank_keys(tabstruct *tab),
		close_cat(catstruct *cat),
		copy_key(tabstruct *tabin, char *keyname, tabstruct *tabout,
			int pos),
		copy_tab(catstruct *catin, char *tabname, int seg,
			catstruct *catout, int pos),
		copy_tabs(catstruct *catin, catstruct *catout),
		findkey(char *, char *, int),
		findnkey(char *, char *, int, int),
		fitsadd(char *fitsbuf, char *keyword, char *comment),
		fitsfind(char *fitsbuf, char *keyword),
		fitspick(char *fitsbuf, char *keyword, void *ptr,
			h_type *htype, t_type *ttype, char *comment),
		fitsread(char *fitsbuf, char *keyword, void *ptr,
			h_type htype, t_type ttype),
		fitsremove(char *fitsbuf, char *keyword),
		fitswrite(char *fitsbuf, char *keyword, void *ptr,
			h_type htype, t_type ttype),
		get_head(tabstruct *tab),
                historyto_cat(catstruct *, const char *, int, char **),
		inherit_cat(catstruct *catin, catstruct *catout),
		init_cat(catstruct *cat),
		map_cat(catstruct *cat),
		open_cat(catstruct *cat, access_type_def at),
		readbintabparam_head(tabstruct *tab),
		read_field(tabstruct *tab, char **keynames, keystruct **keys,
			int nkeys, int field, tabstruct *ftab),
		read_obj(tabstruct *keytab, tabstruct *tab),
		read_obj_at(tabstruct *keytab, tabstruct *tab, long pos),
		remove_key(tabstruct *tab, char *keyname),
		remove_keys(tabstruct *tab),
		remove_tab(catstruct *cat, char *tabname, int seg),
		remove_tabs(catstruct *cat),
		tab_row_len(char *, char *),
		tformof(char *str, t_type ttype, int n),
		tsizeof(char *str),
		update_head(tabstruct *tab),
		update_tab(tabstruct *tab),
		write_obj(tabstruct *tab),
		wstrncmp(char *, char *, int);

extern t_type	ttypeof(char *str);

/* ifdef to make it work with CPP */

#ifdef __cplusplus
}
#endif

#endif
