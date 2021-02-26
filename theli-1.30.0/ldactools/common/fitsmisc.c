 /*
 				fitsmisc.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	The LDAC Tools
*
*	Author:		E.BERTIN, DeNIS/LDAC
*
*	Contents:	miscellaneous functions.
*
*	Last modify:	20/04/97
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/* HISTORY COMMENTS
 *
 * 06.05.2013:
 * I rewrote the swapbytes function without the XOR operator (code
 * transfer from a newer fits library version from Emmanuel). The
 * latter one no longer worked properly on new 64-bit Intel Darwin
 * systems. I do not yet know exactly why!
 */

#include	<ctype.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"fitscat_defs.h"
#include	"fitscat.h"

/****** wstrncmp ***************************************************************
PROTO	int wstrncmp(char *cs, char *ct, int n)
PURPOSE	simple wildcard strcmp.
INPUT	character string 1,
	character string 2,
	maximum number of characters to be compared.
OUTPUT	comparison integer (same meaning as strcmp).
NOTES	-.
AUTHOR	E. Bertin (IAP & Leiden observatory)
VERSION	15/02/96
 ***/
int	wstrncmp(char *cs, char *ct, int n)

  {
   int	diff,i;

  i = n;
  diff = 0;
  do
    {
    diff = ((*cs=='?'&&*ct)||(*ct=='?'&&*cs))?0:*cs-*ct;
    } while (!diff && --i && *(cs++) && *(ct++));

  return diff;
  }


/****** error ******************************************************************
PROTO	void error(int num, char *msg1, char *msg2)
PURPOSE	Handling of detected errors.
INPUT	Error code,
	message string 1,
	message string 2.
OUTPUT	-.
NOTES	I hope it will not be used too often!
AUTHOR	E. Bertin (IAP & Leiden observatory)
VERSION	??/??/92
 ***/
void	error(int num, char *msg1, char *msg2)
  {
  fprintf(stderr, "\n> %s%s\n\n",msg1,msg2);
  exit(num);
  }


/****** findkey ****************************************************************
PROTO	int findkey(char *str, char *key, int size)
PURPOSE	Find an item within a list of keywords.
INPUT	character string,
	an array of character strings containing the list of keywords,
	offset (in char) between each keyword.
OUTPUT	position in the list (0 = first) if keyword matched,
	RETURN_ERROR otherwise.
NOTES	the matching is case-sensitive.
AUTHOR	E. Bertin (IAP & Leiden observatory)
VERSION	15/02/96
 ***/
int	findkey(char *str, char *key, int size)

  {
  int i;

  for (i=0; key[0]; i++, key += size)
    if (!strcmp(str, key))
      return i;

  return RETURN_ERROR;
  }


/********************************* findnkey **********************************
PROTO	int findnkey(char *str, char *key, int size, int nkey)
PURPOSE	Find an item within a list of nkey keywords.
INPUT	character string,
	an array of character strings containing the list of keywords,
	offset (in char) between each keyword.
	number of keywords.
OUTPUT	position in the list (0 = first) if keyword matched,
	RETURN_ERROR otherwise.
NOTES	the matching is case-sensitive.
AUTHOR	E. Bertin (IAP & Leiden observatory)
VERSION	15/02/96
 ***/
int	findnkey(char *str, char *key, int size, int nkey)

  {
  int i;

  for (i=0; i<nkey; i++, key += size)
    if (!strcmp(str, key))
      return i;

  return RETURN_ERROR;
  }


/****** swapbytes **************************************************************
PROTO	void swapbytes(void *ptr, int nb, int n)
PURPOSE	Swap bytes for doubles, longs and shorts (DEC machines or PC for inst.).
INPUT	pointer to the array to be swapped,
	element size in bytes (1,2,4 or 8),
	number of elements.
OUTPUT	-.
NOTES	If nb = 1, nothing is done.
AUTHOR	E. Bertin (IAP & Leiden observatory)
VERSION	20/04/97
 ***/
void	swapbytes(void *ptr, int nb, int n)
  {
    char	*cp, c;
    int	j;

    cp = (char *)ptr;

    if (nb&4) {
      for (j=n; j--; cp+=4) {
        c = cp[3];
        cp[3] = cp[0];
        cp[0] = c;
        c = cp[2];
        cp[2] = cp[1];
        cp[1] = c;
      }
      return;
    }

    if (nb&2) {
      for (j=n; j--; cp+=2) {
        c = cp[1];
        cp[1] = cp[0];
        cp[0] = c;
      }
      return;
    }

    if (nb&1)
      return;

    if (nb&8) {
      for (j=n; j--; cp+=8) {
        c = cp[7];
        cp[7] = cp[0];
        cp[0] = c;
        c = cp[6];
        cp[6] = cp[1];
        cp[1] = c;
        c = cp[5];
        cp[5] = cp[2];
        cp[2] = c;
        c = cp[4];
        cp[4] = cp[3];
        cp[3] = c;
      }
      return;
    }

    error(EXIT_FAILURE, "*Internal Error*: Unknown size in ", "swapbytes()");

    return;
  }


/****** warning ****************************************************************
PROTO	void warning(char *msg1, char *msg2)
PURPOSE	Print a warning message.
INPUT	Message string 1,
	Message string 2.
OUTPUT	-.
NOTES	Messages are sent to the OUTPUT stream.
AUTHOR	E. Bertin (IAP & Leiden observatory)
VERSION	??/??/92
 ***/
void	warning(char *msg1, char *msg2)
  {
  fprintf(OUTPUT, "> WARNING: %s%s\n",msg1,msg2);
  return;
  }


