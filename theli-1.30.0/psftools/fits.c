/**
 ** various routines for reading and writing fits and ascii files
 **
 ** basic I/O ops provided by
 **
 ** write_fits()
 ** read_fits()			-- allocates memory for header + data
 ** write_ascii()
 ** read_ascii()		-- allocates memory for header + data
 **
 ** also exist functions for reading header and data lines separately
 **
 ** read_xxx_head() -- allocates memory for header strings
 ** read_xxx_line() -- assumes memory is allocated by the calling prog
 **
 ** comment handling:
 ** 	argsToString(int argc, char **argv, char *string)
 ** 	add_comment(int argc, char **argv, int *comc, char **comv)
 **
 ** routines read and write to and from stdout and stdin by default
 ** but output can be redirected to any other file descriptor using
 ** 	set_fits_opf()
 **	set_fits_ipf()
 **
 ** this version only supports short int (16 bit) files
 **
 ** have now added versions which can read and write floats, ints as well
 ** as shorts.
 **	void	fwrite_fits(float **f, int N1, int N2, int comc, char *comv[]);
 **	void	fread_fits(float ***f, int *N1, int *N2, int *comc, char *comv[]);
 **
 ** you can set the output pixel type
 **	void	set_output_pixtype(int thetype);
 **
 ** all of above use 2-D images only
 **
 ** N-Dimensional versions of read_fits_head, write_fits_head, write_fits_tail:
 ** read_fits_head_ND()
 ** write_fits_head_ND()
 ** write_fits_tail_ND()
 **
 **/

/*

   ???? added byte swapping for reading fits

   04.07.99: added some WCS keywords to the read_fits_header command
             included ED error handling in some places

   23.01.01: substituted the "theipf" variable by stdin everywhere
             and the "theopf" by stdout. The static definitions in the
             program are not accepted by Linux. This disables the possibility <              to change the input/output stream.
             included "fitscat.h" (swapbytes definition)
             removed unused variables

   19.01.01: substituted back the theipf and theopf in place for stdin, stdout.
             The input/output streams have to be set in the programs
             explicitely now (the old versions did not work uner Linux/Alpha)

   10.01.05: I removed compiler warnings under gcc with optimisation
             enabled (-O3).
*/


#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "fits.h"
#include "arrays.h"

#include "utils_globals.h"
#include "utils_macros.h"
#include "fitscat.h"

FILE	*theipf;
FILE	*theopf;


/* default pixel types */
static	int	bitpixout = SHORT_PIXTYPE;
static	int	bitpixin  = SHORT_PIXTYPE;

/******************************* swapbytes **********************************/
/*
Swap bytes for doubles, longs and shorts (for DEC machines or PC for inst.).
*/
/* void	swapbytes(void *ptr, int nb, int n)
{
 char	*cp;
 int	j;

cp = (char *)ptr;

if (nb&4)
  {
  for (j=n; j--; cp+=4)
    {
    cp[0] ^= (cp[3]^=(cp[0]^=cp[3]));
    cp[1] ^= (cp[2]^=(cp[1]^=cp[2]));
    }
  return;
  }

if (nb&2)
  {
  for (j=n; j--; cp+=2)
    cp[0] ^= (cp[1]^=(cp[0]^=cp[1]));
  return;
  }

if (nb&1)
  return;

if (nb&8)
  {
  for (j=n; j--; cp+=8)
    {
    cp[0] ^= (cp[7]^=(cp[0]^=cp[7]));
    cp[1] ^= (cp[6]^=(cp[1]^=cp[6]));
    cp[2] ^= (cp[5]^=(cp[2]^=cp[5]));
    cp[3] ^= (cp[4]^=(cp[3]^=cp[4]));
    }
  return;
  }

error_exit("*Internal Error*: Unknown size in swapbytes()");

return;
}
*/

void	set_output_pixtype(int thetype)
{
	switch (thetype) {
		case SHORT_PIXTYPE:
			bitpixout = SHORT_PIXTYPE;
			break;
		case FLOAT_PIXTYPE:
			bitpixout = FLOAT_PIXTYPE;
			break;
		case INT_PIXTYPE:
			bitpixout = INT_PIXTYPE;
			break;
		default:
			error_exit("set_output_pixtype: bad pixtype\n");
			break;
	}
}


