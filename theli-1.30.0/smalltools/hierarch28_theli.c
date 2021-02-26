
/*----------------------------------------------------------------------------
                                    E.S.O.
 -----------------------------------------------------------------------------
   File name    :   hierarch28.c
   Author       :   N. Devillard
   Created on   :   July 14th, 1998
   Language     :   ANSI C
   Description  :   header conversion from ESO-FITS to standard FITS
 ---------------------------------------------------------------------------*/

/*
 $Id: hierarch28_theli.c,v 1.2 2018/03/28 12:06:29 thomas Exp $
 $Author: thomas $
 $Date: 2018/03/28 12:06:29 $
 $Revision: 1.2 $
 */

/* HISTORY:
   ========

   21.07.2014:
   I substituted a fprintf statements without format specifier by an
   fputs statement (security fix; fprintf commands without format
   specifier should not be used any more).
 */

/*----------------------------------------------------------------------------
                                Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>


#define NM_SIZ  		512
#define FITS_LINE		80

/*----------------------------------------------------------------------------
                   Private functions and module variables
 ---------------------------------------------------------------------------*/

static char prog_desc[] = "header conversion from ESO to standard FITS" ;

static void             usage(char *) ;
static int              convert_eso_to_std_FITS(char *, char *) ;
static void             free_keys(char **, int n) ;
static void             strip_beg_end(char *) ;
/*static size_t			get_FITS_header_size(char *) ;*/
static int
search_and_replace_kw(
    char    *   buf,
	size_t		bufsize,
    char    **  key_in,
    char    **  key_out,
    int         nk
);
static void search_rep(char *, char *, char *);

static void             generate_default_convtab(char *) ;
static char *           convert_deg_to_str( double deg ) ;

static char CONVTAB_DEFAULT[] =
"#\n\
# Example of conversion table for hierarch28\n\
#\n\
# A note about this file's format:\n\
# Any blank line or line starting with a hash is ignored.\n\
# Declare the keyword names to search and replace with:\n\
#\n\
# OLDKEYWORD1 = NEWKEYWORD1\n\
# OLDKEYWORD2 = NEWKEYWORD2\n\
# etc.\n\
#\n\
# Spaces are allowed within keyword names, to allow e.g.:\n\
#\n\
# HIERARCH ESO DET DIT  = DETDIT\n\
# HIERARCH ESO DET NDIT = DET NDIT\n\
# HIERARCH ESO DET NINT = HIERARCH ESO NINT\n\
#\n\
# The most important restriction is that new keywords shall not be\n\
# longer than the keywords they replace.\n\
#\n\
#\n\
# Translation table for basic keywords used by IRAF\n\
# -------------------------------------------------\n\
#\n\
# Note: hierarch28 will replace keywords in the main header\n\
# and also in extensions.\n\
#\n\
# Disclaimer:\n\
#   this table has been compiled to best knowledge of present\n\
#   IRAF packages. Please let us know of any addition/change.\n\
#\n\
\n\
UTC = UT\n\
LST = ST\n\
RA  = RA\n\
DEC = DEC\n\
\n\
HIERARCH ESO TEL AIRM START = AIRMASS\n\
HIERARCH ESO DPR TYPE       = IMAGETYP\n\
HIERARCH ESO INS FILT1 NAME = FILTER1\n\
HIERARCH ESO INS FILT2 NAME = FILTER2\n\
HIERARCH ESO INS FILT3 NAME = FILTER3\n\
HIERARCH ESO INS FILT4 NAME = FILTER4\n\
HIERARCH ESO INS SLIT2 NAME = SLIT\n\
HIERARCH ESO INS GRIS1 NAME = GRISM\n\
HIERARCH ESO INS GRAT NAME  = GRAT\n\
HIERARCH ESO INS GRAT1 NAME = GRAT1\n\
HIERARCH ESO INS GRAT2 NAME = GRAT2\n\
HIERARCH ESO INS GRAT WLEN  = WLEN\n\
HIERARCH ESO INS GRAT1 WLEN = WLEN1\n\
HIERARCH ESO INS GRAT2 WLEN = WLEN2\n\
HIERARCH ESO INS GRAT ORDER = ORDER\n\
\n\
#\n\
# A note for IRAF users:\n\
# hierarch28 performs a translation to the IRAF convention on the\n\
# following four keywords 'RA', 'DEC', 'UT' and 'ST'. IRAF requires\n\
# these keywords to contain the string representation of their values.\n\
#\n\
# Be aware also that the ESO convention names the keywords UTC and\n\
# LST, whereas the IRAF convention is 'UT' and 'ST'.\n\
#\n\
# e.g.\n\
#\n\
# RA      = ' 09:45:14.594'               / RA (J2000) pointing\n\
# DEC     = '-33:47:09.420'               / DEC (J2000) pointing\n\
# UT      = ' 01:17:21.950'               / UT at start\n\
# ST      = ' 08:19:59.688'               / ST at start\n\
#\n\
# The ESO standard (see http://archive.eso.org/dicb) defines these\n\
# keywords as floating point values with the units degrees for RA/DEC\n\
# and elapsed seconds since midnight for UT/ST.\n\
#\n\
# In order to have this tranlation performed, add\n\
#\n\
# RA  = RA\n\
# DEC = DEC\n\
# UTC = UT\n\
# LST = ST\n\
#\n\
# to the conversion table.\n\
#\n\
\n\
# N. Devillard - March 2000\n\
#\n\
" ;


