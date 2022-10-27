/* 15.06.2001:
   removed compiler warnings under DEC
   (int -> unsigned int and casts to int
   from arguments of tolower, toupper)

   27.03.2006:
   I renamed sorting and indexing functions to the new naming scheme
   described in the sort_globals.h file
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "lists_globals.h"
#include "utils_globals.h"
#include "sort_globals.h"
#include "utils_macros.h"

#ifndef MAXCHAR
#define MAXCHAR 256
#endif

char *to_lower(char *s)
{
    char	   *line;
    int            i, imax = strlen(s) + 1;

    ED_MALLOC(line, "to_lower", char , imax);
    for (i=0; i<imax; i++) line[i]=tolower((int)s[i]);
    return line;
}

char *to_upper(char *s)
{
    char	*line;
    int		i, imax = strlen(s) + 1;

    ED_MALLOC(line, "to_upper", char , imax);
    for (i=0; i<imax; i++) line[i]=toupper((int)s[i]);
    return line;
}

char *strip_space(char *s)
{
    char *t;

    t = s;
    while (*t != ' ') t++;
    *t = '\0';

    return s;
}

int match(char *s, char *p)
{
    int lc, ok, ex;

    for ( ; *p; s++, p++) {
       switch(*p) {
       case '\\':                /* Literal match next char */
                 ++p;
                      /*FALLTHRU*/
       default:                  /* Literal match char */
                 if (*s != *p) return(0);
                 continue;
       case '?':                 /* Match any char */
                 if (*s == '\0') return(0);
                 continue;
       case '*':                 /* Match any chars */
                 if (*++p == '\0') return(1);
                 for ( ; *s; s++)
                    if ((match(s, p))) return(1);
                 return(0);
      case '[':                 /* Class */
                 if ((ex = (p[1] == '^' || p[1] == '!'))) ++p;
                 for (lc = 0400, ok = 0; *++p && *p != ']'; lc = *p)
                    if (*p == '-' ? *s <= *++p && *s >= lc : *s == *p)
                       ok = 1;
                 if (ok == ex) return(0);
                 continue;
      }
    }
    return(*s == '\0');
}

char *decode_path(char *p, int *ident)
{
    char	*s, *t;
    int		alternative = TRUE;

    ED_MALLOC(t, "decode_path", char, MAXCHAR);
    strcpy(t,p);
    if ((match(t,"*/n\?\?\?\?/CA*"))) {
       if ((s=strstr(t,"CA"))) {
          s++;s++;
          s[5] = '\0';
          *ident = atoi(s);
          alternative = FALSE;
       }
    }
    if ((match(t,"*/n\?\?\?\?/RS*"))) {
       if ((s=strstr(t,"RS"))) {
          s++;s++;
          s[5] = '\0';
          *ident = atoi(s);
          alternative = FALSE;
       }
    }
    if ((match(t,"*/n\?\?\?\?/MS*"))) {
       if ((s=strstr(t,"MS"))) {
          s++;s++;
          s[5] = '\0';
          *ident = atoi(s);
          alternative = FALSE;
       }
    }
    if (alternative) {
       if ((s=strrchr(t,'/')))
          s[0]='\0';
    }
    return t;
}

char *leading_zeros(int n, int nz)
{
    char        *s;
    int         j, n_t1, n_t2;

    ED_CALLOC(s, "leading_zeros", char, MAXCHAR);
    n_t1 = n_t2 = n;
    strcpy(s,"");
    if (n!= 0  && (int)log10((double)n) >= nz)
       syserr(FATAL,"leading_zeros", "Number %d does not fit in %d digits\n",
              n, nz);
    for (j=nz-1;j>=0;j--) {
       if ((n_t2 = n_t1/((int)pow((double)10,(double)j))) > 0) {
          n_t1 -= n_t2 * (int)pow((double)10,(double)j);
          sprintf(s,"%s%1d",s,n_t2);
       } else {
          sprintf(s,"%s0", s);
       }
    }
    return s;
}

char *strip_name(int n)
{
    char        *s;

    ED_MALLOC(s, "strip_name", char, MAXCHAR);
    /* changed from 5 to 7 (TE: 02-16-1998) */
    /* changed from 7 to 10 (TE: 02-16-1998) */

    sprintf(s,"RS%s",leading_zeros(n,10));

    return s;
}