void	get_input_pixtype(int *thetype)
{
	*thetype = bitpixin;
}



void	write_fits_head(int N1, int N2, int comc, char *comv[])
{
	int 	com = 0, icom = 0, ic;
	char	record[2880], line[COM_LENGTH];

	for (ic = 0; ic < 2880; ic++)
		record[ic] = ' ';
	sprintf(line, "SIMPLE  =                    T /");
	strncpy(record + icom++, line, 32);
	sprintf(line, "BITPIX  =  %19d /", bitpixout);
	strncpy(record + icom++ * COM_LENGTH, line, 32);
	sprintf(line, "NAXIS   =                    2 /");
	strncpy(record + icom++ * COM_LENGTH, line, 32);
	sprintf(line, "NAXIS1  =  %19d /", N1);
	strncpy(record + icom++ * COM_LENGTH, line, 32);
	sprintf(line, "NAXIS2  =  %19d /", N2);
	strncpy(record + icom++ * COM_LENGTH, line, 32);
	for (com = 0; com < comc; com++) {
		strncpy(record + icom++ * COM_LENGTH, comv[com], COM_LENGTH);
		if (icom == 36) {
			fwrite(record, sizeof(char), 2880, theopf);	
			for (ic = 0; ic < 2880; ic++)
				record[ic] = ' ';
			icom = 0;
		}
	}	
	sprintf(line, "END");
	strncpy(record + icom++ * COM_LENGTH, line, 3);
	fwrite(record, sizeof(char), 2880, theopf);	
}


void	write_fits_line(short *f, int N1)
{
	fwrite(f, sizeof(short), N1, theopf);
}

void	fwrite_fits_line(float *f, int N1)
{
	int i;
	short	*fshort;
	int	*fint;

	switch (bitpixout) {
		case SHORT_PIXTYPE:
			fshort = (short *) f;
			for (i = 0; i < N1; i++)
				fshort[i] = (short) floor(0.5 + f[i]);
			fwrite(fshort, sizeof(short), N1, theopf);
			break;
		case FLOAT_PIXTYPE:
#ifdef BSWAP
                        swapbytes(f, 4, N1);
#endif
			fwrite(f, sizeof(float), N1, theopf);
#ifdef BSWAP
                        swapbytes(f, 4, N1);
#endif
			break;
		case INT_PIXTYPE:
			fint = (int *) f;
			for (i = 0; i < N1; i++)
				fint[i] = (int) floor(0.5 + f[i]);
			fwrite(fint, sizeof(int), N1, theopf);
			break;
		default:
			error_exit("fwrite_fits_line: bad pixtype\n");
	}
}

void	write_fits_tail(int N1, int N2)
{
	int 	ic, pad;
	char	record[2880];
	long	length = 0;

	switch (bitpixout) {
		case SHORT_PIXTYPE:
			length = (long) N1 * (long) N2 * sizeof(short);
			break;
		case FLOAT_PIXTYPE:
			length = (long) N1 * (long) N2 * sizeof(float);
			break;
		case INT_PIXTYPE:
			length = (long) N1 * (long) N2 * sizeof(int);
			break;
		default:
			error_exit("write_fits_tail: illegal pixtype\n");
			break;
	}
	pad = 2880 - length + 2880 * (length / 2880);
	for (ic = 0; ic < pad; ic++)
		record[ic] = ' ';
	fwrite(record, sizeof(char), pad, theopf);	
}


void	write_fits(short **f, int N1, int N2, int comc, char *comv[])
{
	int	i;

	write_fits_head(N1, N2, comc, comv);
	for (i = 0; i < N2; i++)
		write_fits_line(f[i], N1);
	write_fits_tail(N1, N2);
}



void	fwrite_fits(float **f, int N1, int N2, int comc, char *comv[])
{
	int	i;

	write_fits_head(N1, N2, comc, comv);
	for (i = 0; i < N2; i++)
		fwrite_fits_line(f[i], N1);
	write_fits_tail(N1, N2);
}