static char CONVTAB_WFI[] =
"\n\
# No table provided yet for WFI\n\
\n" ;

/*----------------------------------------------------------------------------
                            main()
 ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    char            name_conv[NM_SIZ] ;
    char            name_in[NM_SIZ] ;

    if (argc<2) usage(argv[0]) ;
    if (!strcmp(argv[1], "-g")) {
		if (argc>2) {
			generate_default_convtab(argv[2]) ;
		} else {
			generate_default_convtab(NULL) ;
		}
        return 0 ;
    }

    strcpy(name_in, argv[1]) ;
    if (argc==3) {
        strcpy(name_conv, argv[2]) ;
    } else {
        strcpy(name_conv, "table.conv") ;
    }

    if (convert_eso_to_std_FITS(name_in, name_conv) != 0) {
        fprintf(stderr, "error during conversion: aborting\n") ;
    }
    return 0 ;
}


/*
 * This function only gives the usage for the program
 */

static void usage(char *pname)
{
	printf("\n\n");
	printf("hierarch28 (hierarch-to-eight)\n");
    printf("%s : %s\n", pname, prog_desc) ;
    printf("use : %s [options] <in> [table]\n", pname) ;
	printf("options are:\n");
	printf("\t-g            generates a generic table\n");
	printf("\t-g wfi        generates a table for WFI\n");
	printf("\n");
    printf("default conversion table name is 'table.conv'\n") ;
	printf("\n");
	printf("More help can be found in the comments included in\n");
	printf("the default conversion table. Generate one with\n");
	printf("the -g option and read it.\n");
    printf("\n\n") ;
    exit(0) ;
}


/*-------------------------------------------------------------------------*/
/**
  @name		convert_eso_to_std_FITS
  @memo		Search and replace FITS keywords in main/extension headers.
  @param	name_in		File to modify.
  @param	name_conv	Conversion table name.
  @return	int 0 if Ok, non-zero if error occurred.
  @doc

  The input file is modified in place. Keyword names are replaced
  according to the input conversion table. In some special cases, the
  keyword values are also modified to follow the IRAF convention.
 */
/*--------------------------------------------------------------------------*/


