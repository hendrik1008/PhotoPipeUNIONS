#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "fitscat.h"
#include "tsize.h"
#include "fitscat_defs.h"
#include "utils_globals.h"
#include "options_globals.h"
#include "ssc_globals.h"

/* 21.02.01:
   substituted all long int by the LONG macro

   29.04.03:
   included 'string.h' to avoid compiler warnings

   05.05.03:
   included the tsize.h file

   11.10.03:
   function which_best:
   corrected a bug that did not assign the best channel
   coorectly. In the loop determining dminbest, dmaxbest ...
   'd' has to start at best[1], not at best[0].

   10.01.05:
   I removed compiler warnings under gcc with optimisation
   enabled (-O3).

   10.03.2005:
   function average; new funtion max:
   I introduced MIN and MAX possibilities in the channel merging.
*/

typedef enum {AVE_REG, AVE_PHOT, AVE_PHOT_ERR, AVE_ERR, AVE_FLAG,
              MIN, MAX, BEST, DETECT} mtype;

int which_best(keystruct **key, set_struct *s, int chan)
{
    int		i,j,k;
    double	*best, *d, dminbest, dmaxbest;
    int		*nbest, *n, nminbest, nmaxbest, retcode = 0;

    ED_CALLOC(best,  "which_best", double, s->nelem);
    ED_CALLOC(nbest, "which_best", int , s->nelem);
    d = best; n = nbest;
    for (i=0,j=0;i<s->nelem;i++) {
       if ((chan == -1 || chan == s->chan[i]) && key[s->chan[i]]->nobj) {
          switch (key[s->chan[i]]->ttype) {
          case T_BYTE:   *(d++) = (double) ((BYTE *)key[s->chan[i]]->ptr)[s->fpos[i]-1];
                         break;
          case T_SHORT:  *(d++) =  (double) ((short int *)key[s->chan[i]]->ptr)[s->fpos[i]-1];
                         break;
          case T_LONG:   *(d++) = (double) ((LONG *)key[s->chan[i]]->ptr)[s->fpos[i]-1];
                         break;
          case T_FLOAT:  *(d++) = (double) ((float *)key[s->chan[i]]->ptr)[s->fpos[i]-1];
                         break;
          case T_DOUBLE: *(d++) = ((double *)key[s->chan[i]]->ptr)[s->fpos[i]-1];
                         break;
          case T_STRING:
                         break;
          }
          *(n++) = i; j++;
       }
    }
    k = j;
/*
    if (chan != -1 && j == 0) {
       ED_FREE(best);
       ED_FREE(nbest);
       ED_CALLOC(best,  "which_best", double, key[chan]->nobj);
       ED_CALLOC(nbest, "which_best", int   , key[chan]->nobj);
       d = best; n = nbest;
       for (i=0; i<key[chan]->nobj; i++) {
          switch (key[chan]->ttype) {
          case T_BYTE:   *(d++) = (double) ((BYTE *)key[chan]->ptr)[i];
                         break;
          case T_SHORT:  *(d++) =  (double) ((short int *)key[chan]->ptr)[i];
                         break;
          case T_LONG:   *(d++) = (double) ((LONG *)key[chan]->ptr)[i];
                         break;
          case T_FLOAT:  *(d++) = (double) ((float *)key[chan]->ptr)[i];
                         break;
          case T_DOUBLE: *(d++) = ((double *)key[chan]->ptr)[i];
                         break;
          }
          *(n++) = i;
       }
       j=i-1;
    }
 */
    dmaxbest = dminbest = best[0];
    nmaxbest = nminbest = nbest[0];
    d = &(best[1]); n = &(nbest[1]);

    for (i=1;i<j;i++,n++,d++)
    {
       if (*d < dminbest)
       {
          dminbest = *d;
          nminbest = *n;
       }
       if (*d > dmaxbest)
       {
          dmaxbest = *d;
          nmaxbest = *n;
       }
    }

    ED_FREE(best);
    ED_FREE(nbest);
    if (control.best_type == BEST_MIN)
       retcode = nminbest;
    if (control.best_type == BEST_MAX)
       retcode = nmaxbest;
    if (chan != -1) {
       if (k == 0)
          last_best_fpos[chan] = retcode+1;
       else
          last_best_fpos[chan] = s->fpos[retcode];
    }
    return retcode;
}
/**/
int average(keystruct *outkey, keystruct **inkeys, int indx, set_struct *s,
            mtype type)