void	read_fits_head(int *N1, int *N2, int *comc, char *comv[])
{
	int 	naxis, bitpix, lineno, done = 0;
	char	record[2880];	
	char	*line;
	
	fread(record, sizeof(char), 2880, theipf);
	sscanf(record + 1 * COM_LENGTH + 9, "%d", &bitpix);
	switch (bitpix) {
		case SHORT_PIXTYPE:
		case FLOAT_PIXTYPE:
		case INT_PIXTYPE:
			bitpixin = bitpix;
			break;
		default:
			error_exit("read_fits_head: illegal pixel type\n");
			break;
	}
	sscanf(record + 2 * COM_LENGTH + 9, "%d", &naxis);
	sscanf(record + 3 * COM_LENGTH + 9, "%d", N1);
	sscanf(record + 4 * COM_LENGTH + 9, "%d", N2);
	*comc = 0;
	lineno = 5;
	while (!done) {
		for (; lineno < 36 ; lineno++) {
			line = record + lineno * COM_LENGTH;
			if (!strncmp(line, "END", 3)) {
				done = 1;
				break;
			}
			if (    !strncmp(line, "EXPNUM  =", 9) ||
				!strncmp(line, "OBJECT  =", 9) ||
				!strncmp(line, "RA      =", 9) ||
				!strncmp(line, "DEC     =", 9) ||
				!strncmp(line, "DATEOBS =", 9) ||
				!strncmp(line, "INTTIMER=", 9) ||
				!strncmp(line, "INTTIME =", 9) ||
				!strncmp(line, "UTIME   =", 9) ||
				!strncmp(line, "DETECTOR=", 9) ||
				!strncmp(line, "INSTRUME=", 9) ||
				!strncmp(line, "BSCALE  =", 9) ||
				!strncmp(line, "BZERO   =", 9) ||
				!strncmp(line, "ESCALE  =", 9) ||
				!strncmp(line, "PIXSIZE =", 9) ||
				!strncmp(line, "AIRMASS =", 9) ||
				!strncmp(line, "FILTER  =", 9) ||
				!strncmp(line, "CTYPE1  =", 9) ||
				!strncmp(line, "CTYPE2  =", 9) ||
				!strncmp(line, "CRVAL1  =", 9) ||
				!strncmp(line, "CRVAL2  =", 9) ||
				!strncmp(line, "CRPIX1  =", 9) ||
				!strncmp(line, "CRPIX2  =", 9) ||
				!strncmp(line, "CD1_1   =", 9) ||
				!strncmp(line, "CD2_2   =", 9) ||
				!strncmp(line, "CD2_1   =", 9) ||
				!strncmp(line, "CD1_2   =", 9) ||
				!strncmp(line, "OBSTYPE =", 9) ||
				!strncmp(line, "HISTORY", 7))
                        {
			        ED_CALLOC(comv[*comc], "read_fits_head", char, COM_LENGTH);
				/* comv[*comc] = (char *) calloc(COM_LENGTH, sizeof(char)); */
				strncpy(comv[*comc], line, COM_LENGTH);
				(*comc)++;
		        }
		}
		if (!done)
			fread(record, sizeof(char), 2880, theipf);
		lineno = 0;	
	}
}

void	read_fits_line(short *f, int N1)
{
	fread(f, sizeof(short), N1, theipf);
}

void	fread_fits_line(float *f, int N1)
{
	int	i;
	short	*fshort;
	int	*fint;

	switch (bitpixin) {
		case SHORT_PIXTYPE:
			fshort = (short *) f;
			read_fits_line(fshort, N1);
			for (i = N1 - 1; i >= 0; i--)
				f[i] = (float) fshort[i];
			break;
		case FLOAT_PIXTYPE:
			fread(f, sizeof(float), N1, theipf);
#ifdef BSWAP
                        swapbytes(f, 4, N1);
#endif
			fint = (int *) f;
			break;
		case INT_PIXTYPE:
			fint = (int *) f;
			fread(fint, sizeof(int), N1, theipf);
			for (i = 0; i < N1; i++)
				f[i] = (float) fint[i];
			break;
		default:
			error_exit("fwrite_fits_line: bad pixtype\n");
	}

}