static int convert_eso_to_std_FITS(char * name_in, char * name_conv)
{
    FILE    *   convtab ;
    int         nkeys ;
    int         i ;
    char    **  key_in ;
    char    **  key_out ;
    int         fd ;
    char    *   buf ;
    char        line[NM_SIZ] ;
    char        kw1[FITS_LINE],
                kw2[FITS_LINE] ;
    int         lineno ;
    size_t		fs ;
	struct
		stat	fileinfo ;

    /*
     * Read conversion table and translate it to key_in, key_out
     */
    if ((convtab = fopen(name_conv, "r")) == NULL) {
        fprintf(stderr, "cannot open conversion table: %s\n", name_conv) ;
        return -1 ;
    }
    /*
     * First, count how many keywords we need to translate
     */
    nkeys = 0 ;
    while (fgets(line, FITS_LINE, convtab)!=NULL) {
        if ((line[0] != '#') && (line[0] != '\n')) {
            nkeys ++ ;
        }
    }
    rewind(convtab) ;

    /*
     * Allocate space to store keyword info
     */
    key_in = malloc(nkeys * sizeof(char*)) ;
    key_out = malloc(nkeys * sizeof(char*)) ;

    /*
     * Now read the file through and get the keywords
     */
    i = 0 ;
    lineno = 0 ;
    while (fgets(line, FITS_LINE, convtab)!=NULL) {
        lineno++ ;
        if ((line[0]!='#') && (line[0]!='\n')) {
            if (sscanf(line, "%[^=] = %[^;#]", kw1, kw2)!=2) {
                fprintf(stderr,
                              "*** error parsing table file %s\n", name_conv);
                fprintf(stderr, "line: %d\n", lineno) ;
                free_keys(key_in, i) ;
                free_keys(key_out, i) ;
                fclose(convtab) ;
                return -1 ;
            }
            strip_beg_end(kw1) ;
            strip_beg_end(kw2) ;
            if (strlen(kw2)>strlen(kw1)) {
                fprintf(stderr,
                        "*** error in conversion table %s (line %d)\n",
                        name_conv, lineno);
                fprintf(stderr,
                        "*** error: dest keyword is longer than original\n");
                fprintf(stderr, "orig: [%s] dest: [%s]\n", kw1, kw2);
                fclose(convtab) ;
                free_keys(key_in, i) ;
                free_keys(key_out, i) ;
                return -1 ;
            }
            key_in[i] = strdup(kw1) ;
            key_out[i] = strdup(kw2) ;
            i++ ;
        }
    }
    fclose(convtab) ;

    /*
     * Print out some information about what is being done
     */
    printf("\n\n") ;
    printf("*** hierarch28\n") ;
    printf("\n") ;
    printf("searching %s and replacing the following keywords:\n",
                 name_in) ;
    for (i=0 ; i<nkeys ; i++) {
        printf("\t[%s]\t=>\t[%s]\n", key_in[i], key_out[i]) ;
    }
    printf("\n\n") ;

    /*
     * mmap the input file entirely
     */

	if (stat(name_in, &fileinfo)!=0) {
		fprintf(stderr, "*** error: accessing file [%s]\n", name_in);
        free_keys(key_in, nkeys) ;
        free_keys(key_out, nkeys) ;
        return -1 ;
    }
    fs = (size_t)fileinfo.st_size ;
    if (fs < 1) {
        fprintf(stderr, "error getting FITS header size for %s\n", name_in);
        free_keys(key_in, nkeys) ;
        free_keys(key_out, nkeys) ;
        return -1 ;
    }
    fd = open(name_in, O_RDWR) ;
    if (fd == -1) {
        fprintf(stderr, "cannot open %s: aborting\n", name_in) ;
        free_keys(key_in, nkeys) ;
        free_keys(key_out, nkeys) ;
        return -1 ;
    }
    buf = (char*)mmap(0,
                      fs,
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED,
                      fd,
                      0) ;
    if (buf == (char*)-1) {
        perror("mmap") ;
        fprintf(stderr, "cannot mmap file: %s\n", name_in) ;
        free_keys(key_in, nkeys) ;
        free_keys(key_out, nkeys) ;
        close(fd) ;
        return -1 ;
    }

    /*
     * Apply search and replace for the input keyword lists
     */
    if (search_and_replace_kw(buf, fs, key_in, key_out, nkeys) != 0) {
        fprintf(stderr, "error while doing search and replace\n") ;
    }
    free_keys(key_in, nkeys) ;
    free_keys(key_out, nkeys) ;
    close(fd) ;
    munmap(buf, fs) ;
    return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @name		strip_beg_end
  @memo		Strips out blank characters off a character string.
  @param	s	NULL-terminated string to process.
  @return	void
  @doc

  This function removes heading and trailing blanks from a
  NULL-terminated character string. The input string is modified. The
  input string is assumed to contain only blanks or alphanumeric
  characters (like FITS keywords).
 */
/*--------------------------------------------------------------------------*/

static void strip_beg_end(char * s)
{
    int beg, len ;

    beg = 0 ;
    while (!isalnum((int)s[beg])) beg++ ;

    len = (int)strlen(s) -1 ;
    while (!isalnum((int)s[len])) len -- ;

    strncpy(s, s+beg, len-beg+1) ;
    s[len-beg+1] = (char)0 ;
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @name		free_keys
  @memo		Free memory associated to an array of keys.
  @param	keyt	Key table.
  @param	n		Number of keys in the table.
  @return	void
  @doc

  Memory was initially allocated using strdup(). This frees all the
  keys and the master table pointer.
 */
/*--------------------------------------------------------------------------*/

static void free_keys(char ** keyt, int n)
{
    int i ;

    if (n<1) return ;
    for (i=0 ; i<n ; i++) {
        free(keyt[i]) ;
    }
    free(keyt) ;
}


/*-------------------------------------------------------------------------*/
/**
  @name		search_and_replace_kw
  @memo		Search keywords and replace them over a whole buffer.
  @param	buf			Buffer to modify.
  @param	bufsize		Buffer size in bytes.
  @param	key_in		Input key table.
  @param	key_out		Output key table.
  @param	nk			Number of keys in each table (same).
  @return	int 0 if Ok, non-zero if error occurred.
  @doc

  Main replace function: it browses through the entire file to support
  keyword changes in extensions too. Heavily optimized for speed.
 */
/*--------------------------------------------------------------------------*/

static int search_and_replace_kw(
    char    *   buf,
	size_t		bufsize,
    char    **  key_in,
    char    **  key_out,
    int         nk
)
{
    char *  w ;
	int		i, j ;
	int		in_header ;
	int		match_flag ;
	int	 *	keysizes ;

	/* Pre-compute key sizes to gain time */
	keysizes = malloc(nk * sizeof(int));
	for (i=0 ; i<nk ; i++) {
		keysizes[i] = (int)strlen(key_in[i]);
	}

	/* Browse through file line by line */
	w = buf ;
	in_header=1 ;
	while ((size_t)(w-buf+FITS_LINE)<bufsize) {
		if (in_header) { /* Currently browsing a header */
			if (w[0]=='E' &&
				w[1]=='N' &&
				w[2]=='D' &&
				w[3]==' ') {
				/* Found an END keyword: exit from header */
				in_header=0 ;
			} else {
				/* Compare the current line with all searched keys */
				for (i=0 ; i<nk ; i++) {
					match_flag=1 ;
					for (j=0 ; j<keysizes[i] ; j++) {
						if (key_in[i][j]!=w[j]) {
							match_flag=0 ;
							break ;
						}
					}
					if (match_flag) {
						search_rep(w, key_in[i], key_out[i]);
					}
				}
			}
		} else {
			/* Currently out of header, look for next extension */
			if (w[0]=='X' &&
				w[1]=='T' &&
				w[2]=='E' &&
				w[3]=='N' &&
				w[4]=='S' &&
				w[5]=='I' &&
				w[6]=='O' &&
				w[7]=='N') {
				/* Found a header start */
				in_header=1 ;
			}
		}
		w+=FITS_LINE ;
	}
	free(keysizes);
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @name		search_rep
  @memo		Atomic keyword replacement.
  @param	line	Line to work on.
  @param	key_i	Input key.
  @param	key_o	Output key.
  @return	void
  @doc

  Replace in 'line' the keyword name 'key_i' by 'key_o'. In some
  special cases, the value is also modified to reflect the IRAF
  conventions.
 */
/*--------------------------------------------------------------------------*/

static void search_rep(char * line, char * key_i, char * key_o)
{
	int			i, j ;
	char	*	equal ;
	int			to_copy ;
	char	*	p ;
	char		tmp[FITS_LINE+1];
	char		comment[FITS_LINE+1];

	equal = strstr(line, "=");
	to_copy = FITS_LINE - (equal-line);
	for (i=0 ; i<(int)strlen(key_o) ; i++) {
		line[i] = key_o[i] ;
	}
	if (strlen(key_o)<=8) {
		/* Blank-pad until equal sign is reached */
		for ( ; i<8 ; i++) {
			line[i]=' ';
		}
		/* Add equal sign */
		line[i] = '=' ;
		i++ ;

		/* Handle special cases: the value also needs conversion */
		if(!strcmp(key_o, "RA")) {
			if (*(equal+2)!='\'') {
				/* out key is RA, translate to ' HH:MM:SS.SSS' */
				p = strchr(line+i, '/');
				if (p)
					strncpy(comment, p, line+FITS_LINE-p);
				sprintf(tmp, " %-29.29s %-40.40s",
						convert_deg_to_str(atof(equal+1)/15.),
						(p)? comment : "/ Right Ascension");
				memcpy(line+i, tmp, 71);
			}
		} else if(!strcmp(key_o, "DEC")) {
			if( *(equal+2)!='\'') {
				/* out key is DEC, translate to '+DD:MM:SS.SSS' */
				p = strchr(line+i, '/');
				if (p)
					strncpy(comment, p, line+FITS_LINE-p);
				sprintf(tmp, " %-29.29s %-40.40s",
						convert_deg_to_str(atof(equal+1)),
						(p)? comment : "/ Declination");
				memcpy(line+i, tmp, 71);
			}
		} else if(!strcmp(key_o, "UT")) {
			if( *(equal+2)!='\'') {
				/* out key is UT, translate to ' HH:MM:SS.SSS' */
				p = strchr(line+i, '/');
				if (p)
					strncpy(comment, p, line+FITS_LINE-p);
				sprintf(tmp, " %-29.29s %-40.40s",
						convert_deg_to_str(atof(equal+1)/3600.),
						(p)? comment : "/ UT");
				memcpy(line+i, tmp, 71);
			}
		} else if(!strcmp(key_o, "ST")) {
			if( *(equal+2)!='\'') {
				/* out key is ST, translate to ' HH:MM:SS.SSS' */
				p = strchr(line+i, '/');
				if (p)
					strncpy(comment, p, line+FITS_LINE-p);
				sprintf(tmp, " %-29.29s %-40.40s",
						convert_deg_to_str(atof(equal+1)/3600.),
						(p)? comment : "/ ST");
				memcpy(line+i, tmp, 71);
			}
		} else {
		    /* Copy line from first char after real equal sign */
		    for (j=0 ; j<to_copy ; j++) {
				line[i+j] = equal[j+1];
		    }
		    i+=to_copy-1 ;
		    /* Blank padding */
		    for ( ; i<FITS_LINE ; i++) {
				line[i]=' ';
		    }
		}
	} else {
		/* Blank padding */
		for (i=(int)strlen(key_o) ; i<(int)strlen(key_i) ; i++) {
			line[i]=' ';
		}
	}
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @name		generate_default_convtab
  @memo		Create a default conversion table.
  @param	ins		Table name.
  @return	void
  @doc

  Creates a translation table for the requested instrument. If no
  instrument is specified (ins==NULL) a default table is generated.
 */
/*--------------------------------------------------------------------------*/

static void generate_default_convtab(char * ins)
{
    FILE    *   convtab ;

    if ((convtab = fopen("table.conv", "w")) == NULL) {
        fprintf(stderr, "*** error: cannot create table.conv: aborting\n") ;
        return ;
    }

	if (ins==NULL) {
                fputs(CONVTAB_DEFAULT, convtab);
	} else if (!strcmp(ins, "wfi")) {
                fputs(CONVTAB_WFI, convtab);
	} else {
		fprintf(stderr, "*** error: invalid table name: [%s]\n", ins);
		fprintf(stderr, "*** error: aborting conversion table generation\n");
		return ;
	}
    fclose(convtab) ;
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @name		convert_deg_to_str
  @memo		Convert decimal degrees to ASCII representation.
  @param	d	Double value, decimal degrees in [-90 ; +90].
  @return	Pointer to statically allocated character string.
  @doc

  Converts an angle value from degrees to ASCII representation
  following the IRAF convention. Do not free or modify the returned
  string.
 */
/*--------------------------------------------------------------------------*/

static char * convert_deg_to_str( double deg )
{
    int d, m;
    double s;
    int sign;
    static char buf[13];

    sign = 1;
    if(deg < 0.) sign = -1;

    deg *= sign;
    d = (int)deg;
    m = (int)( (deg - d) * 60);
    s = (deg - d) * 3600. - m * 60;

    sprintf(buf, "'%c%02d:%02d:%06.3f'", (sign<0)? '-' : ' ', d, m, s);

    return(buf);
}