/****** average ***************************************************
 *
 *  NAME	
 *      average - Average the values of the inkeys into outkey
 *
 *  SYNOPSIS
 *      int average(keystruct *outkey, keystruct **inkeys, int nkeys,
 *                  int indx, set_struct *s)
 *
 *  FUNCTION
 *      This routine calculates the average of the first elements of
 *      each of the inkeys values and puts the result in the outkey
 *      first element value.
 *      BYTE types are ORed.
 *
 *  INPUTS
 *      outkey  - Key receiving the averaged output value
 *      inkeys  - Array of arrays of keys containing the values to be averaged
 *      indx    - Index of the output column
 *      s       - Set of objects
 *
 *  RESULTS
 *      outkey  - Array of keys (memory allocated to receive result)
 *
 *  RETURNS
 *      NONE
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *
 ******************************************************************************/
{
    int		i, j, *n;
    char	*lptr, *ptr;
    keystruct	*lkey;
    int		nbest, done;

    ptr = outkey->ptr;
    for (j=0;j<outkey->nbytes;j++) *(ptr++)=0;

    ED_CALLOC(n, "average", int, outkey->nbytes/t_size[outkey->ttype]);
    nbest = 0;
    if (control.best) {
       if (strcmp(col_chan[indx],"ALL") == 0) {
          nbest = which_best(best_key, s, -1);
       } else {
          nbest = which_best(best_key, s, atoi(col_chan[indx]));
       }
    }
    done = FALSE;
    for (j=0;j<s->nelem&&!done;j++) {
       if ((strcmp(col_chan[indx],"ALL") == 0) ||
           (type == DETECT) ||
           s->chan[j] == atoi(col_chan[indx])) {
          lkey = inkeys[j];
          while (lkey != NULL) {
             if (strcmp(lkey->name,col_input[indx]) == 0) {
                lptr = lkey->ptr;
                ptr = outkey->ptr;
                for (i=0;i<outkey->nbytes/t_size[outkey->ttype];i++) {
                   n[i]++;
                   switch (lkey->ttype) {
                   case T_BYTE:
                                switch (type) {
                                case DETECT:
                                   *((BYTE *)ptr) = *((BYTE *)lptr);
                                   if (s->chan[j] == atoi(col_chan[indx])) {
                                      done=TRUE;
                                   }
                                   n[i] = 1;
                                   break;
                                case BEST:
                                   if (j == nbest) {
                                      *((BYTE *)ptr) = *((BYTE *)lptr);
                                      done=TRUE;
                                   }
                                   n[i] = 1;
                                   break;
                                default:
                                   *((BYTE *)ptr) |= *((BYTE *)lptr);
                                }
                                ptr += t_size[T_BYTE];
                                lptr += t_size[T_BYTE];
                                break;
                   case T_SHORT:
                                switch (type) {
                                case DETECT:
                                   *((short int *)ptr)= *((short int *)lptr);
                                   if (s->chan[j] == atoi(col_chan[indx])) {
                                      done=TRUE;
                                   }
                                   n[i] = 1;
                                   break;
                                case BEST:
                                   if (j == nbest) {
                                      *((short int *)ptr) = *((short int *)lptr);
                                      done=TRUE;
                                   }
                                   n[i] = 1;
                                   break;
                                case MIN:
				  if (0 == j) {
				      *((short int *)ptr) = *((short int *)lptr);
				  }
				  else {
				      if(*((short int *)lptr) < *((short int *)ptr)) {
					*((short int *)ptr) = *((short int *)lptr);
				      }
				  }
                                  n[i] = 1;
				  break;
                                case MAX:
				  if (0 == j) {
				      *((short int *)ptr) = *((short int *)lptr);
				  }
				  else {
				      if(*((short int *)lptr) > *((short int *)ptr)) {
					*((short int *)ptr) = *((short int *)lptr);
				      }
				  }
                                  n[i] = 1;
				  break;
                                case AVE_PHOT:
                                case AVE_PHOT_ERR:
                                case AVE_ERR:
                                case AVE_REG:
                                   *((short int *)ptr) += *((short int *)lptr);
                                   break;
                                case AVE_FLAG:
                                   *((short int *)ptr) |= *((short int *)lptr);
                                   break;
                                }
                                ptr += t_size[T_SHORT];
                                lptr += t_size[T_SHORT];
                                break;
                   case T_LONG:
                                switch (type) {
                                case DETECT:
                                   *((LONG *)ptr) = *((LONG *)lptr);
                                   if (s->chan[j] == atoi(col_chan[indx])) {
                                      done=TRUE;
                                   }
                                   n[i] = 1;
                                   break;
                                case BEST:
                                   if (j == nbest) {
                                      *((LONG *)ptr) = *((LONG *)lptr);
                                      done=TRUE;
                                   }
                                   n[i] = 1;
                                   break;
                                case MIN:
				  if (0 == j) {
				      *((LONG *)ptr) = *((LONG *)lptr);
				  }
				  else {
				      if(*((LONG *)lptr) < *((LONG *)ptr)) {
					*((LONG *)ptr) = *((LONG *)lptr);
				      }
				  }
                                  n[i] = 1;
				  break;
                                case MAX:
				  if (0 == j) {
				      *((LONG *)ptr) = *((LONG *)lptr);
				  }
				  else {
				      if(*((LONG *)lptr) > *((LONG *)ptr)) {
					*((LONG *)ptr) = *((LONG *)lptr);
				      }
				  }
                                  n[i] = 1;
				  break;
                                case AVE_PHOT:
                                case AVE_PHOT_ERR:
                                case AVE_ERR:
                                case AVE_REG:
                                   *((LONG *)ptr) += *((LONG *)lptr);
                                   break;
                                case AVE_FLAG:
                                   *((LONG *)ptr) |= *((LONG *)lptr);
                                   break;
                                }
                                ptr += t_size[T_LONG];
                                lptr += t_size[T_LONG];
                                break;
                   case T_FLOAT:
                                switch (type) {
                                case DETECT:
                                   *((float *)ptr) = *((float *)lptr);
                                   if (s->chan[j] == atoi(col_chan[indx])) {
                                      done=TRUE;
                                   }
                                   n[i] = 1;
                                   break;
                                case BEST:
                                   if (j == nbest) {
                                      *((float *)ptr) = *((float *)lptr);
                                      done=TRUE;
                                   }
                                   n[i] = 1;
                                   break;
                                case MIN:
				  if (0 == j) {
				      *((float *)ptr) = *((float *)lptr);
				  }
				  else {
				      if(*((float *)lptr) < *((float *)ptr)) {
					*((float *)ptr) = *((float *)lptr);
				      }
				  }
                                  n[i] = 1;
				  break;
                                case MAX:
				  if (0 == j) {
				      *((float *)ptr) = *((float *)lptr);
				  }
				  else {
				      if(*((float *)lptr) > *((float *)ptr)) {
					*((float *)ptr) = *((float *)lptr);
				      }
				  }
                                  n[i] = 1;
				  break;
                                case AVE_FLAG:
                                case AVE_REG:
                                   *((float *)ptr) += *((float *)lptr);
                                   break;
                                case AVE_PHOT:
                                   if (*((float *)lptr) < DUMCHECK)
                                      *((float *)ptr) +=
                                        exp(*((float *)lptr)/-2.5);
                                   else
                                      n[i]--;
                                   break;
                                case AVE_PHOT_ERR:
                                   if (*((float *)lptr) < DUMCHECK)
                                      *((float *)ptr) +=
                                    (*((float *)lptr)) * (*((float *)lptr));
                                   else
                                      n[i]--;
                                   break;
                                case AVE_ERR:
                                   *((float *)ptr) +=
                                    (*((float *)lptr)) * (*((float *)lptr));
                                   break;
                                }
                                ptr += t_size[T_FLOAT];
                                lptr += t_size[T_FLOAT];
                                break;
                   case T_DOUBLE:
                                switch (type) {
                                case DETECT:
                                   *((double *)ptr) = *((double *)lptr);
                                   if (s->chan[j] == atoi(col_chan[indx])) {
                                      done=TRUE;
                                   }
                                   n[i] = 1;
                                   break;
                                case BEST:
                                   if (j == nbest) {
                                      *((double *)ptr) = *((double *)lptr);
                                      done=TRUE;
                                   }
                                   n[i] = 1;
                                   break;
                                case MIN:
				  if (0 == j) {
				      *((double *)ptr) = *((double *)lptr);
				  }
				  else {
				      if(*((double *)lptr) < *((double *)ptr)) {
					*((double *)ptr) = *((double *)lptr);
				      }
				  }
                                  n[i] = 1;
				  break;
                                case MAX:
				  if (0 == j) {
				      *((double *)ptr) = *((double *)lptr);
				  }
				  else {
				      if(*((double *)lptr) > *((double *)ptr)) {
					*((double *)ptr) = *((double *)lptr);
				      }
				  }
                                  n[i] = 1;
				  break;
                                case AVE_FLAG:
                                case AVE_REG:
                                   *((double *)ptr) += *((double *)lptr);
                                   break;
                                case AVE_PHOT:
                                   if (*((double *)lptr) < DUMCHECK)
                                      *((double *)ptr) += *((double *)lptr);
                                   else
                                      n[i]--;
                                   break;
                                case AVE_PHOT_ERR:
                                   if (*((double *)lptr) < DUMCHECK)
                                      *((double *)ptr) +=
                                     (*((double *)lptr)) * (*((double *)lptr));
                                   else
                                      n[i]--;
                                   break;
                                case AVE_ERR:
                                   *((double *)ptr) +=
                                     (*((double *)lptr)) * (*((double *)lptr));
                                }
                                ptr += t_size[T_DOUBLE];
                                lptr += t_size[T_DOUBLE];
                                break;
                   case T_STRING:
                                break;
                   }
                }
                break;
             }
             lkey = lkey->nextkey;
          }
       }
    }
/*
 *     Fill with dummy values or average correctly
 */
    ptr = outkey->ptr;
    for (i=0;i<outkey->nbytes/t_size[outkey->ttype];i++) {
       switch (outkey->ttype) {
       case T_BYTE:
                        if (n[i] == 0) {
                           *((BYTE *)ptr) = 0;
                        }
                        ptr += t_size[T_BYTE];
                        break;
       case T_SHORT:
                        if (n[i] == 0) {
                           *((short int *)ptr) = 0;
                        } else {
                           if (type == AVE_REG)
                              *((short int *)ptr) /= ((float) n[i]);
                        }
                        ptr += t_size[T_SHORT];
                        break;
       case T_LONG:
                        if (n[i] == 0) {
                           *((LONG *)ptr) = 0;
                        } else {
                           if (type == AVE_REG)
                              *((LONG *)ptr) /= ((float) n[i]);
                        }
                        ptr += t_size[T_LONG];
                        break;
       case T_FLOAT:
                        switch (type) {
                        case MIN:
                        case MAX:
                        case DETECT:
                        case BEST:
                        case AVE_FLAG:
                        case AVE_REG:
                              if (n[i] == 0) {
                                 *((float *)ptr) = 0.0;
                              } else {
                                 *((float *)ptr) /= ((float) n[i]);
                              }
                              break;
                        case AVE_PHOT:
                              if (n[i] == 0) {
                                 *((float *)ptr) = DUMMAG;
                              } else {
                                 *((float *)ptr) /= ((float) n[i]);
                                 *((float *)ptr) = -2.5 * log(*((float *)ptr));
                              }
                              break;
                        case AVE_PHOT_ERR:
                              if (n[i] == 0) {
                                 *((float *)ptr) = DUMMAG;
                              } else {
                                 *((float *)ptr) = sqrt(*((float *)ptr));
                                 *((float *)ptr) /= ((float) n[i]);
                              }
                              break;
                        case AVE_ERR:
                              if (n[i] == 0) {
                                 *((float *)ptr) = 0.0;
                              } else {
                                 *((float *)ptr) = sqrt(*((float *)ptr));
                                 *((float *)ptr) /= ((float) n[i]);
                              }
                              break;
                        }
                        ptr += t_size[T_FLOAT];
                        break;
       case T_DOUBLE:
                        switch (type) {
                        case MIN:
                        case MAX:
                        case DETECT:
                        case BEST:
                        case AVE_FLAG:
                        case AVE_REG:
                              if (n[i] == 0) {
                                 *((double *)ptr) = 0.0;
                              } else {
                                 *((double *)ptr) /= ((double) n[i]);
                              }
                              break;
                        case AVE_PHOT:
                              if (n[i] == 0) {
                                 *((double *)ptr) = DUMMAG;
                              } else {
                                 *((double *)ptr) /= ((double) n[i]);
                              }
                              break;
                        case AVE_PHOT_ERR:
                              if (n[i] == 0) {
                                 *((double *)ptr) = DUMMAG;
                              } else {
                                 *((double *)ptr) = sqrt(*((double *)ptr));
                                 *((double *)ptr) /= ((double) n[i]);
                              }
                              break;
                        case AVE_ERR:
                              if (n[i] == 0) {
                                 *((double *)ptr) = 0.0;
                              } else {
                                 *((double *)ptr) = sqrt(*((double *)ptr));
                                 *((double *)ptr) /= ((double) n[i]);
                              }
                              break;
                        }
                        ptr += t_size[T_DOUBLE];
                        break;
       case T_STRING:
                        break;
       }
    }
    ED_FREE(n);
    return TRUE;
}
/**/
int weighted(keystruct *outkey, keystruct **inkeys, int indx, set_struct *s)
{
    return average(outkey, inkeys, indx, s, AVE_REG);
}
/**/
int average_regular(keystruct *outkey, keystruct **inkeys, int indx,
                    set_struct *s)
{
    return average(outkey, inkeys, indx, s, AVE_REG);
}
/**/
int average_phot_error(keystruct *outkey, keystruct **inkeys, int indx,
                    set_struct *s)
{
    return average(outkey, inkeys, indx, s, AVE_PHOT_ERR);
}
/**/
int average_error(keystruct *outkey, keystruct **inkeys, int indx,
                    set_struct *s)
{
    return average(outkey, inkeys, indx, s, AVE_ERR);
}
/**/
int average_flag(keystruct *outkey, keystruct **inkeys, int indx,
                    set_struct *s)
{
    return average(outkey, inkeys, indx, s, AVE_FLAG);
}
/**/
int average_photometry(keystruct *outkey, keystruct **inkeys, int indx,
                    set_struct *s)
{
    return average(outkey, inkeys, indx, s, AVE_PHOT);
}
/**/
int max(keystruct *outkey, keystruct **inkeys, int indx,
                    set_struct *s)
{
    return average(outkey, inkeys, indx, s, MAX);
}
/**/
int min(keystruct *outkey, keystruct **inkeys, int indx,
                    set_struct *s)
{
    return average(outkey, inkeys, indx, s, MIN);
}
/**/
int detected(keystruct *outkey, keystruct **inkeys, int indx,
                    set_struct *s)
{
    return average(outkey, inkeys, indx, s, DETECT);
}
/**/
int best(keystruct *outkey, keystruct **inkeys, int indx,
                    set_struct *s)
{
    return average(outkey, inkeys, indx, s, BEST);
}