char **uniq_strings(char *s, int len, int *n)
{
    char        **t, tt[MAXCHAR];
    int         i=0, j=0, k;

    ED_MALLOC(t,    "uniq_strings", char *, i+1);
    ED_MALLOC(t[i], "uniq_strings", char  , MAXCHAR);
    strncpy(t[i++], &s[j*len], len);
    for (j=1;j<*n;j++) {
       for (k=0;k<i;k++) {
          if (strncmp(t[k], &s[j*len], len) == 0) {
             break;
          }
       }
       if (k == i) {
          ED_REALLOC(t,   "uniq_strings", char *, i+1);
          ED_MALLOC(t[i], "uniq_strings", char  , MAXCHAR);
          strcpy(t[i++], &s[j*len]);
       }
    }
    *n = i;
/*
 *  Straight insertion sort
 */
    for (j=1;j<*n;j++) {
       strcpy(tt,t[j]);
       k=j-1;
       while (k >= 0 && strcmp(t[k],tt) > 0) {
          strcpy(t[k+1],t[k]);
          k--;
       }
       strcpy(t[k+1],tt);
    }
    return t;
}

int *uniq_int(int *list, int *n)
{
    int         *l_list, *o_list, i, t, j;

    if (*n == 0) return NULL;
    ED_MALLOC(l_list, "uniq", int, *n);
    for (i=0;i<*n;i++) l_list[i] = list[i];
    if (*n == 1) {
       return l_list;
    }
    ED_MALLOC(o_list, "uniq", int, *n);
    isort(*n, l_list);
    t = -999999; j = 0;
    for (i=0;i<*n;i++)
       if (t != l_list[i])
          t = o_list[j++] = l_list[i];

    *n = j;
    ED_FREE(l_list);
    return o_list;
}

int **uniq_2_int(int **list, int *n)
{
    int         **l_list, **o_list, i, t, tt, j;

    if (*n == 0) return NULL;
    ED_MALLOC(l_list, "uniq", int *, 2);
    ED_MALLOC(l_list[0], "uniq", int, *n);
    ED_MALLOC(l_list[1], "uniq", int, *n);
    for (i=0;i<*n;i++) {
       l_list[0][i] = list[0][i];
       l_list[1][i] = list[1][i];
    }
    if (*n == 1) {
       return l_list;
    }
    ED_MALLOC(o_list, "uniq", int *, 2);
    ED_MALLOC(o_list[0], "uniq", int, *n);
    ED_MALLOC(o_list[1], "uniq", int, *n);
    iisort(*n, l_list[0], l_list[1]);
    t = -999999;  tt = -999999; j = 0;
    for (i=0;i<*n;i++)
       if (t != l_list[0][i] || tt != l_list[1][i]) {
          t  = o_list[0][j]   = l_list[0][i];
          tt = o_list[1][j++] = l_list[1][i];
       }
    *n = j;
    ED_FREE(l_list[0]);
    ED_FREE(l_list[1]);
    ED_FREE(l_list);
    return o_list;
}

int string_in_list(char *s, char **t, int n)
{
    int		i;

    for (i=0;i<n;i++)
       if (strcmp(s,t[i]) == 0)
          return 1;
    return 0;
}

int strcmpcu(char *a, char *b)
{
/****** strcmpcu ***************************************************
 *
 *  NAME	
 *      strcmpcu - Compare strings like strcmp but in uppercase
 *
 *  SYNOPSIS
 *      int strcmpcu(char *a, char *b)
 *
 *  FUNCTION
 *      Compare two strings lexicographically in upper case and return
 *      equality or lexicographical difference in the same manner as
 *      the standard C function strcmp().
 *
 *  INPUTS
 *      a    - Input string
 *      b    - Comparison string
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      1    - a is lexicographically greater than b
 *      0    - a is identical to b
 *     -1    - a is lexicographically less than b
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      Is in fact similar to strcmp() but transforms all characters to
 *      upper before the comparison.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
     char *aa, *bb;
     int ia = strlen(a) + 1, ib = strlen(b) + 1;
     unsigned int i;

     ED_CALLOC(aa, "strcmpcu", char, ia);
     ED_CALLOC(bb, "strcmpcu", char, ib);
     for (i=0;i<strlen(a);i++) aa[i]=toupper((int)a[i]);
       aa[i] = '\0';
     for (i=0;i<strlen(b);i++) bb[i]=toupper((int)b[i]);
       bb[i] = '\0';
     i = strcmp(aa, bb);
     ED_FREE(aa);
     ED_FREE(bb);
     return i;
}