void	read_fits(short ***f, int *N1, int *N2, int *comc, char *comv[])
{
	int 	i;
	
	read_fits_head(N1, N2, comc, comv);
	
	*f = (short **) calloc(*N2, sizeof(short *));
	if (!*f) error_exit("read_fits: memory allocation error");
	for (i = 0; i < *N2; i++)
		{
			(*f)[i] = (short *) calloc(*N1, sizeof(short));
			if (!(*f)[i])
				error_exit("read_fits: memory allocation error");
		}
	for (i = 0; i < *N2; i++) {
		read_fits_line((*f)[i], *N1);
	}
}




void	fread_fits(float ***f, int *N1, int *N2, int *comc, char *comv[])
{
	int 	i;
	
	read_fits_head(N1, N2, comc, comv);
	allocFloatArray(f, *N1, *N2);
	for (i = 0; i < *N2; i++) {
		fread_fits_line((*f)[i], *N1);
	}
}


void	write_ascii_head(int N1, int N2, int comc, char *comv[])
{
	fprintf(theopf, "%d %d\n", N1, N2);
}



void	write_ascii_line(short *f, int N1)
{
	int	j;

	for (j = 0; j < N1; j++)
		fprintf(theopf, "%4d ", f[j]);
	fprintf(theopf, "\n");
}



void	write_ascii(short **f, int N1, int N2, int comc, char *comv[])
{
	int	i;

	write_ascii_head(N1, N2, comc, comv);
	for (i = 0; i < N2; i++)
		write_ascii_line(f[i], N1);	
}



void	read_ascii_head(int *N1, int *N2, int *comc, char *comv[])
{
	*comc = 0;
	fscanf(theipf, "%d %d", N1, N2);
}



void	read_ascii_line(short *f, int N1)
{
	int	j, temp;

	for (j = 0; j < N1; j++) {
		if (EOF == fscanf(theipf, "%d", &temp))
			error_exit("read_ascii: read error");
		f[j] = (short) temp;
	}
	
}



void	read_ascii(short ***f, int *N1, int *N2, int *comc, char *comv[])
{
	int i;
	
	read_ascii_head(N1, N2, comc, comv);
	*f = (short **) calloc(*N2, sizeof(short *));
	if (!*f) error_exit("read_ascii: memory allocation error");
	for (i = 0; i < *N2; i++)
		{
			(*f)[i] = (short *) calloc(*N1, sizeof(short));
			if (!(*f)[i])
				error_exit("read_ascii: memory allocation error");
			read_ascii_line((*f)[i], *N1);
		}
}



void	argsToString(int argc, char **argv, char *string)
{
	int	i, j, pos = 0, len;
	
	for (i = 0; i < argc; i++) {
		len = strlen(argv[i]);
		for (j = 0; j < len; j++) {
			string[pos++] = argv[i][j];
			if (pos == COM_LENGTH - 2)
				break;
		}
		string[pos++] = ' ';
		if (pos > COM_LENGTH - 2)
			break;
	}
	string[pos] = '\0';
}




void	add_comment(int argc, char **argv, int *comc, char **comv)
{
	char	argstring[COM_LENGTH];

	if (*comc > MAX_COMMENTS - 1)
		return;

	comv[*comc] = (char *) calloc(COM_LENGTH, sizeof(char));
	argsToString(argc, argv, argstring);
	sprintf(comv[*comc], "HISTORY = ");
	strncpy(comv[*comc] + 10, argstring, COM_LENGTH - 12);
	comv[*comc][COM_LENGTH-1] = '\0';
	(*comc)++;
}


void	add_1comment(char *comment, int *comc, char **comv)
{
	if (*comc > MAX_COMMENTS - 1)
		return;

	comv[*comc] = (char *) calloc(COM_LENGTH, sizeof(char));
	sprintf(comv[*comc], "HISTORY = ");
	strncpy(comv[*comc] + 10, comment, COM_LENGTH - 12);
	comv[*comc][COM_LENGTH-1] = '\0';
	(*comc)++;
}


void	set_fits_opf(FILE *opf)
{
	theopf = opf;
}



void	set_fits_ipf(FILE *ipf)
{
	theipf = ipf;
}


/*
 * multidimensional image routines
 */

void	read_fits_head_ND(int *Naxes, int *N, int *comc, char *comv[])
{
	int 	i, naxes, bitpix, lineno, done = 0;
	char	record[2880];	
	char	*line;
	
	fread(record, sizeof(char), 2880, theipf);
	sscanf(record + 1 * COM_LENGTH + 9, "%d", &bitpix);
	switch (bitpix) {
		case SHORT_PIXTYPE:
		case FLOAT_PIXTYPE:
		case INT_PIXTYPE:
			bitpixin = bitpix;
			break;
		default:
			error_exit("read_fits_head: illegal pixel type\n");
			break;
	}
	sscanf(record + 2 * COM_LENGTH + 9, "%d", &naxes);
	*Naxes = naxes;
	if (naxes > 7)
		error_exit("read_fits_head_ND: too many axes\n");
	for (i = 0; i < naxes; i++)
		sscanf(record + (3 + i) * COM_LENGTH + 9, "%d", &(N[i]));
	*comc = 0;
	lineno = 3 + naxes;
	while (!done) {
		for (; lineno < 36 ; lineno++) {
			line = record + lineno * COM_LENGTH;
			if (!strncmp(line, "END", 3)) {
				done = 1;
				break;
			}
			if (!strncmp(line, "OBJECT  =", 9) ||
				!strncmp(line, "HISTORY =", 9)) {
				comv[*comc] = (char *) calloc(COM_LENGTH, sizeof(char));
				strncpy(comv[*comc], line, COM_LENGTH);
				(*comc)++;
			}
		}
		if (!done)
			fread(record, sizeof(char), 2880, theipf);
		lineno = 0;	
	}
}




void	write_fits_head_ND(int Naxes, int *N, int comc, char *comv[])
{
	int 	i, com = 0, icom, ic;
	char	record[2880], line[COM_LENGTH];
	
	for (ic = 0; ic < 2880; ic++)
		record[ic] = ' ';
	sprintf(line, "SIMPLE  =                    T /");
	strncpy(record, line, 32);
	sprintf(line, "BITPIX  =  %19d /", bitpixout);
	strncpy(record + 1 * COM_LENGTH, line, 32);
	sprintf(line, "NAXIS   =  %19d /", Naxes);
	strncpy(record + 2 * COM_LENGTH, line, 32);
	for (i = 0; i < Naxes; i++) {
		sprintf(line, "NAXIS%1d  =  %19d /", i + 1, N[i]);
		strncpy(record + (3 + i) * COM_LENGTH, line, 32);
	}
	icom = 3 + Naxes;
	for (com = 0; com < comc; com++) {
		strncpy(record + icom++ * COM_LENGTH, comv[com], COM_LENGTH);
		if (icom == 36) {
			fwrite(record, sizeof(char), 2880, theopf);	
			for (ic = 0; ic < 2880; ic++)
				record[ic] = ' ';
			icom = 0;
		}
	}
	sprintf(line, "END");
	strncpy(record + icom++ * COM_LENGTH, line, 3);
	fwrite(record, sizeof(char), 2880, theopf);	
}



void	write_fits_tail_ND(int Naxes, int *N)
{
	int 	i, ic, pad;
	char	record[2880];
	long	length = 0;

	switch (bitpixout) {
		case SHORT_PIXTYPE:
			length = (long) sizeof(short);
			break;
		case FLOAT_PIXTYPE:
			length = (long) sizeof(float);
			break;
		case INT_PIXTYPE:
			length = (long) sizeof(int);
			break;
		default:
			error_exit("write_fits_tail: illegal pixtype\n");
			break;
	}
	for (i = 0; i < Naxes; i++)
		length *= (long) N[i];
	pad = 2880 - length + 2880 * (length / 2880);
	for (ic = 0; ic < pad; ic++)
		record[ic] = ' ';
	fwrite(record, sizeof(char), pad, theopf);	
}




